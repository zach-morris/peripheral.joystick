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
  class CSettings
  {
  private:
    CSettings(void);

  public:
    static CSettings& Get(void);

    void SetSetting(const std::string& strName, const void* value);

    bool IsInitialized(void) const { return m_bInitialized; }

    /*!
     * \brief The analog stick deadzone
     *
     * This is applied to each axis. Axis is scaled appropriately, so position
     * is continuous from -1.0 to 1.0:
     *
     *            |    / 1.0
     *            |   /
     *          __|__/
     *         /  |
     *        /   |--| Deadzone
     *  -1.0 /    |
     */
    float Deadzone(void) const { return m_deadzone; }

  private:
    bool        m_bInitialized;
    float       m_deadzone;
  };
}
