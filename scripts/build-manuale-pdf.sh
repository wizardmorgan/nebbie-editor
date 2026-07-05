#!/usr/bin/env bash
# Build docs/MANUALE_INSTALLAZIONE.pdf from the Markdown source.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DOCS="${ROOT}/docs"
SRC="${DOCS}/MANUALE_INSTALLAZIONE.md"
OUT="${DOCS}/MANUALE_INSTALLAZIONE.pdf"

if [[ ! -f "${SRC}" ]]; then
    echo "ERROR: ${SRC} not found." >&2
    exit 1
fi

if ! command -v pandoc >/dev/null 2>&1; then
    echo "ERROR: pandoc not found. Install: sudo apt-get install pandoc texlive-xetex texlive-lang-european" >&2
    exit 1
fi

PDF_ENGINE="xelatex"
if ! command -v xelatex >/dev/null 2>&1; then
    if command -v pdflatex >/dev/null 2>&1; then
        PDF_ENGINE="pdflatex"
    else
        echo "ERROR: no LaTeX engine (xelatex/pdflatex). Install texlive-xetex." >&2
        exit 1
    fi
fi

echo "==> pandoc ${SRC} -> ${OUT} (engine: ${PDF_ENGINE})"
(
    cd "${DOCS}"
    pandoc "MANUALE_INSTALLAZIONE.md" \
        -o "MANUALE_INSTALLAZIONE.pdf" \
        --pdf-engine="${PDF_ENGINE}" \
        --toc \
        --toc-depth=2 \
        -V documentclass=article \
        -V geometry:margin=2.5cm \
        -V fontsize=11pt \
        -V lang=it \
        -V colorlinks=true \
        -V linkcolor=blue \
        -V urlcolor=blue \
        -V mainfont="DejaVu Serif" \
        -V sansfont="DejaVu Sans" \
        -V monofont="DejaVu Sans Mono"
)

echo ""
echo "PDF created:"
ls -lh "${OUT}"
