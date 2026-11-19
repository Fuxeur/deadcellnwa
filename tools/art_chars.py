"""art_chars.py - heros original (le Veilleur), ennemis et boss. Tout est dessine par code."""
import math
from art_core import *

HCX, HGY, HW, HH = 14, 30, 36, 32       # humanoides : canevas 36x32, pieds a la ligne 30

def hand(r, x, y, c=None):
    r.rect(x - 0.5, y - 0.5, x + 0.5, y + 0.5, c if c is not None else LEATH[2])

# =============================================================== HEROS (24x24)
def hero_draw(cv, P, wpn=None, wang=0, glow=0.0, arrow=False, scarfw=0):
    r = Rig(cv, HCX, HGY, 1.0)
    L, T, tw = 7, 6, 3
    bob = P.get('bob', 0); lean = P.get('lean', 0)
    hip = (P.get('hipdx', 0), -L + bob)
    sh = (lean, -L - T + bob)
    hd = (lean + 1 + P.get('hx', 0), -L - T - 4 + bob + P.get('hy', 0))
    ff = P.get('ff', (2, 0)); bf = P.get('bf', (-2, 0))
    fh = P.get('fh', (3, 5)); bh = P.get('bh', (-3, 5))
    bsh = (sh[0] - 1, sh[1] + 1); fsh = (sh[0] + 1, sh[1] + 1)
    # bras / jambe arriere
    arm(r, bsh, bh, CLOAK[0]); hand(r, bsh[0] + bh[0], bsh[1] + bh[1])
    leg(r, hip, bf, LEATH[0], STEEL[0])
    # pan de cape + echarpe qui flotte
    r.ell(sh[0] - tw, sh[1] + 4, 2.2, 5, CLOAK[0])
    r.line(sh[0] - tw + 1, sh[1] + 1, sh[0] - tw - 6, sh[1] + 2 + scarfw, SCARF[0], 2)
    r.line(sh[0] - tw + 1, sh[1] + 1, sh[0] - tw - 5, sh[1] + 1 + scarfw, SCARF[1], 1)
    # torse
    r.rect(sh[0] - tw, sh[1], sh[0] + tw, hip[1], CLOAK[1])
    r.rect(sh[0] - tw, hip[1] - 1, sh[0] + tw, hip[1] - 1, LEATH[1])
    r.rect(sh[0] + 1, hip[1] - 1, sh[0] + 2, hip[1] - 1, GOLD[1])
    if wpn is None:                                   # epee rangee dans le dos
        r.line(sh[0] - 2, sh[1], sh[0] - 4, sh[1] - 5, LEATH[1], 1)
        r.px(sh[0] - 4, sh[1] - 6, GOLD[1]); r.px(sh[0] - 5, sh[1] - 4, GOLD[1])
    leg(r, hip, ff, LEATH[1], STEEL[1])
    # capuche + masque pale + oeil braise
    r.ell(hd[0], hd[1], 3.8, 4.0, CLOAK[1])
    r.ell(hd[0] - 2.5, hd[1] + 1.8, 2.0, 3.0, CLOAK[0])
    r.ell(hd[0] + 1.6, hd[1] + 0.6, 2.2, 2.8, BONE[1])
    cv.nl = True
    r.px(hd[0] + 2.6, hd[1] + 0.1, SCARF[2])
    r.px(hd[0] + 2.6, hd[1] + 1.4, OUT)
    cv.nl = False
    r.rect(sh[0] - tw, sh[1] - 1, sh[0] + tw, sh[1], SCARF[1])     # col
    # bras avant + arme
    arm(r, fsh, fh, CLOAK[2])
    fx, fy = fsh[0] + fh[0], fsh[1] + fh[1]
    if wpn:
        if arrow:
            r.line(bsh[0] + bh[0], bsh[1] + bh[1], fx + 3, fy, BONE[2], 1)
            r.px(fx + 3.5, fy, STEEL[2])
        weapon(r, fx, fy, wang, wpn, glow)
    hand(r, fx, fy)

def curl(cv, ang, scarf=True):
    """boule roulee (esquive / double saut) : tete et pieds tournent autour du centre."""
    r = Rig(cv, HCX, HGY, 1.0)
    cx, cy = 0, -9
    a = math.radians(ang)
    ca, sa = math.cos(a), math.sin(a)
    if scarf:
        r.line(cx - 4 * ca, cy - 4 * sa, cx - 9 * ca, cy - 9 * sa + 1, SCARF[0], 2)
    r.ell(cx, cy, 6, 6, CLOAK[1])
    r.ell(cx + 1.5 * sa, cy - 1.5 * ca, 3.6, 3.6, CLOAK[0])
    hx, hy = cx + 4.6 * ca, cy + 4.6 * sa
    r.ell(hx, hy, 3.2, 3.2, CLOAK[2])
    r.ell(hx + 0.9 * ca, hy + 0.9 * sa, 1.7, 1.7, BONE[1])
    cv.nl = True; r.px(hx + 1.4 * ca, hy + 1.4 * sa, SCARF[2]); cv.nl = False
    for k_ in (-0.5, 0.5):
        fx, fy = cx - 5 * ca + k_ * 4 * -sa, cy - 5 * sa + k_ * 4 * ca
        r.rect(fx - 1, fy - 1, fx + 0.5, fy + 0.5, STEEL[1])
    r.rect(cx - 6, cy + 5.5, cx - 6, cy + 5.5, SCARF[1])

