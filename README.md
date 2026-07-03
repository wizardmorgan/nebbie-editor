# Nebbie Editor

Editor di mondo per [Nebbie Arcane](https://github.com/NebbieArcane/Server), sviluppato per **Linux** e in futuro **macOS**.

Questo repository è indipendente dal codice del server: non servono permessi sull'organizzazione `NebbieArcane`. Il riferimento di formato è il codice in `NebbieArcane/Server` (`src/db.cpp`, `src/db.hpp`).

## File supportati (roadmap)

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

## Build

```bash
CXX=g++ cmake -S . -B build
cmake --build build
ctest --test-dir build
```

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

Comandi utili in `edit`: `room set`, `mob set`, `obj set`, `validate`, `save`, `quit`.

## Interfaccia grafica (Qt)

Richiede Qt 6 (`qt6-base-dev` su Debian/Ubuntu).

```bash
sudo apt-get install qt6-base-dev   # Linux
CXX=g++ cmake -S . -B build && cmake --build build
./build/nebbie-qt/nebbieedit tests/fixtures
```

Funzioni MVP: apri libreria, elenco stanze/mob/oggetti, modifica campi base, valida, salva.
Creazione nuove entità, ricerca per vnum/nome, mappatura uscite tra stanze (tab Stanze).

## Uso CLI (generale)

```bash
./build/nebbiedit/nebbiedit info tests/fixtures
./build/nebbiedit/nebbiedit load /path/to/mudroot/lib
./build/nebbiedit/nebbiedit zone list
./build/nebbiedit/nebbiedit room show 3001
./build/nebbiedit/nebbiedit mob list
./build/nebbiedit/nebbiedit obj show 1
./build/nebbiedit/nebbiedit validate tests/fixtures
./build/nebbiedit/nebbiedit convert lib roundtrip tests/fixtures /tmp/nebbie-rt
```

## Pubblicazione su GitHub

Repository: **https://github.com/wizardmorgan/nebbie-editor**

### Push dall'agent cloud (soluzione definitiva)

1. Crea un **fine-grained PAT** su GitHub (Contents: Read/Write su `nebbie-editor`)
2. Aggiungilo in **Cursor Dashboard → Cloud Agents → Secrets** come `WIZARDMORGAN_GITHUB_PAT`
3. Riavvia l'agent, poi: `./scripts/git-push.sh`

Guida completa: [docs/GIT-PUSH-DEFINITIVO.md](docs/GIT-PUSH-DEFINITIVO.md)

### Push dal PC

```bash
git clone https://github.com/wizardmorgan/nebbie-editor.git
cd nebbie-editor
git push origin cursor/nebbie-editor-initial-c774
```

## Dati di test

Per testare contro un server reale puoi usare il fork:

```bash
./scripts/fetch-test-data.sh
./build/nebbiedit/nebbiedit info vendor/nebbietest/mudroot/lib
```

## Struttura

```
nebbie-core/   Libreria C++17 (parser + modello)
nebbiedit/     CLI
nebbie-qt/     GUI Qt (nebbieedit)
tests/         Fixture e test automatici
docs/          Documentazione tecnica
```

## Riferimenti

- Server ufficiale: https://github.com/NebbieArcane/Server
- Fork di test: https://github.com/wizardmorgan/nebbietest
- Editor Diku legacy (non Nebbie): https://github.com/wizardmorgan/DikuEdit

## Licenza

MIT
