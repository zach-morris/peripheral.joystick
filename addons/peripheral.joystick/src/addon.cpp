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

#include "api/JoystickManager.h"
#include "log/Log.h"
#include "log/LogAddon.h"

#include "libXBMC_addon.h"
#include "libXBMC_peripheral.h"
#include "xbmc_addon_dll.h"
#include "xbmc_peripheral_dll.h"
#include "xbmc_peripheral_utils.hpp"

#include <string>
#include <vector>

using namespace ADDON;
using namespace JOYSTICK;

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)  do { delete x; x = NULL; } while (0)
#endif

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

  pCapabilities->bProvidesJoysticks = true;

  return PERIPHERAL_NO_ERROR;
}

PERIPHERAL_ERROR PerformJoystickScan(unsigned int* joystick_count, JOYSTICK_CONFIGURATION** joysticks)
{
  if (!joystick_count || !joysticks)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  if (!JOYSTICKS)
    return PERIPHERAL_ERROR_FAILED;

  *joystick_count = 0;
  *joysticks = NULL;

  std::vector<JoystickConfiguration> joystickConfigs;
  PERIPHERAL_ERROR retVal = JOYSTICKS->PerformJoystickScan(joystickConfigs);

  if (retVal == PERIPHERAL_NO_ERROR && !joystickConfigs.empty())
  {
    *joystick_count = joystickConfigs.size();
    *joysticks = new JOYSTICK_CONFIGURATION[joystickConfigs.size()];
    for (unsigned int i = 0; i < joystickConfigs.size(); i++)
      joystickConfigs[i].ToJoystickConfiguration(*joysticks[i]);
  }

  return retVal;
}

void FreeJoysticks(unsigned int joystick_count, JOYSTICK_CONFIGURATION* joysticks)
{
  if (joysticks)
  {
    for (unsigned int i = 0; i < joystick_count; i++)
      JoystickConfiguration::Free(joysticks[i]);
  }
  delete[] joysticks;
}

PERIPHERAL_ERROR RegisterButton(unsigned int joystick_index, JOYSTICK_MAP_BUTTON* button)
{
  if (!button)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  if (!JOYSTICKS)
    return PERIPHERAL_ERROR_FAILED;

  ButtonMap buttonMap(*button);

  return PERIPHERAL_ERROR_NOT_IMPLEMENTED;
}

PERIPHERAL_ERROR UnregisterButton(unsigned int joystick_index, unsigned int button_index)
{
  if (!JOYSTICKS)
    return PERIPHERAL_ERROR_FAILED;

  return PERIPHERAL_ERROR_NOT_IMPLEMENTED;
}

PERIPHERAL_ERROR RegisterTrigger(unsigned int joystick_index, JOYSTICK_MAP_TRIGGER* trigger)
{
  if (!trigger)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  if (!JOYSTICKS)
    return PERIPHERAL_ERROR_FAILED;

  TriggerMap triggerMap(*trigger);

  return PERIPHERAL_ERROR_NOT_IMPLEMENTED;
}

PERIPHERAL_ERROR UnregisterTrigger(unsigned int joystick_index, unsigned int trigger_index)
{
  if (!JOYSTICKS)
    return PERIPHERAL_ERROR_FAILED;

  return PERIPHERAL_ERROR_NOT_IMPLEMENTED;
}

PERIPHERAL_ERROR RegisterAnalogStick(unsigned int joystick_index, JOYSTICK_MAP_ANALOG_STICK* analog_stick)
{
  if (!JOYSTICKS)
    return PERIPHERAL_ERROR_FAILED;

  if (!analog_stick)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  AnalogStickMap analogStickMap(*analog_stick);

  return PERIPHERAL_ERROR_NOT_IMPLEMENTED;
}

PERIPHERAL_ERROR UnregisterAnalogStick(unsigned int joystick_index, unsigned int analog_stick_index)
{
  if (!JOYSTICKS)
    return PERIPHERAL_ERROR_FAILED;

  return PERIPHERAL_ERROR_NOT_IMPLEMENTED;
}

