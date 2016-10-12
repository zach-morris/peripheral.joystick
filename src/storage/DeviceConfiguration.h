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
#pragma once

#include "StorageTypes.h"

namespace JOYSTICK
{
  class CDevice;

  class CDeviceConfiguration
  {
  public:
    CDeviceConfiguration(const CDevice* parent);
    ~CDeviceConfiguration(void) = default;

    void Reset(void);

    const AxisConfiguration& Axes(void) const { return m_axes; }

    bool GetAxis(unsigned int axisIndex, AxisProperties& axisProps) const;

    void LoadAxis(unsigned int axisIndex);

    void SetAxis(const AxisProperties& axisProps);

  private:
    // Construction parameters
    const CDevice* const m_parent;

    // Configuration parameters
    AxisConfiguration m_axes;
  };
}
