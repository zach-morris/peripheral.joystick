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

#include "ButtonMapAPI.h"
#include "storage/Database.h"

#include "platform/threads/mutex.h"
#include "platform/threads/threads.h"
#include "platform/util/timeutils.h"

#include <string>
#include <utility>
#include <vector>

namespace JOYSTICK
{
  class CStorageManager;

  class CDatabaseWeb : public CDatabase, protected PLATFORM::CThread
  {
  public:
    CDatabaseWeb(CStorageManager* manager, CDatabase* userXml, const std::string& strUserId);

    virtual ~CDatabaseWeb(void) { }

    virtual bool GetFeatures(const CDevice& needle, const std::string& strDeviceId,
                             std::vector<ADDON::JoystickFeature*>& features);

    virtual bool MapFeature(const CDevice& needle, const std::string& strDeviceId,
                            const ADDON::JoystickFeature* feature);

  protected:
    virtual void* Process(void);

  private:
    bool ProcessRequest(const CDevice& needle);
    bool ProcessUpdate(const CDevice& needle, const std::string& strControllerId);

    typedef std::string                      ControllerID;
    typedef std::pair<CDevice, ControllerID> UpdateButtonMapJob;

    CStorageManager* const          m_manager;
    CDatabase* const                m_userXml;

    CButtonMapAPI                   m_api;
    std::vector<CDevice>            m_requestQueue;
    std::vector<UpdateButtonMapJob> m_updateQueue;
    PLATFORM::CTimeout              m_updateTimeout;
    PLATFORM::CEvent                m_idleEvent;
    PLATFORM::CMutex                m_mutex;
  };
}
