# Building out-of-tree (recommended)

## Linux

Create and enter a build directory

```shell
mkdir peripheral.joystick-build
cd peripheral.joystick-build
```

Generate a build environment with config for debugging (assuming `$HOME/workspace/kodi` symlinks to the folder you cloned Kodi into)

```shell
cmake -DADDONS_TO_BUILD=peripheral.joystick \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=$HOME/workspace/kodi/addons \
      -DPACKAGE_ZIP=1 \
      $HOME/workspace/kodi/project/cmake/addons
```

The add-on can then be built with `make`.

# Building stand-alone (development)

## Linux

Stand-alone builds are closer to "normal" software builds. The build system looks for its dependencies, by default with `/usr` and `/usr/local` prefixes.

To provide these dependencies yourself in a local working directory (`$HOME/kodi`), build Kodi with an installation prefix

```shell
cd $HOME/workspace/kodi
./bootstrap
./configure --prefix=$HOME/kodi
make
make install
```

Clone p8-platform and create a CMake build directory

```shell
git clone https://github.com/Pulse-Eight/platform.git
cd platform
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=$HOME/kodi \
      ..
make
make install
```

The kodi-platform library was split from p8-platform. Do the same as above for this library:

```
git clone https://github.com/xbmc/kodi-platform.git
...
```

With these dependencies in place, the add-on can be built. You have two options:

* Use the same CMake command for p8-platform above to install into the `--configure` prefix
* Use this command to copy add-ons to your Kodi source tree:

```shell
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_PREFIX_PATH=$HOME/kodi \
      -DCMAKE_INSTALL_PREFIX=$HOME/workspace/kodi/addons \
      -DPACKAGE_ZIP=1 \
      ..
```

## Windows

I'm still working on build instructions for stand-alone builds on Windows. The closest instructions may come from this helpful forum post: http://forum.kodi.tv/showthread.php?tid=173361&pid=2097898#pid2097898.

For now, follow the Windows instructions below.

# Building in-tree (cross-compiling)

Kodi's build system will fetch the add-on from the GitHub URL and git hash specified in [peripheral.joystick.txt](https://github.com/garbear/xbmc/blob/retroplayer-15.1/project/cmake/addons/addons/peripheral.joystick/peripheral.joystick.txt).

## Linux

Ensure that kodi has been built successfully with the desired prefix (`$HOME/kodi` for a local prefix, or omitted for the system prefix (usually `/usr` or `/usr/local`). Then, from the root of Kodi's source tree, run

```shell
make install
```

Build the add-on

```shell
make -C tools/depends/target/binary-addons PREFIX=$HOME/kodi ADDONS="peripheral.joystick"
```

The compiled .so can be found at

```
$HOME/kodi/lib/kodi/addons/peripheral.joystick/peripheral.joystick.so
```

To rebuild the add-on or compile a different one, clean the build directory

```shell
make -C tools/depends/target/binary-addons clean
```

## Windows

First, download and install [CMake](http://www.cmake.org/download/).

To compile on windows, open a command prompt at `tools\buildsteps\win32` and run the script:

```
make-addons.bat install peripheral.joystick
```

Alternatively, generate the `kodi-addons.sln` Visual Studio solution and project files.

```
tools\windows\prepare-binary-addons-dev.bat
```

The generated solution can be found at

```
project\cmake\addons\build\kodi-addons.sln
```

Add-ons can be built individually through their specific project, or all at once by building the solution.

No source code is downloaded at the CMake stage; when the project is built, the add-on's source will be downloaded and compiled.

## OSX

Per [README.osx](https://github.com/garbear/xbmc/blob/retroplayer-15.1/docs/README.osx), enter the `tools/depends` directory and make the add-on:

```shell
cd tools/depends
make -C target/binary-addons ADDONS="peripheral.joystick"
```

To rebuild the add-on or compile a different one, clean the build directory

```shell
make -C target/binary-addons clean
```
