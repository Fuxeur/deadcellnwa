/* dc_ai.c - ennemis (melee, distance, rapide, resistant, lanceur de bombes, garde-bouclier, zombie toxique, volant),
   elites, 2 boss, projectiles, objets (pieces, cellules, schemas, armes), particules. */
#include "dc.h"

typedef struct { uint8_t w, h; int16_t hp; uint8_t dmg, coins; } EDef;
static const EDef ED[ET_COUNT] = {
    {12, 22, 36, 12, 3},      /* ET_JAIL  : melee                         */
    {12, 22, 24, 10, 3},      /* ET_XBOW  : a distance                    */
    {14, 10, 14, 6, 1},       /* ET_RAT   : rapide                        */
    {20, 28, 90, 22, 8},      /* ET_BRUTE : resistant                     */
    {14, 12, 28, 8, 2},       /* ET_SLIME : egouts                        */
    {12, 22, 30, 18, 4},      /* ET_GREN  : artificier (bombes en cloche) */
    {14, 24, 55, 14, 5},      /* ET_SHLD  : garde-bouclier (garde frontale)*/
    {12, 22, 48, 10, 3},      /* ET_TOXIC : putride (nuage a sa mort)      */
    {14, 10, 14, 8, 1},       /* ET_BAT   : volant, pique sur le joueur    */
    {34, 30, 260, 14, 60},    /* ET_GOB   : boss des egouts               */
    {24, 46, 380, 22, 120},   /* ET_KING  : boss final                    */
};
int ent_w(const Ent *e) { return ED[e->type].w; }
int ent_h(const Ent *e) { return ED[e->type].h; }
int enemy_hp_scale(int base, int zone) { return base * (100 + 22 * zone) / 100; }
static int edmg(const Ent *e) { return ED[e->type].dmg * (100 + 10 * G.zone) / 100 * (e->elite ? 125 : 100) / 100; }


#define GRAV 9
#define MAXFALL 140

static int pdx(const Ent *e) { return PX(G.p.x) - PX(e->x); }
static int pdy(const Ent *e) { return (PX(G.p.y) - 10) - (PX(e->y) - ED[e->type].h / 2); }
static void face(Ent *e) { e->dir = (int8_t)((pdx(e) >= 0) ? 1 : -1); }
static int ground_ahead(const Ent *e, int dir, int dist) {
    int x = PX(e->x) + dir * (ED[e->type].w / 2 + dist + 1), t = tile_at(x >> 4, PX(e->y) >> 4);
    return is_solid(t) || t == TL_PLAT;
}
static int wall_ahead(const Ent *e, int dir) {
    int x = PX(e->x) + dir * (ED[e->type].w / 2 + 1);
    return is_solid(tile_at(x >> 4, (PX(e->y) - 4) >> 4));
}
static int touch(const Ent *e) {
    return player_box_overlap(PX(e->x) - ED[e->type].w / 2, PX(e->y) - ED[e->type].h, ED[e->type].w, ED[e->type].h);
}
static void en_phys(Ent *e) {
    e->vy = imin(e->vy + GRAV, MAXFALL);
    int fl = move_box(&e->x, &e->y, &e->vx, &e->vy, ED[e->type].w, ED[e->type].h, 0);
    e->ground = (fl & MB_GROUND) != 0;
    if (PX(e->y) > (LV_H + 3) * TS) e->alive = 0;
}

/* ================================================================= creation / objets */
void add_proj(int type, int owner, int x, int y, int vx, int vy, int dmg, int life, int dir) {
    for (int i = 0; i < MAX_PR; i++) if (G.pr[i].life == 0) {
        Proj *p = &G.pr[i];
        memset(p, 0, sizeof *p);
        p->x = F(x); p->y = F(y); p->vx = vx; p->vy = vy; p->dmg = (int16_t)dmg; p->life = (int16_t)life;
        p->type = (uint8_t)type; p->owner = (uint8_t)owner; p->dir = (int8_t)dir;
        p->pierce = (type == PJ_PARROW) ? 2 : 0;
        return;
    }
}
static Pick *new_pick(void) {
    for (int i = 0; i < MAX_PK; i++) if (G.pk[i].life == 0) { memset(&G.pk[i], 0, sizeof(Pick)); G.pk[i].life = 255; return &G.pk[i]; }
    return 0;
}
void add_pick(int kind, int x, int y, int val, int vx, int vy) {
    Pick *k = new_pick();
    if (!k) return;
    k->x = F(x); k->y = F(y); k->vx = vx; k->vy = vy; k->kind = (uint8_t)kind; k->val = (uint8_t)val;
}
void add_item_pick(int x, int y, const Item *it, int vy) {
    Pick *k = new_pick();
    if (!k) return;
    k->x = F(x); k->y = F(y); k->vy = vy; k->kind = PK_WPN; k->val = it->id; k->lvl = it->lvl; k->q = it->q;
}
void spawn_coins(int x, int y, int total) {
    int n = imin(total, 8), base, rem;
    if (n <= 0) return;
    base = total / n; rem = total % n;
    for (int i = 0; i < n; i++) add_pick(PK_COIN, x, y, base + (i < rem), rn(61) - 30, -(40 + rn(50)));
}
void spawn_cells(int x, int y, int n) {
    for (int i = 0; i < imin(n, 8); i++) add_pick(PK_CELL, x, y, 1, rn(61) - 30, -(50 + rn(50)));
}
void drop_blueprint(int x, int y) {            /* schema d'une arme encore verrouillee et pas deja trouvee */
    int cand[WP_COUNT], c = 0;
    for (int w = 0; w < WP_COUNT; w++) if (!((G.sv.unlocked >> w) & 1) && !((G.sv.found >> w) & 1) && !((G.bp_carry >> w) & 1)) cand[c++] = w;
    if (c) add_pick(PK_BLUE, x, y, cand[rn(c)], rn(31) - 15, -70);
}
void spawn_enemy(int type, int tx, int ty, int zone) {
    for (int i = 0; i < MAX_EN; i++) if (!G.en[i].alive) {
        Ent *e = &G.en[i];
        memset(e, 0, sizeof *e);
        e->type = (uint8_t)type; e->alive = 1;
        e->x = F(tx * TS + 8); e->y = F((ty + 1) * TS);
        e->hp = e->maxhp = (int16_t)enemy_hp_scale(ED[type].hp, zone);
        e->dir = (int8_t)(rn(2) ? 1 : -1); e->st = ES_PATROL; e->home = (int16_t)(tx * TS + 8);
        e->skin = (uint8_t)(type < ET_GOB ? zone : 0); e->cd = (int16_t)(30 + rn(40));
        if (type == ET_SLIME) e->st = ES_CHASE;
        if (type < ET_GOB && rn(100) < 5 + zone) { e->elite = 1; e->hp = e->maxhp = (int16_t)(e->maxhp * 22 / 10); }
        return;
    }
}

