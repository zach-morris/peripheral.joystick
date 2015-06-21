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

#include "FileUtils.h"
#include "filesystem/vfs/VFSFile.h"
#include "filesystem/vfs/VFSFileUtils.h"

#include "kodi/libXBMC_addon.h"

using namespace JOYSTICK;

bool CFileUtils::Initialize(ADDON::CHelper_libXBMC_addon* frontend)
{
  m_frontend = frontend;

  return true;
}

bool CFileUtils::Exists(const std::string& url)
{
  // Create file utils
  FileUtilsPtr fileUtils = CreateFileUtils(url);
  if (fileUtils)
    return fileUtils->Exists(url);

  return false;
}

bool CFileUtils::Stat(const std::string& url, STAT_STRUCTURE& buffer)
{
  // Create file utils
  FileUtilsPtr fileUtils = CreateFileUtils(url);
  if (fileUtils)
    return fileUtils->Stat(url, buffer);

  return false;
}

FilePtr CFileUtils::OpenFile(const std::string& url, READ_FLAG flags /* = READ_FLAG_NONE */)
{
  // Create file
  FilePtr file = CreateFile(url);
  if (file && file->Open(url, flags))
    return file;

  return FilePtr();
}

FilePtr CFileUtils::OpenFileForWrite(const std::string& url, bool bOverWrite /* = false */)
{
  // Create file
  FilePtr file = CreateFile(url);
  if (file && file->OpenForWrite(url, bOverWrite))
    return file;

  return FilePtr();
}

bool CFileUtils::Rename(const std::string& url, const std::string& newUrl)
{
  // Create file utils
  FileUtilsPtr fileUtils = CreateFileUtils(url);
  if (fileUtils)
    return fileUtils->Rename(url, newUrl);

  return false;
}

bool CFileUtils::Delete(const std::string& url)
{
  // Create file utils
  FileUtilsPtr fileUtils = CreateFileUtils(url);
  if (fileUtils)
    return fileUtils->Delete(url);

  return false;
}

bool CFileUtils::SetHidden(const std::string& url, bool bHidden)
{
  // Create file utils
  FileUtilsPtr fileUtils = CreateFileUtils(url);
  if (fileUtils)
    return fileUtils->SetHidden(url, bHidden);

  return false;
}

FilePtr CFileUtils::CreateFile(const std::string& url)
{
  FilePtr file;

  if (m_frontend)
    file = FilePtr(new CVFSFile(m_frontend));

  return file;
}

FileUtilsPtr CFileUtils::CreateFileUtils(const std::string& url)
{
  FileUtilsPtr fileUtils;

  if (m_frontend)
    fileUtils = FileUtilsPtr(new CVFSFileUtils(m_frontend));

  return fileUtils;
}
