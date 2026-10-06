#pragma once

// ═══════════════════════════════════════════════════════════════════════════
//  dmr/Scanner.h — the library entry point
//
//      dmr::Scanner scanner;                       // stream of frames
//      const dmr::ScanResult r = scanner.scan(frame);
//      if (r.ok()) use(r.text());
//
//  The scanner is an OBJECT, not a function, for a reason. The symbol size
//  hint accumulates across frames and must outlive them (Settings::mcHintRun),
//  and the thread pool size is a process-wide setting. Creating a scanner per
//  frame throws both away.
//
//  Still, scan() is const and stores nothing in the object: the result depends
//  on the frame, not on what this instance has read before. The only exception
//  is that hint, and it is harmless: the hint only adds an attempt, and
//  Reed-Solomon acceptance does not depend on it. Candidates are processed in
//  parallel internally, so there is no need for your own thread pool on top of
//  scan().
// ═══════════════════════════════════════════════════════════════════════════

#include <memory>
#include <string>
#include <vector>

#include "Geometry.h"
#include "Image.h"
#include "Settings.h"

namespace dmr {

/// Outcome of processing a frame.
enum class Status {
    Ok         = 0,  ///< at least one code decoded
    NotFound   = 1,  ///< no symbol found on the frame
    NotDecoded = 2,  ///< a symbol was found but not decoded
    Unlicensed = 3,  ///< frame not processed: no trial period and no
                     ///< license; see dmr::license::status() for why
    ImageTooLarge = 4, ///< frame exceeds Settings::imageLimits, not processed
    InvalidImage  = 5, ///< inconsistent frame description: row stride
                       ///< smaller than the width, unknown pixel format
    Failed        = 6, ///< processing aborted by an internal error (e.g.
                       ///< out of memory); the process stays alive
};

/// Breakdown of a successful or failed read: per-module layout, codewords,
/// syndromes. The type is OPAQUE — it holds library internals. Filled only
/// when Settings::collectDetails is set.
struct Details;

/// One decoded code.
struct Code {
    /// Decoded string. Bytes as is: GS separators stay as 0x1D bytes and are
    /// not replaced with a printable form.
    std::string text;

    /// Where the symbol was found on the ORIGINAL frame: the candidate's
    /// bounding rectangle.
    Box box;

    /// The four symbol corners on the same frame. More precise than the box
    /// and therefore provided separately: a symbol on a conveyor is captured
    /// at an angle and cannot be described by a rectangle. For the corner
    /// order see dmr/Geometry.h.
    Quad corners;

    /// Symbol side in modules.
    int moduleCount = 0;
};

struct ScanResult {
    Status status = Status::NotFound;

    /// All decoded codes, one per symbol, in reading order: top to bottom,
    /// left to right for equal top edges. Empty unless status is Ok. With
    /// Settings::maxCodes = 1 holds at most one element.
    std::vector<Code> codes;

    /// The search was cut short by a time limit: the budget
    /// (Settings::budgetMs) or the safety timeout (Settings::timeoutMs).
    ///
    /// Set only when the limit actually cut something off. Success with the
    /// flag set is possible: a code was decoded but some hypotheses were not
    /// checked — this is the best found by the deadline, not everything that
    /// could have been found. Without the flag an empty result due to the
    /// deadline is indistinguishable from an honest "no code on the frame",
    /// and the two call for different decisions.
    bool timedOut = false;

    /// Frame processing time, ms. Excludes loading the image from disk: that
    /// is typically far more expensive than the scan itself, and merging the
    /// two into one number would hide what the caller is paying for.
    double elapsedMs = 0.0;

    /// See Details. Empty unless requested.
    std::shared_ptr<const Details> details;

    bool ok() const { return status == Status::Ok; }

    /// The first code or an empty string — so a caller that needs one code
    /// does not have to write codes.empty() ? "" : codes[0].text every time.
    const std::string& text() const;
};

class Scanner {
public:
    explicit Scanner(Settings s = Settings::stream());
    ~Scanner();

    Scanner(Scanner&&) noexcept;
    Scanner& operator=(Scanner&&) noexcept;
    Scanner(const Scanner&)            = delete;
    Scanner& operator=(const Scanner&) = delete;

    /// Read the codes on a frame.
    ///
    /// @param frame        the whole frame, color or grayscale
    /// @param moduleCount  symbol side in modules; 0 — auto-detect
    ScanResult scan(ImageView frame, int moduleCount = 0) const;

    /// Symbol size suggested by previous frames; 0 — no hint.
    int moduleCountHint() const;

    /// Forget the hint.
    ///
    /// A wrong hint is dropped automatically after a few frames, which is the
    /// right behavior for a camera. But a caller who KNOWS the label has
    /// changed or a new batch has started should not have to wait for those
    /// frames.
    void resetStream();

    const Settings& settings() const;

private:
    struct Impl;
    std::unique_ptr<Impl> p_;
};

/// One-off scan without an object: a probe, a script, a single file.
///
/// Strictly one-off: the per-stream size hint does not and cannot work here —
/// there is nothing to accumulate it from.
ScanResult scan(ImageView frame, const Settings& s = Settings::single());

} // namespace dmr