static void kill_enemy(Ent *e) {
    int boss = (e->type >= ET_GOB), x = PX(e->x), y = PX(e->y), cy = y - ED[e->type].h / 2;
    int loot = 100 + 10 * G.sv.up[UP_LOOT] + (has_mut(MU_LUCK) ? 25 : 0);
    e->st = ES_DEAD; e->t = 0; e->hit = 0; e->vy = -50; e->vx = 0; e->burn = 0;
    G.run_kills++;
    on_kill_hooks();
    spawn_coins(x, cy, ED[e->type].coins * (e->elite ? 2 : 1) * loot / 100);
    burst(x, cy, boss ? 24 : 8, 0);
    if (e->type == ET_TOXIC) add_proj(PJ_CLOUD, 1, x, y - 6, 0, 0, 5, 150, 1);       /* nuage de poison a la mort */
    if (!boss) {
        int r = rn(100), cell_ch = (e->type == ET_BRUTE || e->type == ET_GREN || e->type == ET_SHLD) ? 35 : 22;
        if (e->elite) { spawn_cells(x, cy, 3 + rn(3)); if (rn(100) < 30) drop_blueprint(x, cy); }
        else if (rn(100) < cell_ch * loot / 100) spawn_cells(x, cy, 1);
        if (r < 12) add_pick(PK_HEART, x, y - 10, 0, 0, -50);
        else if (r < 16) add_pick(PK_FLASK, x, y - 10, 0, 0, -50);
        return;
    }
    G.boss_alive = 0; G.shake = 14;
    if (G.arena_x0 >= 0) for (int ty = 2; ty <= 12; ty++) G.map[ty][G.arena_x0 / TS] = TL_EMPTY;
    if (G.door_tx >= 0) G.map[G.door_ty][G.door_tx] = TL_DOOR;
    spawn_cells(x, cy, e->type == ET_GOB ? 8 : 8);
    spawn_cells(x, cy, e->type == ET_GOB ? 0 : 7);
    drop_blueprint(x, cy);
    if (e->type == ET_GOB) {                                    /* arme legendaire (qualite L) */
        Item it;
        item_make(&it, random_weapon(G.p.w[0].id, G.p.w[1].id), 2, 4);
        add_item_pick(x, y - 24, &it, -70);
        banner("BOSS VAINCU", 100);
    } else { G.victory = 1; G.win_t = 0; banner("VICTOIRE !", 140); }
}

void hurt_enemy(Ent *e, int dmg, int dir, int kb, int stun) {
    if (!e->alive || e->st == ES_DEAD || e->st == ES_SLEEP) return;
    int boss = (e->type >= ET_GOB), cy = PX(e->y) - ED[e->type].h / 2;
    if (e->type == ET_SHLD && !g_pierce && dir == -e->dir && (e->st == ES_PATROL || e->st == ES_CHASE || e->st == ES_ATK)) {
        if (stun >= 12) {                                       /* coup lourd : la garde est brisee */
            e->st = ES_HURT; e->t = 50; e->hit = 0; dmg = dmg * 6 / 10; kb = 0;
            add_fx(SB_FX_HIT, SN_FX_HIT, PX(e->x) + dir * -8, cy, 2);
        } else {                                                /* coup leger : bloque par le bouclier */
            burst(PX(e->x) - e->dir * -8, cy, 4, 2); add_fx(SB_FX_HIT, 1, PX(e->x) + (dir > 0 ? -8 : 8), cy, 2);
            e->vx += dir * 16; return;
        }
    }
    if (e->type == ET_BRUTE && stun < 12) { dmg = dmg * 70 / 100; kb = 0; stun = 0; }   /* armure : coups legers reduits */
    if (dmg < 1) dmg = 1;
    if (dmg > 9999) dmg = 9999;                                   /* evite tout debordement des PV (int16) */
    e->hp = (int16_t)(e->hp - dmg); e->flash = 4;
    add_num(PX(e->x), PX(e->y) - ED[e->type].h - 4, dmg, g_crit ? 4 : 2);
    burst(PX(e->x), cy, 4, 0);
    add_fx(SB_FX_HIT, SN_FX_HIT, PX(e->x) - dir * 4, cy, 2);
    if (e->hp <= 0) { kill_enemy(e); return; }
    if (!boss) {
        e->vx += dir * kb * 16; if (kb) e->vy = -30;
        if (stun > 0 && e->st != ES_HURT) { e->st = ES_HURT; e->t = (int16_t)stun; e->hit = 0; }
    }
}

