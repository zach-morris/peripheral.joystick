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

#include "DatabaseWeb.h"
#include "JoystickDefinitions.h"
#include "log/Log.h"
#include "filesystem/FilesystemTypes.h"
#include "filesystem/FileUtils.h"
#include "filesystem/IFile.h"
#include "settings/Settings.h"
#include "storage/StorageManager.h"
#include "storage/web/DeviceQuery.h"
#include "storage/xml/DeviceXml.h"

#include "tinyxml.h"

#include <assert.h>
#include <algorithm>
#include <sstream>

using namespace JOYSTICK;
using namespace PLATFORM;

#define API_QUERY_ACTION     "action"
#define API_QUERY_USER_ID    "random"

// Amount of time to wait before updating button maps
#define UPDATE_DELAY_SEC     10

// Read at most this many bytes per API call to retrieve button maps
#define MAX_BUTTONMAP_BYTES  (10 * 1024 * 1024)  // 10 MB

// Read at most this many bytes per API call to update button map
#define MAX_RESPONSE_BYTES   256 // Size of buffer for each line being logged

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

bool CDatabaseWeb::ProcessRequest(const CDevice& needle)
{
  // Build URL
  std::stringstream ssUrl;
  ssUrl << CSettings::Get().ButtonMapAPI();
  ssUrl << "?" << API_QUERY_ACTION << "=" << GetAction(API_ACTION_GET);
  ssUrl << "&" << API_QUERY_USER_ID << "=" << m_strUserId;
  ssUrl << "&";
  CDeviceQuery(needle).GetQueryString(ssUrl);

  const std::string strUrl = ssUrl.str();

  FilePtr file = CFileUtils::OpenFile(strUrl);
  if (!file)
  {
    esyslog("No VFS file for %s", strUrl.c_str());

    // Don't try again
    Disable();

    return false;
  }

  /*
  const char* strResponse =
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
  */

  std::string strResponse;
  if (!file->ReadFile(strResponse, MAX_BUTTONMAP_BYTES))
  {
    esyslog("Failed to read %s", strUrl.c_str());
    return false;
  }

  dsyslog("Opening %s", strUrl.c_str());

  TiXmlDocument xmlFile;
  if (!xmlFile.Parse(strResponse.c_str()))
  {
    esyslog("Failed to parse xml response line %d: %s", xmlFile.ErrorRow(), xmlFile.ErrorDesc());
    return false;
  }

  TiXmlElement* pRootElement = xmlFile.RootElement();
  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueStr() != BUTTONMAP_XML_ELEM_DEVICE)
  {
    esyslog("Can't find root <%s> tag", BUTTONMAP_XML_ELEM_DEVICE);
    return false;
  }

  CDeviceXml device;
  if (!device.Deserialize(pRootElement))
    return false;

  if (!device.IsValid())
  {
    esyslog("<%s> tag with name=\"%s\" is invalid", BUTTONMAP_XML_ELEM_DEVICE, device.Name().c_str());
    return false;
  }

  CDatabase::MergeDevice(device);

  if (m_userXml->MergeDevice(device))
    m_manager->RefreshButtonMaps(device.Name());

  return true;
}

bool CDatabaseWeb::ProcessUpdate(const CDevice& needle, const std::string& strControllerId)
{
  CDeviceQuery device;

  {
    CLockObject lock(m_mutex);
    std::vector<CDevice>::const_iterator itDevice = std::find(m_devices.begin(), m_devices.end(), needle);
    if (itDevice == m_devices.end())
      return false;

    device = *itDevice;
  }

  // Build URL
  std::stringstream ssUrl;
  ssUrl << CSettings::Get().ButtonMapAPI();
  ssUrl << "?" << API_QUERY_ACTION << "=" << GetAction(API_ACTION_GET);
  ssUrl << "&" << API_QUERY_USER_ID << "=" << m_strUserId;
  ssUrl << "&";
  CDeviceQuery(device).GetQueryString(ssUrl, strControllerId);

  const std::string strUrl = ssUrl.str();

  FilePtr file = CFileUtils::OpenFile(strUrl.c_str());
  if (!file)
  {
    esyslog("No VFS file for %s", strUrl.c_str());

    // Don't try again
    Disable();

    return false;
  }

  std::string strResponse;
  if (!file->ReadFile(strResponse, MAX_RESPONSE_BYTES))
  {
    esyslog("Failed to read %s", strUrl.c_str());
    return false;
  }

  dsyslog("Opening %s", strUrl.c_str());

  dsyslog("Response: %s", strResponse.c_str());

  return true;
}

bool CDatabaseWeb::GetFeatures(const CDevice& needle, const std::string& strControllerId,
                               std::vector<ADDON::JoystickFeature*>& features)
{
  CLockObject lock(m_mutex);

  // Only attempt one request at a time
  if (m_requestQueue.empty())
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

    const UpdateJob updatePair(needle, strControllerId);
    if (std::find(m_updateQueue.begin(), m_updateQueue.end(), updatePair) == m_updateQueue.end())
    {
      m_updateQueue.push_back(updatePair);
      m_idleEvent.Signal();
      CreateThread(false);
    }
  }

  return false;
}

const char* CDatabaseWeb::GetAction(API_ACTION action)
{
  switch (action)
  {
    case API_ACTION_GET: return "get";
    case API_ACTION_PUT: return "put";
    default:
      break;
  }
  return "";
}
