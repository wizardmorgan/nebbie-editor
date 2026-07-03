# Nebbie Editor

Editor di mondo per [Nebbie Arcane](https://github.com/NebbieArcane/Server), sviluppato per **Linux** e in futuro **macOS**.

Questo repository è indipendente dal codice del server: non servono permessi sull'organizzazione `NebbieArcane`. Il riferimento di formato è il codice in `NebbieArcane/Server` (`src/db.cpp`, `src/db.hpp`).

## File supportati (roadmap)

| File | Contenuto | Stato |
|------|-----------|-------|
| `myst.zon` | Zone e reset | Lettura/scrittura iniziale |
| `myst.wld` | Stanze | Lettura/scrittura iniziale |
| `myst.mob` | Mob | Pianificato |
| `myst.obj` | Oggetti | Pianificato |
| `myst.shp` | Negozi | Pianificato |
| `myst.spe` | Special proc | Pianificato |
| `myst.dam` | Messaggi danno | Pianificato |
| `myst.act` | Social | Pianificato |
| `myst.pos` | Pose | Pianificato |
| `myst.gui` | Gilde | Pianificato |

## Build

```bash
CXX=g++ cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## Uso CLI

```bash
./build/nebbiedit/nebbiedit info tests/fixtures
./build/nebbiedit/nebbiedit load /path/to/mudroot/lib
./build/nebbiedit/nebbiedit zone list
./build/nebbiedit/nebbiedit room show 3001
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
tests/         Fixture e test automatici
docs/          Documentazione tecnica
```

## Riferimenti

- Server ufficiale: https://github.com/NebbieArcane/Server
- Fork di test: https://github.com/wizardmorgan/nebbietest
- Editor Diku legacy (non Nebbie): https://github.com/wizardmorgan/DikuEdit

## Licenza

MIT
