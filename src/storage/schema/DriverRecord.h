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

#include "kodi/kodi_peripheral_utils.hpp"

namespace JOYSTICK
{
  class CDriverRecord
  {
  public:
    CDriverRecord(void) { }
    CDriverRecord(const ADDON::Joystick& driverInfo);

    bool operator<(const CDriverRecord& rhs) const;

    bool IsValid(void) const;

    ADDON::Joystick& Properties(void) { return m_driverProperties; }
    const ADDON::Joystick& Properties(void) const { return m_driverProperties; }

    void MergeProperties(const CDriverRecord& record);

    // File name for this record, minus extension
    std::string RootFileName(void) const;

  private:
    ADDON::Joystick m_driverProperties;
  };
}
