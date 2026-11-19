"""art_chars2.py - nouveaux ennemis (roles inspires du genre, dessins originaux) et PNJ des passages."""
import math
from art_core import *
from art_chars import hand, HCX, HGY, HW, HH

# ---------------------------------------------------------------- ARTIFICIER (lanceur de bombes)
def gren_draw(cv, P, bomb=True):
    r = Rig(cv, HCX, HGY, 1.0)
    L, T, tw = 7, 6, 3
    bob = P.get('bob', 0); lean = P.get('lean', 0)
    hip = (0, -L + bob); sh = (lean, -L - T + bob)
    hd = (lean + 1, -L - T - 4 + bob + P.get('hy', 0))
    ff = P.get('ff', (2, 0)); bf = P.get('bf', (-2, 0))
    fh = P.get('fh', (3, 2)); bh = P.get('bh', (-2, 4))
    bsh = (sh[0] - 1, sh[1] + 1); fsh = (sh[0] + 1, sh[1] + 1)
    arm(r, bsh, bh, ACC[0]); hand(r, bsh[0] + bh[0], bsh[1] + bh[1])
    leg(r, hip, bf, LEATH[0], LEATH[0])
    r.rect(sh[0] - tw, sh[1], sh[0] + tw, hip[1] + 2, LEATH[1])
    r.rect(sh[0] - tw + 1, sh[1] + 1, sh[0] + tw - 1, hip[1] + 2, ACC[1])
    for i in range(3):                                        # bandouliere de petites bombes
        bx, by = sh[0] - 2 + i * 2, sh[1] + 2 + i * 1.4
        r.ell(bx, by, 1.1, 1.1, STEEL[0])
        cv.nl = True; r.px(bx, by - 1.6, FIRE[2]); cv.nl = False
    r.rect(sh[0] - tw, hip[1] - 1, sh[0] + tw, hip[1] - 1, LEATH[2])
    leg(r, hip, ff, LEATH[1], LEATH[1])
    r.ell(hd[0], hd[1], 3.8, 4.0, ACC[1]); r.ell(hd[0] - 2.5, hd[1] + 1.8, 2.0, 3.0, ACC[0])
    r.ell(hd[0] + 1.4, hd[1] + 0.6, 2.2, 2.4, LEATH[0])
    cv.nl = True                                              # lunettes de mineur
    r.ell(hd[0] + 0.8, hd[1] - 0.2, 1.3, 1.3, GOLD[2]); r.ell(hd[0] + 2.9, hd[1] - 0.2, 1.1, 1.1, GOLD[2])
    r.px(hd[0] + 0.8, hd[1] - 0.2, OUT); r.px(hd[0] + 2.9, hd[1] - 0.2, OUT)
    cv.nl = False
    arm(r, fsh, fh, ACC[2]); fx, fy = fsh[0] + fh[0], fsh[1] + fh[1]
    if bomb:
        r.ell(fx + 1, fy - 2, 2.6, 2.6, STEEL[0]); r.px(fx + 0.5, fy - 3.5, STEEL[2])
        cv.nl = True; r.px(fx + 1.5, fy - 5, FIRE[2]); r.px(fx + 1.5, fy - 6, FIRE[3]); cv.nl = False
    hand(r, fx, fy)

def build_gren():
    A = {}
    def mk(seq):
        out = []
        for P, k in seq:
            cv = Cv(HW, HH); gren_draw(cv, P, **k); out.append(finish(cv))
        return out
    A['GREN_IDLE'] = mk([(dict(), {})])
    A['GREN_WALK'] = mk([(dict(runpose(i, 4, 3, 2, 2, 1), fh=(3, 2), bh=(-2, 4)), {}) for i in range(4)])
    A['GREN_WIND'] = mk([(dict(lean=-2, fh=(1, -9), bh=(-3, 2), ff=(2, 0), bf=(-3, 0)), {})])
    A['GREN_THROW'] = mk([(dict(lean=3, bob=1, fh=(7, -3), bh=(-1, 3), ff=(4, 0), bf=(-4, 0)), dict(bomb=False))])
    A['GREN_HURT'] = mk([(dict(lean=-3, hy=1, fh=(-3, -3), bh=(-4, -1), ff=(1, 0), bf=(-4, 0)), {})])
    out = mk([(dict(bob=4, lean=-2, ff=(3, 0), bf=(-1, 0), fh=(2, 3), bh=(-3, 3), hy=1), dict(bomb=False))])
    cv = Cv(HW, HH); r = Rig(cv, HCX, HGY)
    r.ell(-1, -3, 9, 2.8, LEATH[1]); r.ell(-1, -3.5, 6, 2, ACC[1]); r.ell(8, -3, 3.2, 3, ACC[1])
    r.ell(-5, -5.5, 1.3, 1.3, STEEL[0]); r.ell(-2, -5.8, 1.3, 1.3, STEEL[0]); r.rect(-10, -3, -8, -1, LEATH[0])
    out.append(finish(cv))
    A['GREN_DEAD'] = out
    return A

