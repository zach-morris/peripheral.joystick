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

#include "ButtonMapXml.h"
#include "JoystickDefinitions.h"
#include "JoystickTranslator.h"
#include "log/Log.h"

#include "tinyxml.h"

#include <cmath>
#include <cstdlib>
#include <sstream>

using namespace JOYSTICK;

bool CButtonMapXml::Serialize(TiXmlElement* pElement) const
{
  if (pElement == NULL)
    return false;

  for (Buttons::const_iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    const std::string& strFeatureName = it->first;
    const ADDON::JoystickFeature* feature = it->second;

    TiXmlElement featureElement(BUTTONMAP_XML_ELEM_FEATURE);
    TiXmlNode* featureNode = pElement->InsertEndChild(featureElement);
    if (featureNode == NULL)
      return false;

    TiXmlElement* featureElem = featureNode->ToElement();
    if (featureElem == NULL)
      return false;

    featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_NAME, strFeatureName);

    switch (feature->Type())
    {
      case JOYSTICK_DRIVER_TYPE_BUTTON:
      {
        const ADDON::DriverButton* button = static_cast<const ADDON::DriverButton*>(feature);

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_BUTTON, button->Index());

        break;
      }
      case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
      {
        const ADDON::DriverHat* hat = static_cast<const ADDON::DriverHat*>(feature);

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_HAT, JoystickTranslator::TranslateHatDir(hat->Direction()));

        break;
      }
      case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
      {
        const ADDON::DriverSemiAxis* semiAxis = static_cast<const ADDON::DriverSemiAxis*>(feature);

        std::ostringstream strAxis;
        if (semiAxis->Direction() == JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE)
          strAxis << "-";
        else
          strAxis << "+";
        strAxis << semiAxis->Index();

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_AXIS, strAxis.str());

        break;
      }
      case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
      {
        const ADDON::DriverAnalogStick* analogStick = static_cast<const ADDON::DriverAnalogStick*>(feature);

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_XAXIS, analogStick->XIndex());
        if (analogStick->XInverted())
          featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_XINVERTED, "true");

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_YAXIS, analogStick->YIndex());
        if (analogStick->YInverted())
          featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_YINVERTED, "true");

        break;
      }
      case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
      {
        const ADDON::DriverAccelerometer* accelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature);

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_XAXIS, accelerometer->XIndex());
        if (accelerometer->XInverted())
          featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_XINVERTED, "true");

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_YAXIS, accelerometer->YIndex());
        if (accelerometer->YInverted())
          featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_YINVERTED, "true");

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_ZAXIS, accelerometer->ZIndex());
        if (accelerometer->ZInverted())
          featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_ZINVERTED, "true");

        break;
      }
      default:
        break;
    }
  }

  return true;
}

bool CButtonMapXml::Deserialize(const TiXmlElement* pElement)
{

  Reset();

  const TiXmlElement* pFeature = pElement->FirstChildElement(BUTTONMAP_XML_ELEM_FEATURE);

  if (!pFeature)
  {
    esyslog("Can't find <%s> tag", BUTTONMAP_XML_ELEM_FEATURE);
    return false;
  }

  while (pFeature)
  {
    const char* name = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_NAME);
    if (!name)
    {
      esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELEM_FEATURE, BUTTONMAP_XML_ATTR_FEATURE_NAME);
      return false;
    }

    ADDON::JoystickFeature* feature = NULL;

    const char* button = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_BUTTON);
    const char* hat = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_HAT);
    const char* axis = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_AXIS);
    const char* xaxis = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_XAXIS);
    const char* yaxis = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_YAXIS);
    const char* zaxis = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_ZAXIS);
    const char* xinverted = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_XINVERTED);
    const char* yinverted = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_YINVERTED);
    const char* zinverted = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_ZINVERTED);

    if (button)
    {
      int buttonIndex = std::atoi(button);
      feature = new ADDON::DriverButton(name, buttonIndex);
    }
    else if (hat)
    {
      const int hatIndex = 0;

      JOYSTICK_DRIVER_HAT_DIRECTION dir = JoystickTranslator::TranslateHatDir(hat);
      if (dir == JOYSTICK_DRIVER_HAT_UNKNOWN)
      {
        esyslog("<%s> tag name=\"%s\" attribute \"%s\" is invalid: \"%s\"",
                BUTTONMAP_XML_ELEM_FEATURE, name, BUTTONMAP_XML_ATTR_FEATURE_HAT, hat);
        return false;
      }

      feature = new ADDON::DriverHat(name, hatIndex, dir);
    }
    else if (axis)
    {
      int axisIndex = std::abs(std::atoi(axis));

      JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir = JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_UNKNOWN;

      if (axis[0] == '+')
        dir = JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE;
      else if (axis[0] == '-')
        dir = JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE;

      if (dir == JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_UNKNOWN)
      {
        esyslog("<%s> tag name=\"%s\" attribute \"%s\" is invalid: \"%s\"",
                BUTTONMAP_XML_ELEM_FEATURE, name, BUTTONMAP_XML_ATTR_FEATURE_AXIS, axis);
        return false;
      }

      feature = new ADDON::DriverSemiAxis(name, axisIndex, dir);
    }
    else if (xaxis && yaxis && !zaxis)
    {
      int axisIndexX = std::atoi(xaxis);
      int axisIndexY = std::atoi(yaxis);

      bool bInvertedX = std::string(xinverted ? xinverted : "") == "true";
      bool bInvertedY = std::string(yinverted ? yinverted : "") == "true";

      feature = new ADDON::DriverAnalogStick(name,
                                             axisIndexX, bInvertedX,
                                             axisIndexY, bInvertedY);
    }
    else if (xaxis && yaxis && zaxis)
    {
      int axisIndexX = std::atoi(xaxis);
      int axisIndexY = std::atoi(yaxis);
      int axisIndexZ = std::atoi(zaxis);

      bool bInvertedX = std::string(xinverted ? xinverted : "") == "true";
      bool bInvertedY = std::string(yinverted ? yinverted : "") == "true";
      bool bInvertedZ = std::string(zinverted ? zinverted : "") == "true";

      feature = new ADDON::DriverAccelerometer(name,
                                               axisIndexX, bInvertedX,
                                               axisIndexY, bInvertedY,
                                               axisIndexZ, bInvertedZ);
    }
    else
    {
      esyslog("Invalid <%s> tag: %s", BUTTONMAP_XML_ELEM_FEATURE, name);
      return false;
    }

    m_buttons[name] = feature;

    pFeature = pFeature->NextSiblingElement(BUTTONMAP_XML_ELEM_FEATURE);
  }

  return true;
}
