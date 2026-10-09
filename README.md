**English** | [Русский](https://github.com/libscanner/datamatrix-reader/blob/main/README.ru.md)

# DataMatrix Reader library (dmr)

Reads GS1 DataMatrix codes from industrial camera frames — for conveyor lines marked under Chestny ZNAK, the Russian product marking system.

`dmr` is a commercial, closed-source C++20 library with a flat C ABI, a Python package and a command-line tool. It reads **DataMatrix (ECC 200)** symbols at low capture resolution — around two pixels per module, a symbol about a hundred pixels across — and verifies every geometry hypothesis with Reed–Solomon error correction. An optional GS1 mode accepts only codes with GS1 structure (AI 01 with a valid GTIN check digit, then AI 21).

This repository contains the public headers, examples, documentation links and license texts. **It contains no library source code.** Prebuilt binaries are attached to [GitHub Releases](https://github.com/libscanner/datamatrix-reader/releases/latest).

## Key numbers

| | |
|---|---|
| **100%** | of frames read correctly (8,773 readable frames from a conveyor dataset) |
| **from 5.3 ms** | minimum time per frame; median over the dataset is 28 ms |

Try it online without installing anything: <https://libscanner.com/en/scan>

## Platforms

| Platform | C++ static library + C ABI + CLI | Python wheel |
|---|---|---|
| Windows x64 | `dmr-1.0.5-windows-x64.zip` | `libscanner_dmr-1.0.5-py3-none-win_amd64.whl` |
| Linux x86-64 | `dmr-1.0.5-linux-x64.tar.gz` | `libscanner_dmr-1.0.5-py3-none-manylinux_2_36_x86_64.whl` |
| Linux ARM64 (Raspberry Pi 3/4/5, 64-bit) | `dmr-1.0.5-linux-arm64.tar.gz` | `libscanner_dmr-1.0.5-py3-none-manylinux_2_36_aarch64.whl` |
| Linux ARMv7 (Raspberry Pi 2/3/4/5, 32-bit) | `dmr-1.0.5-linux-armhf.tar.gz` | `libscanner_dmr-1.0.5-py3-none-manylinux_2_36_armv7l.whl` |

Each platform also has a CLI-only archive (`…-cli.zip` / `…-cli.tar.gz`) with just the `dmr` tool.

- Windows packages are built with MinGW-w64 GCC (MSYS2 MINGW64). MSVC and other compilers use the C ABI (`dmr_c.dll`).
- Linux packages are built with GCC 12 on Debian 12 (glibc 2.36): GCC 12 or newer is required for the C++ API; wheels need glibc 2.36+ (Debian 12 / Raspberry Pi OS Bookworm or newer).
- Python 3.9 or newer, no dependencies.

## Download

- **Binaries:** [GitHub Releases → latest](https://github.com/libscanner/datamatrix-reader/releases/latest) — archives for each platform and Python wheels.
- **Python:** [PyPI → libscanner-dmr](https://pypi.org/project/libscanner-dmr/) — `pip install libscanner-dmr`, imported as `dmr`.
- **Licenses:** buy a plan at <https://libscanner.com/en/> — 3 months, 6 months, 1 year or perpetual (for example <https://libscanner.com/en/products/datamatrix-gs1-yearly>). The activation key appears in your account after purchase.

### Trial

Every build works for **30 days from the first run on a machine** — no registration, no key, no network access during the trial. After that a license key from <https://libscanner.com> is required; until then `scan()` returns `Status::Unlicensed` without processing the frame.

## Package contents

```text
include/dmr/{Dmr,Diagnostics,Geometry,Image,License,Scanner,Settings,Version}.h   C++ API
include/dmr/dmr_c.h                C ABI
lib/libdmr.a                       C++ static library
lib/pkgconfig/dmr.pc
lib/libdmr_c.so | bin/dmr_c.dll    C ABI library (+ lib/dmr_c.dll.a on Windows)
lib/pkgconfig/dmr_c.pc
bin/dmr                            command-line tool
share/doc/dmr/licenses/            third-party licenses
```

The headers in [`include/`](https://github.com/libscanner/datamatrix-reader/tree/main/include/dmr) of this repository declare exactly the same API as the headers shipped in the 1.0.5 packages; only the comments differ (translated to English here, Russian in the packages).

## Quick start

### C++

```cpp
#include <dmr/Dmr.h>
#include <iostream>

int main() {
    dmr::Image frame;
    if (dmr::loadImage("frame.jpg", frame) != dmr::LoadStatus::Ok) {
        std::cerr << "could not read the file\n";
        return 1;
    }

    dmr::Scanner scanner;                          // frame stream of one label
    const dmr::ScanResult r = scanner.scan(frame);  // Image converts implicitly to ImageView

    if (r.ok()) {
        std::cout << r.text() << "\n";
    } else if (r.status == dmr::Status::NotFound) {
        std::cout << "no symbol in the frame\n";
    } else {
        std::cout << "symbol found but could not be read\n";
    }
}
```

Link statically through pkg-config — the query **must** be `--static`:

```sh
PKG_CONFIG_PATH=/path/to/dmr-1.0.5-linux-x64/lib/pkgconfig pkg-config --static --cflags --libs dmr
```

In Meson:

```meson
dmr_dep = dependency('dmr', required: true, static: true)
executable('myprogram', 'main.cpp', dependencies: [dmr_dep],
  link_args: ['-static'])   # Windows: MinGW runtime linked into the program
```

```sh
meson setup build --pkg-config-path="$PWD/dmr-1.0.5-windows-x64/lib/pkgconfig"
meson compile -C build
```

A frame from your own capture pipeline (camera SDK, OpenCV buffer, bitmap) is passed as a view over the existing buffer, without copying:

```cpp
const dmr::ImageView view{ m.data, m.cols, m.rows, (int)m.step, dmr::PixelFormat::Bgr8 };
const dmr::ScanResult r = scanner.scan(view);
```

Complete example: [`examples/cpp`](https://github.com/libscanner/datamatrix-reader/tree/main/examples/cpp). C ABI example (MSVC, other languages): [`examples/c`](https://github.com/libscanner/datamatrix-reader/tree/main/examples/c).

### Python

```sh
pip install libscanner-dmr     # PyPI: Windows x64, Linux x86-64 / ARM64 / ARMv7 (glibc 2.36+)
```

```python
import dmr

img = dmr.Image("frame.jpg", dmr.LoadAs.GRAY)  # the scanner does not need color
scanner = dmr.Scanner()                         # frame stream, the default
result = scanner.scan(img.view())
if result.ok:
    print(result.text.decode("utf-8"))
```

`Code.text` is `bytes`: the GS1 separator stays as byte `0x1D`. The wheel also installs the `dmr` command. Example: [`examples/python`](https://github.com/libscanner/datamatrix-reader/tree/main/examples/python).

### Command line

```sh
dmr label.jpg
dmr --folder ./frames
dmr --license status --json
```

More commands: [`examples/cli`](https://github.com/libscanner/datamatrix-reader/tree/main/examples/cli).

## Documentation

- C++ / C ABI / CLI: <https://libscanner.com/en/docs/datamatrix-gs1/> (Russian: <https://libscanner.com/docs/datamatrix-gs1/>)
- Python: <https://libscanner.com/en/docs/datamatrix-gs1/?lang=python> (Russian: <https://libscanner.com/docs/datamatrix-gs1/?lang=python>)

## License

Proprietary — see [LICENSE.txt](https://github.com/libscanner/datamatrix-reader/blob/main/LICENSE.txt) (End User License Agreement; the Russian text is binding, [LICENSE.en.txt](https://github.com/libscanner/datamatrix-reader/blob/main/LICENSE.en.txt) is an English translation for information). Starting to use the software means accepting the agreement.

Third-party components: [THIRD-PARTY-NOTICES.txt](https://github.com/libscanner/datamatrix-reader/blob/main/THIRD-PARTY-NOTICES.txt) and the license texts in [`licenses/`](https://github.com/libscanner/datamatrix-reader/tree/main/licenses).

Code in [`examples/`](https://github.com/libscanner/datamatrix-reader/tree/main/examples) is licensed under MIT-0 ([examples/LICENSE](https://github.com/libscanner/datamatrix-reader/blob/main/examples/LICENSE)) — copy it into your projects freely; the library itself is proprietary (LICENSE.txt).

## Contact

- Email: [libscanner@yandex.com](mailto:libscanner@yandex.com)
- Feedback form: <https://libscanner.com/en/feedback>
- Security issues: see [SECURITY.md](https://github.com/libscanner/datamatrix-reader/blob/main/SECURITY.md)

“GS1” is a trademark of GS1 AISBL. “Chestny ZNAK” («Честный знак») is a trademark of Operator-CRPT LLC (ООО «Оператор-ЦРПТ»). This product is not affiliated with or certified by the owners of these marks; the names are used only to describe the supported format.
