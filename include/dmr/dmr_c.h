#ifndef DMR_C_H
#define DMR_C_H

/* ═══════════════════════════════════════════════════════════════════════════
 *  dmr/dmr_c.h — flat C ABI of the library
 *
 *  The boundary for callers other than C++: plain C99, none of the included
 *  files pulls in anything from the include/dmr C++ headers.
 *
 *  BOUNDARY RULE: only POD types cross it (integers, double, pointers to
 *  buffers of known size, enumerations as int) and opaque handles. A C++
 *  exception never crosses this boundary — it is caught inside the
 *  implementation and turned into NULL or an error code.
 *
 *      dmr_settings_t settings = { sizeof(dmr_settings_t) };   // struct_size
 *      dmr_settings_stream(&settings);       // preset; fields may be changed
 *      dmr_scanner_t* scanner = dmr_scanner_create(&settings);  // NULL — stream()
 *      dmr_image_view_t view = { ptr, w, h, stride, DMR_PIXEL_BGR8 };
 *      dmr_scan_result_t* r = dmr_scanner_scan(scanner, &view, 0);
 *      if (dmr_scan_result_status(r) == DMR_STATUS_OK) {
 *          size_t len = dmr_scan_result_code_text_len(r, 0);
 *          const char* text = dmr_scan_result_code_text(r, 0);
 *          ...
 *      }
 *      dmr_scan_result_free(r);
 *      dmr_scanner_destroy(scanner);
 *
 *  Memory ownership follows one rule without exceptions: every function that
 *  RETURNS a handle or a string via return hands it over to the caller, and
 *  has a matching dmr_..._free()/dmr_free_string(). Pointers returned by
 *  accessor functions (dmr_scan_result_code_text and the like) live no longer
 *  than the object they were taken from and are not freed separately.
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#  if defined(DMR_C_BUILDING)
#    define DMR_C_API __declspec(dllexport)
#  else
#    define DMR_C_API __declspec(dllimport)
#  endif
#else
#  if defined(DMR_C_BUILDING)
#    define DMR_C_API __attribute__((visibility("default")))
#  else
#    define DMR_C_API
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ── Boundary version ───────────────────────────────────────────────────── */
/*
 * Separate from dmr_version() (the library version): the ABI changes less
 * often and for different reasons. MAJOR — a compatibility break (a removed
 * function, a reordered struct field); MINOR — additions at the end, with no
 * change to what exists. Rule for the future: new fields in existing structs
 * are only appended.
 *
 * ABI 2.0: dmr_settings_t starts with struct_size (see below); timeout_ms,
 * DMR_LOAD_TIMED_OUT and DMR_LOAD_INVALID_ARGUMENT were added. A program built
 * with a 1.x header must be rebuilt: the new library rejects 1.x settings
 * (dmr_scanner_create returns NULL) rather than misreading them.
 */
#define DMR_C_ABI_VERSION_MAJOR 2
#define DMR_C_ABI_VERSION_MINOR 0

DMR_C_API size_t dmr_c_abi_version(char* buf, size_t buf_size);

/* Version of the BUILT library — like dmr::version() in the C++ API. */
DMR_C_API size_t dmr_version(char* buf, size_t buf_size);

/* ── Frame ──────────────────────────────────────────────────────────────── */

typedef enum {
    DMR_PIXEL_GRAY8 = 0,
    DMR_PIXEL_BGR8  = 1,
    DMR_PIXEL_RGB8  = 2,
    DMR_PIXEL_BGRA8 = 3,
    DMR_PIXEL_RGBA8 = 4,
} dmr_pixel_format_t;

/* A view over a CALLER-OWNED buffer: the library does not copy it, free it or
 * keep it. The buffer must stay alive for the whole dmr_scanner_scan() call.
 * Fields and their order are the same as dmr::ImageView in the C++ API. */
typedef struct {
    const uint8_t*      data;
    int                 width;
    int                 height;
    int                 stride;    /* 0 — tightly packed: width * format channels */
    dmr_pixel_format_t  format;
} dmr_image_view_t;

