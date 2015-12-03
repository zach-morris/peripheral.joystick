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

#include "Device.h"
#include "utils/StringUtils.h"

#include <algorithm>
#include <sstream>

using namespace JOYSTICK;

CDevice::CDevice(const ADDON::Joystick& joystick) :
  ADDON::Joystick(joystick)
{
}

bool CDevice::operator==(const CDevice& rhs) const
{
  return Name() == rhs.Name() &&
         Provider() == rhs.Provider() &&
         VendorID() == rhs.VendorID() &&
         ProductID() == rhs.ProductID() &&
         ButtonCount() == rhs.ButtonCount() &&
         HatCount() == rhs.HatCount() &&
         AxisCount() == rhs.AxisCount();
}

bool CDevice::operator<(const CDevice& rhs) const
{
  if (Name() < rhs.Name()) return true;
  if (Name() > rhs.Name()) return false;

  if (Provider() < rhs.Provider()) return true;
  if (Provider() > rhs.Provider()) return false;

  if (VendorID() < rhs.VendorID()) return true;
  if (VendorID() > rhs.VendorID()) return false;

  if (ProductID() < rhs.ProductID()) return true;
  if (ProductID() > rhs.ProductID()) return false;

  if (ButtonCount() < rhs.ButtonCount()) return true;
  if (ButtonCount() > rhs.ButtonCount()) return false;

  if (HatCount() < rhs.HatCount()) return true;
  if (HatCount() > rhs.HatCount()) return false;

  if (AxisCount() < rhs.AxisCount()) return true;
  if (AxisCount() > rhs.AxisCount()) return false;

  return false;
}

bool CDevice::SimilarTo(const CDevice& other) const
{
  if (Provider() != other.Provider())
    return false;

  if (!Name().empty() && !other.Name().empty())
  {
    if (Name() != other.Name())
      return false;
  }

  if (IsVidPidKnown() && other.IsVidPidKnown())
  {
    if (VendorID() != other.VendorID() ||
        ProductID() != other.ProductID())
    {
      return false;
    }
  }

  if (AreElementCountsKnown() && other.AreElementCountsKnown())
  {
    if (ButtonCount() != other.ButtonCount() ||
        HatCount() != other.HatCount() ||
        AxisCount() != other.AxisCount())
    {
      return false;
    }
  }

  return true;
}

bool CDevice::IsValid(void) const
{
  return !Name().empty() &&
         !Provider().empty();
}

void CDevice::MergeProperties(const CDevice& record)
{
  if (!record.Name().empty())
    SetName(record.Name());

  if (!record.Provider().empty())
    SetProvider(record.Provider());

  if (record.IsVidPidKnown())
  {
    SetVendorID(record.VendorID());
    SetProductID(record.ProductID());
  }

  if (record.AreElementCountsKnown())
  {
    SetButtonCount(record.ButtonCount());
    SetHatCount(record.HatCount());
    SetAxisCount(record.AxisCount());
  }
}

std::string CDevice::RootFileName(void) const
{
  std::string baseFilename = StringUtils::MakeSafeUrl(Name());

  // Combine successive runs of underscores (fits more information in smaller
  // space)
  baseFilename.erase(std::unique(baseFilename.begin(), baseFilename.end(),
    [](char a, char b)
    {
      return a == '_' && b == '_';
    }), baseFilename.end());

  // Limit filename to a sane number of characters.
  if (baseFilename.length() > 50)
    baseFilename.erase(baseFilename.begin() + 50, baseFilename.end());

  // Trim trailing underscores left over from chopping the string
  baseFilename = StringUtils::Trim(baseFilename, "_");

  // Append remaining properties
  std::stringstream filename;

  filename << baseFilename;
  if (IsVidPidKnown())
  {
    filename << "_v" << VendorID();
    filename << "_p" << ProductID();
  }
  if (ButtonCount() != 0)
    filename << "_" << ButtonCount() << "b";
  if (HatCount() != 0)
    filename << "_" << HatCount() << "h";
  if (AxisCount() != 0)
    filename << "_" << AxisCount() << "a";

  return filename.str();
}
