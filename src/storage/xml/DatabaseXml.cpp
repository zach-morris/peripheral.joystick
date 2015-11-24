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

#include "DatabaseXml.h"
#include "ButtonMapRecordXml.h"
#include "DriverRecordXml.h"
#include "JoystickDefinitions.h"
#include "log/Log.h"

#include "kodi/libXBMC_addon.h"
#include "tinyxml.h"

#include <algorithm>

using namespace JOYSTICK;
using namespace PLATFORM;

#define USER_BUTTONMAP_XML   "/buttonmap.xml"
#define ADDON_BUTTONMAP_XML  "/resources/buttonmap.xml"

CDatabaseXml::CDatabaseXml(const std::string& strAddonPath, bool bReadOnly)
  : m_strPath(strAddonPath),
    m_bReadOnly(bReadOnly),
    m_bLoadAttempted(false),
    m_bLoaded(false)
{
  if (bReadOnly)
    m_strPath += ADDON_BUTTONMAP_XML;
  else
    m_strPath += USER_BUTTONMAP_XML;
}

bool CDatabaseXml::GetFeatures(const CDriverRecord& driverInfo, const std::string& controllerId,
                               FeatureVector& features)
{
  CLockObject lock(m_mutex);

  if (!Load())
    return false;

  return CDatabase::GetFeatures(driverInfo, controllerId, features);
}

bool CDatabaseXml::MapFeature(const CDriverRecord& driverInfo, const std::string& controllerId,
                              const ADDON::JoystickFeature* feature)
{
  CLockObject lock(m_mutex);

  if (m_bReadOnly)
    return false;

  if (CDatabase::MapFeature(driverInfo, controllerId, feature))
  {
    // Loading might have failed because button map didn't exist. The database
    // is no longer empty, so consider it loaded.
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

  dsyslog("Saving buttonmap: %s", m_strPath.c_str());

  return xmlFile.SaveFile(m_strPath);
}

bool CDatabaseXml::Serialize(TiXmlElement* pElement) const
{
  if (pElement == NULL)
    return false;

  for (Records::const_iterator it = m_records.begin(); it != m_records.end(); ++it)
  {
    const CDriverRecord& driverRecord = it->first;
    const ButtonMaps& buttonMaps = it->second;

    if (buttonMaps.empty())
      continue;

    TiXmlElement deviceElement(BUTTONMAP_XML_ELEM_DEVICE);
    TiXmlNode* deviceNode = pElement->InsertEndChild(deviceElement);
    if (deviceNode == NULL)
      return false;

    TiXmlElement* deviceElem = deviceNode->ToElement();
    if (deviceElem == NULL)
      return false;

    CDriverRecordXml::Serialize(driverRecord, deviceElem);

    for (ButtonMaps::const_iterator it = buttonMaps.begin(); it != buttonMaps.end(); ++it)
    {
      const ControllerID& controllerId = it->first;
      const CButtonMapRecord& buttonMap = it->second;

      if (buttonMap.IsEmpty())
        continue;

      TiXmlElement profileElement(BUTTONMAP_XML_ELEM_CONTROLLER);
      TiXmlNode* profileNode = deviceElem->InsertEndChild(profileElement);
      if (profileNode == NULL)
        continue;

      TiXmlElement* profileElem = profileNode->ToElement();
      if (profileElem == NULL)
        continue;

      profileElem->SetAttribute(BUTTONMAP_XML_ATTR_CONTROLLER_ID, controllerId);

      CButtonMapRecordXml::Serialize(buttonMap, profileElem);
    }
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
    CDriverRecord driverRecord;
    if (!CDriverRecordXml::Deserialize(pDevice, driverRecord))
      return false;

    if (!driverRecord.IsValid())
    {
      esyslog("<%s> tag with name=\"%s\" is invalid", BUTTONMAP_XML_ELEM_DEVICE, driverRecord.Properties().Name().c_str());
      return false;
    }

    const TiXmlElement* pController = pDevice->FirstChildElement(BUTTONMAP_XML_ELEM_CONTROLLER);

    if (!pController)
    {
      esyslog("Device \"%s\": can't find <%s> tag", driverRecord.Properties().Name().c_str(), BUTTONMAP_XML_ELEM_CONTROLLER);
      return false;
    }

    ButtonMaps& buttonMaps = m_records[driverRecord];

    // For logging purposes
    unsigned int totalFeatureCount = 0;

    while (pController)
    {
      const char* id = pController->Attribute(BUTTONMAP_XML_ATTR_CONTROLLER_ID);
      if (!id)
      {
        esyslog("Device \"%s\": <%s> tag has no attribute \"%s\"", driverRecord.Properties().Name().c_str(),
                BUTTONMAP_XML_ELEM_CONTROLLER, BUTTONMAP_XML_ATTR_CONTROLLER_ID);
        return false;
      }

      CButtonMapRecord buttonMap(driverRecord.Properties(), id);
      if (!CButtonMapRecordXml::Deserialize(pController, buttonMap))
        return false;

      if (!buttonMap.IsEmpty())
      {
        totalFeatureCount += buttonMap.FeatureCount();
        buttonMaps[id] = std::move(buttonMap);
      }

      pController = pController->NextSiblingElement(BUTTONMAP_XML_ELEM_CONTROLLER);
    }

    if (!buttonMaps.empty())
      dsyslog("Button map: loaded device \"%s\" with %u controller profiles and %u total features", driverRecord.Properties().Name().c_str(), buttonMaps.size(), totalFeatureCount);

    pDevice = pDevice->NextSiblingElement(BUTTONMAP_XML_ELEM_DEVICE);
  }

  return true;
}
