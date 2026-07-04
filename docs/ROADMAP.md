# Roadmap Nebbie Editor

Stato aggiornato dopo il caricamento completo della lib nebbietest (zone, stanze, mob, oggetti).

## Completato

- [x] Parser `myst.zon` … `myst.gui` (lettura/scrittura)
- [x] Validazione referenze (`nebbiedit validate`)
- [x] MVP editing CLI (`edit`, `room/mob/obj set`, `save_lib`)
- [x] GUI Qt: browse/edit stanze, mob, oggetti, uscite, reset zona, cerca, crea, valida, salva
- [x] Supporto Linux + macOS (CI, app bundle, icona)
- [x] Caricamento lib nebbietest reale (mob avanzati, oggetti estesi, stanze incl. `#0`)

## Prossimo passo immediato

**Consolidamento e qualità** prima della mappa zone:

1. **Validazione in GUI** — pannello errori/warning dopo `Valida`, con link al vnum coinvolto
2. **Test e fixture** — allineare `validate-fixtures` (shop/guild) e test su lib vendor
3. **Salvataggio sicuro** — backup automatico `.bak` prima di `save_lib`, conferma se validazione fallisce

Questo rende affidabile l’editing quotidiano mentre si progetta la mappa.

## Fase 6 — Mappa zone e overview collegamenti

### Obiettivo

Tab o finestra dedicata con **overview di una zona** (o dell’intero mondo filtrato per zona): stanze come nodi, uscite come archi, navigazione rapida verso l’editor stanza.

### Modello dati (nebbie-core)

- `rooms_in_zone(world, zone_num)` → elenco stanze nel range `bottom`–`top`
- `zone_graph(world, zone_num)` → nodi (vnum, nome, settore) + archi (dir, to_room, one-way?)
- Rilevamento **link rotti** (to_room assente o fuori zona)
- Export debug: `nebbiedit zone graph <zone> --dot` per prototipare layout

### Rappresentazione visiva

| Approccio | Pro | Contro | Uso consigliato |
|-----------|-----|--------|-----------------|
| **Grafo 2D per piano** | Semplice, veloce, standard nei builder MUD | Su/giù non sono “spaziali” | **MVP** |
| **Piani multipli (layer Z)** | Su/giù come cambio piano; stesso X/Y per stanza | UI leggermente più complessa | **Fase 6b** |
| **Vista 3D / isometrica** | Intuitiva per torri e dungeon verticali | Molto lavoro (Qt3D, layout 3D, performance) | Opzionale, dopo MVP |

**Raccomandazione:** non partire con il 3D. La maggior parte delle zone nebbie è planare (N/E/S/O); su/giù sono pochi collegamenti e si rappresentano bene così:

- **Piano attivo** = livello Z inferito (BFS da stanza seed, su = Z+1, giù = Z−1)
- Archi orizzontali: frecce colorate per direzione (N verde, E blu, …)
- Archi verticali: linea tratteggiata + icona ↑/↓ (clic apre il piano collegato)
- Stanza selezionata: highlight + pannello uscite come nel tab Stanze

Layout automatico: force-directed o layered DAG sul sotto-grafo orizzontale; posizioni salvate opzionalmente in metadati locali (non nel formato server).

### GUI (nebbie-qt)

1. Tab **Mappa** o finestra **Overview zona…**
2. Combo selezione zona (175 zone su nebbietest)
3. `QGraphicsView` + scene con nodi/edge (no dipendenze esterne in MVP)
4. Doppio clic nodo → tab Stanze con vnum selezionato
5. Filtri: solo link rotti, solo teleport, evidenza reset mob/obj

### CLI di supporto

```bash
nebbiedit zone graph 42 --dot > zone42.dot
nebbiedit zone rooms 42
```

## Fase 7 — Editing avanzato

- Editor shop / special proc / social in GUI
- Diff e roundtrip test su lib reale
- Integrazione validazione con boot server nebbietest (CI opzionale)

## Fase 8 — Mappa mondo (opzionale)

- Vista tutte le zone come macro-nodi (collegamenti inter-zona via teleport / uscite verso altre zone)
- Esportazione immagine / Graphviz per documentazione

## Direzioni uscita (riferimento)

| D# | Nome | Layout 2D |
|----|------|-----------|
| 0 | nord | +Y |
| 1 | est | +X |
| 2 | sud | −Y |
| 3 | ovest | −X |
| 4 | su | layer Z+1 (non disegnato sullo stesso piano) |
| 5 | giù | layer Z−1 |

Il server non definisce coordinate assolute: il layout è **derivato** dal grafo delle uscite, non letto da file.
