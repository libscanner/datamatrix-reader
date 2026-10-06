#pragma once

// ═══════════════════════════════════════════════════════════════════════════
//  dmr/Image.h — the library's input frame and images loaded from disk
//
//  A frame is accepted as an ImageView: pointer, width, height, row stride
//  and pixel format. Not a cv::Mat — this is a deliberate design decision.
//
//  The public headers do not depend on OpenCV: with cv::Mat in the API, every
//  caller would need OpenCV headers and a matching ABI just to pass a frame.
//
//  The five fields of ImageView are enough for everyone: this is exactly what
//  a camera delivers and exactly what lies inside a cv::Mat, a QImage or a
//  Windows bitmap. There is no copying in either direction — the view over a
//  caller-owned buffer is built at the call site:
//
//      const dmr::ImageView view{ m.data, m.cols, m.rows, (int)m.step,
//                                 dmr::PixelFormat::Bgr8 };
//
//  There is intentionally no cv::Mat bridge header: it would add OpenCV to the
//  package dependencies for the sake of one line the caller writes anyway.
// ═══════════════════════════════════════════════════════════════════════════

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace dmr {

/// Pixel byte layout. Eight bits per channel — the library expects nothing else.
///
/// The scanner works on luminance and converts a color frame to it first. If
/// the camera is monochrome, pass Gray8 rather than gray replicated into three
/// channels: the result is the same, and the frame is processed faster.
enum class PixelFormat {
    Gray8,   ///< one channel: luminance; the cheapest for the scanner
    Bgr8,    ///< three channels, OpenCV order
    Rgb8,    ///< three channels, the order used by most other libraries
    Bgra8,   ///< four channels, OpenCV and Windows order
    Rgba8,   ///< four channels
};

/// Number of channels per pixel in this format.
constexpr int channelsOf(PixelFormat f) {
    switch (f) {
        case PixelFormat::Gray8:                        return 1;
        case PixelFormat::Bgr8:  case PixelFormat::Rgb8: return 3;
        case PixelFormat::Bgra8: case PixelFormat::Rgba8: return 4;
    }
    return 0;
}

/// A view over a CALLER-OWNED buffer: the library does not copy it, free it or
/// keep it. The buffer must stay alive for the whole duration of the call.
struct ImageView {
    const uint8_t* data   = nullptr;
    int            width  = 0;
    int            height = 0;

    /// Bytes from the start of one row to the start of the next. ZERO means
    /// "tightly packed", i.e. width * channels. A separate field is needed
    /// because camera frames are almost never tightly packed: rows are
    /// aligned, and a crop of a larger frame inherits the original row stride.
    int stride = 0;

    PixelFormat format = PixelFormat::Gray8;

    bool empty()    const { return data == nullptr || width <= 0 || height <= 0; }
    int  channels() const { return channelsOf(format); }
    /// Row length in bytes, with zero already resolved.
    int  rowBytes() const { return stride > 0 ? stride : width * channels(); }

    const uint8_t* row(int y) const {
        return data + static_cast<std::ptrdiff_t>(y) * rowBytes();
    }
};

/// A frame owned by the library. Needed where the library creates the frame,
/// i.e. when loading from disk.
///
/// Built on std::vector rather than cv::Mat: otherwise the public type would
/// again depend on OpenCV, this time through its size and field layout.
class Image {
public:
    Image() = default;
    Image(int width, int height, PixelFormat format) { reset(width, height, format); }

    /// Allocate a buffer for the frame; previous content is lost. Tightly packed.
    void reset(int width, int height, PixelFormat format);

    /// Release the memory and become empty.
    void clear();

    int         width()    const { return w_; }
    int         height()   const { return h_; }
    int         stride()   const { return stride_; }
    PixelFormat format()   const { return fmt_; }
    int         channels() const { return channelsOf(fmt_); }
    bool        empty()    const { return buf_.empty(); }

    uint8_t*       data()       { return buf_.data(); }
    const uint8_t* data() const { return buf_.data(); }
    uint8_t*       row(int y)       { return buf_.data() + (std::ptrdiff_t)y * stride_; }
    const uint8_t* row(int y) const { return buf_.data() + (std::ptrdiff_t)y * stride_; }

    ImageView view() const { return ImageView{ buf_.data(), w_, h_, stride_, fmt_ }; }

    /// Implicit conversion — so an owning frame can be passed where a view is
    /// expected without calling .view() every time.
    operator ImageView() const { return view(); }

private:
    std::vector<uint8_t> buf_;
    int         w_      = 0;
    int         h_      = 0;
    int         stride_ = 0;
    PixelFormat fmt_    = PixelFormat::Gray8;
};

// ── Paths and encodings ────────────────────────────────────────────────────
//
// These helpers are about the operating system rather than the command line,
// and anyone who reads images from disk needs them: on Windows, a directory
// with a non-ASCII name would otherwise break image loading.

/// A path from a narrow string (a command-line argument, a configuration field).
///
/// A narrow string arrives in the SYSTEM code page (e.g. cp1251 on Russian
/// Windows), while std::filesystem converts it to a wide string through the
/// C++ locale, "C" by default: that locale knows nothing beyond ASCII, and a
/// directory with a non-ASCII name crashes the program with an "Illegal byte
/// sequence" exception. Here the conversion is done explicitly, using the
/// system character locale.
std::filesystem::path toPath(const std::string& s);

/// The reverse: a path to a narrow string in the system code page.
///
/// Symmetric to toPath. On Windows the image decoder opens files through the
/// ANSI API, while path::string() converts through the C++ locale and loses
/// non-ASCII characters — a file just listed from a directory would then fail
/// to open.
std::string fromPath(const std::filesystem::path& p);

/// A path for PRINTING — in UTF-8.
///
/// Captions are printed in UTF-8, while paths from directory listings come in
/// the system code page. Mixing two encodings in one stream is not acceptable:
/// a report parsed later by someone would be half unreadable.
std::string printablePath(const std::filesystem::path& p);

// ── Loading from disk ──────────────────────────────────────────────────────

/// Outcome of an attempt to read a file.
///
/// Two different failures under one message are confusing: a typo in the path
/// looks the same as a corrupted file, and time is wasted in the wrong place.
/// So the library distinguishes them, rather than each caller in its own way.
enum class LoadStatus {
    Ok,          ///< image loaded
    NotFound,    ///< the file does not exist
    Unreadable,  ///< the file exists but is not a readable image
    TooLarge,    ///< size from the header exceeds ImageLimits; pixels were not read
};

// ── Frame size limit ───────────────────────────────────────────────────────

/// The largest frame the library agrees to process.
///
/// The size in a PNG or JPEG header costs the sender nothing: a file of a few
/// dozen bytes can declare a 1 000 000 x 1 000 000 image, and the decoder will
/// dutifully try to allocate memory for it. The limit is checked against the
/// HEADER, before memory for pixels is allocated, and the same values are
/// checked on entry to Scanner::scan().
///
/// The defaults leave a large margin over industrial cameras (the largest
/// sensors are around 65 MP); a frame larger than that is not a camera image.
/// Callers who need more raise the limit explicitly.
struct ImageLimits {
    /// Maximum side, pixels; 0 — kHardMaxSide.
    int maxSide = 16384;
    /// Maximum area, pixels; 0 — kHardMaxPixels.
    long long maxPixels = 100'000'000;

    /// Hard ceiling that cannot be raised by settings: beyond it row and frame
    /// sizes overflow int (width x 4 channels, area x 4 bytes).
    static constexpr int       kHardMaxSide   = 65535;
    static constexpr long long kHardMaxPixels = 1LL << 29;

    /// Whether a w x h frame fits within the limit. Non-positive sizes do not.
    bool allows(long long w, long long h) const {
        if (w <= 0 || h <= 0) return false;
        const long long side = (maxSide > 0 && maxSide < kHardMaxSide) ? maxSide : kHardMaxSide;
        const long long area = (maxPixels > 0 && maxPixels < kHardMaxPixels) ? maxPixels
                                                                              : kHardMaxPixels;
        return w <= side && h <= side && w * h <= area;
    }
};

/// How to load an image.
///
/// A separate enumeration rather than PixelFormat: the decoder supports
/// exactly these two forms, and offering five values of which two work would
/// mislead the caller.
///
/// For the scanner Gray is the best choice: it does not need color and reduces
/// the frame to luminance first. An image from a monochrome camera (grayscale
/// in a three-channel JPEG) loads faster as grayscale and the scanner skips the
/// conversion; the result is byte-for-byte the same — each Color channel is
/// that same luminance.
enum class LoadAs {
    Color,  ///< three channels, PixelFormat::Bgr8
    Gray,   ///< one channel, PixelFormat::Gray8
};

/// Load an image from disk, handling the path encoding.
///
/// On failure out is cleared: half a frame from a previous attempt is worse
/// than an empty one. A frame larger than limits is not decoded:
/// LoadStatus::TooLarge.
LoadStatus loadImage(const std::string& path, Image& out,
                     LoadAs as = LoadAs::Color,
                     const ImageLimits& limits = ImageLimits{});

} // namespace dmr
