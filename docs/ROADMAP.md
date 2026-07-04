# Roadmap Nebbie Editor

## Completato

- [x] Parser `myst.zon` … `myst.gui` (lettura/scrittura)
- [x] Validazione referenze (`nebbiedit validate`)
- [x] MVP editing CLI + GUI Qt completa
- [x] Supporto Linux + macOS (CI, app bundle, icona)
- [x] Caricamento lib nebbietest reale
- [x] Autosalvataggio workspace (10s) + versioni (60s) in `.nebbie/`
- [x] Backup pre-salvataggio, cronologia ripristino
- [x] Validazione GUI con navigazione al vnum
- [x] **Prototipo mappa**: tab Mappa + `nebbiedit zone graph --dot`
- [x] **Mappa interattiva (MVP)**: `QGraphicsView`, zoom/pan, doppio clic → stanza
- [x] **Config libreria predefinita**: file testo `nebbieedit.conf` + dialogo al primo avvio

## Prossimo passo immediato

**Mappa Fase 6c** — piani Z per su/giù, filtri link rotti, layout migliorato.

## Fase 6 — Mappa zone

### Stato attuale (master)

- Tab **Mappa**: selezione zona, statistiche, export Graphviz DOT
- API `build_zone_graph`, `zone_graph_to_dot`, `rooms_in_zone`
- Archi su/giù come tratteggio blu nel DOT

### Prossimo incremento

1. `QGraphicsView` interattivo (nodi stanza, frecce direzionali)
2. Piani Z per su/giù (selettore livello)
3. Doppio clic → tab Stanze
4. Filtri: link rotti, teleport

Vedi sezione "Rappresentazione visiva" sotto per 2D vs 3D.

### Rappresentazione visiva

| Approccio | Uso consigliato |
|-----------|-----------------|
| Grafo 2D per piano | **MVP interattivo** |
| Piani multipli (layer Z) | **Fase 6b** |
| Vista 3D | Opzionale, dopo MVP |

## Fase 7 — Distribuzione e installazione

Conviene **dopo** le funzioni editor principali (mappa interattiva, editing stabile), ma la base è già predisposta.

### macOS

| Stadio | Cosa | Stato |
|--------|------|-------|
| A | `./scripts/build.sh --macos-bundle` → `.app` | ✅ |
| B | `./scripts/install-macos.sh` → copia in `/Applications` | ✅ |
| C | DMG firmato, notarizzazione, Sparkle/update | Roadmap |

**Risposta:** sì, mettere Applications/DMG/notarizzazione **per ultimo** tra i task prodotto ha senso; intanto `install-macos.sh` evita di lanciare l'app dalla cartella `build/`.

Config utente (indipendente dall'installazione):

```
~/Library/Application Support/Nebbie/nebbieedit.conf
```

### Linux

| Stadio | Cosa | Stato |
|--------|------|-------|
| A | `cmake --install` + `.desktop` | ✅ `scripts/install-linux.sh` |
| B | Pacchetto `.deb` / AppImage | Roadmap |
| C | Flatpak | Opzionale |

```bash
./scripts/install-linux.sh /usr/local   # oppure ~/.local
```

Config:

```
~/.config/Nebbie/nebbieedit.conf
```

Contenuto:

```ini
# Nebbie Editor configuration
lib_path=/path/to/mudroot/lib
```

## Fase 8 — Database Nebbie (persistenza oltre i file myst)

Obiettivo: **integrazione con il database del server Nebbie** in tabelle dedicate, senza perdere nessun campo dei file `myst.*`, con migrazione bidirezionale file ↔ DB.

### Priorità (come richiesto)

1. **Zone** (`myst.zon`) — metadati zona, range vnum, reset
2. **Stanze** (`myst.wld`) — uscite, flag, settore, teleport, extra desc
3. **Oggetti** (`myst.obj`) — tipo, valori, affect, extra
4. **Dopo:** mob, shop, special proc, dam/act/pos/gui

### Schema (bozza)

- `nebbie_zones`, `nebbie_zone_resets`
- `nebbie_rooms`, `nebbie_room_exits`, `nebbie_room_extra_desc`
- `nebbie_objects`, `nebbie_object_affects`, …
- Colonne per ogni campo del parser attuale + JSON per estensioni future
- `source_revision` / `updated_at` per sync con file

### Strategia

1. **nebbie-core**: layer `World` invariato; adapter `load_from_db` / `save_to_db`
2. Script SQL migrazione iniziale + test roundtrip DB vs `myst.*`
3. Modalità editor: file, DB, o ibrido (file master finché il server non migra)
4. Validazione referenze anche su FK DB

Non sostituisce i file finché il boot server non supporta DB; l'editor può offrire export/import.

## Fase 9 — Editing avanzato

- Editor shop / special proc / social in GUI
- Diff e roundtrip su lib reale
- Validazione vs boot server nebbietest (CI)

## Fase 10 — Mappa mondo (opzionale)

- Vista macro-zone (collegamenti inter-zona)
- Export immagine / Graphviz

## Sessione e versioning (GUI)

```
mudroot/lib/.nebbie/
  workspace/     # autosalvataggio ogni 10s
  versions/      # cronologia ogni 60s + pre-save
```

## Direzioni uscita

| D# | Nome | Layout 2D |
|----|------|-----------|
| 0 | nord | +Y |
| 1 | est | +X |
| 2 | sud | −Y |
| 3 | ovest | −X |
| 4 | su | layer Z+1 |
| 5 | giù | layer Z−1 |
