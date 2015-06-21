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
#pragma once

#include "UserID.h"

#include <string>

namespace JOYSTICK
{
  class CDevice;
  class CDeviceQuery;
  class CDeviceXml;

  class CButtonMapAPI
  {
  public:
    CButtonMapAPI(void) { }

    bool Initialize(const std::string& strUserPath);

    void Deinitialize(void) { }

    bool RequestButtonMap(const CDevice& device, CDeviceXml& result);

    bool UpdateControllerProfile(const CDeviceQuery& device, const std::string& strControllerId);

  private:
    enum API_ACTION
    {
      API_ACTION_GET,
      API_ACTION_PUT,
    };

    std::string BuildURL(API_ACTION action, const CDevice& device, const std::string strControllerId = "");

    static bool GetURL(const std::string& strUrl, std::string& strResponse);
    static const char* GetAction(API_ACTION action);

    CUserID m_userId;
  };
}
