/* dc_render.c - rendu par bandes de 16 lignes (320x16 pixels RGB565) : aucun tampon plein ecran en RAM. */
#include "dc.h"

#define C565(r, g, b) ((uint16_t)((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3)))
#define C_WHITE  C565(255, 255, 255)
#define C_SHADOW C565(10, 8, 20)
#define C_GOLD   C565(255, 214, 80)
#define C_RED    C565(224, 64, 64)
#define C_GREEN  C565(120, 232, 90)
#define C_CYAN   C565(110, 190, 255)
#define C_GREY   C565(150, 150, 170)
#define C_ORANGE C565(255, 150, 50)
#define C_PANEL  C565(18, 14, 34)
#define C_PANEL2 C565(40, 32, 70)

static uint16_t *S;
static int SY;

static void px(int x, int y, uint16_t c) { if ((unsigned)x < SCR_W && (unsigned)(y - SY) < STRIP_H) S[(y - SY) * SCR_W + x] = c; }
static void rect(int x, int y, int w, int h, uint16_t c) {
    int y0 = y < SY ? SY : y, y1 = (y + h > SY + STRIP_H) ? SY + STRIP_H : y + h;
    int x0 = x < 0 ? 0 : x, x1 = (x + w > SCR_W) ? SCR_W : x + w;
    for (int yy = y0; yy < y1; yy++) { uint16_t *d = S + (yy - SY) * SCR_W; for (int xx = x0; xx < x1; xx++) d[xx] = c; }
}
static uint16_t lit(uint16_t c, int a) {
    int r = (c >> 11) + a, g = ((c >> 5) & 63) + a * 2, b = (c & 31) + a;
    if (r > 31) r = 31;
    if (g > 63) g = 63;
    if (b > 31) b = 31;
    return (uint16_t)((r << 11) | (g << 5) | b);
}
static uint16_t tint_red(uint16_t c) { int r = ((c >> 11) & 31) + 12; if (r > 31) r = 31; return (uint16_t)((c & 0x07FF) | (r << 11)); }

/* ---------------------------------------------------------------- texte (police 5x7 + accents, agrandie par entier) */
static int tw(const char *s, int sc) { int n = 0; while (s[n]) n++; return n * 6 * sc; }
static void text(int x, int y, const char *s, uint16_t c, int sc) {
    if (y + 8 * sc < SY || y - sc >= SY + STRIP_H) return;
    for (; *s; s++, x += 6 * sc) {
        int ch = (uint8_t)*s;
        if (ch >= 'a' && ch <= 'z') ch -= 32;
        if (ch >= 144) ch = ' ';
        const uint8_t *g = FONT9[ch];
        for (int r = 0; r < 9; r++) {
            int b = g[r], yy = y + (r - 1) * sc;
            if (!b || yy + sc <= SY || yy >= SY + STRIP_H) continue;
            for (int i = 0; i < 5; i++) if (b & (16 >> i)) rect(x + i * sc, yy, sc, sc, c);
        }
    }
}
static void text_sh(int x, int y, const char *s, uint16_t c, int sc) { text(x + sc, y + sc, s, C_SHADOW, sc); text(x, y, s, c, sc); }
static void text_c(int cx, int y, const char *s, uint16_t c, int sc) { text_sh(cx - tw(s, sc) / 2, y, s, c, sc); }
static char *i2s(int v, char *o) {                     /* o doit contenir au moins 12 octets */
    char t[12]; int n = 0, i = 0, neg = v < 0;
    if (neg) v = -v;
    do { t[n++] = (char)('0' + v % 10); v /= 10; } while (v);
    if (neg) o[i++] = '-';
    while (n) o[i++] = t[--n];
    o[i] = 0; return o;
}
static void cat(char *d, const char *s) { while (*d) d++; while ((*d++ = *s++)) {} }

/* ---------------------------------------------------------------- sprites */
static void spr(int id, int x, int y, int flip, int mode, int skin) {
    const Spr *sp = &SPR[id]; int w = sp->w, h = sp->h;
    int y0 = y > SY ? y : SY, y1 = (y + h < SY + STRIP_H) ? y + h : SY + STRIP_H;
    if (y0 >= y1 || x >= SCR_W || x + w <= 0) return;
    const uint8_t *base = SPRDATA + sp->off;
    for (int yy = y0; yy < y1; yy++) {
        const uint8_t *row = base + (yy - y) * w;
        uint16_t *dst = S + (yy - SY) * SCR_W;
        for (int i = 0; i < w; i++) {
            int dx = x + i;
            if ((unsigned)dx >= SCR_W) continue;
            uint8_t c = row[flip ? (w - 1 - i) : i];
            if (!c) continue;
            if (skin && c >= ACC_LO && c < ACC_LO + 3) c = SKINMAP[skin][c - ACC_LO];
            uint16_t col = PAL565[c];
            if (mode == 1) col = C_WHITE; else if (mode == 2) col = tint_red(col);
            dst[dx] = col;
        }
    }
}
static void spr2x(int id, int x, int y, int flip) {                /* agrandissement x2 (ecran titre) */
    const Spr *sp = &SPR[id]; int w = sp->w, h = sp->h; const uint8_t *base = SPRDATA + sp->off;
    for (int yy = y; yy < y + 2 * h; yy++) {
        if (yy < SY || yy >= SY + STRIP_H) continue;
        const uint8_t *row = base + ((yy - y) >> 1) * w;
        for (int i = 0; i < 2 * w; i++) {
            uint8_t c = row[flip ? (w - 1 - (i >> 1)) : (i >> 1)];
            if (c) px(x + i, yy, PAL565[c]);
        }
    }
}
static void spr_f(int id, int x, int y, int ax, int flip, int mode, int skin) {   /* ancre pieds : (x,y) = centre-bas */
    const Spr *sp = &SPR[id];
    spr(id, flip ? x - (sp->w - 1 - ax) : x - ax, y - sp->h + 1, flip, mode, skin);
}

