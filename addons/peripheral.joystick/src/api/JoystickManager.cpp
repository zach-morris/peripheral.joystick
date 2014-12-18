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
  #include "directinput/JoystickDirectInput.h"
#endif
#if defined(HAVE_XINPUT)
  #include "xinput/JoystickXInput.h"
#endif
#if defined(HAVE_LINUX_JOYSTICK)
  #include "linux/JoystickLinux.h"
#endif
#if defined(HAVE_SDL)
  #include "sdl/JoystickSDL.h"
#endif

using namespace ADDON;
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
  m_joysticks.push_back(new CJoystickDirectInput);
#endif
#if defined(HAVE_XINPUT)
  m_joysticks.push_back(new CJoystickXInput);
#endif
#if defined(HAVE_LINUX_JOYSTICK)
  m_joysticks.push_back(new CJoystickLinux);
#endif
#if defined(HAVE_SDL)
  m_joysticks.push_back(new CJoystickSDL);
#endif

  for (std::vector<IJoystick*>::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
    (*it)->Initialize();

  return !m_joysticks.empty();
}

void CJoystickManager::Deinitialize(void)
{
  CLockObject lock(m_joystickMutex);

  for (std::vector<IJoystick*>::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
    delete *it;

  m_joysticks.clear();
}

PERIPHERAL_ERROR CJoystickManager::PerformJoystickScan(std::vector<JoystickConfiguration>& joysticks)
{
  CLockObject lock(m_joystickMutex);

  for (std::vector<IJoystick*>::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
  {
    PERIPHERAL_ERROR result = (*it)->PerformJoystickScan(joysticks);
    if (result != PERIPHERAL_NO_ERROR)
      return result;
  }
  
  if (joysticks.empty())
    return PERIPHERAL_ERROR_NOT_CONNECTED;

  return PERIPHERAL_NO_ERROR;
}

bool CJoystickManager::GetEvents(EventMap& events)
{
  CLockObject lock(m_joystickMutex);

  bool bResult(true);

  for (std::vector<IJoystick*>::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
    bResult &= (*it)->GetEvents(events);

  return bResult;
}