def lying(cv, k=0):
    r = Rig(cv, HCX, HGY, 1.0)
    r.ell(-1, -3, 9, 2.6, CLOAK[1])
    r.ell(-3, -4.5, 5, 1.6, CLOAK[0])
    r.ell(7, -3, 3, 2.8, CLOAK[1])
    r.ell(8, -3, 1.7, 1.9, BONE[1])
    r.line(-8, -2, -11, -1 + k, SCARF[1], 2)
    r.rect(-10, -3, -8, -1, STEEL[0])
    r.line(2, -1, 11, -1, STEEL[2], 1)                 # epee tombee
    r.px(1, -1, GOLD[1])

def build_hero():
    A = {}
    def mk(seq, w=HW, h=HH):
        out = []
        for P, k in seq:
            cv = Cv(w, h); hero_draw(cv, P, **k); out.append(finish(cv))
        return out
    A['HERO_IDLE'] = mk([(dict(bob=0), dict(scarfw=0)), (dict(bob=1, fh=(3, 6), bh=(-3, 6)), dict(scarfw=1))])
    A['HERO_RUN'] = mk([(runpose(i, 6), dict(scarfw=(-1) ** i)) for i in range(6)])
    A['HERO_JUMP'] = mk([(dict(lean=1, ff=(3, -3), bf=(-2, -1), fh=(4, -3), bh=(-3, -2)), dict(scarfw=-2))])
    A['HERO_FALL'] = mk([(dict(ff=(2, -1), bf=(-2, 0), fh=(4, -2), bh=(-4, -3), hy=1), dict(scarfw=2))])
    out = []
    for ang in (60, 200):
        cv = Cv(HW, HH); curl(cv, ang); out.append(finish(cv))
    A['HERO_FLIP'] = out
    A['HERO_SWORD'] = mk([
        (dict(lean=-1, fh=(-3, -5), bh=(-3, 3), ff=(3, 0), bf=(-3, 0)), dict(wpn='sword', wang=-130, scarfw=1)),
        (dict(lean=2, fh=(5, -2), bh=(0, 2), ff=(4, 0), bf=(-4, 0)), dict(wpn='sword', wang=-8, scarfw=-1)),
        (dict(lean=2, bob=1, fh=(5, 3), bh=(0, 3), ff=(4, 0), bf=(-4, 0)), dict(wpn='sword', wang=50))])
    A['HERO_DAGGER'] = mk([
        (dict(lean=1, fh=(3, -1), bh=(-2, 3), ff=(3, 0), bf=(-3, 0)), dict(wpn='dagger', wang=-15)),
        (dict(lean=3, ff=(5, 0), bf=(-3, 0), fh=(7, -1), bh=(-1, 3)), dict(wpn='dagger', wang=0, scarfw=-1))])
    A['HERO_HAMMER'] = mk([
        (dict(lean=-1, fh=(2, -4), bh=(0, -3), ff=(2, 0), bf=(-2, 0)), dict(wpn='hammer', wang=-95)),
        (dict(lean=3, bob=2, fh=(6, 1), bh=(4, 1), ff=(4, 0), bf=(-4, 0)), dict(wpn='hammer', wang=75, scarfw=-1)),
        (dict(lean=2, bob=1, fh=(5, 3), bh=(3, 3), ff=(4, 0), bf=(-3, 0)), dict(wpn='hammer', wang=82))])
    A['HERO_BOW'] = mk([
        (dict(fh=(7, -1), bh=(0, -1)), dict(wpn='bow', wang=0, arrow=True)),
        (dict(lean=-1, fh=(7, -1), bh=(1, -1)), dict(wpn='bow', wang=0, scarfw=1))])
    A['HERO_CAST'] = mk([
        (dict(fh=(5, -4), bh=(-2, 2)), dict(wpn='staff', wang=-62, glow=0.3)),
        (dict(lean=1, fh=(5, -2), bh=(-1, 2)), dict(wpn='staff', wang=-30, glow=1.0, scarfw=-1))])
    A['HERO_ROLL'] = []
    for ang in (0, 90, 180, 270):
        cv = Cv(HW, HH); curl(cv, ang); A['HERO_ROLL'].append(finish(cv))
    A['HERO_HURT'] = mk([(dict(lean=-3, hy=1, fh=(-3, -5), bh=(-4, -2), ff=(1, 0), bf=(-4, 0)), dict(scarfw=2))])
    out = mk([(dict(bob=4, lean=-2, ff=(3, 0), bf=(-1, 0), fh=(2, 3), bh=(-3, 3), hy=1), dict(scarfw=2))])
    for k in (0, 1):
        cv = Cv(HW, HH); lying(cv, k); out.append(finish(cv))
    A['HERO_DEAD'] = out
    return A

