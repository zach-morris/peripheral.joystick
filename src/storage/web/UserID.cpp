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

#include "UserID.h"
#include "filesystem/FileUtils.h"

#include <fstream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>

using namespace JOYSTICK;

#define USER_ID_FILE   "special://userdata/addon_data/peripheral.joystick/random.txt"

#if defined(_WIN32)
  #define snprintf  _snprintf
#endif

bool CUserID::Load(const std::string& strUserPath)
{
  m_strUserId = LoadRandomNumber(strUserPath);

  return IsLoaded();
}

bool CUserID::IsLoaded(void) const
{
  return !m_strUserId.empty();
}

std::string CUserID::LoadRandomNumber(const std::string& strUserPath)
{
  const size_t userIdLength = 12;

  std::string strUserId;
  strUserId.reserve(userIdLength);

  FilePtr file = CFileUtils::OpenFile(USER_ID_FILE);
  if (file)
  {
    if (file->ReadFile(strUserId, userIdLength) != userIdLength)
    {
      file->Close();

      file = CFileUtils::OpenFileForWrite(USER_ID_FILE);

      if (file)
      {
        std::srand(std::time(NULL));

        char randNo[userIdLength + 1];
        snprintf(randNo, sizeof(randNo), "%04X%04X%04X", std::rand() % 0x10000,
                                                         std::rand() % 0x10000,
                                                         std::rand() % 0x10000);
        strUserId = randNo;

        file->Write(userIdLength, strUserId);
      }
    }
  }

  return strUserId;
}