/* ---------------------------------------------------------------- fond */
static int hash2(int a, int b) { unsigned h = (unsigned)a * 374761393u + (unsigned)b * 668265263u; h = (h ^ (h >> 13)) * 1274126177u; return (int)(h >> 16); }

static void draw_sky(int camx, int z) {
    const uint16_t *zc = ZCOL[z];
    for (int yy = SY; yy < SY + STRIP_H; yy++) {
        uint16_t c = yy < 70 ? zc[0] : (yy < 130 ? lit(zc[0], 2) : (yy < 180 ? zc[1] : zc[2]));
        rect(0, yy, SCR_W, 1, c);
    }
    for (int i = 0; i < 40; i++) {                                    /* etoiles */
        int x = hash2(i, 1) % 400 - (camx / 8) % 400, y = hash2(i, 2) % 150;
        if (x < 0) x += 400;
        if (x < SCR_W && (((G.frame >> 4) + i) & 7)) px(x, y, i & 3 ? C565(200, 210, 255) : C_WHITE);
    }
    int mx = 240 - (camx / 10) % 400;                                  /* lune */
    for (int yy = SY; yy < SY + STRIP_H; yy++) for (int xx = mx - 14; xx <= mx + 14; xx++)
        if ((xx - mx) * (xx - mx) + (yy - 50) * (yy - 50) <= 196 && ((xx - mx + 6) * (xx - mx + 6) + (yy - 46) * (yy - 46)) > 150) px(xx, yy, C565(240, 232, 200));
    int sc = camx / 4;                                                 /* tours lointaines */
    for (int xx = 0; xx < SCR_W; xx++) {
        int k = ((xx + sc) >> 4) & 15, hgt = 70 + (hash2(k, 7) % 4) * 14;
        if (hash2(k, 9) & 1) hgt -= 30;
        if (SY + STRIP_H > 240 - hgt) for (int yy = (240 - hgt > SY ? 240 - hgt : SY); yy < SY + STRIP_H; yy++) px(xx, yy, ZCOL[z][1]);
    }
}

static void draw_bg(int camx) {
    int z = G.zone;
    if (z == 3) { draw_sky(camx, 3); return; }
    const uint8_t *t = SPRDATA + SPR[SB_BGT + z].off;
    int sc = camx / 2;
    for (int yy = SY; yy < SY + STRIP_H; yy++) {
        const uint8_t *row = t + (yy & 31) * 32;
        uint16_t *d = S + (yy - SY) * SCR_W;
        for (int xx = 0; xx < SCR_W; xx++) d[xx] = PAL565[row[(xx + sc) & 31]];
    }
    int per = 160, first = sc / per - 1;                                /* decor lointain tous les 160 px */
    for (int k = first; k < first + 4; k++) spr(SB_BGD + z, k * per + 64 - sc, 30, 0, 0, 0);
}

static void draw_glow(int camx) {
    if (!G.sv.opt_fx) return;
    for (int i = 0; i < G.torch_n; i++) {
        int cx = G.torch_x[i] - camx, cy = G.torch_y[i], R = 46, R2 = R * R;
        if (cx < -R || cx > SCR_W + R) continue;
        for (int yy = SY; yy < SY + STRIP_H; yy++) {
            int dy = yy - cy; if (dy < -R || dy > R) continue;
            uint16_t *d = S + (yy - SY) * SCR_W;
            for (int xx = (cx - R < 0 ? 0 : cx - R); xx <= cx + R && xx < SCR_W; xx++) {
                int d2 = (xx - cx) * (xx - cx) + dy * dy;
                if (d2 < R2) d[xx] = lit(d[xx], 1 + ((R2 - d2) * 5 / R2) + (((G.frame >> 2) + i) & 1));
            }
        }
    }
}

static void draw_tiles(int camx) {
    int ty = SY / TS, z = G.zone;
    if (ty >= LV_H) return;
    for (int tx = camx / TS; tx <= (camx + SCR_W - 1) / TS && tx < G.lvw; tx++) {
        if (tx < 0) continue;
        int t = G.map[ty][tx], sx = tx * TS - camx;
        switch (t) {
            case TL_SOLID: case TL_GATE:
                if (t == TL_GATE) { spr(SB_TL_GATE, sx, SY, 0, 0, 0); break; }
                spr(SB_TL_SOLID + z * 3 + (hash2(tx, ty) & 3) % 3, sx, SY, 0, 0, 0);
                if (ty > 0 && !is_solid(G.map[ty - 1][tx])) spr(SB_TL_CAP + z, sx, SY, 0, 0, 0);
                break;
            case TL_PLAT: spr(SB_TL_PLAT + z, sx, SY, 0, 0, 0); break;
            case TL_SPIKE: spr(SB_TL_SPIKE, sx, SY, 0, 0, 0); break;
            case TL_TOXIC: spr(SB_TL_TOXIC + ((G.frame >> 3) & 1), sx, SY, 0, 0, 0); break;
            case TL_TORCH: spr(SB_TL_TORCH + ((G.frame >> 2) % 3), sx + 4, SY, 0, 0, 0); break;
            case TL_DOOR: spr(SB_TL_DOOR, sx, SY - TS, 0, 0, 0); break;
            default: break;
        }
        if (ty + 1 < LV_H && G.map[ty + 1][tx] == TL_DOOR) spr(SB_TL_DOOR, sx, SY, 0, 0, 0);   /* moitie haute de la porte */
    }
}

