# C example — dynamic linking with dmr_c (C ABI)

`dmr_c` is the same library behind a flat C interface (`dmr/dmr_c.h`). The compiler runtime is linked into it
statically, so it works with MSVC, Clang, GCC and any language that can call C functions.

## Linux

```sh
export PKG_CONFIG_PATH=/path/to/dmr-1.0.4-linux-x64/lib/pkgconfig
cc main.c -o test_dmr_c $(pkg-config --cflags --libs dmr_c) -Wl,-rpath,'$ORIGIN'
cp /path/to/dmr-1.0.4-linux-x64/lib/libdmr_c.so .
./test_dmr_c frame.jpg
```

## Windows, MinGW-w64 GCC

```sh
export PKG_CONFIG_PATH=/c/path/to/dmr-1.0.4-windows-x64/lib/pkgconfig
gcc main.c -o test_dmr_c.exe $(pkg-config --cflags --libs dmr_c)
cp /c/path/to/dmr-1.0.4-windows-x64/bin/dmr_c.dll .
```

## Windows, MSVC

The package has no `.lib` file; generate the import library from the DLL. In an MSYS2 shell:

```sh
gendef dmr_c.dll          # package mingw-w64-x86_64-tools; writes dmr_c.def
```

Then in the Developer Command Prompt for VS (x64):

```bat
lib /def:dmr_c.def /machine:x64 /out:dmr_c.lib
cl /I C:\path\to\dmr-1.0.4-windows-x64\include main.c dmr_c.lib
```

At run time `dmr_c.dll` must be next to the `.exe` (or on `PATH`); after purchase the key file `license.key`
goes next to `dmr_c.dll` / `libdmr_c.so`.

Full C ABI reference: <https://libscanner.com/en/docs/datamatrix-gs1/#install-dynamic>
