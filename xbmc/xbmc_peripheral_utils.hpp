/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
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

#include "xbmc_peripheral_types.h"

#include <string>
#include <string.h>
#include <vector>

#ifndef SAFE_DELETE
  #define SAFE_DELETE(x)  do { delete x; x = NULL; } while (0)
#endif

#ifndef SAFE_DELETE_ARRAY
  #define SAFE_DELETE_ARRAY(x)  do { delete[] x; x = NULL; } while (0)
#endif

namespace ADDON
{
  class PeripheralScanResult
  {
  public:
    PeripheralScanResult(void) { }

    PeripheralScanResult(const PERIPHERAL_SCAN_RESULT& scanResult)
    : m_type(scanResult.type),
      m_index(scanResult.peripheral_index),
      m_strName(scanResult.name ? scanResult.name : ""),
      m_vendorId(scanResult.vendor_id),
      m_productId(scanResult.product_id)
    {
    }

    PERIPHERAL_TYPE    Type(void) const      { return m_type; }
    unsigned int       Index(void) const     { return m_index; }
    const std::string& Name(void) const      { return m_strName; }
    unsigned int       VendorID(void) const  { return m_vendorId; }
    unsigned int       ProductID(void) const { return m_productId; }

    void SetType(PERIPHERAL_TYPE type)        { m_type      = type; }
    void SetIndex(unsigned int index)         { m_index     = index; }
    void SetName(const std::string& strName)  { m_strName   = strName; }
    void SetVendorID(unsigned int vendorId)   { m_vendorId  = vendorId; }
    void SetProductID(unsigned int productId) { m_productId = productId; }

    void ToStruct(PERIPHERAL_SCAN_RESULT& scanResult) const
    {
      scanResult.type             = m_type;
      scanResult.peripheral_index = m_index;
      scanResult.name             = new char[m_strName.size() + 1];
      scanResult.vendor_id        = m_vendorId;
      scanResult.product_id       = m_productId;

      strcpy(scanResult.name, m_strName.c_str());
    }

    static void ToStructs(const std::vector<PeripheralScanResult>& results, PERIPHERAL_SCAN_RESULT** resultStructs)
    {
      if (!resultStructs)
        return;

      if (results.empty())
      {
        *resultStructs = NULL;
      }
      else
      {
        *resultStructs = new PERIPHERAL_SCAN_RESULT[results.size()];
        for (unsigned int i = 0; i < results.size(); i++)
          results.at(i).ToStruct((*resultStructs)[i]);
      }
    }

    static void FreeStruct(PERIPHERAL_SCAN_RESULT& scanResult)
    {
      SAFE_DELETE_ARRAY(scanResult.name);
    }

    static void FreeStructs(unsigned int resultCount, PERIPHERAL_SCAN_RESULT* results)
    {
      if (results)
      {
        for (unsigned int i = 0; i < resultCount; i++)
          FreeStruct(results[i]);
      }
      SAFE_DELETE_ARRAY(results);
    }

  private:
    PERIPHERAL_TYPE m_type;
    unsigned int    m_index;
    std::string     m_strName;
    unsigned int    m_vendorId;
    unsigned int    m_productId;
  };

  class JoystickButton
  {
  public:
    JoystickButton(void) : m_id(), m_type() { }

    JoystickButton(JOYSTICK_ID id, JOYSTICK_BUTTON_TYPE type, const std::string& strLabel)
    : m_id(id), 
      m_type(type), 
      m_strLabel(strLabel) 
    {
    }

    JoystickButton(const JOYSTICK_BUTTON& button)
    : m_id(button.id),
      m_type(button.type), 
      m_strLabel(button.label ? button.label : "")
    {
    }

    JOYSTICK_ID          ID(void) const    { return m_id; }
    JOYSTICK_BUTTON_TYPE Type(void) const  { return m_type; }
    const std::string&   Label(void) const { return m_strLabel; }

    void SetID(JOYSTICK_ID id)                 { m_id = id; }
    void SetType(JOYSTICK_BUTTON_TYPE type)    { m_type = type; }
    void SetLabel(const std::string& strLabel) { m_strLabel = strLabel; }

    void ToStruct(JOYSTICK_BUTTON& button)
    {
      button.id    = m_id;
      button.type  = m_type;
      button.label = new char[m_strLabel.size() + 1];

      strcpy(button.label, m_strLabel.c_str());
    }

    static void FreeStruct(JOYSTICK_BUTTON& button)
    {
      SAFE_DELETE_ARRAY(button.label);
    }

  private:
    JOYSTICK_ID          m_id;
    JOYSTICK_BUTTON_TYPE m_type;
    std::string          m_strLabel;
  };

