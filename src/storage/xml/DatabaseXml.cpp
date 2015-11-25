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
#include "filesystem/DirectoryUtils.h"
#include "storage/ButtonMapDefinitions.h"
#include "log/Log.h"

#include "kodi/libXBMC_addon.h"
#include "tinyxml.h"

#include <algorithm>
#include <set>

using namespace JOYSTICK;
using namespace PLATFORM;

// Subfolder for XML data
#define RESOURCES_XML_FOLDER  "xml"

// XML file to store devices
#define DEVICES_XML           "devices.xml"

// Subfolder for button map data
#define BUTTON_MAPS_FOLDER    "buttonmaps"

CDatabaseXml::CDatabaseXml(const std::string& strBasePath, bool bReadOnly)
 :  m_bReadOnly(bReadOnly),
    m_bLoadAttempted(false),
    m_bLoaded(false)
{
  std::string strXmlPath = strBasePath + "/" RESOURCES_XML_FOLDER;

  // Ensure directory exists
  if (!bReadOnly && !CDirectoryUtils::Exists(strXmlPath))
    CDirectoryUtils::Create(strXmlPath);

  m_strDevicesXmlPath = strXmlPath + "/" DEVICES_XML;
  m_strDataPath = strXmlPath + "/" BUTTON_MAPS_FOLDER;

  if (!bReadOnly && !CDirectoryUtils::Exists(m_strDataPath))
    CDirectoryUtils::Create(m_strDataPath);
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

  dsyslog("Loading devices: %s", m_strDevicesXmlPath.c_str());

  TiXmlDocument xmlFile;
  if (!xmlFile.LoadFile(m_strDevicesXmlPath))
  {
    esyslog("Error opening file: %s", xmlFile.ErrorDesc());
    return false;
  }

  TiXmlElement* pRootElement = xmlFile.RootElement();
  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueStr() != DEVICES_XML_ROOT)
  {
    esyslog("Can't find root <%s> tag", DEVICES_XML_ROOT);
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

  TiXmlElement rootElement(DEVICES_XML_ROOT);
  TiXmlNode* root = xmlFile.InsertEndChild(rootElement);
  if (root == NULL)
    return false;

  TiXmlElement* devicesElem = root->ToElement();
  if (!devicesElem)
    return false;

  if (!Serialize(devicesElem))
    return false;

  dsyslog("Saving devices: %s", m_strDevicesXmlPath.c_str());

  return xmlFile.SaveFile(m_strDevicesXmlPath);
}

bool CDatabaseXml::Serialize(TiXmlElement* pElement) const
{
  if (pElement == NULL)
    return false;

  // Keep track of which providers we've seen
  std::set<std::string> seenProviders;

  for (Records::const_iterator it = m_records.begin(); it != m_records.end(); ++it)
  {
    const CDriverRecord& driverRecord = it->first;
    const ButtonMaps& buttonMaps = it->second;

    if (buttonMaps.empty())
      continue;

    TiXmlElement deviceElement(DEVICES_XML_ELEM_DEVICE);
    TiXmlNode* deviceNode = pElement->InsertEndChild(deviceElement);
    if (deviceNode == NULL)
      return false;

    TiXmlElement* deviceElem = deviceNode->ToElement();
    if (deviceElem == NULL)
      return false;

    CDriverRecordXml::Serialize(driverRecord, deviceElem);

    const std::string& strProvider = driverRecord.Properties().Provider();

    std::string strProviderDir = m_strDataPath + "/" + strProvider;

    // Check if the provider has been seen before
    if (seenProviders.find(strProvider) == seenProviders.end())
    {
      // First time we see the provider, make sure the folder exists
      if (!CDirectoryUtils::Exists(strProviderDir))
        CDirectoryUtils::Create(strProviderDir);

      seenProviders.insert(strProvider);
    }

    std::string strFileName = driverRecord.RootFileName() + ".xml";
    deviceElem->SetAttribute(BUTTONMAP_XML_ATTR_DATA_PATH, strFileName);

    std::string strButtonMapPath = strProviderDir + "/" + strFileName;
    if (!SaveButtonMaps(driverRecord, strButtonMapPath))
      return false;
  }

  return true;
}

bool CDatabaseXml::SaveButtonMaps(const CDriverRecord& driverRecord, const std::string& strPath) const
{
  TiXmlDocument xmlFile;

  TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
  xmlFile.LinkEndChild(decl);

  TiXmlElement rootElement(BUTTONMAP_XML_ROOT);
  TiXmlNode* root = xmlFile.InsertEndChild(rootElement);
  if (root == NULL)
    return false;

  TiXmlElement* pElem = root->ToElement();
  if (!pElem)
    return false;

  TiXmlElement deviceElement(DEVICES_XML_ELEM_DEVICE);
  TiXmlNode* deviceNode = pElem->InsertEndChild(deviceElement);
  if (deviceNode == NULL)
    return false;

  TiXmlElement* deviceElem = deviceNode->ToElement();
  if (deviceElem == NULL)
    return false;

  CDriverRecordXml::Serialize(driverRecord, deviceElem);

  if (!SerializeButtonMaps(driverRecord, deviceElem))
    return false;

  return xmlFile.SaveFile(strPath);
}