# ---------------------------------------------------------------- GARDE-BOUCLIER
def shld_draw(cv, P, sdx=0, sword=True):
    r = Rig(cv, HCX, HGY, 1.0)
    L, T, tw = 7, 7, 4
    bob = P.get('bob', 0); lean = P.get('lean', 0)
    hip = (0, -L + bob); sh = (lean, -L - T + bob)
    hd = (lean + 1, -L - T - 3 + bob + P.get('hy', 0))
    ff = P.get('ff', (2, 0)); bf = P.get('bf', (-2, 0))
    bh = P.get('bh', (-3, 4))
    bsh = (sh[0] - 2, sh[1] + 1)
    arm(r, bsh, bh, ACC[0]); hand(r, bsh[0] + bh[0], bsh[1] + bh[1])
    if sword:
        from art_core import weapon
        weapon(r, bsh[0] + bh[0], bsh[1] + bh[1], P.get('sang', -60), 'dagger')
    leg(r, hip, bf, LEATH[0], STEEL[0], 2, 2)
    r.rect(sh[0] - tw, sh[1], sh[0] + tw, hip[1] + 1, STEEL[0])
    r.rect(sh[0] - 2, sh[1] + 1, sh[0] + 2, hip[1] + 2, ACC[1])
    r.rect(sh[0] - tw, hip[1] - 1, sh[0] + tw, hip[1] - 1, LEATH[1])
    leg(r, hip, ff, LEATH[1], STEEL[1], 2, 2)
    r.ell(hd[0], hd[1], 3.6, 3.7, STEEL[1]); r.rect(hd[0] - 3, hd[1] - 4, hd[0] + 2, hd[1] - 3, STEEL[0])
    cv.nl = True; r.rect(hd[0] + 0.3, hd[1] - 0.3, hd[0] + 3, hd[1] + 0.4, OUT); r.px(hd[0] + 2, hd[1], RED[2]); cv.nl = False
    r.px(hd[0] - 1, hd[1] + 2, STEEL[2])
    r.ell(hd[0] - 0.5, hd[1] - 4.5, 1.3, 1.3, GOLD[1])                    # cimier dore
    sx = sh[0] + 4 + sdx                                                   # grand bouclier tour
    r.rect(sx, sh[1] - 3, sx + 4, hip[1] + 6, STEEL[1])
    r.rect(sx, sh[1] - 3, sx, hip[1] + 6, STEEL[2]); r.rect(sx + 4, sh[1] - 3, sx + 4, hip[1] + 6, STEEL[0])
    r.rect(sx + 1, sh[1] + 2, sx + 3, sh[1] + 3, GOLD[1]); r.rect(sx + 2, sh[1], sx + 2, sh[1] + 6, GOLD[1])
    r.rect(sx, hip[1] + 6, sx + 4, hip[1] + 6, STEEL[0])
    hand(r, sx + 1, sh[1] + 4, STEEL[2])

def build_shld():
    A = {}
    def mk(seq):
        out = []
        for P, k in seq:
            cv = Cv(HW, HH); shld_draw(cv, P, **k); out.append(finish(cv))
        return out
    A['SHLD_IDLE'] = mk([(dict(), {})])
    A['SHLD_WALK'] = mk([(runpose(i, 4, 2.5, 1, 1, 1), {}) for i in range(4)])
    A['SHLD_WIND'] = mk([(dict(lean=-2, bh=(-4, 2)), dict(sdx=-3))])
    A['SHLD_BASH'] = mk([(dict(lean=3, bob=1, ff=(4, 0), bf=(-3, 0), bh=(-2, 3)), dict(sdx=5))])
    A['SHLD_HURT'] = mk([(dict(lean=-3, hy=1, bh=(-4, 0), bf=(-4, 0)), dict(sdx=-4, sword=False))])
    out = mk([(dict(bob=5, lean=-3, ff=(4, 0), bf=(-1, 0), bh=(-3, 3), hy=1), dict(sdx=-5, sword=False))])
    cv = Cv(HW, HH); r = Rig(cv, HCX, HGY)
    r.ell(-1, -3, 9, 2.8, STEEL[0]); r.ell(-1, -3.5, 6, 2, ACC[1]); r.ell(8, -3, 3.2, 3, STEEL[1])
    r.rect(-6, -9, -1, -1, STEEL[1]); r.rect(-6, -9, -6, -1, STEEL[2]); r.rect(-4, -6, -3, -4, GOLD[1])
    out.append(finish(cv))
    A['SHLD_DEAD'] = out
    return A

