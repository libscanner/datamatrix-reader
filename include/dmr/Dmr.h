#pragma once

// ═══════════════════════════════════════════════════════════════════════════
//  dmr/Dmr.h — the entire public API in one include
//
//  Reads DataMatrix codes (ECC200, ISO/IEC 16022) from frames with about two
//  pixels per module: conveyor lines, cameras, labels.
//
//      #include <dmr/Dmr.h>
//
//      dmr::Scanner scanner;                        // stream of frames
//      dmr::Image   frame;
//      if (dmr::loadImage("frame.jpg", frame) == dmr::LoadStatus::Ok) {
//          const dmr::ScanResult r = scanner.scan(frame);
//          if (r.ok()) use(r.text());
//      }
//
//  A frame from a caller-owned buffer — no copy, just a view over it:
//
//      const dmr::ImageView view{ ptr, w, h, stride, dmr::PixelFormat::Bgr8 };
//
//  Headers can also be included individually, which is sometimes useful:
//  Settings.h and Scanner.h pull in <functional> and <vector>, while code that
//  only builds a frame view needs just Image.h.
// ═══════════════════════════════════════════════════════════════════════════

#include "Diagnostics.h"
#include "Geometry.h"
#include "Image.h"
#include "License.h"
#include "Scanner.h"
#include "Settings.h"
#include "Version.h"
