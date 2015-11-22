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

#include "ButtonMapRecordXml.h"
#include "storage/schema/ButtonMapRecord.h"
#include "JoystickDefinitions.h"
#include "JoystickTranslator.h"
#include "log/Log.h"

#include "tinyxml.h"

#include <cmath>
#include <cstdlib>
#include <sstream>
#include <string>

using namespace JOYSTICK;

bool CButtonMapRecordXml::Serialize(const CButtonMapRecord& record, TiXmlElement* pElement)
{
  if (pElement == NULL)
    return false;

  for (CButtonMapRecord::ButtonMap::const_iterator it = record.GetButtonMap().begin(); it != record.GetButtonMap().end(); ++it)
  {
    const std::string& strFeatureName = it->first;
    const ADDON::JoystickFeature* feature = it->second;

    if (!IsValid(feature))
      continue;

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

        SerializePrimitive(featureElem, primitiveFeature->Primitive());

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
      {
        const ADDON::AnalogStick* analogStick = static_cast<const ADDON::AnalogStick*>(feature);

        if (!SerializePrimitiveTag(featureElem, analogStick->Up(), BUTTONMAP_XML_ELEM_UP))
          return false;

        if (!SerializePrimitiveTag(featureElem, analogStick->Down(), BUTTONMAP_XML_ELEM_DOWN))
          return false;

        if (!SerializePrimitiveTag(featureElem, analogStick->Right(), BUTTONMAP_XML_ELEM_RIGHT))
          return false;

        if (!SerializePrimitiveTag(featureElem, analogStick->Left(), BUTTONMAP_XML_ELEM_LEFT))
          return false;

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
      {
        const ADDON::Accelerometer* accelerometer = static_cast<const ADDON::Accelerometer*>(feature);

        if (!SerializePrimitiveTag(featureElem, accelerometer->PositiveX(), BUTTONMAP_XML_ELEM_POSITIVE_X))
          return false;

        if (!SerializePrimitiveTag(featureElem, accelerometer->PositiveY(), BUTTONMAP_XML_ELEM_POSITIVE_Y))
          return false;

        if (!SerializePrimitiveTag(featureElem, accelerometer->PositiveZ(), BUTTONMAP_XML_ELEM_POSITIVE_Z))
          return false;

        break;
      }
      default:
        break;
    }
  }

  return true;
}

bool CButtonMapRecordXml::IsValid(const ADDON::JoystickFeature* feature)
{
  bool bIsValid = false;

  switch (feature->Type())
  {
    case JOYSTICK_FEATURE_TYPE_PRIMITIVE:
    {
      const ADDON::PrimitiveFeature* primitiveFeature = static_cast<const ADDON::PrimitiveFeature*>(feature);

      if (primitiveFeature->Primitive().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
        bIsValid = true;

      break;
    }
    case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
    {
      const ADDON::AnalogStick* analogStick = static_cast<const ADDON::AnalogStick*>(feature);

      if (analogStick->Up().Type()    != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN ||
          analogStick->Down().Type()  != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN ||
          analogStick->Right().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN ||
          analogStick->Left().Type()  != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
      {
        bIsValid = true;
      }

      break;
    }
    case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
    {
      const ADDON::Accelerometer* accelerometer = static_cast<const ADDON::Accelerometer*>(feature);

      if (accelerometer->PositiveX().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN ||
          accelerometer->PositiveY().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN ||
          accelerometer->PositiveZ().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
      {
        bIsValid = true;
      }

      break;
    }
    default:
      break;
  }

  return bIsValid;
}

bool CButtonMapRecordXml::SerializePrimitiveTag(TiXmlElement* pElement, const ADDON::DriverPrimitive& primitive, const char* tagName)
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

    SerializePrimitive(primitiveElem, primitive);
  }

  return true;
}

void CButtonMapRecordXml::SerializePrimitive(TiXmlElement* pElement, const ADDON::DriverPrimitive& primitive)
{
  switch (primitive.Type())
  {
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON:
    {
      pElement->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_BUTTON, primitive.DriverIndex());
      break;
    }
    case JOYSTICK_DRIVER_PRIMITIVE_TYPE_HAT_DIRECTION:
    {
      pElement->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_HAT, JoystickTranslator::TranslateHatDir(primitive.HatDirection()));
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

      pElement->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_AXIS, strAxis.str());
      break;
    }
    default:
      break;
  }
}

