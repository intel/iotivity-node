# iotivity-node

## Description
This project provides a Javascript API for [OCF][] functionality. The API follows a maintained [specification][] and is implemented as a native addon using [iotivity][] as its backend.

## Status
<a href="https://ci.appveyor.com/project/gabrielschulhof/iotivity-node/branch/master">
	<img alt="Windows Build Status" src="https://ci.appveyor.com/api/projects/status/github/otcshare/iotivity-node?branch=master&svg=true"></img>
</a>
<a href="https://travis-ci.org/otcshare/iotivity-node">
	<img alt="Posix Build Status" src="https://travis-ci.org/otcshare/iotivity-node.svg?branch=master"></img>
</a>
<a href='https://coveralls.io/github/otcshare/iotivity-node?branch=master'>
	<img src='https://coveralls.io/repos/github/otcshare/iotivity-node/badge.svg?branch=master' alt='Coverage Status' />
</a>

## Installation

* On Linux and OSX
  0. Make sure [node][] is up and running
  0. Install the following packages, which your distribution should provide:
      0. unzip, scons, git, and make
      0. Development headers for boost (at least 1.55), libuuid, and glib2
      0. A C compiler and a C++ compiler (gcc-5 or later)
  0. Clone this repository
  0. cd `iotivity-node`
  0. Run `npm install`