# =============================================================== GEOLIER (24x24)
def jail_draw(cv, P, club=True, cang=60):
    r = Rig(cv, HCX, HGY, 1.0)
    L, T, tw = 7, 7, 4
    bob = P.get('bob', 0); lean = P.get('lean', 0)
    hip = (0, -L + bob); sh = (lean, -L - T + bob)
    hd = (lean + 1, -L - T - 3 + bob + P.get('hy', 0))
    ff = P.get('ff', (2, 0)); bf = P.get('bf', (-2, 0))
    fh = P.get('fh', (3, 5)); bh = P.get('bh', (-3, 5))
    bsh = (sh[0] - 2, sh[1] + 1); fsh = (sh[0] + 2, sh[1] + 1)
    arm(r, bsh, bh, ACC[0]); hand(r, bsh[0] + bh[0], bsh[1] + bh[1])
    leg(r, hip, bf, LEATH[0], STEEL[0], 2, 2)
    r.rect(sh[0] - tw, sh[1], sh[0] + tw, hip[1] + 1, STEEL[0])
    r.rect(sh[0] - 2, sh[1] + 1, sh[0] + 2, hip[1] + 2, ACC[1])
    r.rect(sh[0] - tw, hip[1] - 1, sh[0] + tw, hip[1] - 1, LEATH[1])
    r.px(sh[0], hip[1] - 1, GOLD[1])
    leg(r, hip, ff, LEATH[1], STEEL[1], 2, 2)
    r.ell(sh[0] - tw + 0.5, sh[1] + 0.5, 2.2, 1.8, STEEL[1])
    r.ell(sh[0] + tw - 0.5, sh[1] + 0.5, 2.2, 1.8, STEEL[1])
    # casque seau
    r.ell(hd[0], hd[1], 3.6, 3.7, STEEL[1])
    r.rect(hd[0] - 3, hd[1] - 4, hd[0] + 2, hd[1] - 3, STEEL[0])
    cv.nl = True
    r.rect(hd[0] + 0.3, hd[1] - 0.3, hd[0] + 3, hd[1] + 0.4, OUT)
    r.px(hd[0] + 2, hd[1], RED[2])
    cv.nl = False
    r.px(hd[0] - 1, hd[1] + 2, STEEL[2]); r.px(hd[0] + 1, hd[1] - 2, STEEL[2])
    r.ell(hd[0] - 0.5, hd[1] - 4.5, 1.3, 1.3, RED[1])            # cimier
    arm(r, fsh, fh, ACC[2]); fx, fy = fsh[0] + fh[0], fsh[1] + fh[1]
    if club:
        weapon(r, fx, fy, cang, 'club')
    hand(r, fx, fy)

def build_jailer():
    A = {}
    def mk(seq):
        out = []
        for P, k in seq:
            cv = Cv(HW, HH); jail_draw(cv, P, **k); out.append(finish(cv))
        return out
    A['JAIL_IDLE'] = mk([(dict(), dict(cang=70))])
    A['JAIL_WALK'] = mk([(runpose(i, 4, 3, 2, 2, 1), dict(cang=70)) for i in range(4)])
    A['JAIL_WIND'] = mk([(dict(lean=-1, fh=(1, -5), bh=(-2, -3), ff=(2, 0), bf=(-3, 0)), dict(cang=-100))])
    A['JAIL_SWING'] = mk([(dict(lean=3, bob=1, fh=(6, 3), bh=(2, 3), ff=(4, 0), bf=(-4, 0)), dict(cang=40))])
    A['JAIL_HURT'] = mk([(dict(lean=-3, hy=1, fh=(-3, -4), bh=(-4, -2), ff=(1, 0), bf=(-4, 0)), dict(cang=-120))])
    out = mk([(dict(bob=4, lean=-2, ff=(3, 0), bf=(-1, 0), fh=(2, 3), bh=(-3, 3), hy=1), dict(club=False))])
    cv = Cv(HW, HH); r = Rig(cv, HCX, HGY)
    r.ell(-1, -3, 9, 2.8, STEEL[0]); r.ell(-1, -3.5, 6, 2, ACC[1]); r.ell(8, -3, 3.2, 3, STEEL[1])
    r.rect(-10, -3, -8, -1, LEATH[0]); r.line(2, -1, 11, -1, LEATH[2], 2)
    out.append(finish(cv))
    A['JAIL_DEAD'] = out
    return A

