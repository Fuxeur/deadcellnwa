#!/usr/bin/env python3
"""amalgamate.py <sortie.c> : assemble tout le jeu en UN SEUL fichier C (build/main.c).
Ce fichier remplace src/main.c de l'exemple officiel NumWorks (epsilon-sample-app-c) : aucune modification du Makefile n'est necessaire."""
import sys, os, re
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'build', 'main.c')
order = ['dc_assets.h', 'dc.h', 'dc_assets.c', 'dc_rooms.c', 'dc_core.c', 'dc_ai.c', 'dc_render.c', 'main_eadk.c']
parts = ['/* DEAD CELLS - NUMWORKS EDITION : fichier unique genere par tools/amalgamate.py - ne pas modifier a la main. */\n']
for name in order:
    txt = open(os.path.join(ROOT, 'src', name)).read()
    txt = re.sub(r'^\s*#include\s+"dc(_assets)?\.h"\s*$', '', txt, flags=re.M)
    parts.append('\n/* ======== %s ======== */\n' % name + txt)
os.makedirs(os.path.dirname(out), exist_ok=True)
open(out, 'w').write(''.join(parts))
print('ecrit', out, os.path.getsize(out), 'octets')
