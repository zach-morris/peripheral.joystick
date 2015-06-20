/*
 *      Copyright (C) 2014-2015 Garrett Brown
 *      Copyright (C) 2014-2015 Team XBMC
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

#include "DatabaseXml.h"
#include "DeviceXml.h"
#include "JoystickDefinitions.h"
#include "log/Log.h"

#include "kodi/libXBMC_addon.h"
#include "tinyxml.h"

#include <algorithm>

using namespace JOYSTICK;
using namespace PLATFORM;

CDatabaseXml::CDatabaseXml(const std::string& strXmlPath, bool bReadOnly)
  : m_strPath(strXmlPath),
    m_bReadOnly(bReadOnly),
    m_bLoadAttempted(false),
    m_bLoaded(false)
{
}

bool CDatabaseXml::GetFeatures(const CDevice& needle, const std::string& strControllerId,
                               std::vector<ADDON::JoystickFeature*>& features)
{
  CLockObject lock(m_mutex);

  if (!Load())
    return false;

  return CDatabase::GetFeatures(needle, strControllerId, features);
}

bool CDatabaseXml::MapFeature(const CDevice& needle, const std::string& strControllerId,
                              const ADDON::JoystickFeature* feature)
{
  CLockObject lock(m_mutex);

  if (m_bReadOnly)
    return false;

  if (CDatabase::MapFeature(needle, strControllerId, feature))
  {
    // Loading might have failed because button map didn't exist. The database
    // is no longer empty, so consider it loaded.
    m_bLoaded = true;

    Save();

    return true;
  }

  return false;
}

bool CDatabaseXml::MergeDevice(const CDevice& device)
{
  CLockObject lock(m_mutex);

  if (m_bReadOnly)
    return false;

  if (CDatabase::MergeDevice(device))
  {
    m_bLoaded = true;

    Save();

    return true;
  }

  return false;
}

bool CDatabaseXml::Load(void)
{
  if (m_bLoadAttempted)
    return m_bLoaded;

  m_bLoadAttempted = true;

  dsyslog("Loading buttonmap: %s", m_strPath.c_str());

  TiXmlDocument xmlFile;
  if (!xmlFile.LoadFile(m_strPath))
    return false;

  TiXmlElement* pRootElement = xmlFile.RootElement();
  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueStr() != BUTTONMAP_XML_ROOT)
  {
    esyslog("Can't find root <%s> tag", BUTTONMAP_XML_ROOT);
    return false;
  }

  if (!Deserialize(pRootElement))
    return false;

  m_bLoaded = true;

  return true;
}

bool CDatabaseXml::Save(void) const
{
  if (m_bReadOnly)
    return false;

  TiXmlDocument xmlFile;

  TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
  xmlFile.LinkEndChild(decl);

  TiXmlElement rootElement(BUTTONMAP_XML_ROOT);
  TiXmlNode* root = xmlFile.InsertEndChild(rootElement);
  if (root == NULL)
    return false;

  TiXmlElement devices(BUTTONMAP_XML_ELEM_DEVICES);
  TiXmlNode* pDevices = root->InsertEndChild(devices);
  if (pDevices == NULL)
    return false;

  TiXmlElement* devicesElem = pDevices->ToElement();
  if (!devicesElem)
    return false;

  if (!Serialize(devicesElem))
    return false;

  return xmlFile.SaveFile(m_strPath);
}

bool CDatabaseXml::Serialize(TiXmlElement* pElement) const
{
  if (pElement == NULL)
    return false;

  for (std::vector<CDevice>::const_iterator it = m_devices.begin(); it != m_devices.end(); ++it)
  {
    CDeviceXml device(*it);

    TiXmlElement deviceElement(BUTTONMAP_XML_ELEM_DEVICE);
    TiXmlNode* deviceNode = pElement->InsertEndChild(deviceElement);
    if (deviceNode == NULL)
      return false;

    TiXmlElement* deviceElem = deviceNode->ToElement();
    if (deviceElem == NULL)
      return false;

    device.Serialize(deviceElem);
  }

  return true;
}

bool CDatabaseXml::Deserialize(const TiXmlElement* pElement)
{
  if (!pElement)
    return false;

  const TiXmlElement* pDevices = pElement->FirstChildElement(BUTTONMAP_XML_ELEM_DEVICES);

  if (!pDevices)
  {
    esyslog("Can't find <%s> tag", BUTTONMAP_XML_ELEM_DEVICES);
    return false;
  }

  const TiXmlNode* pDevicesNode = pDevices->ToElement();
  if (!pDevicesNode)
    return false;

  const TiXmlElement* pDevice = pDevicesNode->FirstChildElement(BUTTONMAP_XML_ELEM_DEVICE);

  if (!pDevice)
  {
    esyslog("Can't find <%s> tag", BUTTONMAP_XML_ELEM_DEVICE);
    return false;
  }

  while (pDevice)
  {
    CDeviceXml device;
    if (!device.Deserialize(pDevice))
      return false;

    if (!device.IsValid())
    {
      esyslog("<%s> tag with name=\"%s\" is invalid", BUTTONMAP_XML_ELEM_DEVICE, device.Name().c_str());
      return false;
    }

    m_devices.push_back(device);

    pDevice = pDevice->NextSiblingElement(BUTTONMAP_XML_ELEM_DEVICE);
  }

  return true;
}
