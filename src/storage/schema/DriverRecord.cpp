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

#include "DriverRecord.h"
#include "utils/StringUtils.h"

#include <sstream>

using namespace JOYSTICK;

CDriverRecord::CDriverRecord(const ADDON::Joystick& driverInfo)
 : m_driverProperties(driverInfo)
{
}

bool CDriverRecord::operator<(const CDriverRecord& rhs) const
{
  if (m_driverProperties.Name() < rhs.m_driverProperties.Name()) return true;
  if (m_driverProperties.Name() > rhs.m_driverProperties.Name()) return false;

  if (m_driverProperties.Provider() < rhs.m_driverProperties.Provider()) return true;
  if (m_driverProperties.Provider() > rhs.m_driverProperties.Provider()) return false;

  if (m_driverProperties.VendorID() < rhs.m_driverProperties.VendorID()) return true;
  if (m_driverProperties.VendorID() > rhs.m_driverProperties.VendorID()) return false;

  if (m_driverProperties.ProductID() < rhs.m_driverProperties.ProductID()) return true;
  if (m_driverProperties.ProductID() > rhs.m_driverProperties.ProductID()) return false;

  if (m_driverProperties.ButtonCount() < rhs.m_driverProperties.ButtonCount()) return true;
  if (m_driverProperties.ButtonCount() > rhs.m_driverProperties.ButtonCount()) return false;

  if (m_driverProperties.HatCount() < rhs.m_driverProperties.HatCount()) return true;
  if (m_driverProperties.HatCount() > rhs.m_driverProperties.HatCount()) return false;

  if (m_driverProperties.AxisCount() < rhs.m_driverProperties.AxisCount()) return true;
  if (m_driverProperties.AxisCount() > rhs.m_driverProperties.AxisCount()) return false;

  return false;
}

bool CDriverRecord::IsValid(void) const
{
  return !m_driverProperties.Name().empty() &&
         !m_driverProperties.Provider().empty();
}

void CDriverRecord::MergeProperties(const CDriverRecord& record)
{
  if (!record.m_driverProperties.Name().empty())
    m_driverProperties.SetName(record.m_driverProperties.Name());

  if (!record.m_driverProperties.Provider().empty())
    m_driverProperties.SetProvider(record.m_driverProperties.Provider());

  if (record.m_driverProperties.IsVidPidKnown())
  {
    m_driverProperties.SetVendorID(record.m_driverProperties.VendorID());
    m_driverProperties.SetProductID(record.m_driverProperties.ProductID());
  }

  if (record.m_driverProperties.ButtonCount() != 0 ||
      record.m_driverProperties.HatCount() != 0 ||
      record.m_driverProperties.AxisCount() != 0)
  {
    m_driverProperties.SetButtonCount(record.m_driverProperties.ButtonCount());
    m_driverProperties.SetHatCount(record.m_driverProperties.HatCount());
    m_driverProperties.SetAxisCount(record.m_driverProperties.AxisCount());
  }
}

std::string CDriverRecord::RootFileName(void) const
{
  std::string baseFilename = StringUtils::MakeSafeUrl(m_driverProperties.Name());

  // TODO: Combine successive runs of underscores (fits more information in
  // smaller space)

  // Limit filename to a sane number of characters.
  if (baseFilename.length() > 40)
    baseFilename.erase(baseFilename.begin() + 40, baseFilename.end());

  // Trim trailing underscores left over from chopping the string
  baseFilename = StringUtils::Trim(baseFilename, "_");

  // Append remaining properties
  std::stringstream filename;

  filename << baseFilename;
  if (m_driverProperties.IsVidPidKnown())
  {
    filename << "_v" << m_driverProperties.VendorID();
    filename << "_p" << m_driverProperties.ProductID();
  }
  if (m_driverProperties.ButtonCount() != 0)
    filename << "_" << m_driverProperties.ButtonCount() << "b";
  if (m_driverProperties.HatCount() != 0)
    filename << "_" << m_driverProperties.HatCount() << "h";
  if (m_driverProperties.AxisCount() != 0)
    filename << "_" << m_driverProperties.AxisCount() << "a";

  return filename.str();
}
