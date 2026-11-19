"""art_world.py - effets, objets, tuiles, decors et police 5x7 (art original genere par code)."""
import math, random
from art_core import *

def mk(w, h, fn, **kw):
    cv = Cv(w, h); fn(cv, **kw); return finish(cv, kw.get('shade', True), kw.get('outline', True))

def raw(w, h, fn, *a):
    cv = Cv(w, h); fn(cv, *a); return finish(cv, False, False)

# ------------------------------------------------------------------ effets
def arc(cv, cx, cy, r0, wmax, a0, a1, cols):
    mid = (a0 + a1) / 2.0; half = (a1 - a0) / 2.0
    for y in range(cv.h):
        for x in range(cv.w):
            dx, dy = x - cx, y - cy
            r = math.hypot(dx, dy)
            a = math.degrees(math.atan2(dy, dx))
            if a0 <= a <= a1:
                u = abs(a - mid) / half
                w = wmax * (1 - u * u)
                if r0 <= r <= r0 + w:
                    t = (r - r0) / max(w, 1e-6)
                    cv.px(x, y, cols[min(len(cols) - 1, int(t * len(cols)))])

def fx_slash(f):
    cv = Cv(32, 32)
    rng = [(-85, -5, 5), (-62, 62, 6), (8, 85, 4)][f]
    arc(cv, 5, 16, 12, rng[2], rng[0], rng[1], [STEEL[1], STEEL[2], BONE[2], WHITE])
    return finish(cv, False, False)

def fx_smash(f):
    cv = Cv(48, 24)
    rx, ry = (9, 17, 23)[f], (4, 6, 8)[f]
    cv.nl = True
    for t in range(0, 360, 4):
        a = math.radians(t)
        x = 24 + rx * math.cos(a); y = 21 - ry * abs(math.sin(a))
        cv.px(x, y, (BONE[2], BONE[1], BONE[0])[f]); cv.px(x, y + 1, (WHITE, BONE[2], BONE[1])[f])
    return finish(cv, False, False)

def fx_hit(f):
    cv = Cv(16, 16); cv.nl = True
    L = (3, 6, 7)[f]
    for i in range(8):
        a = math.radians(i * 45 + 22)
        l = L if i % 2 == 0 else L * 0.6
        cv.line(8, 8, 8 + l * math.cos(a), 8 + l * math.sin(a), (WHITE, FIRE[3], FIRE[2])[f], 1)
    if f == 0: cv.ell(8, 8, 2, 2, WHITE)
    return finish(cv, False, False)

def fx_dust(f):
    cv = Cv(8, 8); cv.nl = True
    r = (2, 3, 3.5)[f]
    for y in range(8):
        for x in range(8):
            if (x - 3.5) ** 2 + (y - 4) ** 2 <= r * r and (f < 2 or (x + y) % 2 == 0):
                cv.px(x, y, BONE[1] if f < 2 else BONE[0])
    return finish(cv, False, False)

def fx_fire(f):
    cv = Cv(16, 16); cv.nl = True
    j = (0, 1, 0, -1)[f]
    cv.ell(3.5, 8 + j, 3, 2.2, FIRE[0]); cv.ell(6.5, 8 - j, 3.6, 3, FIRE[1])
    cv.ell(10, 8, 4, 3.8, FIRE[2]); cv.ell(10.8, 8, 2.5, 2.3, FIRE[3])
    cv.px(1, 7 + j, FIRE[1]); cv.px(0, 9 - j, FIRE[0])
    return finish(cv, False, False)

def fx_expl(f):
    cv = Cv(32, 32); cv.nl = True
    if f == 0:
        cv.ell(16, 16, 7, 7, FIRE[2]); cv.ell(16, 16, 4, 4, FIRE[3])
    elif f == 1:
        cv.ell(16, 16, 12, 12, FIRE[1]); cv.ell(16, 16, 9, 9, FIRE[2]); cv.ell(16, 16, 5, 5, FIRE[3])
    elif f == 2:
        cv.ell(16, 16, 15, 15, FIRE[0]); cv.ell(16, 16, 12, 12, FIRE[1]); cv.ell(16, 16, 8, 8, FIRE[2])
        for i in range(8):
            a = math.radians(i * 45); cv.px(16 + 15 * math.cos(a), 16 + 15 * math.sin(a), FIRE[3])
    else:
        for y in range(32):
            for x in range(32):
                d = math.hypot(x - 16, y - 16)
                if 11 <= d <= 15 and (x + y) % 2 == 0:
                    cv.px(x, y, BONE[0])
    return finish(cv, False, False)