/* ---------------------------------------------------------------- entites */
static const uint8_t ACX[ET_COUNT] = { 14, 14, 14, 20, 11, 14, 14, 14, 16, 26, 22 };

static int esp(const Ent *e) {
    int a = e->anim, mv = e->vx != 0, t = e->t;
    switch (e->type) {
    case ET_JAIL:  return e->st == ES_DEAD ? SB_JAIL_DEAD + (t > 8) : e->st == ES_HURT ? SB_JAIL_HURT : e->st == ES_WIND ? SB_JAIL_WIND : e->st == ES_ATK ? SB_JAIL_SWING : mv ? SB_JAIL_WALK + ((a >> 2) & 3) : SB_JAIL_IDLE;
    case ET_XBOW:  return e->st == ES_DEAD ? SB_XBOW_DEAD + (t > 8) : e->st == ES_HURT ? SB_XBOW_HURT : e->st == ES_WIND ? SB_XBOW_AIM : (e->st == ES_REC && t > 8) ? SB_XBOW_SHOOT : mv ? SB_XBOW_WALK + ((a >> 2) & 3) : SB_XBOW_IDLE;
    case ET_RAT:   return e->st == ES_DEAD ? SB_RAT_DEAD + (t > 8) : e->st == ES_HURT ? SB_RAT_HURT : e->st == ES_ATK ? SB_RAT_LEAP : SB_RAT_RUN + (mv ? ((a >> 1) & 3) : 0);
    case ET_BRUTE: return e->st == ES_DEAD ? SB_BRUTE_DEAD + (t > 8) : e->st == ES_HURT ? SB_BRUTE_HURT : e->st == ES_WIND ? SB_BRUTE_WIND : e->st == ES_ATK ? SB_BRUTE_SLAM : mv ? SB_BRUTE_WALK + ((a >> 3) & 3) : SB_BRUTE_IDLE;
    case ET_SLIME: return e->st == ES_DEAD ? SB_SLIME_DEAD + (t > 8) : e->st == ES_HURT ? SB_SLIME_HURT : e->st == ES_AIR ? SB_SLIME_HOP : SB_SLIME_IDLE + ((a >> 4) & 1);
    case ET_GREN:  return e->st == ES_DEAD ? SB_GREN_DEAD + (t > 8) : e->st == ES_HURT ? SB_GREN_HURT : e->st == ES_WIND ? SB_GREN_WIND : (e->st == ES_REC && t > 7) ? SB_GREN_THROW : mv ? SB_GREN_WALK + ((a >> 2) & 3) : SB_GREN_IDLE;
    case ET_SHLD:  return e->st == ES_DEAD ? SB_SHLD_DEAD + (t > 8) : e->st == ES_HURT ? SB_SHLD_HURT : e->st == ES_WIND ? SB_SHLD_WIND : e->st == ES_ATK ? SB_SHLD_BASH : mv ? SB_SHLD_WALK + ((a >> 3) & 3) : SB_SHLD_IDLE;
    case ET_TOXIC: return e->st == ES_DEAD ? SB_TOXIC_DEAD + (t > 8) : e->st == ES_HURT ? SB_TOXIC_HURT : e->st == ES_ATK ? SB_TOXIC_ATK : mv ? SB_TOXIC_WALK + ((a >> 3) & 3) : SB_TOXIC_IDLE;
    case ET_BAT:   return e->st == ES_DEAD ? SB_BAT_DEAD + (t > 8) : e->st == ES_HURT ? SB_BAT_HURT : e->st == ES_ATK ? SB_BAT_DIVE : SB_BAT_FLY + ((a >> 1) & 3);
    case ET_GOB:
        if (e->st == ES_DEAD) return SB_GOB_DEAD + (t > 25);
        if (e->st == ES_AIR) return (e->aux == 2 && e->t == 1) ? SB_GOB_SLAM : SB_GOB_HOP;
        if (e->st == ES_WIND) return e->aux == 1 ? SB_GOB_SPIT : SB_GOB_SLAM;
        if (e->st == ES_REC) return t > 10 ? (e->aux == 1 ? SB_GOB_SPIT : SB_GOB_SLAM) : SB_GOB_IDLE;
        return SB_GOB_IDLE + ((a >> 4) & 1);
    default:
        if (e->st == ES_DEAD) return SB_KING_DEAD + (t > 35 ? 2 : t > 15 ? 1 : 0);
        if (e->st == ES_WIND) return e->aux == 0 ? SB_KING_CWIND : e->aux == 1 ? SB_KING_SWIND : SB_KING_SLWIND;
        if (e->st == ES_ATK) return e->aux == 0 ? SB_KING_DASH : e->aux == 1 ? SB_KING_SWEEP : SB_KING_SLAM;
        if (e->st == ES_REC) return e->aux == 2 ? SB_KING_SLAM : (e->aux == 0 && t > 22) ? SB_KING_HURT : SB_KING_IDLE;
        return mv ? SB_KING_WALK + ((a >> 3) & 3) : SB_KING_IDLE + ((a >> 4) & 1);
    }
}

