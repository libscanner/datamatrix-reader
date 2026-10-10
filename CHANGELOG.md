# Changelog

## 1.0.7 — 2026-10-10

- Reads frames that other readers read and earlier versions did not:
  - symbols on a cylinder (bottle, tube): the module grid now follows the alternating edges of the symbol, so a label bent by 120–180° is read (on synthetic sweeps 40 of 72 frames before, 71 now);
  - pale symbols on a white label (print contrast 0.12–0.08): 0 of 72 before, 72 now;
  - mirrored symbols (printed on the back of a transparent film): 0 of 24 before, 24 now;
  - codes under glossy film with a glare spot, and pale codes where trimming cut off a row of modules: 4 more frames of 327 on our pharmacy-package set;
  - symbols on a plane tilted by 70–75°, and symbols at 2.4–2.9 pixels per module where the default resampling scale missed the modules;
  - symbols filled to capacity (no pad codewords) now accept erasures when at least 8 syndromes remain unused.
- On synthetic sweeps of 648 frames (tilt, bend, contrast, noise, damage, gain, mirror) 451 are read instead of 306; no false reads there or on 3500 frames without a valid code.
- Checked frame by frame against 1.0.6 on our production-line sets: camera conveyor 8923 of 8959 (+1), pharmacy line 4012 of 4019 (+1); no frame lost, no string changed. Frames that are read are not slower; frames that are not read at all spend about 0.1 s more on the extra attempts (cut by the time budget, if set).

## 1.0.6 — 2026-10-09

- Faster on frames that the fast path does not read (about one frame in eight on a pharmacy line): grid refinement on the first detector's candidates now runs before the second detector, the search goes depth-first per candidate, the texture detector uses two variance windows instead of four, and the deep-search ladder is ordered by measured yield. On a 500-frame pharmacy-line sample the 90th percentile of per-frame time went from 51 to 15 ms; frames read within a 60 ms budget went from 472 to 495 of 499 readable, on a 299-frame conveyor sample from 285 to 295 of 296. Well-readable frames are unchanged.
- With a time budget (`Settings::budgetMs`), a stage whose typical cost exceeds the remaining time is no longer started; the slow tail passes (soft gates, inverted candidates) run only in the pass without a symbol-size hint.
- `Settings::mcHintRun`: documented that on a line where the symbol size changes from batch to batch the hint should be disabled (0).
- Checked frame by frame against 1.0.5 on our production-line sets: camera conveyor 8922 of 8959 frames, pharmacy line 4011 of 4019 — identical strings; synthetic sets and multi-code mode unchanged.

## 1.0.5 — 2026-10-09

- New: inverted symbols — light modules on a dark background (laser marking on dark plastic, print on black packaging) — are read. On synthetic frames 24 of 24 instead of 0. When nothing has been read on a frame, the first candidate crops are retried inverted on the fast path; frames that are read are not affected. An inverted frame currently costs about 1 s, because it goes through the regular search first.
- Checked frame by frame against 1.0.4 on our production-line sets: camera conveyor 8922 of 8959 frames, pharmacy line 4011 of 4019 — identical; no false reads on 4000 frames without a valid code; per-frame time on frames that are read is unchanged (a frame that is not read at all pays up to six extra fast attempts).

## 1.0.4 — 2026-10-09

- Fixed: a white label on a dark box with the label edge one or two modules from the symbol was not read (the symbol merged with the dark background). At a 2-module edge the read rate went from 5 of 12 to 12 of 12 on synthetic frames, at 1 module from 0 to 8 of 12.
- Fixed: a symbol touching the edge of the frame was never read (its finder pattern was smeared outside the frame); it is read now.
- Fixed: a false read — a 40×40 symbol with text right next to two of its sides could decode to garbage when all error-correction capacity was spent on erasures. At least four syndromes are now always left to verify the result; multi-block symbols without pad codewords get the same margin as one-block ones. No false reads on 4000 frames without a valid code (text, QR, Code 128, fake and half symbols, overdamaged and mirrored symbols).
- Dot-peen (DPM) marking reads markedly better: 8–12 of 12 synthetic frames instead of 0–8.
- Checked frame by frame against 1.0.3 on our production-line sets: camera conveyor 8922 of 8959 frames (+2), pharmacy line 4011 of 4019 (unchanged); no frame lost, no string changed; per-frame time unchanged.

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
