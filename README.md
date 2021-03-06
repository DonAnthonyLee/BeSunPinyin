# BeSunPinyin
SunPinyin wrapper for EIME/BeOS/HaikuOS.

It's an input method add-on developing for EIME/BeOS/HaikuOS using libsunpinyin-2.0/3.0.


## Dependency for building BeSunPinyin
**1. libsunpinyin**

+ Ubuntu Linux

Run commands below to install libsunpinyin.
```bash
sudo apt-get install libsunpinyin-dev sunpinyin-data
```

+ HaikuOS

1.1. Install libsunpinyin & libsunpinyin_devel by HaikuDepot;

1.2. Run the script located at "utils/gen-sunpinyin-data-hpkg.sh", then install the package.

*Note: The current "libsunpinyin" package provided by HaikuDepot might can't work properly, you might need to recompile libsunpinyin.so.3 with the following command from sunpinyin's sources :*

```bash
scons --prefix=/boot/system
```

+ Windows

See [the repository](https://github.com/DonAnthonyLee/sunpinyin/) for more detail (No package released yet).


**2. Lite BeAPI**

+ BeOS/HaikuOS

Most of the sources written in native BeOS API, so BeSunPinyin don't rely on this when building on BeOS/HaikuOS.


+ Other platforms

See [the repository](https://github.com/DonAnthonyLee/etkxx-devel-binary) for more detail.

*Note: For the time being writing this, it's still WIP for improving.*



**3. EIME or input_server**

+ BeOS/HaikuOS

When you run "make" to build BeSunPinyin, it will link against the "input_server" of system automatically.


+ Other platforms

You will need XIM/Wayland/TSF or any other implementation of EIME to build BeSunPinyin.

*Note: For the time being writing this, it's still WIP, and I will keep updating the progress on [老李的杂货铺](https://donanthonylee.github.io) .*


## How to use it after building

+ BeOS/HaikuOS

Copy the file named "SunPinyin.so" to add-ons directory (like /boot/home/config/add-ons/input_server/methods/) of input server, then restart input server or system.

*Note: Copy the file to /boot/home/config/non-packaged/add-ons/input_server/methods/ when using HaikuOS without creating .hpkg.*

If anything blocked booting, you can press the SPACE when booting and select "Disable user add-ons" to disable it.


+ Windows

Copy the file named "SunPinyin.dll" (code signed if necessary) to %APPDATA\eime\addons, then restart the implementation of EIME.

*Note: You can copy the files named "libsunpinyin.dll" and "sqlite3.dll" to %APPDATA\eime .*


+ Other platforms

Copy the file named "SunPinyin.so" to "addons" directory of EIME, then restart the implementation of EIME.


