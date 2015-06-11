# iotivity-node
## Description
This project provides [iotivity](http://iotivity.org/) node.js bindings.

## Installation

iotivity-node depends on [iotivity](http://iotivity.org/) proper. It has been tested against [0.9.1](https://gerrit.iotivity.org/gerrit/gitweb?p=iotivity.git;a=tree;hb=0.9.1) on Linux.

During compilation, it needs environment variables to retrieve the iotivity compiler and linker flags. This means that you have to first build and install iotivity. The following procedure is known to work on Linux:

0. Grab a [snapshot](https://gerrit.iotivity.org/gerrit/gitweb?p=iotivity.git;a=snapshot;h=0.9.1;sf=tgz) of iotivity from its git repository and unpack it locally.
0. Make sure a compiler, make, and [scons](http://www.scons.org/) (a build tool) are installed. Your distribution should provide all these tools.
0. ```cd iotivity```
0. scons has the concept of targets just like make. You can get a list of targets contained in the iotivity repository, as well as a listing of recognized build flags via ```scons --help```. The only target you need for the node.js bindings is ```liboctbstack```. Thus, run ```scons liboctbstack``` to build this target.
0. Now that iotivity is built, clone this repository, and change directory into it.
0. Set the following environment variables:
	* ```OCTBSTACK_CFLAGS``` - this should contain the compiler flags for locating the iotivity include files. For example, the value of this variables can be ```-I/home/nix/iot/iotivity/resource/csdk/stack/include```.
	* ```OCTBSTACK_LIBS``` - this should contain the linker flags necessary for locating ```liboctbstack.so``` both at compile time and at runtime. It's value can be as simple as ```-loctbstack``` if liboctbstack is in /usr/lib, but may need to be as complex as ```-L/home/nix/iot/iotivity/out/linux/x86/release -loctbstack -Wl,-rpath=/home/nix/iot/iotivity/out/linux/x86/release``` if liboctbstack is located on an unusual path.
0. Run ```npm install``` with these environment variables set.

Alternatively, you can use some rudimentary install scripts for both iotivity and this repository. Using them will help you avoid having to set environment variables.

0. Grab [install.sh](https://raw.githubusercontent.com/gabrielschulhof/iotivity/install/install.sh) and [octbstack.pc.in](https://raw.githubusercontent.com/gabrielschulhof/iotivity/install/octbstack.pc.in) and place them in the root of the iotivity repository.
0. As root, change directory to the iotivity repository and run ```./install.sh```. The script recognizes two environment variables: ```PREFIX``` is set to ```/usr``` by default, and ```DESTDIR``` is unset. Use ```PREFIX``` to install to a location other than ```/usr``` and use ```DESTDIR``` to set an additional prefix where to put the files. This latter option is especially useful for packaging. Examples:

    ```PREFIX=/usr/local ./install.sh``` will install the files into ```/usr/local``` instead.

    ```PREFIX=/usr/local DESTDIR=/home/username/iotivity-installation ./install.sh``` will install the files into ```/home/username/iotivity-installation/usr/local```, but will configure liboctbstack to work when loaded from ```/usr/local```.
0. After having installed iotivity using the above script, you can run ```./dist.sh``` from the root of this repository without first having to set any environment variables. The script will grab the environment via ```pkg-config``` from the file installed above, and will build, test, and create a directory structure under ```dist/``` which is suitable for deployment on a target device.

## Placing the binaries onto a device
The distribution scripts (```dist.sh``` in iotivity-node, and ```install.sh``` in iotivity) when used together make it easy to create a binary tarball which can be unpacked into the root directory of a device:

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

The JavaScript examples are located in [js/](./js/) and come in pairs of one client and one server, each illustrating a basic aspect of iotivity. To run them, open two terminals and change directory to the root of the iotivity-node repository in both. Always launch the server before the client. For example, in one terminal you can run ```node js/server.observe.js``` and in the other terminal you can run ```node js/client.observe.js```.

If you wish to run the client on one machine and the server on another, make sure no firewall is running on either machine.

## Tests

To run the tests, simply run ```npm test```. A script called ```dist.sh``` is also provided. It runs ```npm install```, runs the tests, then runs ```npm prune --production``` to remove all sources, headers, and build dependencies. After that, it copies relevant files to ```dist/``` and creates a tarball from them. This is followed by running ```npm install``` again to restore the development environment.

## Maintenance

To build against a new upstream versions:

0. Build, and optionally install the new version of iotivity
0. Run ```./update-enums-and-constants.sh```
0. Run ```./dist.sh```

The script ```./update-enums-and-constants.sh``` reads the header files ocstackconfig.h and octypes.h and generates the contents of src/constants.cc and src/enums.cc. Read the comments in the script before you modify either src/constants.cc or src/enums.cc.

## Coding Style
Please follow the [jQuery](http://contribute.jquery.org/style-guide/js/) coding style for the JavaScript files.

The C++ files can be formatted using ```clang-format -style=Google```:
```BASH
find src -type f | while read; do
  clang-format -style=Google "$REPLY" > "$REPLY".new && mv "$REPLY".new "$REPLY"
done
```
