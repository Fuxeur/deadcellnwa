#!/usr/bin/env bash
# build_nwa.sh : produit DEAD_CELLS_NUMWORKS.nwa (Linux, macOS, Windows via WSL).
# Principe : on greffe le jeu (un seul fichier C) sur l'exemple OFFICIEL NumWorks, qui apporte le bon systeme de compilation.
# Prerequis : git, node + npm, arm-none-eabi-gcc (paquet gcc-arm-none-eabi), make, python3, acces internet.
set -euo pipefail
cd "$(dirname "$0")"
for t in git node npm arm-none-eabi-gcc make python3; do command -v "$t" >/dev/null 2>&1 || { echo "Il manque l'outil : $t (voir INSTALL.md)"; exit 1; }; done
python3 tools/amalgamate.py build/main.c
rm -rf build/sample
git clone --depth 1 https://github.com/numworks/epsilon-sample-app-c.git build/sample
cp build/main.c build/sample/src/main.c
cp src/icon.png build/sample/src/icon.png
# Certaines versions de l'exemple utilisent une ancienne option de nwlink : on la corrige si besoin (sans effet sinon).
sed -i.bak 's/eadk-cflags)/eadk-cflags-device)/' build/sample/Makefile || true
echo "--- lignes nwlink du Makefile officiel ---"; grep -n "nwlink\|NWLINK" build/sample/Makefile | head -10 || true
( cd build/sample && make clean && make build )
NWA="$(find build/sample -name '*.nwa' | head -n 1)"
[ -n "$NWA" ] || { echo "ECHEC : aucun fichier .nwa produit"; exit 1; }
cp "$NWA" DEAD_CELLS_NUMWORKS.nwa
SIZE=$(wc -c < DEAD_CELLS_NUMWORKS.nwa | tr -d ' ')
echo "Fichier : DEAD_CELLS_NUMWORKS.nwa = $SIZE octets ($((SIZE / 1024)) Ko)"
if [ "$SIZE" -gt 6000000 ]; then echo "ERREUR : depasse 6000 Ko"; exit 2; fi
echo "OK : taille inferieure a 6000 Ko. Installe-le avec la page officielle NumWorks (voir INSTALL.md)."
