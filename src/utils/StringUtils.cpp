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

#include "StringUtils.h"

#include <algorithm>

using namespace JOYSTICK;

std::string StringUtils::MakeSafeUrl(const std::string& str)
{
  std::string safeUrl;

  safeUrl.reserve(str.size());

  std::transform(str.begin(), str.end(), std::back_inserter(safeUrl),
    [](char c)
    {
      if (('a' <= c && c <= 'z') ||
          ('A' <= c && c <= 'Z') ||
          ('0' <= c && c <= '9') ||
           c == '-' ||
           c == '.' ||
           c == '_' ||
           c == '~')
      {
        return c;
      }
      return '_';
    });

  return safeUrl;
}
