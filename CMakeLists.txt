project(peripheral.joystick)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(kodi REQUIRED)
find_package(kodiplatform REQUIRED)
find_package(Sdl)

include_directories(${kodiplatform_INCLUDE_DIRS}
                    ${SDL_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/src)

set(DEPLIBS ${kodiplatform_LIBRARIES})

set(JOYSTICK_SOURCES src/addon.cpp
                     src/api/Joystick.cpp
                     src/api/JoystickInterface.cpp
                     src/api/JoystickManager.cpp
                     src/log/Log.cpp
                     src/log/LogAddon.cpp
                     src/log/LogConsole.cpp
                     src/log/LogSyslog.cpp)

if(SDL_FOUND)
  list(APPEND JOYSTICK_SOURCES src/api/sdl/JoystickInterfaceSDL.cpp
                               src/api/sdl/JoystickSDL.cpp)
  list(APPEND DEPLIBS ${SDL_LIBRARIES})
endif()
if(LINUX)
  list(APPEND JOYSTICK_SOURCES src/api/linux/JoystickInterfaceLinux.cpp
                               src/api/linux/JoystickLinux.cpp)
endif()

build_addon(peripheral.joystick JOYSTICK DEPLIBS)

include(CPack)
