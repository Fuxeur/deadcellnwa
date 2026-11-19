/* dc_core.c - etat global, physique, niveaux semi-aleatoires, passages (PNJ), joueur, armes, mutations, menus, sauvegarde. */
#include "dc.h"

Game G;
Ent *g_src;
uint8_t g_pierce, g_crit;

/* ================================================================= utilitaires */
uint32_t rnd(void) {
    uint32_t x = G.rng;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    G.rng = x ? x : 0x9E3779B9u;
    return G.rng;
}
int rn(int n) { return n <= 0 ? 0 : (int)(rnd() % (uint32_t)n); }

void banner(const char *s, int t) {
    int i = 0;
    while (s[i] && i < 39) { G.banner[i] = s[i]; i++; }
    G.banner[i] = 0; G.banner_t = (int16_t)t;
}
void msg(const char *s) {
    int i = 0;
    while (s[i] && i < 31) { G.msg[i] = s[i]; i++; }
    G.msg[i] = 0; G.msg_t = 70;
}

/* ================================================================= tables de donnees */
/* Atk : {duree totale, debut actif, fin actif, elan, degats, portee, y haut, y bas, recul, etourdissement} */
const Wpn WPN[WP_COUNT] = {
    { EAC "P" EAC "E",            WK_MELEE,  LG_CRIT3,  {12, 3, 6, 0, 12, 26, -22, -2, 3, 6},  {22, 8, 12, 0, 26, 34, -26, 0, 6, 12}, {0} },
    { "DAGUE",                    WK_MELEE,  LG_VAMP,   {7, 2, 4, 0, 7, 20, -20, -2, 1, 3},    {14, 3, 8, 4, 15, 24, -20, -2, 3, 8}, {0} },
    { "RAPI" EGR "RE",            WK_MELEE,  LG_IGNITE, {10, 3, 5, 0, 9, 34, -20, -4, 2, 4},   {20, 8, 11, 3, 22, 44, -20, -4, 4, 10}, {0} },
    { "MARTEAU",                  WK_MELEE,  LG_CRIT3,  {22, 9, 12, 0, 30, 30, -26, 2, 6, 24}, {34, 14, 18, 0, 48, 44, -22, 0, 8, 36}, {0} },
    { "BOUCLIER",                 WK_SHIELD, LG_VAMP,   {14, 4, 7, 0, 6, 18, -22, 0, 5, 18},   {14, 4, 7, 0, 6, 18, -22, 0, 5, 18}, {0} },
    { "ARC",                      WK_BOW,    LG_IGNITE, {14, 5, 5, 0, 9, 0, 0, 0, 0, 0},       {26, 12, 12, 0, 24, 0, 0, 0, 0, 0}, {0} },
    { "ARBAL" EGR "TE",           WK_BOW,    LG_IGNITE, {26, 10, 10, 0, 22, 0, 0, 0, 0, 0},    {40, 20, 20, 0, 44, 0, 0, 0, 0, 0}, {0} },
    { "B" ACI "TON DE FEU",       WK_FIRE,   LG_CRIT3,  {18, 7, 7, 0, 11, 0, 0, 0, 0, 0},      {30, 14, 14, 0, 8, 0, 0, 0, 0, 0}, {0} },
    { "SCEPTRE D'" EAC "CLAIR",   WK_ELEC,   LG_CRIT3,  {16, 6, 6, 0, 14, 0, 0, 0, 0, 0},      {36, 16, 16, 0, 10, 0, 0, 0, 0, 0}, {0} },
    { "FAUX-SERRES",              WK_DUAL,   LG_VAMP,   {16, 5, 9, 0, 16, 34, -26, 0, 4, 8},   {28, 10, 14, 0, 32, 40, -28, 2, 6, 14}, {9, 2, 6, 2, 9, 22, -20, -4, 2, 4} },
};
const char *const ZONE_NAME[NZONES] = {
    "QUARTIERS DES PRISONNIERS", "PROMENADE DES CONDAMN" EAC "S", EAC "GOUTS TOXIQUES", "REMPARTS", "CH" ACI "TEAU FINAL"
};
const char *const UP_NAME[UP_COUNT] = { "VIE", "D" EAC "G" ACI "TS", "FIOLES", "ESQUIVE", "BUTIN", "QUALIT" EAC };
const uint8_t UP_MAX[UP_COUNT] = { 5, 5, 2, 3, 3, 3 };
static const uint8_t UP_COST[UP_COUNT][5] = {
    {10, 20, 35, 55, 80}, {12, 25, 45, 70, 100}, {25, 60, 0, 0, 0}, {15, 35, 60, 0, 0}, {20, 45, 80, 0, 0}, {30, 70, 140, 0, 0}
};
static const uint8_t BP_COST[WP_COUNT] = { 0, 0, 12, 20, 20, 18, 35, 30, 40, 60 };

const char *const MU_NAME[MU_COUNT] = {
    "REVANCHE", "INSTINCT", "SAIGNEMENT", "D" EAC "SENGAGEMENT", "ENCHA" ICI "NEMENT", "CARAPACE", "CALME", "ADR" EAC "NALINE", "GOURMAND", "CHANCE"
};
const char *const MU_DESC[MU_COUNT] = {
    "COUP +60 % APR" EGR "S D" EAC "G" ACI "T", "TUER SOIGNE 4 PV", "LES COUPS FONT SAIGNER", "APR" EGR "S ESQUIVE : +40 %",
    "COUPS ENCHA" ICI "N" EAC "S +5 %", "D" EAC "G" ACI "TS SUBIS -20 %", "IMMOBILE : COUP +50 %", "APR" EGR "S TUER : VITESSE",
    "FIOLES SOIGNENT PLUS", "BUTIN +25 %"
};

static const uint8_t ZONE_ROOMS[NZONES] = { 5, 6, 6, 7, 6 };
static const uint8_t ZONE_ARENA[NZONES] = { 0, 0, 1, 0, 1 };
/* poids des ennemis par zone :        JAIL XBOW RAT BRUTE SLIME GREN SHLD TOXIC BAT */
static const uint8_t EW[NZONES][9] = { {40, 0, 25, 0, 0, 0, 15, 20, 0}, {25, 25, 0, 0, 0, 20, 15, 0, 15}, {0, 15, 15, 0, 25, 0, 0, 30, 15},
                                       {0, 25, 0, 15, 0, 25, 20, 0, 15}, {0, 15, 0, 20, 0, 20, 20, 10, 15} };