/* ================================================================= ennemis simples */
static void patrol_turn(Ent *e, int speed, int range) {
    e->vx = e->dir * speed;
    if (!ground_ahead(e, e->dir, 2) || wall_ahead(e, e->dir) || iabs(PX(e->x) - e->home) > range) { e->dir = (int8_t)-e->dir; e->vx = e->dir * speed; }
}

static void en_jail(Ent *e) {
    int adx = iabs(pdx(e)), dy = iabs(pdy(e)), near_ = adx < 130 && dy < 44;
    switch (e->st) {
    case ES_PATROL: patrol_turn(e, 14, 72); if (near_) e->st = ES_CHASE; break;
    case ES_CHASE:
        face(e);
        if (adx > 190) { e->st = ES_PATROL; e->vx = 0; break; }
        if (adx <= 28 && dy < 28) { e->st = ES_WIND; e->t = 14; e->vx = 0; break; }
        e->vx = (ground_ahead(e, e->dir, 2) && !wall_ahead(e, e->dir)) ? e->dir * 18 : 0;
        break;
    case ES_WIND: e->vx = 0; if (--e->t <= 0) { e->st = ES_ATK; e->t = 5; e->hit = 0; } break;
    case ES_ATK: {
        int x0 = (e->dir > 0) ? PX(e->x) + 2 : PX(e->x) - 28;
        e->vx = 0;
        if (!e->hit && player_box_overlap(x0, PX(e->y) - 24, 26, 22)) { hurt_player(edmg(e), PX(e->x), 3); e->hit = 1; }
        if (--e->t <= 0) { e->st = ES_REC; e->t = 18; }
        break; }
    case ES_REC: e->vx = 0; if (--e->t <= 0) e->st = ES_CHASE; break;
    default: break;
    }
}

static void en_xbow(Ent *e) {
    int adx = iabs(pdx(e)), dy = iabs(pdy(e)), seen = adx < 200 && dy < 60;
    if (e->cd > 0) e->cd--;
    switch (e->st) {
    case ES_PATROL: patrol_turn(e, 10, 60); if (seen) e->st = ES_CHASE; break;
    case ES_CHASE:
        face(e);
        if (adx > 270) { e->st = ES_PATROL; e->vx = 0; break; }
        if (adx < 70 && ground_ahead(e, -e->dir, 2) && !wall_ahead(e, -e->dir)) e->vx = -e->dir * 14;
        else if (adx > 150 && ground_ahead(e, e->dir, 2) && !wall_ahead(e, e->dir)) e->vx = e->dir * 10;
        else e->vx = 0;
        if (e->cd <= 0 && seen) { e->st = ES_WIND; e->t = 18; e->vx = 0; }
        break;
    case ES_WIND:
        e->vx = 0; face(e);
        if (--e->t <= 0) {
            add_proj(PJ_BOLT, 1, PX(e->x) + e->dir * 12, PX(e->y) - 14, e->dir * 72, 0, edmg(e), 90, e->dir);
            e->st = ES_REC; e->t = 14; e->cd = (int16_t)(50 + rn(30));
        }
        break;
    case ES_REC: e->vx = 0; if (--e->t <= 0) e->st = ES_CHASE; break;
    default: break;
    }
}

static void en_rat(Ent *e) {
    int adx = iabs(pdx(e)), dy = iabs(pdy(e));
    switch (e->st) {
    case ES_PATROL:
        patrol_turn(e, 12, 80);
        if (adx < 130 && dy < 40) { face(e); e->st = ES_WIND; e->t = 10; e->vx = 0; }
        break;
    case ES_WIND: e->vx = 0; if (--e->t <= 0) { e->st = ES_ATK; e->t = 36; e->hit = 0; } break;
    case ES_ATK:
        e->vx = e->dir * 50;
        if (wall_ahead(e, e->dir) || !ground_ahead(e, e->dir, 0)) { e->vx = 0; e->t = 1; }
        if (!e->hit && touch(e)) { hurt_player(edmg(e), PX(e->x), 3); e->hit = 1; }
        if (--e->t <= 0) { e->st = ES_REC; e->t = 26; e->vx = 0; }
        break;
    case ES_REC: e->vx = 0; if (--e->t <= 0) e->st = ES_PATROL; break;
    default: break;
    }
}

