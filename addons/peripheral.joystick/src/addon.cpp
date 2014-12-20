/*
 *      Copyright (C) 2014 Garrett Brown
 *      Copyright (C) 2014 Team XBMC
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

#define PERIPHERAL_ADDON_JOYSTICKS

#include "api/Joystick.h"
#include "api/JoystickManager.h"
#include "log/Log.h"
#include "log/LogAddon.h"
#include "utils/CommonMacros.h"

#include "libXBMC_addon.h"
#include "libXBMC_peripheral.h"
#include "xbmc_addon_dll.h"
#include "xbmc_peripheral_dll.h"
#include "xbmc_peripheral_utils.hpp"

#include <string>
#include <vector>

using namespace ADDON;
using namespace JOYSTICK;

extern "C"
{

CHelper_libXBMC_addon*      FRONTEND;
CHelper_libXBMC_peripheral* PERIPHERAL;
CJoystickManager*           JOYSTICKS;

ADDON_STATUS ADDON_Create(void* callbacks, void* props)
{
  try
  {
    if (!callbacks || !props)
      throw ADDON_STATUS_UNKNOWN;

    PERIPHERAL_PROPERTIES* peripheralProps = static_cast<PERIPHERAL_PROPERTIES*>(props);

    FRONTEND = new CHelper_libXBMC_addon;
    if (!FRONTEND || !FRONTEND->RegisterMe(callbacks))
      throw ADDON_STATUS_PERMANENT_FAILURE;

    PERIPHERAL = new CHelper_libXBMC_peripheral;
    if (!PERIPHERAL || !PERIPHERAL->RegisterMe(callbacks))
      throw ADDON_STATUS_PERMANENT_FAILURE;
  }
  catch (const ADDON_STATUS& status)
  {
    SAFE_DELETE(FRONTEND);
    SAFE_DELETE(PERIPHERAL);
    return status;
  }

  CLog::Get().SetPipe(new CLogAddon(FRONTEND));

  return ADDON_STATUS_OK;
}

void ADDON_Stop()
{
}

void ADDON_Destroy()
{
  CLog::Get().SetType(SYS_LOG_TYPE_CONSOLE);

  SAFE_DELETE(FRONTEND);
  SAFE_DELETE(PERIPHERAL);
}

ADDON_STATUS ADDON_GetStatus()
{
  return FRONTEND && PERIPHERAL ? ADDON_STATUS_OK : ADDON_STATUS_UNKNOWN;
}

bool ADDON_HasSettings()
{
  return false;
}

unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
{
  return 0;
}

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  return ADDON_STATUS_OK;
}

void ADDON_FreeSettings()
{
}

void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
{
}

const char* GetPeripheralAPIVersion(void)
{
  return PERIPHERAL_API_VERSION;
}

const char* GetMinimumPeripheralAPIVersion(void)
{
  return PERIPHERAL_MIN_API_VERSION;
}

PERIPHERAL_ERROR GetAddonCapabilities(PERIPHERAL_CAPABILITIES *pCapabilities)
{
  if (!pCapabilities)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  pCapabilities->provides_joysticks = true;

  return PERIPHERAL_NO_ERROR;
}

PERIPHERAL_ERROR PerformDeviceScan(unsigned int* peripheral_count, PERIPHERAL_SCAN_RESULT** scan_results)
{
  if (!peripheral_count || !scan_results)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  if (!JOYSTICKS)
    return PERIPHERAL_ERROR_FAILED;

  std::vector<PeripheralScanResult> results;
  std::vector<CJoystick*> joysticks;
  if (JOYSTICKS->PerformJoystickScan(joysticks))
  {
    for (std::vector<CJoystick*>::const_iterator it = joysticks.begin(); it != joysticks.end(); ++it)
    {
      PeripheralScanResult result;
      result.SetType(PERIPHERAL_TYPE_JOYSTICK);
      result.SetIndex((*it)->RequestedPlayer()); // TODO: Joystick index
      result.SetName((*it)->Name());
      result.SetVendorID(0); // TODO
      result.SetProductID(0); // TODO
      results.push_back(result);
    }
    *peripheral_count = results.size();
    PeripheralScanResult::ToStructs(results, scan_results);
    return PERIPHERAL_NO_ERROR;
  }

  return PERIPHERAL_ERROR_FAILED;
}

void FreeScanResults(unsigned int peripheral_count, PERIPHERAL_SCAN_RESULT* scan_results)
{
  PeripheralScanResult::FreeStructs(peripheral_count, scan_results);
}

PERIPHERAL_ERROR GetJoystickInfo(unsigned int index, JOYSTICK_INFO* info)
{
  if (!info)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  CJoystick* joystick = CJoystickManager::Get().GetJoystick(index);
  if (!joystick)
    return PERIPHERAL_ERROR_FAILED;

  joystick->ToStruct(*info);

  return PERIPHERAL_NO_ERROR;
}

void FreeJoystickInfo(JOYSTICK_INFO* info)
{
  if (!info)
    return;

  CJoystick::FreeStruct(*info);
}

PERIPHERAL_ERROR GetEvents(unsigned int* event_count, PERIPHERAL_EVENT** events)
{
  if (!event_count || !events)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  if (!JOYSTICKS)
    return PERIPHERAL_ERROR_FAILED;

  std::vector<PeripheralEvent> peripheralEvents;
  if (JOYSTICKS->GetEvents(peripheralEvents))
  {
    *event_count = peripheralEvents.size();
    PeripheralEvent::ToStructs(peripheralEvents, events);
    return PERIPHERAL_NO_ERROR;
  }

  return PERIPHERAL_ERROR_FAILED;
}

void FreeEvents(unsigned int event_count, PERIPHERAL_EVENT* events)
{
  PeripheralEvent::FreeStructs(event_count, events);
}

PERIPHERAL_ERROR GetButtonMap(unsigned int index, JOYSTICK_BUTTON_MAP* button_map)
{
  return PERIPHERAL_ERROR_NOT_IMPLEMENTED;
}

PERIPHERAL_ERROR FreeButtonMap(JOYSTICK_BUTTON_MAP* button_map)
{
  return PERIPHERAL_ERROR_NOT_IMPLEMENTED;
}

PERIPHERAL_ERROR UpdateButtonMap(unsigned int index, JOYSTICK_BUTTON_MAP_PAIR* key_value_pair)
{
  return PERIPHERAL_ERROR_NOT_IMPLEMENTED;
}

} // extern "C"