static void draw_enemy(const Ent *e, int camx) {
    if (!e->alive) return;
    int x = PX(e->x) - camx, y = PX(e->y);
    if (x < -70 || x > SCR_W + 70) return;
    int mode = e->flash > 0 ? 1 : (e->elite && ((G.frame >> 3) & 1)) ? 2 : 0;
    if (e->type == ET_BAT) { const Spr *sp = &SPR[esp(e)]; spr(esp(e), e->dir < 0 ? x - (sp->w - 1 - 16) : x - 16, y - 15, e->dir < 0, mode, 0); return; }
    spr_f(esp(e), x, y, ACX[e->type], e->dir < 0, mode, e->skin);
    if (e->burn > 0 && (G.frame & 2)) px(x + (hash2(G.frame, e->type) % 7) - 3, y - ent_h(e) - (G.frame & 3), C_ORANGE);
}

static void draw_player(int camx) {
    Player *p = &G.p;
    int x = PX(p->x) - camx, y = PX(p->y), id, flip = p->dir < 0, mode = 0;
    const Wpn *w = &WPN[p->w[p->cur].id];
    if (p->st == PS_DEAD) id = SB_HERO_DEAD + (p->dead_t > 24 ? 2 : p->dead_t > 8 ? 1 : 0);
    else if (p->st == PS_HURT) id = SB_HERO_HURT;
    else if (p->st == PS_ROLL) id = SB_HERO_ROLL + ((p->t >> 1) & 3);
    else if (p->st == PS_ATK) {
        const Atk *a = p->atk_b ? &w->b : (p->heavy ? &w->h : &w->l);
        int f = p->atk_t < a->a0 ? 0 : (p->atk_t <= a->a1 ? 1 : 2);
        switch (w->kind) {
            case WK_BOW: id = SB_HERO_BOW + (f > 0); break;
            case WK_FIRE: case WK_ELEC: id = SB_HERO_CAST + (f > 0); break;
            case WK_DUAL: id = p->atk_b ? SB_HERO_DAGGER + (f > 0) : SB_HERO_SWORD + f; break;
            case WK_SHIELD: id = SB_HERO_DAGGER + (f > 0); break;
            default:
                switch (p->w[p->cur].id) {
                    case WP_DAGGER: case WP_RAPIER: id = SB_HERO_DAGGER + (f > 0); break;
                    case WP_HAMMER: id = SB_HERO_HAMMER + f; break;
                    default: id = SB_HERO_SWORD + f; break;
                }
        }
    }
    else if (!p->ground) id = (p->jumps == 2 && p->flip_t > 0) ? SB_HERO_FLIP + ((p->flip_t >> 3) & 1) : (p->vy < 0 ? SB_HERO_JUMP : SB_HERO_FALL);
    else if (p->vx != 0) id = SB_HERO_RUN + ((p->anim / 3) % 6);
    else id = SB_HERO_IDLE + ((p->anim >> 4) & 1);
    if (p->hold >= 14 && (G.frame & 2)) mode = 1;                       /* charge prete : clignote */
    if (p->inv > 0 && p->st != PS_ROLL && p->st != PS_DEAD && (G.frame & 2)) return;
    spr_f(id, x, y, 14, flip, mode, 0);
    if (p->guard) {                                                     /* bouclier leve devant le heros */
        spr(SB_IT_WPN + WP_SHIELD, x + p->dir * 8 - 8, y - 24, 0, p->guard_t <= 8 ? 1 : 0, 0);
    }
}

static void draw_projectiles(int camx) {
    for (int i = 0; i < MAX_PR; i++) {
        const Proj *p = &G.pr[i];
        if (!p->life) continue;
        int x = PX(p->x) - camx, y = PX(p->y), fl = p->vx < 0;
        switch (p->type) {
            case PJ_ARROW: case PJ_PARROW: spr(SB_FX_ARROW, x - 8, y - 2, fl, 0, 0); break;
            case PJ_BOLT: case PJ_XBOLT: spr(SB_FX_BOLT, x - 6, y - 2, fl, 0, 0); break;
            case PJ_FIRE: spr(SB_FX_FIRE + ((p->anim >> 1) & 3), x - 8, y - 8, fl, 0, 0); break;
            case PJ_SPIT: spr(SB_FX_SPIT + ((p->anim >> 2) & 1), x - 4, y - 4, 0, 0, 0); break;
            case PJ_WAVE: spr(SB_FX_WAVE + ((p->anim >> 2) & 1), x - 8, y - 15, fl, 0, 0); break;
            case PJ_BOMB: spr(SB_FX_BOMB + ((p->anim >> 2) & 1), x - 4, y - 4, 0, 0, 0); break;
            default: spr(SB_FX_CLOUD + ((p->anim >> 3) & 1), x - 12, y - 16, 0, 0, 0); break;
        }
    }
}

