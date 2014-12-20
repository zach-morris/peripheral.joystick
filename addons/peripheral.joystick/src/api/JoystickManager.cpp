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

using namespace JOYSTICK;
using namespace PLATFORM;

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
      esyslog("Failed to initialize interface %s", m_interfaces.at(i)->Name());
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

  // TODO: Manager joysticks better
  for (JoystickMap::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
  {
    for (std::vector<CJoystick*>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
      delete *it2;
  }
  m_joysticks.clear();
}

bool CJoystickManager::PerformJoystickScan(std::vector<CJoystick*>& joysticks)
{
  CLockObject lock(m_joystickMutex);

  bool bReturn(true);

  std::vector<CJoystick*> scanResults;
  for (std::vector<CJoystickInterface*>::iterator it = m_interfaces.begin(); it != m_interfaces.end(); ++it)
  {
    bReturn &= (*it)->ScanForJoysticks(scanResults);
    joysticks.insert(joysticks.end(), scanResults.begin(), scanResults.end()); // TODO
    m_joysticks[*it] = scanResults; // TODO: Manage joysticks better
  }

  return bReturn;
}

CJoystick* CJoystickManager::GetJoystick(unsigned int index) const
{
  CLockObject lock(m_joystickMutex);

  for (JoystickMap::const_iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
  {
    for (std::vector<CJoystick*>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
    {
      if ((*it2)->RequestedPlayer() == index) // TODO: Use index instead of requested player number
        return *it2;
    }
  }

  return NULL;
}

/*!
* @brief Get all events that have occurred since the last call to GetEvents()
*/
bool CJoystickManager::GetEvents(std::vector<ADDON::PeripheralEvent>& events)
{
  CLockObject lock(m_joystickMutex);

  bool bResult(true);

  for (JoystickMap::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
  {
    for (std::vector<CJoystick*>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
      bResult &= (*it2)->GetEvents(events);
  }

  return bResult;
}
