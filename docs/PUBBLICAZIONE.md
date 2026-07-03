# Pubblicare nebbie-editor su GitHub

Il repository destinazione è: https://github.com/wizardmorgan/nebbie-editor

## Perché compare il 403 con `cursor[bot]` (anche con l'app Cursor in Read/Write)

Gli agent cloud di Cursor usano l'identità **`cursor[bot]`**. Su GitHub l'app Cursor può risultare configurata con **Contents: Read and write**, ma il **token usato a runtime** non sempre eredita quel permesso su **ogni** repository.

Sintomo verificato su questo progetto:

| Repository | Push da agent cloud |
|------------|---------------------|
| `wizardmorgan/DikuEdit` | OK |
| `wizardmorgan/nebbie-editor` | 403 denied to cursor[bot] |

Quindi il problema **non** è “mancano i permessi globali all'app”, ma che **`nebbie-editor` non è ancora incluso nel token effettivo** dell'agent (repo creato dopo l'installazione, cache del token, o bug noto dell'integrazione Cursor).

Messaggio tipico:

```
remote: Permission to wizardmorgan/nebbie-editor.git denied to cursor[bot].
fatal: unable to access '...': The requested URL returned error: 403
```

---

## Metodo 1 — Push dal tuo PC (consigliato)

### Se hai già la cartella del progetto in Cursor desktop

```bash
cd /percorso/a/nebbie-editor
git remote set-url origin https://github.com/wizardmorgan/nebbie-editor.git
git push -u origin cursor/nebbie-editor-initial-c774
```

GitHub ti chiederà login nel browser, oppure usa un [Personal Access Token](https://github.com/settings/tokens) con scope **repo**.

### Se parti da zero (repo GitHub già creato vuoto)

```bash
git clone https://github.com/wizardmorgan/nebbie-editor.git
cd nebbie-editor
```

Poi copia dentro i file del progetto dall'agent cloud, oppure importa il bundle git (se disponibile):

```bash
git pull /percorso/a/nebbie-editor.bundle cursor/nebbie-editor-initial-c774
git push -u origin cursor/nebbie-editor-initial-c774
```

### Impostare `main` come branch predefinito

Dopo il primo push, su GitHub → Settings → Branches puoi impostare `cursor/nebbie-editor-initial-c774` come default, oppure:

```bash
git checkout -b main
git push -u origin main
```

---

## Metodo 2 — Ricollegare `nebbie-editor` all'app Cursor

Se l'app Cursor è già in Read/Write ma il push fallisce solo su `nebbie-editor`, prova **in questo ordine**:

1. https://github.com/settings/installations → **Cursor** → **Configure**
2. **Repository access** → passa a *Only select repositories*, aggiungi esplicitamente **nebbie-editor**, salva  
   (oppure: rimuovi e ri-aggiungi il repo se già selezionato)
3. https://cursor.com/dashboard → disconnetti e riconnetti GitHub
4. Riavvia l'agent cloud e rilancia `./scripts/publish-github.sh`

Se dopo questi passaggi il 403 persiste, è un limite noto del token runtime di Cursor: usa il Metodo 1 (push dal tuo PC).

---

## Metodo 3 — Personal Access Token (HTTPS)

1. Crea un token su https://github.com/settings/tokens → *Generate new token (classic)*
2. Scope: **repo**
3. Push una tantum:

```bash
git push https://wizardmorgan:<IL_TUO_TOKEN>@github.com/wizardmorgan/nebbie-editor.git cursor/nebbie-editor-initial-c774
```

Non committare il token e non condividerlo.

---

## Verifica

Dopo il push, apri:

https://github.com/wizardmorgan/nebbie-editor/tree/cursor/nebbie-editor-initial-c774

Dovresti vedere almeno 2 commit e le cartelle `nebbie-core/`, `nebbiedit/`, `tests/`.

---

## Aprire una Pull Request

Su GitHub: **Compare & pull request** dal branch `cursor/nebbie-editor-initial-c774` verso `main` (o `master`).

Build locale di controllo:

```bash
CXX=g++ cmake -S . -B build && cmake --build build && ctest --test-dir build
```
