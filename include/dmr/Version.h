#pragma once

// ── Library version ───────────────────────────────────────────────────────

#include <string>

#define DMR_VERSION_MAJOR 1
#define DMR_VERSION_MINOR 0
#define DMR_VERSION_PATCH 6
#define DMR_VERSION_STRING "1.0.6"

/// The version as a number, for preprocessor comparisons:
///     #if DMR_VERSION >= DMR_VERSION_AT(1, 1, 0)
#define DMR_VERSION_AT(ma, mi, pa) ((ma) * 10000 + (mi) * 100 + (pa))
#define DMR_VERSION \
    DMR_VERSION_AT(DMR_VERSION_MAJOR, DMR_VERSION_MINOR, DMR_VERSION_PATCH)

namespace dmr {

/// The version of the BUILT library, not of the header.
///
/// That is the point: the macros above describe the header you compiled
/// against, this function describes the library you linked with. The two can
/// easily diverge — an installed library tends to outlive the memory of who
/// built it.
std::string version();

} // namespace dmr