static void en_brute(Ent *e) {
    int adx = iabs(pdx(e)), dy = iabs(pdy(e));
    switch (e->st) {
    case ES_PATROL: patrol_turn(e, 8, 60); if (adx < 120 && dy < 48) e->st = ES_CHASE; break;
    case ES_CHASE:
        face(e);
        if (adx > 200) { e->st = ES_PATROL; e->vx = 0; break; }
        if (adx < 40 && dy < 34) { e->st = ES_WIND; e->t = 24; e->vx = 0; break; }
        e->vx = (ground_ahead(e, e->dir, 2) && !wall_ahead(e, e->dir)) ? e->dir * 9 : 0;
        break;
    case ES_WIND: e->vx = 0; if (--e->t <= 0) { e->st = ES_ATK; e->t = 6; e->hit = 0; add_fx(SB_FX_SMASH, SN_FX_SMASH, PX(e->x) + e->dir * 26, PX(e->y) - 8, 3); } break;
    case ES_ATK: {
        int x0 = (e->dir > 0) ? PX(e->x) + 2 : PX(e->x) - 40;
        e->vx = 0;
        if (!e->hit && player_box_overlap(x0, PX(e->y) - 28, 38, 28)) { hurt_player(edmg(e), PX(e->x), 5); e->hit = 1; }
        if (--e->t <= 0) { e->st = ES_REC; e->t = 32; }
        break; }
    case ES_REC: e->vx = 0; if (--e->t <= 0) e->st = ES_CHASE; break;
    default: break;
    }
}

static void en_slime(Ent *e) {
    int adx = iabs(pdx(e));
    if (e->st == ES_AIR) {
        if (e->ground) { e->st = ES_CHASE; e->vx = 0; e->cd = (int16_t)(30 + rn(30)); }
    } else {
        e->vx = 0;
        if (e->cd > 0) e->cd--;
        if (e->cd <= 0 && adx < 170) { face(e); e->vy = -95; e->vx = e->dir * 28; e->st = ES_AIR; e->ground = 0; }
    }
    if (touch(e)) hurt_player(edmg(e), PX(e->x), 3);
}

/* ---- artificier : garde ses distances et lance des bombes en cloche */
static void en_gren(Ent *e) {
    int dx = pdx(e), adx = iabs(dx), dy = iabs(pdy(e)), seen = adx < 210 && dy < 90;
    if (e->cd > 0) e->cd--;
    switch (e->st) {
    case ES_PATROL: patrol_turn(e, 10, 60); if (seen) e->st = ES_CHASE; break;
    case ES_CHASE:
        face(e);
        if (adx > 280) { e->st = ES_PATROL; e->vx = 0; break; }
        if (adx < 80 && ground_ahead(e, -e->dir, 2) && !wall_ahead(e, -e->dir)) e->vx = -e->dir * 14;
        else if (adx > 170 && ground_ahead(e, e->dir, 2) && !wall_ahead(e, e->dir)) e->vx = e->dir * 11;
        else e->vx = 0;
        if (e->cd <= 0 && seen) { e->st = ES_WIND; e->t = 20; e->vx = 0; }
        break;
    case ES_WIND:
        e->vx = 0; face(e);
        if (--e->t <= 0) {
            add_proj(PJ_BOMB, 1, PX(e->x) + e->dir * 8, PX(e->y) - 24, clampi(pdx(e) / 2, -80, 80), -96, edmg(e), 120, e->dir);
            e->st = ES_REC; e->t = 14; e->cd = (int16_t)(70 + rn(40));
        }
        break;
    case ES_REC: e->vx = 0; if (--e->t <= 0) e->st = ES_CHASE; break;
    default: break;
    }
}

/* ---- garde-bouclier : garde frontale (voir hurt_enemy) ; vulnerable quand il prepare son coup de bouclier */
static void en_shld(Ent *e) {
    int adx = iabs(pdx(e)), dy = iabs(pdy(e));
    switch (e->st) {
    case ES_PATROL: patrol_turn(e, 8, 56); if (adx < 140 && dy < 48) e->st = ES_CHASE; break;
    case ES_CHASE:
        face(e);
        if (adx > 220) { e->st = ES_PATROL; e->vx = 0; break; }
        if (adx < 36 && dy < 30) { e->st = ES_WIND; e->t = 16; e->vx = 0; break; }
        e->vx = (ground_ahead(e, e->dir, 2) && !wall_ahead(e, e->dir)) ? e->dir * 12 : 0;
        break;
    case ES_WIND: e->vx = 0; if (--e->t <= 0) { e->st = ES_ATK; e->t = 6; e->hit = 0; } break;
    case ES_ATK: {
        int x0 = (e->dir > 0) ? PX(e->x) + 2 : PX(e->x) - 26;
        e->vx = e->dir * 20;
        if (wall_ahead(e, e->dir) || !ground_ahead(e, e->dir, 0)) e->vx = 0;
        if (!e->hit && player_box_overlap(x0, PX(e->y) - 24, 24, 22)) { hurt_player(edmg(e), PX(e->x), 5); e->hit = 1; }
        if (--e->t <= 0) { e->st = ES_REC; e->t = 22; }
        break; }
    case ES_REC: e->vx = 0; if (--e->t <= 0) e->st = ES_CHASE; break;
    default: break;
    }
}

