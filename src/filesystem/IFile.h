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

#include <stdint.h>
#include <string>

namespace JOYSTICK
{
  // File flags from File.h of Kodi
  enum READ_FLAG
  {
    // Perform a normal read
    READ_FLAG_NONE    = 0x00,
    // Indicate that caller can handle truncated reads, where function returns before entire buffer has been filled
    READ_TRUNCATED    = 0x01,
    // Indicate that that caller support read in the minimum defined chunk size, this disables internal cache then
    READ_CHUNKED      = 0x02,
    // Use cache to access this file
    READ_CACHED       = 0x04,
    // Open without caching. regardless to file type.
    READ_NO_CACHE     = 0x08,
    // Calcuate bitrate for file while reading
    READ_BITRATE      = 0x10,
    // Indicate the caller will seek between multiple streams in the file frequently
    READ_MULTI_STREAM = 0x20,
  };

  inline READ_FLAG operator|(READ_FLAG lhs, READ_FLAG rhs)
  {
    return static_cast<READ_FLAG>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
  }

  // Seek flags from stdio.h
  enum SEEK_FLAG
  {
    SEEK_FLAG_SET = 0, // Beginning of file
    SEEK_FLAG_CUR = 1, // Current position of the file pointer
    SEEK_FLAG_END = 2, // End of file Implementation not required
  };

  class IFile
  {
  public:
    virtual ~IFile(void) { }

    virtual bool Open(const std::string& url, READ_FLAG flags = READ_FLAG_NONE) = 0;

    virtual bool OpenForWrite(const std::string& url, bool bOverWrite = false) = 0;

    virtual int64_t Read(uint64_t byteCount, std::string& buffer) = 0;

    virtual int64_t ReadLine(std::string& buffer) = 0;

    virtual int64_t ReadFile(std::string& buffer, const uint64_t maxBytes = 0) = 0;

    virtual int64_t Write(uint64_t byteCount, const std::string& buffer) = 0;

    virtual void Flush(void) { }

    virtual int64_t Seek(int64_t filePosition, SEEK_FLAG whence = SEEK_FLAG_SET) = 0;

    virtual int64_t Truncate(uint64_t size) = 0;

    virtual int64_t GetPosition(void) = 0;

    virtual int64_t GetLength(void) = 0;

    virtual void Close(void) = 0;
  };
}
