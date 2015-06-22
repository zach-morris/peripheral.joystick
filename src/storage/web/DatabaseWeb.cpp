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
#include "settings/Settings.h"
#include "storage/StorageManager.h"
#include "storage/web/DeviceQuery.h"
#include "storage/xml/DeviceXml.h"

#include <assert.h>
#include <algorithm>

using namespace JOYSTICK;
using namespace PLATFORM;

// Amount of time to wait before updating button maps
#define UPDATE_DELAY_SEC        10

// Delay for this long when an API call fails
#define API_FAILURE_DELAY_MINS  30

CDatabaseWeb::CDatabaseWeb(CStorageManager* manager, CDatabase* userXml, const std::string& strUserPath)
  : m_manager(manager),
    m_userXml(userXml)
{
  assert(m_manager);
  assert(m_userXml);

  if (!m_api.Initialize(strUserPath))
    Disable();
}

void* CDatabaseWeb::Process(void)
{
  while (!IsStopped() && IsEnabled())
  {
    CDevice requestDevice;
    UpdateButtonMapJob updateJob;

    {
      CLockObject lock(m_mutex);
      if (m_requestQueue.empty() && m_updateQueue.empty())
        break;

      if (!m_requestQueue.empty())
        requestDevice = m_requestQueue[0];

      if (!m_updateQueue.empty() && m_updateTimeout.TimeLeft() == 0)
        updateJob = m_updateQueue[0];
    }

    if (requestDevice.IsValid())
    {
      if (!ProcessRequest(requestDevice))
      {
        // Delay until next API call is allowed
        const unsigned int delayMs = API_FAILURE_DELAY_MINS * 60 * 1000;
        Sleep(delayMs);
      }

      CLockObject lock(m_mutex);
      m_requestQueue.erase(m_requestQueue.begin());

      continue;
    }

    const CDevice&      updateJobDevice       = updateJob.first;
    const ControllerID& updateJobControllerId = updateJob.second;

    if (updateJobDevice.IsValid())
    {
      if (!ProcessUpdate(updateJobDevice, updateJobControllerId))
      {
        // Delay until next API call is allowed
        const unsigned int delayMs = API_FAILURE_DELAY_MINS * 60 * 1000;
        Sleep(delayMs);
      }

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
  CDeviceXml device;
  if (m_api.RequestButtonMap(needle, device))
  {
    CDatabase::MergeDevice(device);

    if (m_userXml->MergeDevice(device))
      m_manager->RefreshButtonMaps(device.Name());

    return true;
  }

  return false;
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

  return m_api.UpdateControllerProfile(device, strControllerId);
}

bool CDatabaseWeb::GetFeatures(const CDevice& needle, const std::string& strControllerId,
                               std::vector<ADDON::JoystickFeature*>& features)
{
  if (!CSettings::Get().UseButtonMapAPI())
    return false;

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
  if (!CSettings::Get().UseButtonMapAPI())
    return false;

  CLockObject lock(m_mutex);

  if (CDatabase::MapFeature(needle, strControllerId, feature))
  {
    m_updateTimeout.Init(UPDATE_DELAY_SEC * 1000);

    const UpdateButtonMapJob updatePair(needle, strControllerId);
    if (std::find(m_updateQueue.begin(), m_updateQueue.end(), updatePair) == m_updateQueue.end())
    {
      m_updateQueue.push_back(updatePair);
      m_idleEvent.Signal();
      CreateThread(false);
    }
  }

  return false;
}
