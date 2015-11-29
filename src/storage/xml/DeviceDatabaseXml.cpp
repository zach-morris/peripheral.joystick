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

#include "DeviceDatabaseXml.h"
#include "DeviceXml.h"
#include "filesystem/FileUtils.h"
#include "storage/ButtonMapDefinitions.h"
#include "log/Log.h"

#include <tinyxml.h>

using namespace JOYSTICK;

// XML file to store devices
#define DEVICES_XML  "devices.xml"

CDeviceDatabaseXml::CDeviceDatabaseXml(const std::string& strBasePath, bool bReadOnly) :
  m_bReadOnly(bReadOnly),
  m_strBasePath(strBasePath)
{
  m_strDevicesXmlPath = m_strBasePath + "/" DEVICES_XML;
}

bool CDeviceDatabaseXml::GetDevice(const ADDON::Joystick& joystick, CDevice& record)
{
  CDevice retVal;

  // First check local cache
  if (!CDeviceDatabase::GetDevice(joystick, retVal))
  {
    CDevice needle(joystick);

    // See if the record is similar to one in drivers.xml
    dsyslog("Loading devices.xml: %s", m_strDevicesXmlPath.c_str());

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

    const TiXmlElement* pDevice = pRootElement->FirstChildElement(DEVICES_XML_ELEM_DEVICE);

    if (!pDevice)
    {
      esyslog("Can't find <%s> tag", DEVICES_XML_ELEM_DEVICE);
      return false;
    }

    while (pDevice)
    {
      CDevice driverRecord;
      if (!CDeviceXml::Deserialize(pDevice, driverRecord))
        return false;

      if (!driverRecord.IsValid())
      {
        esyslog("<%s> tag with name=\"%s\" is invalid", DEVICES_XML_ELEM_DEVICE, driverRecord.Name().c_str());
        return false;
      }

      if (driverRecord.SimilarTo(needle))
      {
        // Found a match, record result
        retVal = driverRecord;

        // If not read only, check to see if new information was observed
        if (!m_bReadOnly)
        {
          CDevice combinedRecord = driverRecord;
          combinedRecord.MergeProperties(needle);

          bool bNewInformation = (driverRecord != combinedRecord);

          if (bNewInformation)
          {
            // Update path to button map
            const char* path = pDevice->Attribute(BUTTONMAP_XML_ATTR_DATA_PATH);
            if (!path)
            {
              esyslog("<%s> tag has no \"%s\" attribute", DEVICES_XML_ELEM_DEVICE, BUTTONMAP_XML_ATTR_DATA_PATH);
              return false;
            }

            std::string strExistingPath = m_strBasePath + "/" + path;
            std::string strNewPath = combinedRecord.BuildPath(m_strBasePath, ".xml");

            // This should always be the case
            if (CFileUtils::Exists(strExistingPath) && !CFileUtils::Rename(strExistingPath, strNewPath))
              esyslog("Failed to rename button map: %s -> %s", strExistingPath.c_str(), strNewPath.c_str());

            retVal = combinedRecord;
          }
        }

        break;
      }

      const TiXmlElement* pDevice = pRootElement->FirstChildElement(DEVICES_XML_ELEM_DEVICE);
    }

    if (!retVal.IsValid())
      retVal = needle;

    m_driverRecords[needle] = retVal;

    /* TODO
    if (!m_bReadOnly)
      Save();
    */
  }

  record = retVal;
  return true;
}