static void draw_line(int x0, int y0, int x1, int y1, uint16_t c) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1, dy = y1 > y0 ? y0 - y1 : y1 - y0, sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, err = dx + dy;
    for (int n = 0; n < 600; n++) {
        px(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void draw_particles(int camx) {
    for (int i = 0; i < MAX_PA; i++) {
        const Part *a = &G.pa[i];
        if (!a->life) continue;
        int x = PX(a->x) - camx, y = PX(a->y);
        switch (a->kind) {
            case PA_SPARK: { uint16_t c = a->col == 1 ? C_RED : a->col == 2 ? C_GOLD : a->col == 3 ? C_GREEN : C565(255, 240, 200); rect(x, y, 2, 2, c); break; }
            case PA_SMOKE: rect(x, y, 3, 3, C565(140, 140, 160)); break;
            case PA_EMBER: px(x, y, C_ORANGE); break;
            case PA_FX: { int f = (a->max - a->life) / (a->val ? a->val : 1); if (f >= a->nf) f = a->nf - 1;
                const Spr *sp = &SPR[a->num + f]; spr(a->num + f, x - sp->w / 2, y - sp->h / 2, a->col, 0, 0); break; }
            case PA_NUM: { char b[12]; uint16_t c = a->col == 1 ? C_RED : a->col == 3 ? C_GREEN : a->col == 4 ? C_ORANGE : C_WHITE; i2s(a->num, b); text_sh(x - tw(b, 1) / 2, y, b, c, 1); break; }
            case PA_LINE: {
                int x1 = a->vx - camx, y1 = a->vy, mx = (x + x1) / 2 + ((G.frame * 7 + i) % 9 - 4), my = (y + y1) / 2 + ((G.frame * 5 + i) % 9 - 4);
                draw_line(x, y, mx, my, C565(200, 232, 255)); draw_line(mx, my, x1, y1, C565(120, 190, 255)); break; }
            default: break;
        }
    }
}

static void draw_picks(int camx) {
    for (int i = 0; i < MAX_PK; i++) {
        const Pick *k = &G.pk[i];
        if (!k->life) continue;
        int x = PX(k->x) - camx, y = PX(k->y), bob = (G.frame >> 3) & 1;
        if (x < -30 || x > SCR_W + 30) continue;
        switch (k->kind) {
            case PK_COIN: spr(SB_IT_COIN + ((G.frame >> 2) & 3), x - 4, y - 8, 0, 0, 0); break;
            case PK_CELL: spr(SB_IT_CELL + ((G.frame >> 3) & 1), x - 4, y - 8, 0, 0, 0); break;
            case PK_HEART: spr(SB_IT_HEART, x - 5, y - 10 - bob, 0, 0, 0); break;
            case PK_FLASK: spr(SB_IT_FLASK, x - 5, y - 12 - bob, 0, 0, 0); break;
            case PK_BLUE: spr(SB_IT_BLUEPRINT, x - 5, y - 12 - bob, 0, 0, 0); break;
            case PK_WPN: { char lab[8]; Item it; it.id = k->val; it.lvl = k->lvl ? k->lvl : 1; it.q = k->q; item_label(&it, lab);
                spr(SB_IT_WPN + k->val, x - 8, y - 18 - bob, 0, 0, 0); text_sh(x - tw(lab, 1) / 2, y - 29, lab, it.q == 4 ? C_ORANGE : C_GOLD, 1); break; }
            case PK_CHEST: spr(SB_IT_CHEST + k->st, x - 8, y - 13, 0, 0, 0); break;
            default: {                                                  /* PNJ */
                int base = k->val ? SB_NPC_ORACLE : SB_NPC_PASSEUR;
                spr(base + ((G.frame >> 4) & 1), x - 12, y - 31, 0, 0, 0);
                if (iabs(PX(G.p.x) - PX(k->x)) < 22) text_sh(x - 6, y - 42, "OK", C_GOLD, 1);
                break; }
        }
    }
}

/* ---------------------------------------------------------------- monde */
static void draw_world(void) {
    int sh = G.shake > 0 ? ((G.frame & 1) ? G.shake / 2 : -G.shake / 2) : 0, camx = PX(G.cam_x) + sh;
    draw_bg(camx);
    draw_tiles(camx);
    draw_picks(camx);
    for (int i = 0; i < MAX_EN; i++) draw_enemy(&G.en[i], camx);
    draw_player(camx);
    draw_projectiles(camx);
    draw_particles(camx);
    draw_glow(camx);
}

/* ---------------------------------------------------------------- interface */
static void dim(void) { for (int i = 0; i < SCR_W * STRIP_H; i++) S[i] = (uint16_t)((S[i] >> 1) & 0x7BEF); }
static void fill_bg(uint16_t c) { rect(0, SY, SCR_W, STRIP_H, c); }
static void draw_bar(int x, int y, int w, int h, int v, int max, uint16_t c, uint16_t back) {
    int f = max > 0 ? v * w / max : 0;
    if (f < 0) f = 0;
    if (f > w) f = w;
    rect(x - 2, y - 2, w + 4, h + 4, C_SHADOW); rect(x, y, w, h, back);
    rect(x, y, f, h, c); rect(x, y, f, h / 3 + 1, lit(c, 8));
}
static void price_at(int right, int y, int price, int gold) {
    char b[12];
    if (price < 0) { text_sh(right - tw("MAX", 2), y, "MAX", C_GOLD, 2); return; }
    i2s(price, b);
    spr(gold ? SB_IT_COIN : SB_IT_CELL, right + 4, y + 3, 0, 0, 0);
    text_sh(right - tw(b, 2), y, b, gold ? C_GOLD : C_CYAN, 2);
}

static void draw_hud(void) {
    Player *p = &G.p; char b[16], t[16];
    text_sh(6, 5, HEARTC, C_RED, 2);
    draw_bar(26, 8, 100, 10, p->hp, p->maxhp, C_RED, C565(60, 20, 30));
    i2s(p->hp, b); cat(b, "/"); cat(b, i2s(p->maxhp, t)); text_sh(134, 5, b, C_WHITE, 2);
    spr(SB_IT_COIN + ((G.frame >> 2) & 3), 232, 7, 0, 0, 0); text_sh(246, 4, i2s((int)G.run_gold, b), C_GOLD, 2);
    spr(SB_IT_CELL + ((G.frame >> 3) & 1), 232, 25, 0, 0, 0); text_sh(246, 21, i2s((int)G.cells_run, b), C_CYAN, 2);
    if (G.boss_alive && G.boss_idx >= 0 && G.en[G.boss_idx].alive)
        draw_bar(70, 46, 180, 8, G.en[G.boss_idx].hp, G.en[G.boss_idx].maxhp, C565(200, 40, 200), C565(40, 10, 40));
    for (int s = 0; s < 2; s++) {
        int x0 = 6 + s * 32;
        char lab[8]; item_label(&p->w[s], lab);
        rect(x0 - 1, 205, 28, 28, s == p->cur ? C_GOLD : C_GREY); rect(x0, 206, 26, 26, C_PANEL);
        spr(SB_IT_WPN + p->w[s].id, x0 + 5, 210, 0, 0, 0);
        text_sh(x0 + 13 - tw(lab, 1) / 2, 233, lab, p->w[s].q == 4 ? C_ORANGE : C_GOLD, 1);
    }
    spr(SB_IT_FLASK, 76, 210, 0, 0, 0); b[0] = 'X'; i2s(p->flasks, b + 1); text_sh(90, 208, b, C_WHITE, 2);
    if (G.banner_t > 0) text_c(160, 64, G.banner, C_WHITE, 2);
    if (G.msg_t > 0) text_c(160, G.passage ? 124 : 184, G.msg, C_GOLD, 2);
    if (G.hint_t > 0) text_c(160, 150, "OK : ATTAQUE    RETOUR : ESQUIVE", C_WHITE, 1);
    if (G.passage) { text_c(160, 100, "PARLE AUX PNJ AVEC OK", C_GOLD, 1); }
}

static void draw_title(void) {
    static const int cx_[6] = { 14, 62, 118, 196, 246, 284 }, cw_[6] = { 30, 26, 34, 28, 30, 24 }, ch_[6] = { 120, 150, 96, 142, 104, 126 };
    draw_sky(G.frame / 2, 4);
    for (int i = 0; i < 6; i++) {
        rect(cx_[i], 240 - ch_[i], cw_[i], ch_[i], C565(12, 8, 26));
        for (int k = 0; k < cw_[i] / 6; k++) rect(cx_[i] + k * 6, 240 - ch_[i] - 5, 4, 5, C565(12, 8, 26));
        for (int k = 0; k < 3; k++) if (hash2(i, k) & 1) rect(cx_[i] + 7 + k * 7, 240 - ch_[i] + 14 + k * 22, 3, 5, ((G.frame >> 5) + i + k) & 3 ? C_GOLD : C565(120, 70, 20));
    }
    rect(0, 212, SCR_W, 28, C565(8, 5, 18));
    for (int i = 0; i < 26; i++) { int x = hash2(i, 3) % 320, y = 240 - ((G.frame * (1 + i % 3) + hash2(i, 4)) % 260); rect(x, y, 2, 2, ((G.frame >> 3) + i) & 1 ? C_ORANGE : C565(255, 200, 80)); }
    spr2x(SB_HERO_IDLE + ((G.frame >> 5) & 1), 14, 150, 0);
    text_c(160, 18, "DEAD CELLS", C_ORANGE, 4);
    text_c(160, 56, "NUMWORKS EDITION", C_WHITE, 2);
    static const char *const TM[5] = { "JOUER", "BOUTIQUE", "OPTIONS", "COMMANDES", "QUITTER" };
    for (int i = 0; i < 5; i++) {
        int sel = (i == G.menu_sel), y = 92 + i * 22;
        text_c(160, y, TM[i], sel ? C_GOLD : C_GREY, 2);
        if (sel) { text_sh(160 - tw(TM[i], 2) / 2 - 20, y, ARIGHT, C_GOLD, 2); }
    }
    text_c(160, 224, "PROJET DE FAN NON OFFICIEL", C_GREY, 1);
    if (!G.save_ok) text_c(160, 212, "SAUVEGARDE INDISPONIBLE", C_RED, 1);
}

static void draw_options(void) {
    char b[40], t[12];
    fill_bg(C_PANEL); text_c(160, 8, "OPTIONS", C_GOLD, 3);
    for (int i = 0; i < 4; i++) {
        int y = 56 + i * 26; b[0] = 0;
        if (i == 0) { cat(b, "EFFETS : "); cat(b, G.sv.opt_fx ? "OUI" : "NON"); }
        else if (i == 1) { cat(b, "MODE FACILE : "); cat(b, G.sv.opt_easy ? "OUI" : "NON"); }
        else if (i == 2) cat(b, G.menu_sub ? "CONFIRMER ? (OK)" : "EFFACER PROGRESSION");
        else cat(b, "RETOUR");
        if (i == G.menu_sel) rect(20, y - 4, 280, 22, C_PANEL2);
        text_c(160, y, b, i == G.menu_sel ? C_GOLD : C_GREY, 2);
    }
    b[0] = 0; cat(b, "PARTIES "); cat(b, i2s(G.sv.runs, t)); cat(b, "  VICTOIRES "); cat(b, i2s(G.sv.wins, t)); cat(b, "  ENNEMIS "); cat(b, i2s((int)G.sv.kills, t));
    text_c(160, 176, b, C_GREY, 1);
    b[0] = 0; cat(b, "RECORD : ZONE "); cat(b, i2s(G.sv.best_zone, t)); cat(b, "   CELLULES : "); cat(b, i2s((int)G.sv.cells, t));
    text_c(160, 192, b, C_CYAN, 1);
    if (G.msg_t > 0) text_c(160, 212, G.msg, C_GOLD, 1);
}

static void draw_help(void) {
    static const char *const P0[9] = { ALEFT ARIGHT " : BOUGER", AUP " : SAUTER (2 FOIS)", ADOWN " : TRAVERSER PLATEFORME", "OK : ATTAQUE", "OK MAINTENU : COUP LOURD",
        "RETOUR : ESQUIVE", "EXE : FIOLE DE SOIN", "SUPPR : CHANGER D'ARME", "BO" ICI "TE " AGR " OUTILS : PAUSE" };
    static const char *const P1[9] = { "BOUCLIER : OK = GARDE", "PARADE JUSTE : 0 D" EAC "G" ACI "TS", "ET RENVOIE LES TIRS", "FAUX-SERRES : OK / SUPPR", "ALTERNER LES 2 COUPS =", "COUP CRITIQUE",
        "PASSAGES : OK SUR UN PNJ", "CELLULES : AM" EAC "LIORATIONS", "MUTATIONS : 1 PARTIE" };
    fill_bg(C_PANEL); text_c(160, 6, "COMMANDES", C_GOLD, 3);
    for (int i = 0; i < 9; i++) text(14, 36 + i * 20, G.help_page ? P1[i] : P0[i], i == 0 && !G.help_page ? C_WHITE : C_GREY, 2);
    text_c(160, 224, ALEFT " " ARIGHT " : PAGE    RETOUR : FERMER", C_GOLD, 1);
}

static const char *const UP_DESC[UP_COUNT] = { "+10 PV MAX", "+8 % DE D" EAC "G" ACI "TS", "+1 FIOLE DE SOIN", "ESQUIVE PLUS PROTECTRICE", "+10 % DE BUTIN ET CELLULES", "ARMES TROUV" EAC "ES PLUS RARES" };
static const char *const WSHORT[WP_COUNT] = { EAC "P" EAC "E", "DAGUE", "RAPI" EGR "RE", "MARTEAU", "BOUCLIER", "ARC", "ARBAL" EGR "TE", "B" ACI "TON FEU", "SCEPTRE", "FAUX-SERRES" };

static void draw_shop(void) {
    uint8_t k[24], a[24]; int n = shop_rows(k, a);
    char b[40], t[12];
    fill_bg(C_PANEL);
    text_sh(8, 6, G.in_run ? "LE PASSEUR" : "BOUTIQUE", C_GOLD, 3);
    spr(SB_IT_CELL, 296, 11, 0, 0, 0); text_sh(290 - tw(i2s((int)G.sv.cells, t), 2), 8, t, C_CYAN, 2);
    if (G.in_run) { spr(SB_IT_COIN, 296, 29, 0, 0, 0); text_sh(290 - tw(i2s((int)G.run_gold, t), 2), 26, t, C_GOLD, 2); }
    int sel = G.menu_sel < n ? G.menu_sel : n - 1, first = sel - 4;
    if (first > n - 9) first = n - 9;
    if (first < 0) first = 0;
    for (int r = 0; r < 9 && first + r < n; r++) {
        int i = first + r, y = 46 + r * 19, gold = 0, price;
        b[0] = 0;
        switch (k[i]) {
            case RW_UP: cat(b, UP_NAME[a[i]]); cat(b, " "); cat(b, i2s(G.sv.up[a[i]], t)); cat(b, "/"); cat(b, i2s(UP_MAX[a[i]], t)); break;
            case RW_BP: cat(b, "SCH. "); cat(b, WSHORT[a[i]]); break;
            case RW_WLVL: cat(b, "ARME : NIVEAU +1"); break;
            case RW_WQ: cat(b, "ARME : QUALIT" EAC " +"); break;
            default: cat(b, "RETOUR"); break;
        }
        price = shop_price(k[i], a[i], &gold);
        if (i == sel) rect(6, y - 3, 308, 19, C_PANEL2);
        text_sh(12, y, b, i == sel ? C_GOLD : C_GREY, 2);
        if (k[i] != RW_BACK) price_at(298, y, price, gold);
    }
    b[0] = 0;
    switch (k[sel]) {
        case RW_UP: cat(b, UP_DESC[a[sel]]); break;
        case RW_BP: cat(b, "D" EAC "BLOQUE CETTE ARME POUR LES PARTIES"); break;
        case RW_WLVL: { char lab[8]; item_label(&G.p.w[G.p.cur], lab); cat(b, "ARME " ); cat(b, lab); cat(b, " : NIVEAU SUP" EAC "RIEUR"); break; }
        case RW_WQ: cat(b, "QUALIT" EAC " : + / ++ / S"); break;
        default: break;
    }
    text(8, 224, b, C_GREY, 1);
    if (G.msg_t > 0) text_c(160, 212, G.msg, C_RED, 1);
}

static void draw_mut(void) {
    char b[40], t[12];
    fill_bg(C_PANEL);
    text_sh(8, 6, "L'ORACLE", C_GOLD, 3);
    spr(SB_IT_COIN, 296, 11, 0, 0, 0); text_sh(290 - tw(i2s((int)G.run_gold, t), 2), 8, t, C_GOLD, 2);
    if (!G.mut_pending) {
        text_c(160, 70, "UNE MUTATION PAR PASSAGE", C_WHITE, 2);
        text_c(160, 100, "REVIENS AU SUIVANT", C_GREY, 2);
        rect(60, 168, 200, 22, C_PANEL2); text_c(160, 172, "RETOUR", C_GOLD, 2);
    } else {
        for (int i = 0; i < 3; i++) {
            int y = 40 + i * 40;
            if (G.mut_off[i] == 255) continue;
            if (i == G.menu_sel) rect(6, y - 3, 308, 38, C_PANEL2);
            text_sh(12, y, MU_NAME[G.mut_off[i]], i == G.menu_sel ? C_GOLD : C_WHITE, 2);
            text(12, y + 18, MU_DESC[G.mut_off[i]], C_GREY, 2);
        }
        b[0] = 0; cat(b, "RELANCER ("); cat(b, i2s(40 * (G.reroll_n + 1), t)); cat(b, " PI" EGR "CES)");
        if (G.menu_sel == 3) rect(6, 160, 308, 20, C_PANEL2);
        text_sh(12, 162, b, G.menu_sel == 3 ? C_GOLD : C_GREY, 2);
        if (G.menu_sel == 4) rect(6, 184, 308, 20, C_PANEL2);
        text_sh(12, 186, "PASSER", G.menu_sel == 4 ? C_GOLD : C_GREY, 2);
    }
    b[0] = 0; cat(b, "MUTATIONS ACTIVES : "); cat(b, i2s(G.mut_n, t));
    text(8, 224, b, C_CYAN, 1);
    if (G.msg_t > 0) text_c(160, 212, G.msg, C_RED, 1);
}

static void draw_panel_menu(const char *title, const char *const *items, int n, int sel) {
    rect(50, 20, 220, 200, C_PANEL); rect(52, 22, 216, 196, C565(30, 24, 56));
    text_c(160, 30, title, C_GOLD, 3);
    for (int i = 0; i < n; i++) text_c(160, 74 + i * 24, items[i], i == sel ? C_GOLD : C_GREY, 2);
}

static void draw_overlay(void) {
    char b[40], t[12];
    if (G.scene == SC_PAUSE) {
        static const char *const PM[3] = { "REPRENDRE", "COMMANDES", "ABANDONNER" };
        draw_panel_menu("PAUSE", PM, 3, G.menu_sel);
        for (int s = 0; s < 2; s++) { char lab[8]; b[0] = 0; item_label(&G.p.w[s], lab); cat(b, s == G.p.cur ? "> " : "  "); cat(b, WSHORT[G.p.w[s].id]); cat(b, " "); cat(b, lab); text(66, 152 + s * 12, b, s == G.p.cur ? C_GOLD : C_GREY, 1); }
        for (int i = 0; i < G.mut_n; i++) text(66, 178 + i * 10, MU_NAME[G.mut[i]], C_CYAN, 1);
    } else if (G.scene == SC_DEAD || G.scene == SC_WIN) {
        int win = G.scene == SC_WIN;
        rect(30, 30, 260, 180, C_PANEL); rect(32, 32, 256, 176, C565(30, 24, 56));
        text_c(160, 42, win ? "VICTOIRE !" : "VOUS " ECI "TES MORT", win ? C_GOLD : C_RED, 3);
        b[0] = 0; cat(b, "ZONE ATTEINTE : "); cat(b, i2s(G.zone + 1, t)); text_c(160, 84, b, C_WHITE, 2);
        b[0] = 0; cat(b, "ENNEMIS TU" EAC "S : "); cat(b, i2s((int)G.run_kills, t)); text_c(160, 106, b, C_WHITE, 2);
        b[0] = 0; cat(b, "CELLULES GARD" EAC "ES : +"); cat(b, i2s((int)G.bank_gain, t)); text_c(160, 128, b, C_CYAN, 2);
        if (G.bank_lost > 0) { b[0] = 0; cat(b, "CELLULES PERDUES : "); cat(b, i2s((int)G.bank_lost, t)); text_c(160, 150, b, C_RED, 2); }
        text_c(160, 184, "OK : BOUTIQUE   RETOUR : MENU", C_GREY, 1);
    }
}

void game_render_strip(uint16_t *buf, int y0) {
    S = buf; SY = y0;
    switch (G.scene) {
        case SC_TITLE: draw_title(); break;
        case SC_OPTIONS: draw_options(); break;
        case SC_HELP: draw_help(); break;
        case SC_SHOP: draw_shop(); break;
        case SC_MUT: draw_mut(); break;
        default:
            draw_world();
            if (G.scene == SC_PLAY) draw_hud();
            else { dim(); draw_overlay(); }
            break;
    }
}
