# Trasferire modifiche non pushate a un altro agent

Quando l'agent non riesce a pushare su GitHub, le modifiche restano **solo nel workspace locale** (cartella `.git` inclusa). Un nuovo agent che clona da GitHub **non le vede** finché non le importi.

## Cosa c'è da trasferire (esempio attuale)

4 commit su `cursor/nebbie-editor-initial-c774` non ancora su GitHub:

- `myst.shp` / `myst.spe` (shop + special proc)
- setup push PAT + script diagnostici

---

## Metodo 1 — Archivio zip (consigliato)

**Cursor non ha un tasto "Download" per scaricare l'intero workspace.** Gli "Artifacts" nella pagina dell'agent sono soprattutto screenshot/video di demo, non un export del codice.

### Come ottenere lo zip

1. Chiedi all'agent: **"preparami lo zip"** → esegue `./scripts/create-export-zip.sh`
2. Scarica il file con una di queste vie:
   - **Desktop remoto** (consigliato): su [cursor.com/agents](https://cursor.com/agents) apri questa sessione → **Remote desktop** / controllo desktop → nel VM il file è in `/workspace/nebbie-editor-export.zip`
   - **API Cursor** (se hai API key): endpoint artifacts dell'agent

### Sul PC, dopo lo zip

```bash
unzip nebbie-editor-export.zip
cd nebbie-editor
git log --oneline -6
git remote set-url origin https://github.com/wizardmorgan/nebbie-editor.git
git push -u origin cursor/nebbie-editor-initial-c774
```

Il nuovo agent con `WIZARDMORGAN_GITHUB_PAT` può poi clonare da GitHub e avrà tutto.

---

## Metodo 2 — Solo bundle git (file piccolo, ~12 KB)

Utile se vuoi importare **solo i commit mancanti** nel nuovo agent, senza copiare tutto il progetto.

### Esporta (agent vecchio)

```bash
./scripts/export-unpushed.sh
```

Il file è: `dist/nebbie-editor-unpushed.bundle`

Scaricalo dall'archivio del workspace (o copia il file dal zip dell'agent).

### Importa (nuovo agent)

Il nuovo agent deve avere già il repo clonato **allo stesso punto di GitHub** (commit `d8eac5a` o successivo uguale a `origin`):

```bash
cd /workspace/nebbie-editor   # o dove è clonato
git fetch origin
git checkout cursor/nebbie-editor-initial-c774
git pull /percorso/al/nebbie-editor-unpushed.bundle cursor/nebbie-editor-initial-c774
```

Verifica:

```bash
git log --oneline -5
./scripts/git-push.sh
```

---

## Metodo 3 — Nuovo agent + secret PAT (senza PC)

1. Aggiungi `WIZARDMORGAN_GITHUB_PAT` in [Cursor Dashboard → Secrets](https://cursor.com/dashboard).
2. Avvia **nuovo** cloud agent su `nebbie-editor`.
3. Scarica l'archivio dall'agent **vecchio** (Metodo 1) o il bundle (Metodo 2).
4. Nel nuovo agent, chiedi all'AI di importare il bundle o sostituire i file dall'archivio, poi `./scripts/git-push.sh`.

---

## Checklist rapida

| Passo | Fatto? |
|-------|--------|
| Secret `WIZARDMORGAN_GITHUB_PAT` nel nuovo agent | |
| Modifiche esportate (archivio o bundle) | |
| `git pull ...bundle` o push dal PC | |
| `git push` riuscito | |
| CI verde su GitHub Actions | |

---

## Verifica su GitHub

Dopo il push:

https://github.com/wizardmorgan/nebbie-editor/commits/cursor/nebbie-editor-initial-c774

Devono comparire file come `nebbie-core/src/shp_io.cpp`, `nebbie-core/src/spe_io.cpp`, `.cursor/setup-git-auth.sh`.
