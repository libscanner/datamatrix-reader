# Command-line tool `dmr`

The `dmr` tool ships in every archive (`bin/dmr`, `bin\dmr.exe`), in the CLI-only archives (`…-cli.zip` /
`…-cli.tar.gz`) and in the Python wheel (after `pip install` the `dmr` command is on the environment's `PATH`;
`python -m dmr` works too). Full option list: `dmr --help`.

The tool prints its messages in Russian: “запуск” is startup time, “обработка” is processing time, “код” is
the decoded code.

## Usage

```text
dmr <image> [module_count] [--verbose] [--rot 0..3] [--budget-ms N] [--codes N]
dmr --batch  <list>  [module_count] [--verbose]
dmr --folder <directory> [module_count] [--verbose]
dmr --serve  [module_count] [--verbose]
dmr --license status | activate <key> | refresh | deactivate | fingerprint [--json]
```

## Single image

```console
$ dmr label.jpg
запуск: 4.6 мс
обработка: 24.9 мс
код: 0104627191145677215n5Gr,pHTGIU&<GS>93CuXU
```

Useful options:

```sh
dmr label.jpg --codes 0          # every code in the frame (0 = as many as found)
dmr label.jpg --gs1              # accept only GS1-structured codes (AI 01 + valid GTIN, then AI 21)
dmr label.jpg --budget-ms 120    # time budget per image
```

## Batch and folder

One process for a set of images known in advance; one line per result on stdout:
`path<TAB>exit code<TAB>milliseconds<TAB>text`.

```sh
dmr --folder ./frames
dmr --batch list.txt             # one path per line
```

## Streaming (`--serve`)

Paths arrive one per line on stdin and are processed immediately; the scanner stays alive between requests.
Each answer ends with the line `#END#` and is flushed at once.

```console
$ mkfifo queue
$ dmr --serve < queue &
$ echo /path/to/image.jpg > queue
/path/to/image.jpg	0	91.2	0104627191145677215n5Gr,pHTGIU&<GS>93CuXU
#END#
```

`--root <directory>` restricts `--serve` to images inside that directory; `--corners` adds the symbol corners
as a fifth column.

## License

```sh
dmr --license status             # trial period / license state
dmr --license activate DMR-XXXXX-XXXXX-XXXXX-XXXXX
dmr --license refresh            # check with the server now
dmr --license request request.json   # offline activation: upload in your account, get license.lic
dmr --license import license.lic     # accept a license file (also renewals)
dmr --license deactivate         # release this machine before moving the license
dmr --license fingerprint        # machine fingerprint, for support requests
dmr --license status --json      # one JSON line, for scripts
```

```console
$ dmr --license status --json
{"state":"trial","can_scan":true,"valid_until":1792831857,"refresh_after":0,"offline_until":0,"days_left":28,"license_id":"","activation_id":"","offline_mode":"","clock_rollback":false,"message":"Пробный период: осталось 28 дн."}
```

`state` is a stable machine-readable code: `trial`, `active`, `refresh_due`, `trial_expired`, `expired`,
`network_required`, `wrong_machine`, `invalid`.

Documentation: <https://libscanner.com/en/docs/datamatrix-gs1/#cli>
