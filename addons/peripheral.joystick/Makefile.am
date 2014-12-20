#
# Makefile for the joysticks add-on for XBMC
#
# See the README for copyright information and
# how to reach the author.
#

ADDONBINNAME    = Kodi_Joystick
ADDONNAME       = peripheral.joystick
LIBNAME         = libjoystick-addon
lib_LTLIBRARIES = libjoystick-addon.la

include ../Makefile.include.am

libjoystick_addon_la_SOURCES = src/addon.cpp \
                               src/api/Joystick.cpp \
                               src/api/JoystickInterface.cpp \
                               src/api/JoystickManager.cpp \
                               src/api/linux/JoystickInterfaceLinux.cpp \
                               src/api/linux/JoystickLinux.cpp \
                               src/api/sdl/JoystickInterfaceSDL.cpp \
                               src/api/sdl/JoystickSDL.cpp \
                               src/log/Log.cpp \
                               src/log/LogAddon.cpp \
                               src/log/LogConsole.cpp \
                               src/log/LogSyslog.cpp

libjoystick_addon_la_LDFLAGS = @TARGET_LDFLAGS@ @LIBS_DL@
