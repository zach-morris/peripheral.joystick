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

#include "DatabaseWeb.h"
#include "JoystickDefinitions.h"
#include "log/Log.h"
#include "storage/StorageManager.h"
#include "storage/web/DeviceQuery.h"
#include "storage/xml/DeviceXml.h"

#include "tinyxml.h"

#include <assert.h>
#include <algorithm>
#include <sstream>

using namespace JOYSTICK;
using namespace PLATFORM;

// Amount of time to wait before updating button maps
#define UPDATE_DELAY_SEC  30

CDatabaseWeb::CDatabaseWeb(CStorageManager* manager, CDatabase* userXml, const std::string& strUserId)
  : m_manager(manager),
    m_userXml(userXml),
    m_strUserId(strUserId)
{
  assert(m_manager);
  assert(m_userXml);
}

void* CDatabaseWeb::Process(void)
{
  while (true)
  {
    CDevice request;
    UpdateJob update;

    {
      CLockObject lock(m_mutex);
      if (m_requestQueue.empty() && m_updateQueue.empty())
        break;

      if (!m_requestQueue.empty())
        request = m_requestQueue[0];

      if (!m_updateQueue.empty() && m_updateTimeout.TimeLeft() == 0)
        update = m_updateQueue[0];
    }

    if (request.IsValid())
    {
      ProcessRequest(request);
      CLockObject lock(m_mutex);
      m_requestQueue.erase(m_requestQueue.begin());
      if (!m_requestQueue.empty())
        continue;
    }

    if (update.first.IsValid())
    {
      ProcessUpdate(update.first, update.second);
      CLockObject lock(m_mutex);
      m_updateQueue.erase(m_updateQueue.begin());
    }

    uint32_t timeLeft = m_updateTimeout.TimeLeft();
    if (timeLeft > 0)
      m_idleEvent.Wait(timeLeft);
  }

  return NULL;
}

void CDatabaseWeb::ProcessRequest(const CDevice& needle)
{
  static const char* strXml =
    "<device name=\"Keyboard\" provider=\"application\">\n"
        "<controller id=\"game.controller.nes\">\n"
            "<feature name=\"a\" button=\"90\"/>\n"
            "<feature name=\"b\" button=\"88\"/>\n"
            "<feature name=\"down\" button=\"129\"/>\n"
            "<feature name=\"left\" button=\"130\"/>\n"
            "<feature name=\"right\" button=\"131\"/>\n"
            "<feature name=\"select\" button=\"92\"/>\n"
            "<feature name=\"start\" button=\"13\"/>\n"
            "<feature name=\"up\" button=\"128\"/>\n"
        "</controller>\n"
    "</device>\n";

  TiXmlDocument xmlFile;
  if (!xmlFile.Parse(strXml))
  {
    esyslog("Failed to parse xml response line %d: %s", xmlFile.ErrorRow(), xmlFile.ErrorDesc());
    return;
  }

  TiXmlElement* pRootElement = xmlFile.RootElement();
  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueStr() != BUTTONMAP_XML_ELEM_DEVICE)
  {
    esyslog("Can't find root <%s> tag", BUTTONMAP_XML_ELEM_DEVICE);
    return;
  }

  CDeviceXml device;
  if (!device.Deserialize(pRootElement))
    return;

  if (!device.IsValid())
  {
    esyslog("<%s> tag with name=\"%s\" is invalid", BUTTONMAP_XML_ELEM_DEVICE, device.Name().c_str());
    return;
  }

  CDatabase::MergeDevice(device);

  if (m_userXml->MergeDevice(device))
    m_manager->RefreshButtonMaps(device.Name());
}

void CDatabaseWeb::ProcessUpdate(const CDevice& needle, const std::string& strControllerId)
{
  std::vector<CDevice>::const_iterator itDevice = std::find(m_devices.begin(), m_devices.end(), needle);
  if (itDevice != m_devices.end())
  {
    CDeviceQuery device(*itDevice);

    std::stringstream ss;
    device.GetQueryString(ss, strControllerId);
    dsyslog("Opening URL: www.test.com/buttonmap?random=%s&%s", m_strUserId.c_str(),
            ss.str().c_str());
  }
}

bool CDatabaseWeb::GetFeatures(const CDevice& needle, const std::string& strControllerId,
                               std::vector<ADDON::JoystickFeature*>& features)
{
  CLockObject lock(m_mutex);

  if (std::find(m_requestQueue.begin(), m_requestQueue.end(), needle) == m_requestQueue.end())
  {
    m_requestQueue.push_back(needle);
    m_idleEvent.Signal();
    CreateThread(false);
  }

  return false;
}

bool CDatabaseWeb::MapFeature(const CDevice& needle, const std::string& strControllerId,
                              const ADDON::JoystickFeature* feature)
{
  CLockObject lock(m_mutex);

  if (CDatabase::MapFeature(needle, strControllerId, feature))
  {
    m_updateTimeout.Init(UPDATE_DELAY_SEC * 1000);

    UpdateJob updatePair(needle, strControllerId);
    if (std::find(m_updateQueue.begin(), m_updateQueue.end(), updatePair) == m_updateQueue.end())
    {
      m_updateQueue.push_back(updatePair);
      m_idleEvent.Signal();
      CreateThread(false);
    }
  }

  return false;
}
