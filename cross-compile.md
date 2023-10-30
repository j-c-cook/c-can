# Cross Compile

These notes describe the process required to cross-compile `c-can` for the armv7l architecture 
using the [arm-linux-gnueabihf][#gneeabihf] toolchain. Some of these notes are sourced from 
[gist.cross-compile-python.md][#gist]. All of the following commands are valid for Ubuntu.

## Download and unpack the toolchain 

```angular2html
cd $HOME/Downloads
wget https://releases.linaro.org/components/toolchain/binaries/7.5-2019.12/arm-linux-gnueabihf/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz
sudo mkdir /opt/
tar -xvf gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz -C /opt/
```

It's going to be useful to create a local variable named `cross` that describes the path to the cross-compilation 
toolchain. 
```angular2html
export cross=/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf
```

## Cross-compile library dependencies

The two dependencies of this library at the time of writing this are the `math` and `zlib` libraries. The math library, 
`m` is shipped with the toolchain. However, the zlib library, `zlib`, must be manually cross compiled and then linked. 

First, download and unpack the library. The zlib version used here is `v1.2.11`. The version has been chosen based off
of the version that is given in the buildroot package used to create the `rootfs`. Check the zlib version that is 
loaded onto the target device. 
```angular2html
cd $HOME/Downloads
wget https://github.com/madler/zlib/archive/refs/tags/v1.2.11.tar.gz  # zlib version 1.2.11
tar -xvf v1.2.11.tar.gz && cd zlib-1.2.11/
```

The library needs to be informed of which tools to use, and given an installation prefix that describes where the 
library will be installed once it is built. 
```angular2html
CHOST="${cross}" CC="${cross}-gcc" CXX="${cross}-g++" AR="${cross}-ar" LD="${cross}-ld" RANLIB="${cross}-ranlib" ./configure --prefix=$HOME/zlibArm
```
<details><summary>The details inside this dropdown contain the configure output.</summary>

```
Using /opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-ar
Using /opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-ranlib
Using /opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-nm
Checking for shared library support...
Building shared library libz.so.1.2.11 with /opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc.
Checking for size_t... Yes.
Checking for off64_t... Yes.
Checking for fseeko... Yes.
Checking for strerror... Yes.
Checking for unistd.h... Yes.
Checking for stdarg.h... Yes.
Checking whether to use vs[n]printf() or s[n]printf()... using vs[n]printf().
Checking for vsnprintf() in stdio.h... Yes.
Checking for return value of vsnprintf()... Yes.
Checking for attribute(visibility) support... Yes.
```

</details>

Now the `z` library can be compiled and installed. 
```angular2html
make
make install
```
Remember that the installation prefix was set to `$HOME/zlibArm`, and that is where the library can now be found at.

## Cross-compile c-can using cmake

This step can be done with the aid of an integrated development environment (IDE) (if it supports CMAKE) or from the 
terminal. The documentation here will describe how to compile from the terminal, given that everyone running Linux has
access to that. IDE specific documentation is not described here.

Set the external cmake cache entries that are related to the toolchain. Note that at any point the `CmakeCache.txt` 
file can be inspected for accuracy. 
```angular2html
mkdir build && cd build
cmake .. -DCMAKE_ADDR2LINE="${cross}-addr2line" -DCMAKE_AR="${cross}-ar" -DCMAKE_C_COMPILER="${cross}-gcc" -DCMAKE_C_COMPILER_AR="${cross}-ar" -DCMAKE_C_COMPILER_RANLIB="${cross}-gcc-ranlib" -DCMAKE_LINKER="${cross}-ld" -DCMAKE_NM="${cross}-nm" -DCMAKE_OBJCOPY="${cross}-objcopy" -DCMAKE_OBJDUMP="${cross}-objdump" -DCMAKE_RANLIB="${cross}-ranlib" -DCMAKE_READELF="${cross}-readelf" -DCMAKE_STRIP="{cross}-strip" 
```

<details><summary>The output after setting the toolchain cmake cache variables.</summary>

```angular2html
-- The C compiler identification is GNU 7.5.0
-- Check for working C compiler: /opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
-- Check for working C compiler: /opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /path/to/c-can/build
```
</details>

Now the project specific variables need to be passed in. 
```angular2html
cmake .. -DZLIB_INCLUDE_DIR="$HOME/zlibArm/include" -DZLIB_LIBRARY="$HOME/zlibArm/lib/libz.so" -DMATH_LIBRARY=/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/usr/lib/libm.so -DCMAKE_INSTALL_PREFIX="$HOME/c_canlibArm"
```

Now the library can be built and installed.
```angular2html
make
make install
```

Note: The project can now be zipped using `tar -czvf file.tar.gz directory`.



[#gneeabihf]: https://releases.linaro.org/components/toolchain/binaries/7.5-2019.12/arm-linux-gnueabihf/
[#gist]: https://gist.github.com/j-c-cook/2a291dc0bfaa2f6639272e344ff66e62
