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
      case JOYSTICK_FEATURE_TYPE_PRIMITIVE:
      {
        const ADDON::PrimitiveFeature* primitiveFeature = static_cast<const ADDON::PrimitiveFeature*>(feature);

        if (!SerializePrimitive(featureElem, primitiveFeature->Primitive(), BUTTONMAP_XML_ELEM_PRIMITIVE))
          return false;

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
      {
        const ADDON::AnalogStick* analogStick = static_cast<const ADDON::AnalogStick*>(feature);

        if (!SerializePrimitive(featureElem, analogStick->Up(), BUTTONMAP_XML_ELEM_UP))
          return false;

        if (!SerializePrimitive(featureElem, analogStick->Down(), BUTTONMAP_XML_ELEM_DOWN))
          return false;

        if (!SerializePrimitive(featureElem, analogStick->Right(), BUTTONMAP_XML_ELEM_RIGHT))
          return false;

        if (!SerializePrimitive(featureElem, analogStick->Left(), BUTTONMAP_XML_ELEM_LEFT))
          return false;

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
      {
        const ADDON::Accelerometer* accelerometer = static_cast<const ADDON::Accelerometer*>(feature);

        if (!SerializePrimitive(featureElem, accelerometer->PositiveX(), BUTTONMAP_XML_ELEM_POSITIVE_X))
          return false;

        if (!SerializePrimitive(featureElem, accelerometer->PositiveY(), BUTTONMAP_XML_ELEM_POSITIVE_Y))
          return false;

        if (!SerializePrimitive(featureElem, accelerometer->PositiveZ(), BUTTONMAP_XML_ELEM_POSITIVE_Z))
          return false;

        break;
      }
      default:
        break;
    }
  }

  return true;
}

bool CButtonMapXml::SerializePrimitive(TiXmlElement* pElement, const ADDON::DriverPrimitive& primitive, const char* tagName) const
{
  if (primitive.Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
  {
    if (pElement == NULL)
      return false;

    TiXmlElement primitiveElement(tagName);
    TiXmlNode* primitiveNode = pElement->InsertEndChild(primitiveElement);
    if (primitiveNode == NULL)
      return false;

    TiXmlElement* primitiveElem = primitiveNode->ToElement();
    if (primitiveElem == NULL)
      return false;

    switch (primitive.Type())
    {
      case JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON:
      {
        primitiveElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_BUTTON, primitive.DriverIndex());
        break;
      }
      case JOYSTICK_DRIVER_PRIMITIVE_TYPE_HAT_DIRECTION:
      {
        primitiveElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_HAT, JoystickTranslator::TranslateHatDir(primitive.HatDirection()));
        break;
      }

      case JOYSTICK_DRIVER_PRIMITIVE_TYPE_SEMIAXIS:
      {
          std::ostringstream strAxis;
        if (primitive.SemiAxisDirection() == JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE)
          strAxis << "-";
        else
          strAxis << "+";
        strAxis << primitive.DriverIndex();

        primitiveElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_AXIS, strAxis.str());
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

    std::string strName(name);

    ADDON::JoystickFeature* feature = nullptr;

    const TiXmlElement* pPrimitive = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_PRIMITIVE);
    const TiXmlElement* pUp = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_UP);
    const TiXmlElement* pDown = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_DOWN);
    const TiXmlElement* pRight = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_RIGHT);
    const TiXmlElement* pLeft = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_LEFT);
    const TiXmlElement* pPositiveX = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_POSITIVE_X);
    const TiXmlElement* pPositiveY = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_POSITIVE_Y);
    const TiXmlElement* pPositiveZ = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_POSITIVE_Z);

    JOYSTICK_FEATURE_TYPE type = JOYSTICK_FEATURE_TYPE_UNKNOWN;

    if (pPrimitive)
      type = JOYSTICK_FEATURE_TYPE_PRIMITIVE;
    else if (pUp && pDown && pRight && pLeft)
      type = JOYSTICK_FEATURE_TYPE_ANALOG_STICK;
    else if (pPositiveX && pPositiveY && pPositiveZ)
      type = JOYSTICK_FEATURE_TYPE_ACCELEROMETER;

    switch (type)
    {
      case JOYSTICK_FEATURE_TYPE_PRIMITIVE:
      {
        ADDON::DriverPrimitive primitive;

        if (!DeserializePrimitive(pPrimitive, primitive, strName))
          return false;

        feature = new ADDON::PrimitiveFeature(strName, primitive);

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
      {
        ADDON::DriverPrimitive up;
        ADDON::DriverPrimitive down;
        ADDON::DriverPrimitive right;
        ADDON::DriverPrimitive left;

        if (!DeserializePrimitive(pUp, up, strName) ||
            !DeserializePrimitive(pDown, down, strName) ||
            !DeserializePrimitive(pRight, right, strName) ||
            !DeserializePrimitive(pLeft, left, strName))
        {
          return false;
        }

        feature = new ADDON::AnalogStick(strName, up, down, right, left);

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
      {
        ADDON::DriverPrimitive positiveX;
        ADDON::DriverPrimitive positiveY;
        ADDON::DriverPrimitive positiveZ;

        if (!DeserializePrimitive(pPositiveX, positiveX, strName) ||
            !DeserializePrimitive(pPositiveY, positiveY, strName) ||
            !DeserializePrimitive(pPositiveZ, positiveZ, strName))
        {
          return false;
        }

        feature = new ADDON::Accelerometer(strName, positiveX, positiveY, positiveZ);

        break;
      }
      default:
        break;
    }

    if (feature)
      m_buttons[name] = feature;

    pFeature = pFeature->NextSiblingElement(BUTTONMAP_XML_ELEM_FEATURE);
  }

  return true;
}

