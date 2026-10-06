#pragma once

// ── Diagnostics ──────────────────────────────────────────────────────────
//
// Two different things that should not be confused.
//
// EXPLANATION (explain) — what diagnostics are usually wanted for: everything
// that lets a read be checked from the outside is collected from a finished
// result. Returns a string; what to do with it is up to the caller.
//
// LOG STREAM (dbg) — debug messages emitted during processing from many places
// inside the library. Silenced by default.
//
// Real I/O errors go to neither: they are returned as codes (LoadStatus in
// dmr/Image.h), because they mean the work was not done, not that someone is
// curious.

#include <functional>
#include <ostream>
#include <string>
#include <string_view>

namespace dmr {

struct Details;   // dmr/Scanner.h

/// What to include in the explanation besides the result itself.
struct ExplainOptions {
    /// Symbol size given by the caller when scanning; 0 — it was auto-detected.
    /// Needed so the explanation repeats exactly the read it explains.
    int moduleCount = 0;

    /// Rotation given when scanning; -1 — it was auto-detected.
    int forcedRotation = -1;

    /// Save debug PNGs to the working directory: region crops with module
    /// centers, the grid over the canvas.
    ///
    /// This is the only place the library writes files, and only on explicit
    /// request.
    bool images = false;

    /// Image name — used for captions and debug file names.
    std::string imagePath;
};

/// Explain a scan result: region nodes, the error rate in the fixed patterns,
/// centers of the alternating (timing) pattern, the cleaned data matrix,
/// codewords and Reed-Solomon syndromes.
///
/// When a symbol was found but not decoded there is nothing to build the
/// breakdown from — then the crop is rectified and decoded again so the
/// failure can be explained. This costs about as much as scanning the frame.
///
/// It does NOT issue a verdict: the frame status was set by the scanner and
/// must not be replaced by the explanation — the explanation decodes without
/// acceptance checks and would report "decoded" on a rejected frame.
///
/// @param details  ScanResult::details; collected when Settings::collectDetails is set
std::string explain(const Details& details, const ExplainOptions& opt = {});

// ── Debug log stream ──────────────────────────────────────────────────────
//
// Messages emitted during processing from many places inside the library. They
// go wherever the caller says and NOWHERE by default: the library has no right
// to write to someone else's stderr — it does not know what the process is
// doing or who reads that stream.

/// Where messages go. Delivered one line at a time, with a trailing newline.
///
/// Called from the thread where the message originated, but never from more
/// than one thread at a time — the sink needs no mutex of its own. Lines from
/// parallel passes are not interleaved: each thread accumulates its own.
using LogSink = std::function<void(std::string_view line)>;

/// Install a sink. An empty one turns logging off.
void setLogSink(LogSink sink);

/// Turn the debug log on or off. Off by default.
void setVerbose(bool on);

/// Whether logging is active: turned on AND a sink is installed.
///
/// The second condition matters: without a sink the library would build lines
/// only to throw them away, and a failed frame produces hundreds of them.
bool isVerbose();

/// The debug log stream: to the sink if there is one, otherwise discarded.
std::ostream& dbg();

} // namespace dmr

// Short name for places where debug output is obvious from context.
using dmr::dbg;