/* ---- putride : zombie toxique, lent, laisse un nuage de poison en mourant */
static void en_toxic(Ent *e) {
    int adx = iabs(pdx(e)), dy = iabs(pdy(e));
    switch (e->st) {
    case ES_PATROL: patrol_turn(e, 7, 50); if (adx < 120 && dy < 44) e->st = ES_CHASE; break;
    case ES_CHASE:
        face(e);
        if (adx > 200) { e->st = ES_PATROL; e->vx = 0; break; }
        if (adx <= 26 && dy < 30) { e->st = ES_WIND; e->t = 16; e->vx = 0; break; }
        e->vx = (ground_ahead(e, e->dir, 2) && !wall_ahead(e, e->dir)) ? e->dir * 11 : 0;
        break;
    case ES_WIND: e->vx = 0; if (--e->t <= 0) { e->st = ES_ATK; e->t = 5; e->hit = 0; } break;
    case ES_ATK: {
        int x0 = (e->dir > 0) ? PX(e->x) + 2 : PX(e->x) - 30;
        e->vx = 0;
        if (!e->hit && player_box_overlap(x0, PX(e->y) - 22, 28, 20)) { hurt_player(edmg(e), PX(e->x), 3); e->hit = 1; }
        if (--e->t <= 0) { e->st = ES_REC; e->t = 20; }
        break; }
    case ES_REC: e->vx = 0; if (--e->t <= 0) e->st = ES_CHASE; break;
    default: break;
    }
}

/* ---- volant : plane au-dessus du joueur puis pique */
static void bat_move(Ent *e) {
    int32_t nx = e->x + e->vx, ny;
    if (is_solid(tile_at(PX(nx) >> 4, (PX(e->y) - 5) >> 4))) { e->vx = 0; if (e->st == ES_ATK) { e->st = ES_REC; e->t = 20; } else e->dir = (int8_t)-e->dir; }
    else e->x = nx;
    ny = e->y + e->vy;
    if (is_solid(tile_at(PX(e->x) >> 4, (PX(ny) - 5) >> 4)) || PX(ny) < 3 * TS) { e->vy = 0; if (e->st == ES_ATK) { e->st = ES_REC; e->t = 20; } }
    else e->y = ny;
    if (PX(e->y) > 13 * TS) { e->y = F(13 * TS); e->vy = 0; if (e->st == ES_ATK) { e->st = ES_REC; e->t = 20; } }
}
static void en_bat(Ent *e) {
    int dx = pdx(e), adx = iabs(dx), ty = imax(3 * TS + 10, PX(G.p.y) - 52);
    switch (e->st) {
    case ES_PATROL:
        e->vx = e->dir * 8; e->vy = (e->anim & 32) ? 4 : -4;
        if (adx < 170 && iabs(pdy(e)) < 150) e->st = ES_CHASE;
        break;
    case ES_CHASE:
        face(e);
        e->vx = (adx > 16) ? ((dx > 0) ? 18 : -18) : 0;
        e->vy = clampi((ty - PX(e->y)) * 2, -16, 16);
        if (e->cd > 0) e->cd--;
        if (e->cd <= 0 && adx < 90) { e->st = ES_WIND; e->t = 12; e->vx = e->vy = 0; }
        if (adx > 300) e->st = ES_PATROL;
        break;
    case ES_WIND:
        face(e); e->vx = e->vy = 0;
        if (--e->t <= 0) {
            int ddx = PX(G.p.x) - PX(e->x), ddy = (PX(G.p.y) - 10) - (PX(e->y) - 5), n = imax(1, imax(iabs(ddx), iabs(ddy)));
            e->vx = ddx * 64 / n; e->vy = ddy * 64 / n; e->st = ES_ATK; e->t = 26; e->hit = 0;
        }
        break;
    case ES_ATK:
        if (!e->hit && touch(e)) { hurt_player(edmg(e), PX(e->x), 3); e->hit = 1; }
        if (e->hit || --e->t <= 0) { e->st = ES_REC; e->t = 20; }
        break;
    case ES_REC:
        e->vx = -e->dir * 10; e->vy = -20;
        if (--e->t <= 0) { e->st = ES_CHASE; e->cd = (int16_t)(50 + rn(40)); }
        break;
    default: break;
    }
    bat_move(e);
}

/* ================================================================= boss 1 : le Gobeur */
static void waves(Ent *e, int speed, int dmg, int life) {
    int x = PX(e->x), y = PX(e->y);
    add_proj(PJ_WAVE, 1, x - 14, y, -speed, 0, dmg, life, -1);
    add_proj(PJ_WAVE, 1, x + 14, y, speed, 0, dmg, life, 1);
    add_fx(SB_FX_SMASH, SN_FX_SMASH, x, y - 8, 3);
    G.shake = 8;
}

static void en_gob(Ent *e) {
    int dx = pdx(e), enr = e->hp * 100 < e->maxhp * 40;
    switch (e->st) {
    case ES_IDLE:
        e->vx = 0; face(e);
        if (--e->t <= 0) {
            int a; do { a = rn(3); } while (a == e->aux && rn(3));
            e->aux = (uint8_t)a; e->st = ES_WIND; e->t = (int16_t)(a == 1 ? 18 : 14);
        }
        break;
    case ES_WIND:
        e->vx = 0; face(e);
        if (--e->t <= 0) {
            if (e->aux == 0) { e->vy = -100; e->vx = clampi(dx * 16 / 22, -48, 48); e->st = ES_AIR; e->t = 0; e->ground = 0; }
            else if (e->aux == 1) {
                int n = enr ? 5 : 3;
                for (int k = 0; k < n; k++) add_proj(PJ_SPIT, 1, PX(e->x) + e->dir * 14, PX(e->y) - 24, e->dir * (26 + k * 11), -(50 + k * 20), edmg(e) - 3, 120, e->dir);
                e->st = ES_REC; e->t = (int16_t)(enr ? 16 : 28);
            } else { e->vy = -170; e->vx = 0; e->st = ES_AIR; e->t = 0; e->ground = 0; }
        }
        break;
    case ES_AIR:
        if (e->aux == 2) {
            if (e->t == 0) { e->vx = clampi(dx * 2, -48, 48); if (e->vy >= 0) { e->t = 1; e->vy = 130; e->vx = 0; } }
            else e->vx = 0;
        }
        if (e->ground) { waves(e, 40, edmg(e) - 2, 46); e->st = ES_REC; e->t = (int16_t)(enr ? 14 : 26); e->vx = 0; }
        break;
    case ES_REC: e->vx = 0; if (--e->t <= 0) { e->st = ES_IDLE; e->t = (int16_t)(enr ? 14 : 30 + rn(20)); } break;
    default: break;
    }
    if (e->st != ES_SLEEP && touch(e)) hurt_player(edmg(e), PX(e->x), 4);
}

