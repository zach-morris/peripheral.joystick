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
 *
 */

#include "JoystickDirectInput.h"
#include "api/JoystickManager.h"
#include "log/Log.h"

#include <dinputd.h>

// For getting the GUIDs of XInput devices
#include <wbemidl.h>
#include <oleauto.h>

#pragma comment(lib, "Dinput8.lib")
#pragma comment(lib, "dxguid.lib")

using namespace ADDON;
using namespace JOYSTICK;

#define MAX_AXISAMOUNT  32768
#define AXIS_MIN       -32768  /* minimum value for axis coordinate */
#define AXIS_MAX        32767  /* maximum value for axis coordinate */

#define JOY_POV_360  JOY_POVBACKWARD * 2
#define JOY_POV_NE   (JOY_POVFORWARD + JOY_POVRIGHT) / 2
#define JOY_POV_SE   (JOY_POVRIGHT + JOY_POVBACKWARD) / 2
#define JOY_POV_SW   (JOY_POVBACKWARD + JOY_POVLEFT) / 2
#define JOY_POV_NW   (JOY_POVLEFT + JOY_POV_360) / 2

#ifndef SAFE_RELEASE
  #define SAFE_RELEASE(p)   do { if(p) { (p)->Release(); (p)=NULL; } } while (0)
#endif

HWND g_hWnd = NULL;  // TODO: https://stackoverflow.com/questions/6202547/win32-get-main-wnd-handle-of-application

CJoystickDirectInput::CJoystickDirectInput(void)
 : m_pDirectInput(NULL)
{
}

void CJoystickDirectInput::Deinitialize(void)
{
  m_joysticks.clear();

  // Release any DirectInput objects
  SAFE_RELEASE(m_pDirectInput);
}

PERIPHERAL_ERROR CJoystickDirectInput::PerformJoystickScan(std::vector<JoystickConfiguration>& joysticks)
{
  Deinitialize();

  HRESULT hr;

  hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&m_pDirectInput, NULL);
  if (FAILED(hr))
  {
    esyslog("%s: Failed to create DirectInput", __FUNCTION__);
    return PERIPHERAL_ERROR_FAILED;
  }

  hr = m_pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);
  if (FAILED(hr))
  {
    esyslog("%s: Joystick enumeration failed", __FUNCTION__);
    return PERIPHERAL_ERROR_FAILED;
  }

  // Initialize axes
  for (std::vector<DirectInputJoystick>::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
  {
    // Enumerate the joystick objects. The callback function enabled user
    // interface elements for objects that are found, and sets the min/max
    // values properly for discovered axes.
    hr = it->m_joystickDevice->EnumObjects(EnumObjectsCallback, it->m_joystickDevice, DIDFT_ALL);
    if (FAILED(hr))
      break;

    joysticks.push_back(it->m_configuration);
  }

  if (FAILED(hr))
  {
    esyslog("%s: Failed to enumerate objects", __FUNCTION__);
    return PERIPHERAL_ERROR_FAILED;
  }

  return PERIPHERAL_NO_ERROR;
}

//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a
//       joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK CJoystickDirectInput::EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{
  LPDIRECTINPUTDEVICE8 pJoy = static_cast<LPDIRECTINPUTDEVICE8>(pContext);

  // For axes that are returned, set the DIPROP_RANGE property for the
  // enumerated axis in order to scale min/max values.
  if (pdidoi->dwType & DIDFT_AXIS)
  {
    DIPROPRANGE diprg;
    diprg.diph.dwSize       = sizeof(DIPROPRANGE);
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    diprg.diph.dwHow        = DIPH_BYID;
    diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
    diprg.lMin              = AXIS_MIN;
    diprg.lMax              = AXIS_MAX;

    // Set the range for the axis
    HRESULT hr = pJoy->SetProperty(DIPROP_RANGE, &diprg.diph);
    if (FAILED(hr))
      esyslog(__FUNCTION__" : Failed to set property on %s", pdidoi->tszName);
  }
  return DIENUM_CONTINUE;
}

