# Nebbie Editor

Editor di mondo per [Nebbie Arcane](https://github.com/NebbieArcane/Server), sviluppato per **Linux** e **macOS**.

Questo repository è indipendente dal codice del server: non servono permessi sull'organizzazione `NebbieArcane`. Il riferimento di formato è il codice in `NebbieArcane/Server` (`src/db.cpp`, `src/db.hpp`).

## File supportati

| File | Contenuto | Stato |
|------|-----------|-------|
| `myst.zon` | Zone e reset | Lettura/scrittura |
| `myst.wld` | Stanze | Lettura/scrittura |
| `myst.mob` | Mob | Lettura/scrittura |
| `myst.obj` | Oggetti | Lettura/scrittura |
| `myst.shp` | Negozi | Lettura/scrittura |
| `myst.spe` | Special proc | Lettura/scrittura |
| `myst.dam` | Messaggi danno | Lettura/scrittura |
| `myst.act` | Social | Lettura/scrittura |
| `myst.pos` | Pose | Lettura/scrittura |
| `myst.gui` | Gilde | Lettura/scrittura |

## Build (Linux e macOS)

```bash
./scripts/install-deps.sh    # dipendenze di sistema
./scripts/build.sh --test    # configure, build, test
```

### Linux (Debian/Ubuntu)

```bash
sudo apt-get install -y build-essential cmake qt6-base-dev
./scripts/build.sh --test
./build/nebbiedit/nebbiedit info tests/fixtures
./build/nebbie-qt/nebbieedit tests/fixtures
```

### macOS

```bash
# Xcode Command Line Tools + Homebrew: https://brew.sh
./scripts/install-deps.sh
./scripts/build.sh --test
./build/nebbiedit/nebbiedit info tests/fixtures
./build/nebbie-qt/nebbieedit tests/fixtures
```

App bundle macOS (opzionale):

```bash
./scripts/build.sh --macos-bundle
./scripts/install-macos.sh          # copia in /Applications
open /Applications/nebbieedit.app   # primo avvio: selezione lib salvata in config
```

Installazione Linux:

```bash
./scripts/install-linux.sh ~/.local
```

Pacchetti installabili:

```bash
# Debian/Ubuntu (.deb) — su Linux
./scripts/package-deb.sh
sudo dpkg -i dist/nebbie-editor_*_amd64.deb
sudo apt-get install -f

# macOS (.dmg) — su macOS
./scripts/package-dmg.sh
open dist/nebbie-editor_*_macos.dmg
```

**Windows non è supportato** (solo Linux e macOS). Vedi [docs/PLATFORM.md](docs/PLATFORM.md).

## Uso CLI (MVP)

```bash
# Ispeziona
./build/nebbiedit/nebbiedit info tests/fixtures
./build/nebbiedit/nebbiedit validate tests/fixtures

# Modifica one-shot (carica → modifica → valida → salva)
./build/nebbiedit/nebbiedit room set tests/fixtures 3001 --name "Nuova stanza" --desc "Testo"
./build/nebbiedit/nebbiedit mob set tests/fixtures 1 --short "Puff aggiornato" --level 30
./build/nebbiedit/nebbiedit obj set tests/fixtures 1 --short "Elmo nuovo" --cost 99

# Sessione interattiva
./build/nebbiedit/nebbiedit edit tests/fixtures
```

## Interfaccia grafica (Qt)

Richiede Qt 6. Funzioni: browse/edit stanze-mob-oggetti, creazione entità, ricerca, uscite, reset zona, valida, salva, **autosalvataggio** e cronologia versioni (`.nebbie/` nella lib).

```bash
./build/nebbie-qt/nebbieedit /path/to/mudroot/lib
```

## Uso CLI (generale)

```bash
./build/nebbiedit/nebbiedit zone list
./build/nebbiedit/nebbiedit room show 3001
./build/nebbiedit/nebbiedit convert lib roundtrip tests/fixtures /tmp/nebbie-rt
```

## Dati di test

```bash
./scripts/fetch-test-data.sh
./build/nebbiedit/nebbiedit info vendor/nebbietest/mudroot/lib
```

## Struttura

```
nebbie-core/   Libreria C++17 (parser + modello)
nebbiedit/     CLI
nebbie-qt/     GUI Qt (nebbieedit)
scripts/       install-deps.sh, build.sh (Linux + macOS)
tests/         Fixture e test automatici
docs/          Documentazione tecnica
```

## Riferimenti

- Server ufficiale: https://github.com/NebbieArcane/Server
- Fork di test: https://github.com/wizardmorgan/nebbietest
- Piattaforme: [docs/PLATFORM.md](docs/PLATFORM.md)
- Roadmap: [docs/ROADMAP.md](docs/ROADMAP.md)

## Licenza

MIT
