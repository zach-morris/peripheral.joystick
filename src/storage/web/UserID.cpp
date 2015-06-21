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

#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>

using namespace JOYSTICK;

#define USER_RANDOM_NO_FILE   "/random_number.txt"

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
  std::string randomNo;

  const std::string strPath = strUserPath + USER_RANDOM_NO_FILE;

  std::ifstream file(strPath.c_str());
  const bool bExists = (bool)file;

  if (bExists)
  {
    file >> randomNo;
  }
  else
  {
    std::srand(std::time(NULL));

    char randNo[13];
    std::snprintf(randNo, sizeof(randNo), "%04X%04X%04X", std::rand() % 0x10000,
                                                          std::rand() % 0x10000,
                                                          std::rand() % 0x10000);
    randomNo = randNo;

    std::ofstream ofile(strPath.c_str());
    ofile << randomNo;
  }

  return randomNo;
}
