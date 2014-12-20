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
 */

#include "JoystickInterface.h"
#include "utils/CommonMacros.h"

using namespace JOYSTICK;

CJoystickInterface::CJoystickInterface(const char* strName)
 : m_strName(strName)
{
  ASSERT(m_strName);
}

bool CJoystickInterface::ScanForJoysticks(std::vector<CJoystick*>& results)
{
  bool bReturn(false);

  std::vector<CJoystick*> joysticks;
  if (PerformJoystickScan(joysticks))
  {
    bReturn = true;

    // TODO: Manage added/removed joysticks
    results.insert(results.end(), joysticks.begin(), joysticks.end());
  }

  return bReturn;
}
