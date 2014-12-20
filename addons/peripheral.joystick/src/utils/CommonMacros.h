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
#pragma once

#if !defined(ASSERT)
  #if defined(DEBUG) || defined(_DEBUG)
    #include <assert.h>
    #define ASSERT(x)  assert(x)
  #else
    #include "log/Log.h"
    #define ASSERT(x)  esyslog("FAILED ASSERT in %s line %s: %s", __FILE__, __LINE__, #x)
  #endif
#endif

#if !defined(SAFE_DELETE)
  #define SAFE_DELETE(x)  do { delete (x); (x) = NULL; } while (0)
#endif

#if !defined(SAFE_DELETE_ARRAY)
  #define SAFE_DELETE_ARRAY(x)  do { delete[] (x); (x) = NULL; } while (0)
#endif

#if !defined(SAFE_RELEASE)
  #define SAFE_RELEASE(p)  do { if(p) { (p)->Release(); (p)=NULL; } } while (0)
#endif
