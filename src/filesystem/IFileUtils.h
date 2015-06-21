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

#include <stdlib.h>
#include <string>
#include <time.h>

namespace JOYSTICK
{
  struct STAT_STRUCTURE
  {
    uint32_t        deviceId;         // ID of device containing file
    uint64_t        size;             // Total size, in bytes
    struct timespec accessTime;       // Time of last access
    struct timespec modificationTime; // Time of last modification
    struct timespec statusTime;       // Time of last status change
    bool            isDirectory;      // The stat url is a directory
    bool            isSymLink;        // The stat url is a symbolic link
    bool            isHidden;         // The file is hidden
  };

  class IFileUtils
  {
  public:
    virtual ~IFileUtils(void) { }

    virtual bool Exists(const std::string& url) = 0;

    virtual bool Stat(const std::string& url, STAT_STRUCTURE& buffer) = 0;

    virtual bool Rename(const std::string& url, const std::string& newUrl) = 0;

    virtual bool Delete(const std::string& url) = 0;

    virtual bool SetHidden(const std::string& url, bool bHidden) { return false; }
  };
}
