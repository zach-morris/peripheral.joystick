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

#include "DriverRecordXml.h"
#include "storage/schema/DriverRecord.h"
#include "JoystickDefinitions.h"
#include "log/Log.h"

#include "tinyxml.h"

#include <cstdlib>

using namespace JOYSTICK;

bool CDriverRecordXml::Serialize(const CDriverRecord& record, TiXmlElement* pElement)
{
  pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_NAME, record.Properties().Name());
  pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_PROVIDER, record.Properties().Provider());
  if (record.Properties().VendorID() != 0 &&
      record.Properties().ProductID() != 0)
  {
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_VID, record.Properties().VendorID());
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_PID, record.Properties().ProductID());
  }
  if (record.Properties().ButtonCount() != 0)
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_BUTTONCOUNT, record.Properties().ButtonCount());
  if (record.Properties().HatCount() != 0)
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_HATCOUNT, record.Properties().HatCount());
  if (record.Properties().AxisCount() != 0)
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_AXISCOUNT, record.Properties().AxisCount());
  return true;
}

bool CDriverRecordXml::Deserialize(const TiXmlElement* pElement, CDriverRecord& record)
{
  const char* name = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_NAME);
  if (!name)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ROOT, BUTTONMAP_XML_ATTR_DEVICE_NAME);
    return false;
  }
  record.Properties().SetName(name);

  const char* provider = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_PROVIDER);
  if (!provider)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ROOT, BUTTONMAP_XML_ATTR_DEVICE_PROVIDER);
    return false;
  }
  record.Properties().SetProvider(provider);

  const char* vid = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_VID);
  if (vid)
    record.Properties().SetVendorID(std::atoi(vid));

  const char* pid = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_PID);
  if (pid)
    record.Properties().SetProductID(std::atoi(pid));

  const char* buttonCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_BUTTONCOUNT);
  if (buttonCount)
    record.Properties().SetButtonCount(std::atoi(buttonCount));

  const char* hatCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_HATCOUNT);
  if (hatCount)
    record.Properties().SetHatCount(std::atoi(hatCount));

  const char* axisCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_AXISCOUNT);
  if (axisCount)
    record.Properties().SetAxisCount(std::atoi(axisCount));

  return true;
}