# =============================================================== ARBALETRIER (24x24)
def xbow_draw(cv, P, ang=0, bolt=False, fire=False):
    r = Rig(cv, HCX, HGY, 1.0)
    L, T, tw = 7, 6, 3
    bob = P.get('bob', 0); lean = P.get('lean', 0)
    hip = (0, -L + bob); sh = (lean, -L - T + bob)
    hd = (lean + 1, -L - T - 4 + bob + P.get('hy', 0))
    ff = P.get('ff', (2, 0)); bf = P.get('bf', (-2, 0))
    fh = P.get('fh', (5, 2)); bh = P.get('bh', (2, 3))
    bsh = (sh[0] - 1, sh[1] + 1); fsh = (sh[0] + 1, sh[1] + 1)
    arm(r, bsh, bh, ACC[0]); hand(r, bsh[0] + bh[0], bsh[1] + bh[1])
    leg(r, hip, bf, LEATH[0], LEATH[0])
    r.rect(sh[0] - tw, sh[1], sh[0] + tw, hip[1] + 3, ACC[1])
    r.rect(sh[0] - tw + 1, hip[1] + 3, sh[0] + tw - 1, hip[1] + 4, ACC[0])
    r.rect(sh[0] - tw, hip[1] - 1, sh[0] + tw, hip[1] - 1, LEATH[1])
    leg(r, hip, ff, LEATH[1], LEATH[1])
    r.ell(hd[0], hd[1], 3.8, 4.0, ACC[1])
    r.ell(hd[0] - 2.5, hd[1] + 1.8, 2.0, 3.0, ACC[0])
    r.ell(hd[0] + 1.4, hd[1] + 0.6, 2.2, 2.6, LEATH[0])
    cv.nl = True; r.px(hd[0] + 2.2, hd[1] + 0.3, RED[2]); r.px(hd[0] + 2.2, hd[1] + 1.5, RED[2]); cv.nl = False
    arm(r, fsh, fh, ACC[2]); fx, fy = fsh[0] + fh[0], fsh[1] + fh[1]
    weapon(r, fx - 1, fy, ang, 'xbow')
    if bolt:
        a = math.radians(ang)
        r.line(fx + 2, fy, fx + 2 + 6 * math.cos(a), fy + 6 * math.sin(a), LEATH[2], 1)
        r.px(fx + 8.5 * math.cos(a) + 2, fy + 8.5 * math.sin(a), STEEL[2])
    if fire:
        cv.nl = True
        r.ell(fx + 11, fy - 1, 1.6, 1.6, FIRE[3]); r.px(fx + 13, fy - 1, FIRE[2])
        cv.nl = False
    hand(r, fx, fy)

def build_xbow():
    A = {}
    def mk(seq):
        out = []
        for P, k in seq:
            cv = Cv(HW, HH); xbow_draw(cv, P, **k); out.append(finish(cv))
        return out
    A['XBOW_IDLE'] = mk([(dict(), dict(ang=0))])
    A['XBOW_WALK'] = mk([(dict(runpose(i, 4, 3, 2, 1, 1), fh=(5, 2), bh=(2, 3)), dict(ang=5)) for i in range(4)])
    A['XBOW_AIM'] = mk([(dict(lean=-1, fh=(6, -2), bh=(3, 0)), dict(ang=0, bolt=True))])
    A['XBOW_SHOOT'] = mk([(dict(lean=-2, fh=(5, -2), bh=(2, 0)), dict(ang=-6, fire=True))])
    A['XBOW_HURT'] = mk([(dict(lean=-3, hy=1, fh=(1, -1), bh=(-3, 1), ff=(1, 0), bf=(-4, 0)), dict(ang=-30))])
    out = mk([(dict(bob=4, lean=-2, ff=(3, 0), bf=(-1, 0), fh=(1, 3), bh=(-2, 3), hy=1), dict(ang=80))])
    cv = Cv(HW, HH); r = Rig(cv, HCX, HGY)
    r.ell(-1, -3, 9, 2.8, ACC[1]); r.ell(-3, -4, 5, 1.8, ACC[0]); r.ell(8, -3, 3.2, 3, ACC[2])
    r.rect(-10, -3, -8, -1, LEATH[0]); weapon(r, 2, -1, 0, 'xbow')
    out.append(finish(cv))
    A['XBOW_DEAD'] = out
    return A

