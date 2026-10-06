// SPDX-License-Identifier: MIT-0
/*
 * dmr through the flat C ABI (dmr_c.dll / libdmr_c.so).
 * Works with any C compiler, MSVC included, and is the model for bindings
 * from other languages (C#, Delphi, Go...).
 *
 *   test_dmr_c <image>
 */

#include <dmr/dmr_c.h>

#include <stdio.h>

int main(int argc, char** argv) {
    char version[64];
    dmr_image_t* image = NULL;
    dmr_scanner_t* scanner = NULL;
    dmr_scan_result_t* result = NULL;
    dmr_image_view_t view;
    int rc = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <image>\n", argv[0]);
        return 2;
    }

    dmr_version(version, sizeof version);
    printf("dmr %s\n", version);

    /* The path is in the process's system code page, like argv. */
    if (dmr_load_image(argv[1], DMR_LOAD_GRAY, &image) != DMR_LOAD_OK) {
        fprintf(stderr, "could not read %s\n", argv[1]);
        return 1;
    }

    scanner = dmr_scanner_create(NULL);   /* NULL: default settings (frame stream) */
    if (!scanner)
        goto done;

    view = dmr_image_view(image);
    result = dmr_scanner_scan(scanner, &view, 0);   /* 0: detect the symbol size */
    if (!result)                                     /* NULL only on an internal error */
        goto done;

    switch (dmr_scan_result_status(result)) {
        case DMR_STATUS_OK: {
            int i, n = dmr_scan_result_code_count(result);
            for (i = 0; i < n; ++i) {
                /* Text may contain byte 0x1D (GS1 separator): always use the length. */
                const char* text = dmr_scan_result_code_text(result, i);
                size_t len = dmr_scan_result_code_text_len(result, i);
                printf("%.*s\n", (int)len, text);
            }
            rc = 0;
            break;
        }
        case DMR_STATUS_NOT_FOUND:
            printf("no symbol in the frame\n");
            break;
        case DMR_STATUS_UNLICENSED: {
            char* status = dmr_license_status_json();   /* JSON, same as "dmr --license status --json" */
            fprintf(stderr, "no trial period and no license: %s\n", status ? status : "");
            if (status)
                dmr_free_string(status);
            break;
        }
        default:
            printf("symbol found but could not be read\n");
            break;
    }

done:
    if (result)
        dmr_scan_result_free(result);
    if (scanner)
        dmr_scanner_destroy(scanner);
    dmr_image_free(image);
    return rc;
}
