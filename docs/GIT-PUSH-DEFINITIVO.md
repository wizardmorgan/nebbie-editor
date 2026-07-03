# Push definitivo per nebbie-editor (cloud agent + PC)

## Il problema

L'agent cloud pusha come **`cursor[bot]`** tramite l'app GitHub di Cursor. Su alcuni repo il token funziona (es. `DikuEdit`), su altri no (es. `nebbie-editor`):

```
remote: Permission to wizardmorgan/nebbie-editor.git denied to cursor[bot].
```

Anche con l'app Cursor in **Read/Write**, i repo creati dopo l'installazione a volte non entrano nel token runtime dell'agent.

---

## Soluzione A — PAT nei Secrets Cursor (consigliata, definitiva)

Funziona sempre: l'agent pusha con **il tuo account** `wizardmorgan`.

### 1. Crea un Fine-grained PAT su GitHub

1. https://github.com/settings/tokens?type=beta → **Generate new token**
2. Nome: `cursor-nebbie-editor`
3. Repository access: **Only select** → `nebbie-editor` (oppure tutti i tuoi repo)
4. Permissions → **Contents**: Read and write
5. Genera e copia il token (`github_pat_...`)

### 2. Aggiungi il secret in Cursor

1. https://cursor.com/dashboard → **Cloud Agents** → **Secrets**
2. Aggiungi:
   - **Name:** `WIZARDMORGAN_GITHUB_PAT`
   - **Value:** il token copiato
3. **Non** chiamarlo `GH_TOKEN` (conflitto con il token interno Cursor)

### 3. Riavvia l'agent

Avvia una **nuova** sessione cloud su `nebbie-editor` (i secret si caricano al boot).

### 4. Push

```bash
./scripts/git-push.sh
```

Lo script `.cursor/setup-git-auth.sh` viene eseguito automaticamente all'avvio dell'ambiente (`environment.json`).

---

## Soluzione B — Riparare l'app GitHub Cursor (senza PAT)

Prova se vuoi che continui a usare solo `cursor[bot]`:

1. https://github.com/settings/installations → **Cursor** → **Configure**
2. **Repository access** → passa a *Only select repositories*
3. Seleziona **esplicitamente** `nebbie-editor` (anche se prima era "All repositories")
4. Verifica **Contents: Read and write** → Save
5. https://cursor.com/dashboard → disconnetti e riconnetti GitHub
6. Nuova sessione agent → `git push`

Se dopo questi passaggi `DikuEdit` pusha ma `nebbie-editor` no, usa la **Soluzione A**.

---

## Soluzione C — Solo dal PC (senza cloud push)

Se non vuoi configurare secret:

```bash
git clone https://github.com/wizardmorgan/nebbie-editor.git
cd nebbie-editor
# copia modifiche dall'archivio agent se necessario
git push origin cursor/nebbie-editor-initial-c774
```

Il login GitHub nel browser o un PAT locale funzionano sempre.

---

## Verifica

Dopo un push riuscito:

```bash
git ls-remote origin cursor/nebbie-editor-initial-c774
```

Oppure controlla: https://github.com/wizardmorgan/nebbie-editor/commits

La CI deve diventare verde: https://github.com/wizardmorgan/nebbie-editor/actions

---

## Riepilogo

| Metodo | Chi pusha | Quando usarlo |
|--------|-----------|---------------|
| App Cursor (`cursor[bot]`) | Bot | Solo se funziona sul repo |
| `WIZARDMORGAN_GITHUB_PAT` | Tu (`wizardmorgan`) | **Consigliato per nebbie-editor** |
| Push dal PC | Tu | Macchina senza agent / backup |
