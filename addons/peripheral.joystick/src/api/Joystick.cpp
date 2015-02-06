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
 */

#include "Joystick.h"
#include "JoystickInterface.h"
#include "log/Log.h"
#include "utils/CommonMacros.h"

#include <platform/util/timeutils.h>

using namespace JOYSTICK;

#define ANALOG_EPSILON  0.0001f

const float CJoystick::m_deadzoneRange = 0.2f; // TODO: Get deadzone from settings

CJoystick::CJoystick(CJoystickInterface* api)
 : m_api(api),
   m_discoverTimeMs(PLATFORM::GetTimeMs()),
   m_firstEventTimeMs(-1),
   m_lastEventTimeMs(-1)
{
  ASSERT(m_api);
  SetProvider(api->Name());
}

bool CJoystick::Initialize(void)
{
  m_state.buttons.assign(ButtonCount(), JOYSTICK_STATE_BUTTON());
  m_state.hats.assign(HatCount(), JOYSTICK_STATE_HAT());
  m_state.axes.assign(AxisCount(), JOYSTICK_STATE_AXIS());

  isyslog("Initialized joystick \"%s\" (%s), axes: %u, hats: %u, buttons: %u",
    Name().c_str(), m_api->Name().c_str(), AxisCount(), HatCount(), ButtonCount());

  return true;
}

bool CJoystick::GetEvents(std::vector<ADDON::PeripheralEvent>& events)
{
  const bool bSuccess = ScanEvents(events);

  if (bSuccess && !events.empty())
    UpdateTimers();

  return bSuccess;
}

void CJoystick::GetButtonEvents(const std::vector<JOYSTICK_STATE_BUTTON>& buttons, std::vector<ADDON::PeripheralEvent>& events)
{
  if (buttons.size() != m_state.buttons.size())
  {
    esyslog("Error: button mismatch! Updating %u buttons, but state contains %u buttons", buttons.size(), m_state.buttons.size());
    return;
  }

  for (unsigned int i = 0; i < buttons.size(); i++)
  {
    if (buttons[i] != m_state.buttons[i])
      events.push_back(ADDON::PeripheralEvent(DriverIndex(), i, buttons[i]));
  }

  m_state.buttons.assign(buttons.begin(), buttons.end());
}

void CJoystick::GetHatEvents(const std::vector<JOYSTICK_STATE_HAT>& hats, std::vector<ADDON::PeripheralEvent>& events)
{
  if (hats.size() != m_state.hats.size())
  {
    esyslog("Error: hat mismatch! Updating %u hats, but state contains %u hats", hats.size(), m_state.hats.size());
    return;
  }

  for (unsigned int i = 0; i < hats.size(); i++)
  {
    if (hats[i] != m_state.hats[i])
      events.push_back(ADDON::PeripheralEvent(DriverIndex(), i, hats[i]));
  }

  m_state.hats.assign(hats.begin(), hats.end());
}

void CJoystick::GetAxisEvents(const std::vector<JOYSTICK_STATE_AXIS>& axes, std::vector<ADDON::PeripheralEvent>& events)
{
  if (axes.size() != m_state.axes.size())
  {
    esyslog("Error: axis mismatch! Updating %u axes, but state contains %u axes", axes.size(), m_state.axes.size());
    return;
  }

  for (unsigned int i = 0; i < axes.size(); i++)
  {
    if (axes[i] != 0.0f || m_state.axes[i] != 0.0f)
      events.push_back(ADDON::PeripheralEvent(DriverIndex(), i, axes[i]));
  }

  m_state.axes.assign(axes.begin(), axes.end());
}

float CJoystick::NormalizeAxis(long value, long maxAxisAmount)
{
  float position = 1.0f * CONSTRAIN(-maxAxisAmount, value, maxAxisAmount) / maxAxisAmount;

  if (position > m_deadzoneRange)
    return (float)(position - m_deadzoneRange) / (float)(1.0f - m_deadzoneRange);
  else if (position < -m_deadzoneRange)
    return (float)(position + m_deadzoneRange) / (float)(1.0f - m_deadzoneRange);
  else
    return 0.0f;
}

void CJoystick::UpdateTimers(void)
{
  if (m_firstEventTimeMs < 0)
    m_firstEventTimeMs = PLATFORM::GetTimeMs();
  m_lastEventTimeMs = PLATFORM::GetTimeMs();
}
