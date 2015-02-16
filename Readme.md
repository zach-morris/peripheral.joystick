
To build peripheral add-ons:

```shell
KODI_PREFIX=$HOME/kodi

git clone -b retroplayer-15alpha1-6438+6227 https://github.com/garbear/xbmc.git xbmc
cd xbmc
./bootstrap
./configure
make -j8
make install DESTDIR=$KODI_PREFIX
make -C tools/depends/target/xbmc-peripheral-addons/ PREFIX=$KODI_PREFIX
cd ..
```

To build just joystick add-on, after doing the above:

```shell
git clone -b cmake https://github.com/garbear/peripheral.joystick.git peripheral.joystick
mkdir peripheral.joystick-project
cd peripheral.joystick-project

cmake ../peripheral.joystick -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$KODI_PREFIX -DCMAKE_INSTALL_PREFIX=$KODI_PREFIX

# or, for eclipse project files:

cmake ../peripheral.joystick -G"Eclipse CDT4 - Unix Makefiles" -D_ECLIPSE_VERSION=4.4 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$KODI_PREFIX -DCMAKE_INSTALL_PREFIX=$KODI_PREFIX

# and then

make
```