# =============================================================== RAT (24x16)
def rat_draw(cv, mode='run', ph=0):
    t = ph * math.pi / 2
    if mode == 'run':
        bob = R(0.8 * abs(math.sin(t))); by = 9 - bob
        cv.line(4, by, 0, by - 3 + (1 if ph % 2 else 0), SKIN[0], 1)
        for lx, off in ((15, 0), (13, math.pi), (7, math.pi), (5, 0)):
            fx = lx + 3 * math.sin(t + off); fy = 14 - 2 * max(0, math.cos(t + off))
            cv.line(lx, by + 2, fx, fy, ACC[0], 1)
            cv.px(fx + 1, 14, ACC[0])
        cv.ell(10, by, 6.5, 3.5, ACC[1]); cv.ell(17.5, by - 0.5, 4, 3.2, ACC[1])
        cv.ell(21.5, by + 0.5, 2, 1.4, ACC[2]); cv.ell(16, by - 3.8, 1.6, 1.8, SKIN[1])
        cv.nl = True; cv.px(19, by - 1.5, RED[2]); cv.px(22, by + 1.8, BONE[2]); cv.nl = False
    elif mode == 'leap':
        cv.line(3, 8, 0, 4, SKIN[0], 1)
        for lx, fx, fy in ((16, 22, 11), (14, 20, 12), (7, 1, 12), (5, 0, 11)):
            cv.line(lx, 9, fx, fy, ACC[0], 1)
        cv.ell(11, 8, 7.5, 3, ACC[1]); cv.ell(18.5, 6.5, 4, 3, ACC[1]); cv.ell(22, 7.5, 1.8, 1.3, ACC[2])
        cv.ell(17, 3.5, 1.5, 1.7, SKIN[1])
        cv.nl = True; cv.px(20, 5.5, RED[2]); cv.px(22.5, 9, BONE[2]); cv.px(21.5, 9, BONE[2]); cv.nl = False
    elif mode == 'hurt':
        cv.line(4, 11, 0, 9, SKIN[0], 1)
        for lx in (15, 13, 7, 5):
            cv.line(lx, 12, lx + 1, 14, ACC[0], 1)
        cv.ell(10, 10.5, 7, 3, ACC[1]); cv.ell(17, 11, 4, 2.6, ACC[1]); cv.ell(21, 11.5, 1.8, 1.2, ACC[2])
        cv.nl = True; cv.px(18, 10, BONE[2]); cv.px(19, 11, BONE[2]); cv.nl = False
    elif mode == 'dead0':
        cv.ell(12, 11, 7.5, 3, ACC[1]); cv.ell(12, 12, 5, 1.4, SKIN[2]); cv.ell(19, 11.5, 3.2, 2.4, ACC[1])
        for lx in (9, 12, 15):
            cv.line(lx, 9, lx + 1, 6, ACC[0], 1)
        cv.line(4, 12, 0, 12, SKIN[0], 1)
    else:
        cv.ell(12, 12.5, 9, 1.6, ACC[0]); cv.ell(19, 12.5, 3, 1.5, ACC[1]); cv.line(3, 13, 0, 13, SKIN[0], 1)

def build_rat():
    A = {}
    A['RAT_RUN'] = []
    for i in range(4):
        cv = Cv(28, 16); cv.ox = 2; rat_draw(cv, 'run', i); A['RAT_RUN'].append(finish(cv))
    for n, m in (('RAT_LEAP', 'leap'), ('RAT_HURT', 'hurt')):
        cv = Cv(28, 16); cv.ox = 2; rat_draw(cv, m); A[n] = [finish(cv)]
    A['RAT_DEAD'] = []
    for m in ('dead0', 'dead1'):
        cv = Cv(28, 16); cv.ox = 2; rat_draw(cv, m); A['RAT_DEAD'].append(finish(cv))
    return A

# =============================================================== BRUTE (32x32)
def brute_draw(cv, P, club_ang=70, shield=True, windup=False):
    r = Rig(cv, 20, 42, 1.3)
    L, T, tw = 5, 7, 5
    bob = P.get('bob', 0); lean = P.get('lean', 0)
    hip = (0, -L + bob); sh = (lean, -L - T + bob)
    hd = (lean + 1, -L - T - 2.5 + bob + P.get('hy', 0))
    ff = P.get('ff', (2, 0)); bf = P.get('bf', (-2, 0))
    fh = P.get('fh', (3, 3)); bh = P.get('bh', (-3, 4))
    bsh = (sh[0] - tw + 1, sh[1] + 1.5); fsh = (sh[0] + tw - 1, sh[1] + 1.5)
    arm(r, bsh, bh, ACC[0], 3)
    leg(r, hip, bf, LEATH[0], STEEL[0], 3, 2)
    weapon(r, bsh[0] + bh[0], bsh[1] + bh[1], club_ang, 'club')
    r.rect(sh[0] - tw, sh[1], sh[0] + tw, hip[1] + 2, STEEL[0])
    r.rect(sh[0] - 3, sh[1] + 1, sh[0] + 3, hip[1] + 3, ACC[1])
    r.rect(sh[0] - tw, hip[1] - 1, sh[0] + tw, hip[1], LEATH[1])
    r.rect(sh[0] - 1, hip[1] - 1, sh[0] + 1, hip[1], GOLD[1])
    leg(r, hip, ff, LEATH[1], STEEL[1], 3, 2)
    r.ell(sh[0] - tw, sh[1] + 1, 2.8, 2.2, STEEL[1]); r.ell(sh[0] + tw, sh[1] + 1, 2.8, 2.2, STEEL[1])
    r.ell(hd[0], hd[1], 3.4, 3.4, STEEL[1])
    r.line(hd[0] - 2.5, hd[1] - 2, hd[0] - 5, hd[1] - 5, BONE[1], 1)
    r.line(hd[0] + 2.5, hd[1] - 2, hd[0] + 5, hd[1] - 5, BONE[1], 1)
    cv.nl = True; r.rect(hd[0] + 0.3, hd[1] - 0.3, hd[0] + 3, hd[1] + 0.3, OUT); r.px(hd[0] + 2, hd[1], RED[2]); cv.nl = False
    arm(r, fsh, fh, ACC[2], 3); fx, fy = fsh[0] + fh[0], fsh[1] + fh[1]
    if shield:
        sx = fx + 1.5
        r.rect(sx - 2, fy - 7, sx + 1.5, fy + 5, STEEL[1])
        r.rect(sx - 2, fy - 7, sx - 2, fy + 5, STEEL[2])
        r.rect(sx - 1, fy - 1, sx + 0.5, fy + 1, GOLD[1])
        r.rect(sx - 2, fy + 5, sx + 1.5, fy + 5, STEEL[0])

