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

#include "JoystickInterfaceXInput.h"
#include "JoystickXInput.h"
#include "XInputDLL.h"
#include "api/JoystickTypes.h"
#include "log/Log.h"

#include <array>
#include <Xinput.h>

using namespace JOYSTICK;

#define MAX_JOYSTICKS 4

// --- DefaultFeatures ---------------------------------------------------------

namespace JOYSTICK
{
  struct FeatureStruct
  {
    const char*           controllerId;
    const char*           name;
    JOYSTICK_FEATURE_TYPE type;
    unsigned int          driverIndex;
  };

  std::array<FeatureStruct, 4> DefaultFeatures = {
    {
      { "game.controller.default", "leftmotor",   JOYSTICK_FEATURE_TYPE_MOTOR, MOTOR_LEFT },
      { "game.controller.default", "rightmotor",  JOYSTICK_FEATURE_TYPE_MOTOR, MOTOR_RIGHT },
      { "game.controller.ps",      "strongmotor", JOYSTICK_FEATURE_TYPE_MOTOR, MOTOR_LEFT },
      { "game.controller.ps",      "weakmotor",   JOYSTICK_FEATURE_TYPE_MOTOR, MOTOR_RIGHT },
    }
  };
}

// --- CJoystickInterfaceXInput ------------------------------------------------

const char* CJoystickInterfaceXInput::Name(void) const
{
  return INTERFACE_XINPUT;
}

bool CJoystickInterfaceXInput::Initialize(void)
{
  return CXInputDLL::Get().Load();
}

void CJoystickInterfaceXInput::Deinitialize(void)
{
  CXInputDLL::Get().Unload();
}

bool CJoystickInterfaceXInput::ScanForJoysticks(JoystickVector& joysticks)
{
  XINPUT_STATE_EX controllerState; // No need to memset, only checking for controller existence

  for (unsigned int i = 0; i < MAX_JOYSTICKS; i++)
  {
    if (!CXInputDLL::Get().GetState(i, controllerState))
      continue;

    isyslog("Found a XInput controller on port %u", i);
    joysticks.push_back(JoystickPtr(new CJoystickXInput(i)));
  }

  return true;
}

void CJoystickInterfaceXInput::GetFeatures(const std::string& controllerId, FeatureVector& features)
{
  for (auto& featureStruct : DefaultFeatures)
  {
    if (controllerId == featureStruct.controllerId)
    {
      ADDON::JoystickFeature feature(featureStruct.name, featureStruct.type);
      feature.SetPrimitive(ADDON::DriverPrimitive::CreateMotor(featureStruct.driverIndex));
      features.push_back(std::move(feature));
    }
  }
}