# ---------------------------------------------------------------- PUTRIDE (zombie vert toxique)
def toxic_draw(cv, P, reach=False):
    r = Rig(cv, HCX, HGY, 1.0)
    L, T, tw = 6, 6, 3
    bob = P.get('bob', 0); lean = P.get('lean', 2)
    hip = (0, -L + bob); sh = (lean, -L - T + bob + 1)
    hd = (lean + 2, -L - T - 2 + bob + P.get('hy', 0))
    ff = P.get('ff', (2, 0)); bf = P.get('bf', (-2, 0))
    fh = P.get('fh', (4, 6)); bh = P.get('bh', (2, 6))
    bsh = (sh[0] - 1, sh[1] + 1); fsh = (sh[0] + 1, sh[1] + 1)
    arm(r, bsh, bh, GREEN[0], 2); hand(r, bsh[0] + bh[0], bsh[1] + bh[1], GREEN[0])
    leg(r, hip, bf, ACC[0], ACC[0], 2, 1)
    r.rect(sh[0] - tw, sh[1], sh[0] + tw, hip[1] + 1, GREEN[1])                 # torse verdatre
    r.rect(sh[0] - tw, sh[1] + 2, sh[0] + 1, hip[1] + 1, ACC[0])               # haillons
    for x in (-3, -1, 1):
        r.rect(sh[0] + x, hip[1] + 1, sh[0] + x, hip[1] + 3, ACC[0])
    cv.nl = True; r.px(sh[0] + 2, sh[1] + 3, GREEN[2]); r.px(sh[0] + 1, sh[1] + 5, GREEN[2]); cv.nl = False
    leg(r, hip, ff, ACC[1], ACC[0], 2, 1)
    r.ell(hd[0], hd[1], 3.5, 3.4, GREEN[1])
    r.rect(hd[0] - 1, hd[1] + 2, hd[0] + 2, hd[1] + 3.5, GREEN[0])             # machoire pendante
    cv.nl = True
    r.px(hd[0] + 0.5, hd[1] - 0.5, RED[2]); r.px(hd[0] + 2.2, hd[1] - 0.5, RED[2])
    r.px(hd[0] + 1, hd[1] + 2.5, OUT); r.px(hd[0] + 2, hd[1] + 3.5, GREEN[2])
    cv.nl = False
    arm(r, fsh, fh, GREEN[1], 2)
    fx, fy = fsh[0] + fh[0], fsh[1] + fh[1]
    hand(r, fx, fy, GREEN[1])
    cv.nl = True; r.px(fx + 1.5, fy + 1, BONE[2]); r.px(fx + 1.5, fy + 2.5, GREEN[2]); cv.nl = False

def build_toxic():
    A = {}
    def mk(seq):
        out = []
        for P, k in seq:
            cv = Cv(HW, HH); toxic_draw(cv, P, **k); out.append(finish(cv))
        return out
    A['TOXIC_IDLE'] = mk([(dict(), {})])
    A['TOXIC_WALK'] = mk([(dict(runpose(i, 4, 2.5, 1, 1, 1), lean=2, fh=(4, 6), bh=(2, 6)), {}) for i in range(4)])
    A['TOXIC_ATK'] = mk([(dict(lean=4, bob=1, fh=(8, 0), bh=(6, 1), ff=(4, 0), bf=(-3, 0)), dict(reach=True))])
    A['TOXIC_HURT'] = mk([(dict(lean=-1, hy=1, fh=(1, 3), bh=(0, 4), bf=(-3, 0)), {})])
    cv = Cv(HW, HH); r = Rig(cv, HCX, HGY)
    r.ell(-1, -3, 8, 3, GREEN[1]); r.ell(-1, -3.6, 5, 2, ACC[0]); r.ell(7, -3, 3, 2.8, GREEN[1])
    r.px(8, -3.6, RED[2]); r.rect(-9, -2, -8, -1, GREEN[0])
    d0 = finish(cv)
    cv = Cv(HW, HH); r = Rig(cv, HCX, HGY)
    r.ell(0, -1.6, 11, 1.8, GREEN[1]); r.ell(0, -2, 7, 1.2, GREEN[2]); r.ell(6, -2.8, 2.2, 1.4, BONE[1])
    A['TOXIC_DEAD'] = [d0, finish(cv)]
    return A