def build_brute():
    A = {}
    def mk(seq):
        out = []
        for P, k in seq:
            cv = Cv(44, 44); brute_draw(cv, P, **k); out.append(finish(cv))
        return out
    A['BRUTE_IDLE'] = mk([(dict(), dict())])
    A['BRUTE_WALK'] = mk([(runpose(i, 4, 2.5, 1, 1, 1), dict()) for i in range(4)])
    A['BRUTE_WIND'] = mk([(dict(lean=-1, bh=(-1, -9), fh=(2, 3)), dict(club_ang=-105))])
    A['BRUTE_SLAM'] = mk([(dict(lean=3, bob=2, bh=(5, 4), fh=(4, 4)), dict(club_ang=60))])
    A['BRUTE_HURT'] = mk([(dict(lean=-2, hy=1, bh=(-3, -2), fh=(-1, 2), bf=(-3, 0)), dict(club_ang=-115))])
    out = mk([(dict(bob=5, lean=-3, ff=(4, 0), bf=(-1, 0), bh=(-3, 3), fh=(1, 3), hy=1), dict(shield=False, club_ang=100))])
    cv = Cv(44, 44); r = Rig(cv, 20, 42, 1.3)
    r.ell(-1, -3, 9, 2.8, STEEL[0]); r.ell(-1, -3.5, 6, 2, ACC[1]); r.ell(8, -3, 3.4, 3.2, STEEL[1])
    r.rect(-10, -3, -8, -1, LEATH[0]); r.line(2, -1, 11, -1, LEATH[2], 2)
    out.append(finish(cv))
    A['BRUTE_DEAD'] = out
    return A

# =============================================================== SLIME (20x16)
def slime_draw(cv, sx=1.0, sy=1.0, mode='n'):
    ry = 6.2 * sy; rx = 8 * sx
    if mode == 'dead1':
        cv.ell(10, 13.2, 8.5, 1.5, GREEN[1]); cv.px(5, 11, GREEN[2]); cv.px(14, 10.5, GREEN[2]); return
    if mode == 'dead0':
        cv.ell(10, 12, 8.2, 3.2, GREEN[1]); cv.ell(8, 10.6, 2, 1.1, GREEN[2]); cv.px(4, 8.5, GREEN[2]); return
    cv.ell(10, 14.2 - ry, rx, ry, GREEN[1])
    cv.ell(7.5, 14.2 - ry - 1.2, 2.2, 1.4, GREEN[2])
    ey = 14.2 - ry - 0.2
    cv.nl = True
    if mode == 'hurt':
        cv.line(6, ey, 9, ey + 1, OUT, 1); cv.line(11, ey + 1, 14, ey, OUT, 1)
    else:
        for ex in (7.5, 12.5):
            cv.ell(ex, ey, 1.7, 1.9, BONE[2]); cv.px(ex + 0.6, ey + 0.3, OUT)
    cv.px(10, ey + 3, GREEN[0]); cv.px(11, ey + 3, GREEN[0])
    cv.nl = False

def build_slime():
    A = {}
    spec = [('SLIME_IDLE', [(1.0, 1.0, 'n'), (1.1, 0.88, 'n')]), ('SLIME_HOP', [(0.78, 1.22, 'n')]),
            ('SLIME_HURT', [(1.15, 0.8, 'hurt')]), ('SLIME_DEAD', [(1, 1, 'dead0'), (1, 1, 'dead1')])]
    for n, fl in spec:
        A[n] = []
        for sx, sy, m in fl:
            cv = Cv(22, 18); cv.ox = 1; cv.oy = 2; slime_draw(cv, sx, sy, m); A[n].append(finish(cv))
    return A

