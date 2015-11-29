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

#include <string>

class TiXmlElement;

namespace ADDON
{
  class DriverPrimitive;
  class JoystickFeature;
}

namespace JOYSTICK
{
  class CButtonMap;

  class CButtonMapXml
  {
  public:
    static bool Serialize(const CButtonMap& record, TiXmlElement* pElement);
    static bool Deserialize(const TiXmlElement* pElement, CButtonMap& record);

  private:
    static bool IsValid(const ADDON::JoystickFeature* feature);
    static bool SerializeFeature(TiXmlElement* pElement, const ADDON::DriverPrimitive& primitive, const char* tagName);
    static bool SerializePrimitiveTag(TiXmlElement* pElement, const ADDON::DriverPrimitive& primitive, const char* tagName);
    static void SerializePrimitive(TiXmlElement* pElement, const ADDON::DriverPrimitive& primitive);
    static bool DeserializePrimitive(const TiXmlElement* pElement, ADDON::DriverPrimitive& primitive, const std::string& featureName);
  };
}
