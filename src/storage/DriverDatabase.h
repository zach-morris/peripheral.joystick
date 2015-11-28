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

#include "storage/schema/DriverRecord.h"

#include "kodi/kodi_peripheral_utils.hpp"

#include <map>

namespace JOYSTICK
{
  class CDriverDatabase
  {
  public:
    CDriverDatabase(void) { }
    virtual ~CDriverDatabase(void) { }

    /*!
     * \brief Get the driver record associated with the given driver info
     * \param joystick The known driver info
     * \return The driver record
     *
     * If joystick is missing fields, the returned record
     */
    virtual bool GetDriverRecord(const ADDON::Joystick& joystick, CDriverRecord& record);

  protected:
    typedef std::map<CDriverRecord, CDriverRecord> DriverRecordMap; // Partial driver information -> full driver information

    DriverRecordMap m_driverRecords;
  };
}
