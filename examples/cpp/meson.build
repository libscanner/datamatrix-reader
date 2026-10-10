# SPDX-License-Identifier: MIT-0
# Build the dmr C++ example against a prebuilt package from GitHub Releases.
#
#   meson setup build --pkg-config-path=/path/to/dmr-1.0.7-<platform>/lib/pkgconfig
#   meson compile -C build
#
# The .pc files in the package are relocatable: the unpack directory can be anywhere.

project('test_dmr', 'cpp',
  version : '0.1.0',
  default_options : ['cpp_std=c++20', 'warning_level=3', 'buildtype=release'])

# libdmr.a is a static library: the dependency must be static so that
# Libs.private (zlib, libpng, libjpeg-turbo, system libraries) is linked too.
dmr_dep = dependency('dmr', required : true, static : true)

link_args = []
if host_machine.system() == 'windows'
  # Link the MinGW runtime into the program; otherwise libstdc++-6.dll,
  # libgcc_s_seh-1.dll and libwinpthread-1.dll must sit next to the .exe.
  link_args += ['-static']
endif

executable('test_dmr', 'main.cpp',
  dependencies : dmr_dep,
  link_args : link_args)
