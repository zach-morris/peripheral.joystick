/*
 *      Copyright (C) 2014 Garrett Brown
 *      Copyright (C) 2014 Team XBMC
 *      Portions Copyright (C) 2013-2014 Lars Op den Kamp
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

#include "LogAddon.h"

#include "libXBMC_addon.h"

using namespace ADDON;
using namespace JOYSTICK;

void CLogAddon::Log(SYS_LOG_LEVEL level, const char* logline)
{
  addon_log_t loglevel;

  switch (level)
  {
  case SYS_LOG_ERROR:
    loglevel = LOG_ERROR;
    break;
  case SYS_LOG_INFO:
    loglevel = LOG_INFO;
    break;
  case SYS_LOG_DEBUG:
    loglevel = LOG_DEBUG;
    break;
  default:
    return;
  }

  if (m_frontend)
    m_frontend->Log(loglevel, logline);
}