# ---------------------------------------------------------------- CHAUVE-SOURIS (volante, pique sur le joueur)
def bat_draw(cv, ph=0, mode='fly'):
    wy = (-7, -3, 3, -3)[ph]
    if mode == 'dive':
        wing_tips = ((9, 17), (23, 17)); sh_ = 4
    elif mode == 'dead':
        wing_tips = ((10, 13), (22, 13)); sh_ = 0
    else:
        wing_tips = ((3, 10 + wy), (29, 10 + wy)); sh_ = 0
    for tipx, tipy in wing_tips:
        sx = 14 if tipx < 16 else 18
        for k in range(0, 8):
            t = k / 7.0
            cv.line(tipx, tipy, sx + (1 if tipx < 16 else -1) * 0, 8 + 5 * t + sh_, ACC[0] if k % 2 == 0 else ACC[1], 1)
        cv.line(sx, 9 + sh_, tipx, tipy, ACC[2], 1)
    cv.ell(16, 10 + sh_, 3.4, 4.2, ACC[1])
    cv.ell(16, 6 + sh_, 3.0, 2.8, ACC[1])
    cv.px(13.5, 3 + sh_, ACC[1]); cv.px(18.5, 3 + sh_, ACC[1]); cv.px(13, 2 + sh_, ACC[2]); cv.px(19, 2 + sh_, ACC[2])
    cv.nl = True
    cv.px(14.5, 6 + sh_, RED[2]); cv.px(17.5, 6 + sh_, RED[2])
    if mode != 'dead':
        cv.px(15, 8.5 + sh_, BONE[2]); cv.px(17, 8.5 + sh_, BONE[2])
    cv.nl = False

def build_bat():
    A = {}
    A['BAT_FLY'] = []
    for i in range(4):
        cv = Cv(32, 20); bat_draw(cv, i); A['BAT_FLY'].append(finish(cv))
    cv = Cv(32, 20); bat_draw(cv, 0, 'dive'); A['BAT_DIVE'] = [finish(cv)]
    cv = Cv(32, 20); bat_draw(cv, 2); A['BAT_HURT'] = [finish(cv)]
    out = []
    cv = Cv(32, 20); bat_draw(cv, 0, 'dead'); out.append(finish(cv))
    cv = Cv(32, 20); cv.ell(16, 16, 9, 1.8, ACC[0]); cv.ell(16, 15, 3, 1.5, ACC[1]); cv.px(14, 14, RED[2]); out.append(finish(cv))
    A['BAT_DEAD'] = out
    return A

# ---------------------------------------------------------------- PNJ des passages (24x32)
def npc_draw(cv, kind, f):
    r = Rig(cv, 12, 30, 1.0)
    pul = (0, 1)[f]
    if kind == 0:      # le Passeur : robe indigo, capuche, grosse orbe de cellules bleues dans les mains
        r.ell(0, -9, 6.5, 9, CLOAK[1]); r.rect(-6, -3, 6, -1, CLOAK[0]); r.rect(-5, -1, 5, 0, CLOAK[0])
        r.ell(0, -17, 4.2, 4.4, CLOAK[1]); r.ell(1.2, -16.5, 2.4, 2.6, LEATH[0])
        cv.nl = True; r.px(1.6, -17, ELEC[2]); r.px(3, -17, ELEC[2]); cv.nl = False
        r.line(-4, -11, 2, -8, CLOAK[2], 2); r.line(4, -11, 1, -8, CLOAK[2], 2)
        cv.nl = True
        r.ell(0, -8.5, 3.2 + pul * 0.6, 3.2 + pul * 0.6, ELEC[1]); r.ell(0, -8.5, 1.8, 1.8, ELEC[2]); r.px(-0.5, -9, WHITE)
        cv.nl = False
    else:              # l'Oracle : robe pourpre et or, cristal flottant avec un oeil
        r.ell(0, -9, 6.5, 9, ACCS[4][1]); r.rect(-6, -3, 6, -1, ACCS[4][0]); r.rect(-5, -1, 5, 0, ACCS[4][0])
        r.rect(-1, -14, 1, -2, GOLD[1])
        r.ell(0, -17, 4.2, 4.4, ACCS[4][1]); r.ell(1.2, -16.5, 2.4, 2.6, LEATH[0])
        cv.nl = True; r.px(1.6, -17, GOLD[2]); r.px(3, -17, GOLD[2]); cv.nl = False
        r.line(-4, -11, -3, -8, ACCS[4][2], 2); r.line(4, -11, 3, -8, ACCS[4][2], 2)
        cv.nl = True
        cy = -22 - pul
        r.ell(0, cy, 3, 3.6, ACCS[4][2]); r.ell(0, cy, 1.8, 2.2, WHITE); r.px(0, cy, RED[1]); r.px(0, cy - 0.4, OUT)
        r.px(-4, cy + 1, GOLD[2]); r.px(4, cy - 1, GOLD[2])
        cv.nl = False

def build_npc():
    A = {}
    for k, n in ((0, 'NPC_PASSEUR'), (1, 'NPC_ORACLE')):
        A[n] = []
        for f in range(2):
            cv = Cv(24, 32); npc_draw(cv, k, f); A[n].append(finish(cv))
    return A

def build_new_chars():
    A = {}
    for f in (build_gren, build_shld, build_toxic, build_bat, build_npc):
        A.update(f())
    return A
