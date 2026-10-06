#pragma once

// ═══════════════════════════════════════════════════════════════════════════
//  dmr/Settings.h — scanner operating mode
//
//  Every default here was chosen by measurement, not by taste; the key
//  reasoning is summarized in the comment to each field.
//
//  That is why PRESETS come first, not fields. Knowledge such as "trying
//  several symbol sizes suits a folder of images and hurts a camera stream"
//  is useless if the caller never reads it and ends up with a default that
//  does not fit their case. A preset carries that knowledge in its name.
//
//  What is NOT here: the hypothesis ladder — the set of rectification
//  geometries, grid refinement modes and blending ratios tried on a frame that
//  failed to decode. That is the internal search strategy, not an operating
//  mode; it may change between versions and is not part of the API contract.
// ═══════════════════════════════════════════════════════════════════════════

#include <string>
#include <vector>

#include "Image.h"   // ImageLimits

namespace dmr {

struct Settings {
    // ── Presets ────────────────────────────────────────────────────────────

    /// A stream of frames with ONE kind of label: a conveyor, a camera. Default.
    ///
    /// The symbol size in such a stream does not change for hours, so the hint
    /// from previous frames is on (mcHintRun), and trying multiple sizes is
    /// off: in a camera stream it practically never adds a read, while every
    /// failed frame pays for it in time.
    static Settings stream();

    /// One-off images: a folder, a single file, a manual check.
    ///
    /// Frames are unrelated, so there is no per-stream hint; trying multiple
    /// sizes is on instead — it recovers exactly those frames that otherwise
    /// fail to decode. The cost is known and high: a failed frame takes
    /// several times longer (seconds instead of fractions of a second).
    static Settings single();

    /// Several codes per frame.
    ///
    /// Several times more expensive, and not because of extra decode attempts:
    /// early exit is the main saving on a conveyor, as the vast majority of
    /// frames are decoded on the first or second candidate. Looking for several
    /// codes means going through all candidates and paying the cost of a
    /// failure on EVERY frame. Best combined with budgetMs.
    ///
    /// @param maxCodes  0 — as many as found, N — at most N.
    static Settings multi(int maxCodes = 0);

    // ── What to look for ───────────────────────────────────────────────────

    /// How many codes to look for per frame: 1 — one (default), 0 — as many as
    /// found, N — at most N. For the cost see multi().
    int maxCodes = 1;

    /// Accept only GS1-formatted marking codes.
    ///
    /// A successful Reed-Solomon correction does NOT prove a correct read: with
    /// many erasures it can converge to a different valid codeword. This has
    /// been observed: the reader returned success with garbage, and neither
    /// the scheme structure nor the pad check per the standard rejected it. The
    /// check looks at the format: AI 01, fourteen GTIN digits with a MATCHING
    /// check digit, then AI 21.
    ///
    /// Off by default: it turns a general-purpose scanner into a GS1-specific
    /// one, and codes with a different structure will be rejected.
    bool requireGs1 = false;

    /// Symbol rotation, 0..3 — quarter turns clockwise; -1 — auto-detect.
    int forcedRotation = -1;

    // ── How much to spend ──────────────────────────────────────────────────

    /// The largest frame the scanner agrees to process: 16384 per side and
    /// 100 MP by default. A larger frame gets Status::ImageTooLarge at once,
    /// without processing. Pass the same limit to loadImage(): there it is
    /// checked against the file header, before memory for pixels is allocated.
    ///
    /// This is a protection, not a quality setting: a camera frame fits with a
    /// large margin, while without a limit a file of a few dozen bytes with a
    /// 1 000 000 x 1 000 000 header eats all the process memory.
    ImageLimits imageLimits;

    /// Time budget per frame, ms; 0 — unlimited.
    ///
    /// The deadline is SOFT. It is checked between stages, not inside them:
    /// rectification or a single decode cannot be interrupted midway, so the
    /// frame time may exceed the budget by the duration of the running stage.
    /// The lower bound is the first detector: without candidates there is
    /// nothing to spend the budget on, and on a frame with heavy texture the
    /// detector alone can take well over a hundred milliseconds.
    ///
    /// The budget also selects what to trade for speed in the deep search:
    /// when set, the scanner minimizes frame latency; when not set, CPU time.
    /// For the detector and the fast path the same is decided by rowParallel.
    /// The result is the same in all cases.
    double budgetMs = 0.0;

    /// Safety timeout: time limit per frame, ms; 0 — no limit.
    ///
    /// A protection against hostile frames, not a latency setting: a frame
    /// filled edge to edge with hatching or texture can otherwise keep the CPU
    /// busy for minutes. The deadline works like budgetMs — if both are set,
    /// the smaller applies, on expiry the result is marked timedOut — and is
    /// just as soft.
    ///
    /// The only difference from budgetMs: the safety timeout does NOT select
    /// what to trade for speed (see above). That is why it is on by default
    /// without changing either the result or the cost of a normal frame: a
    /// camera image is processed in tens of milliseconds, a failed one with
    /// size search in seconds. Turn it off (0) only for processing trusted
    /// images for which ten seconds is not enough.
    double timeoutMs = 10000.0;