PERIPHERAL_ERROR GetEvents(unsigned int* event_count, JOYSTICK_EVENT** events)
{
  if (!event_count || !events)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  if (!JOYSTICKS)
    return PERIPHERAL_ERROR_FAILED;

  *event_count = 0;
  *events = NULL;

  EventMap joystickEvents;
  JOYSTICKS->GetEvents(joystickEvents);

  for (EventMap::const_iterator it = joystickEvents.begin(); it != joystickEvents.end(); ++it)
  {
    unsigned int joystickIndex = it->first;
    const EventVector& vecEvents = it->second;

    if (!vecEvents.empty())
    {
      *event_count = vecEvents.size();
      *events = new JOYSTICK_EVENT[vecEvents.size()];

      for (unsigned int i = 0; i < vecEvents.size(); i++)
      {
        const EventPtr& eventPtr = vecEvents.at(i);

        events[i]->type = eventPtr->Type();
        events[i]->joystick_index = joystickIndex;

        switch (eventPtr->Type())
        {
        case JOYSTICK_EVENT_TYPE_RAW_BUTTON:
        {
          JOYSTICK_EVENT_RAW_BUTTON* eventStruct = new JOYSTICK_EVENT_RAW_BUTTON;
          eventStruct->index = static_cast<ButtonEvent*>(eventPtr.get())->Index();
          eventStruct->state = static_cast<ButtonEvent*>(eventPtr.get())->State();
          events[i]->event = eventStruct;
          break;
        }
        case JOYSTICK_EVENT_TYPE_RAW_HAT:
        {
          JOYSTICK_EVENT_RAW_HAT* eventStruct = new JOYSTICK_EVENT_RAW_HAT;
          eventStruct->index = static_cast<HatEvent*>(eventPtr.get())->Index();
          eventStruct->state = static_cast<HatEvent*>(eventPtr.get())->State();
          events[i]->event = eventStruct;
          break;
        }
        case JOYSTICK_EVENT_TYPE_RAW_AXIS:
        {
          JOYSTICK_EVENT_RAW_AXIS* eventStruct = new JOYSTICK_EVENT_RAW_AXIS;
          eventStruct->index = static_cast<AxisEvent*>(eventPtr.get())->Index();
          eventStruct->state = static_cast<AxisEvent*>(eventPtr.get())->State();
          events[i]->event = eventStruct;
          break;
        }
        case JOYSTICK_EVENT_TYPE_MAPPED_BUTTON:
        {
          JOYSTICK_EVENT_MAPPED_BUTTON* eventStruct = new JOYSTICK_EVENT_MAPPED_BUTTON;
          eventStruct->id = static_cast<MappedButtonEvent*>(eventPtr.get())->ID();
          eventStruct->state = static_cast<MappedButtonEvent*>(eventPtr.get())->State();
          events[i]->event = eventStruct;
          break;
        }
        case JOYSTICK_EVENT_TYPE_MAPPED_TRIGGER:
        {
          JOYSTICK_EVENT_MAPPED_TRIGGER* eventStruct = new JOYSTICK_EVENT_MAPPED_TRIGGER;
          eventStruct->id = static_cast<MappedTriggerEvent*>(eventPtr.get())->ID();
          eventStruct->state = static_cast<MappedTriggerEvent*>(eventPtr.get())->State();
          events[i]->event = eventStruct;
          break;
        }
        case JOYSTICK_EVENT_TYPE_MAPPED_ANALOG_STICK:
        {
          JOYSTICK_EVENT_MAPPED_ANALOG_STICK* eventStruct = new JOYSTICK_EVENT_MAPPED_ANALOG_STICK;
          eventStruct->id = static_cast<MappedAnalogStickEvent*>(eventPtr.get())->ID();
          eventStruct->state = static_cast<MappedAnalogStickEvent*>(eventPtr.get())->State();
          events[i]->event = eventStruct;
          break;
        }
        case JOYSTICK_EVENT_TYPE_NONE:
        default:
          break;
        }
      }
    }
  }

  return PERIPHERAL_ERROR_NOT_IMPLEMENTED;
}

void FreeEvents(unsigned int event_count, JOYSTICK_EVENT* events)
{
  if (events)
  {
    for (unsigned int i = 0; i < event_count; i++)
      delete events[i].event;
  }
  delete[] events;
}

} // extern "C"
