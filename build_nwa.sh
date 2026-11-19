
#!/usr/bin/env bash
# Build automatique NumWorks NWA
set -euo pipefail

cd "$(dirname "$0")"

for t in git node npm arm-none-eabi-gcc make python3; do
    command -v "$t" >/dev/null 2>&1 || {
        echo "Il manque l'outil : $t"
        exit 1
    }
done

echo "=== Génération du code C ==="
python3 tools/amalgamate.py build/main.c

echo "=== Récupération du template NumWorks ==="
rm -rf build/sample
git clone --depth 1 \
    https://github.com/numworks/epsilon-sample-app-c.git \
    build/sample

echo "=== Installation du jeu ==="
cp build/main.c build/sample/src/main.c
cp src/icon.png build/sample/src/icon.png

echo "=== Vérification du Makefile ==="
grep -n "nwlink\|NWLINK" build/sample/Makefile

echo "=== Compilation NWA ==="
(
    cd build/sample
    make clean
    make build
)

echo "=== Recherche du fichier NWA ==="
NWA="$(find build/sample -name '*.nwa' -type f | head -n 1)"

if [ -z "$NWA" ]; then
    echo "ERREUR : aucun fichier NWA produit"
    exit 1
fi

cp "$NWA" DEAD_CELLS_NUMWORKS.nwa

SIZE=$(wc -c < DEAD_CELLS_NUMWORKS.nwa | tr -d ' ')

echo "=================================="
echo "Fichier : DEAD_CELLS_NUMWORKS.nwa"
echo "Taille : $SIZE octets"
echo "=================================="

if [ "$SIZE" -gt 6000000 ]; then
    echo "ERREUR : dépassement de 6000 Ko"
    exit 2
fi

echo "OK : compilation terminée !"