/* ================================================================= boss final : le Roi Fele */
static void en_king(Ent *e) {
    int adx = iabs(pdx(e)), enr = e->hp * 2 < e->maxhp;
    switch (e->st) {
    case ES_IDLE:
        face(e);
        e->vx = (adx > 44) ? e->dir * (enr ? 20 : 15) : 0;
        if (e->cd > 0) e->cd--;
        if (e->cd <= 0) {
            int a = rn(3);
            if (a == e->aux && rn(2)) a = (a + 1) % 3;
            e->aux = (uint8_t)a; e->st = ES_WIND; e->t = (int16_t)(a == 1 ? 18 : 22); e->vx = 0; e->hit = 0;
        }
        break;
    case ES_WIND:
        e->vx = 0; if (e->aux != 0) face(e);
        if (--e->t <= 0) {
            e->st = ES_ATK; e->hit = 0; e->t = (int16_t)(e->aux == 0 ? 34 : (e->aux == 1 ? 6 : 5));
            if (e->aux == 2) {
                waves(e, 48, edmg(e) - 6, 56);
                if (enr) {
                    int rats = 0;
                    for (int i = 0; i < MAX_EN; i++) if (G.en[i].alive && G.en[i].type == ET_RAT) rats++;
                    for (; rats < 2 && G.arena_x0 >= 0; rats++) spawn_enemy(ET_RAT, G.arena_x0 / TS + 2 + rn(16), 12, G.zone);
                }
            }
        }
        break;
    case ES_ATK:
        if (e->aux == 0) {
            e->vx = e->dir * (enr ? 90 : 78);
            if (!e->hit && touch(e)) { hurt_player(edmg(e), PX(e->x), 6); e->hit = 1; }
            if (wall_ahead(e, e->dir)) { e->st = ES_REC; e->t = 40; e->vx = 0; G.shake = 8; burst(PX(e->x) + e->dir * 12, PX(e->y) - 20, 8, 0); }
            else if (--e->t <= 0) { e->st = ES_REC; e->t = 22; e->vx = 0; }
        } else if (e->aux == 1) {
            int x0 = (e->dir > 0) ? PX(e->x) + 4 : PX(e->x) - 62;
            e->vx = 0;
            if (!e->hit && player_box_overlap(x0, PX(e->y) - 44, 58, 44)) { hurt_player(edmg(e), PX(e->x), 5); e->hit = 1; }
            if (--e->t <= 0) { e->st = ES_REC; e->t = 18; }
        } else { e->vx = 0; if (--e->t <= 0) { e->st = ES_REC; e->t = 24; } }
        break;
    case ES_REC: e->vx = 0; if (--e->t <= 0) { e->st = ES_IDLE; e->cd = (int16_t)(enr ? 24 + rn(20) : 46 + rn(30)); } break;
    default: break;
    }
    if (e->st != ES_SLEEP && e->st != ES_ATK && touch(e)) hurt_player(edmg(e) / 2, PX(e->x), 3);
}

static void enemy_update(Ent *e) {
    if (!e->alive) return;
    e->anim++;
    if (e->flash > 0) e->flash--;
    if (e->burn > 0 && e->st != ES_DEAD && e->st != ES_SLEEP) {                 /* brulure / saignement */
        e->burn--;
        if (e->burn % 15 == 0) {
            int d = 3 + G.zone;
            e->hp = (int16_t)(e->hp - d); e->flash = 2;
            add_part(PA_EMBER, PX(e->x), PX(e->y) - ED[e->type].h, rn(21) - 10, -12, 12, 0);
            if (e->hp <= 0) kill_enemy(e);
        }
    }
    if (e->st == ES_SLEEP) { en_phys(e); return; }
    if (e->st == ES_DEAD) {
        e->t++; e->vx = e->vx * 3 / 4; en_phys(e);
        if (e->t >= (e->type >= ET_GOB ? 50 : 26)) e->alive = 0;
        return;
    }
    if (e->type < ET_GOB && iabs(pdx(e)) > 420) return;               /* trop loin : on ne simule pas */
    if (e->st == ES_HURT) {
        e->vx = e->vx * 3 / 4;
        if (--e->t <= 0) e->st = ES_CHASE;
        en_phys(e);
        return;
    }
    g_src = e;
    switch (e->type) {
    case ET_JAIL: en_jail(e); break;
    case ET_XBOW: en_xbow(e); break;
    case ET_RAT: en_rat(e); break;
    case ET_BRUTE: en_brute(e); break;
    case ET_SLIME: en_slime(e); break;
    case ET_GREN: en_gren(e); break;
    case ET_SHLD: en_shld(e); break;
    case ET_TOXIC: en_toxic(e); break;
    case ET_BAT: en_bat(e); break;
    case ET_GOB: en_gob(e); break;
    default: en_king(e); break;
    }
    g_src = 0;
    if (e->type != ET_BAT) en_phys(e);
}

