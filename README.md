# DingusPPC

Written by divingkatae and maximumspatium

Be warned the program is highly unfinished and could use a lot of testing. Any feedback is welcome.

## Philosophy of Use

While many other PowerPC emus exist (PearPC, Sheepshaver), none of them currently attempt emulation of PPC Macs accurately (except for QEMU).

This program aims to not only improve upon what Sheepshaver, PearPC, and other PowerPC mac emulators have done, but also to provide a better debugging environment. This currently is designed to work best with PowerPC Old World ROMs, including those of the PowerMac G3 Beige.

## Implemented Features

This emulator has a debugging environment, complete with a disassembler. We also have implemented enough to allow OpenFirmware to boot, going so far as to allow audio playback of the boot-up jingles.

## How to Use

This program currently uses the command prompt to work.

There are a few command line arguments one must enter when starting the program.

```
-r, --realtime
```

Run the emulator in runtime.

```
-d, --debugger
```

Enter the interactive debugger.

```
-b, --bootrom TEXT:FILE
```

Specifies the Boot ROM path (optional; looks for bootrom.bin by default)

```
-m, --machine TEXT
```

Specify machine ID (optional; will attempt to determine machine ID from the boot rom otherwise)

As of now, only the Power Macintosh G3 Beige is implemented.

## How to Compile

You need to install development tools first.

At least, a C++11 compliant compiler and [CMake](https://cmake.org) are required.

You will also have to recursive clone or run 
```
git submodule update --init --recursive
```

This is because the CubeB module is not included by default. All other components are already included in the thirdparty folder and compiled along with the rest of DingusPPC.

For example, to build the project in a Unix-like environment, you will need to run
the following commands in the OS terminal:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make dingusppc
```
You may specify another build type using the variable CMAKE_BUILD_TYPE.

Future versions may drop SDL 2 as a requirement.

For Raspbian, you may also need the following command:
```
sudo apt install doxygen graphviz
```

## Testing

DingusPPC includes a test suite for verifying the correctness of its PowerPC CPU
emulation. To build the tests, use the following terminal commands:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make testppc
```

## Intended Minimum Requirements

- Windows 7 or newer (64-bit), Linux 4.4 or newer, Mac OS X 10.9 or newer (64-bit)
- Intel Core 2 Duo or better
- 2 GB of RAM
- 2 GB of Hard Disk Space
- Graphics Card with a minimum resolution of 800*600

## Compiler Requirements

- GCC 4.7 or newer (i.e. CodeBlocks 13.12 or newer)
- Visual Studio 2013 or newer
