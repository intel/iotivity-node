# Examples

This directory contains examples of using the bindings. You can copy the files into an environment
where iotivity and iotivity-node are installed.

**NOTE:** Please do not copy this directory in its entirety. Copy individual files only. The
reason for this is that this directory contains a fake [node_modules/](./node_modules/)
subdirectory that causes node to resolve the "iotivity" package to "../../", which is incorrect in
most cases. Nevertheless, this setup makes it possible to require iotivity-node via
```require( "iotivity" );``` from the examples, which makes the examples more true-to-life.
