/*
 *      Copyright (C) 2014-2015 Garrett Brown
 *      Copyright (C) 2014-2015 Team XBMC
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

#include "storage/ButtonMap.h"
#include "storage/ButtonMapTypes.h"
#include "storage/Device.h"

#include "platform/threads/mutex.h"

#include <map>
#include <string>

namespace JOYSTICK
{
  class CDeviceDatabase;

  class CDatabase
  {
  public:
    CDatabase(void);

    virtual ~CDatabase(void) { }

    virtual bool Enable(void);

    virtual void Disable(void);

    virtual bool IsEnabled(void) const { return m_bEnabled; }

    virtual bool GetFeatures(const CDevice& driverInfo, const std::string& controllerId,
                             FeatureVector& features);

    virtual bool MapFeature(const CDevice& driverInfo, const std::string& controllerId,
                            const ADDON::JoystickFeature& feature);

  protected:
    typedef std::string                        ControllerID;
    typedef std::map<ControllerID, CButtonMap> ButtonMaps;
    typedef std::map<CDevice, ButtonMaps>      Records;

    Records m_records;

  public: // TODO
    CDeviceDatabase* m_driverDatabase; // Must be set by derived class

  private:
    bool m_bEnabled;
    PLATFORM::CMutex m_mutex;
  };
}