/* ================================================================= sauvegarde */
static uint16_t crc16(const uint8_t *p, int n) {
    uint16_t c = 0xFFFF;
    while (n--) { c ^= *p++; for (int i = 0; i < 8; i++) c = (c & 1) ? (uint16_t)((c >> 1) ^ 0xA001) : (uint16_t)(c >> 1); }
    return c;
}
static void save_defaults(void) {
    memset(&G.sv, 0, sizeof G.sv);
    G.sv.magic = SAVE_MAGIC; G.sv.version = 2; G.sv.opt_fx = 1;
    G.sv.unlocked = (uint16_t)((1 << WP_SWORD) | (1 << WP_DAGGER)); G.sv.found = G.sv.unlocked;
}
static void save_load(void) {
    Save t;
    int n = plat_load(&t, (int)sizeof t);
    if (n == (int)sizeof t && t.magic == SAVE_MAGIC && t.crc == crc16((const uint8_t *)&t, (int)sizeof t - 2)) {
        G.sv = t; G.save_ok = 1;
        G.sv.unlocked |= (uint16_t)((1 << WP_SWORD) | (1 << WP_DAGGER));
    } else { save_defaults(); G.save_ok = (n >= 0); }
}
void save_store(void) {
    G.sv.crc = crc16((const uint8_t *)&G.sv, (int)sizeof G.sv - 2);
    G.save_ok = (plat_store(&G.sv, (int)sizeof G.sv) == 0);
}

