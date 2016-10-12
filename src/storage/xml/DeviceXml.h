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

#include "storage/StorageTypes.h"

class TiXmlElement;

namespace JOYSTICK
{
  class CDevice;
  class CDeviceConfiguration;

  class CDeviceXml
  {
  public:
    static bool Serialize(const CDevice& record, TiXmlElement* pElement);
    static bool Deserialize(const TiXmlElement* pElement, CDevice& record);

    static bool SerializeConfig(const CDeviceConfiguration& config, TiXmlElement* pElement);
    static bool DeserializeConfig(const TiXmlElement* pElement, CDeviceConfiguration& config);

    static bool SerializeAxis(const AxisProperties& axisProps, TiXmlElement* pElement);
    static bool DeserializeAxis(const TiXmlElement* pElement, AxisProperties& axisProps);
  };
}