def fx_arrow(cv):
    cv.nl = True
    cv.line(1, 2, 13, 2, LEATH[2], 1); cv.rect(13, 1, 14, 3, STEEL[2]); cv.px(15, 2, STEEL[2])
    cv.px(0, 1, RED[2]); cv.px(0, 3, RED[2]); cv.px(1, 1, RED[1]); cv.px(1, 3, RED[1])

def fx_bolt(cv):
    cv.nl = True
    cv.line(0, 2, 9, 2, LEATH[2], 1); cv.rect(9, 1, 10, 3, STEEL[1]); cv.px(11, 2, STEEL[2])
    cv.px(0, 1, OUT); cv.px(0, 3, OUT)

def fx_spit(cv, f):
    cv.nl = True
    cv.ell(4, 4, 3 - f * 0.4, 3 + f * 0.4, GREEN[1]); cv.px(3, 3, GREEN[2]); cv.px(5, 6, GREEN[0])

def fx_wave(cv, f):
    cv.nl = True
    h = (13, 9)[f]
    for y in range(h):
        w = 6 * (1 - y / h) + 1.5
        cv.rect(8 - w, 15 - y, 8 + w, 15 - y, FIRE[1] if y < h * 0.6 else FIRE[0])
        w2 = w * 0.55
        cv.rect(8 - w2, 15 - y, 8 + w2, 15 - y, FIRE[2] if y < h * 0.5 else FIRE[1])
        w3 = w * 0.25
        if y < h * 0.6: cv.rect(8 - w3, 15 - y, 8 + w3, 15 - y, FIRE[3])

# ------------------------------------------------------------------ objets
def it_coin(cv, f):
    rx = (2.6, 1.8, 0.8, 1.8)[f]
    cv.ell(4, 4, rx, 2.7, GOLD[1])
    if rx > 1.5: cv.px(3, 3, GOLD[2])

HEART = [".##..##.", "########", "########", ".######.", "..####..", "...##..."]
def it_heart(cv):
    for y, row in enumerate(HEART):
        for x, ch in enumerate(row):
            if ch == '#': cv.px(1 + x, 2 + y, RED[1])
    cv.nl = True; cv.px(2, 3, RED[2]); cv.px(3, 3, RED[2]); cv.nl = False

def it_flask(cv):
    cv.rect(4, 2, 5, 4, BONE[2]); cv.rect(4, 1, 5, 1, LEATH[2])
    cv.ell(5, 8, 4, 3.4, BONE[1])
    cv.nl = True; cv.ell(5, 8.6, 3, 2.4, RED[1]); cv.rect(3, 7, 7, 7, RED[2]); cv.px(3, 8, WHITE); cv.nl = False

def it_chest(cv, op):
    cv.rect(1, 7, 14, 12, LEATH[1]); cv.rect(1, 7, 14, 7, LEATH[2])
    cv.rect(3, 7, 4, 12, GOLD[1]); cv.rect(11, 7, 12, 12, GOLD[1]); cv.rect(7, 8, 8, 9, GOLD[2])
    if not op:
        cv.rect(1, 3, 14, 6, LEATH[2]); cv.rect(2, 2, 13, 2, LEATH[2]); cv.rect(3, 3, 4, 6, GOLD[1]); cv.rect(11, 3, 12, 6, GOLD[1])
    else:
        cv.nl = True; cv.rect(2, 4, 13, 6, GOLD[2]); cv.rect(4, 3, 11, 3, FIRE[3]); cv.nl = False
        cv.rect(2, 0, 13, 1, LEATH[2]); cv.rect(3, 0, 4, 1, GOLD[1])