  class Joystick
  {
  public:
    Joystick(void) : m_requestedPlayer(0), m_buttonCount(0), m_hatCount(0), m_axisCount(0) { }

    Joystick(JOYSTICK_INFO& info)
    : m_strName(info.name),
      m_requestedPlayer(info.requested_player_num),
      m_buttonCount(info.virtual_layout.button_count),
      m_hatCount(info.virtual_layout.hat_count),
      m_axisCount(info.virtual_layout.axis_count)
    {
      if (info.physical_layout.buttons)
      {
        for (unsigned int i = 0; i < info.physical_layout.button_count; i++)
          m_buttons.push_back(JoystickButton(info.physical_layout.buttons[i]));
      }
    }

    virtual ~Joystick(void) { }

    const std::string& Name(void) const            { return m_strName; }
    unsigned int       RequestedPlayer(void) const { return m_requestedPlayer; }
    unsigned int       ButtonCount(void) const     { return m_buttonCount; }
    unsigned int       HatCount(void) const        { return m_hatCount; }
    unsigned int       AxisCount(void) const       { return m_axisCount; }

    const std::vector<JoystickButton>& Buttons(void) const { return m_buttons; }

    void SetName(const std::string& strName)              { m_strName         = strName; }
    void SetRequestedPlayer(unsigned int requestedPlayer) { m_requestedPlayer = requestedPlayer; }
    void SetButtonCount(unsigned int buttonCount)         { m_buttonCount     = buttonCount; }
    void SetHatCount(unsigned int hatCount)               { m_hatCount        = hatCount; }
    void SetAxisCount(unsigned int axisCount)             { m_axisCount       = axisCount; }

    std::vector<JoystickButton>& Buttons(void) { return m_buttons; }

    void ToStruct(JOYSTICK_INFO& info)
    {
      info.name                               = new char[m_strName.size() + 1];
      info.requested_player_num               = m_requestedPlayer;
      info.virtual_layout.button_count        = m_buttonCount;
      info.virtual_layout.hat_count           = m_hatCount;
      info.virtual_layout.axis_count          = m_axisCount;
      info.physical_layout.button_count       = m_buttons.size();
      info.physical_layout.buttons            = NULL;

      strcpy(info.name, m_strName.c_str());

      if (!m_buttons.empty())
      {
        info.physical_layout.buttons = new JOYSTICK_BUTTON[m_buttons.size()];
        for (unsigned int i = 0; i < m_buttons.size(); i++)
          m_buttons[i].ToStruct(info.physical_layout.buttons[i]);
      }
    }

    static void FreeStruct(JOYSTICK_INFO& info)
    {
      SAFE_DELETE_ARRAY(info.name);

      if (info.physical_layout.buttons)
      {
        for (unsigned int i = 0; i < info.physical_layout.button_count; i++)
          JoystickButton::FreeStruct(info.physical_layout.buttons[i]);
      }
      SAFE_DELETE_ARRAY(info.physical_layout.buttons);
    }

  private:
    std::string  m_strName;
    unsigned int m_requestedPlayer;
    unsigned int m_buttonCount;
    unsigned int m_hatCount;
    unsigned int m_axisCount;

    std::vector<JoystickButton> m_buttons;
  };

  class PeripheralEvent
  {
  public:
    PeripheralEvent(void) : m_index(0), m_type(), m_data(NULL) { }

    PeripheralEvent(const PERIPHERAL_EVENT& event)
    : m_index(event.peripheral_index),
      m_type(event.type),
      m_data(NULL)
    {
      SetData(event.event_data);
    }

    ~PeripheralEvent(void) { ClearData(); }

    unsigned int        PeripheralIndex(void) const { return m_index; }
    JOYSTICK_EVENT_TYPE Type(void) const            { return m_type; }

    template <typename EVENT_TYPE>
    const EVENT_TYPE& EventAsType(void) const
    {
      static const EVENT_TYPE emptyEvent = { };
      return m_data ? *static_cast<const EVENT_TYPE*>(m_data) : emptyEvent;
    }

    void ClearData(void)
    {
      PERIPHERAL_EVENT event = { m_index, m_type, m_data };
      FreeStruct(event);
    }

