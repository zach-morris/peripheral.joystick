/*
 *      Copyright (C) 2014-2015 Garrett Brown
 *      Copyright (C) 2014-2015 Team XBMC
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

#include "kodi/kodi_peripheral_utils.hpp"
#include "platform/threads/mutex.h"

#include <vector>

namespace JOYSTICK
{
  class CJoystick;
  class IJoystickInterface;

  class IScannerCallback
  {
  public:
    virtual ~IScannerCallback(void) { }

    /*!
     * \brief Trigger a scan for joysticks
     */
    virtual void TriggerScan(void) = 0;
  };

  class CJoystickManager
  {
  private:
    CJoystickManager(void);

  public:
    static CJoystickManager& Get(void);
    virtual ~CJoystickManager(void) { Deinitialize(); }

    /*!
     * \brief Initialize the joystick manager
     *
     * \param scanner The callback used to trigger a scan
     */
    bool Initialize(IScannerCallback* scanner);

    /*!
     * \brief Deinitialize the joystick manager
     */
    void Deinitialize(void);

    /*!
     * \brief Scan the available interfaces for joysticks
     *
     * \param joysticks The discovered joysticks; must be deallocated
     *
     * \return true if the scan succeeded (even if there are no joysticks)
     */
    bool PerformJoystickScan(std::vector<CJoystick*>& joysticks);

    CJoystick* GetJoystick(unsigned int index) const;

    /*!
    * \brief Get all events that have occurred since the last call to GetEvents()
    */
    bool GetEvents(std::vector<ADDON::PeripheralEvent>& events);

    /*!
     * \brief Trigger a scan for joysticks through the callback
     */
    void TriggerScan(void);

  private:
    IScannerCallback*                m_scanner;
    std::vector<IJoystickInterface*> m_interfaces;
    std::vector<CJoystick*>          m_joysticks;
    unsigned int                     m_nextJoystickIndex;
    mutable PLATFORM::CMutex         m_interfacesMutex;
    mutable PLATFORM::CMutex         m_joystickMutex;
  };
}
