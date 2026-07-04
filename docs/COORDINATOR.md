# Nebbie Coordinator — requisiti server

Servizio leggero per **indice mondo** (sola lettura) e **prenotazioni vnum** (API). L'editor Qt e la CLI lo contattano via HTTPS; i builder non usano Git né SSH.

## Cosa ospitare sul tuo server online

| Percorso web | Tipo | Uso |
|--------------|------|-----|
| `/nebbie/world-index.json` | File statico | Indice mondo scaricato dall'editor |
| `/nebbie/api/v1/reservations` | API PHP | GET lista, POST nuova prenotazione |
| `/nebbie/api/v1/reservations/{id}` | API PHP | DELETE annulla (fase 3) |

Directory suggerita sul server:

```
/var/www/nebbie/
  world-index.json      ← aggiornato da script export/upload
  api/
    index.php           ← router API (da services/nebbie-coordinator)
  data/
    coordinator.sqlite3 ← prenotazioni (scrivibile da PHP)
```

## Cosa mi serve da te per il prototipo

1. **URL base HTTPS** — es. `https://tuodominio.it/nebbie`
2. **Spazio web** con PHP 8.x + SQLite (o MySQL se preferisci, adattiamo)
3. **Token API** condiviso con i builder — es. `coordinator_token=...` in `nebbieedit.conf`
4. **Permesso scrittura** per la cartella `data/` (solo il PHP ci scrive)
5. **(Opzionale)** Credenziali FTP/SFTP o pannello per caricare `world-index.json` finché l'upload automatico da produzione non c'è

**Non serve** dare ai builder accesso SSH al server di produzione.

## Configurazione editor (`nebbieedit.conf`)

```ini
lib_path=/percorso/alla/tua/lib/locale
index_url=https://tuodominio.it/nebbie/world-index.json
coordinator_url=https://tuodominio.it/nebbie/api/v1
coordinator_token=un-segreto-lungo
builder_name=Morgan
```

## Prototipo locale (questo laptop)

Fingiamo che la produzione sia la cartella `tests/fixtures`:

```bash
# 1. Genera l'indice dal "mondo produzione" locale
./build/nebbiedit/nebbiedit world-index export tests/fixtures /tmp/world-index.json

# 2. Caricalo sul tuo server (esempio)
curl -T /tmp/world-index.json https://tuodominio.it/nebbie/world-index.json

# 3. Prova una prenotazione
curl -X POST https://tuodominio.it/nebbie/api/v1/reservations \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer IL_TUO_TOKEN" \
  -d '{"builder":"Morgan","zone_num":1,"kind":"room","start_vnum":3010,"end_vnum":3019,"note":"prova"}'
```

## Deploy coordinator PHP

```bash
cd services/nebbie-coordinator
cp config.example.php config.php   # modifica token e path DB
# Copia public/ e src/ sul server sotto /nebbie/api/
```

Vedi `services/nebbie-coordinator/README.md` per dettagli.

## Fasi

| Fase | Cosa | Stato |
|------|------|--------|
| 1 | Export `world-index.json` + download editor | Prototipo in branch |
| 2 | API prenotazioni + merge nell'editor | Prototipo PHP + client |
| 3 | Upload lavori + review web | Roadmap (riuso idea MudEditorFE form) |

## Produzione reale (dopo il prototipo)

Un cron sul server mud (o macchina admin con copia read-only della lib) eseguirà:

```bash
nebbiedit world-index export /path/mudroot/lib /tmp/world-index.json
curl -T /tmp/world-index.json https://tuodominio.it/nebbie/world-index.json
```

I builder scaricano l'indice; non toccano la lib di produzione.
