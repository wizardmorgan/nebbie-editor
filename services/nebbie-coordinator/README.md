# Nebbie Coordinator (prototipo)

Servizio PHP minimale ispirato a **MudEditorBE** `ZoneListController` + `ZoneList` model, ma semplificato:

- niente Git
- niente parsing `aree.index`
- solo `world-index.json` + tabella prenotazioni SQLite

## Installazione

1. Copia la cartella sul server web
2. `cp config.example.php config.php` e imposta `api_token`
3. Assicurati che `data/` sia scrivibile da PHP
4. Punta il web server a `public/`
5. Carica `world-index.json` nella root del servizio (accanto a `data/`)

## Endpoint

| Metodo | URL | Auth |
|--------|-----|------|
| GET | `/world-index.json` | No |
| GET | `/api/v1/reservations` | Bearer token |
| POST | `/api/v1/reservations` | Bearer token |

## Test locale con PHP built-in

```bash
cd services/nebbie-coordinator
cp config.example.php config.php
php -S 127.0.0.1:8080 -t public
curl http://127.0.0.1:8080/../world-index.json
```

## MudEditor legacy — cosa riusare

| Componente | Riutilizzabile? | Note |
|------------|-----------------|------|
| `ZoneList` model (start/end/path/name/status) | **Concetto sì** | → tabella `reservations` + campi vnum |
| `ZoneListController` POST JSON | **Pattern sì** | Stesso flusso risposta `{status, err_code, reason}` |
| SQLite in `dbconf.php` | **Sì** | Stesso approccio |
| `DataParser` + git path | **No** | Leggeva `aree.index` da repo Git aree |
| `Api.php` read/write zone files | **Parziale** | Fase 3 upload; parser diverso da nebbie-core |
| MudEditorFE Angular 4 | **No codice** | UI obsoleta; riusare idea form prenotazione zona |
| Auth `?mammoletta=gemma` | **No** | Sostituito da Bearer token |
