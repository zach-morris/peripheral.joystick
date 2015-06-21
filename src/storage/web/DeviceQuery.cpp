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

#include "DeviceQuery.h"
#include "ButtonMapQuery.h"
#include "JoystickDefinitions.h"

using namespace JOYSTICK;

void CDeviceQuery::GetQueryString(std::stringstream& ss,
                                  const std::string& strControllerId /* = "" */) const
{
  ss << BUTTONMAP_XML_ATTR_DEVICE_NAME << "=" << m_strName;
  ss << "&" << BUTTONMAP_XML_ATTR_DEVICE_PROVIDER << "=" << m_strProvider;

  if (m_vid != 0 && m_pid != 0)
  {
    ss << "&" << BUTTONMAP_XML_ATTR_DEVICE_VID << "=" << m_vid;
    ss << "&" << BUTTONMAP_XML_ATTR_DEVICE_PID << "=" << m_pid;
  }

  if (m_buttonCount != 0)
    ss << "&" << BUTTONMAP_XML_ATTR_DEVICE_BUTTONCOUNT << "=" << m_buttonCount;

  if (m_hatCount != 0)
    ss << "&" << BUTTONMAP_XML_ATTR_DEVICE_HATCOUNT << "=" << m_hatCount;

  if (m_axisCount != 0)
    ss << "&" << BUTTONMAP_XML_ATTR_DEVICE_AXISCOUNT << "=" << m_axisCount;

  if (!strControllerId.empty())
  {
    ButtonMaps::const_iterator it = m_buttonMaps.find(strControllerId);
    if (it != m_buttonMaps.end())
    {
      ss << "&" << BUTTONMAP_XML_ELEM_CONTROLLER << "=" << strControllerId;
      CButtonMapQuery buttonMap(it->second);
      buttonMap.GetQueryString(ss);
    }
  }
}
