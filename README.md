# iotivity-node

## Description
This project provides [iotivity][] node.js bindings.

## Build status
<a href="https://travis-ci.org/otcshare/iotivity-node">
	<img alt="Build Status" src="https://travis-ci.org/otcshare/iotivity-node.svg?branch=master"></img>
</a>

## Installation

### For the impatient:
0. Make sure [node][] is up and running
0. Install the following packages, which your distribution should provide:
    0. unzip, scons, and make
    0. Development headers for boost, libuuid, and libcurl

        boost and libcurl are needed during the build process, but iotivity-node does not ultimately depend on either.
    0. A C compiler and a C++ compiler
0. Run ```npm install```

iotivity-node provides two interfaces: The low-level interface is a close mapping of the iotivity C API.
```JS
var iotivity = require( "iotivity-node/lowlevel" );
```
will give you access to the low-level interface.

There is also a high-level interface which implements the [JS API spec](./spec/iot-js-spec.md). This interface requires the ```Promise``` object which is only available starting node.js 4.0.
```JS
var device = require( "iotivity-node" )();
```
will give you an instance of OicDevice as described in the JS API spec.

### In more detail:
iotivity-node depends on [iotivity][] proper. It has been tested against [1.0.0][] on Linux. iotivity depends on development headers for libuuid and boost.

iotivity-node requires a compiler that implements the C++11 standard.

During compilation, iotivity-node downloads iotivity from its git repository, builds it, and links against it. If you wish to build iotivity separately, set the environment variable OCTBSTACK_CFLAGS to contain the compiler arguments necessary for building against iotivity, and also set the environment variable OCTBSTACK_LIBS to contain the linker arguments necessary for linking against iotivity. If both variables are set to non-empty values, iotivity-node will skip the step of downloading and building iotivity from sources. If you choose to build iotivity separately, you can use the following procedure, which is known to work on Linux:

0. Grab a [snapshot][] of iotivity from its git repository and unpack it locally.
0. Make sure a compiler, make, [scons][] (a build tool), and the headers for the above-mentioned library dependencies (boost and libuuid) are installed. Your distribution should provide all these tools and libraries.
0. ```cd iotivity```
0. scons has the concept of targets just like make. You can get a list of targets contained in the iotivity repository, as well as a listing of recognized build flags via ```scons --help```. The only target you need for the node.js bindings is ```liboctbstack```. Thus, run ```scons liboctbstack``` to build this target.

    On OSX you need more targets than just ```liboctbstack``` because on that platform iotivity does not build ```liboctbstack``` as a shared library, but rather as an archive. Thus, you need to build all targets that correspond to archives that go into the Linux ```liboctbstack``` shared library:
    * ```libconnectivity_abstraction```
    * ```libcoap```
    * ```c_common```
    * ```libocsrm```
0. Now that iotivity is built, clone this repository, and change directory into it.
0. Set the following environment variables:
	* ```OCTBSTACK_CFLAGS``` - this should contain the compiler flags for locating the iotivity include files. For example, the value of this variables can be ```-I/home/nix/iot/iotivity/resource/csdk/stack/include```.
	* ```OCTBSTACK_LIBS``` - this should contain the linker flags necessary for locating ```liboctbstack.so``` both at compile time and at runtime. Its value can be as simple as ```-loctbstack``` if liboctbstack is in /usr/lib, but may need to be as complex as ```-L/home/nix/iot/iotivity/out/linux/x86/release -loctbstack -Wl,-rpath=/home/nix/iot/iotivity/out/linux/x86/release``` if liboctbstack.so is located on an unusual path.
0. Run ```npm install``` with these environment variables set.

Alternatively, you can use some rudimentary install scripts for both iotivity and this repository. Using them will help you avoid having to set the environment variables ```OCTBSTACK_CFLAGS``` and ```OCTBSTACK_LIBS```, because the scripts will supply them to the build process.

0. Grab [install.sh][] and [octbstack.pc.in][] and place them in the root of the iotivity repository.
0. As root, change directory to the iotivity repository and run ```./install.sh```. The script recognizes two environment variables: ```PREFIX``` is set to ```/usr``` by default, and ```DESTDIR``` is unset. Use ```PREFIX``` to install to a location other than ```/usr``` and use ```DESTDIR``` to set an additional prefix where to put the files. This latter option is especially useful for packaging. Examples:

    ```PREFIX=/usr/local ./install.sh``` will install the files into ```/usr/local``` instead.

    ```PREFIX=/usr/local DESTDIR=/home/username/iotivity-installation ./install.sh``` will install the files into ```/home/username/iotivity-installation/usr/local```, but will configure liboctbstack to work when loaded from ```/usr/local```.
0. After having installed iotivity using the above script, you can run ```./dist.sh``` from the root of this repository without first having to set any environment variables. The script will grab the environment via ```pkg-config``` from the file installed above, and will build, test, and create a directory structure under ```dist/``` which is suitable for deployment on a target device. See ```./dist.sh --help``` for more options.

## Placing the binaries onto a device
The distribution scripts ```dist.sh``` and ```install.sh``` when used together make it easy to create a binary tarball which can be unpacked into the root directory of a device:

0. Change directory to the root of the ```iotivity``` repository and run ```DESTDIR=/tmp/iotivity-installation ./install.sh```
0. ```mkdir -p /tmp/iotivity-installation/usr/lib/node_modules```
0. Change directory to the root of the ```iotivity-node``` repository.
0. ```./dist.sh```
0. ```cd dist```
0. ```cp -a iotivity /tmp/iotivity-installation/usr/lib/node_modules```
0. ```cd /tmp/iotivity-installation```
0. ```rm -rf usr/include usr/lib/pkgconfig```
0. ```tar cvjf iotivity.bin.tar.bz2 *```

You can now transfer iotivity.bin.tar.bz2 to the device and then unpack it into the root directory.

## Examples

The JavaScript examples are located in [js/](./js/) and come in pairs of one client and one server, each illustrating a basic aspect of iotivity. To run them, open two terminals and change directory to the root of the iotivity-node repository in both. Always launch the server before the client. For example, in one terminal you can run ```node js/server.discoverable.js``` and in the other terminal you can run ```node js/client.discovery.js```.

Make sure no firewall is running (or one is properly configured to allow iotivity-related traffic) on the machine(s) where these applications are running.

[iotivity]: http://iotivity.org/
[node]: https://nodejs.org/
[1.0.0]: https://gerrit.iotivity.org/gerrit/gitweb?p=iotivity.git;a=tree;hb=1.0.0
[snapshot]: https://gerrit.iotivity.org/gerrit/gitweb?p=iotivity.git;a=snapshot;h=1.0.0;sf=tgz
[scons]: http://www.scons.org/
[install.sh]: https://raw.githubusercontent.com/otcshare/iotivity-node/1.0.0-1/install.sh
[octbstack.pc.in]: https://raw.githubusercontent.com/otcshare/iotivity-node/1.0.0-1/octbstack.pc.in