/* ================================================================= projectiles */
static void explode(Proj *p) {
    int x = PX(p->x), y = PX(p->y);
    add_fx(SB_FX_EXPL, SN_FX_EXPL, x, y, 3);
    G.shake = imax(G.shake, 4);
    g_pierce = 1;                                                       /* le souffle ignore les boucliers */
    if (p->owner == 0 || p->type == PJ_FIRE) {
        for (int i = 0; i < MAX_EN; i++) {
            Ent *e = &G.en[i];
            if (!e->alive || e->st == ES_DEAD || e->st == ES_SLEEP) continue;
            if (iabs(PX(e->x) - x) < 30 && iabs(PX(e->y) - ED[e->type].h / 2 - y) < 28) {
                hurt_enemy(e, p->dmg, p->dir, 2, 8);
                if (p->type == PJ_FIRE && e->st != ES_DEAD) e->burn = 90;
            }
        }
    } else if (iabs(PX(G.p.x) - x) < 34 && iabs(PX(G.p.y) - 10 - y) < 30) hurt_player(p->dmg, x, 5);
    g_pierce = 0;
    p->life = 0;
}

static void proj_update(void) {
    for (int i = 0; i < MAX_PR; i++) {
        Proj *p = &G.pr[i];
        if (!p->life) continue;
        p->life--; p->anim++;
        if (p->type == PJ_SPIT) p->vy += 5;
        if (p->type == PJ_BOMB) p->vy += 6;
        p->x += p->vx; p->y += p->vy;
        int x = PX(p->x), y = PX(p->y);
        if (p->type == PJ_CLOUD) {                                      /* nuage de poison : immobile, degats sur la duree */
            if (player_box_overlap(x - 14, y - 16, 28, 16)) hurt_player(p->dmg, x, 0);
            continue;
        }
        if (x < 0 || x > G.lvw * TS || y > (LV_H + 2) * TS || y < 0) { p->life = 0; continue; }
        if (is_solid(tile_at(x >> 4, (p->type == PJ_WAVE ? y - 4 : y) >> 4))) {
            if (p->type == PJ_FIRE || p->type == PJ_BOMB) explode(p);
            else { if (p->type != PJ_WAVE) burst(x, y, 3, 0); p->life = 0; }
            continue;
        }
        if (p->type == PJ_BOMB && p->life == 0) { explode(p); continue; }
        if (p->owner == 0 && p->type != PJ_BOMB) {
            for (int k = 0; k < MAX_EN; k++) {
                Ent *e = &G.en[k];
                if (!e->alive || e->st == ES_DEAD || e->st == ES_SLEEP || (p->hm & (1u << k))) continue;
                int ex = PX(e->x), ey = PX(e->y);
                if (iabs(x - ex) < ED[e->type].w / 2 + 4 && y > ey - ED[e->type].h - 2 && y < ey + 2) {
                    if (p->type == PJ_FIRE) { explode(p); break; }
                    p->hm |= (1u << k);
                    g_pierce = (uint8_t)(p->type == PJ_XBOLT);          /* l'arbalete perce les boucliers */
                    hurt_enemy(e, p->dmg, p->dir, p->type == PJ_PARROW ? 4 : 2, p->type == PJ_PARROW ? 14 : 6);
                    g_pierce = 0;
                    if (p->pierce == 0) { p->life = 0; break; }
                    p->pierce--;
                }
            }
        } else if (p->owner == 1 || p->type == PJ_BOMB) {
            int hw = (p->type == PJ_WAVE) ? 7 : (p->type == PJ_SPIT ? 4 : 6);
            int y0 = (p->type == PJ_WAVE) ? y - 14 : y - 3, h = (p->type == PJ_WAVE) ? 14 : 6;
            if (p->owner == 1 && player_box_overlap(x - hw, y0, hw * 2, h)) {
                if (p->type != PJ_WAVE && player_parry_window(x)) {     /* PARADE : le projectile est renvoye */
                    p->owner = 0; p->vx = -p->vx; p->vy = -p->vy / 2; p->dir = (int8_t)-p->dir; p->dmg = (int16_t)(p->dmg * 2); p->hm = 0; p->life = 80;
                    burst(x, y, 8, 2); msg("PARADE !"); G.hitstop = 3;
                } else if (p->type == PJ_BOMB) explode(p);
                else { hurt_player(p->dmg, x, 3); if (p->type != PJ_WAVE) p->life = 0; }
            } else if (p->owner == 0 && p->type == PJ_BOMB) {          /* bombe renvoyee : touche un ennemi */
                for (int k = 0; k < MAX_EN; k++) {
                    Ent *e = &G.en[k];
                    if (e->alive && e->st != ES_DEAD && e->st != ES_SLEEP && iabs(x - PX(e->x)) < ED[e->type].w / 2 + 6 && y > PX(e->y) - ED[e->type].h - 4 && y < PX(e->y) + 2) { explode(p); break; }
                }
            }
        }
    }
}

