/*
 *      Copyright (C) 2014 Garrett Brown
 *      Copyright (C) 2014 Team XBMC
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
 */
#pragma once

#include <string>
#include <vector>

namespace JOYSTICK
{
  class CJoystick;

  class CJoystickInterface
  {
  public:
    CJoystickInterface(const std::string& strName);
    virtual ~CJoystickInterface(void) { }

    virtual bool Initialize(void) = 0;
    virtual void Deinitialize(void) = 0;

    bool ScanForJoysticks(std::vector<CJoystick*>& results);

    const std::string& Name(void) const { return m_strName; }

  protected:
    virtual bool PerformJoystickScan(std::vector<CJoystick*>& joysticks) = 0;

    // Helper functions to offer a buffer for device scanners that require static callbacks
    void AddScanResult(CJoystick* joystick);
    unsigned int ScanResultCount(void) const;
    void GetScanResults(std::vector<CJoystick*>& joysticks) const;
    void ClearScanResults(void);

  private:
    std::string             m_strName;
    std::vector<CJoystick*> m_scanResults;
  };
}
