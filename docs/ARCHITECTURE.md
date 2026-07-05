# Architettura Nebbie Editor

## Principio guida

Il parser del server (`NebbieArcane/Server` / `nebbietest`, `src/db.cpp`) è la specifica. L'editor non deve inventare formati alternativi.

## Layout dati server (`mudroot/lib/`)

```
myst.zon  myst.wld  myst.mob  myst.obj  myst.shp  myst.spe
myst.dam  myst.act  myst.pos  myst.gui

objects/    # override per-vnum oggetto (oltre a myst.obj)
mobiles/    # override per-vnum mobile (indice; lettura runtime incompleta)
rooms/      # override per-vnum stanza (oltre a myst.wld)
zones/      # override reset per zona (oltre a myst.zon)
world/      # stato runtime stanza (oggetti/mob a terra), NON prototipi
```

### Modello: monolite + overlay

| Percorso | Ruolo | Boot server | Editor oggi |
|----------|-------|-------------|-------------|
| `myst.obj` | Catalogo oggetti | Base | ✅ carica/salva |
| `objects/<vnum>` | Oggetto singolo | **Sostituisce** la voce in `myst.obj` per quel vnum; può esistere **solo** in directory | ✅ export/apply |
| `myst.wld` | Catalogo stanze | Base | ✅ |
| `rooms/<vnum>` | Stanza singola | **Sovrascrive** la stanza già caricata da `myst.wld` | ✅ export/apply |
| `myst.zon` | Zone + reset | Base | ✅ |
| `zones/<n>.zon` | Reset di una zona | **Sovrascrive** i comandi reset della zona | ✅ export/apply |
| `myst.mob` | Catalogo mob | Base | ✅ |
| `mobiles/<vnum>` | Mob singolo | Indicizzato come oggetti, ma `read_mobile()` **non** legge `pos==-1` | ⚠️ export/apply (sperimentale) |
| `myst.shp/spe/dam/act/pos/gui` | Tabelle monolitiche | Solo file | ✅ |
| `world/<vnum>` | Inventario stanza live | Dopo boot | ❌ (non è editing mondo) |

**Ordine boot** (`boot_db`): `myst.zon` → `zones/*` → `myst.wld` → `rooms/*` → indici `myst.mob`+`mobiles/` → indici `myst.obj`+`objects/` → …

### Serve cambiare il mud?

- **Oggetti / stanze / zone (reset):** no per il modello attuale; il server già supporta overlay. L'editor può continuare a scrivere solo `myst.*` oppure, in futuro, emulare `osave`/`rsave`/`zsave` scrivendo nelle directory.
- **Mobiles:** sì, **lato server** manca il branch `pos==-1` in `read_mobile()` (gli oggetti lo hanno in `read_object()`). Finché non è allineato, `mobiles/` è rischioso.
- **Shop/spe/dam/act/pos/gui:** nessuna directory analoga oggi; resterebbero monolitici o richiederebbero nuova logica server.

### Approccio editor (evoluzione)

1. **Ora:** `World` in memoria da `myst.*` + overlay (`load_lib` applica `rooms/`, `objects/`, `mobiles/`, `zones/`).
2. **Export:** `export_myst_to_overlays()` / CLI `nebbiedit overlay export <lib>` scrive file per-entità compatibili col boot server.
3. **Save:** modalità sicura (`myst.*` only) vs compatibile server (file in overlay per entità modificate).
4. **DB (roadmap):** stesso modello `World`, persistenza alternativa; i file restano source of truth finché il boot non migra.

## Moduli editor

1. **nebbie-core** — modello in memoria + I/O (portable C++17)
2. **nebbiedit** — CLI Linux/macOS
3. **nebbie-qt** — GUI Qt (`nebbieedit`): browse + edit + save + mappa

Vedi [ROADMAP.md](ROADMAP.md) per mappa interattiva, packaging e database.
