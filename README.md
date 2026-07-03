# Nebbie Editor

Editor di mondo per [Nebbie Arcane](https://github.com/NebbieArcane/Server), sviluppato per **Linux** e in futuro **macOS**.

Questo repository è indipendente dal codice del server: non servono permessi sull'organizzazione `NebbieArcane`. Il riferimento di formato è il codice in `NebbieArcane/Server` (`src/db.cpp`, `src/db.hpp`).

## File supportati (roadmap)

| File | Contenuto | Stato |
|------|-----------|-------|
| `myst.zon` | Zone e reset | Lettura/scrittura |
| `myst.wld` | Stanze | Lettura/scrittura |
| `myst.mob` | Mob | Lettura/scrittura iniziale |
| `myst.obj` | Oggetti | Lettura/scrittura iniziale |
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
./build/nebbiedit/nebbiedit mob list
./build/nebbiedit/nebbiedit obj show 1
./build/nebbiedit/nebbiedit convert lib roundtrip tests/fixtures /tmp/nebbie-rt
```

## Pubblicazione su GitHub

Repository: **https://github.com/wizardmorgan/nebbie-editor**

> **403 con `cursor[bot]`?** L'agent cloud non può pushare sul tuo account senza permessi di scrittura.  
> Pusha dal tuo PC con il tuo login GitHub, oppure configura l'app Cursor.  
> Guida: [docs/PUBBLICAZIONE.md](docs/PUBBLICAZIONE.md)

```bash
# Dal tuo computer:
git remote set-url origin https://github.com/wizardmorgan/nebbie-editor.git
git push -u origin cursor/nebbie-editor-initial-c774
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