bool CDatabaseXml::SerializeButtonMaps(const CDriverRecord& driverRecord, TiXmlElement* pElement) const
{
  Records::const_iterator itRecord = m_records.find(driverRecord);
  if (itRecord == m_records.end())
    return false;

  const ButtonMaps& buttonMaps = itRecord->second;
  for (ButtonMaps::const_iterator it = buttonMaps.begin(); it != buttonMaps.end(); ++it)
  {
    const ControllerID& controllerId = it->first;
    const CButtonMapRecord& buttonMap = it->second;

    if (buttonMap.IsEmpty())
      continue;

    TiXmlElement profileElement(BUTTONMAP_XML_ELEM_CONTROLLER);
    TiXmlNode* profileNode = pElement->InsertEndChild(profileElement);
    if (profileNode == NULL)
      continue;

    TiXmlElement* profileElem = profileNode->ToElement();
    if (profileElem == NULL)
      continue;

    profileElem->SetAttribute(BUTTONMAP_XML_ATTR_CONTROLLER_ID, controllerId);

    CButtonMapRecordXml::Serialize(buttonMap, profileElem);
  }
  return true;
}

bool CDatabaseXml::Deserialize(const TiXmlElement* pElement)
{
  if (!pElement)
    return false;

  const TiXmlElement* pDevice = pElement->FirstChildElement(DEVICES_XML_ELEM_DEVICE);

  if (!pDevice)
  {
    esyslog("Can't find <%s> tag", DEVICES_XML_ELEM_DEVICE);
    return false;
  }

  while (pDevice)
  {
    CDriverRecord driverRecord;
    if (!CDriverRecordXml::Deserialize(pDevice, driverRecord))
      return false;

    if (!driverRecord.IsValid())
    {
      esyslog("<%s> tag with name=\"%s\" is invalid", DEVICES_XML_ELEM_DEVICE, driverRecord.Properties().Name().c_str());
      return false;
    }

    const char* path = pDevice->Attribute(BUTTONMAP_XML_ATTR_DATA_PATH);
    if (!path)
    {
      esyslog("<%s> tag has no \"%s\" attribute", DEVICES_XML_ELEM_DEVICE, BUTTONMAP_XML_ATTR_DATA_PATH);
      return false;
    }
    std::string strDataPath = m_strDataPath + "/" + driverRecord.Properties().Provider() + "/" + path;

    if (!LoadButtonMaps(strDataPath, driverRecord))
      return false;
  }

  return true;
}

bool CDatabaseXml::LoadButtonMaps(const std::string& strPath, const CDriverRecord& driverRecord)
{
  ButtonMaps& buttonMaps = m_records[driverRecord];

  TiXmlDocument xmlFile;
  if (!xmlFile.LoadFile(strPath))
  {
    esyslog("Error opening %s: %s", strPath.c_str(), xmlFile.ErrorDesc());
    return false;
  }

  TiXmlElement* pRootElement = xmlFile.RootElement();
  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueStr() != BUTTONMAP_XML_ROOT)
  {
    esyslog("Can't find root <%s> tag", BUTTONMAP_XML_ROOT);
    return false;
  }

  const TiXmlElement* pDevice = pRootElement->FirstChildElement(DEVICES_XML_ELEM_DEVICE);

  if (!pDevice)
  {
    esyslog("Device \"%s\": can't find <%s> tag", driverRecord.Properties().Name().c_str(), DEVICES_XML_ELEM_DEVICE);
    return false;
  }

  const TiXmlElement* pController = pDevice->FirstChildElement(BUTTONMAP_XML_ELEM_CONTROLLER);

  if (!pController)
  {
    esyslog("Device \"%s\": can't find <%s> tag", driverRecord.Properties().Name().c_str(), BUTTONMAP_XML_ELEM_CONTROLLER);
    return false;
  }

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

    if (buttonMap.IsEmpty())
    {
      esyslog("Device \"%s\" has no features for controller %s", driverRecord.Properties().Name().c_str(), id);
    }
    else
    {
      totalFeatureCount += buttonMap.FeatureCount();
      buttonMaps[id] = std::move(buttonMap);
    }

    pController = pController->NextSiblingElement(BUTTONMAP_XML_ELEM_CONTROLLER);
  }

  dsyslog("Loaded device \"%s\" with %u controller profiles and %u total features", driverRecord.Properties().Name().c_str(), buttonMaps.size(), totalFeatureCount);

  return true;
}
