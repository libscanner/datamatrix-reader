#pragma once

// ── Result geometry ───────────────────────────────────────────────────────
//
// Library-own types rather than cv::Rect, for the same reason the frame is
// accepted as a view rather than a cv::Mat (dmr/Image.h): the public headers
// do not depend on OpenCV.
//
namespace dmr {

/// A point on the frame, in pixels. Fractional: symbol corners are located with
/// sub-module precision, and rounding to integers would lose exactly the
/// precision they are computed for.
struct Point {
    float x = 0.0f;
    float y = 0.0f;
};

/// A rectangle on the ORIGINAL frame, in pixels.
///
/// It has a practical purpose: two decoded candidates with the same string
/// are one symbol seen twice if their boxes overlap, and two identical labels
/// if they do not. Without coordinates they cannot be told apart.
struct Box {
    int x      = 0;
    int y      = 0;
    int width  = 0;
    int height = 0;

    bool empty() const { return width <= 0 || height <= 0; }
};

/// The four corners of the symbol on the ORIGINAL frame, in circular order.
///
/// A box says "somewhere here", a quad says exactly where: a symbol on a
/// conveyor is captured at an angle and cannot be described by an
/// axis-aligned rectangle. Useful for drawing overlays on the frame or
/// measuring localization quality.
///
/// ORDER. The corners go around the symbol, and index 3 holds the corner the
/// detector took for the corner of the solid L-shaped finder pattern. That is
/// the DETECTOR's decision, made from edge density before any decoding; the
/// final symbol orientation is chosen later and may differ. So corner[3] can
/// be used for drawing, but not for conclusions about the content.
struct Quad {
    Point corner[4];
};

} // namespace dmr