bool CButtonMapRecordXml::Deserialize(const TiXmlElement* pElement, CButtonMapRecord& record)
{
  const TiXmlElement* pFeature = pElement->FirstChildElement(BUTTONMAP_XML_ELEM_FEATURE);

  if (!pFeature)
  {
    esyslog("Can't find <%s> tag", BUTTONMAP_XML_ELEM_FEATURE);
    return false;
  }

  while (pFeature)
  {
    // The deserialized feature
    ADDON::JoystickFeature* feature = nullptr;

    const char* name = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_NAME);
    if (!name)
    {
      esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELEM_FEATURE, BUTTONMAP_XML_ATTR_FEATURE_NAME);
      return false;
    }
    std::string strName(name);

    const TiXmlElement* pUp = nullptr;
    const TiXmlElement* pDown = nullptr;
    const TiXmlElement* pRight = nullptr;
    const TiXmlElement* pLeft = nullptr;

    const TiXmlElement* pPositiveX = nullptr;
    const TiXmlElement* pPositiveY = nullptr;
    const TiXmlElement* pPositiveZ = nullptr;

    // Determine the feature type
    JOYSTICK_FEATURE_TYPE type = JOYSTICK_FEATURE_TYPE_UNKNOWN;

    ADDON::DriverPrimitive primitive;
    if (DeserializePrimitive(pFeature, primitive, strName))
    {
      type = JOYSTICK_FEATURE_TYPE_PRIMITIVE;
    }
    else
    {
      pUp = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_UP);
      pDown = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_DOWN);
      pRight = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_RIGHT);
      pLeft = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_LEFT);

      if (pUp || pDown || pRight || pLeft)
      {
        type = JOYSTICK_FEATURE_TYPE_ANALOG_STICK;
      }
      else
      {
        pPositiveX = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_POSITIVE_X);
        pPositiveY = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_POSITIVE_Y);
        pPositiveZ = pFeature->FirstChildElement(BUTTONMAP_XML_ELEM_POSITIVE_Z);

        if (pPositiveX || pPositiveY || pPositiveZ)
        {
          type = JOYSTICK_FEATURE_TYPE_ACCELEROMETER;
        }
        else
        {
          esyslog("Feature \"%s\": <%s> tag is not a valid primitive", strName.c_str(), BUTTONMAP_XML_ELEM_FEATURE);
          return false;
        }
      }
    }

    // Deserialize according to type
    switch (type)
    {
      case JOYSTICK_FEATURE_TYPE_PRIMITIVE:
      {
        // Already deserialized
        feature = new ADDON::PrimitiveFeature(strName, primitive);
        break;
      }
      case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
      {
        ADDON::DriverPrimitive up;
        ADDON::DriverPrimitive down;
        ADDON::DriverPrimitive right;
        ADDON::DriverPrimitive left;

        bool bSuccess = true;

        if (pUp && !DeserializePrimitive(pUp, up, strName))
        {
          esyslog("Feature \"%s\": <%s> tag is not a valid primitive", strName.c_str(), BUTTONMAP_XML_ELEM_UP);
          bSuccess = false;
        }

        if (pDown && !DeserializePrimitive(pDown, down, strName))
        {
          esyslog("Feature \"%s\": <%s> tag is not a valid primitive", strName.c_str(), BUTTONMAP_XML_ELEM_DOWN);
          bSuccess = false;
        }

        if (pRight && !DeserializePrimitive(pRight, right, strName))
        {
          esyslog("Feature \"%s\": <%s> tag is not a valid primitive", strName.c_str(), BUTTONMAP_XML_ELEM_RIGHT);
          bSuccess = false;
        }

        if (pLeft && !DeserializePrimitive(pLeft, left, strName))
        {
          esyslog("Feature \"%s\": <%s> tag is not a valid primitive", strName.c_str(), BUTTONMAP_XML_ELEM_LEFT);
          bSuccess = false;
        }

        if (!bSuccess)
          return false;

        feature = new ADDON::AnalogStick(strName, up, down, right, left);

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
      {
        ADDON::DriverPrimitive positiveX;
        ADDON::DriverPrimitive positiveY;
        ADDON::DriverPrimitive positiveZ;

        bool bSuccess = true;

        if (pPositiveX && !DeserializePrimitive(pPositiveX, positiveY, strName))
        {
          esyslog("Feature \"%s\": <%s> tag is not a valid primitive", strName.c_str(), BUTTONMAP_XML_ELEM_POSITIVE_X);
          bSuccess = false;
        }

        if (pPositiveY && !DeserializePrimitive(pPositiveY, positiveY, strName))
        {
          esyslog("Feature \"%s\": <%s> tag is not a valid primitive", strName.c_str(), BUTTONMAP_XML_ELEM_POSITIVE_Y);
          bSuccess = false;
        }

        if (pPositiveZ && !DeserializePrimitive(pPositiveZ, positiveZ, strName))
        {
          esyslog("Feature \"%s\": <%s> tag is not a valid primitive", strName.c_str(), BUTTONMAP_XML_ELEM_POSITIVE_Z);
          bSuccess = false;
        }

        if (!bSuccess)
          return false;

        feature = new ADDON::Accelerometer(strName, positiveX, positiveY, positiveZ);

        break;
      }
      default:
        break;
    }

    if (feature)
      record.MapFeature(feature);

    pFeature = pFeature->NextSiblingElement(BUTTONMAP_XML_ELEM_FEATURE);
  }

  return true;
}

bool CButtonMapRecordXml::DeserializePrimitive(const TiXmlElement* pElement, ADDON::DriverPrimitive& primitive, const std::string& featureName)
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