    /// Split image-kernel rows — rectification, detector smoothing and
    /// gradient, module thresholds — across threads while a frame is processed
    /// on one thread: in the detector and the fast path. The result does not
    /// depend on it.
    ///
    /// This trades CPU time for latency. For a camera delivering a frame per
    /// second, frame latency drops noticeably (most at the high percentiles).
    /// CPU time per frame roughly doubles, and when processing frames back to
    /// back on a power-limited machine the wall-clock gain disappears: busy
    /// cores lower the main core's frequency. Turn it off if several scanners
    /// run on the machine or throughput matters more than frame latency.
    bool rowParallel = true;

    /// How many symbol sizes to try when the size estimate cannot be trusted;
    /// 1 — do not try others.
    ///
    /// The run-length size estimate can fail, and its failure means the size
    /// was taken from the correlation maximum — a quantity that carries no
    /// signal on a blurred alternating pattern. Then, as a last pass and only on
    /// a frame where NOTHING was decoded, sizes from a ranked list are tried.
    /// For the cost see single().
    int maxSizeTries = 1;

    /// Rectification canvas scales for the LAST pass, pixels per module. Empty
    /// — do not try.
    ///
    /// At two or three pixels per module in the source, the canvas scale is a
    /// lottery: an image may decode at most scale values yet fail at exactly
    /// the default one. This search recovers a small share of otherwise failed
    /// frames.
    ///
    /// Expensive: a new scale requires a new rectification, which is the most
    /// expensive part. Best enabled together with requireGs1 — the search
    /// increases the number of attempts and with them the chance of a false
    /// convergence.
    std::vector<int> warpScales;

    /// Parse the list of scales from a string such as "7,8,10".
    ///
    /// Provided by the library because settings arrive as strings not only
    /// from the command line but also from configuration files or form fields,
    /// and there is no need for every caller to parse them again. Unparsable
    /// and non-positive values are silently skipped — this is a setting, not
    /// user input, and there is nothing to fail on.
    void setWarpScales(const std::string& list);

    /// How many candidates to take from the L-pattern detector.
    ///
    /// Six is more than enough for one symbol. Candidates are suppressed by
    /// overlap, so they are spread over the frame, but on a frame with four or
    /// five symbols six boxes may not suffice. Each extra candidate means one
    /// more full decode pass.
    int maxCandidates = 6;

    /// Memory limit, MB per frame, for reusing intermediate work; 0 — store
    /// nothing. Does not affect the RESULT: whatever does not fit is recomputed.
    double readerCacheMb = 64.0;

    // ── Frame stream ───────────────────────────────────────────────────────

    /// How many consecutive frames must agree on one symbol size for it to be
    /// tried FIRST from then on; 0 — no hint.
    ///
    /// The hint does not replace auto-detection; it is added before it as the
    /// first attempt, so reading can only improve. Only a frame on which the
    /// hint does not work pays: it gets one extra cheap pass. Five — so that
    /// one random run does not lock in a wrong size.
    int mcHintRun = 5;

    // ── Fine tuning ────────────────────────────────────────────────────────

    /// Maximum error rate in the fixed (finder and timing) modules for a result
    /// to be considered at all. The content of the L-shaped finder and the
    /// timing pattern is known in advance, so its error rate directly checks
    /// that the grid landed on a real symbol and not on text or a conveyor
    /// rail. Without this check false positives were observed.
    double maxPatternError = 0.25;

    /// Repeat the L-pattern search at a finer scale; 0 — do not repeat.
    ///
    /// Not a new default but a SECOND hypothesis: it runs only on a frame that
    /// was decoded neither by the first pass nor by the texture detector. As a
    /// global setting the finer scale loses some reads and is slower; as a
    /// fallback it costs almost nothing, because the vast majority of frames
    /// never reach it, and it recovers some hard images.
    double lpatRetryScale = 0.8;

    /// Take module brightness directly from the source, bypassing the
    /// rectification canvas.
    bool directSampling = false;

    /// Candidate sources. Turning one off is meant for comparing detectors on
    /// the same image set.
    bool useLPatternFinder = true;
    bool useTextureFinder  = true;

    /// Collect a breakdown of the successful or failed read
    /// (ScanResult::details).
    ///
    /// Off by default: the breakdown keeps the winning hypothesis's reader and
    /// the frame crop in memory, about two megabytes per result. In a frame
    /// stream that is extra memory pressure for data nobody asked for.
    bool collectDetails = false;
};

} // namespace dmr
