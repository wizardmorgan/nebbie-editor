# Nebbie Editor — riepilogo progetto

Documento di sintesi ad alto livello: obiettivi, architettura, funzionalità, decisioni e passi operativi seguiti dall’avvio del progetto alla distribuzione multi-piattaforma. Non è un log delle conversazioni di sviluppo.

---

## 1. Obiettivo

**Nebbie Editor** è un editor di mondo per [Nebbie Arcane](https://github.com/NebbieArcane/Server): legge e scrive i file `myst.*` nella directory `mudroot/lib/` del server, con la stessa semantica del boot runtime.

**Principio guida:** il parser del server (`src/db.cpp`) è la specifica. L’editor non inventa formati alternativi.

**Repository indipendente** dal codice del server: sviluppo, test e distribuzione senza permessi sull’organizzazione NebbieArcane. Per i test si usa il fork [nebbietest](https://github.com/wizardmorgan/nebbietest).

---

## 2. Architettura software

| Modulo | Ruolo |
|--------|--------|
| **nebbie-core** | Libreria C++17: modello `World` in memoria, parser/scrittura `myst.*`, validazione, editing, overlay, sessioni |
| **nebbiedit** | CLI per ispezione, modifica one-shot, sessione interattiva, export overlay |
| **nebbie-qt** (`nebbieedit`) | GUI Qt 6: browse/edit, mappe, coordinator, autosalvataggio |
| **services/nebbie-coordinator** | API PHP + SQLite per prenotazioni vnum tra builder (opzionale) |

Il flusso dati canonico:

```
myst.* (+ overlay opzionali)  →  load_lib()  →  World  →  edit/validate  →  save_lib()  →  myst.*
```

---

## 3. Fasi di sviluppo (cronologia logica)

### Fase 1 — Parser e modello base

- Implementazione lettura/scrittura per tutti i file `myst.zon`, `myst.wld`, `myst.mob`, `myst.obj`, `myst.shp`, `myst.spe`, `myst.dam`, `myst.act`, `myst.pos`, `myst.gui`.
- Modello in memoria (`World`, `Zone`, `Room`, `Mobile`, `GameObject`, …) allineato ai campi del server.
- CLI `nebbiedit` con comandi `info`, `load`, `validate`, `convert lib roundtrip`.
- Test automatici su fixture minime + roundtrip save/load.

**Decisione:** un solo modello `World` come fonte di verità in memoria; i file restano il formato di persistenza principale finché il server non migra a database.

### Fase 2 — Editing MVP (CLI + GUI)

- Comandi one-shot: `room set`, `mob set`, `obj set` con validazione prima del salvataggio.
- GUI Qt: liste stanze/mob/oggetti, ricerca, creazione entità, tab validazione con navigazione agli errori.
- Sessioni: autosalvataggio in `.nebbie/workspace/`, cronologia versioni in `.nebbie/versions/`, backup pre-salvataggio.

**Decisione:** validazione obbligatoria di default; flag `--force` solo in CLI per casi eccezionali.

### Fase 3 — Editor completi (allineamento OLC / Arcane)

- **Cataloghi** (`mob_catalog`, `obj_catalog`, `room_catalog`, `zone_catalog`): etichette in inglese, legende in UI per flag, affect, reset commands.
- **Widget dedicati:** `MobEditorWidget`, `ObjEditorWidget`, `RoomEditorWidget`, `ZoneEditorWidget`.
- Zone: metadati (`name`, `top`, `lifespan`, `reset_mode`, `bottom` derivato) + tabella reset (`M/O/G/E/P/D/C/H…`).
- Stanze: descrizione monospace ampia per ASCII art; settore, uscite, extra desc, illuminazione.

**Decisione:** terminologia e layout allineati all’OLC del server, non inventare alias italiani per i campi tecnici.

### Fase 4 — Mappa e navigazione

- Mappa per zona (`ZoneMapWidget`): grafo stanze per piano Z, zoom/pan, link rotti, doppio clic → stanza.
- Mappa mondo (`WorldZoneMapWidget`): collegamenti inter-zona, vnum usati/liberi.
- Export PNG e DOT (Graphviz); `nebbiedit zone graph --dot`.

### Fase 5 — Coordinator e world index

- **World index** (`world_index.hpp/cpp`): indice zone con range vnum, prenotazioni, bottom/top ricalcolati da `myst.zon`.
- Menu **Coordinator** in GUI: config URL/token, refresh index, export JSON, **Reserve vnums**.
- Validazione prenotazioni: `zone_num`, `start_vnum`, `end_vnum` obbligatori; range stanze deve rientrare in `bottom`–`top` della zona.
- Servizio PHP `nebbie-coordinator` con SQLite (`reservations`).

**Decisione:** il coordinator gestisce solo prenotazioni builder, non la persistenza del mondo; il world index è JSON statico + merge remoto.

### Fase 6 — Overlay (monolite + file per entità)

Il server supporta **monolite + overlay**:

| Directory | Contenuto |
|-----------|-----------|
| `rooms/<vnum>` | Corpo stanza (come `rsave`) |
| `objects/<vnum>` | Corpo oggetto (come `osave`) |
| `mobiles/<vnum>` | Corpo mob (sperimentale lato server) |
| `zones/<n>.zon` | Solo comandi reset + `S` |

**Implementato:**
- `export_myst_to_overlays()` / `apply_overlays()` in `overlay_io.hpp`.
- `load_lib()` applica overlay dopo `myst.*` (ordine boot: zone → rooms → mobiles → objects).
- CLI `nebbiedit overlay export`; GUI **Strumenti → Esporta overlay…**
- Test round-trip overlay.

**Decisione sulle zone:** l’overlay `zones/<n>.zon` contiene **solo i reset**; metadati zona restano in `myst.zon`. Duplicare l’header nell’overlay creerebbe due fonti di verità in conflitto.

**Decisione sui mob:** export/apply implementati; uso in produzione limitato finché il server non allinea `read_mobile()` per file overlay-only (`pos==-1`, come già fa `read_object()`).

### Fase 7 — Import overlay in database

- Script `scripts/import-overlays-to-db.sh` (Python): importa i corpi file overlay in tabelle SQLite con **nome scelto interattivamente** per tipo (`nebbie-room`, `nebbie-mob`, …).
- Ogni riga: `id` (vnum/zone), `body` (testo verbatim per il mud), `source_relpath`, `imported_at`.
- Opzione `--export-first` per generare overlay da `myst.*` prima dell’import.

**Decisione:** fase preparatoria alla roadmap DB (Fase 8); i file restano source of truth; le tabelle SQLite sono archivio/query, non sostituiscono ancora il boot.

### Fase 8 — Distribuzione multi-piattaforma

| Piattaforma | Artefatto | Script |
|-------------|-----------|--------|
| Linux (Debian/Ubuntu) | `.deb` | `package-deb.sh` |
| macOS | `.dmg` (app + CLI) | `package-dmg.sh` |
| Windows | `.zip` portatile + **installer setup** | `package-windows.ps1`, `package-windows-installer.ps1` |

CI GitHub Actions: `build-linux`, `build-macos`, `build-windows`; workflow `packages` pubblica deb/dmg/zip/installer come artifact.

### Fase 9 — Port Windows

Estensione da Linux/macOS-only a tre piattaforme:

- **I/O binario** (`file_io.cpp`): `rb`/`wb`, `_wfopen` per percorsi Unicode.
- **CLI:** `wmain` con conversione argv UTF-16 → UTF-8.
- **Qt:** `path_util.hpp` per `QString` ↔ `std::filesystem::path`; `QCoreApplication::arguments()`.
- **Script PowerShell:** `build.ps1`, `install-deps.ps1`, `fetch-test-data.ps1`, `prepare-windows-package.ps1`.
- **Installer Inno Setup 6:** wizard IT/EN, collegamenti menu Start, opzione desktop e PATH per `nebbiedit`.

**Decisione:** installer `.exe` (Inno Setup) invece di MSI — più semplice da integrare in CI, standard per app desktop Qt su Windows.

---

## 4. Funzionalità operative (cosa può fare l’utente)

### Sviluppatore / builder

1. Clonare il repo, installare dipendenze (`install-deps.sh` / `install-deps.ps1`).
2. Build e test: `build.sh --test` o `build.ps1 -Test`.
3. Aprire una lib: GUI `nebbieedit <path>` o menu File; config persistita in `nebbieedit.conf`.
4. Modificare stanze, mob, oggetti, zone; validare (Ctrl+R); salvare.
5. Esportare overlay per deploy compatibile col server.
6. (Opzionale) Importare overlay in SQLite per archivio o analisi.
7. (Opzionale) Usare coordinator per prenotare range vnum.

### Distribuzione agli utenti finali

- **Linux:** `sudo dpkg -i nebbie-editor_*.deb`
- **macOS:** aprire `.dmg`, trascinare `nebbieedit.app` in Applications
- **Windows:** eseguire `nebbie-editor_*_windows_setup.exe` oppure estrarre lo zip portatile

---

## 5. Decisioni architetturali riassunte

| Tema | Decisione |
|------|-----------|
| Specifica formati | Server NebbieArcane, non documentazione esterna |
| Modello dati | `World` unico in memoria; file `myst.*` master |
| Overlay | Replica regole boot server; zone = solo reset |
| Coordinator | SQLite separato; solo prenotazioni |
| DB mondo | Roadmap; import overlay come primo passo |
| Cross-platform | C++17 + Qt6; niente API POSIX nel core |
| Windows I/O | File binari + wide path; no CRLF in scrittura |
| Validazione | Prima del save; errori navigabili in GUI |
| Lingua UI campi tecnici | Inglese (OLC); messaggi utente misti IT/EN in evoluzione |
| Packaging | Nativo per OS (.deb, .dmg, Inno setup); CI artifact |

---

## 6. Test e qualità

- **11 test CTest** su fixture: load, validate, MVP edit, session, reset, world-index, overlay.
- Test opzionali su `vendor/nebbietest` se presente (`fetch-test-data.sh`).
- CI su ogni push/PR: build + test su Linux, macOS, Windows.
- Smoke test CLI post-build su tutte le piattaforme.

---

## 7. Roadmap residua (non completata)

- Database Nebbie bidirezionale (`load_from_db` / `save_from_db`) — Fase 8 ROADMAP
- Fix server `read_mobile()` per overlay mob
- DMG firmato / notarizzazione macOS
- Editor shop, special proc, social in GUI
- `save_lib` con modalità overlay (salvare solo entità modificate in `rooms/` ecc.)
- Session backup che includa directory overlay

Vedi [ROADMAP.md](ROADMAP.md) per il dettaglio.

---

## 8. Riferimenti rapidi

| Documento | Contenuto |
|-----------|-----------|
| [ARCHITECTURE.md](ARCHITECTURE.md) | Layout lib server, overlay, boot order |
| [PLATFORM.md](PLATFORM.md) | Build e pacchetti per OS |
| [ROADMAP.md](ROADMAP.md) | Stato funzionalità e piani |
| [README.md](../README.md) | Quick start |

| PR / branch principali | Contenuto |
|------------------------|-----------|
| Editor mob/obj/room/zone + overlay | `cursor/mob-editor-full-92f1` |
| Supporto Windows | `cursor/windows-support-92f1` |

---

## 9. Albero operativo sintetico

```
                    ┌─────────────────┐
                    │   myst.* lib    │
                    └────────┬────────┘
                             │ load_lib (+ overlay)
                             ▼
                    ┌─────────────────┐
                    │  World (memory) │
                    └────────┬────────┘
           ┌─────────────────┼─────────────────┐
           ▼                 ▼                 ▼
    ┌────────────┐   ┌────────────┐   ┌────────────┐
    │  nebbiedit │   │ nebbieedit │   │ coordinator│
    │    (CLI)   │   │   (Qt GUI) │   │ (optional) │
    └─────┬──────┘   └─────┬──────┘   └────────────┘
          │                │
          └────────┬───────┘
                   │ validate → save_lib / export overlay
                   ▼
    ┌──────────────────────────────────────┐
    │ myst.*  +  rooms/ objects/ zones/ …  │
    └──────────────────────────────────────┘
                   │ (opzionale)
                   ▼
            SQLite overlay tables
```

---

*Versione documento: allineata al progetto nebbie-editor 0.1.0 — luglio 2026.*