/* A frame loaded from disk — the library owns the buffer. */
typedef struct dmr_image dmr_image_t;   /* opaque */

typedef enum { DMR_LOAD_COLOR = 0, DMR_LOAD_GRAY = 1 } dmr_load_as_t;
typedef enum {
    DMR_LOAD_OK         = 0,
    DMR_LOAD_NOT_FOUND  = 1,
    DMR_LOAD_UNREADABLE = 2,
    DMR_LOAD_TOO_LARGE  = 3,   /* size from the header exceeds the limit (ABI 1.1) */
    DMR_LOAD_TIMED_OUT  = 4,   /* decoding did not finish within the deadline from
                                * settings (budget_ms/timeout_ms) — ABI 2.0 */
    DMR_LOAD_INVALID_ARGUMENT = 5, /* settings->struct_size is smaller than
                                    * DMR_SETTINGS_MIN_SIZE — ABI 2.0 */
} dmr_load_status_t;

/*
 * path is in the process's SYSTEM code page, like argv. On Windows this is not
 * UTF-8 unless the process code page has been explicitly set to UTF-8 (the
 * same rule as for dmr::loadImage in the C++ API and for paths in
 * "dmr --serve").
 *
 * *out is NULL on any failure. On DMR_LOAD_OK *out is a new dmr_image_t,
 * which must be freed with dmr_image_free().
 */
DMR_C_API dmr_load_status_t dmr_load_image(const char* path, dmr_load_as_t as,
                                           dmr_image_t** out);

/* Settings are defined below; forward-declared here for dmr_load_image_ex. */
typedef struct dmr_settings dmr_settings_t;

/* Same, but with the frame size limit from settings (max_image_side,
 * max_image_pixels); settings == NULL — defaults (16384 per side, 100 MP).
 * dmr_load_image() is the same as this call with NULL. A frame over the limit
 * is not decoded: DMR_LOAD_TOO_LARGE, checked against the file header before
 * memory for pixels is allocated. ABI 1.1.
 *
 * ABI 2.0: decoding is subject to the frame deadline from settings — the
 * smaller of budget_ms and timeout_ms (NULL — the default timeout_ms, 10 s);
 * if it is exceeded, DMR_LOAD_TIMED_OUT. An invalid settings->struct_size
 * gives DMR_LOAD_INVALID_ARGUMENT. */
DMR_C_API dmr_load_status_t dmr_load_image_ex(const char* path, dmr_load_as_t as,
                                              const dmr_settings_t* settings,
                                              dmr_image_t** out);

DMR_C_API dmr_image_view_t dmr_image_view(const dmr_image_t* image);
DMR_C_API void             dmr_image_free(dmr_image_t* image);

/* ── Settings ───────────────────────────────────────────────────────────── */

#define DMR_MAX_WARP_SCALES 16

/* Corresponds to dmr::Settings in the C++ API — same fields and defaults.
 *
 * STRUCT SIZE (ABI 2.0). The first field is struct_size: sizeof(dmr_settings_t)
 * as seen by the CALLER, from the header it was compiled with. The caller sets
 * it, BEFORE applying a preset:
 *
 *     dmr_settings_t s = { sizeof(dmr_settings_t) };
 *     dmr_settings_single(&s);
 *
 * It tells the library how many fields the caller has: a preset writes no
 * further than struct_size, and dmr_scanner_create/dmr_load_image_ex take only
 * those fields from the struct, the rest are the stream() preset defaults.
 * This way a program built with an older header works with a newer library
 * that has more fields at the end. A struct_size smaller than
 * DMR_SETTINGS_MIN_SIZE (the ABI 2.0 struct size) is rejected: the preset
 * writes nothing and returns 0, dmr_scanner_create returns NULL. */