BOOL CALLBACK CJoystickDirectInput::EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext)
{
  HRESULT hr;

  // Skip verified XInput devices
  if (IsXInputDevice(&pdidInstance->guidProduct))
    return DIENUM_CONTINUE;

  CJoystickDirectInput* context = static_cast<CJoystickDirectInput*>(pContext);

  LPDIRECTINPUTDEVICE8 pJoystick = NULL;

  // Obtain an interface to the enumerated joystick.
  hr = context->m_pDirectInput->CreateDevice(pdidInstance->guidInstance, &pJoystick, NULL);
  if (FAILED(hr))
  {
    esyslog("%s: Failed to CreateDevice: %s", __FUNCTION__, pdidInstance->tszProductName);
    return DIENUM_CONTINUE;
  }

  // Set the data format to "simple joystick" - a predefined data format.
  // A data format specifies which controls on a device we are interested in,
  // and how they should be reported. This tells DInput that we will be
  // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
  hr = pJoystick->SetDataFormat(&c_dfDIJoystick2);
  if (FAILED(hr))
  {
    esyslog("%s: Failed to SetDataFormat on: %s", __FUNCTION__, pdidInstance->tszProductName);
    return DIENUM_CONTINUE;
  }

  // Set the cooperative level to let DInput know how this device should
  // interact with the system and with other DInput applications.
  hr = pJoystick->SetCooperativeLevel(g_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
  if (FAILED(hr))
  {
    esyslog("%s: Failed to SetCooperativeLevel on: %s", __FUNCTION__, pdidInstance->tszProductName);
    return DIENUM_CONTINUE;
  }

  DIDEVCAPS diDevCaps;
  diDevCaps.dwSize = sizeof(DIDEVCAPS);
  hr = pJoystick->GetCapabilities(&diDevCaps);
  if (FAILED(hr))
  {
    esyslog("%s: Failed to GetCapabilities for: %s", __FUNCTION__, pdidInstance->tszProductName);
    return DIENUM_CONTINUE;
  }

  isyslog("%s: Enabled Joystick: \"%s\" (DirectInput)", __FUNCTION__, pdidInstance->tszProductName);
  isyslog("%s: Total Axes: %d Total Hats: %d Total Buttons: %d", __FUNCTION__,
    diDevCaps.dwAxes, diDevCaps.dwPOVs, diDevCaps.dwButtons);

  DirectInputJoystick joystick;
  joystick.m_joystickDevice = pJoystick;

  joystick.m_configuration.SetIndex(0); // Set by CJoystickManager
  joystick.m_configuration.SetRequestedPlayer(context->m_joysticks.size() + 1);
  joystick.m_configuration.SetName(pdidInstance->tszProductName ? pdidInstance->tszProductName : "");
  joystick.m_configuration.SetIconPath(""); // TODO

  for (unsigned int i = 0; i < diDevCaps.dwButtons; i++)
    joystick.m_configuration.ButtonIndexes().push_back(i);
  for (unsigned int i = 0; i < diDevCaps.dwPOVs; i++)
    joystick.m_configuration.HatIndexes().push_back(i);
  for (unsigned int i = 0; i < diDevCaps.dwAxes; i++)
    joystick.m_configuration.AxisIndexes().push_back(i);
  
  context->m_joysticks.push_back(joystick);

  return DIENUM_CONTINUE;
}

//-----------------------------------------------------------------------------
// Enum each PNP device using WMI and check each device ID to see if it contains
// "IG_" (ex. "VID_045E&PID_028E&IG_00"). If it does, then it's an XInput device.
// Unfortunately this information can not be found by just using DirectInput.
// See http://msdn.microsoft.com/en-us/library/windows/desktop/ee417014(v=vs.85).aspx
//-----------------------------------------------------------------------------
bool CJoystickDirectInput::IsXInputDevice(const GUID* pGuidProductFromDirectInput)
{
  IWbemLocator*         pIWbemLocator = NULL;
  IEnumWbemClassObject* pEnumDevices = NULL;
  IWbemClassObject*     pDevices[20] = { 0 };
  IWbemServices*        pIWbemServices = NULL;
  BSTR                  bstrNamespace = NULL;
  BSTR                  bstrDeviceID = NULL;
  BSTR                  bstrClassName = NULL;
  DWORD                 uReturned = 0;
  bool                  bIsXinputDevice = false;
  UINT                  iDevice = 0;
  VARIANT               var;
  HRESULT               hr;

  // CoInit if needed
  hr = CoInitialize(NULL);
  bool bCleanupCOM = SUCCEEDED(hr);

  try
  {
    // Create WMI
    hr = CoCreateInstance(__uuidof(WbemLocator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID*)&pIWbemLocator);
    if (FAILED(hr) || pIWbemLocator == NULL)
      throw hr;

    bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2");
    if (bstrNamespace == NULL)
      throw hr;

    bstrClassName = SysAllocString(L"Win32_PNPEntity");
    if (bstrClassName == NULL)
      throw hr;

    bstrDeviceID = SysAllocString(L"DeviceID");
    if (bstrDeviceID == NULL)
      throw hr;

    // Connect to WMI
    hr = pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, 0L, 0L, NULL, NULL, &pIWbemServices);
    if (FAILED(hr) || pIWbemServices == NULL)
      throw hr;

    // Switch security level to IMPERSONATE
    CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL,
      RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &pEnumDevices);
    if (FAILED(hr) || pEnumDevices == NULL)
      throw hr;

    // Loop over all devices
    do
    {
      // Get 20 at a time
      hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
      if (FAILED(hr))
        throw hr;

      for (iDevice = 0; iDevice < uReturned; iDevice++)
      {
        // Don't compare IDs if we already found our XInput device
        if (!bIsXinputDevice)
        {
          // For each device, get its device ID
          hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
          if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL)
          {
            // Check if the device ID contains "IG_". If it does, then it's an XInput
            // device. This information can not be found from DirectInput.
            if (wcsstr(var.bstrVal, L"IG_"))
            {
              // If it does, then get the VID/PID from var.bstrVal
              DWORD dwPid = 0;
              DWORD dwVid = 0;
              WCHAR *strVid = wcsstr(var.bstrVal, L"VID_");
              if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1)
                dwVid = 0;
              WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
              if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1)
                dwPid = 0;

              // Compare the VID/PID to the DInput device
              DWORD dwVidPid = MAKELONG(dwVid, dwPid);
              if (dwVidPid == pGuidProductFromDirectInput->Data1)
                bIsXinputDevice = true;
            }
          }
        }

        SAFE_RELEASE(pDevices[iDevice]);
      }
    } while (uReturned);
  }
  catch (HRESULT hr_error)
  {
    esyslog("%s: Error while testing for XInput device! hr=%ld", __FUNCTION__, hr_error);
  }

  if (bstrNamespace)
    SysFreeString(bstrNamespace);
  if (bstrDeviceID)
    SysFreeString(bstrDeviceID);
  if (bstrClassName)
    SysFreeString(bstrClassName);
  for (iDevice = 0; iDevice < 20; iDevice++)
    SAFE_RELEASE(pDevices[iDevice]);
  SAFE_RELEASE(pEnumDevices);
  SAFE_RELEASE(pIWbemLocator);
  SAFE_RELEASE(pIWbemServices);

  if (bCleanupCOM)
    CoUninitialize();

  return bIsXinputDevice;
}

