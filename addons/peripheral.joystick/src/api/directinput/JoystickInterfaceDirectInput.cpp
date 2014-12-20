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

#include "JoystickInterfaceDirectInput.h"
#include "JoystickDirectInput.h"
#include "api/JoystickTypes.h"
#include "log/Log.h"
#include "utils/CommonMacros.h"

// For getting the GUIDs of XInput devices
#include <wbemidl.h>
#include <oleauto.h>

#pragma comment(lib, "Dinput8.lib")
#pragma comment(lib, "dxguid.lib")

using namespace JOYSTICK;

HWND g_hWnd = NULL;  // TODO: https://stackoverflow.com/questions/6202547/win32-get-main-wnd-handle-of-application

CJoystickInterfaceDirectInput::CJoystickInterfaceDirectInput(void)
 : CJoystickInterface(INTERFACE_DIRECTINPUT),
   m_pDirectInput(NULL)
{
}

bool CJoystickInterfaceDirectInput::Initialize(void)
{
  HRESULT hr;

  hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<VOID**>(&m_pDirectInput), NULL);
  if (FAILED(hr))
  {
    esyslog("%s: Failed to create DirectInput", __FUNCTION__);
    return false;
  }

  return true;
}

void CJoystickInterfaceDirectInput::Deinitialize(void)
{
  ClearScanResults();

  SAFE_RELEASE(m_pDirectInput);
}

bool CJoystickInterfaceDirectInput::PerformJoystickScan(std::vector<CJoystick*>& joysticks)
{
  Deinitialize();

  HRESULT hr;

  hr = m_pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);
  if (FAILED(hr))
  {
    esyslog("%s: Joystick enumeration failed", __FUNCTION__);
    return false;
  }

  // TODO
  joysticks.insert(joysticks.end(), m_scanResults.begin(), m_scanResults.end());
  ClearScanResults();

  return true;
}

void CJoystickInterfaceDirectInput::AddScanResult(CJoystick* joystick)
{
  joystick->SetRequestedPlayer(m_scanResults.size() + 1);
  m_scanResults.push_back(joystick);
}

void CJoystickInterfaceDirectInput::ClearScanResults(void)
{
  for (std::vector<CJoystick*>::iterator it = m_scanResults.begin(); it != m_scanResults.end(); ++it)
    delete *it;
  m_scanResults.clear();
}

BOOL CALLBACK CJoystickInterfaceDirectInput::EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext)
{
  HRESULT hr;

  // Skip verified XInput devices
  if (IsXInputDevice(&pdidInstance->guidProduct))
    return DIENUM_CONTINUE;

  CJoystickInterfaceDirectInput* context = static_cast<CJoystickInterfaceDirectInput*>(pContext);

  LPDIRECTINPUTDEVICE8 pJoystick = NULL;

  // Obtain an interface to the enumerated joystick.
  hr = context->m_pDirectInput->CreateDevice(pdidInstance->guidInstance, &pJoystick, NULL);
  if (FAILED(hr) || pJoystick == NULL)
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

  CJoystick* joystick = new CJoystickDirectInput(pJoystick, context);
  joystick->SetName(pdidInstance->tszProductName ? pdidInstance->tszProductName : "");
  joystick->SetRequestedPlayer(0); // TODO
  joystick->SetButtonCount(diDevCaps.dwButtons);
  joystick->SetHatCount(diDevCaps.dwPOVs);
  joystick->SetAxisCount(diDevCaps.dwAxes);
  context->AddScanResult(joystick);

  return DIENUM_CONTINUE;
}

//-----------------------------------------------------------------------------
// Enum each PNP device using WMI and check each device ID to see if it contains
// "IG_" (ex. "VID_045E&PID_028E&IG_00"). If it does, then it's an XInput device.
// Unfortunately this information can not be found by just using DirectInput.
// See http://msdn.microsoft.com/en-us/library/windows/desktop/ee417014(v=vs.85).aspx
//-----------------------------------------------------------------------------
bool CJoystickInterfaceDirectInput::IsXInputDevice(const GUID* pGuidProductFromDirectInput)
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
