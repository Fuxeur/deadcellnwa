"""art_core.py - palette, canevas pixel-art, rig de personnages (art 100 % original, genere par code)."""
import math

def R(v):
    return int(math.floor(v + 0.5))

# ------------------------------------------------------------------ palette
PAL = [(255, 0, 255)]          # indice 0 = transparent
RAMPS = {}

def _hex(h):
    h = h.lstrip('#')
    return tuple(int(h[i:i + 2], 16) for i in (0, 2, 4))

def ramp(name, *hs):
    idx = []
    for h in hs:
        PAL.append(_hex(h))
        idx.append(len(PAL) - 1)
    RAMPS[name] = idx
    return idx

OUT   = ramp('OUT',   '#0b0914')[0]
BONE  = ramp('BONE',  '#8a8270', '#c8bfa6', '#efe7cf')
CLOAK = ramp('CLOAK', '#1b2140', '#2f3c66', '#5068a0')
SCARF = ramp('SCARF', '#8f2410', '#e05a1a', '#ffa63c')
STEEL = ramp('STEEL', '#3a4054', '#808ba4', '#d0d8ea')
LEATH = ramp('LEATH', '#33200f', '#654026', '#9a6a3c')
SKIN  = ramp('SKIN',  '#7a4433', '#bf8259', '#e9b085')
GOLD  = ramp('GOLD',  '#7a5a10', '#dcae2c', '#fff0a0')
RED   = ramp('RED',   '#4e0d13', '#a41e2a', '#e24a58')
GREEN = ramp('GREEN', '#17401a', '#3a962e', '#8fe84c')
FIRE  = ramp('FIRE',  '#7e180a', '#e44c18', '#ffb232', '#fff4b8')
ELEC  = ramp('ELEC',  '#1c3a8a', '#4aa0ff', '#c0e8ff')
WHITE = ramp('WHITE', '#ffffff')[0]
# habits "accent" des ennemis : 5 variantes (une par zone) -> SKINMAP
ACCS = [
    ramp('ACC0', '#3a3026', '#695641', '#a08a63'),   # khaki  (zone 1, base)
    ramp('ACC1', '#4f1220', '#9a2838', '#d85468'),   # cramoisi (zone 2)
    ramp('ACC2', '#12403a', '#278a74', '#5cc4a8'),   # vert-de-gris (zone 3)
    ramp('ACC3', '#1a3556', '#3668a0', '#70a6e0'),   # bleu acier (zone 4)
    ramp('ACC4', '#2e1a52', '#66409e', '#a47ae0'),   # violet (zone 5)
]
ACC = ACCS[0]
# pierres des zones : [fond sombre, mur sombre, mur moyen, mur clair]
ZR = [
    ramp('Z1', '#0f121b', '#1a1f2f', '#2e3951', '#4e5e80'),
    ramp('Z2', '#1a1410', '#2c2218', '#4b3a28', '#7a6044'),
    ramp('Z3', '#0a1a16', '#133028', '#24584a', '#3f8a72'),
    ramp('Z4', '#161c34', '#26325a', '#44589a', '#86a0d6'),
    ramp('Z5', '#140a1c', '#26143a', '#44285e', '#7a4a98'),
]

LIGHTER, DARKER = {}, {}
for _n, _idx in RAMPS.items():
    for _i, _c in enumerate(_idx):
        LIGHTER[_c] = _idx[min(_i + 1, len(_idx) - 1)]
        DARKER[_c] = _idx[max(_i - 1, 0)]

def to565(rgb):
    r, g, b = rgb
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

