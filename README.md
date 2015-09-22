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

### In more detail:
iotivity-node depends on [iotivity][] proper. It has been tested against [0.9.2][] on Linux. iotivity depends on development headers for libuuid and boost.

iotivity-node requires a compiler that implements the C++11 standard.

During compilation, iotivity-node downloads iotivity from its git repository, builds it, and links against it. If you wish to build iotivity separately, set the environment variable OCTBSTACK_CFLAGS to contain the compiler arguments necessary for building against iotivity, and also set the environment variable OCTBSTACK_LIBS to contain the linker arguments necessary for linking against iotivity. If both variables are set to non-empty values, iotivity-node will skip the step of downloading and building iotivity from sources. If you choose to build iotivity separately, you can use the following procedure, which is known to work on Linux:

0. Grab a [snapshot][] of iotivity from its git repository and unpack it locally.
0. Make sure a compiler, make, [scons][] (a build tool), and the headers for the above-mentioned library dependencies (boost and libuuid) are installed. Your distribution should provide all these tools and libraries.
0. ```cd iotivity```
0. scons has the concept of targets just like make. You can get a list of targets contained in the iotivity repository, as well as a listing of recognized build flags via ```scons --help```. The only target you need for the node.js bindings is ```liboctbstack```. Thus, run ```scons liboctbstack``` to build this target.
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

## Tests

To run the tests, simply run ```npm test```. A script called ```dist.sh``` is also provided. It runs ```npm install```, runs the tests, then runs ```npm prune --production``` to remove all sources, headers, and build dependencies. After that, it copies relevant files to ```dist/``` and creates a tarball from them. This is followed by running ```npm install``` again to restore the development environment.

The ```dist.sh``` script accepts the optional command line argument ```--testonly```. This will cause the script to exit after having run the tests.

A crude way of estimating which functions still need to be tested is this:

```
diff -u \
	<( find tests/tests -type f | zerosep | xargs -0 grep -E 'iotivity\.[_a-zA-Z0-9]*\(' | sed -r 's/^.*iotivity\.([_A-Za-z0-9]*)\(.*$/\1/'  | sort -u ) \
	<( cat src/functions.cc | grep '^  SET_FUNCTION' | sed 's/);$//' | awk '{ print $2;}' | sort )
```

## Maintenance

### To build against a new upstream version:

0. Modify the "version" field in ```package.json```, setting it to the version of iotivity against which you're building
0. Modify the references to the version in ```README.md```
0. Build and optionally install the new version of iotivity
0. If you haven't installed iotivity in the previous step, you must now export environment variables ```OCTBSTACK_CFLAGS``` and ```OCTBSTACK_LIBS```, because the next step needs them.
0. Run ```./update-enums-and-constants.sh``` followed by ```grunt format``` to re-format ```src/constants.cc``` and ```src/enums.cc```
0. Update the commitid in ```build-csdk.sh```
0. If the ```CFLAGS``` and/or ```LIBS``` have changed, modify ```install.sh``` and/or ```octbstack.pc.in``` and make the same modifications in ```binding.gyp```
0. Test the build with both built-in iotivity and with iotivity via pkgconfig:

    ```sh
	nvm use 0.12 && git clean -x -d -f -f && npm run-script ci && \
	nvm use 0.10 && git clean -x -d -f -f && npm run-script ci && \
	nvm use 0.12 && git clean -x -d -f -f && ./dist.sh --testonly && \
	nvm use 0.10 && git clean -x -d -f -f && ./dist.sh --testonly
	```

    The build may fail in ```src/constants.cc``` or ```src/enums.cc```, complaining about undefined constants. That's because ```update-constants-and-enums.sh``` is unaware of C precompiler definitions and so it may harvest constants which are not actually defined under the set of precompiler flags used for building. Thus, you may need to edit ```src/constants.cc``` and ```src/enums.cc``` by hand **after** having run ```update-constants-and-enums.sh```.

The script ```./update-constants-and-enums.sh``` reads the C SDK header files and generates the contents of ```src/constants.cc``` and ```src/enums.cc```. Read the comments in the script before you modify either ```src/constants.cc``` or ```src/enums.cc```.

### Coding Style And Principles
Please follow the [jQuery][] coding style for the JavaScript files. You can format all JS and C++ files with the following command:
```
grunt format
```

When writing the bindings, data arriving from Javascript is considered unreliable and must be validated. If it does not validate correctly, an exception must be thrown immediately after the failed check, and the process must be aborted. Data arriving from C is considered reliable and can be assigned to Javascript immediately.

Functions converting Javascript structures to C structures are named ```c_CStructureName``` and have the following signature:

```C++
bool c_CStructureName( Local<Object> jsStructureName, CStructureName **p_putStructurePointerHere );
```

This signature allows us to throw an exception and return false if any part of ```jsStructureName``` fails validation. The caller can then also return false immediately, and the binding can ultimately return undefined immediately.

As a general principle, if a Javascript value fails validation, throw an exception immediately. Do not return false and expect the caller to throw the exception. Call it exception-at-the-point-of-failure.

Pointers to structures received from the C API may always be null. The functions converting those pointers to Javascript objects (js_CStructureName()) assume that they are not null. So, wrap the call to such a function in a null-check.

When filling out a C structure in a function c_CStructureName, create a local structure first, and only if all validations pass, memcpy() the local structure into the structure passed in.

[iotivity]: http://iotivity.org/
[node]: https://nodejs.org/
[0.9.2]: https://gerrit.iotivity.org/gerrit/gitweb?p=iotivity.git;a=tree;hb=0.9.2
[snapshot]: https://gerrit.iotivity.org/gerrit/gitweb?p=iotivity.git;a=snapshot;h=0.9.2;sf=tgz
[scons]: http://www.scons.org/
[install.sh]: https://raw.githubusercontent.com/otcshare/iotivity-node/master/install.sh
[octbstack.pc.in]: https://raw.githubusercontent.com/otcshare/iotivity-node/master/octbstack.pc.in
[jQuery]: http://contribute.jquery.org/style-guide/js/
