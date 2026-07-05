# Piattaforme supportate

Nebbie Editor supporta **Linux**, **macOS** e **Windows** come piattaforme di pari livello.

## Requisiti

| Componente | Linux | macOS | Windows |
|------------|-------|-------|---------|
| Compilatore | GCC 9+ o Clang 10+ | Xcode CLT / Clang | MSVC 2019+ (VS 2022) |
| CMake | 3.16+ | 3.16+ (Homebrew) | 3.16+ |
| C++ | C++17 | C++17 | C++17 |
| Qt 6 (GUI) | `qt6-base-dev` | `qt@6` (Homebrew) | Qt 6 MSVC kit |

## Build rapido

Linux / macOS:

```bash
./scripts/install-deps.sh
./scripts/build.sh --test
```

Windows (PowerShell):

```powershell
.\scripts\install-deps.ps1
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.5.3\msvc2019_64"
.\scripts\build.ps1 -Test
```

### Windows (dettaglio)

1. Installa [Visual Studio 2022 Build Tools](https://visualstudio.microsoft.com/downloads/) con workload **Desktop development with C++**
2. Installa [CMake](https://cmake.org/download/) e [Qt 6](https://www.qt.io/download-open-source) (kit MSVC 64-bit)
3. Imposta `CMAKE_PREFIX_PATH` sul root del kit Qt
4. Build e test:

```powershell
.\scripts\build.ps1 -Test
.\build\nebbiedit\Release\nebbiedit.exe info tests\fixtures
.\build\nebbie-qt\Release\nebbieedit.exe
```

Config libreria predefinita (GUI): `%APPDATA%\Nebbie\nebbieedit.conf`

Pacchetto zip portatile (include DLL Qt via `windeployqt`):

```powershell
.\scripts\package-windows.ps1
```

### macOS (dettaglio)

```bash
xcode-select --install
./scripts/install-deps.sh
./scripts/build.sh --test
```

Avvio GUI:

```bash
./scripts/build.sh --macos-bundle
open build/nebbie-qt/nebbieedit.app
```

Oppure senza bundle:

```bash
./build/nebbie-qt/nebbieedit tests/fixtures
```

### Linux (dettaglio)

```bash
sudo apt-get install -y build-essential cmake qt6-base-dev
./scripts/build.sh --test
./scripts/install-linux.sh /usr/local   # oppure ~/.local
```

Config libreria predefinita (GUI): `~/.config/Nebbie/nebbieedit.conf`

## Regole di sviluppo

1. **Niente codice solo-Linux** — niente `apt` nel codice C++, niente path fissi tipo `/usr/...` nel core.
2. **API portable** — `std::filesystem`, I/O binario via `nebbie::open_file_read/write`, niente dipendenze OS-specific nel parser.
3. **CI obbligatoria su tutte le piattaforme** — ogni PR deve passare `build-linux`, `build-macos` e `build-windows`.
4. **Script** — bash su Linux/macOS, PowerShell su Windows (`build.ps1`, `package-windows.ps1`).
5. **Qt cross-platform** — la GUI usa solo Qt Widgets standard; path Unicode via `path_util.hpp`.
6. **Path utente** — `std::filesystem::path` nativo; su Qt usare `path_from_qstring` / `qstring_from_path`.

## CI

GitHub Actions:

- `build-linux` — `ubuntu-latest`
- `build-macos` — `macos-latest`
- `build-windows` — `windows-latest`
- `packages` — genera `.deb`, `.dmg` e `.zip` Windows come artifact

## Pacchetti

| Piattaforma | Script | Output |
|-------------|--------|--------|
| Linux (Debian/Ubuntu) | `./scripts/package-deb.sh` | `dist/nebbie-editor_<version>_<arch>.deb` |
| macOS | `./scripts/package-dmg.sh` | `dist/nebbie-editor_<version>_macos.dmg` |
| Windows | `.\scripts\package-windows.ps1` | `dist/nebbie-editor_<version>_windows.zip` + `*_windows_setup.exe` |
| Windows (solo installer) | `.\scripts\package-windows-installer.ps1` | `dist/nebbie-editor_<version>_windows_setup.exe` |
| Auto (Linux/macOS) | `./scripts/package.sh` | `.deb` o `.dmg` |

Il `.deb` installa `nebbiedit` e `nebbieedit` in `/usr/bin` e il file `.desktop`.
Il `.dmg` contiene `nebbieedit.app`, `bin/nebbiedit` e un collegamento ad `Applications`.
Lo zip Windows contiene `nebbiedit.exe`, `nebbieedit.exe` e le DLL Qt necessarie.
L’installer Inno Setup (`.exe`) aggiunge collegamenti nel menu Start, opzione icona desktop e opzione PATH per la CLI.

**Installer:** installa Inno Setup 6, poi `.\scripts\package-windows-installer.ps1`.

## Risoluzione problemi

### Windows: Qt non trovato

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.5.3\msvc2019_64"
.\scripts\build.ps1
```

### Windows: percorsi Unicode

Usa `QCoreApplication::arguments()` e seleziona la libreria dalla GUI; il core apre i file con `_wfopen` / path wide.

### macOS: Qt non trovato

```bash
export CMAKE_PREFIX_PATH="$(brew --prefix qt@6)"
./scripts/build.sh
```

### macOS: `nebbieedit` non si apre da Finder

Usa `./scripts/build.sh --macos-bundle` e poi:

```bash
./scripts/install-macos.sh
open /Applications/nebbieedit.app
```

Al primo avvio l'app chiede la cartella `mudroot/lib` e la salva in:

`~/Library/Application Support/Nebbie/nebbieedit.conf`

### Linux: solo CLI senza Qt

```bash
./scripts/build.sh --no-qt
```