/* ================================================================= objets : niveau I..XV et qualite + / ++ / S / L */
static const char *const ROM[16] = { "", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII", "XIII", "XIV", "XV" };
const char *roman(int n) { return ROM[clampi(n, 0, 15)]; }
void item_label(const Item *it, char *out) {
    static const char *const Q[5] = { "", "+", "++", "S", "L" };
    const char *r = roman(it->lvl), *q = Q[it->q > 4 ? 0 : it->q];
    int i = 0;
    while (*r) out[i++] = *r++;
    while (*q) out[i++] = *q++;
    out[i] = 0;
}
int item_power(const Item *it) {
    static const uint8_t QB[5] = { 0, 2, 4, 6, 6 };
    return it->lvl + QB[it->q > 4 ? 0 : it->q];
}
void item_make(Item *it, int id, int bonus, int force_q) {
    int ql = G.sv.up[UP_QUAL], r = rn(100), q = 0;
    it->id = (uint8_t)id;
    it->lvl = (uint8_t)clampi(1 + 2 * G.zone + bonus + rn(3), 1, 15);
    if (r < 4 * ql) q = 3;
    else if (r < 4 * ql + 4 + 6 * ql) q = 2;
    else if (r < 4 * ql + 4 + 6 * ql + 14 + 10 * ql) q = 1;
    it->q = (uint8_t)(force_q >= 0 ? force_q : q);
}
int random_weapon(int not_a, int not_b) {
    int w, tries = 0;
    do {
        do { w = rn(WP_COUNT); } while (!((G.sv.unlocked >> w) & 1));
        tries++;
    } while ((w == not_a || w == not_b) && tries < 12);
    return w;
}
int has_mut(int id) {
    for (int i = 0; i < G.mut_n; i++) if (G.mut[i] == id) return 1;
    return 0;
}

/* ================================================================= effets visuels */
static int pa_next;
static Part *new_part(void) {
    Part *a = 0;
    for (int i = 0; i < MAX_PA; i++) if (G.pa[i].life == 0) { a = &G.pa[i]; break; }
    if (!a) { a = &G.pa[pa_next]; pa_next = (pa_next + 1) % MAX_PA; }
    memset(a, 0, sizeof *a);
    return a;
}
void add_part(int kind, int x, int y, int vx, int vy, int life, int col) {
    Part *a = new_part();
    a->x = F(x); a->y = F(y); a->vx = (int16_t)vx; a->vy = (int16_t)vy;
    a->life = (uint8_t)life; a->max = (uint8_t)life; a->kind = (uint8_t)kind; a->col = (uint8_t)col;
}
static void fx_set(int sb, int nf, int x, int y, int delay, int flip) {
    if (!G.sv.opt_fx && sb != SB_FX_SLASH && sb != SB_FX_SMASH && sb != SB_FX_EXPL) return;
    Part *a = new_part();
    a->x = F(x); a->y = F(y); a->life = (uint8_t)(nf * delay); a->max = a->life; a->kind = PA_FX;
    a->col = (uint8_t)flip; a->nf = (uint8_t)nf; a->val = (uint8_t)delay; a->num = (int16_t)sb;
}
void add_fxf(int sb, int nf, int x, int y, int delay, int flip) { fx_set(sb, nf, x, y, delay, flip); }
void add_fx(int sb, int nf, int x, int y, int delay) { fx_set(sb, nf, x, y, delay, 0); }
void add_num(int x, int y, int val, int col) {
    Part *a = new_part();
    if (val > 999) val = 999;
    a->x = F(x); a->y = F(y); a->vy = -10; a->life = 26; a->max = 26; a->kind = PA_NUM; a->col = (uint8_t)col; a->num = (int16_t)val;
}
void burst(int x, int y, int n, int col) {
    if (!G.sv.opt_fx) return;
    for (int i = 0; i < n; i++) add_part(PA_SPARK, x, y, rn(81) - 40, -rn(70) - 10, 10 + rn(10), col);
}

/* ================================================================= physique des boites */
int tile_at(int tx, int ty) {
    if (tx < 0 || tx >= G.lvw || ty < 0) return TL_SOLID;
    if (ty >= LV_H) return TL_EMPTY;
    return G.map[ty][tx];
}
int is_solid(int t) { return t == TL_SOLID || t == TL_GATE; }

int move_box(int32_t *x, int32_t *y, int32_t *vx, int32_t *vy, int w, int h, int drop) {
    int fl = 0, hw = w / 2;
    if (*vx) {
        int32_t nx = *x + *vx;
        int x0 = PX(nx) - hw, x1 = x0 + w - 1, y0 = PX(*y) - h, y1 = PX(*y) - 1;
        int tx = (*vx > 0) ? (x1 >> 4) : (x0 >> 4), hit = 0;
        for (int ty = (y0 >> 4); ty <= (y1 >> 4); ty++) if (is_solid(tile_at(tx, ty))) { hit = 1; break; }
        if (hit) { nx = (*vx > 0) ? F(tx * TS - hw) : F((tx + 1) * TS + hw); *vx = 0; fl |= MB_WALL; }
        *x = nx;
    }
    if (*vy) {
        int32_t ny = *y + *vy;
        int x0 = PX(*x) - hw, x1 = x0 + w - 1, tx0 = x0 >> 4, tx1 = x1 >> 4;
        if (*vy > 0) {
            int feet = PX(ny), oldfeet = PX(*y), ty = (feet - 1) >> 4, hit = 0;
            for (int tx = tx0; tx <= tx1 && !hit; tx++) {
                int t = tile_at(tx, ty);
                if (is_solid(t)) hit = 1;
                else if (t == TL_PLAT && !drop && oldfeet <= ty * TS && feet > ty * TS) hit = 1;
            }
            if (hit) { ny = F(ty * TS); *vy = 0; fl |= MB_GROUND; }
        } else {
            int top = PX(ny) - h, ty = top >> 4, hit = 0;
            for (int tx = tx0; tx <= tx1 && !hit; tx++) if (is_solid(tile_at(tx, ty))) hit = 1;
            if (hit) { ny = F((ty + 1) * TS + h); *vy = 0; fl |= MB_CEIL; }
        }
        *y = ny;
    }
    return fl;
}

int player_box_overlap(int x, int y, int w, int h) {
    int px = PX(G.p.x), py = PX(G.p.y);
    return x < px + 5 && x + w > px - 5 && y < py && y + h > py - 20;
}

/* ================================================================= niveaux semi-aleatoires + passages */
static int pick_enemy(int zone) {
    int tot = 0;
    for (int i = 0; i < 9; i++) tot += EW[zone][i];
    int r = rn(tot);
    for (int i = 0; i < 9; i++) { if (r < EW[zone][i]) return i; r -= EW[zone][i]; }
    return ET_JAIL;
}

static void level_reset(int zone) {
    memset(G.map, 0, sizeof G.map); memset(G.en, 0, sizeof G.en); memset(G.pr, 0, sizeof G.pr);
    memset(G.pa, 0, sizeof G.pa); memset(G.pk, 0, sizeof G.pk);
    G.zone = (uint8_t)zone; G.arena_on = 0; G.boss_alive = 0; G.boss_idx = -1; G.arena_x0 = -1;
    G.door_tx = G.door_ty = -1; G.torch_n = 0; G.passage = 0; G.hint_t = 0;
}
static void level_finish_player(void) {
    Player *p = &G.p;
    p->vx = p->vy = 0; p->st = PS_NORMAL; p->inv = 60; p->t = p->atk_t = p->hold = p->buf_atk = 0; p->ground = 0; p->hitmask = 0; p->guard = 0;
    G.cam_x = F(imax(0, imin(PX(p->x) - 160, G.lvw * TS - SCR_W)));
}

void level_build(int zone) {
    level_reset(zone);
    G.nrooms = ZONE_ROOMS[zone]; G.lvw = (int16_t)(G.nrooms * ROOM_W);
    int seq[MAX_ROOMS], last = -1, n = G.nrooms;
    seq[0] = 0; seq[n - 1] = ZONE_ARENA[zone] ? 2 : 1;
    for (int i = 1; i < n - 1; i++) { int r; do { r = 3 + rn(10); } while (r == last); seq[i] = r; last = r; }
    int pct = 62 + 7 * zone;
    for (int i = 0; i < n; i++) {
        for (int y = 0; y < LV_H; y++) for (int x = 0; x < ROOM_W; x++) {
            char ch = DC_ROOMS[seq[i]][y][x];
            int tx = i * ROOM_W + x, t = TL_EMPTY;
            switch (ch) {
                case '#': t = TL_SOLID; break;
                case '=': t = TL_PLAT; break;
                case '^': t = TL_SPIKE; break;
                case 'D': t = TL_DOOR; break;
                case 'T': t = TL_TORCH; if (G.torch_n < 24) { G.torch_x[G.torch_n] = (int16_t)(tx * TS + 4); G.torch_y[G.torch_n] = (int16_t)(y * TS + 6); G.torch_n++; } break;
                default: break;
            }
            G.map[y][tx] = (uint8_t)t;
            int cx = tx * TS + 8, fy = (y + 1) * TS;
            if (ch == 'S') { G.p.x = F(cx); G.p.y = F(fy); }
            else if (ch == 'e' && rn(100) < pct) spawn_enemy(pick_enemy(zone), tx, y, zone);
            else if (ch == 'E') spawn_enemy(pick_enemy(zone), tx, y, zone);
            else if (ch == 'c') add_pick(PK_CHEST, cx, fy, 0, 0, 0);
            else if (ch == 'f') add_pick(PK_FLASK, cx, fy - 6, 0, 0, 0);
            else if (ch == 'h') add_pick(PK_HEART, cx, fy - 6, 0, 0, 0);
            else if (ch == 'o') { for (int k = -1; k <= 1; k++) add_pick(PK_COIN, cx + k * 10, fy - 8, 1, 0, 0); }
            else if (ch == 'B') {
                spawn_enemy(zone == 2 ? ET_GOB : ET_KING, tx, y, zone);
                for (int k = 0; k < MAX_EN; k++) if (G.en[k].alive && G.en[k].type >= ET_GOB) { G.boss_idx = (int16_t)k; G.en[k].st = ES_SLEEP; }
            }
            else if (ch == 'd') { G.door_tx = (int16_t)tx; G.door_ty = (int16_t)y; }
        }
        if (seq[i] == 2) G.arena_x0 = (int16_t)(i * ROOM_W * TS);
    }
    for (int y = 0; y < LV_H; y++) { G.map[y][0] = TL_SOLID; G.map[y][G.lvw - 1] = TL_SOLID; }
    if (zone == 2) for (int x = 0; x < G.lvw; x++) if (G.map[13][x] == TL_EMPTY && G.map[14][x] == TL_EMPTY) { G.map[13][x] = TL_TOXIC; G.map[14][x] = TL_SOLID; }
    level_finish_player();
    G.hint_t = zone == 0 ? 260 : 0;
    banner(ZONE_NAME[zone], 100);
}

static void build_passage(int zone) {          /* salle sure : PNJ des cellules (Passeur) et des mutations (Oracle) */
    level_reset(zone);
    G.passage = 1; G.nrooms = 1; G.lvw = ROOM_W;
    for (int y = 0; y < LV_H; y++) for (int x = 0; x < ROOM_W; x++) {
        char ch = DC_ROOMS[13][y][x];
        int t = TL_EMPTY, cx = x * TS + 8, fy = (y + 1) * TS;
        switch (ch) {
            case '#': t = TL_SOLID; break;
            case 'D': t = TL_DOOR; break;
            case 'T': t = TL_TORCH; if (G.torch_n < 24) { G.torch_x[G.torch_n] = (int16_t)(x * TS + 4); G.torch_y[G.torch_n] = (int16_t)(y * TS + 6); G.torch_n++; } break;
            case 'S': G.p.x = F(cx); G.p.y = F(fy); break;
            case 'N': add_pick(PK_NPC, cx, fy, 0, 0, 0); break;
            case 'M': add_pick(PK_NPC, cx, fy, 1, 0, 0); break;
            default: break;
        }
        G.map[y][x] = (uint8_t)t;
    }
    for (int y = 0; y < LV_H; y++) { G.map[y][0] = TL_SOLID; G.map[y][G.lvw - 1] = TL_SOLID; }
    level_finish_player();
    banner("PASSAGE", 90);
}

/* ================================================================= joueur */
#define P_W 10
#define P_H 20
#define GRAV 9
#define JUMP_V 124
#define JUMP2_V 104
#define MAXRUN 42
#define MAXFALL 140
#define HOLD_T 14

static void run_end(int win);
static void leave_passage(void);
static void zone_clear(void);

static const Wpn *cur_wpn(void) { return &WPN[G.p.w[G.p.cur].id]; }
static const Atk *cur_atk(const Wpn *w) { const Player *p = &G.p; return p->atk_b ? &w->b : (p->heavy ? &w->h : &w->l); }

static int dmg_of(const Atk *a, const Item *it) {
    int d = a->dmg * (100 + 6 * (item_power(it) - 1)) / 100;
    d = d * (100 + 8 * G.sv.up[UP_DMG]) / 100;
    int pct = 100 + G.p.buff_pct + (has_mut(MU_COMBO) ? 5 * G.p.combo : 0);
    return imax(1, d * pct / 100);
}

static void start_attack(int heavy, int b) {
    Player *p = &G.p; const Wpn *w = cur_wpn();
    int now = b ? 2 : 1;
    p->st = PS_ATK; p->atk_t = 0; p->heavy = (uint8_t)heavy; p->atk_b = (uint8_t)b; p->hitmask = 0; p->buf_atk = 0;
    if (G.keys & K_L) p->dir = -1; else if (G.keys & K_R) p->dir = 1;
    p->crit_next = (uint8_t)(w->kind == WK_DUAL && p->last_atk && p->last_atk != now);   /* faux-serres : critique en alternant */
    p->last_atk = (uint8_t)now; p->dual_t = 50;
    p->buff_pct = 0;
    if (has_mut(MU_REVENGE) && p->mt_veng > 0) { p->buff_pct += 60; p->mt_veng = 0; }
    if (has_mut(MU_DISENGAGE) && p->mt_dis > 0) { p->buff_pct += 40; p->mt_dis = 0; }
    if (has_mut(MU_CALM) && p->mt_calm >= 20) { p->buff_pct += 50; p->mt_calm = 0; }
    const Atk *a = cur_atk(w);
    if (a->lunge) p->vx = p->dir * a->lunge * 16;
}

static void nearest_target(int range_x, int range_y, int dir, Ent **out) {
    int best = 99999; *out = 0;
    int px = PX(G.p.x), py = PX(G.p.y) - 10;
    for (int i = 0; i < MAX_EN; i++) {
        Ent *e = &G.en[i];
        if (!e->alive || e->st == ES_DEAD || e->st == ES_SLEEP) continue;
        int dx = PX(e->x) - px, dy = PX(e->y) - 10 - py;
        if (dir && dx * dir < -8) continue;
        if (iabs(dx) > range_x || iabs(dy) > range_y) continue;
        if (iabs(dx) < best) { best = iabs(dx); *out = e; }
    }
}

static void strike(Ent *e, const Atk *a, const Item *it, int d, int crit, int dir) {
    Player *p = &G.p;
    if (it->q == 4 && WPN[it->id].leg == LG_CRIT3 && (++p->crit3 % 3) == 0) crit = 1;
    if (crit) d *= 2;
    g_crit = (uint8_t)crit;
    hurt_enemy(e, d, dir, a->kb, a->stun);
    g_crit = 0;
    if (e->st != ES_DEAD) {
        if (has_mut(MU_BLEED) && !e->burn) e->burn = 90;
        if (it->q == 4 && WPN[it->id].leg == LG_IGNITE) e->burn = 90;
    }
    if (it->q == 4 && WPN[it->id].leg == LG_VAMP) p->hp = (int16_t)imin(p->maxhp, p->hp + 1);
    if (has_mut(MU_COMBO)) { if (p->combo < 5) p->combo++; p->combo_t = 45; }
    G.hitstop = p->heavy ? 4 : 3;
}

static void attack_active(const Wpn *w, const Atk *a) {
    Player *p = &G.p; const Item *it = &p->w[p->cur];
    int px = PX(p->x), py = PX(p->y), dir = p->dir, dm = dmg_of(a, it), id = it->id;
    if (w->kind == WK_MELEE || w->kind == WK_SHIELD || w->kind == WK_DUAL) {
        int x0, x1, aoe = (id == WP_HAMMER && p->heavy);
        if (aoe) { x0 = px - a->reach; x1 = px + a->reach; }
        else if (dir > 0) { x0 = px + 2; x1 = px + 2 + a->reach; } else { x1 = px - 2; x0 = px - 2 - a->reach; }
        if (p->atk_t == a->a0) {
            if (id == WP_HAMMER) { add_fxf(SB_FX_SMASH, SN_FX_SMASH, px + (aoe ? 0 : dir * 24), py - 8, 3, 0); G.shake = aoe ? 8 : 5; }
            else if (id == WP_SWORD || id == WP_SCYTHE) add_fxf(SB_FX_SLASH, SN_FX_SLASH, px + dir * 11, py - 12, p->heavy ? 3 : 2, dir < 0);
            else add_fxf(SB_FX_HIT, 1, px + dir * 18, py - 12, 2, 0);
        }
        for (int i = 0; i < MAX_EN; i++) {
            Ent *e = &G.en[i];
            if (!e->alive || e->st == ES_DEAD || e->st == ES_SLEEP || (p->hitmask & (1u << i))) continue;
            int ew = ent_w(e), eh = ent_h(e), ex = PX(e->x), ey = PX(e->y);
            if (e->type == ET_BAT) ey += 6;
            if (x0 < ex + ew / 2 && x1 > ex - ew / 2 && py + a->y0 < ey && py + a->y1 > ey - eh) {
                int d = dm, crit = p->crit_next;
                if (id == WP_DAGGER && e->dir == dir) crit = 1;                         /* coup dans le dos */
                if (id == WP_RAPIER && iabs(ex - px) > a->reach * 70 / 100) d = d * 8 / 5;   /* pointe de la rapiere */
                p->hitmask |= (1u << i);
                strike(e, a, it, d, crit, dir);
                if (id != WP_HAMMER) G.shake = imax(G.shake, 2);
            }
        }
    } else if (p->atk_t == a->a0) {
        int sx = px + dir * 10, sy = py - 12;
        if (w->kind == WK_BOW) {
            if (id == WP_XBOW) add_proj(PJ_XBOLT, 0, sx, sy, dir * 150, 0, dm, 70, dir);
            else add_proj(p->heavy ? PJ_PARROW : PJ_ARROW, 0, sx, sy, dir * (p->heavy ? 144 : 112), 0, dm, 70, dir);
        } else if (w->kind == WK_FIRE) {
            if (!p->heavy) add_proj(PJ_FIRE, 0, sx, sy, dir * 80, 0, dm, 70, dir);
            else for (int k = -1; k <= 1; k++) add_proj(PJ_FIRE, 0, sx, sy, dir * 80, k * 18, dm, 70, dir);
        } else {                                                                      /* WK_ELEC */
            Ent *t1 = 0, *t2 = 0;
            if (!p->heavy) {
                nearest_target(150, 60, dir, &t1);
                if (t1) {
                    add_part(PA_LINE, sx, sy, PX(t1->x), PX(t1->y) - 10, 5, 0);
                    hurt_enemy(t1, dm, dir, 1, 8);
                    int idx = (int)(t1 - G.en);
                    for (int i = 0; i < MAX_EN; i++) {
                        Ent *e = &G.en[i];
                        if (i == idx || !e->alive || e->st == ES_DEAD || e->st == ES_SLEEP) continue;
                        if (iabs(PX(e->x) - PX(t1->x)) < 56 && iabs(PX(e->y) - PX(t1->y)) < 40) { t2 = e; break; }
                    }
                    if (t2) { add_part(PA_LINE, PX(t1->x), PX(t1->y) - 10, PX(t2->x), PX(t2->y) - 10, 5, 0); hurt_enemy(t2, imax(1, dm / 2), dir, 0, 6); }
                } else add_part(PA_LINE, sx, sy, sx + dir * 70, sy + rn(9) - 4, 4, 0);
            } else {
                for (int i = 0; i < MAX_EN; i++) {
                    Ent *e = &G.en[i];
                    if (!e->alive || e->st == ES_DEAD || e->st == ES_SLEEP) continue;
                    if (iabs(PX(e->x) - px) < 150 && iabs(PX(e->y) - py) < 80) {
                        add_part(PA_LINE, PX(e->x), py - 120, PX(e->x), PX(e->y) - 8, 6, 0);
                        hurt_enemy(e, dm, dir, 1, 10);
                    }
                }
                G.shake = 5;
            }
        }
    }
}

static int on_plat(void) {
    Player *p = &G.p; int feet = PX(p->y), x0 = PX(p->x) - 5, x1 = PX(p->x) + 4, plat = 0;
    if (feet % TS) return 0;
    for (int tx = x0 >> 4; tx <= (x1 >> 4); tx++) {
        int t = tile_at(tx, feet >> 4);
        if (is_solid(t)) return 0;
        if (t == TL_PLAT) plat = 1;
    }
    return plat;
}

static void player_respawn_room(void) {
    Player *p = &G.p;
    int room = clampi(PX(p->x) / (ROOM_W * TS), 0, G.nrooms - 1);
    p->x = F(room * ROOM_W * TS + 24); p->y = F(13 * TS); p->vx = p->vy = 0; p->inv = 50; p->st = PS_NORMAL; p->atk_t = 0;
}

int player_parry_window(int from_x) {
    Player *p = &G.p;
    return p->guard && p->guard_t <= 8 && ((from_x - PX(p->x)) * p->dir) >= -3;
}

void on_kill_hooks(void) {
    Player *p = &G.p;
    if (has_mut(MU_INSTINCT)) { p->hp = (int16_t)imin(p->maxhp, p->hp + 4); add_num(PX(p->x), PX(p->y) - 26, 4, 3); }
    if (has_mut(MU_ADREN)) p->mt_adr = 90;
}

static void open_npc(void) {
    Player *p = &G.p;
    for (int i = 0; i < MAX_PK; i++) {
        Pick *k = &G.pk[i];
        if (k->life && k->kind == PK_NPC && iabs(PX(k->x) - PX(p->x)) < 22) {
            G.in_run = 1; G.menu_sel = 0; G.menu_sub = 0;
            G.scene = (k->val == 0) ? SC_SHOP : SC_MUT;
            return;
        }
    }
}

static void player_update(void) {
    Player *p = &G.p; uint16_t k = G.keys, pr = G.pressed;
    const Wpn *w;
    p->anim++;
    if (p->st == PS_DEAD) {
        p->dead_t++; p->vy = imin(p->vy + GRAV, MAXFALL);
        p->vx = p->vx * 7 / 8;
        move_box(&p->x, &p->y, &p->vx, &p->vy, P_W, P_H, 0);
        return;
    }
    if (p->inv > 0) p->inv--;
    if (p->roll_cd > 0) p->roll_cd--;
    if (p->atk_cd > 0) p->atk_cd--;
    if (p->drop_t > 0) p->drop_t--;
    if (p->flip_t > 0) p->flip_t--;
    if (p->swap_t > 0) p->swap_t--;
    if (p->mt_veng > 0) p->mt_veng--;
    if (p->mt_dis > 0) p->mt_dis--;
    if (p->mt_adr > 0) p->mt_adr--;
    if (p->combo_t > 0 && --p->combo_t == 0) p->combo = 0;
    if (p->dual_t > 0 && --p->dual_t == 0) p->last_atk = 0;
    if (p->ground) { p->coyote = 5; p->jumps = 0; } else if (p->coyote > 0) p->coyote--;
    if (!p->ground && p->coyote == 0 && p->jumps == 0) p->jumps = 1;
    if (pr & K_U) p->jbuf = 5; else if (p->jbuf > 0) p->jbuf--;
    if (p->ground && p->st == PS_NORMAL && !(k & (K_L | K_R))) { if (p->mt_calm < 60) p->mt_calm++; } else if (p->st != PS_NORMAL || (k & (K_L | K_R))) p->mt_calm = 0;

    w = cur_wpn();
    int dual = (w->kind == WK_DUAL);
    if (w->kind == WK_SHIELD && (k & K_OK) && p->st == PS_NORMAL && !G.passage) { p->guard = 1; p->guard_t++; } else { p->guard = 0; p->guard_t = 0; }

    if (G.passage && (pr & K_OK) && p->st == PS_NORMAL) { open_npc(); if (G.scene != SC_PLAY) return; }

    /* ---- armes : changer d'arme, 2e attaque de la faux-serres, fiole */
    if (p->st == PS_NORMAL) {
        if ((pr & K_SWAP) && !G.passage) {
            if (dual) { if (p->atk_cd == 0) start_attack(0, 1); }
            else if (p->swap_t == 0) {
                char buf[32], lab[8]; const char *nm = WPN[p->w[p->cur ^ 1].id].name; int i = 0;
                p->cur ^= 1; p->swap_t = 12;
                item_label(&p->w[p->cur], lab);
                while (*nm && i < 22) { buf[i] = *nm++; i++; }
                buf[i++] = ' '; for (int j = 0; lab[j] && i < 31; j++) buf[i++] = lab[j];
                buf[i] = 0; msg(buf);
            }
        }
        if ((pr & K_FLASK) && p->flasks > 0 && p->hp < p->maxhp) {
            int heal = p->maxhp * (has_mut(MU_GLUTTON) ? 65 : 50) / 100;
            p->hp = (int16_t)imin(p->maxhp, p->hp + heal); p->flasks--;
            add_num(PX(p->x), PX(p->y) - 26, heal, 3); burst(PX(p->x), PX(p->y) - 12, 8, 3);
        }
    }

    /* ---- attaque : declenchee au relachement de OK (maintenu = attaque lourde ; bouclier : maintenu = garde) */
    if (!G.passage) {
        if (k & K_OK) { if (p->hold < 250) p->hold++; }
        if ((G.prev & K_OK) && !(k & K_OK)) {
            int hold = p->hold;
            if (p->st == PS_NORMAL && p->atk_cd == 0) {
                if (w->kind == WK_SHIELD) { if (hold < HOLD_T) start_attack(0, 0); }
                else start_attack(hold >= HOLD_T, 0);
            } else if (p->st == PS_ATK) p->buf_atk = 10;
            p->hold = 0;
        }
        if (p->st == PS_NORMAL && p->buf_atk > 0 && p->atk_cd == 0 && p->hold == 0 && w->kind != WK_SHIELD) start_attack(0, 0);
    }
    if (p->buf_atk > 0) p->buf_atk--;
    w = cur_wpn();

    /* ---- esquive (Retour) */
    if ((pr & K_BACK) && p->roll_cd == 0 && p->st != PS_HURT && p->st != PS_ROLL) {
        const Atk *a = cur_atk(w);
        if (p->st == PS_NORMAL || (p->st == PS_ATK && p->atk_t >= a->a1)) {
            int d = (k & K_L) ? -1 : (k & K_R) ? 1 : p->dir;
            p->dir = (int8_t)d; p->st = PS_ROLL; p->t = 0; p->atk_t = 0; p->hold = 0; p->guard = 0;
            p->inv = (int16_t)imax(p->inv, 10 + 2 * G.sv.up[UP_DODGE]);
            add_part(PA_SMOKE, PX(p->x), PX(p->y) - 2, -d * 12, -4, 14, 0);
        }
    }

    /* ---- horizontal */
    int want = ((k & K_R) ? 1 : 0) - ((k & K_L) ? 1 : 0);
    if (p->st == PS_ROLL) {
        p->vx = p->dir * 67; p->t++;
        if (p->t >= 14) { p->st = PS_NORMAL; p->roll_cd = (int16_t)(20 - 3 * G.sv.up[UP_DODGE]); p->vx = p->dir * 20; if (has_mut(MU_DISENGAGE)) p->mt_dis = 60; }
    } else if (p->st == PS_HURT) {
        p->t--; p->vx = p->vx * 15 / 16; if (p->t <= 0) p->st = PS_NORMAL;
    } else {
        const Atk *a = cur_atk(w);
        int lunging = (p->st == PS_ATK && a->lunge && p->atk_t <= a->a1);
        int maxs = MAXRUN;
        if (p->st == PS_ATK) maxs = MAXRUN * 2 / 5;
        else if (p->guard) maxs = MAXRUN / 2;
        else if (has_mut(MU_ADREN) && p->mt_adr > 0) maxs = MAXRUN * 120 / 100;
        if (!lunging) {
            if (want) {
                int target = want * maxs, acc = p->ground ? 7 : 5;
                if (p->vx < target) p->vx = imin(target, (int)p->vx + acc); else if (p->vx > target) p->vx = imax(target, (int)p->vx - acc);
                if (p->st == PS_NORMAL) p->dir = (int8_t)want;
            } else {
                int fr = p->ground ? 8 : 3;
                if (p->vx > 0) p->vx = imax(0, (int)p->vx - fr); else if (p->vx < 0) p->vx = imin(0, (int)p->vx + fr);
            }
        }
        if (p->st == PS_ATK) {
            p->atk_t++;
            if (p->atk_t >= a->a0 && p->atk_t <= a->a1) attack_active(w, a);
            if (p->atk_t >= a->total) { p->st = PS_NORMAL; p->atk_cd = 0; }
        }
    }

    /* ---- saut / double saut / traversee de plateforme */
    if (p->st == PS_NORMAL || p->st == PS_ATK) {
        if ((pr & K_D) && p->ground && on_plat()) { p->drop_t = 10; p->ground = 0; p->y += F(2); p->jbuf = 0; p->coyote = 0; }
        if (p->jbuf > 0) {
            if (p->ground || p->coyote > 0) {
                p->vy = -JUMP_V; p->ground = 0; p->coyote = 0; p->jbuf = 0; p->jumps = 1;
                add_fx(SB_FX_DUST, SN_FX_DUST, PX(p->x), PX(p->y) - 3, 3);
            } else if (p->jumps < 2) {
                p->vy = -JUMP2_V; p->jumps = 2; p->jbuf = 0; p->flip_t = 16;
                add_fx(SB_FX_DUST, SN_FX_DUST, PX(p->x), PX(p->y) - 2, 3);
            }
        }
        if (!(k & K_U) && p->vy < -48 && p->jumps == 1) p->vy = -48;
    }

    /* ---- gravite + deplacement */
    p->vy = imin(p->vy + GRAV, MAXFALL);
    int was_air = !p->ground, vy_before = (int)p->vy;
    int fl = move_box(&p->x, &p->y, &p->vx, &p->vy, P_W, P_H, p->drop_t > 0);
    p->ground = (fl & MB_GROUND) != 0;
    if (p->ground && was_air && vy_before > 70) add_fx(SB_FX_DUST, SN_FX_DUST, PX(p->x), PX(p->y) - 3, 3);

    /* ---- dangers (piques, liquide toxique, fosses) */
    int px = PX(p->x), py = PX(p->y);
    if (p->inv == 0 && p->st != PS_DEAD) {
        for (int ty = (py - 20) >> 4; ty <= ((py - 1) >> 4); ty++) for (int tx = (px - 5) >> 4; tx <= ((px + 4) >> 4); tx++) {
            int t = tile_at(tx, ty);
            if (t == TL_SPIKE && px + 5 > tx * TS + 2 && px - 5 < tx * TS + 14 && py > ty * TS + 6) { hurt_player(18, px, 0); G.p.vy = -90; }
            else if (t == TL_TOXIC && py > ty * TS + 3) { hurt_player(7, px, 0); G.p.vy = -70; }
        }
    }
    if (py > (LV_H + 2) * TS && p->st != PS_DEAD) {
        p->hp = (int16_t)(p->hp - p->maxhp * 35 / 100);
        if (p->hp <= 0) { p->hp = 0; p->st = PS_DEAD; p->dead_t = 0; p->vy = 0; p->y = F((LV_H + 2) * TS); }
        else { player_respawn_room(); msg("CHUTE !"); G.shake = 6; }
    }
    if (p->st != PS_DEAD && !G.boss_alive && tile_at(px >> 4, (py - 1) >> 4) == TL_DOOR) { if (G.passage) leave_passage(); else zone_clear(); }
}

void hurt_player(int dmg, int from_x, int kb) {
    Player *p = &G.p;
    if (p->inv > 0 || p->st == PS_DEAD || G.scene != SC_PLAY || G.passage) return;
    int blocked = 0, front = ((from_x - PX(p->x)) * p->dir) >= -3;
    if (p->guard && front && p->st == PS_NORMAL) {
        if (p->guard_t <= 8) {                                     /* PARADE parfaite : aucun degat, l'attaquant est etourdi */
            if (g_src && g_src->type < ET_GOB) { g_src->st = ES_HURT; g_src->t = 50; g_src->hit = 0; g_src->vx = 0; }
            p->inv = 14; G.hitstop = 5; G.shake = 4;
            burst(PX(p->x) + p->dir * 8, PX(p->y) - 12, 10, 2); add_fx(SB_FX_HIT, SN_FX_HIT, PX(p->x) + p->dir * 8, PX(p->y) - 12, 2);
            msg("PARADE !");
            return;
        }
        dmg = dmg * 20 / 100; kb /= 2; blocked = 1;                /* garde : 80 % des degats absorbes */
    }
    if (G.sv.opt_easy) dmg = dmg * 65 / 100;
    if (has_mut(MU_CARAPACE)) dmg = dmg * 80 / 100;
    if (dmg < 1) dmg = 1;
    p->hp = (int16_t)(p->hp - dmg); p->atk_t = 0; p->hold = 0; p->buf_atk = 0; p->combo = 0;
    if (has_mut(MU_REVENGE)) p->mt_veng = 90;
    int d = (PX(p->x) >= from_x) ? 1 : -1;
    G.shake = 6;
    add_num(PX(p->x), PX(p->y) - 24, dmg, 1); burst(PX(p->x), PX(p->y) - 12, 7, 1);
    if (blocked && p->hp > 0) { p->inv = 14; p->vx = d * kb * 16; add_fx(SB_FX_HIT, SN_FX_HIT, PX(p->x) + p->dir * 8, PX(p->y) - 12, 2); return; }
    p->inv = 36; G.hitstop = 4; p->vx = d * kb * 16; if (kb) p->vy = -60;
    add_fx(SB_FX_HIT, SN_FX_HIT, PX(p->x), PX(p->y) - 12, 2);
    if (p->hp <= 0) { p->hp = 0; p->st = PS_DEAD; p->dead_t = 0; p->vy = -80; p->vx = d * 30; }
    else { p->st = PS_HURT; p->t = 10; }
}

/* ================================================================= arene de boss */
int boss_trigger_check(void) {
    if (G.arena_x0 < 0 || G.arena_on || G.boss_idx < 0 || !G.en[G.boss_idx].alive) return 0;
    if (PX(G.p.x) > G.arena_x0 + 4 * TS) {
        G.arena_on = 1; G.boss_alive = 1;
        for (int y = 2; y <= 12; y++) G.map[y][G.arena_x0 / TS] = TL_GATE;
        Ent *b = &G.en[G.boss_idx]; b->st = ES_IDLE; b->t = 70;
        banner(b->type == ET_GOB ? "LE GOBEUR" : "LE ROI F" ECI "L" EAC, 110);
        G.shake = 8;
        return 1;
    }
    return 0;
}

/* ================================================================= deroulement d'une partie */
static void mut_offer(void) {
    int cand[MU_COUNT], c = 0;
    for (int m = 0; m < MU_COUNT; m++) if (!has_mut(m)) cand[c++] = m;
    for (int i = 0; i < 3; i++) {
        if (c == 0) { G.mut_off[i] = 255; continue; }
        int j = rn(c); G.mut_off[i] = (uint8_t)cand[j]; cand[j] = cand[--c];
    }
}

static void bank_run(void) {                  /* a l'entree d'un passage : cellules et schemas portes sont mis en lieu sur */
    G.sv.cells += (uint32_t)G.cells_run; G.cells_run = 0;
    G.sv.found |= G.bp_carry; G.bp_carry = 0;
    if (G.zone + 2 > G.sv.best_zone) G.sv.best_zone = (uint8_t)(G.zone + 2);
    save_store();
}

static void zone_clear(void) {
    Player *p = &G.p;
    if (G.zone + 1 >= NZONES) { run_end(1); return; }
    int hp = p->hp, fl = p->flasks;
    bank_run();
    build_passage(G.zone);
    p->hp = (int16_t)imin(p->maxhp, hp + p->maxhp * 30 / 100); p->flasks = (uint8_t)imin(p->maxflasks, fl + 1);
    G.mut_pending = (uint8_t)(G.mut_n < 4); G.reroll_n = 0;
    if (G.mut_pending) mut_offer();
    msg("CELLULES SAUVEGARD" EAC "ES");
}

static void leave_passage(void) {
    Player *p = &G.p;
    int hp = p->hp, fl = p->flasks, nz = G.zone + 1;
    level_build(nz);
    p->hp = (int16_t)hp; p->flasks = (uint8_t)fl;
}

static void run_end(int win) {
    int carried = (int)G.cells_run, keep = win ? carried : carried * 30 / 100;
    G.bank_gain = keep; G.bank_lost = carried - keep;
    G.sv.cells += (uint32_t)keep; G.cells_run = 0;
    if (win) G.sv.found |= G.bp_carry;
    G.bp_carry = 0;
    G.sv.runs++; if (win) G.sv.wins++; else G.sv.deaths++;
    G.sv.kills += (uint32_t)G.run_kills;
    if (G.zone + 1 > G.sv.best_zone) G.sv.best_zone = (uint8_t)(G.zone + 1);
    save_store();
    G.scene = win ? SC_WIN : SC_DEAD; G.menu_sel = 0; G.win_t = 0; G.in_run = 0;
    G.victory = (uint8_t)win;
}

static void run_start(void) {
    Player *p = &G.p;
    memset(p, 0, sizeof *p);
    p->maxhp = (int16_t)(100 + 10 * G.sv.up[UP_HP]); p->hp = p->maxhp;
    p->maxflasks = (uint8_t)(2 + G.sv.up[UP_FLASK]); p->flasks = p->maxflasks; p->dir = 1;
    G.run_gold = 0; G.run_kills = 0; G.cells_run = 0; G.bp_carry = 0; G.run_time = 0; G.victory = 0; G.shake = G.hitstop = 0;
    G.mut_n = 0; G.mut_pending = 0; G.in_run = 1; G.zone = 0;
    {   /* armes de depart au hasard parmi celles debloquees grace aux schemas */
        int melee[5] = { WP_SWORD, WP_DAGGER, WP_RAPIER, WP_HAMMER, WP_SCYTHE }, pool[5], n = 0;
        for (int i = 0; i < 5; i++) if ((G.sv.unlocked >> melee[i]) & 1) pool[n++] = melee[i];
        int a = pool[rn(n)];
        item_make(&p->w[0], a, 0, -1);
        if (a == WP_SCYTHE) p->w[1] = p->w[0];
        else {
            int b; do { b = random_weapon(a, -1); } while (b == WP_SCYTHE && n > 1 && rn(4));
            if (b == WP_SCYTHE) b = (a == WP_SWORD) ? WP_DAGGER : WP_SWORD;
            item_make(&p->w[1], b, 0, -1);
        }
    }
    level_build(0);
    G.scene = SC_PLAY;
}

/* ================================================================= boutique du Passeur (cellules) */
int shop_rows(uint8_t *kind, uint8_t *arg) {
    int n = 0;
    for (int u = 0; u < UP_COUNT; u++) { kind[n] = RW_UP; arg[n++] = (uint8_t)u; }
    for (int w = 0; w < WP_COUNT; w++) if (((G.sv.found >> w) & 1) && !((G.sv.unlocked >> w) & 1)) { kind[n] = RW_BP; arg[n++] = (uint8_t)w; }
    if (G.in_run) { kind[n] = RW_WLVL; arg[n++] = 0; kind[n] = RW_WQ; arg[n++] = 0; }
    kind[n] = RW_BACK; arg[n++] = 0;
    return n;
}
int shop_price(int kind, int arg, int *gold) {
    const Item *it = &G.p.w[G.p.cur];
    *gold = 0;
    switch (kind) {
        case RW_UP: return G.sv.up[arg] >= UP_MAX[arg] ? -1 : UP_COST[arg][G.sv.up[arg]];
        case RW_BP: return BP_COST[arg];
        case RW_WLVL: *gold = 1; return it->lvl >= 15 ? -1 : 20 + 12 * it->lvl;
        case RW_WQ: *gold = 1; return it->q >= 3 ? -1 : 50 + 40 * it->q;
        default: return -1;
    }
}

static void nav(int n) {
    if (G.pressed & K_U) G.menu_sel = (int16_t)((G.menu_sel + n - 1) % n);
    if (G.pressed & K_D) G.menu_sel = (int16_t)((G.menu_sel + 1) % n);
}

static void leave_shop(void) {
    G.scene = G.in_run ? SC_PLAY : SC_TITLE; G.menu_sel = 1;
}

static void shop_update(void) {
    uint8_t k[24], a[24];
    int n = shop_rows(k, a);
    if (G.menu_sel >= n) G.menu_sel = (int16_t)(n - 1);
    nav(n);
    if (G.msg_t > 0) G.msg_t--;
    if (G.pressed & K_BACK) { leave_shop(); return; }
    if (!(G.pressed & K_OK)) return;
    int s = G.menu_sel, gold;
    if (k[s] == RW_BACK) { leave_shop(); return; }
    int price = shop_price(k[s], a[s], &gold);
    if (price < 0) { msg("AU MAXIMUM"); return; }
    if (gold ? G.run_gold < price : G.sv.cells < (uint32_t)price) { msg(gold ? "PAS ASSEZ DE PI" EGR "CES" : "PAS ASSEZ DE CELLULES"); return; }
    if (gold) G.run_gold -= price; else G.sv.cells -= (uint32_t)price;
    Player *p = &G.p;
    switch (k[s]) {
        case RW_UP:
            G.sv.up[a[s]]++;
            if (G.in_run && a[s] == UP_HP) { p->maxhp += 10; p->hp += 10; }
            if (G.in_run && a[s] == UP_FLASK) { p->maxflasks++; p->flasks++; }
            break;
        case RW_BP: G.sv.unlocked |= (uint16_t)(1 << a[s]); msg("ARME D" EAC "BLOQU" EAC "E"); break;
        case RW_WLVL: p->w[p->cur].lvl++; break;
        default: p->w[p->cur].q++; break;
    }
    if (p->w[p->cur].id == WP_SCYTHE) p->w[p->cur ^ 1] = p->w[p->cur];
    save_store();
}

/* ================================================================= mutations de l'Oracle (valables pour la partie seulement) */
static void mut_update(void) {
    int n = G.mut_pending ? 5 : 1;
    if (G.menu_sel >= n) G.menu_sel = (int16_t)(n - 1);
    nav(n);
    if (G.msg_t > 0) G.msg_t--;
    if (G.pressed & K_BACK) { G.scene = SC_PLAY; return; }
    if (!(G.pressed & K_OK)) return;
    int s = G.menu_sel;
    if (!G.mut_pending || s == 4) { G.scene = SC_PLAY; return; }
    if (s == 3) {                                                                         /* relancer le tirage contre des pieces */
        int cost = 40 * (G.reroll_n + 1);
        if (G.run_gold < cost) { msg("PAS ASSEZ DE PI" EGR "CES"); return; }
        G.run_gold -= cost; G.reroll_n++; mut_offer(); return;
    }
    if (G.mut_off[s] == 255) return;
    G.mut[G.mut_n++] = G.mut_off[s]; G.mut_pending = 0;
    if (G.mut_off[s] == MU_GLUTTON) { G.p.maxflasks++; G.p.flasks++; }
    msg(MU_NAME[G.mut_off[s]]);
    G.scene = SC_PLAY;
}

/* ================================================================= scenes */
static void cam_update(void) {
    int target;
    if (G.arena_on) target = G.arena_x0; else target = PX(G.p.x) - 160 + G.p.dir * 28;
    target = imax(0, imin(target, G.lvw * TS - SCR_W));
    G.cam_x += (F(target) - G.cam_x) / 5;
}

static void play_update(void) {
    if (G.pressed & K_PAUSE) { G.scene = SC_PAUSE; G.menu_sel = 0; return; }
    if (G.hitstop > 0) { G.hitstop--; return; }
    G.run_time++;
    player_update();
    if (G.scene != SC_PLAY) return;
    boss_trigger_check();
    ai_update();
    cam_update();
    if (G.banner_t > 0) G.banner_t--;
    if (G.msg_t > 0) G.msg_t--;
    if (G.hint_t > 0) G.hint_t--;
    if (G.shake > 0) G.shake--;
    if (G.p.st == PS_DEAD && G.p.dead_t > 70) run_end(0);
    else if (G.victory && G.scene == SC_PLAY && ++G.win_t > 110) run_end(1);
}

static void title_update(void) {
    nav(5);
    if (G.pressed & K_OK) {
        switch (G.menu_sel) {
            case 0: run_start(); break;
            case 1: G.in_run = 0; G.scene = SC_SHOP; G.menu_sel = 0; break;
            case 2: G.scene = SC_OPTIONS; G.menu_sel = 0; G.menu_sub = 0; break;
            case 3: G.scene = SC_HELP; break;
            default: G.quit = 1; break;
        }
    }
}

static void options_update(void) {
    nav(4);
    if (G.msg_t > 0) G.msg_t--;
    if (G.pressed & K_BACK) { G.scene = SC_TITLE; G.menu_sel = 2; return; }
    if (G.pressed & (K_OK | K_L | K_R)) {
        switch (G.menu_sel) {
            case 0: G.sv.opt_fx ^= 1; save_store(); break;
            case 1: G.sv.opt_easy ^= 1; save_store(); break;
            case 2:
                if (G.pressed & K_OK) {
                    if (G.menu_sub) { save_defaults(); save_store(); G.menu_sub = 0; msg("PROGRESSION EFFAC" EAC "E"); } else G.menu_sub = 1;
                }
                break;
            default: if (G.pressed & K_OK) { G.scene = SC_TITLE; G.menu_sel = 2; } break;
        }
    }
    if (G.menu_sel != 2) G.menu_sub = 0;
}

void game_update(uint16_t keys) {
    G.prev = G.keys; G.keys = keys; G.pressed = (uint16_t)(keys & ~G.prev); G.frame++;
    switch (G.scene) {
        case SC_TITLE: title_update(); break;
        case SC_PLAY: play_update(); break;
        case SC_PAUSE:
            nav(3);
            if (G.pressed & (K_PAUSE | K_BACK)) { G.scene = SC_PLAY; break; }
            if (G.pressed & K_OK) {
                if (G.menu_sel == 0) G.scene = SC_PLAY;
                else if (G.menu_sel == 1) { G.scene = SC_HELP; G.menu_sub = 1; }
                else { run_end(0); G.scene = SC_TITLE; G.menu_sel = 0; }
            }
            break;
        case SC_SHOP: shop_update(); break;
        case SC_MUT: mut_update(); break;
        case SC_OPTIONS: options_update(); break;
        case SC_HELP:
            if (G.pressed & (K_R | K_L)) G.help_page ^= 1;
            if (G.pressed & (K_BACK | K_OK)) { G.help_page = 0; if (G.menu_sub) { G.scene = SC_PAUSE; G.menu_sub = 0; } else { G.scene = SC_TITLE; G.menu_sel = 3; } }
            break;
        case SC_DEAD: case SC_WIN:
            G.win_t++;
            part_update();
            if (G.pressed & K_OK) { G.in_run = 0; G.scene = SC_SHOP; G.menu_sel = 0; }
            if (G.pressed & K_BACK) { G.scene = SC_TITLE; G.menu_sel = 0; }
            break;
        default: break;
    }
}

void game_init(uint32_t seed) {
    memset(&G, 0, sizeof G);
    G.rng = seed ? seed : 0x1234567u;
    save_load();
    G.scene = SC_TITLE; G.menu_sel = 0;
}
int game_quit_requested(void) { return G.quit; }
