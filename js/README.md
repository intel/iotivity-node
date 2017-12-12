# `iotivity-node` Examples

## COAPS

The files `high-level-client.coaps.js` and `high-level-server.coaps.js` are for
a client/server pair illustrating DTLS access to a resource. To run the example,
please follow the steps below:

0. In addition to `iotivity-node`, build `iotivity` with `SECURED=1` and
    `RD_MODE=all`. Let the root of the `iotivity` repository be `$iotivityRoot`.

0. Calculate the sha256 sum of the server. The example below assumes the server
    is located in `/home/user/iotivity-node/js/high-level-server.coaps.js`.
    Replace the path in the example below with the actual absolute path to the
    file `high-level-server.coaps.js`.

    ```sh
    echo -n '/home/user/iotivity-node/js/high-level-server.coaps.js' | sha256sum
    ```

    Copy the resulting value to the clipboard. Let the value be `$serverHash`.

0. Create the directory `${HOME}/.iotivity-node/$serverHash`.

    ```sh
    mkdir -p ~/.iotivity-node/$serverHash
    ```

0. Copy the file
    `$iotivityRoot/resource/csdk/security/provisioning/sample/oic_svr_db_server_justworks.dat`
    to `${HOME}/.iotivity-node/$serverHash/oic_svr_db.dat`

0. Start the server

    ```sh
    node high-level-server.coaps.js
    ```

0. Switch to the directory where iotivity built the provisioning client, and run
    it from there. Let `$platform` be the platform for which you build iotivity,
    `$architecture` be the architecture for which you built iotivity, and
    `$buildType` be the build type - i.e., `release` for a production build, and
    `debug` for a development build.

    ```sh
    cd $iotivityRoot/out/$platform/$architecture/$buildType/resource/csdk/security/provisioning/sample
    ./provisioningclient
    ```

0. Choose 11 from the menu by typing `11` and pressing `Enter` to discover
    unowned devices. The server should be listed under "Discovered Unowned
    Devices" with uuid `12345678-1234-1234-1234-123456789012`.

0. Choose 20 from the menu by typing `20` and pressing `Enter` to assume
    ownership of the devices discovered in the previous step.

0. Choose 99 to exit the client.

0. Calculate the sha256 sum of the client. The example below assumes the client
    is located in `/home/user/iotivity-node/js/high-level-client.coaps.js`.
    Replace the path in the example below with the actual absolute path to the
    file `high-level-client.coaps.js`.

    ```sh
    echo -n '/home/user/iotivity-node/js/high-level-client.coaps.js' | sha256sum
    ```

    Copy the resulting value to the clipboard. Let the value be `$clientHash`.

0. Create the directory `${HOME}/.iotivity-node/$clientHash`.

    ```sh
    mkdir -p ~/.iotivity-node/$clientHash
    ```

0. Copy the file `oic_svr_db_client.dat` from the current directory (in which
    the `provisioningclient` program was built) to the newly created directory
    and rename it to `oic_svr_db.dat`.

    ```sh
    cp ./oic_svr_db_client.dat ${HOME}/.iotivity-node/$clientHash/oic_svr_db.dat
    ```

0. Launch the client.

    ```sh
    node high-level-client.coaps.js
    ```