struct dmr_settings {
    uint32_t struct_size;        /* sizeof(dmr_settings_t) as seen by the caller */
    int    max_codes;
    int    require_gs1;          /* 0/1 */
    int    forced_rotation;      /* -1 — auto-detect */
    double budget_ms;
    int    row_parallel;         /* 0/1 */
    int    max_size_tries;
    int    warp_scales[DMR_MAX_WARP_SCALES];
    int    warp_scales_count;    /* 0 — do not try */
    int    max_candidates;
    double reader_cache_mb;
    int    mc_hint_run;
    double max_pattern_error;
    double lpat_retry_scale;
    int    direct_sampling;      /* 0/1 */
    int    use_lpattern_finder;  /* 0/1 */
    int    use_texture_finder;   /* 0/1 */
    int    collect_details;      /* 0/1 — this ABI version does not provide
                                   * explain(); kept so that the struct
                                   * matches dmr::Settings */
    /* ── ABI 1.1 ──
     * Frame size limit (dmr::Settings::imageLimits): a larger frame gets
     * DMR_STATUS_IMAGE_TOO_LARGE without processing. 0 — the library ceiling
     * (65535 per side, 2^29 pixels). Preset defaults are 16384 and
     * 100 000 000.
     *
     * The fields were appended at the end. Fill the struct with a preset
     * (dmr_settings_stream and the like) of the same library version you
     * compile against: the preset writes the whole struct, and a caller built
     * with an ABI 1.0 header would pass it a buffer too short for these
     * fields. */
    int     max_image_side;
    int64_t max_image_pixels;
    /* ── ABI 2.0 ──
     * Safety timeout (dmr::Settings::timeoutMs): time limit per frame, ms;
     * 0 — no limit. Works like budget_ms (if both are set, the smaller
     * applies; on expiry dmr_scan_result_timed_out() == 1), but does not
     * select the scanner's mode of operation. Preset default is 10000. Also
     * applies to decoding in dmr_load_image_ex. */
    double  timeout_ms;
};

/* The smallest struct_size the library accepts: the ABI 2.0 struct size.
 * Fields added later lie beyond this boundary. */
#define DMR_SETTINGS_MIN_SIZE \
    (offsetof(struct dmr_settings, timeout_ms) + sizeof(double))

/* Fill *out with a preset — no further than out->struct_size bytes, which the
 * caller set BEFORE the call (see above). Return 1; 0 — out == NULL or
 * struct_size < DMR_SETTINGS_MIN_SIZE, in which case *out is untouched. */
DMR_C_API int dmr_settings_stream(dmr_settings_t* out);
DMR_C_API int dmr_settings_single(dmr_settings_t* out);
DMR_C_API int dmr_settings_multi(dmr_settings_t* out, int max_codes);

/* ── Scanner ────────────────────────────────────────────────────────────── */

typedef struct dmr_scanner dmr_scanner_t;   /* opaque */

typedef enum {
    DMR_STATUS_OK             = 0,
    DMR_STATUS_NOT_FOUND      = 1,
    DMR_STATUS_NOT_DECODED    = 2,
    DMR_STATUS_UNLICENSED     = 3,
    /* ABI 1.1, same as dmr::Status: */
    DMR_STATUS_IMAGE_TOO_LARGE = 4,  /* frame exceeds max_image_side/max_image_pixels */
    DMR_STATUS_INVALID         = 5,  /* inconsistent frame description: stride
                                      * smaller than width * channels, unknown
                                      * format, size overflow */
    DMR_STATUS_FAILED          = 6,  /* processing aborted by an internal error
                                      * (e.g. out of memory) */
    /* The values below do not exist in dmr::Status (C++ API) — they concern
     * the boundary itself, not the algorithm, and appear only on an internal
     * error of the C layer (e.g. a C++ exception caught at the boundary). */
    DMR_STATUS_INTERNAL_ERROR = -1,
} dmr_status_t;

/* settings == NULL — dmr_settings_stream() (the default, like Scanner() in
 * the C++ API). Returns NULL on an internal error and when
 * settings->struct_size < DMR_SETTINGS_MIN_SIZE (which typically includes the
 * settings of a program built with an ABI 1.x header: max_codes sits where
 * struct_size is expected). */
DMR_C_API dmr_scanner_t* dmr_scanner_create(const dmr_settings_t* settings);
DMR_C_API void           dmr_scanner_destroy(dmr_scanner_t* scanner);