def icon_wpn(cv, k):
    if k == 0:       # epee
        cv.line(6, 9, 13, 2, STEEL[2], 2); cv.line(4, 11, 6, 9, LEATH[1], 2)
        cv.line(4, 7, 8, 11, GOLD[1], 1); cv.px(3, 12, GOLD[1])
    elif k == 1:     # dague
        cv.line(7, 9, 12, 4, STEEL[2], 2); cv.line(5, 11, 7, 9, LEATH[1], 2); cv.line(5, 8, 8, 11, GOLD[1], 1)
    elif k == 2:     # arc
        cv.line(10, 1, 13, 8, LEATH[2], 1); cv.line(13, 8, 10, 14, LEATH[2], 1); cv.line(10, 1, 10, 14, BONE[2], 1)
        cv.line(3, 8, 12, 8, BONE[2], 1); cv.px(2, 7, RED[2]); cv.px(2, 9, RED[2]); cv.px(13, 8, STEEL[2])
    elif k == 3:     # marteau
        cv.line(3, 13, 10, 6, LEATH[1], 1); cv.rect(8, 1, 14, 6, STEEL[1]); cv.rect(8, 1, 14, 1, STEEL[2])
    elif k == 4:     # baton de feu
        cv.line(3, 13, 10, 6, LEATH[1], 1); cv.nl = True
        cv.ell(11.5, 4.5, 3, 3, FIRE[1]); cv.ell(11.5, 4.5, 2, 2, FIRE[2]); cv.px(11, 4, FIRE[3]); cv.nl = False
    else:            # sceptre d'eclair
        cv.line(3, 13, 10, 6, STEEL[1], 1); cv.nl = True
        cv.ell(11.5, 4.5, 3, 3, ELEC[1]); cv.px(11, 4, WHITE); cv.px(12, 5, WHITE)
        cv.line(13, 1, 11, 3, ELEC[2], 1); cv.nl = False

# ------------------------------------------------------------------ tuiles
def tile_solid(z, v):
    cv = Cv(16, 16); rnd = random.Random(z * 97 + v * 13 + 5)
    bd, d, m, l = ZR[z]
    cv.rect(0, 0, 15, 15, m)
    for y in (7, 15): cv.rect(0, y, 15, y, d)
    for x in (7, 15): cv.rect(x, 0, x, 6, d)
    for x in (3, 11): cv.rect(x, 8, x, 14, d)
    for x in range(16):
        if rnd.random() < 0.7: cv.px(x, 0, l)
        if rnd.random() < 0.7: cv.px(x, 8, l)
    for _ in range(8 + v * 3):
        x, y = rnd.randrange(16), rnd.randrange(16)
        if cv.a[y][x] == m: cv.px(x, y, rnd.choice([d, l, d]))
    if v == 1:
        cv.line(10, 1, 12, 5, d, 1); cv.px(12, 6, d)
    if v == 2:
        cv.line(2, 9, 5, 12, d, 1); cv.px(5, 13, d)
    return finish(cv, False, False)

CAPCOL = [(GREEN[0], GREEN[1]), (GOLD[0], LEATH[2]), (GREEN[1], GREEN[2]), (STEEL[1], WHITE), (ACCS[4][1], ACCS[4][2])]
def tile_cap(z):
    cv = Cv(16, 6); rnd = random.Random(z + 41)
    bd, d, m, l = ZR[z]; c0, c1 = CAPCOL[z]
    cv.rect(0, 0, 15, 1, l)
    if z in (0, 1, 2):
        cv.rect(0, 1, 15, 1, c1)
        for x in range(16):
            if rnd.random() < 0.55: cv.px(x, 2, c0)
            if z == 2 and rnd.random() < 0.3: cv.rect(x, 3, x, 4 + rnd.randrange(2), c1)
    elif z == 3:
        cv.rect(0, 0, 15, 0, c1)
        for x in range(16):
            if rnd.random() < 0.3: cv.px(x, 2, c0)
    else:
        cv.rect(0, 0, 15, 0, c1)
        for x in (2, 6, 11): cv.rect(x, 1, x + 1, 2, c0); cv.px(x, 3, c0)
    return finish(cv, False, False)

def tile_plat(kind):
    cv = Cv(16, 6)
    if kind == 'wood':
        cv.rect(0, 0, 15, 1, LEATH[2]); cv.rect(0, 2, 15, 3, LEATH[1]); cv.rect(0, 4, 15, 4, LEATH[0])
        for x in (4, 9, 13): cv.rect(x, 0, x, 3, LEATH[0])
        cv.px(1, 2, STEEL[1]); cv.px(14, 2, STEEL[1])
    elif kind == 'steel':
        cv.rect(0, 0, 15, 1, STEEL[2]); cv.rect(0, 2, 15, 3, STEEL[1]); cv.rect(0, 4, 15, 4, STEEL[0])
        for x in range(1, 16, 3): cv.rect(x, 2, x, 3, STEEL[0])
    else:
        cv.rect(0, 0, 15, 1, GOLD[2]); cv.rect(0, 2, 15, 3, ACCS[4][1]); cv.rect(0, 4, 15, 4, ACCS[4][0])
        for x in (3, 8, 12): cv.px(x, 2, GOLD[1])
    return finish(cv, False, False)

