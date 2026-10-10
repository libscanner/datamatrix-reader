# C++ example — static linking with libdmr.a

Requirements:

- Windows: MinGW-w64 GCC from [MSYS2](https://www.msys2.org/), **MINGW64** shell (the package is built there, not in UCRT64):
  `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-pkgconf mingw-w64-x86_64-meson`.
  MSVC cannot link `libdmr.a` — use the [C ABI example](https://github.com/libscanner/datamatrix-reader/tree/main/examples/c) instead.
- Linux: GCC 12 or newer (the package is built with GCC 12 on Debian 12), pkg-config, Meson.

Download and unpack the archive for your platform from [Releases](https://github.com/libscanner/datamatrix-reader/releases/latest), then:

```sh
meson setup build --pkg-config-path=/path/to/dmr-1.0.7-linux-x64/lib/pkgconfig
meson compile -C build
./build/test_dmr frame.jpg
./build/test_dmr frame.jpg --all     # every code in the frame
```

Without Meson — the pkg-config query must be `--static`:

```sh
export PKG_CONFIG_PATH=/path/to/dmr-1.0.7-linux-x64/lib/pkgconfig
g++ -std=c++20 -O2 main.cpp -o test_dmr $(pkg-config --static --cflags --libs dmr)
# Windows (MINGW64): add -static to link the compiler runtime into the .exe
```

Nothing needs to sit next to the program except, after purchase, the key file `license.key`
(see the [License section of the documentation](https://libscanner.com/en/docs/datamatrix-gs1/#license)).