DMR_C_API int  dmr_scanner_module_count_hint(const dmr_scanner_t* scanner);
DMR_C_API void dmr_scanner_reset_stream(dmr_scanner_t* scanner);

/* ── Result ─────────────────────────────────────────────────────────────── */

typedef struct dmr_scan_result dmr_scan_result_t;   /* opaque */

/* module_count — symbol side in modules, if known in advance; 0 —
 * auto-detect. Ownership of the result passes to the caller.
 * NULL — internal error (a caught C++ exception); this cannot be told apart
 * by DMR_STATUS_NOT_FOUND, because for "code not found" the result is not
 * NULL — check the pointer itself. */
DMR_C_API dmr_scan_result_t* dmr_scanner_scan(dmr_scanner_t* scanner,
                                              const dmr_image_view_t* frame,
                                              int module_count);
DMR_C_API void dmr_scan_result_free(dmr_scan_result_t* result);

DMR_C_API dmr_status_t dmr_scan_result_status(const dmr_scan_result_t* result);
DMR_C_API int           dmr_scan_result_timed_out(const dmr_scan_result_t* result);
DMR_C_API double        dmr_scan_result_elapsed_ms(const dmr_scan_result_t* result);
DMR_C_API int           dmr_scan_result_code_count(const dmr_scan_result_t* result);

/* The pointer lives no longer than result. The decoded string's bytes are
 * returned as is: the GS1 separator stays a 0x1D byte, not replaced with a
 * printable form (unlike the output of the dmr command-line tool) — the C ABI
 * has no reason to hide it. Because of this the text may contain a 0 byte, so
 * the length MUST be taken from dmr_scan_result_code_text_len, not strlen(). */
DMR_C_API const char* dmr_scan_result_code_text(const dmr_scan_result_t* result, int index);
DMR_C_API size_t      dmr_scan_result_code_text_len(const dmr_scan_result_t* result, int index);
DMR_C_API void        dmr_scan_result_code_box(const dmr_scan_result_t* result, int index,
                                               int* x, int* y, int* width, int* height);
/* corners_xy — eight floats, x0,y0,x1,y1,x2,y2,x3,y3; for the corner order see
 * dmr/Geometry.h (Quad) in the C++ API. */
DMR_C_API void dmr_scan_result_code_corners(const dmr_scan_result_t* result, int index,
                                            float corners_xy[8]);
DMR_C_API int  dmr_scan_result_code_module_count(const dmr_scan_result_t* result, int index);

/* ── Diagnostics ────────────────────────────────────────────────────────── */

/* line is not guaranteed to be a null-terminated C string — read exactly len
 * bytes. Called from the thread where the message
 * originated, but never from more than one thread at a time (like LogSink in
 * the C++ API). */
typedef void (*dmr_log_sink_t)(const char* line, size_t len, void* user);

DMR_C_API void dmr_set_log_sink(dmr_log_sink_t sink, void* user);
DMR_C_API void dmr_set_verbose(int on);
DMR_C_API int  dmr_is_verbose(void);

/* ── License ────────────────────────────────────────────────────────────── */
/*
 * Return the same JSON format as "dmr --license ... --json" rather than
 * separately designed structs: one contract instead of two. Free every
 * non-NULL result with dmr_free_string() after use. NULL — internal error.
 */

DMR_C_API void dmr_license_configure(const char* server_url, const char* app_version,
                                     int timeout_ms, int auto_check);

DMR_C_API char* dmr_license_status_json(void);
DMR_C_API char* dmr_license_activate_json(const char* key);
DMR_C_API char* dmr_license_refresh_json(void);
DMR_C_API char* dmr_license_deactivate_json(const char* reason);              /* reason may be NULL */
DMR_C_API char* dmr_license_deactivate_to_file_json(const char* path, const char* reason);
DMR_C_API char* dmr_license_write_activation_request_json(const char* path);
DMR_C_API char* dmr_license_import_license_file_json(const char* path);
DMR_C_API char* dmr_license_fingerprint_json(void);

DMR_C_API void dmr_free_string(char* s);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DMR_C_H */
