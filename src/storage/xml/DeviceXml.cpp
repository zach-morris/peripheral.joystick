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

#include "DeviceXml.h"
#include "ButtonMapXml.h"
#include "JoystickDefinitions.h"
#include "log/Log.h"

#include "tinyxml.h"

#include <cstdlib>

using namespace JOYSTICK;

bool CDeviceXml::Serialize(TiXmlElement* pElement) const
{
  pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_NAME,     m_strName);
  pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_PROVIDER, m_strProvider);
  if (m_vid != 0 &&
      m_pid != 0)
  {
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_VID, m_vid);
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_PID, m_pid);
  }
  if (m_buttonCount != 0 &&
      m_hatCount    != 0 &&
      m_axisCount   != 0)
  {
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_BUTTONCOUNT, m_buttonCount);
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_HATCOUNT,    m_hatCount);
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_AXISCOUNT,   m_axisCount);
  }

  for (ButtonMaps::const_iterator it = m_buttonMaps.begin(); it != m_buttonMaps.end(); ++it)
  {
    const std::string& controllerId = it->first;
    CButtonMapXml buttons(it->second);

    TiXmlElement profileElement(BUTTONMAP_XML_ELEM_CONTROLLER);
    TiXmlNode* profileNode = pElement->InsertEndChild(profileElement);
    if (profileNode == NULL)
      continue;

    TiXmlElement* profileElem = profileNode->ToElement();
    if (profileElem == NULL)
      continue;

    profileElem->SetAttribute(BUTTONMAP_XML_ATTR_CONTROLLER_ID, controllerId);

    buttons.Serialize(profileElem);
  }

  return true;
}

bool CDeviceXml::Deserialize(const TiXmlElement* pElement)
{
  Reset();

  const char* name = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_NAME);
  if (!name)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELEM_DEVICE, BUTTONMAP_XML_ATTR_DEVICE_NAME);
    return false;
  }
  m_strName = name;

  const char* provider = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_PROVIDER);
  if (!provider)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELEM_DEVICE, BUTTONMAP_XML_ATTR_DEVICE_PROVIDER);
    return false;
  }
  m_strProvider = provider;

  const char* vid = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_VID);
  if (vid)
    m_vid = std::atoi(vid);

  const char* pid = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_PID);
  if (pid)
    m_pid = std::atoi(pid);

  const char* buttonCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_BUTTONCOUNT);
  if (buttonCount)
    m_buttonCount = std::atoi(buttonCount);

  const char* hatCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_HATCOUNT);
  if (hatCount)
    m_hatCount = std::atoi(hatCount);

  const char* axisCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_AXISCOUNT);
  if (axisCount)
    m_axisCount = std::atoi(axisCount);

  const TiXmlElement* pProfile = pElement->FirstChildElement(BUTTONMAP_XML_ELEM_CONTROLLER);

  if (!pProfile)
  {
    esyslog("Device \"%s\": can't find <%s> tag", m_strName.c_str(), BUTTONMAP_XML_ELEM_CONTROLLER);
    return false;
  }

  while (pProfile)
  {
    const char* id = pProfile->Attribute(BUTTONMAP_XML_ATTR_CONTROLLER_ID);
    if (!id)
    {
      esyslog("Device \"%s\": <%s> tag has no attribute \"%s\"", m_strName.c_str(),
              BUTTONMAP_XML_ELEM_CONTROLLER, BUTTONMAP_XML_ATTR_CONTROLLER_ID);
      return false;
    }

    CButtonMapXml buttons;
    if (!buttons.Deserialize(pProfile))
      return false;

    m_buttonMaps[id] = buttons;

    pProfile = pProfile->NextSiblingElement(BUTTONMAP_XML_ELEM_CONTROLLER);
  }

  if (!m_buttonMaps.empty())
    dsyslog("Button map: loaded device \"%s\" with %u controller profiles", m_strName.c_str(), m_buttonMaps.size());

  return true;
}
