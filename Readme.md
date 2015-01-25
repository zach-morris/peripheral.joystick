
Begin by cloning the following repos

```shell
git clone -b joysticks-cmake git://github.com/garbear/xbmc.git
git clone                    git://github.com/Montellese/kodi-platform.git
git clone -b cmake           git://github.com/garbear/peripheral.joystick.git
```

If you f*ck up and want a clean slate in an instant, try again in a subdirectory

```shell
echo "Screw this"
mkdir kodi-workspace
cd    kodi-workspace
git clone -b joysticks-cmake ../xbmc
git clone                    ../kodi-platform
git clone -b cmake           ../peripheral.joystick
```

Choose a target prefix

```shell
KODI_PREFIX=`pwd`/target
```

Build Kodi

```shell
cd xbmc
./bootstrap
./configure --prefix=$KODI_PREFIX
time make -j8
make install
cd ..
```

Remove xbmc symlink to work around bug fixed in https://github.com/xbmc/xbmc/pull/6271

```shell
rm target/include/xbmc
```

Build kodi-platform

```shell
cd kodi-platform
mkdir build
cd    build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$KODI_PREFIX -DCMAKE_INSTALL_PREFIX=$KODI_PREFIX
time make
sudo make install
cd ..
```

Build peripheral.joystick

```shell
cd peripheral.joystick
mkdir build
cd    build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$KODI_PREFIX -DCMAKE_INSTALL_PREFIX=$KODI_PREFIX
time make
cd ../..
```

Build all peripheral add-ons as Kodi build depends (optional)

```shell
# Use the repo in our workspace
echo "peripheral.joystick `pwd`/peripheral.joystick origin/cmake" > \
  xbmc/project/cmake/addons/addons/peripheral.joystick/peripheral.joystick.txt

cd xbmc
time make -C tools/depends/target/xbmc-peripheral-addons PREFIX=$KODI_PREFIX
```

To clean the build depends for peripheral add-ons

```shell
make -C tools/depends/target/xbmc-peripheral-addons clean
```
