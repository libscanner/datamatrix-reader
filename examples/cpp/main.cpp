// SPDX-License-Identifier: MIT-0
// Minimal dmr example: read DataMatrix codes from an image file.
//
//   test_dmr <image> [--all]
//
// --all looks for every code in the frame instead of the first one.

#include <dmr/Dmr.h>

#include <cstring>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <image> [--all]\n";
        return 2;
    }
    const bool all = argc > 2 && std::strcmp(argv[2], "--all") == 0;

    std::cout << "dmr " << dmr::version() << " (headers " << DMR_VERSION_STRING << ")\n";

    // Trial period or license: without either, scan() returns Status::Unlicensed.
    const dmr::LicenseStatus lic = dmr::license::status();
    if (!lic.canScan()) {
        std::cerr << lic.message << "\n";
        return 3;
    }

    // Gray is the cheapest input for the scanner; the result is the same as for Color.
    dmr::Image frame;
    switch (dmr::loadImage(argv[1], frame, dmr::LoadAs::Gray)) {
        case dmr::LoadStatus::Ok:         break;
        case dmr::LoadStatus::NotFound:   std::cerr << "no such file\n"; return 1;
        default:                          std::cerr << "not a readable image\n"; return 1;
    }

    // Single images: Settings::single() tries all symbol sizes.
    // For a camera stream keep one Scanner with the default Settings::stream().
    const dmr::Scanner scanner(all ? dmr::Settings::multi(0) : dmr::Settings::single());
    const dmr::ScanResult r = scanner.scan(frame);

    if (!r.ok()) {
        std::cout << (r.status == dmr::Status::NotFound ? "no symbol in the frame\n"
                                                        : "symbol found but could not be read\n");
        return 1;
    }

    for (const dmr::Code& c : r.codes) {
        // c.text holds raw bytes: the GS1 separator stays as byte 0x1D.
        std::cout << c.moduleCount << "x" << c.moduleCount << " at (" << c.box.x << ", " << c.box.y
                  << "): " << c.text << "\n";
    }
    std::cout << r.elapsedMs << " ms\n";
    return 0;
}
