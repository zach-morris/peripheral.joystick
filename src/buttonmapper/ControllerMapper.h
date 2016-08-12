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

#include "ButtonMapTypes.h"
#include "storage/IDatabase.h"
#include "storage/StorageTypes.h"

namespace ADDON
{
  class Joystick;
}

namespace JOYSTICK
{
  class CControllerMapper : public IDatabaseCallbacks
  {
  public:
    CControllerMapper() = default;
    virtual ~CControllerMapper() = default;

    // implementation of IDatabaseCallbacks
    virtual void OnAdd(const DevicePtr& driverInfo, const ButtonMap& buttonMap);

    void TransformFeatures(const std::string& fromController,
                           const std::string& toController,
                           const FeatureVector& features,
                           FeatureVector& transformedFeatures);

  private:
    bool AddControllerMap(const std::string& controllerFrom, const FeatureVector& featuresFrom,
                          const std::string& controllerTo, const FeatureVector& featuresTo);

    const FeatureOccurrences& GetFeatureMap(const ControllerMapItem& needle, bool bSwap);
    void ReduceModel(const FeatureOccurrences& model, FeatureOccurrences& result, bool bSwap);

    ControllerMap m_map;
    ControllerMap m_reducedMap;
    DeviceSet     m_observedDevices;
  };
}
