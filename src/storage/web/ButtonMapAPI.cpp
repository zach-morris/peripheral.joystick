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

#include "ButtonMapAPI.h"
#include "DeviceQuery.h"
#include "filesystem/FileUtils.h"
#include "filesystem/IFile.h"
#include "JoystickDefinitions.h"
#include "log/Log.h"
#include "settings/Settings.h"
#include "storage/xml/DeviceXml.h"

#include "tinyxml.h"

using namespace JOYSTICK;

#define API_QUERY_ACTION        "action"
#define API_QUERY_USER_ID       "random"

// Read at most this many bytes per API call to retrieve button maps
#define MAX_BUTTONMAP_BYTES     (10 * 1024 * 1024)  // 10 MB

// Read at most this many bytes per API call to update button map
#define MAX_RESPONSE_BYTES      256 // Size of buffer for each line being logged

bool CButtonMapAPI::Initialize(const std::string& strUserPath)
{
  return m_userId.Load(strUserPath);
}

bool CButtonMapAPI::RequestButtonMap(const CDevice& device, CDeviceXml& result)
{
  std::string strUrl = BuildURL(API_ACTION_GET, device);

  /**** Response looks like:

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
  if (!GetURL(strUrl, strResponse))
    return false;

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

  if (!result.Deserialize(pRootElement))
    return false;

  if (!result.IsValid())
  {
    esyslog("<%s> tag with name=\"%s\" is invalid", BUTTONMAP_XML_ELEM_DEVICE, result.Name().c_str());
    return false;
  }

  return true;
}

bool CButtonMapAPI::UpdateControllerProfile(const CDeviceQuery& device, const std::string& strControllerId)
{
  std::string strUrl = BuildURL(API_ACTION_GET, device);

  std::string strResponse;
  if (!GetURL(strUrl, strResponse))
    return false;

  return true;
}

std::string CButtonMapAPI::BuildURL(API_ACTION action, const CDevice& device, const std::string strControllerId /* = "" */)
{
  std::string strUrl;

  if (device.IsValid() && m_userId.IsLoaded())
  {
    std::stringstream ssUrl;
    ssUrl << CSettings::Get().ButtonMapAPI();
    ssUrl << "?" << API_QUERY_ACTION << "=" << GetAction(action);
    ssUrl << "&" << API_QUERY_USER_ID << "=" << m_userId.GetID();
    ssUrl << "&";
    CDeviceQuery(device).GetQueryString(ssUrl, strControllerId);
    strUrl = ssUrl.str();
  }

  return strUrl;
}

bool CButtonMapAPI::GetURL(const std::string& strUrl, std::string& strResponse)
{
  if (strUrl.empty())
    return false;

  if (!CFileUtils::ExistsOnVFS(strUrl))
  {
    esyslog("No VFS file for %s", strUrl.c_str());
    //Disable(); // Don't try again
    return false;
  }

  FilePtr file = CFileUtils::OpenFile(strUrl);
  if (!file)
    return false;

  if (file->ReadFile(strResponse, MAX_BUTTONMAP_BYTES) <= 0)
  {
    esyslog("Failed to read %s", strUrl.c_str());
    return false;
  }

  dsyslog("Opened %s", strUrl.c_str());

  return true;
}

const char* CButtonMapAPI::GetAction(API_ACTION action)
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
