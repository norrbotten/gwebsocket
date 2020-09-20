
DIR=$(pwd)

# download zlib and openssl

cd dependencies

if [ -d "./zlib" ]; then
    rm -rf zlib
fi

if [ -d "./openssl" ]; then
    rm -rf openssl
fi

curl -LO https://zlib.net/zlib-1.2.11.tar.gz && tar -xzvf zlib-1.2.11.tar.gz && mv zlib-1.2.11 zlib
curl -LO https://www.openssl.org/source/openssl-1.1.1g.tar.gz && tar -xzvf openssl-1.1.1g.tar.gz && mv openssl-1.1.1g openssl

rm *.tar.gz

# build them for 32 bit

cd zlib
CFLAGS=-m32 ./configure
make -j

cd ../openssl
./Configure -m32 linux-generic32
make -j

# build ixwebsocket

cd ../ixwebsocket

if [ -d "./build" ]; then
    rm -rf build
fi

mkdir build && cd build

cmake .. -DUSE_TLS=1 -DCMAKE_CXX_FLAGS=-m32

export LD_LIBRARY_PATH=$DIR/dependencies/zlib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
make -j

cd ../../..

# build the project itself

if [ -d "./build" ]; then
    rm -rf build
fi

premake5 gmake2

pwd

cd build

make -j config=release && cp module/bin/gmsv_gwebsocket_linux.dll ../.
