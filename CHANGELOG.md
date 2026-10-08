# Changelog

## 1.0.3 — 2026-10-08

- Fixed: symbols on a plane tilted by 40–60° (strong perspective) are found; at 50° the read rate went from 16 % to 100 % on synthetic frames.
- Fixed: a uniform grid inside a one-block symbol could be accepted as a code with an empty string; it is now rejected.
- Error correction now also works for symbols without pad codewords (errors up to (nsym − 3) / 2).
- Checked frame by frame against 1.0.2 on our production-line sets: camera conveyor 8920 of 8959 frames (+1), pharmacy line 4011 of 4019 (unchanged); no frame lost, no string changed; per-frame time unchanged.

## 1.0.2 — 2026-10-08

- Fixed: a symbol rotated by exactly 45° (also 135°, 225°, 315°) was not found — the corner ordering collapsed the quadrilateral into a triangle. Any rotation angle is read now.
- Fixed: a symbol on a light label lying entirely inside the candidate crop (a white label on a grey box) was not found; the background ring around the label is now discarded.
- Read rate and per-frame time on the production-line frame sets are unchanged.

## 1.0.1 — 2026-10-07

- Fixed: frames in `Rgb8` and `Rgba8` pixel formats were rejected — `scan()` returned `NotFound` immediately, so a camera delivering RGB frames read nothing. They are now read exactly like the same frame in `Bgr8` / `Bgra8` (the caller's buffer is not modified).
- Python package is published on PyPI as `libscanner-dmr` (imported as `dmr`); the wheels are named `libscanner_dmr-*.whl`.

## 1.0.0 — 2026-10-04

First public release.

- C++20 static library `libdmr.a` (headers `include/dmr/`), flat C ABI library `dmr_c` (`dmr/dmr_c.h`).
- Python package `dmr` (wheels for Windows x64 and Linux x86-64, ARM64, ARMv7; Python 3.9+), with the `dmr` command.
- Command-line tool `dmr`: single image, `--batch`, `--folder`, `--serve`, `--license`.
- Presets `Settings::stream()`, `single()`, `multi(n)`; optional GS1 structure check (`requireGs1`).
- Platforms: Windows x64, Linux x86-64, Linux ARM64, Linux ARMv7 (Raspberry Pi).
- 30-day trial from the first run on a machine; license activation online or offline.