bool CJoystickDirectInput::GetEvents(EventMap& events)
{
  /*
  for (std::vector<DirectInputJoystick>::iterator it = m_joysticks.begin(); it != m_joysticks.end(); ++it)
  {
    CJoystickState &state = InitialState();

    HRESULT hr;

    LPDIRECTINPUTDEVICE8 pJoy = m_joystickDevice;
    DIJOYSTATE2 js; // DInput joystick state

    hr = pJoy->Poll();

    if (FAILED(hr))
    {
      int i = 0;
      // DInput is telling us that the input stream has been interrupted. We
      // aren't tracking any state between polls, so we don't have any special
      // reset that needs to be done. We just re-acquire and try again 10 times.
      do
      {
        hr = pJoy->Acquire();
      } while (hr == DIERR_INPUTLOST && i++ < 10);

      // hr may be DIERR_OTHERAPPHASPRIO or other errors. This may occur when the
      // app is minimized or in the process of switching, so just try again later.
      return false;
    }

    // Get the input's device state
    hr = pJoy->GetDeviceState(sizeof(DIJOYSTATE2), &js);
    if (FAILED(hr))
      return false; // The device should have been acquired during the Poll()

    // Gamepad buttons
    for (unsigned int b = 0; b < state.buttons.size(); b++)
      state.buttons[b] = ((js.rgbButtons[b] & 0x80) ? 1 : 0);

    // Gamepad hats
    for (unsigned int h = 0; h < state.hats.size(); h++)
    {
      state.hats[h].Center();
      bool bCentered = ((js.rgdwPOV[h] & 0xFFFF) == 0xFFFF);
      if (!bCentered)
      {
        if ((JOY_POV_NW <= js.rgdwPOV[h] && js.rgdwPOV[h] <= JOY_POV_360) || js.rgdwPOV[h] <= JOY_POV_NE)
          state.hats[h][CJoystickHat::UP] = true;
        else if (JOY_POV_SE <= js.rgdwPOV[h] && js.rgdwPOV[h] <= JOY_POV_SW)
          state.hats[h][CJoystickHat::DOWN] = true;

        if (JOY_POV_NE <= js.rgdwPOV[h] && js.rgdwPOV[h] <= JOY_POV_SE)
          state.hats[h][CJoystickHat::RIGHT] = true;
        else if (JOY_POV_SW <= js.rgdwPOV[h] && js.rgdwPOV[h] <= JOY_POV_NW)
          state.hats[h][CJoystickHat::LEFT] = true;
      }
    }

    // Gamepad axes
    long amounts[] = { js.lX, js.lY, js.lZ, js.lRx, js.lRy, js.lRz };
    for (unsigned int a = 0; a < std::min(state.axes.size(), 6U); a++)
      state.SetAxis(a, amounts[a], MAX_AXISAMOUNT);

    UpdateState(state);
  }
  */
  return false;
}