def tile_spike(cv):
    cv.rect(0, 12, 15, 15, STEEL[0])
    for i in range(4):
        x0 = i * 4
        for h in range(11):
            w = max(0, 1.6 - h * 0.15)
            cv.rect(x0 + 1.5 - w, 11 - h, x0 + 1.5 + w, 11 - h, STEEL[1] if h < 9 else STEEL[2])
        cv.px(x0 + 1, 6, STEEL[2]); cv.px(x0 + 1, 7, STEEL[2])

def tile_toxic(cv, f):
    cv.nl = True
    cv.rect(0, 0, 15, 15, GREEN[0]); cv.rect(0, 3, 15, 15, GREEN[1])
    for x in range(16):
        y = 2 + R(1.2 * math.sin((x + f * 4) * math.pi / 4))
        cv.rect(x, y, x, y + 1, GREEN[2])
    cv.px(3 + f * 5, 8, GREEN[2]); cv.px(11 - f * 4, 11, GREEN[2]); cv.px(7, 6 + f, GREEN[0])

def tile_door(cv):
    cv.rect(0, 4, 15, 31, BONE[0]); cv.ell(8, 8, 8, 8, BONE[0]); cv.rect(0, 12, 15, 31, BONE[0])
    cv.ell(8, 9, 6, 6, LEATH[1]); cv.rect(2, 9, 13, 31, LEATH[1])
    for x in (5, 8, 11): cv.rect(x, 5, x, 31, LEATH[0])
    cv.rect(2, 13, 13, 14, STEEL[1]); cv.rect(2, 24, 13, 25, STEEL[1])
    cv.ell(11, 21, 1.4, 1.4, GOLD[1]); cv.nl = True
    cv.rect(2, 30, 13, 31, FIRE[2]); cv.rect(7, 3, 8, 3, FIRE[3]); cv.nl = False

def tile_gate(cv):
    for x in (1, 5, 9, 13):
        cv.rect(x, 1, x + 1, 15, STEEL[1]); cv.px(x, 0, STEEL[2]); cv.px(x + 1, 0, STEEL[2])
    cv.rect(0, 4, 15, 5, STEEL[0]); cv.rect(0, 12, 15, 13, STEEL[0])

def tile_torch(cv, f):
    cv.rect(3, 9, 4, 15, LEATH[1]); cv.rect(2, 8, 5, 9, STEEL[1]); cv.rect(1, 15, 6, 15, STEEL[0])
    cv.nl = True
    j = (0, 1, -1)[f]
    cv.ell(3.5, 4.5, 2.2, 3.2 + j * 0.5, FIRE[1]); cv.ell(3.5, 5, 1.5, 2.4 + j * 0.4, FIRE[2]); cv.px(3.5, 5.5, FIRE[3]); cv.px(3 + j, 0.5, FIRE[1])

def bg_wall(z):
    cv = Cv(32, 32); rnd = random.Random(z + 300)
    bd, d, m, l = ZR[z]
    cv.rect(0, 0, 31, 31, bd)
    for y in (7, 15, 23, 31): cv.rect(0, y, 31, y, d)
    for row in range(4):
        for x in ((15, 31) if row % 2 == 0 else (7, 23)):
            cv.rect(x, row * 8, x, row * 8 + 6, d)
    for _ in range(26):
        x, y = rnd.randrange(32), rnd.randrange(32)
        if cv.a[y][x] == bd: cv.px(x, y, d)
    return finish(cv, False, False)

