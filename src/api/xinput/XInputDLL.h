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

#include "p8-platform/threads/mutex.h"

#include <string>
#include <windows.h>
#include <Xinput.h>

// Missing from XInput API
#define XINPUT_GAMEPAD_GUIDE  0x0400

namespace JOYSTICK
{
  struct XINPUT_GAMEPAD_EX
  {
    WORD  wButtons;
    BYTE  bLeftTrigger;
    BYTE  bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
    DWORD dwPaddingReserved;
  };

  struct XINPUT_STATE_EX
  {
    DWORD             dwPacketNumber;
    XINPUT_GAMEPAD_EX Gamepad;
  };

  class CXInputDLL
  {
  private:
    CXInputDLL(void);

  public:
    static CXInputDLL& Get(void);
    virtual ~CXInputDLL(void) { Unload(); }

    bool Load(void);
    void Unload(void);

    // Available after library is loaded successfully
    const std::string& Version(void) const { return m_strVersion; }

    bool GetState(unsigned int controllerId, XINPUT_STATE_EX& state);
    bool SetState(unsigned int controllerId, XINPUT_VIBRATION& vibration);
    bool GetCapabilities(unsigned int controllerId, XINPUT_CAPABILITIES& caps);

  private:
    // Forward decl's for XInput API's we load dynamically and use if available
    // [in] Index of the gamer associated with the device
    // [out] Receives the current state
    typedef DWORD (WINAPI* FnXInputGetState)(DWORD dwUserIndex, XINPUT_STATE_EX* pState);

    // [in] Index of the gamer associated with the device
    // [in, out] The vibration information to send to the controller
    typedef DWORD (WINAPI* FnXInputSetState)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

    // [in] Index of the gamer associated with the device
    // [in] Input flags that identify the device type
    // [out] Receives the capabilities
    typedef DWORD (WINAPI* FnXInputGetCapabilities)(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities);

    HMODULE                 m_dll;
    std::string             m_strVersion;
    FnXInputGetState        m_getState;
    FnXInputSetState        m_setState;
    FnXInputGetCapabilities m_getCaps;
    P8PLATFORM::CMutex        m_mutex;
  };
}