bool CButtonMapXml::DeserializePrimitive(const TiXmlElement* pElement, ADDON::DriverPrimitive& primitive, const std::string& featureName) const
{
  const char* button = pElement->Attribute(BUTTONMAP_XML_ATTR_FEATURE_BUTTON);
  const char* hat = pElement->Attribute(BUTTONMAP_XML_ATTR_FEATURE_HAT);
  const char* axis = pElement->Attribute(BUTTONMAP_XML_ATTR_FEATURE_AXIS);

  JOYSTICK_DRIVER_PRIMITIVE_TYPE primitiveType = JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN;

  if (button)
    primitiveType = JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON;
  else if (hat)
    primitiveType = JOYSTICK_DRIVER_PRIMITIVE_TYPE_HAT_DIRECTION;
  else if (axis)
    primitiveType = JOYSTICK_DRIVER_PRIMITIVE_TYPE_SEMIAXIS;

  switch (primitiveType)
  {
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON:
    {
      int buttonIndex = std::atoi(button);

      primitive = ADDON::DriverPrimitive(buttonIndex);

      break;
    }
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_HAT_DIRECTION:
    {
      const int hatIndex = 0;

      JOYSTICK_DRIVER_HAT_DIRECTION dir = JoystickTranslator::TranslateHatDir(hat);
      if (dir == JOYSTICK_DRIVER_HAT_UNKNOWN)
      {
        esyslog("<%s> tag name=\"%s\" attribute \"%s\" is invalid: \"%s\"",
                BUTTONMAP_XML_ELEM_FEATURE, featureName.c_str(), BUTTONMAP_XML_ATTR_FEATURE_HAT, hat);
        return false;
      }

      primitive = ADDON::DriverPrimitive(hatIndex, dir);

      break;
    }
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_SEMIAXIS:
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
                BUTTONMAP_XML_ELEM_FEATURE, featureName.c_str(), BUTTONMAP_XML_ATTR_FEATURE_AXIS, axis);
        return false;
      }

      primitive = ADDON::DriverPrimitive(axisIndex, dir);

      break;
    }
    default:
      break;
  }

  return true;
}