def bg_deco(z):
    cv = Cv(32, 48); bd, d, m, l = ZR[z]
    if z == 0:                                           # fenetre a barreaux, clair de lune
        cv.rect(4, 14, 27, 47, d); cv.ell(16, 16, 12, 12, d)
        cv.rect(7, 16, 24, 47, m); cv.ell(16, 17, 9, 9, m)
        for y in range(16, 48):
            cv.rect(7, y, 24, y, ZR[0][3] if y < 40 else m)
        for x in (10, 15, 20): cv.rect(x, 8, x + 1, 47, d)
        cv.rect(7, 28, 24, 29, d)
    elif z == 1:                                         # banniere cramoisie
        cv.rect(15, 0, 16, 6, STEEL[0]); cv.rect(5, 6, 26, 7, GOLD[1])
        cv.rect(7, 8, 24, 38, RED[1]); cv.rect(7, 8, 8, 38, RED[0]); cv.rect(23, 8, 24, 38, RED[0])
        for i in range(0, 18, 6): cv.rect(7 + i, 38, 12 + i, 42 - (i // 6) % 2 * 2, RED[1])
        cv.ell(15.5, 20, 5, 6, GOLD[1]); cv.ell(15.5, 20, 3, 4, RED[0]); cv.rect(14, 16, 17, 24, GOLD[2])
    elif z == 2:                                         # sortie d'egout + coulure toxique
        cv.ell(16, 12, 12, 11, d); cv.ell(16, 12, 9, 8, bd)
        cv.rect(11, 18, 21, 47, GREEN[0])
        for y in range(18, 48):
            cv.rect(11 + (y % 4 == 0), y, 21 - (y % 5 == 0), y, GREEN[1] if y % 3 else GREEN[2])
        cv.ell(16, 12, 6, 5, GREEN[0])
    elif z == 3:                                         # meurtriere sur le ciel
        cv.rect(8, 10, 23, 47, d); cv.ell(16, 12, 8, 8, d)
        cv.rect(11, 12, 20, 47, ZR[3][2]); cv.ell(16, 14, 5, 5, ZR[3][2])
        for y in range(14, 44):
            cv.rect(11, y, 20, y, ZR[3][3] if y < 30 else ZR[3][2])
        cv.px(14, 20, WHITE); cv.px(18, 26, WHITE)
    else:                                                # vitrail violet et or
        cv.ell(16, 14, 12, 13, GOLD[0]); cv.rect(4, 14, 27, 47, GOLD[0])
        cv.ell(16, 15, 10, 11, ACCS[4][0]); cv.rect(6, 15, 25, 47, ACCS[4][0])
        for i, (x0, x1) in enumerate(((7, 14), (17, 24))):
            for y in range(6, 47, 8):
                col = (ACCS[4][1], ACCS[4][2], GOLD[1], RED[1])[(i + y // 8) % 4]
                if cv.a[y + 1][x0] != 0: cv.rect(x0, y + 1, x1, y + 6, col)
        cv.rect(15, 5, 16, 47, GOLD[1]); cv.rect(6, 28, 25, 28, GOLD[1])
    return finish(cv, False, False)

# ------------------------------------------------------------------ police 5x7
G = {
 'A': [".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"],
 'B': ["####.", "#...#", "#...#", "####.", "#...#", "#...#", "####."],
 'C': [".###.", "#...#", "#....", "#....", "#....", "#...#", ".###."],
 'D': ["####.", "#...#", "#...#", "#...#", "#...#", "#...#", "####."],
 'E': ["#####", "#....", "#....", "####.", "#....", "#....", "#####"],
 'F': ["#####", "#....", "#....", "####.", "#....", "#....", "#...."],
 'G': [".###.", "#...#", "#....", "#.###", "#...#", "#...#", ".###."],
 'H': ["#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"],
 'I': [".###.", "..#..", "..#..", "..#..", "..#..", "..#..", ".###."],
 'J': ["..###", "...#.", "...#.", "...#.", "...#.", "#..#.", ".##.."],
 'K': ["#...#", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "#...#"],
 'L': ["#....", "#....", "#....", "#....", "#....", "#....", "#####"],
 'M': ["#...#", "##.##", "#.#.#", "#.#.#", "#...#", "#...#", "#...#"],
 'N': ["#...#", "##..#", "#.#.#", "#..##", "#...#", "#...#", "#...#"],
 'O': [".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."],
 'P': ["####.", "#...#", "#...#", "####.", "#....", "#....", "#...."],
 'Q': [".###.", "#...#", "#...#", "#...#", "#.#.#", "#..#.", ".##.#"],
 'R': ["####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#"],
 'S': [".####", "#....", "#....", ".###.", "....#", "....#", "####."],
 'T': ["#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."],
 'U': ["#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."],
 'V': ["#...#", "#...#", "#...#", "#...#", "#...#", ".#.#.", "..#.."],
 'W': ["#...#", "#...#", "#...#", "#.#.#", "#.#.#", "##.##", "#...#"],
 'X': ["#...#", "#...#", ".#.#.", "..#..", ".#.#.", "#...#", "#...#"],
 'Y': ["#...#", "#...#", ".#.#.", "..#..", "..#..", "..#..", "..#.."],
 'Z': ["#####", "....#", "...#.", "..#..", ".#...", "#....", "#####"],
 '0': [".###.", "#...#", "#..##", "#.#.#", "##..#", "#...#", ".###."],
 '1': ["..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###."],
 '2': [".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####"],
 '3': ["####.", "....#", "....#", ".###.", "....#", "....#", "####."],
 '4': ["...#.", "..##.", ".#.#.", "#..#.", "#####", "...#.", "...#."],
 '5': ["#####", "#....", "####.", "....#", "....#", "#...#", ".###."],
 '6': [".###.", "#....", "#....", "####.", "#...#", "#...#", ".###."],
 '7': ["#####", "....#", "...#.", "..#..", ".#...", ".#...", ".#..."],
 '8': [".###.", "#...#", "#...#", ".###.", "#...#", "#...#", ".###."],
 '9': [".###.", "#...#", "#...#", ".####", "....#", "....#", ".###."],
 '.': [".....", ".....", ".....", ".....", ".....", ".##..", ".##.."],
 ',': [".....", ".....", ".....", ".....", ".##..", ".##..", ".#..."],
 ':': [".....", ".##..", ".##..", ".....", ".##..", ".##..", "....."],
 '!': ["..#..", "..#..", "..#..", "..#..", "..#..", ".....", "..#.."],
 '?': [".###.", "#...#", "....#", "...#.", "..#..", ".....", "..#.."],
 '-': [".....", ".....", ".....", "#####", ".....", ".....", "....."],
 '+': [".....", "..#..", "..#..", "#####", "..#..", "..#..", "....."],
 '/': ["....#", "....#", "...#.", "..#..", ".#...", "#....", "#...."],
 '%': ["##..#", "##..#", "...#.", "..#..", ".#...", "#..##", "#..##"],
 '(': ["...#.", "..#..", ".#...", ".#...", ".#...", "..#..", "...#."],
 ')': [".#...", "..#..", "...#.", "...#.", "...#.", "..#..", ".#..."],
 "'": ["..#..", "..#..", ".#...", ".....", ".....", ".....", "....."],
 '=': [".....", ".....", "#####", ".....", "#####", ".....", "....."],
 '>': ["#....", ".#...", "..#..", "...#.", "..#..", ".#...", "#...."],
 '<': ["....#", "...#.", "..#..", ".#...", "..#..", "...#.", "....#"],
 '*': [".....", "#.#.#", ".###.", "#####", ".###.", "#.#.#", "....."],
 ' ': [".....", ".....", ".....", ".....", ".....", ".....", "....."],
}
ARROWS = {   # codes speciaux
 136: [".....", "..#..", ".#...", "#####", ".#...", "..#..", "....."],   # <-
 137: [".....", "..#..", "...#.", "#####", "...#.", "..#..", "....."],   # ->
 138: ["..#..", ".###.", "#.#.#", "..#..", "..#..", "..#..", "..#.."],   # haut
 139: ["..#..", "..#..", "..#..", "..#..", "#.#.#", ".###.", "..#.."],   # bas
 140: [".#.#.", "#####", "#####", "#####", ".###.", "..#..", "....."],   # coeur
}
ACCENTS = {128: ('E', '.#.#.'.replace('.#.#.', '...#.')), 129: ('E', '.#...'), 130: ('A', '.#...'),
           131: ('E', '..#..'), 132: ('C', None), 133: ('A', '..#..'), 134: ('I', '..#..')}

def font_table():
    """144 glyphes x 9 octets : [rangee accent][7 rangees][rangee basse]; bit 4 = pixel de gauche."""
    t = [[0] * 9 for _ in range(144)]
    def put(code, rows, acc=None, desc=None):
        assert len(rows) == 7 and all(len(r) == 5 for r in rows), (code, rows)
        if acc: t[code][0] = int(acc.replace('#', '1').replace('.', '0'), 2)
        for i, r in enumerate(rows):
            t[code][1 + i] = int(r.replace('#', '1').replace('.', '0'), 2)
        if desc: t[code][8] = int(desc.replace('#', '1').replace('.', '0'), 2)
    for ch, rows in G.items():
        put(ord(ch), rows)
    for code, rows in ARROWS.items():
        put(code, rows)
    for code, (base, acc) in ACCENTS.items():
        put(code, G[base], acc, '.#...' if base == 'C' else None)
    return t

# ------------------------------------------------------------------ ajouts : icones des 10 armes, bombe, nuage toxique, cellule, schema
def icon_wpn10(cv, k):
    """ordre = identifiants d'armes du jeu : epee, dague, rapiere, marteau, bouclier, arc, arbalete, baton de feu, sceptre, faux-serres."""
    if k == 0:
        icon_wpn(cv, 0)
    elif k == 1:
        icon_wpn(cv, 1)
    elif k == 2:       # rapiere : lame fine + garde en coupe
        cv.line(6, 9, 14, 1, STEEL[2], 1); cv.line(4, 11, 6, 9, LEATH[1], 2)
        cv.ell(6.5, 9.5, 2.4, 2.4, GOLD[1]); cv.px(3, 12, GOLD[1])
    elif k == 3:
        icon_wpn(cv, 3)
    elif k == 4:       # bouclier
        cv.ell(8, 8, 5.5, 6.5, STEEL[1]); cv.ell(8, 8, 3.6, 4.6, STEEL[0])
        cv.rect(7, 4, 8, 12, GOLD[1]); cv.rect(5, 7, 10, 8, GOLD[1]); cv.px(8, 8, GOLD[2])
    elif k == 5:
        icon_wpn(cv, 2)
    elif k == 6:       # arbalete
        cv.line(2, 8, 13, 8, LEATH[1], 2); cv.line(10, 2, 10, 14, LEATH[2], 1); cv.line(10, 2, 14, 8, BONE[2], 1)
        cv.line(10, 14, 14, 8, BONE[2], 1); cv.line(3, 8, 14, 8, STEEL[2], 1); cv.px(14, 8, WHITE)
    elif k == 7:
        icon_wpn(cv, 4)
    elif k == 8:
        icon_wpn(cv, 5)
    else:              # faux-serres : lame courbe + serres
        cv.line(3, 13, 9, 5, LEATH[1], 1)
        for t in range(0, 16):
            a = math.radians(-100 + t * 11)
            cv.px(9 + 6 * math.cos(a), 8 + 6 * math.sin(a), STEEL[2])
        cv.line(13, 6, 14, 9, STEEL[1], 1); cv.px(14, 10, STEEL[2])
        cv.line(4, 12, 7, 14, GOLD[1], 1)

def fx_bomb(cv, f):
    cv.nl = True
    cv.ell(4, 5, 3, 3, STEEL[0]); cv.px(3, 4, STEEL[2]); cv.px(5, 6, STEEL[1])
    cv.px(5, 1, LEATH[2]); cv.px(5, 0, FIRE[2] if f == 0 else FIRE[3])
    if f: cv.px(6, 0, FIRE[2])

def fx_cloud(cv, f):
    cv.nl = True
    for (x, y, r, c) in ((6, 10, 5, GREEN[0]), (12, 8, 6, GREEN[1]), (18, 10, 5, GREEN[0]), (10, 11, 4, GREEN[1]), (15, 11, 4, GREEN[1])):
        for yy in range(16):
            for xx in range(24):
                if (xx - x) ** 2 + (yy - y + (f)) ** 2 <= r * r and (xx + yy + f) % 3 != 0:
                    cv.px(xx, yy, c)
    cv.px(9, 6 + f, GREEN[2]); cv.px(15, 4 - f, GREEN[2]); cv.px(19, 7, GREEN[2])

def it_cell(cv, f):
    cv.nl = True
    cv.ell(4, 4, 3.2 + f * 0.3, 3.2 + f * 0.3, ELEC[0]); cv.ell(4, 4, 2.2, 2.2, ELEC[1]); cv.px(3, 3, WHITE); cv.px(4, 3, ELEC[2])

def it_blueprint(cv):
    cv.rect(1, 1, 8, 10, ELEC[1]); cv.rect(1, 1, 8, 1, ELEC[2])
    for y in (3, 5, 7): cv.rect(2, y, 7, y, WHITE)
    cv.rect(2, 9, 4, 9, ELEC[2])
