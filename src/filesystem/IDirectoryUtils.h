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

namespace JOYSTICK
{
  class IDirectoryUtils
  {
  public:
    virtual ~IDirectoryUtils(void) { }

    /*!
     * \brief Create a directory
     * \param path Path to the directory
     * \return True if path is created, false otherwise
     */
    virtual bool Create(const std::string& path);

    /*!
     * \brief Check if a directory exists
     * \param path Directory to check
     * \return True if the directory exists, false otherwise
     */
    virtual bool Exists(const std::string& path);

    /*!
     * \brief Remove a directory
     * \param path The directory to remove
     * \return True if the directory was removed, false otherwise
     */
    virtual bool Remove(const std::string& path);
  };
}
