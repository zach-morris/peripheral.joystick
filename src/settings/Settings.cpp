/*
 *      Copyright (C) 2015 Garrett Brown
 *      Copyright (C) 2015 Team XBMC
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

#include "Settings.h"
#include "log/Log.h"

using namespace JOYSTICK;

#define SETTING_DEADZONE  "deadzone"

CSettings::CSettings(void)
  : m_deadzone(0.0f)
{
}

CSettings& CSettings::Get(void)
{
  static CSettings _instance;
  return _instance;
}

void CSettings::SetSetting(const std::string& strName, const void* value)
{
  if (strName == SETTING_DEADZONE)
  {
    m_deadzone = *static_cast<const float*>(value);
    dsyslog("Setting \"%s\" set to %f", SETTING_DEADZONE, m_deadzone);
  }
}