    void SetData(void* event)
    {
      ClearData();

      if (!event)
        return;

      switch (m_type)
      {
      case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
        m_data = new JOYSTICK_EVENT_VIRTUAL_BUTTON(*static_cast<JOYSTICK_EVENT_VIRTUAL_BUTTON*>(event));
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:
        m_data = new JOYSTICK_EVENT_VIRTUAL_HAT(*static_cast<JOYSTICK_EVENT_VIRTUAL_HAT*>(event));
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:
        m_data = new JOYSTICK_EVENT_VIRTUAL_AXIS(*static_cast<JOYSTICK_EVENT_VIRTUAL_AXIS*>(event));
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
        m_data = new JOYSTICK_EVENT_BUTTON_DIGITAL(*static_cast<JOYSTICK_EVENT_BUTTON_DIGITAL*>(event));
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:
        m_data = new JOYSTICK_EVENT_BUTTON_ANALOG(*static_cast<JOYSTICK_EVENT_BUTTON_ANALOG*>(event));
        break;
      case JOYSTICK_EVENT_TYPE_ANALOG_STICK:
        m_data = new JOYSTICK_EVENT_ANALOG_STICK(*static_cast<JOYSTICK_EVENT_ANALOG_STICK*>(event));
        break;
      case JOYSTICK_EVENT_TYPE_ACCELEROMETER:
        m_data = new JOYSTICK_EVENT_ACCELEROMETER(*static_cast<JOYSTICK_EVENT_ACCELEROMETER*>(event));
        break;
      case JOYSTICK_EVENT_TYPE_NONE:
      default:
        break;
      }
    }

    void ToStruct(PERIPHERAL_EVENT& event) const
    {
      event.peripheral_index = m_index;
      event.type             = m_type;
      event.event_data       = NULL;

      switch (m_type)
      {
      case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
        event.event_data = new JOYSTICK_EVENT_VIRTUAL_BUTTON(EventAsType<JOYSTICK_EVENT_VIRTUAL_BUTTON>());
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:
        event.event_data = new JOYSTICK_EVENT_VIRTUAL_HAT(EventAsType<JOYSTICK_EVENT_VIRTUAL_HAT>());
        break;
      case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:
        event.event_data = new JOYSTICK_EVENT_VIRTUAL_AXIS(EventAsType<JOYSTICK_EVENT_VIRTUAL_AXIS>());
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
        event.event_data = new JOYSTICK_EVENT_BUTTON_DIGITAL(EventAsType<JOYSTICK_EVENT_BUTTON_DIGITAL>());
        break;
      case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:
        event.event_data = new JOYSTICK_EVENT_BUTTON_ANALOG(EventAsType<JOYSTICK_EVENT_BUTTON_ANALOG>());
        break;
      case JOYSTICK_EVENT_TYPE_ANALOG_STICK:
        event.event_data = new JOYSTICK_EVENT_ANALOG_STICK(EventAsType<JOYSTICK_EVENT_ANALOG_STICK>());
        break;
      case JOYSTICK_EVENT_TYPE_ACCELEROMETER:
        event.event_data = new JOYSTICK_EVENT_ACCELEROMETER(EventAsType<JOYSTICK_EVENT_ACCELEROMETER>());
        break;
      case JOYSTICK_EVENT_TYPE_NONE:
      default:
        break;
      }
    }

    static void ToStructs(const std::vector<PeripheralEvent>& events, PERIPHERAL_EVENT** eventStructs)
    {
      if (!eventStructs)
        return;

      if (events.empty())
      {
        *eventStructs = NULL;
      }
      else
      {
        *eventStructs = new PERIPHERAL_EVENT[events.size()];
        for (unsigned int i = 0; i < events.size(); i++)
          events.at(i).ToStruct((*eventStructs)[i]);
      }
    }

    static void FreeStruct(PERIPHERAL_EVENT& event)
    {
      switch (event.type)
      {
        case JOYSTICK_EVENT_TYPE_VIRTUAL_BUTTON:
          delete static_cast<JOYSTICK_EVENT_VIRTUAL_BUTTON*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_VIRTUAL_HAT:
          delete static_cast<JOYSTICK_EVENT_VIRTUAL_HAT*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_VIRTUAL_AXIS:
          delete static_cast<JOYSTICK_EVENT_VIRTUAL_AXIS*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_BUTTON_DIGITAL:
          delete static_cast<JOYSTICK_EVENT_BUTTON_DIGITAL*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_BUTTON_ANALOG:
          delete static_cast<JOYSTICK_EVENT_BUTTON_ANALOG*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_ANALOG_STICK:
          delete static_cast<JOYSTICK_EVENT_ANALOG_STICK*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_ACCELEROMETER:
          delete static_cast<JOYSTICK_EVENT_ACCELEROMETER*>(event.event_data);
          break;
        case JOYSTICK_EVENT_TYPE_NONE:
        default:
          break;
      }
      event.event_data = NULL;
    }

    static void FreeStructs(unsigned int eventCount, PERIPHERAL_EVENT* events)
    {
      if (events)
      {
        for (unsigned int i = 0; i < eventCount; i++)
          FreeStruct(events[i]);
      }
      SAFE_DELETE_ARRAY(events);
    }

  private:
    unsigned int        m_index;
    JOYSTICK_EVENT_TYPE m_type;
    void*               m_data;
  };
}
