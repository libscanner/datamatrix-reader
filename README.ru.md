[English](https://github.com/libscanner/datamatrix-reader/blob/main/README.md) | **Русский**

# Библиотека чтения DataMatrix (dmr)

Распознавание кодов DataMatrix структуры GS1 с кадров промышленной камеры — для конвейеров с маркировкой «Честного знака».

`dmr` — коммерческая библиотека с закрытым исходным кодом: C++20 API, плоский C ABI, пакет для Python и утилита командной строки. Читает символы **DataMatrix (ECC 200)** при низком разрешении съёмки — около двух пикселей на модуль, символ порядка сотни пикселей на кадре — и проверяет каждую гипотезу геометрии исправлением ошибок Рида — Соломона. Необязательный режим GS1 принимает только коды структуры GS1 (AI 01 с верной контрольной цифрой GTIN, затем AI 21).

В этом репозитории — публичные заголовки, примеры, ссылки на документацию и тексты лицензий. **Исходного кода библиотеки здесь нет.** Собранные библиотеки приложены к [выпускам на GitHub](https://github.com/libscanner/datamatrix-reader/releases/latest).

## Ключевые цифры

| | |
|---|---|
| **100%** | кадров прочитаны верно (8773 читаемых кадров на конвейерной выборке) |
| **от 5,3 мс** | минимальное время чтения кадра; медиана по корпусу — 28 мс |

Попробовать без установки: <https://libscanner.com/scan>

## Платформы

| Платформа | Статическая библиотека C++ + C ABI + утилита | Колесо Python |
|---|---|---|
| Windows x64 | `dmr-1.0.2-windows-x64.zip` | `libscanner_dmr-1.0.2-py3-none-win_amd64.whl` |
| Linux x86-64 | `dmr-1.0.2-linux-x64.tar.gz` | `libscanner_dmr-1.0.2-py3-none-manylinux_2_36_x86_64.whl` |
| Linux ARM64 (Raspberry Pi 3/4/5, 64 бит) | `dmr-1.0.2-linux-arm64.tar.gz` | `libscanner_dmr-1.0.2-py3-none-manylinux_2_36_aarch64.whl` |
| Linux ARMv7 (Raspberry Pi 2/3/4/5, 32 бит) | `dmr-1.0.2-linux-armhf.tar.gz` | `libscanner_dmr-1.0.2-py3-none-manylinux_2_36_armv7l.whl` |

Для каждой платформы есть и архив только с утилитой `dmr` (`…-cli.zip` / `…-cli.tar.gz`).

- Сборки под Windows сделаны MinGW-w64 GCC (MSYS2, оболочка MINGW64). Из MSVC и других компиляторов — через C ABI (`dmr_c.dll`).
- Сборки под Linux сделаны GCC 12 на Debian 12 (glibc 2.36): для C++ API нужен GCC 12 или новее; колёсам — glibc 2.36+ (Debian 12 / Raspberry Pi OS Bookworm и новее).
- Python 3.9 и новее, без зависимостей.

## Загрузка

- **Сборки:** [выпуски на GitHub → последний](https://github.com/libscanner/datamatrix-reader/releases/latest) — архивы для каждой платформы и колёса Python.
- **Python:** [PyPI → libscanner-dmr](https://pypi.org/project/libscanner-dmr/) — `pip install libscanner-dmr`, импорт — `dmr`.
- **Лицензии:** тарифы на <https://libscanner.com/> — 3 месяца, 6 месяцев, год или бессрочно (например, <https://libscanner.com/products/datamatrix-gs1-yearly>). Ключ активации появляется в личном кабинете после покупки.

### Пробный период

Любая сборка работает **30 дней с первого запуска на машине** — без регистрации, без ключа и без обращения к сети. Дальше нужен ключ лицензии с <https://libscanner.com>; без него `scan()` сразу возвращает `Status::Unlicensed` и кадр не обрабатывает.

## Состав поставки

```text
include/dmr/{Dmr,Diagnostics,Geometry,Image,License,Scanner,Settings,Version}.h   C++ API
include/dmr/dmr_c.h                C ABI
lib/libdmr.a                       статическая библиотека C++
lib/pkgconfig/dmr.pc
lib/libdmr_c.so | bin/dmr_c.dll    библиотека C ABI (+ lib/dmr_c.dll.a под Windows)
lib/pkgconfig/dmr_c.pc
bin/dmr                            утилита командной строки
share/doc/dmr/licenses/            лицензии стороннего кода
```

Заголовки в [`include/`](https://github.com/libscanner/datamatrix-reader/tree/main/include/dmr) этого репозитория объявляют тот же API, что заголовки поставки 1.0.2; отличаются только комментарии (здесь — на английском, в поставке — на русском).

## Быстрый старт

### C++

```cpp
#include <dmr/Dmr.h>
#include <iostream>

int main() {
    dmr::Image frame;
    if (dmr::loadImage("frame.jpg", frame) != dmr::LoadStatus::Ok) {
        std::cerr << "не удалось прочитать файл\n";
        return 1;
    }

    dmr::Scanner scanner;                          // поток кадров одной этикетки
    const dmr::ScanResult r = scanner.scan(frame);  // Image сам приводится к ImageView

    if (r.ok()) {
        std::cout << r.text() << "\n";
    } else if (r.status == dmr::Status::NotFound) {
        std::cout << "символа на кадре нет\n";
    } else {
        std::cout << "символ найден, но не прочитан\n";
    }
}
```

Статическая линковка через pkg-config — запрос **обязательно** с `--static`:

```sh
PKG_CONFIG_PATH=/путь/к/dmr-1.0.2-linux-x64/lib/pkgconfig pkg-config --static --cflags --libs dmr
```

В Meson:

```meson
dmr_dep = dependency('dmr', required: true, static: true)
executable('myprogram', 'main.cpp', dependencies: [dmr_dep],
  link_args: ['-static'])   # Windows: рантайм MinGW внутрь программы
```

```sh
meson setup build --pkg-config-path="$PWD/dmr-1.0.2-windows-x64/lib/pkgconfig"
meson compile -C build
```

Кадр из собственного конвейера захвата (SDK камеры, буфер OpenCV, битмап) передаётся видом на уже имеющийся буфер, без копирования:

```cpp
const dmr::ImageView view{ m.data, m.cols, m.rows, (int)m.step, dmr::PixelFormat::Bgr8 };
const dmr::ScanResult r = scanner.scan(view);
```

Полный пример: [`examples/cpp`](https://github.com/libscanner/datamatrix-reader/tree/main/examples/cpp). Пример на C ABI (MSVC, другие языки): [`examples/c`](https://github.com/libscanner/datamatrix-reader/tree/main/examples/c).

### Python

```sh
pip install libscanner-dmr     # PyPI: Windows x64, Linux x86-64 / ARM64 / ARMv7 (glibc 2.36+)
```

```python
import dmr

img = dmr.Image("кадр.jpg", dmr.LoadAs.GRAY)   # сканеру цвет не нужен
scanner = dmr.Scanner()                         # поток кадров, умолчание
result = scanner.scan(img.view())
if result.ok:
    print(result.text.decode("utf-8"))
```

`Code.text` — `bytes`: разделитель GS1 остаётся байтом `0x1D`. Колесо ставит и команду `dmr`. Пример: [`examples/python`](https://github.com/libscanner/datamatrix-reader/tree/main/examples/python).

### Командная строка

```sh
dmr label.jpg
dmr --folder ./frames
dmr --license status --json
```

Другие команды: [`examples/cli`](https://github.com/libscanner/datamatrix-reader/tree/main/examples/cli).

## Документация

- C++ / C ABI / утилита: <https://libscanner.com/docs/datamatrix-gs1/> (English: <https://libscanner.com/en/docs/datamatrix-gs1/>)
- Python: <https://libscanner.com/docs/datamatrix-gs1/?lang=python> (English: <https://libscanner.com/en/docs/datamatrix-gs1/?lang=python>)

## Лицензия

Проприетарная — см. [LICENSE.txt](https://github.com/libscanner/datamatrix-reader/blob/main/LICENSE.txt) (лицензионное соглашение с пользователем; юридическую силу имеет русский текст, [LICENSE.en.txt](https://github.com/libscanner/datamatrix-reader/blob/main/LICENSE.en.txt) — перевод на английский для сведения). Начало использования ПО означает согласие с соглашением.

Сторонние компоненты: [THIRD-PARTY-NOTICES.ru.txt](https://github.com/libscanner/datamatrix-reader/blob/main/THIRD-PARTY-NOTICES.ru.txt) (на английском — [THIRD-PARTY-NOTICES.txt](https://github.com/libscanner/datamatrix-reader/blob/main/THIRD-PARTY-NOTICES.txt)) и тексты лицензий в [`licenses/`](https://github.com/libscanner/datamatrix-reader/tree/main/licenses).

Код в [`examples/`](https://github.com/libscanner/datamatrix-reader/tree/main/examples) — под лицензией MIT-0 ([examples/LICENSE](https://github.com/libscanner/datamatrix-reader/blob/main/examples/LICENSE)): копируйте в свои проекты свободно; сама библиотека — проприетарная (LICENSE.txt).

## Связь

- Почта: [libscanner@yandex.com](mailto:libscanner@yandex.com)
- Форма обратной связи: <https://libscanner.com/feedback>
- Уязвимости: см. [SECURITY.md](https://github.com/libscanner/datamatrix-reader/blob/main/SECURITY.md)

«GS1» — товарный знак GS1 AISBL. «Честный знак» — товарный знак ООО «Оператор-ЦРПТ». Продукт не связан с владельцами этих знаков и ими не сертифицирован; названия упоминаются только для описания поддерживаемого формата.