# ------------------------------------------------------------------ canevas
class Cv:
    def __init__(s, w, h):
        s.w, s.h = w, h
        s.a = [[0] * w for _ in range(h)]
        s.lk = [[False] * w for _ in range(h)]
        s.nl = False                      # True -> pixels non ombres
        s.ox = s.oy = 0                   # decalage de dessin (marges)

    def px(s, x, y, c):
        x = R(x) + s.ox; y = R(y) + s.oy
        if 0 <= x < s.w and 0 <= y < s.h:
            s.a[y][x] = c
            s.lk[y][x] = s.nl

    def rect(s, x0, y0, x1, y1, c):
        for y in range(R(y0), R(y1) + 1):
            for x in range(R(x0), R(x1) + 1):
                s.px(x, y, c)

    def ell(s, cx, cy, rx, ry, c):
        rx = max(rx, 0.6); ry = max(ry, 0.6)
        for y in range(int(cy - ry - 2), int(cy + ry + 3)):
            for x in range(int(cx - rx - 2), int(cx + rx + 3)):
                if ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 <= 1.0:
                    s.px(x, y, c)

    def line(s, x0, y0, x1, y1, c, th=1):
        n = int(max(abs(x1 - x0), abs(y1 - y0)) * 2) + 1
        for i in range(n + 1):
            t = i / n
            x = x0 + (x1 - x0) * t
            y = y0 + (y1 - y0) * t
            if th <= 1:
                s.px(x, y, c)
            else:
                h = th / 2.0
                s.rect(x - h + 0.5, y - h + 0.5, x + h - 0.5, y + h - 0.5, c)

    def blit(s, o, ox, oy):
        for y in range(o.h):
            for x in range(o.w):
                if o.a[y][x]:
                    s.px(ox + x, oy + y, o.a[y][x])

def finish(cv, shade=True, outline=True):
    """ombrage de rebord (haut/gauche clair, bas/droite sombre) + contour sombre."""
    w, h, a = cv.w, cv.h, cv.a
    out = [r[:] for r in a]
    if shade:
        for y in range(h):
            for x in range(w):
                c = a[y][x]
                if c == 0 or cv.lk[y][x]:
                    continue
                up = (y == 0 or a[y - 1][x] == 0)
                dn = (y == h - 1 or a[y + 1][x] == 0)
                lf = (x == 0 or a[y][x - 1] == 0)
                rt = (x == w - 1 or a[y][x + 1] == 0)
                if (up or lf) and not (dn or rt):
                    out[y][x] = LIGHTER.get(c, c)
                elif (dn or rt) and not (up or lf):
                    out[y][x] = DARKER.get(c, c)
    if outline:
        res = [r[:] for r in out]
        for y in range(h):
            for x in range(w):
                if out[y][x] == 0:
                    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                        xx, yy = x + dx, y + dy
                        if 0 <= xx < w and 0 <= yy < h and out[yy][xx] not in (0, OUT):
                            res[y][x] = OUT
                            break
        out = res
    return out

# ------------------------------------------------------------------ rig
class Rig:
    """coordonnees en unites : x relatif au centre, y relatif au sol (negatif = haut)."""
    def __init__(s, cv, cx, gy, k=1.0):
        s.cv, s.cx, s.gy, s.k = cv, cx, gy, k
    def X(s, u): return s.cx + u * s.k
    def Y(s, v): return s.gy + v * s.k
    def th(s, t): return max(1, R(t * s.k))
    def line(s, u0, v0, u1, v1, c, t=1):
        s.cv.line(s.X(u0), s.Y(v0), s.X(u1), s.Y(v1), c, s.th(t))
    def rect(s, u0, v0, u1, v1, c):
        s.cv.rect(s.X(u0), s.Y(v0), s.X(u1), s.Y(v1), c)
    def ell(s, u, v, ru, rv, c):
        s.cv.ell(s.X(u), s.Y(v), ru * s.k, rv * s.k, c)
    def px(s, u, v, c):
        s.cv.px(s.X(u), s.Y(v), c)

def leg(r, hip, foot, c, boot, t=2, toe=2):
    r.line(hip[0], hip[1], foot[0], foot[1] - 1, c, t)
    r.rect(foot[0] - 1, foot[1] - 1, foot[0] + toe, foot[1], boot)

def arm(r, sh, hand, c, t=2):
    r.line(sh[0], sh[1], sh[0] + hand[0], sh[1] + hand[1], c, t)

