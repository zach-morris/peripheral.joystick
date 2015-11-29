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
  class CDevice : public ADDON::Joystick
  {
  public:
    CDevice(void) { }
    CDevice(const ADDON::Joystick& joystick);

    /*!
     * \brief Define a comparison operator for driver records
     */
    bool operator==(const CDevice& rhs) const;
    bool operator!=(const CDevice& rhs) const { return !(*this == rhs); }

    /*!
     * \brief Define a total order for driver records
     */
    bool operator<(const CDevice& rhs) const;

    /*!
     * \brief Define a similarity metric for driver records
     */
    bool SimilarTo(const CDevice& rhs) const;

    /*!
     * \brief Define a validity test for driver records
     */
    bool IsValid(void) const;

    /*!
     * \brief Merge valid (known) properties of given record into this record
     */
    void MergeProperties(const CDevice& record);

    /*!
     * \brief Build a filename out of the record's properties
     * \return A sensible filename, lacking an extension (which can be added by
     *         the caller)
     *
     * The filename is derived from all driver properties. This avoids
     * collisions in the storage backend. An example filename is:
     *
     * Gamepad_F310_v1133_p1133_15b_6a
     *
     * Currently, the properties are concatenated by underscores. This may
     * change in the future by simply appending a hash of the data to the
     * driver name.
     */
    std::string RootFileName(void) const;

    /*!
     * \brief Get a folder name to avoid clashes between different joystick drivers
     * \return A folder name based on the provider
     */
    const std::string& Folder(void) const { return Provider(); }

    /*!
     * \brief Build a complete path to this joystick's button maps
     * \return A path to the button maps for this driver info
     */
    std::string BuildPath(const std::string& strBaseDir, const std::string& strExtension) const;
  };
}
