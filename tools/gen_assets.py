#!/usr/bin/env python3
"""gen_assets.py <dossier_src> <dossier_assets> : genere dc_assets.h/.c, dc_rooms.c, apercus PNG et icon.png (55x56)."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from art_core import *
from art_chars import build_all_chars
from art_chars2 import build_new_chars
import art_world as W
import rooms as RM
from PIL import Image, ImageDraw

SRC = sys.argv[1] if len(sys.argv) > 1 else 'src'
PREV = sys.argv[2] if len(sys.argv) > 2 else 'assets'

def shaded(w, h, fn, *a, shade=True, outline=True):
    cv = Cv(w, h); fn(cv, *a); return finish(cv, shade, outline)

REG = []
def reg(name, frames):
    h, w = len(frames[0]), len(frames[0][0])
    for f in frames:
        assert len(f) == h and len(f[0]) == w, name
    REG.append((name, frames))

for n, fr in build_all_chars().items():
    reg(n, fr)
for n, fr in build_new_chars().items():
    reg(n, fr)
reg('FX_SLASH', [W.fx_slash(i) for i in range(3)])
reg('FX_SMASH', [W.fx_smash(i) for i in range(3)])
reg('FX_HIT',   [W.fx_hit(i) for i in range(3)])
reg('FX_DUST',  [W.fx_dust(i) for i in range(3)])
reg('FX_FIRE',  [W.fx_fire(i) for i in range(4)])
reg('FX_EXPL',  [W.fx_expl(i) for i in range(4)])
reg('FX_ARROW', [shaded(16, 5, W.fx_arrow, shade=False, outline=False)])
reg('FX_BOLT',  [shaded(12, 5, W.fx_bolt, shade=False, outline=False)])
reg('FX_SPIT',  [shaded(8, 8, W.fx_spit, f, shade=False, outline=False) for f in range(2)])
reg('FX_WAVE',  [shaded(16, 16, W.fx_wave, f, shade=False, outline=False) for f in range(2)])
reg('IT_COIN',  [shaded(8, 8, W.it_coin, f) for f in range(4)])
reg('IT_HEART', [shaded(10, 10, W.it_heart)])
reg('IT_FLASK', [shaded(10, 12, W.it_flask)])
reg('IT_CHEST', [shaded(16, 14, W.it_chest, False), shaded(16, 14, W.it_chest, True)])
reg('IT_WPN',   [shaded(16, 16, W.icon_wpn10, k) for k in range(10)])
reg('FX_BOMB',  [shaded(8, 8, W.fx_bomb, f, shade=False, outline=False) for f in range(2)])
reg('FX_CLOUD', [shaded(24, 16, W.fx_cloud, f, shade=False, outline=False) for f in range(2)])
reg('IT_CELL',  [shaded(8, 8, W.it_cell, f, shade=False, outline=False) for f in range(2)])
reg('IT_BLUEPRINT', [shaded(10, 12, W.it_blueprint)])
reg('TL_SOLID', [W.tile_solid(z, v) for z in range(5) for v in range(3)])
reg('TL_CAP',   [W.tile_cap(z) for z in range(5)])
reg('TL_PLAT',  [W.tile_plat(k) for k in ('wood', 'wood', 'wood', 'steel', 'gold')])
reg('TL_SPIKE', [shaded(16, 16, W.tile_spike)])
reg('TL_TOXIC', [shaded(16, 16, W.tile_toxic, f, shade=False, outline=False) for f in range(2)])
reg('TL_DOOR',  [shaded(16, 32, W.tile_door)])
reg('TL_GATE',  [shaded(16, 16, W.tile_gate)])
reg('TL_TORCH', [shaded(8, 16, W.tile_torch, f, shade=False, outline=False) for f in range(3)])
reg('BGT',      [W.bg_wall(z) for z in range(5)])
reg('BGD',      [W.bg_deco(z) for z in range(5)])

# ---------------------------------------------------------------- export C
blob, table, enum_lines = [], [], []
idx = 0
for name, frames in REG:
    enum_lines.append(f"  SB_{name} = {idx}, SN_{name} = {len(frames)},")
    for f in frames:
        h, w = len(f), len(f[0])
        table.append((w, h, len(blob)))
        for row in f:
            blob.extend(row)
        idx += 1
SPR_COUNT = idx
assert max(blob) < 255 and len(PAL) < 255

def carr(vals, per=24):
    return ",\n".join("  " + ",".join(str(v) for v in vals[i:i + per]) for i in range(0, len(vals), per))

H = f"""/* GENERE par tools/gen_assets.py - ne pas modifier a la main. Art 100 % original. */
#ifndef DC_ASSETS_H
#define DC_ASSETS_H
#include <stdint.h>
#define NPAL {len(PAL)}
#define SPR_COUNT {SPR_COUNT}
#define ACC_LO {ACCS[0][0]}
typedef struct {{ uint8_t w, h; uint32_t off; }} Spr;   /* decalage dans SPRDATA : pas de pointeur -> pas de relocation */
extern const uint16_t PAL565[NPAL];
extern const Spr SPR[SPR_COUNT];
extern const uint8_t SPRDATA[{len(blob)}];
extern const uint8_t SKINMAP[5][3];
extern const uint8_t FONT9[144][9];
extern const uint16_t ZCOL[5][4];
enum {{
{chr(10).join(enum_lines)}
}};
#endif
"""
font = W.font_table()
C = '/* GENERE par tools/gen_assets.py */\n#include "dc_assets.h"\n'
C += "const uint16_t PAL565[NPAL] = {\n" + carr([to565(c) for c in PAL], 12) + "\n};\n"
C += "const uint8_t SKINMAP[5][3] = {" + ",".join("{%d,%d,%d}" % tuple(a) for a in ACCS) + "};\n"
C += "const uint16_t ZCOL[5][4] = {" + ",".join("{" + ",".join(str(to565(PAL[i])) for i in z) + "}" for z in ZR) + "};\n"
C += "const uint8_t FONT9[144][9] = {\n" + ",\n".join("  {" + ",".join(str(b) for b in g) + "}" for g in font) + "\n};\n"
C += "const Spr SPR[SPR_COUNT] = {\n" + ",\n".join("  {%d,%d,%d}" % t for t in table) + "\n};\n"
C += f"const uint8_t SPRDATA[{len(blob)}] = {{\n" + carr(blob) + "\n};\n"
open(os.path.join(SRC, 'dc_assets.h'), 'w').write(H)
open(os.path.join(SRC, 'dc_assets.c'), 'w').write(C)

rs = RM.build(); RM.validate(rs)
RC = '/* GENERE par tools/gen_assets.py : salles (voir tools/rooms.py) */\n#define DC_NROOMS %d\n' % len(rs)
RC += 'const char DC_ROOMS[DC_NROOMS][15][21] = {\n'
for r in rs:
    RC += '  { /* %s */\n' % r.name + ",\n".join('    "%s"' % row for row in r.rows()) + '\n  },\n'
RC += '};\n'
open(os.path.join(SRC, 'dc_rooms.c'), 'w').write(RC)

# ---------------------------------------------------------------- apercus
def to_img(frame, scale=1, bg=None):
    h, w = len(frame), len(frame[0])
    im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    px = im.load()
    for y in range(h):
        for x in range(w):
            c = frame[y][x]
            if c: px[x, y] = PAL[c] + (255,)
    return im.resize((w * scale, h * scale), Image.NEAREST) if scale != 1 else im

def sheet(names, path, scale=3, cols_px=1000):
    items = [(n, fr) for n, fr in REG if n in names]
    x = y = 6; rowh = 0
    pos = []
    for n, frames in items:
        w = len(frames[0][0]) * scale; h = len(frames[0]) * scale
        total = (w + 4) * len(frames)
        if x + total > cols_px and x > 6:
            x = 6; y += rowh + 18; rowh = 0
        pos.append((n, frames, x, y)); x += total + 14; rowh = max(rowh, h)
    H_ = y + rowh + 22
    im = Image.new('RGBA', (cols_px, H_), (34, 30, 48, 255)); d = ImageDraw.Draw(im)
    for n, frames, x, y in pos:
        d.text((x, y), n, fill=(200, 200, 215, 255))
        for i, f in enumerate(frames):
            im.alpha_composite(to_img(f, scale), (x + i * (len(f[0]) * scale + 4), y + 12))
    im.convert('RGB').save(path)

chars_n = [n for n, _ in REG if n.split('_')[0] in ('HERO', 'JAIL', 'XBOW', 'RAT', 'BRUTE', 'SLIME', 'GOB', 'KING', 'GREN', 'SHLD', 'TOXIC', 'BAT', 'NPC')]
world_n = [n for n, _ in REG if n not in chars_n and n not in ('TL_SOLID', 'BGT', 'BGD')]
os.makedirs(PREV, exist_ok=True)
sheet(chars_n, os.path.join(PREV, 'apercu_personnages.png'), 3)
sheet(world_n, os.path.join(PREV, 'apercu_monde.png'), 3)
sheet(['TL_SOLID'], os.path.join(PREV, 'apercu_tuiles.png'), 3, 800)
sheet(['BGT', 'BGD'], os.path.join(PREV, 'apercu_decors.png'), 2, 800)

# salles
cell = 12
col = {'#': (90, 100, 130), '=': (190, 140, 70), '^': (220, 220, 230), 'e': (220, 60, 60), 'E': (255, 0, 0), 'c': (240, 200, 40),
       'f': (230, 60, 140), 'h': (255, 90, 90), 'o': (255, 220, 80), 'T': (255, 150, 40), 'S': (60, 220, 90), 'D': (120, 200, 255),
       'd': (60, 110, 160), 'B': (200, 40, 200), '.': (22, 20, 32)}
cols_n = 3; rw = 20 * cell + 10; rh = 15 * cell + 22
im = Image.new('RGB', (cols_n * rw, ((len(rs) + cols_n - 1) // cols_n) * rh), (10, 10, 16)); d = ImageDraw.Draw(im)
for i, r in enumerate(rs):
    ox, oy = (i % cols_n) * rw + 4, (i // cols_n) * rh + 14
    d.text((ox, oy - 12), r.name, fill=(220, 220, 230))
    for y, row in enumerate(r.rows()):
        for x, ch in enumerate(row):
            d.rectangle([ox + x * cell, oy + y * cell, ox + x * cell + cell - 2, oy + y * cell + cell - 2], fill=col.get(ch, (255, 0, 255)))
im.save(os.path.join(PREV, 'apercu_salles.png'))

# icone 55x56 (format NumWorks) : decor original
def make_icon(path):
    S = 4
    im = Image.new('RGBA', (55, 56), (0, 0, 0, 0)); px = im.load()
    for y in range(56):
        for x in range(55):
            # coins arrondis
            cx = min(x, 54 - x); cy = min(y, 55 - y)
            if cx < 6 and cy < 6 and (6 - cx) ** 2 + (6 - cy) ** 2 > 36: continue
            t = y / 55.0
            px[x, y] = (int(22 + 40 * t), int(14 + 14 * t), int(44 + 30 * t), 255)
    d = ImageDraw.Draw(im)
    d.ellipse([34, 6, 48, 20], fill=(240, 226, 190, 255)); d.ellipse([38, 4, 52, 18], fill=(34, 24, 62, 255))
    for x0, w_, h_ in ((2, 8, 22), (11, 7, 30), (19, 9, 18), (29, 7, 26), (37, 8, 20), (46, 7, 28)):
        d.rectangle([x0, 55 - h_, x0 + w_, 55], fill=(18, 12, 30, 255))
        for k in range(x0, x0 + w_ + 1, 3): d.rectangle([k, 55 - h_ - 2, k + 1, 55 - h_], fill=(18, 12, 30, 255))
    hero = [fr for n, fr in REG if n == 'HERO_IDLE'][0][0]
    him = to_img(hero, 2)
    im.alpha_composite(him, (4, 8))
    d.rectangle([0, 53, 54, 55], fill=(90, 50, 40, 255))
    im.save(path)
make_icon(os.path.join(SRC, 'icon.png'))
make_icon(os.path.join(PREV, 'icon.png'))

tot = len(blob)
print(f"sprites : {SPR_COUNT} images, SPRDATA = {tot} octets ({tot/1024:.1f} Ko), palette = {len(PAL)} couleurs, salles = {len(rs)}")