# =============================================================== GOBEUR (48x40)
def gob_draw(cv, sx=1.0, sy=1.0, lift=0, mouth=0, mode='n'):
    gy = 42 - lift
    if mode == 'dead':
        cv.ell(26, 36, 22, 3, GREEN[1]); cv.ell(26, 35, 15, 1.6, GREEN[2])
        cv.ell(17, 37, 4, 2, GREEN[2]); cv.px(36, 36, GREEN[2])
        cv.ell(32, 37, 3, 2.2, BONE[1]); cv.px(33, 37, OUT); cv.px(31, 37, OUT)
        return
    bh = 11 * sy
    for lx in (10, 38):
        cv.ell(26 + (lx - 24) * sx, gy - 3, 5 * sx, 3.2, GREEN[0])
    cv.ell(26, gy - bh, 19 * sx, bh, GREEN[1])
    cv.ell(26, gy - bh * 0.4, 13 * sx, bh * 0.55, GREEN[2])
    hump = gy - 2 * bh + 3
    cv.ell(26, hump, 13 * sx, 6.5 * sy, GREEN[1])
    for wx, wy in ((12, -3), (35, -2), (28, -6), (18, 0)):
        cv.px(26 + (wx - 24) * sx, hump + wy, GREEN[0])
    cv.nl = True
    for ex in (15, 33):
        exx = 26 + (ex - 24) * sx
        cv.ell(exx, hump - 4, 4.2, 4.2, GOLD[2]); cv.ell(exx, hump - 4, 4.2, 4.2, GOLD[2])
        if mode == 'hurt':
            cv.line(exx - 3, hump - 4, exx + 3, hump - 4, OUT, 1)
        else:
            cv.rect(exx + 0, hump - 6, exx + 1, hump - 2, OUT)
    cv.nl = False
    my = hump + 6
    if mouth:
        cv.ell(26, my + 2, 12 * sx, 4.5, RED[0])
        cv.ell(26, my + 3, 7 * sx, 2.2, RED[1])
        for tx in range(-10, 11, 4):
            cv.px(26 + tx * sx, my - 1, BONE[2])
        cv.ell(26, my + 5, 3, 2.4, GREEN[2])           # glob de poison
    else:
        cv.line(26 - 14 * sx, my + 1, 26 + 14 * sx, my + 1, GREEN[0], 1)
        cv.line(26 - 14 * sx, my + 1, 24 - 16 * sx, my - 1, GREEN[0], 1)
        cv.line(26 + 14 * sx, my + 1, 26 + 16 * sx, my - 1, GREEN[0], 1)
        for tx in (-8, -2, 4, 10):
            cv.px(26 + tx * sx, my + 2, BONE[2])

def build_gob():
    A = {}
    spec = [('GOB_IDLE', [dict(), dict(sx=1.04, sy=0.93)]), ('GOB_HOP', [dict(sx=0.82, sy=1.25, lift=7)]),
            ('GOB_SPIT', [dict(sx=0.95, sy=1.05, mouth=1)]), ('GOB_SLAM', [dict(sx=1.22, sy=0.7)]),
            ('GOB_HURT', [dict(sx=1.08, sy=0.9, mode='hurt')]), ('GOB_DEAD', [dict(sx=1.1, sy=0.55, mode='hurt'), dict(mode='dead')])]
    for n, fl in spec:
        A[n] = []
        for kw in fl:
            cv = Cv(52, 44); gob_draw(cv, **kw); A[n].append(finish(cv))
    return A