def runpose(i, n, stride=5, lift=3, arm_sw=4, bobamp=1):
    th = 2 * math.pi * i / n
    s, c = math.sin(th), math.cos(th)
    return dict(ff=(R(stride * s), -R(lift * max(0, c))),
                bf=(R(-stride * s), -R(lift * max(0, -c))),
                fh=(R(2 - arm_sw * s) + 1, 4 - R(abs(c))),
                bh=(R(-2 + arm_sw * s), 4 - R(abs(c))),
                bob=R(bobamp * abs(s)))

# armes tenues en main : (hx,hy) = main en unites, ang en degres (0=droite, 90=bas, -90=haut)
def weapon(r, hx, hy, ang, kind, glow=0):
    a = math.radians(ang)
    dx, dy = math.cos(a), math.sin(a)
    def pt(l): return (hx + dx * l, hy + dy * l)
    def perp(l, w):
        px_, py_ = pt(l)
        return (px_ - dy * w, py_ + dx * w), (px_ + dy * w, py_ - dx * w)
    if kind == 'sword':
        r.line(*pt(0), *pt(1.5), LEATH[1], 1)
        p0, p1 = perp(1.8, 2.2); r.line(*p0, *p1, GOLD[1], 1)
        r.line(*pt(2.2), *pt(11.5), STEEL[2], 1)
        r.px(*pt(12), WHITE)
    elif kind == 'dagger':
        r.line(*pt(0), *pt(1.2), LEATH[1], 1)
        p0, p1 = perp(1.6, 1.5); r.line(*p0, *p1, GOLD[1], 1)
        r.line(*pt(2), *pt(6.5), STEEL[2], 1)
    elif kind == 'hammer':
        r.line(*pt(-1), *pt(10), LEATH[1], 1)
        p0, p1 = perp(10.5, 3.2)
        r.line(*p0, *p1, STEEL[1], 3)
        r.line(*perp(11.8, 2.6)[0], *perp(11.8, 2.6)[1], STEEL[2], 1)
    elif kind == 'bow':
        top = (hx + 0.5, hy - 7); bot = (hx + 0.5, hy + 7); mid = (hx + 2.5, hy)
        r.line(*top, *mid, LEATH[2], 1); r.line(*mid, *bot, LEATH[2], 1)
        r.line(*top, *bot, BONE[2], 1)
    elif kind in ('staff', 'rod'):
        r.line(*pt(-2), *pt(12), LEATH[1] if kind == 'staff' else STEEL[1], 1)
        tip = pt(13)
        rp = ELEC if kind == 'rod' else FIRE
        r.cv.nl = True
        r.ell(tip[0], tip[1], 1.6 + glow, 1.6 + glow, rp[1])
        r.ell(tip[0], tip[1], 0.9 + glow * 0.5, 0.9 + glow * 0.5, rp[-1])
        r.cv.nl = False
    elif kind == 'club':
        r.line(*pt(-1), *pt(9), LEATH[1], 2)
        r.ell(*pt(9.5), 2.2, 2.2, LEATH[2])
        r.px(*pt(11), STEEL[2]); r.px(pt(9.5)[0], pt(9.5)[1] - 2, STEEL[2])
    elif kind == 'xbow':
        r.line(*pt(-2), *pt(8), LEATH[1], 2)
        (a0, a1) = perp(6.5, 4.2)
        r.line(*a0, *a1, LEATH[2], 1)
        r.line(*a0, *pt(3.5), BONE[1], 1); r.line(*a1, *pt(3.5), BONE[1], 1)
        r.line(*pt(2), *pt(9), STEEL[1], 1)
    elif kind == 'greatsword':
        r.line(*pt(-2), *pt(2), LEATH[1], 2)
        p0, p1 = perp(2.6, 4.2); r.line(*p0, *p1, GOLD[1], 1)
        r.line(*pt(3.2), *pt(13), STEEL[2], 2)
        r.line(*pt(3.2), *pt(13), STEEL[1], 1)
        r.px(*pt(13.5), WHITE)
