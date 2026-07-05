# Piattaforme supportate

Nebbie Editor supporta **Linux** e **macOS** come piattaforme di pari livello.

## Requisiti

| Componente | Linux | macOS |
|------------|-------|-------|
| Compilatore | GCC 9+ o Clang 10+ | Xcode CLT / Clang (Apple) |
| CMake | 3.16+ | 3.16+ (Homebrew) |
| C++ | C++17 | C++17 |
| Qt 6 (GUI) | `qt6-base-dev` | `qt@6` (Homebrew) |

## Build rapido

```bash
./scripts/install-deps.sh
./scripts/build.sh --test
```

### macOS (dettaglio)

```bash
xcode-select --install          # se mancano gli Xcode Command Line Tools
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
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
2. **API portable** — `std::filesystem`, `std::fstream` / `FILE*`, niente dipendenze OS-specific nel parser.
3. **CI obbligatoria su entrambe le piattaforme** — ogni PR deve passare `build-linux` e `build-macos`.
4. **Script bash POSIX** — gli script in `scripts/` devono funzionare su Linux e macOS (bash 3.2+ su macOS).
5. **Qt cross-platform** — la GUI usa solo Qt Widgets standard; testare su entrambi i sistemi quando si tocca `nebbie-qt/`.
6. **Path utente** — usare `std::filesystem::path` e `QString::fromUtf8` per percorsi con caratteri non ASCII.

## CI

GitHub Actions:

- `build-linux` — `ubuntu-latest`
- `build-macos` — `macos-latest`
- `packages` — genera `.deb` (Linux) e `.dmg` (macOS) come artifact

Entrambi i job build eseguono: configure, build, ctest, smoke CLI, validazione fixture, MVP edit, roundtrip phase-4, verifica binario Qt.

## Pacchetti

| Piattaforma | Script | Output |
|-------------|--------|--------|
| Linux (Debian/Ubuntu) | `./scripts/package-deb.sh` | `dist/nebbie-editor_<version>_<arch>.deb` |
| macOS | `./scripts/package-dmg.sh` | `dist/nebbie-editor_<version>_macos.dmg` |
| Auto (OS corrente) | `./scripts/package.sh` | come sopra |

Il `.deb` installa `nebbiedit` e `nebbieedit` in `/usr/bin` e il file `.desktop`.
Il `.dmg` contiene `nebbieedit.app`, `bin/nebbiedit` e un collegamento ad `Applications`.

**Windows:** non supportato in questa versione.

## Risoluzione problemi

### macOS: Qt non trovato

```bash
export CMAKE_PREFIX_PATH="$(brew --prefix qt@6)"
./scripts/build.sh
```

### macOS: `nebbieedit` non si apre da Finder

Usa `./scripts/build.sh --macos-bundle` e poi:

```bash
./scripts/install-macos.sh    # copia in /Applications
open /Applications/nebbieedit.app
```

Al primo avvio (senza argomenti) l'app chiede la cartella `mudroot/lib` e la salva in:

`~/Library/Application Support/Nebbie/nebbieedit.conf`

L'icona dell'app è in `nebbie-qt/icons/` (`nebbieedit.icns`). Per rigenerarla da sorgente:

```bash
pip install pillow   # se non già installato
python3 scripts/make-macos-icon.py
./scripts/build.sh --macos-bundle
```

### Linux: solo CLI senza Qt

```bash
./scripts/build.sh --no-qt
```