* On Windows ([video](https://www.youtube.com/watch?v=RgsZpv8IrWA))
  0. Install [node][]
  0. In a PowerShell running as Administrator, run `npm install -g --production windows-build-tools`. This will install Python and the toolchain necessary for building iotivity-node. While this command runs, you can perform some of the following steps.
  0. Install [git][]
  0. Install [7-Zip][]
  0. The installation of the `windows-build-tools` package eventually indicates that it has installed Python. After that message appears, you can perform some of the steps below.
  0. In a command prompt, append the python folder, the python scripts folder, and the 7-Zip folder to your PATH. The paths you append are based on your Windows user name, so replace "yourusernamehere" in the example below with your actual Windows user name.

      ```
      setx PATH "%PATH%;c:\Users\yourusernamehere\.windows-build-tools\python27;c:\Users\yourusernamehere\.windows-build-tools\python27\scripts;c:\Program Files\7-Zip"
      ```
  0. Close the command prompt and reopen it.
  0. In the command prompt, run `pip install --egg scons` to install scons (a python package)
  0. Wait for the installation of the `windows-build-tools` to complete. Afterwards, you can perform the remaining steps.
  0. Clone this repository and the change directory into it
  0. Run `npm install` to build iotivity-node.
  0. After the successful completion of the above command, you are ready to use iotivity-node. You can use the usual npm process of adding iotivity-node to the `dependencies` section of your package's `package.json` file.

After installation using the steps above, you may want to run the iotivity-node test suite. To do so, perform the following steps from the iotivity-node repository root. The steps apply to all platforms:

0. Run `npm -g install grunt-cli`
0. Run `grunt test`

The file [appveyor.yml](./appveyor.yml) provides an example of the commands necessary for setting up a Windows environment, and the file [.travis.yml](./.travis.yml) provides an example of the commands necessary for setting up the Linux and OSX environments.

### In more detail:
iotivity-node depends on [iotivity][] proper. It has been tested against [1.2.0][]. The above installation instructions cover the dependencies for both iotivity-node and iotivity.

iotivity-node requires a compiler that implements the C++11 standard.

During compilation, iotivity-node downloads iotivity from its git repository, builds it, and links against it. If you wish to build iotivity separately, set the environment variable `OCTBSTACK_CFLAGS` to contain the compiler arguments necessary for building against iotivity, and also set the environment variable `OCTBSTACK_LIBS` to contain the linker arguments necessary for linking against iotivity. If both variables are set to non-empty values, iotivity-node will skip the step of downloading and building iotivity from sources. If you choose to build iotivity separately, you can use the following procedure:

0. Grab a [snapshot][] of iotivity from its git repository and unpack it locally.
0. Make sure a build toolchain, [scons][] (a build tool), and the headers for the above-mentioned library dependencies are installed. Your distribution should provide all these tools and libraries.
0. `cd iotivity`
0. If you're building against version 1.2.0 of iotivity on OSX or Windows, you will first need to apply the downstream patches which iotivity-node provides in the `patches/` subdirectory. You can use `git apply <path-to-patch>` for this. These patches are on track to appear in later versions of iotivity, so they will disappear from later versions of iotivity-node.
0. scons has the concept of targets just like make. You can get a list of targets contained in the iotivity repository, as well as a listing of recognized build flags via `scons --help`. The only targets you need for the node.js bindings are `octbstack` and `json2cbor`. Thus, run `scons SECURED=1 json2cbor octbstack` to build these targets.

    On OSX you need more targets than just `octbstack` and `json2cbor` because on that platform iotivity does not build `octbstack` as a shared library, but rather as an archive. Thus, you need to build all targets that correspond to archives that go into the Linux `liboctbstack` shared library:

    * `c_common`
    * `coap`
    * `connectivity_abstraction`
    * `json2cbor`
    * `logger`
    * `ocsrm`
    * `octbstack`
    * `routingmanager`

0. Now that iotivity is built, clone this repository, and change directory into it.
0. Set the following environment variables:
	* `OCTBSTACK_CFLAGS` - this should contain the compiler flags for locating the iotivity include files. For example, the value of this variable can be `-I/home/nix/iot/iotivity/resource/csdk/stack/include`.
	* `OCTBSTACK_LIBS` - this should contain the linker flags necessary for locating `liboctbstack.so` both at compile time and at runtime. Its value can be as simple as `-loctbstack` if liboctbstack is in /usr/lib, but may need to be as complex as `-L/home/nix/iot/iotivity/out/linux/x86/release -loctbstack -Wl,-rpath=/home/nix/iot/iotivity/out/linux/x86/release` if liboctbstack.so is located on an unusual path.
0. Run `npm install` with these environment variables set.

## Provisioning and device ID persistence

The high-level JS API provides a means for persisting the device ID across instantiations of a script according to the [iotivity wiki][]. This mechanism is also responsible for initially creating the configuration file that stores security-related information for a given script. It does so by creating a directory ```${HOME}/.iotivity-node```. Thereunder, it creates directories whose name is the sha256 checksum of the absolute path of the given script. Thus, if you write a script located in ```/home/user/myscript.js``` that uses the high-level JS API, its persistent state will be stored in the directory
```
/home/user/.iotivity-node/1abfb1b70eaa1ccc17a42990723b153a0d4b913a8b15161f8043411fc7f24fb1
```
in a file named ```oic_svr_db.dat```. The file initially contains enough information to persist the device ID used whenever you run ```/home/user/myscript.js```. You can add more information to the file in accordance with the [iotivity wiki][], and using the `json2cbor` tool. The tool is located in `iotivity-installed/bin` off the root of this repository, or, if you have chosen to build iotivity externally, then in the output directory created by the iotivity build process.

## Examples

The JavaScript examples are located in [js/](./js/) and come in pairs of one client and one server, each illustrating a basic aspect of iotivity. To run them, open two terminals and change directory to the root of the iotivity-node repository in both. Always launch the server before the client. For example, in one terminal you can run ```node js/server.discoverable.js``` and in the other terminal you can run ```node js/client.discovery.js```.

Make sure no firewall is running (or one is properly configured to allow iotivity-related traffic and especially multicast traffic) on the machine(s) where these applications are running.

[iotivity]: http://iotivity.org/
[node]: https://nodejs.org/
[1.2.0]: https://gerrit.iotivity.org/gerrit/gitweb?p=iotivity.git;a=tree;hb=1.2.0
[snapshot]: https://gerrit.iotivity.org/gerrit/gitweb?p=iotivity.git;a=snapshot;h=1.2.0;sf=tgz
[scons]: http://www.scons.org/
[iotivity wiki]: https://wiki.iotivity.org/faq_s
[video]: https://www.youtube.com/watch?v=95VTB_qgYfw
[specification]: https://github.com/01org/iot-js-api/tree/ocf-1.1.0/api/ocf
[OCF]: http://openconnectivity.org/
[git]: http://git-scm.org/
[7-Zip]: http://7-zip.org/
