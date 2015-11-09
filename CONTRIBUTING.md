## Maintenance

### Coding Style And Principles
Please follow the [jQuery][] coding style for the JavaScript files. You can format all JS and C++ files with the following command:
```
grunt format
```

When writing the bindings, data arriving from Javascript is considered unreliable and must be validated. If it does not validate correctly, an exception must be thrown immediately after the failed check, and the function must return immediately. Data arriving from C is considered reliable and can be assigned to Javascript immediately.

Functions converting Javascript structures to C structures are named ```c_CStructureName``` and have the following signature:

```C++
bool c_CStructureName( Local<Object> jsStructureName, CStructureName **p_putStructurePointerHere );
```

This signature allows us to throw an exception and return false if any part of ```jsStructureName``` fails validation. The caller can then also return false immediately, and the binding can ultimately return undefined immediately.

As a general principle, if a Javascript value fails validation, throw an exception immediately. Do not return false and expect the caller to throw the exception. Call it exception-at-the-point-of-failure.

Pointers to structures received from the C API may always be null. The functions converting those pointers to Javascript objects (js_CStructureName()) assume that they are not null. So, wrap the call to such a function in a null-check.

When filling out a C structure in a function c_CStructureName, create a local structure first, and only if all validations pass, memcpy() the local structure into the structure passed in.

### Additional dependencies
```clang-format``` is required for formatting the C++ source code and, since the test suite ensures that the code is properly formatted by comparing the code as present to the code after an application of ```grunt format```, ```clang-format``` is also required for running the tests. Your distribution should provide this utility.

### Tests

To run the tests, simply run ```grunt test```. A script called ```dist.sh``` is also provided. It runs ```npm install```, runs the tests, then runs ```npm prune --production``` to remove all sources, headers, and build dependencies. After that, it copies relevant files to ```dist/``` and creates a tarball from them. This is followed by running ```npm install``` again to restore the development environment.

The ```dist.sh``` script accepts the optional command line argument ```--testonly```. This will cause the script to exit after having run the tests.

A crude way of estimating which functions still need to be tested is this:

```
diff -u \
	<( find tests/tests -type f | zerosep | xargs -0 grep -E 'iotivity\.[_a-zA-Z0-9]*\(' | sed -r 's/^.*iotivity\.([_A-Za-z0-9]*)\(.*$/\1/'  | sort -u ) \
	<( cat src/functions.cc | grep '^  SET_FUNCTION' | sed 's/);$//' | awk '{ print $2;}' | sort )
```

### Releasing
0. The version number in ```package.json``` is structured as described [below](#version-number-structure). Increment the suffix by one.
0. Edit README.md and increment the suffix in all references containing the package version.
0. Make sure the build passes for the commitid you wish to tag.
0. Tag the commit. The name of the tag is identical to the ```version``` field in ```package.json```.
0. git push origin --tags
0. npm publish

### To build against a new upstream version:

0. Modify the ```"version"``` field in ```package.json```, setting it to the version of iotivity against which you're building.

    If <a name="version-number-structure"></a>you're building against a released version, set the version as that version. Otherwise, set the version to the next patchlevel release and append ```-pre-<commitid>``` where ```<commitid>``` is the commitid of the revision against which you're building. Add the suffix ```-n``` where ```n``` is a number starting at 0. This number represents the number of times the package has been released for a given version of iotivity. Examples of version numbers are ```0.8.0-5``` and ```0.8.1-pre-1057bc0fbad1a6fee15bad6e75f00dca5ec105ed-0```. Use the resulting version number in ```README.md``` as well as ```octbstack.pc.in```.
0. Modify the references to the version in ```README.md```.
0. Modify the value of the variable ```version``` in ```octbstack.pc.in```.
0. Build and optionally install the new version of iotivity.
0. If you haven't installed iotivity in the previous step, you must now export environment variables ```OCTBSTACK_CFLAGS``` and ```OCTBSTACK_LIBS```, because the next step needs them.
0. Run ```./update-enums.sh``` to update ```src/enums.cc``` followed by ```grunt format``` to re-format the source.
0. If the ```CFLAGS``` and/or ```LIBS``` have changed, modify ```install.sh``` and/or ```octbstack.pc.in``` and make the same modifications in ```binding.gyp```.
0. Test the build with both built-in iotivity and with iotivity via pkgconfig:

    ```sh
	nvm use 0.12 && git clean -x -d -f -f && npm run-script ci && \
	nvm use 0.10 && git clean -x -d -f -f && npm run-script ci && \
	nvm use 0.12 && git clean -x -d -f -f && ./dist.sh --testonly && \
	nvm use 0.10 && git clean -x -d -f -f && ./dist.sh --testonly
	```

    The build may fail in ```src/constants.cc``` or ```src/enums.cc```, complaining about undefined constants. That's because ```update-enums.sh``` is unaware of C precompiler definitions and so it may harvest enum values which are not actually defined under the set of precompiler flags used for building. Thus, you may need to edit ```src/enums.cc``` by hand **after** having run ```update-enums.sh```.

The script ```./update-enums.sh``` reads the C SDK header files and generates the contents of ```src/enums.cc```. Read the comments in the script before you modify ```src/enums.cc```.

[jQuery]: http://contribute.jquery.org/style-guide/js/
