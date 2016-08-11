/*
 *      Copyright (C) 2016 Garrett Brown
 *      Copyright (C) 2016 Team Kodi
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "ControllerMapper.h"

#include "kodi_peripheral_utils.hpp"

#include <algorithm>

using namespace JOYSTICK;

void CControllerMapper::OnAdd(const CDevice& driverInfo, const ButtonMap& buttonMap)
{
  bool bChanged = false;

  for (auto itTo = buttonMap.begin(); itTo != buttonMap.end(); ++itTo)
  {
    // Only allow controller map items where "from" compares before "to"
    for (auto itFrom = buttonMap.begin(); itFrom->first < itTo->first; ++itFrom)
    {
      bChanged |= AddControllerMap(itFrom->first, itFrom->second, itTo->first, itTo->second);
    }
  }

  // Clear cache
  if (bChanged)
    m_reducedMap.clear();
}

bool CControllerMapper::AddControllerMap(const std::string& controllerFrom, const FeatureVector& featuresFrom,
                                         const std::string& controllerTo, const FeatureVector& featuresTo)
{
  bool bChanged = false;

  ControllerMapItem needle = { controllerFrom, controllerTo };

  auto it = m_map.find(needle);
  if (it == m_map.end())
  {
    m_map.insert(needle);
    it = m_map.find(needle);
  }

  ControllerMapItem& controllerItem = const_cast<ControllerMapItem&>(*it);

  for (auto itFromFeature = featuresFrom.begin(); itFromFeature != featuresFrom.end(); ++itFromFeature)
  {
    auto itToFeature = std::find_if(featuresTo.begin(), featuresTo.end(),
      [&itFromFeature](const ADDON::JoystickFeature& feature)
      {
        if (itFromFeature->Type() == feature.Type())
        {
          switch (feature.Type())
          {
          case JOYSTICK_FEATURE_TYPE_SCALAR:
          case JOYSTICK_FEATURE_TYPE_MOTOR:
          {
            return itFromFeature->Primitive() == feature.Primitive();
          }
          case JOYSTICK_FEATURE_TYPE_ANALOG_STICK:
          {
            return itFromFeature->Up()    == feature.Up() &&
                   itFromFeature->Down()  == feature.Down() &&
                   itFromFeature->Right() == feature.Right() &&
                   itFromFeature->Left()  == feature.Left();
          }
          case JOYSTICK_FEATURE_TYPE_ACCELEROMETER:
          {
            return itFromFeature->PositiveX() == feature.PositiveX() &&
                   itFromFeature->PositiveY() == feature.PositiveY() &&
                   itFromFeature->PositiveZ() == feature.PositiveZ();
          }
          default:
            break;
          }
        }
        return false;
      });

    if (itToFeature != featuresTo.end())
    {
      FeatureMapItem featureMapItem = { itFromFeature->Name(), itToFeature->Name() };
      ++controllerItem.featureMap[featureMapItem];
      bChanged = true;
    }
  }

  return bChanged;
}

void CControllerMapper::TransformFeatures(const std::string& fromController,
                                          const std::string& toController,
                                          const FeatureVector& features,
                                          FeatureVector& transformedFeatures)
{
  bool bSwap = (fromController >= toController);

  ControllerMapItem needle = { bSwap ? toController : fromController,
                               bSwap ? fromController : toController };

  const FeatureOccurrences& featureMap = GetFeatureMap(needle, bSwap);

  for (auto itMap = featureMap.begin(); itMap != featureMap.end(); ++itMap)
  {
    const std::string& fromFeature = bSwap ? itMap->first.second : itMap->first.first;
    const std::string& toFeature = bSwap ? itMap->first.first : itMap->first.second;

    auto itFrom = std::find_if(features.begin(), features.end(),
      [fromFeature](const ADDON::JoystickFeature& feature)
      {
        return feature.Name() == fromFeature;
      });

    if (itFrom != features.end())
    {
      ADDON::JoystickFeature transformedFeature(*itFrom);
      transformedFeature.SetName(toFeature);
      transformedFeatures.emplace_back(std::move(transformedFeature));
    }
  }
}

const FeatureOccurrences& CControllerMapper::GetFeatureMap(const ControllerMapItem& needle, bool bSwap)
{
  ControllerMap::iterator it = m_reducedMap.find(needle);
  if (it == m_reducedMap.end())
  {
    m_reducedMap.insert(needle);
    it = m_reducedMap.find(needle);
  }

  ControllerMapItem& controllerItem = const_cast<ControllerMapItem&>(*it);

  FeatureOccurrences& featureMap = controllerItem.featureMap;

  if (featureMap.empty())
  {
    auto it = m_map.find(needle);
    if (it != m_map.end())
    {
      const ControllerMapItem& model = *it;
      ReduceModel(model.featureMap, featureMap, bSwap);
    }
  }

  return featureMap;
}

void CControllerMapper::ReduceModel(const FeatureOccurrences& model, FeatureOccurrences& result, bool bSwap)
{
  // First pass, calculate max occurrence counts
  std::map<std::string, unsigned int> maxFeatureCounts;
  for (auto it = model.begin(); it != model.end(); ++it)
  {
    const std::string& fromFeature = it->first.first;
    const std::string& toFeature = it->first.second;
    const std::string& feature = bSwap ? toFeature : fromFeature;
    const unsigned int count = it->second;

    unsigned int& maxCount = maxFeatureCounts[feature];
    if (count > maxCount)
      maxCount = count;
  }

  // Second pass, assign features with max occurrences to result
  for (auto it = model.begin(); it != model.end(); ++it)
  {
    const std::string& fromFeature = it->first.first;
    const std::string& toFeature = it->first.second;
    const std::string& feature = bSwap ? toFeature : fromFeature;
    const unsigned int count = it->second;

    unsigned int& targetCount = maxFeatureCounts[feature];
    if (targetCount == count)
    {
      result[std::make_pair(fromFeature, toFeature)] = 1;
      targetCount = 0;
    }
  }
}
