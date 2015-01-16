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
#endif
#if defined(HAVE_SDL)
  #include "sdl/JoystickInterfaceSDL.h"
#endif
#include "log/Log.h"
#include "utils/CommonMacros.h"

#include <algorithm>

using namespace JOYSTICK;
using namespace PLATFORM;

struct ScanResultEqual
{
  ScanResultEqual(const CJoystick* needle) : m_needle(needle) { }

  bool operator()(const CJoystick* rhs)
  {
    if (m_needle == NULL || rhs == NULL)
      return m_needle == rhs;

    return m_needle->API()       == rhs->API()       &&
           m_needle->Type()      == rhs->Type()      &&
           m_needle->Name()      == rhs->Name()      &&
           m_needle->VendorID()  == rhs->VendorID()  &&
           m_needle->ProductID() == rhs->ProductID();
  }

private:
  const CJoystick* const m_needle;
};

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
#endif
#if defined(HAVE_SDL)
  m_interfaces.push_back(new CJoystickInterfaceSDL);
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

  return !m_interfaces.empty();
}

void CJoystickManager::Deinitialize(void)
{
  CLockObject lock(m_joystickMutex);

  for (std::vector<CJoystickInterface*>::iterator it = m_interfaces.begin(); it != m_interfaces.end(); ++it)
    delete *it;
  m_interfaces.clear();

  for (std::vector<CJoystick*>::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
    delete *it;
  m_joysticks.clear();
}

bool CJoystickManager::PerformJoystickScan(std::vector<CJoystick*>& joysticks)
{
  CLockObject lock(m_joystickMutex);

  bool bReturn(false);

  std::vector<CJoystick*> scanResults;
  for (std::vector<CJoystickInterface*>::iterator itInterface = m_interfaces.begin(); itInterface != m_interfaces.end(); ++itInterface)
  {
    if ((*itInterface)->ScanForJoysticks(scanResults))
    {
      bReturn = true;

      // Check for removed joysticks
      std::vector<CJoystick*> removedJoysticks;
      for (std::vector<CJoystick*>::iterator itJoystick = m_joysticks.begin(); itJoystick != m_joysticks.end(); ++itJoystick)
      {
        if (*itInterface == (*itJoystick)->API())
        {
          if (std::find_if(scanResults.begin(), scanResults.end(), ScanResultEqual(*itJoystick)) == scanResults.end())
            removedJoysticks.push_back(*itJoystick);
        }
      }

      // Remove expired joysticks
      for (std::vector<CJoystick*>::iterator itRemoved = removedJoysticks.begin(); itRemoved != removedJoysticks.end(); ++itRemoved)
      {
        std::vector<CJoystick*>::iterator itJoystick = std::find(m_joysticks.begin(), m_joysticks.end(), *itRemoved);
        ASSERT(itJoystick != m_joysticks.end());
        m_joysticks.erase(itJoystick);

        delete *itRemoved;
      }

      // Add new joysticks
      for (std::vector<CJoystick*>::iterator itResult = scanResults.begin(); itResult != scanResults.end(); ++itResult)
      {
        bool bSuccess(false);

        if (std::find_if(m_joysticks.begin(), m_joysticks.end(), ScanResultEqual(*itResult)) == m_joysticks.end())
        {
          (*itResult)->SetIndex(m_nextJoystickIndex++);
          if ((*itResult)->Initialize())
          {
            m_joysticks.push_back(*itResult);
            bSuccess = true;
          }
        }

        if (!bSuccess)
          delete *itResult;
      }
    }
  }

  joysticks.assign(m_joysticks.begin(), m_joysticks.end());

  return bReturn;
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

  bool bResult(true);

  for (std::vector<CJoystick*>::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
    bResult &= (*it)->GetEvents(events);

  return bResult;
}
