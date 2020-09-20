# GWebSocket

A websocket module for Garry's Mod

# Installing

You have two options:
1. Download the compiled module in this repo, should work for most linux machines running 32 bit srcds
2. Build it yourself

## Using the ready made module
1. Click on `gmsv_gwebsocket_linux.dll` above
2. Click on the download button
3. Stick it in your servers `lua/bin` folder, if it doesnt exist, create it.

## Building it yourself

This project uses premake5 to generate makefiles and currently only supports 32 bit linux builds

1. Clone the repository `git clone --recurse-submodules https://github.com/norrbotten/gwebsocket.git`
2. Generate makefiles `premake5 gmake2`
3. Make the module `cd makefiles; make module config=release`
4. Your compiled and ready module is now in `build/module/bin`

### Dependencies

* [IXWebSocket](https://github.com/machinezone/IXWebSocket/)
* [lunar-sol2](https://github.com/norrbotten/lunar-sol2)

IXWebSocket needs to be built for 32 bit and installed, for this you will additionally need
32 bit builds of OpenSSL and ZLib.

# Using it in Garry's Mod

1. Require it in your script `require("gwebsocket")`
2. This sets a global variable called `gwebsocket`, so don't overwrite it (blame garry for no returns from require)
3. See the examples
