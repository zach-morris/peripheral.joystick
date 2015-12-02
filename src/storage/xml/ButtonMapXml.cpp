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
#include "storage/ButtonMapDefinitions.h"
#include "storage/ButtonMapTranslator.h"
#include "log/Log.h"

#include "tinyxml.h"

#include <cmath>
#include <cstdlib>
#include <sstream>
#include <string>

using namespace JOYSTICK;

bool CButtonMapXml::Serialize(const FeatureVector& features, TiXmlElement* pElement)
{
  if (pElement == NULL)
    return false;

  for (FeatureVector::const_iterator it = features.begin(); it != features.end(); ++it)
  {
    const ADDON::JoystickFeature& feature = *it;

    if (!IsValid(feature))
      continue;

    TiXmlElement featureElement(BUTTONMAP_XML_ELEM_FEATURE);
    TiXmlNode* featureNode = pElement->InsertEndChild(featureElement);
    if (featureNode == NULL)
      return false;

    TiXmlElement* featureElem = featureNode->ToElement();
    if (featureElem == NULL)
      return false;

    featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_NAME, feature.Name());

    switch (feature.Type())
    {
      case JOYSTICK_FEATURE_TYPE_SCALAR:
      {
        SerializePrimitive(featureElem, feature.Primitive());

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
      {
        if (!SerializePrimitiveTag(featureElem, feature.Up(), BUTTONMAP_XML_ELEM_UP))
          return false;

        if (!SerializePrimitiveTag(featureElem, feature.Down(), BUTTONMAP_XML_ELEM_DOWN))
          return false;

        if (!SerializePrimitiveTag(featureElem, feature.Right(), BUTTONMAP_XML_ELEM_RIGHT))
          return false;

        if (!SerializePrimitiveTag(featureElem, feature.Left(), BUTTONMAP_XML_ELEM_LEFT))
          return false;

        break;
      }
      case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
      {
        if (!SerializePrimitiveTag(featureElem, feature.PositiveX(), BUTTONMAP_XML_ELEM_POSITIVE_X))
          return false;

        if (!SerializePrimitiveTag(featureElem, feature.PositiveY(), BUTTONMAP_XML_ELEM_POSITIVE_Y))
          return false;

        if (!SerializePrimitiveTag(featureElem, feature.PositiveZ(), BUTTONMAP_XML_ELEM_POSITIVE_Z))
          return false;

        break;
      }
      default:
        break;
    }
  }

  return true;
}

bool CButtonMapXml::IsValid(const ADDON::JoystickFeature& feature)
{
  bool bIsValid = false;

  switch (feature.Type())
  {
    case JOYSTICK_FEATURE_TYPE_SCALAR:
    {
      if (feature.Primitive().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
        bIsValid = true;
      break;
    }
    case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
    {
      if (feature.Up().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN ||
          feature.Down().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN ||
          feature.Right().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN ||
          feature.Left().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
      {
        bIsValid = true;
      }
      break;
    }
    case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
    {
      if (feature.PositiveX().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN ||
          feature.PositiveY().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN ||
          feature.PositiveZ().Type() != JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
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

bool CButtonMapXml::SerializePrimitiveTag(TiXmlElement* pElement, const ADDON::DriverPrimitive& primitive, const char* tagName)
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

void CButtonMapXml::SerializePrimitive(TiXmlElement* pElement, const ADDON::DriverPrimitive& primitive)
{
  std::string strPrimitive = ButtonMapTranslator::ToString(primitive);
  if (!strPrimitive.empty())
  {
    switch (primitive.Type())
    {
      case JOYSTICK_DRIVER_PRIMITIVE_TYPE_BUTTON:
      {
        pElement->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_BUTTON, strPrimitive);
        break;
      }
      case JOYSTICK_DRIVER_PRIMITIVE_TYPE_HAT_DIRECTION:
      {
        pElement->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_HAT, strPrimitive);
        break;
      }
      case JOYSTICK_DRIVER_PRIMITIVE_TYPE_SEMIAXIS:
      {
        pElement->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_AXIS, strPrimitive);
        break;
      }
      default:
        break;
    }
  }
}

bool CButtonMapXml::Deserialize(const TiXmlElement* pElement, FeatureVector& features)
{
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

    const TiXmlElement* pUp = nullptr;
    const TiXmlElement* pDown = nullptr;
    const TiXmlElement* pRight = nullptr;
    const TiXmlElement* pLeft = nullptr;

    const TiXmlElement* pPositiveX = nullptr;
    const TiXmlElement* pPositiveY = nullptr;
    const TiXmlElement* pPositiveZ = nullptr;

    // Determine the feature type
    JOYSTICK_FEATURE_TYPE type;

    ADDON::DriverPrimitive primitive;
    if (DeserializePrimitive(pFeature, primitive, strName))
    {
      type = JOYSTICK_FEATURE_TYPE_SCALAR;
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

    ADDON::JoystickFeature feature(strName, type);

    // Deserialize according to type
    switch (type)
    {
      case JOYSTICK_FEATURE_TYPE_SCALAR:
      {
        feature.SetPrimitive(primitive);
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

        feature.SetUp(up);
        feature.SetDown(down);
        feature.SetRight(right);
        feature.SetLeft(left);

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

        feature.SetPositiveX(positiveX);
        feature.SetPositiveY(positiveY);
        feature.SetPositiveZ(positiveZ);

        break;
      }
      default:
        break;
    }

    features.push_back(feature);

    pFeature = pFeature->NextSiblingElement(BUTTONMAP_XML_ELEM_FEATURE);
  }

  return true;
}

bool CButtonMapXml::DeserializePrimitive(const TiXmlElement* pElement, ADDON::DriverPrimitive& primitive, const std::string& featureName)
{
  const char* button = pElement->Attribute(BUTTONMAP_XML_ATTR_FEATURE_BUTTON);
  if (button)
  {
    primitive = ButtonMapTranslator::ToDriverPrimitive(button);
  }
  else
  {
    const char* hat = pElement->Attribute(BUTTONMAP_XML_ATTR_FEATURE_HAT);
    if (hat)
    {
      primitive = ButtonMapTranslator::ToDriverPrimitive(hat);
    }
    else
    {
      const char* axis = pElement->Attribute(BUTTONMAP_XML_ATTR_FEATURE_AXIS);
      if (axis)
      {
        primitive = ButtonMapTranslator::ToDriverPrimitive(axis);
      }
      else
      {
        return false;
      }
    }
  }

  return true;
}
