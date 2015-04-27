project(peripheral.joystick)

cmake_minimum_required(VERSION 2.6)
enable_language(CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

include(CheckIncludeFiles)

# --- Add-on Dependencies ------------------------------------------------------

find_package(kodi REQUIRED)
find_package(kodiplatform REQUIRED)

include_directories(${INCLUDES}
                    ${PROJECT_SOURCE_DIR}/src
                    ${KODI_INCLUDE_DIR}
                    ${kodiplatform_INCLUDE_DIRS})

set(JOYSTICK_SOURCES src/addon.cpp
                     src/api/Joystick.cpp
                     src/api/JoystickInterface.cpp
                     src/api/JoystickManager.cpp
                     src/api/PeripheralScanner.cpp
                     src/devices/Buttons.cpp
                     src/devices/Device.cpp
                     src/devices/Devices.cpp
                     src/JoystickTranslator.cpp
                     src/log/Log.cpp
                     src/log/LogAddon.cpp
                     src/log/LogConsole.cpp)

check_include_files("syslog.h" HAVE_SYSLOG)

if(HAVE_SYSLOG)
  list(APPEND JOYSTICK_SOURCES src/log/LogSyslog.cpp)
endif()
                               
list(APPEND DEPLIBS ${kodiplatform_LIBRARIES})

# --- Cocoa --------------------------------------------------------------------

if("${CORE_SYSTEM_NAME}" STREQUAL "darwin")
  find_library(COCOA_LIBRARY Cocoa)

  add_definitions(-DHAVE_COCOA)

  list(APPEND JOYSTICK_SOURCES src/api/cocoa/JoystickCocoa.cpp
                               src/api/cocoa/JoystickInterfaceCocoa.cpp)

  list(APPEND DEPLIBS ${COCOA_LIBRARY})
endif()

# --- SDL ----------------------------------------------------------------------

find_package(Sdl)

if(SDL_FOUND)
  include_directories(${SDL_INCLUDE_DIRS})

  add_definitions(-DHAVE_SDL)

  list(APPEND JOYSTICK_SOURCES src/api/sdl/JoystickInterfaceSDL.cpp
                               src/api/sdl/JoystickSDL.cpp)

  list(APPEND DEPLIBS ${SDL_LIBRARIES})
endif()

# --- Linux Joystick API -------------------------------------------------------

check_include_files(linux/joystick.h HAVE_LINUX_JOYSTICK_H)

if(HAVE_LINUX_JOYSTICK_H)
  add_definitions(-DHAVE_LINUX_JOYSTICK)

  list(APPEND JOYSTICK_SOURCES src/api/linux/JoystickInterfaceLinux.cpp
                               src/api/linux/JoystickLinux.cpp)
endif()

# ------------------------------------------------------------------------------

build_addon(peripheral.joystick JOYSTICK DEPLIBS)

include(CPack)
