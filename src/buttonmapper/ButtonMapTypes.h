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
 */
#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ADDON
{
  class JoystickFeature;
};

namespace JOYSTICK
{
  typedef std::string                            ControllerID;
  typedef std::string                            FeatureName;

  typedef std::vector<ADDON::JoystickFeature>    FeatureVector;
  typedef std::map<ControllerID, FeatureVector>  ButtonMap;

  typedef std::pair<FeatureName, FeatureName>    FeatureMapItem; // From feature -> To feature
  typedef std::map<FeatureMapItem, unsigned int> FeatureOccurrences; // Feature map item -> occurrence count

  struct ControllerMapItem
  {
    std::string        fromController;
    std::string        toController;
    FeatureOccurrences featureMap;

    bool operator<(const ControllerMapItem& other) const
    {
      if (fromController < other.fromController) return true;
      if (fromController > other.fromController) return false;

      if (toController < other.toController) return true;
      if (toController > other.toController) return false;

      return false;
    }
  };

  typedef std::set<ControllerMapItem> ControllerMap;
}
