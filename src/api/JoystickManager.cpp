/*
 *      Copyright (C) 2014 Garrett Brown
 *      Copyright (C) 2014 Team XBMC
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "utils/CommonIncludes.h"
#include "JoystickManager.h"
#include "Joystick.h"
#include "JoystickInterface.h"

#if defined(HAVE_DIRECT_INPUT)
  #include "directinput/JoystickInterfaceDirectInput.h"
#endif
#if defined(HAVE_XINPUT)
  #include "xinput/JoystickInterfaceXInput.h"
#endif
#if defined(HAVE_LINUX_JOYSTICK)
  #include "linux/JoystickInterfaceLinux.h"
#elif defined(HAVE_SDL)
  #include "sdl/JoystickInterfaceSDL.h"
#endif
#if defined(HAVE_COCOA)
  #include "cocoa/JoystickInterfaceCocoa.h"
#endif

#include "log/Log.h"
#include "utils/CommonMacros.h"

#include <algorithm>

using namespace JOYSTICK;
using namespace PLATFORM;

namespace JOYSTICK
{
  struct ScanResultEqual
  {
    ScanResultEqual(const CJoystick* needle) : m_needle(needle) { }

    bool operator()(const CJoystick* rhs)
    {
      if (m_needle != NULL)
        return m_needle->Equals(rhs);

      return m_needle == rhs;
    }

  private:
    const CJoystick* const m_needle;
  };

  template <class T>
  void safe_delete_vector(std::vector<T*>& vec)
  {
    for (typename std::vector<T*>::iterator it = vec.begin(); it != vec.end(); ++it)
      delete *it;
    vec.clear();
  }
}

CJoystickManager& CJoystickManager::Get(void)
{
  static CJoystickManager _instance;
  return _instance;
}

bool CJoystickManager::Initialize(void)
{
  CLockObject lock(m_joystickMutex);

#if defined(HAVE_DIRECT_INPUT)
  m_interfaces.push_back(new CJoystickInterfaceDirectInput);
#endif
#if defined(HAVE_XINPUT)
  m_interfaces.push_back(new CJoystickInterfaceXInput);
#endif
#if defined(HAVE_LINUX_JOYSTICK)
  m_interfaces.push_back(new CJoystickInterfaceLinux);
#elif defined(HAVE_SDL)
  m_interfaces.push_back(new CJoystickInterfaceSDL);
#endif
#if defined(HAVE_COCOA)
  m_interfaces.push_back(new CJoystickInterfaceCocoa);
#endif

  // Initialise all known interfaces
  for (int i = (int)m_interfaces.size() - 1; i >= 0; i--)
  {
    if (!m_interfaces.at(i)->Initialize())
    {
      esyslog("Failed to initialize interface %s", m_interfaces.at(i)->Name().c_str());
      delete m_interfaces.at(i);
      m_interfaces.erase(m_interfaces.begin() + i);
    }
  }

  return true;
}

void CJoystickManager::Deinitialize(void)
{
  CLockObject lock(m_joystickMutex);

  safe_delete_vector(m_interfaces);
  safe_delete_vector(m_joysticks);
}

bool CJoystickManager::PerformJoystickScan(std::vector<CJoystick*>& joysticks)
{
  CLockObject lock(m_joystickMutex);

  // Scan for joysticks
  std::vector<CJoystick*> scanResults;
  for (std::vector<CJoystickInterface*>::iterator itInterface = m_interfaces.begin(); itInterface != m_interfaces.end(); ++itInterface)
    (*itInterface)->ScanForJoysticks(scanResults);

  // Unregister removed joysticks
  for (int i = (int)m_joysticks.size() - 1; i >= 0; i--)
  {
    if (std::find_if(scanResults.begin(), scanResults.end(), ScanResultEqual(m_joysticks.at(i))) == scanResults.end())
    {
      delete m_joysticks.at(i);
      m_joysticks.erase(m_joysticks.begin() + i);
    }
  }

  // Register new joysticks
  for (std::vector<CJoystick*>::iterator itJoystick = scanResults.begin(); itJoystick != scanResults.end(); ++itJoystick)
  {
    bool bSuccess(false);

    if (std::find_if(m_joysticks.begin(), m_joysticks.end(), ScanResultEqual(*itJoystick)) == m_joysticks.end())
    {
      if ((*itJoystick)->Initialize())
      {
        (*itJoystick)->SetIndex(m_nextJoystickIndex++);

        isyslog("Initialized joystick %u: \"%s\" (%s), axes: %u, hats: %u, buttons: %u",
                (*itJoystick)->Index(), (*itJoystick)->Name().c_str(), (*itJoystick)->API()->Name().c_str(),
                (*itJoystick)->AxisCount(), (*itJoystick)->HatCount(), (*itJoystick)->ButtonCount());

        m_joysticks.push_back(*itJoystick);
        bSuccess = true;
      }
    }

    if (!bSuccess)
      delete *itJoystick;
  }

  joysticks = m_joysticks;

  return true;
}

CJoystick* CJoystickManager::GetJoystick(unsigned int index) const
{
  CLockObject lock(m_joystickMutex);

  for (std::vector<CJoystick*>::const_iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
  {
    if ((*it)->Index() == index)
      return *it;
  }

  return NULL;
}

bool CJoystickManager::GetEvents(std::vector<ADDON::PeripheralEvent>& events)
{
  CLockObject lock(m_joystickMutex);

  for (std::vector<CJoystick*>::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
    (*it)->GetEvents(events);

  return true;
}