# =============================================================== ROI FELE (40x48)
def king_draw(cv, P, ang=70, two=True, charge=False):
    r = Rig(cv, 22, 62, 1.9)
    L, T, tw = 6.2, 6.5, 3.2
    bob = P.get('bob', 0); lean = P.get('lean', 0)
    hip = (0, -L + bob); sh = (lean, -L - T + bob)
    hd = (lean + 0.8, -L - T - 3.6 + bob + P.get('hy', 0))
    ff = P.get('ff', (2, 0)); bf = P.get('bf', (-2, 0))
    fh = P.get('fh', (3, 3)); bh = P.get('bh', (-2, 3))
    bsh = (sh[0] - 1.5, sh[1] + 1); fsh = (sh[0] + 1.5, sh[1] + 1)
    # cape dechiree
    r.ell(sh[0] - 2.5, sh[1] + 6, 4.2, 7.5, ACCS[4][0])
    for i, dx in enumerate((-6, -4, -2, 0)):
        r.rect(sh[0] + dx - 0.5, sh[1] + 11 - (i % 2), sh[0] + dx + 0.5, sh[1] + 13 - (i % 2), ACCS[4][0])
    arm(r, bsh, bh, BONE[0], 1.4); hand(r, bsh[0] + bh[0], bsh[1] + bh[1], BONE[1])
    leg(r, hip, bf, BONE[0], STEEL[0], 1.4, 2)
    r.rect(sh[0] - tw, sh[1], sh[0] + tw, hip[1] + 1, STEEL[0])
    for i in range(3):
        r.rect(sh[0] - 2, sh[1] + 1.2 + i * 1.7, sh[0] + 2, sh[1] + 1.6 + i * 1.7, BONE[1])
    r.rect(sh[0], sh[1], sh[0], hip[1], BONE[1])
    r.rect(sh[0] - tw, hip[1] - 1, sh[0] + tw, hip[1] - 1, GOLD[1])
    leg(r, hip, ff, BONE[1], STEEL[1], 1.4, 2)
    r.ell(sh[0] - tw, sh[1], 2.0, 1.5, STEEL[1]); r.ell(sh[0] + tw, sh[1], 2.0, 1.5, STEEL[1])
    r.px(sh[0] - tw - 1, sh[1] - 2, GOLD[2]); r.px(sh[0] + tw + 1, sh[1] - 2, GOLD[2])
    # crane + couronne
    r.ell(hd[0], hd[1], 3.0, 3.1, BONE[1])
    r.rect(hd[0] - 1.8, hd[1] + 2, hd[0] + 1.8, hd[1] + 3.2, BONE[0])
    cv.nl = True
    r.ell(hd[0] + 0.7, hd[1] - 0.3, 1.0, 1.0, OUT); r.ell(hd[0] - 1.4, hd[1] - 0.3, 0.9, 0.9, OUT)
    r.px(hd[0] + 0.9, hd[1] - 0.3, RED[2] if not charge else FIRE[3])
    cv.nl = False
    r.rect(hd[0] - 3, hd[1] - 4.5, hd[0] + 3, hd[1] - 3.4, GOLD[1])
    for dx in (-3, -1, 1, 3):
        r.rect(hd[0] + dx - 0.4, hd[1] - 6, hd[0] + dx + 0.4, hd[1] - 4.5, GOLD[2])
    arm(r, fsh, fh, BONE[1], 1.4); fx, fy = fsh[0] + fh[0], fsh[1] + fh[1]
    if two:
        a = math.radians(ang)
        hand(r, fx - 2.2 * math.cos(a), fy - 2.2 * math.sin(a), BONE[2])
    weapon(r, fx, fy, ang, 'greatsword')
    hand(r, fx, fy, BONE[2])

def build_king():
    A = {}
    def mk(seq):
        out = []
        for P, k in seq:
            cv = Cv(64, 64); king_draw(cv, P, **k); out.append(finish(cv))
        return out
    A['KING_IDLE'] = mk([(dict(fh=(2, 5), bh=(0, 4)), dict(ang=78)), (dict(bob=1, fh=(2, 6), bh=(0, 5)), dict(ang=82))])
    A['KING_WALK'] = mk([(dict(runpose(i, 4, 2.5, 1, 1, 1), fh=(2, 5), bh=(0, 4)), dict(ang=80)) for i in range(4)])
    A['KING_CWIND'] = mk([(dict(lean=-1, ff=(3, 0), bf=(-3, 0), fh=(0, 0), bh=(-2, 1)), dict(ang=-140, charge=True))])
    A['KING_DASH'] = mk([(dict(lean=4, bob=1, ff=(5, -1), bf=(-5, 0), fh=(1, 1), bh=(0, 2)), dict(ang=-6, charge=True))])
    A['KING_SWIND'] = mk([(dict(lean=-1, ff=(3, 0), bf=(-3, 0), fh=(-1, -6), bh=(-2, -3)), dict(ang=-125))])
    A['KING_SWEEP'] = mk([(dict(lean=3, bob=1, ff=(5, 0), bf=(-4, 0), fh=(2, -1), bh=(0, 0)), dict(ang=-2))])
    A['KING_SLWIND'] = mk([(dict(lean=-1, fh=(0, -6), bh=(-1, -4)), dict(ang=-98))])
    A['KING_SLAM'] = mk([(dict(lean=3, bob=2, ff=(4, 0), bf=(-3, 0), fh=(5, 2), bh=(3, 2)), dict(ang=80))])
    A['KING_HURT'] = mk([(dict(lean=-2, hy=1, fh=(0, 0), bh=(-2, 2), bf=(-3, 0)), dict(ang=-118))])
    out = mk([(dict(bob=5, lean=-2, ff=(3, 0), bf=(0, 0), fh=(0, 3), bh=(-2, 3), hy=1), dict(ang=110)),
              (dict(bob=9, lean=-4, ff=(4, 0), bf=(1, 0), fh=(-3, 3), bh=(-4, 3), hy=2), dict(ang=150))])
    cv = Cv(64, 64); r = Rig(cv, 22, 62, 1.9)
    r.ell(0, -2.2, 10, 1.6, STEEL[0]); r.ell(-1, -2.4, 6, 1.4, ACCS[4][1]); r.ell(7, -2.5, 2.4, 2, BONE[1])
    r.ell(7, -3.8, 2.1, 1.1, GOLD[1]); r.line(-8, -1.2, 9, -1.2, STEEL[2], 1); r.ell(-9, -1.6, 1.2, 1, BONE[0])
    out.append(finish(cv))
    A['KING_DEAD'] = out
    return A

def build_all_chars():
    A = {}
    for f in (build_hero, build_jailer, build_xbow, build_rat, build_brute, build_slime, build_gob, build_king):
        A.update(f())
    return A
