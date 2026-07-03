# Pubblicare nebbie-editor su GitHub

Il repository destinazione è: https://github.com/wizardmorgan/nebbie-editor

## Perché compare il 403 con `cursor[bot]`

Gli agent cloud di Cursor usano l'identità **`cursor[bot]`**. Su molti account personali GitHub questa identità può **leggere** i repository ma **non scrivere**, anche se il repo esiste ed è tuo.

Messaggio tipico:

```
remote: Permission to wizardmorgan/nebbie-editor.git denied to cursor[bot].
fatal: unable to access '...': The requested URL returned error: 403
```

Questo **non** è un errore del codice: serve un push con **le tue** credenziali GitHub, oppure dare all'app Cursor il permesso di scrittura sul repo.

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

## Metodo 2 — Permessi all'app Cursor su GitHub

1. Vai su https://github.com/settings/installations
2. Clicca **Configure** accanto a **Cursor**
3. **Repository access** → seleziona **nebbie-editor** (o *All repositories*)
4. Verifica che **Contents** sia **Read and write**
5. Salva e rilancia `./scripts/publish-github.sh`

Se dopo questa configurazione il push funziona dall'agent cloud, non serve altro.

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
