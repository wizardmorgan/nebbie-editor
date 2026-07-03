# Architettura Nebbie Editor

## Principio guida

Il parser del server (`NebbieArcane/Server`, `src/db.cpp`) è la specifica. L'editor non deve inventare formati alternativi.

## Layout dati server

```
mudroot/lib/
  myst.zon myst.wld myst.mob myst.obj myst.shp myst.spe
  myst.dam myst.act myst.pos myst.gui
  objects/ mobiles/ rooms/ zones/
```

## Moduli editor

1. **nebbie-core** — modello in memoria + I/O
2. **nebbiedit** — CLI Linux/macOS
3. **nebbie-qt** (futuro) — GUI

## Fasi

1. `myst.zon` + `myst.wld` (in corso)
2. `myst.mob` + `myst.obj`
3. `myst.shp` + `myst.spe`
4. `myst.dam` + `myst.act` + `myst.pos` + `myst.gui`
5. Validazione referenze incrociate (`validate_world`, `nebbiedit validate`)
6. **MVP editing** — `save_lib`, `nebbiedit edit`, one-shot `room/mob/obj set`
7. Validazione integrazione con boot del server
