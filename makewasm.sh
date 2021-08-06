#!/bin/sh
WASISDK=~/wasi-sdk-12.0
echo $WASISDK
mkdir -p build && cd build
cmake .. \
	-D CMAKE_TOOLCHAIN_FILE=$WASISDK/share/cmake/wasi-sdk.cmake \
	-D WASI_SDK_PREFIX=$WASISDK \
	-D CMAKE_SYSROOT=$WASISDK/share/wasi-sysroot \
	-D CMAKE_CXX_FLAGS=-fno-exceptions
cmake --build . --config Release --target gameboycore
cd ..

$WASISDK/bin/clang++ \
        -flto \
        -Wl,--export-all \
        -Wl,--lto-O2 \
	-fno-exceptions \
	-Wno-c++11-narrowing \
	--sysroot=$WASISDK/share/wasi-sysroot -O2 -o gb.wasm -Isrc/gameboycore/include machine-gb.c build/src/gameboycore/libgameboycore.a 

time wasm3 gb.wasm

convert -size 160x144 -depth 8 RGBA:test.rgba test.png
