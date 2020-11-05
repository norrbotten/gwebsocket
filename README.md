# GWebSocket

A websocket module for Garry's Mod

# Installing

You have two options:
1. Download the compiled module in this repo, should work for most linux machines running 32 bit srcds
2. Build it yourself

Currently you should build the module yourself, since the current release doesn't clean up listener threads after lua GC's the websocket instance.

## Using the ready made module
1. Click on `gmsv_gwebsocket_linux.dll` above
2. Click on the download button
3. Stick it in your servers `lua/bin` folder, if the folder doesnt exist, create it.

## Building it yourself

This project uses premake5 to generate makefiles and currently only supports 32 bit linux builds

0. If you are on a 64 bit system, make sure your compiler is setup for crosscompiling to 32 bit. If you use GCC/G++, install the `gcc-multilib` and `g++-multilib` packages with your favorite package manager.
1. Clone the repository `git clone --recurse-submodules https://github.com/norrbotten/gwebsocket.git`
2. Run the `setup_linux.sh` script. It will download and build all dependencies.
2. Generate makefiles `premake5 gmake2`
3. Make the module `cd build && make -j config=release && cd ..`
4. Your ready to use module is now in `build/module/bin`. Copy it to your `lua/bin` folder.

### Dependencies

* [IXWebSocket](https://github.com/machinezone/IXWebSocket/)
* [lunar-sol2](https://github.com/norrbotten/lunar-sol2)

IXWebSocket needs to be built for 32 bit and installed, for this you will additionally need
32 bit builds of OpenSSL and ZLib. Assuming you're on linux, these can be automatically
downloaded and built using the provided script.

# Using it in Garry's Mod

1. Require it in your script `require("gwebsocket")`
2. This sets a global variable called `gwebsocket`, so don't overwrite it (blame garry for no returns from require)
3. See the examples. Since i actually now have a reason to not abandon this project, usage is subject to change.