/* ================================================================= objets ramassables */
static void take_weapon(const Item *it) {
    Player *p = &G.p;
    char buf[32], lab[8]; const char *nm = WPN[it->id].name; int i = 0;
    if (it->id == WP_SCYTHE) { p->w[0] = *it; p->w[1] = *it; }                 /* arme double : occupe les deux emplacements */
    else {
        p->w[p->cur] = *it;
        if (p->w[p->cur ^ 1].id == WP_SCYTHE) {
            item_make(&p->w[p->cur ^ 1], random_weapon(it->id, WP_SCYTHE), 0, -1);
            if (p->w[p->cur ^ 1].id == WP_SCYTHE) p->w[p->cur ^ 1].id = WP_SWORD;
        }
    }
    item_label(it, lab);
    while (*nm && i < 22) buf[i++] = *nm++;
    buf[i++] = ' ';
    for (int j = 0; lab[j] && i < 31; j++) buf[i++] = lab[j];
    buf[i] = 0; msg(buf);
}

static void chest_open(Pick *k) {
    int x = PX(k->x), y = PX(k->y), r = rn(100);
    k->st = 1;
    if (r < 36) spawn_coins(x, y - 12, 12 + rn(14));
    else if (r < 52) add_pick(PK_HEART, x, y - 16, 0, 0, -60);
    else if (r < 62) add_pick(PK_FLASK, x, y - 16, 0, 0, -60);
    else if (r < 72) { drop_blueprint(x, y - 14); spawn_cells(x, y - 12, 2); }
    else { Item it; item_make(&it, random_weapon(G.p.w[0].id, G.p.w[1].id), rn(2), -1); add_item_pick(x, y - 16, &it, -70); }
    burst(x, y - 10, 10, 2);
    add_fx(SB_FX_HIT, SN_FX_HIT, x, y - 10, 2);
}

static void pick_update(void) {
    Player *p = &G.p;
    for (int i = 0; i < MAX_PK; i++) {
        Pick *k = &G.pk[i];
        if (!k->life || k->kind == PK_NPC) continue;
        int x = PX(k->x), y = PX(k->y);
        if (k->kind == PK_CHEST) {
            if (!k->st && p->st != PS_DEAD && player_box_overlap(x - 8, y - 14, 16, 14)) chest_open(k);
            continue;
        }
        int magnet = 0;
        if ((k->kind == PK_COIN || k->kind == PK_CELL) && p->st != PS_DEAD) {
            int dx = PX(p->x) - x, dy = (PX(p->y) - 10) - (y - 3);
            if (iabs(dx) < 44 && iabs(dy) < 36) { k->x += clampi(dx, -3, 3) * 16; k->y += clampi(dy, -3, 3) * 16; magnet = 1; }
        }
        if (!magnet) {
            k->vy = imin(k->vy + 6, 100);
            int vyb = (int)k->vy;
            int fl = move_box(&k->x, &k->y, &k->vx, &k->vy, 6, 6, 0);
            if (fl & MB_GROUND) { k->vx = k->vx * 3 / 4; if (vyb > 40) k->vy = -vyb / 3; }
        }
        x = PX(k->x); y = PX(k->y);
        if (y > (LV_H + 2) * TS) { k->life = 0; continue; }
        if (p->st == PS_DEAD || !player_box_overlap(x - 4, y - 8, 8, 8)) continue;
        switch (k->kind) {
        case PK_COIN: G.run_gold += imax(1, k->val); k->life = 0; break;
        case PK_CELL: G.cells_run += k->val; k->life = 0; break;
        case PK_BLUE: G.bp_carry |= (uint16_t)(1 << k->val); msg("SCH" EAC "MA TROUV" EAC " !"); k->life = 0; break;
        case PK_HEART:
            if (p->hp < p->maxhp) { int h = p->maxhp / 5; p->hp = (int16_t)imin(p->maxhp, p->hp + h); add_num(PX(p->x), PX(p->y) - 26, h, 3); k->life = 0; }
            break;
        case PK_FLASK:
            if (p->flasks < p->maxflasks) { p->flasks++; msg("FIOLE +1"); k->life = 0; }
            break;
        case PK_WPN: { Item it; it.id = k->val; it.lvl = k->lvl ? k->lvl : 1; it.q = k->q; take_weapon(&it); k->life = 0; break; }
        default: break;
        }
    }
}

/* ================================================================= particules */
void part_update(void) {
    for (int i = 0; i < MAX_PA; i++) {
        Part *a = &G.pa[i];
        if (!a->life) continue;
        a->life--;
        switch (a->kind) {
        case PA_SPARK: a->vy = (int16_t)(a->vy + 3); a->x += a->vx; a->y += a->vy; break;
        case PA_SMOKE: a->x += a->vx; a->y += a->vy; a->vx = (int16_t)(a->vx * 7 / 8); break;
        case PA_EMBER: a->x += a->vx; a->y += a->vy; break;
        case PA_NUM: a->y += a->vy; a->vy = (int16_t)(a->vy * 9 / 10); break;
        default: break;
        }
    }
}

void ai_update(void) {
    for (int i = 0; i < MAX_EN; i++) enemy_update(&G.en[i]);
    proj_update();
    pick_update();
    part_update();
}

void kill_all_for_test(void) {
    for (int i = 0; i < MAX_EN; i++) if (G.en[i].alive && G.en[i].st != ES_DEAD) { G.en[i].hp = 1; hurt_enemy(&G.en[i], 9999, 1, 0, 0); }
}
