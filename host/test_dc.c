/* test_dc.c - tests automatiques (sur PC, avec ASan/UBSan) : salles, niveaux, physique, collisions, armes, bouclier, ennemis,
   boss, passages/PNJ, mutations, sauvegarde, rendu, fuzz et partie complete jouee par un robot. */
#include "dc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SAVEF "/tmp/dc_unit.sav"
static int n_run, n_fail;
#define CHECK(c, ...) do { n_run++; if (!(c)) { n_fail++; printf("    ECHEC ligne %d : ", __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)
static void GROUP(const char *s) { printf("[%s]\n", s); }

uint32_t plat_millis(void) { static uint32_t t; return t += 33; }
int plat_load(void *b, int l) { FILE *f = fopen(SAVEF, "rb"); int n; if (!f) return 0; n = (int)fread(b, 1, (size_t)l, f); fclose(f); return n; }
int plat_store(const void *b, int l) { FILE *f = fopen(SAVEF, "wb"); if (!f) return -1; fwrite(b, 1, (size_t)l, f); fclose(f); return 0; }

static void run(uint16_t k, int n) { while (n-- > 0) game_update(k); }
static void clear_world(void) { memset(G.en, 0, sizeof G.en); memset(G.pr, 0, sizeof G.pr); memset(G.pk, 0, sizeof G.pk); }
static void new_game(unsigned seed) { remove(SAVEF); game_init(seed); game_update(K_OK); game_update(0); clear_world(); G.p.inv = 0; }
static void tp(int x, int y) { G.p.x = F(x); G.p.y = F(y); G.p.vx = G.p.vy = 0; G.cam_x = F(x - 160 < 0 ? 0 : x - 160); }
static Ent *newest(void) { Ent *r = 0; for (int i = 0; i < MAX_EN; i++) if (G.en[i].alive) r = &G.en[i]; return r; }
static Ent *dummy(int type, int dx, int dir) {
    spawn_enemy(type, (PX(G.p.x) + dx) / 16, 12, 0);
    Ent *e = newest();
    e->elite = 0; e->hp = e->maxhp = 5000; e->dir = (int8_t)dir; e->x = F(PX(G.p.x) + dx); e->st = ES_REC; e->t = 30000;
    return e;
}
static void equip(int slot, int id, int lvl, int q) { G.p.w[slot].id = (uint8_t)id; G.p.w[slot].lvl = (uint8_t)lvl; G.p.w[slot].q = (uint8_t)q; }
static void settle(void) { G.p.st = PS_NORMAL; G.p.atk_cd = 0; G.p.inv = 0; G.p.hold = 0; G.p.buf_atk = 0; G.p.vx = 0; G.p.dir = 1; G.p.hp = G.p.maxhp; G.hitstop = 0; }
static int count_pr(int owner) { int n = 0; for (int i = 0; i < MAX_PR; i++) if (G.pr[i].life && G.pr[i].owner == owner) n++; return n; }
static Pick *find_pick(int kind) { for (int i = 0; i < MAX_PK; i++) if (G.pk[i].life && G.pk[i].kind == kind) return &G.pk[i]; return 0; }

/* ---------------------------------------------------------------- T01 salles */
static void t_rooms(void) {
    GROUP("T01 salles (14 modeles)");
    for (int r = 0; r < 14; r++) {
        int S = 0, D = 0, B = 0, d = 0, N = 0, M = 0;
        for (int y = 0; y < 15; y++) {
            CHECK(strlen(DC_ROOMS[r][y]) == 20, "salle %d ligne %d : longueur %d", r, y, (int)strlen(DC_ROOMS[r][y]));
            for (int x = 0; x < 20; x++) { char c = DC_ROOMS[r][y][x]; S += c == 'S'; D += c == 'D'; B += c == 'B'; d += c == 'd'; N += c == 'N'; M += c == 'M'; }
            if (y >= 2 && y <= 12) CHECK(DC_ROOMS[r][y][0] != '#' && DC_ROOMS[r][y][19] != '#', "salle %d : bord ferme ligne %d", r, y);
        }
        CHECK(DC_ROOMS[r][13][0] == '#' && DC_ROOMS[r][13][19] == '#', "salle %d : sol coupe aux bords", r);
        if (r == 0) CHECK(S == 1, "depart");
        if (r == 1) CHECK(D == 1, "sortie");
        if (r == 2) CHECK(B == 1 && d == 1, "arene");
        if (r == 13) CHECK(S == 1 && D == 1 && N == 1 && M == 1, "passage");
    }
}

/* ---------------------------------------------------------------- T02 generation des niveaux */
static void t_levels(void) {
    GROUP("T02 generation semi-aleatoire (5 zones x 40 graines)");
    int minen = 99, maxen = 0, elites = 0, total = 0;
    for (int z = 0; z < NZONES; z++) for (int seed = 1; seed <= 40; seed++) {
        new_game((unsigned)(seed * 13 + z)); level_build(z); G.scene = SC_PLAY;
        int doors = 0, bosses = 0;
        for (int y = 0; y < LV_H; y++) for (int x = 0; x < G.lvw; x++) doors += G.map[y][x] == TL_DOOR;
        for (int i = 0; i < MAX_EN; i++) {
            Ent *e = &G.en[i];
            if (!e->alive) continue;
            int tx = PX(e->x) >> 4, ty = PX(e->y) >> 4;
            int below = tile_at(tx, ty);
            if (e->type >= ET_GOB) bosses++; else { total++; elites += e->elite; }
            CHECK(below == TL_SOLID || below == TL_PLAT, "zone %d graine %d : ennemi %d en l'air (tuile %d)", z, seed, e->type, below);
            CHECK(!is_solid(tile_at(tx, (PX(e->y) - 1) >> 4)), "ennemi dans un mur");
        }
        CHECK(G.lvw == G.nrooms * ROOM_W && G.lvw <= LV_W, "largeur");
        CHECK(G.torch_n <= 24, "torches");
        int ne = 0; for (int i = 0; i < MAX_EN; i++) ne += G.en[i].alive;
        if (ne < minen) minen = ne;
        if (ne > maxen) maxen = ne;
        if (z == 2 || z == 4) CHECK(bosses == 1 && G.arena_x0 >= 0 && G.door_tx >= 0 && doors == 0, "zone %d : arene incomplete", z);
        else CHECK(bosses == 0 && doors == 1, "zone %d : porte de sortie (%d)", z, doors);
        CHECK(is_solid(tile_at(PX(G.p.x) >> 4, PX(G.p.y) >> 4)), "depart sans sol");
    }
    printf("    ennemis par niveau : %d a %d ; elites : %d sur %d (%.1f %%)\n", minen, maxen, elites, total, total ? 100.0 * elites / total : 0.0);
}

/* ---------------------------------------------------------------- T03 physique et collisions */
static void t_physics(void) {
    GROUP("T03 physique : gravite, saut, double saut, mur, plateformes");
    new_game(1); tp(5 * 16 + 8, 100); run(0, 60);
    CHECK(G.p.ground && PX(G.p.y) == 13 * 16, "atterrissage : sol=%d y=%d", G.p.ground, PX(G.p.y));
    int miny = 999; settle();
    for (int i = 0; i < 24; i++) { game_update(K_U); if (PX(G.p.y) < miny) miny = PX(G.p.y); }
    run(0, 40);
    CHECK(208 - miny >= 46 && 208 - miny <= 52, "saut complet : %d px", 208 - miny);
    settle(); G.p.y = F(208); G.p.vy = 0; run(0, 10); miny = 999;
    game_update(K_U); game_update(0);
    for (int i = 0; i < 30; i++) { game_update(0); if (PX(G.p.y) < miny) miny = PX(G.p.y); }
    CHECK(208 - miny < 40, "saut court : %d px (doit etre plus bas)", 208 - miny);
    run(0, 40); settle(); miny = 999;
    for (int i = 0; i < 8; i++) game_update(K_U);
    for (int i = 0; i < 3; i++) game_update(0);
    for (int i = 0; i < 40; i++) { game_update(i < 2 ? K_U : 0); if (PX(G.p.y) < miny) miny = PX(G.p.y); }
    CHECK(208 - miny > 70, "double saut : %d px (> 70 attendu)", 208 - miny);
    run(0, 60); settle(); tp(8 * 16, 13 * 16); run(0, 5); int x0 = PX(G.p.x);
    run(K_R, 30);
    CHECK(PX(G.p.x) - x0 >= 62 && PX(G.p.x) - x0 <= 82, "vitesse de course : %d px/30 images", PX(G.p.x) - x0);
    run(K_L, 120);
    CHECK(PX(G.p.x) == 21, "arret contre le mur gauche : x=%d (attendu 21)", PX(G.p.x));
    /* plateforme traversable */
    new_game(2); for (int x = 5; x <= 8; x++) G.map[11][x] = TL_PLAT;
    tp(6 * 16 + 8, 13 * 16); run(0, 3); settle();
    for (int i = 0; i < 20; i++) game_update(K_U);
    run(0, 50);
    CHECK(PX(G.p.y) == 11 * 16 && G.p.ground, "atterrir sur la plateforme (y=%d)", PX(G.p.y));
    game_update(K_D); run(0, 40);
    CHECK(PX(G.p.y) == 13 * 16, "traverser la plateforme vers le bas (y=%d)", PX(G.p.y));
    /* chute dans une fosse : degats puis reapparition */
    new_game(3); G.map[13][6] = G.map[14][6] = G.map[13][7] = G.map[14][7] = TL_EMPTY;
    tp(6 * 16 + 16, 13 * 16 - 4); G.p.y = F(13 * 16); int hp0 = G.p.hp; run(0, 80);
    CHECK(G.p.hp == hp0 - hp0 * 35 / 100 && PX(G.p.y) == 13 * 16, "chute en fosse : hp %d->%d y=%d", hp0, G.p.hp, PX(G.p.y));
    G.p.hp = 20; G.p.y = F(13 * 16); tp(6 * 16 + 16, 13 * 16); run(0, 80);
    CHECK(G.scene == SC_DEAD || G.p.st == PS_DEAD, "chute mortelle");
}

/* ---------------------------------------------------------------- T04 armes : degats, niveaux, qualites */
static int hit_dmg(int id, int lvl, int q, int heavy, int dx, int dir, uint16_t pre) {
    new_game(9); G.sv.unlocked = 0x3FF; equip(0, id, lvl, q); equip(1, id, lvl, q); G.p.cur = 0; tp(5 * 16 + 8, 13 * 16); run(0, 3); settle();
    Ent *e = dummy(ET_JAIL, dx, dir); int hp0 = e->hp;
    if (pre) run(pre, 1);
    run(K_OK, heavy ? 16 : 2);
    for (int i = 0; i < 70; i++) { game_update(0); e->st = ES_REC; e->t = 30000; }
    return hp0 - e->hp;
}
static void t_weapons(void) {
    GROUP("T04 armes : degats de base, niveau I..XV, qualites + ++ S L");
    static const char *nm[WP_COUNT] = { "epee", "dague", "rapiere", "marteau", "bouclier", "arc", "arbalete", "baton de feu", "sceptre", "faux-serres" };
    for (int id = 0; id < WP_COUNT; id++) {
        int l = hit_dmg(id, 1, 0, 0, 18, -1, 0), h = (id == WP_SHIELD) ? 0 : hit_dmg(id, 1, 0, 1, 18, -1, 0);
        CHECK(l == WPN[id].l.dmg || (id == WP_FIRE && l >= WPN[id].l.dmg), "%s leger : %d degats (attendu %d)", nm[id], l, WPN[id].l.dmg);
        if (id != WP_SHIELD) CHECK(h > 0, "%s lourd : %d degats", nm[id], h);
        if (id == WP_ELEC) CHECK(h >= WPN[id].h.dmg, "sceptre lourd %d", h);
        printf("    %-14s leger %3d   lourd %3d\n", nm[id], l, h);
    }
    int base = hit_dmg(WP_SWORD, 1, 0, 0, 18, -1, 0), hi = hit_dmg(WP_SWORD, 15, 0, 0, 18, -1, 0), q3 = hit_dmg(WP_SWORD, 5, 3, 0, 18, -1, 0);
    CHECK(hi > base * 17 / 10, "niveau XV (%d) bien superieur au niveau I (%d)", hi, base);
    CHECK(q3 == base * (100 + 6 * 10) / 100, "niveau V qualite S : %d (attendu %d)", q3, base * 160 / 100);
    int q1 = hit_dmg(WP_SWORD, 5, 1, 0, 18, -1, 0), q2 = hit_dmg(WP_SWORD, 5, 2, 0, 18, -1, 0), q0 = hit_dmg(WP_SWORD, 5, 0, 0, 18, -1, 0);
    CHECK(q0 < q1 && q1 < q2 && q2 < q3, "ordre des qualites : %d < %d < %d < %d", q0, q1, q2, q3);
    CHECK(hit_dmg(WP_DAGGER, 1, 0, 0, 18, 1, 0) == 2 * WPN[WP_DAGGER].l.dmg, "dague : coup dans le dos x2");
    CHECK(hit_dmg(WP_RAPIER, 1, 0, 0, 30, -1, 0) > WPN[WP_RAPIER].l.dmg, "rapiere : coup de pointe majore");
    printf("    epee : niveau I = %d, niveau XV = %d, V+S = %d\n", base, hi, q3);
    /* faux-serres : critique en alternant les deux attaques */
    new_game(9); G.sv.unlocked = 0x3FF; equip(0, WP_SCYTHE, 1, 0); equip(1, WP_SCYTHE, 1, 0); G.p.cur = 0; tp(5 * 16 + 8, 13 * 16); run(0, 3); settle();
    Ent *e = dummy(ET_JAIL, 18, -1); int hp0 = e->hp;
    run(K_OK, 2); run(0, 25); e->st = ES_REC; int a1 = hp0 - e->hp; hp0 = e->hp; settle();
    game_update(K_SWAP); run(0, 20); e->st = ES_REC; int b1 = hp0 - e->hp; hp0 = e->hp; settle();
    game_update(K_SWAP); run(0, 20); e->st = ES_REC; int b2 = hp0 - e->hp;
    CHECK(a1 == WPN[WP_SCYTHE].l.dmg, "faux-serres A : %d", a1);
    CHECK(b1 == 2 * WPN[WP_SCYTHE].b.dmg, "serres B apres A = critique : %d (attendu %d)", b1, 2 * WPN[WP_SCYTHE].b.dmg);
    CHECK(b2 == WPN[WP_SCYTHE].b.dmg, "serres B apres B = pas de critique : %d", b2);
    printf("    faux-serres : A=%d, B apres A=%d (critique), B apres B=%d\n", a1, b1, b2);
    /* arme double : prend les deux emplacements ; changement d'arme */
    new_game(4); equip(0, WP_SWORD, 1, 0); equip(1, WP_DAGGER, 1, 0); G.p.cur = 0; settle(); game_update(K_SWAP); game_update(0);
    CHECK(G.p.cur == 1, "changer d'arme");
}

/* ---------------------------------------------------------------- T05 bouclier : garde, parade, projectiles, garde ennemie */
static void t_shield(void) {
    GROUP("T05 bouclier : garde, parade, renvoi de projectile, garde-bouclier ennemi");
    new_game(5); equip(0, WP_SHIELD, 1, 0); G.p.cur = 0; tp(5 * 16 + 8, 13 * 16); run(0, 3); settle();
    Ent *j = dummy(ET_JAIL, 14, -1);
    run(K_OK, 3); CHECK(G.p.guard, "garde levee");
    g_src = j; hurt_player(20, PX(G.p.x) + 10, 3); g_src = 0;
    CHECK(G.p.hp == G.p.maxhp && j->st == ES_HURT, "parade parfaite : hp=%d/%d, attaquant etourdi=%d", G.p.hp, G.p.maxhp, j->st == ES_HURT);
    settle(); run(K_OK, 20); G.p.inv = 0; hurt_player(20, PX(G.p.x) + 10, 3);
    CHECK(G.p.hp == G.p.maxhp - 4, "garde tardive : 80 %% absorbes (hp=%d)", G.p.hp);
    settle(); run(K_OK, 20); G.p.inv = 0; hurt_player(20, PX(G.p.x) - 10, 3);
    CHECK(G.p.hp == G.p.maxhp - 20, "coup dans le dos non pare (hp=%d)", G.p.hp);
    settle(); G.p.guard_t = 0; G.p.guard = 0; G.keys = 0; G.prev = 0; run(K_OK, 3); G.p.inv = 0; clear_world();
    add_proj(PJ_BOLT, 1, PX(G.p.x) + 12, PX(G.p.y) - 12, -72, 0, 10, 60, -1);
    run(K_OK, 2); int reflected = 0; for (int i = 0; i < MAX_PR; i++) if (G.pr[i].life && G.pr[i].owner == 0) reflected++;
    CHECK(reflected == 1 && G.p.hp == G.p.maxhp, "projectile renvoye (%d) sans degat", reflected);
    /* garde-bouclier ennemi */
    new_game(6); tp(5 * 16 + 8, 13 * 16); run(0, 3); settle();
    Ent *s = dummy(ET_SHLD, 20, -1); s->st = ES_CHASE; int hp0 = s->hp;
    hurt_enemy(s, 10, 1, 3, 6); CHECK(s->hp == hp0, "bouclier : coup leger de face bloque");
    hurt_enemy(s, 10, 1, 3, 20); CHECK(s->hp < hp0 && s->st == ES_HURT, "bouclier : coup lourd brise la garde");
    s->st = ES_CHASE; hp0 = s->hp; hurt_enemy(s, 10, -1, 3, 6); CHECK(s->hp < hp0, "bouclier : coup dans le dos touche");
    s->st = ES_CHASE; hp0 = s->hp; g_pierce = 1; hurt_enemy(s, 10, 1, 3, 6); g_pierce = 0; CHECK(s->hp < hp0, "bouclier : l'arbalete le perce");
    s->st = ES_WIND; hp0 = s->hp; hurt_enemy(s, 10, 1, 3, 6); CHECK(s->hp < hp0, "bouclier : vulnerable pendant son coup de bouclier");
}

/* ---------------------------------------------------------------- T06 objets (niveau / qualite) */
static void t_items(void) {
    GROUP("T06 objets : niveaux I..XV, qualites, etiquettes");
    char lab[8]; Item it;
    static const char *R[15] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII", "XIII", "XIV", "XV" };
    for (int i = 1; i <= 15; i++) CHECK(strcmp(roman(i), R[i - 1]) == 0, "chiffre romain %d", i);
    it.id = WP_SWORD; it.lvl = 12; it.q = 2; item_label(&it, lab); CHECK(strcmp(lab, "XII++") == 0, "etiquette %s", lab);
    it.lvl = 15; it.q = 4; item_label(&it, lab); CHECK(strcmp(lab, "XVL") == 0, "etiquette %s", lab);
    it.lvl = 3; it.q = 3; item_label(&it, lab); CHECK(strcmp(lab, "IIIS") == 0, "etiquette %s", lab);
    new_game(8); int cnt[5] = { 0 }, lmin = 99, lmax = 0;
    for (int z = 0; z < NZONES; z++) { G.zone = (uint8_t)z; for (int i = 0; i < 2000; i++) { item_make(&it, WP_SWORD, 0, -1); cnt[it.q]++; if (it.lvl < lmin) lmin = it.lvl; if (it.lvl > lmax) lmax = it.lvl; } }
    CHECK(lmin >= 1 && lmax <= 15, "niveaux dans 1..15 (%d..%d)", lmin, lmax);
    CHECK(cnt[3] == 0 && cnt[4] == 0 && cnt[1] > 0 && cnt[2] > 0, "sans amelioration : seulement rien/+/++ (S=%d L=%d)", cnt[3], cnt[4]);
    G.sv.up[UP_QUAL] = 3; memset(cnt, 0, sizeof cnt);
    for (int i = 0; i < 4000; i++) { item_make(&it, WP_SWORD, 0, -1); cnt[it.q]++; }
    CHECK(cnt[3] > 0 && cnt[4] == 0, "qualite S possible avec l'amelioration (S=%d), L jamais au hasard", cnt[3]);
    item_make(&it, WP_BOW, 2, 4); CHECK(it.q == 4, "qualite legendaire forcee");
    G.zone = 4; item_make(&it, WP_BOW, 5, -1); CHECK(it.lvl >= 10 && it.lvl <= 15, "niveau en zone 5 : %d", it.lvl);
    printf("    qualites tirees (amelioration niveau 3) : rien %d, + %d, ++ %d, S %d (L jamais au hasard)\n", cnt[0], cnt[1], cnt[2], cnt[3]);
}

/* ---------------------------------------------------------------- T07 ennemis */
static void t_enemies(void) {
    GROUP("T07 ennemis : detection, deplacement, attaque, mort, butin");
    static const char *nm[ET_COUNT] = { "geolier", "arbaletrier", "rat", "brute", "slime", "artificier", "garde-bouclier", "putride", "chauve-souris", "Gobeur", "Roi Fele" };
    for (int t = 0; t < ET_GOB; t++) {
        new_game(100 + t); tp(5 * 16 + 8, 13 * 16); run(0, 3); settle(); clear_world();
        spawn_enemy(t, (PX(G.p.x) + 110) / 16, t == ET_BAT ? 8 : 12, 0);
        Ent *e = newest(); e->elite = 0; if (t == ET_BAT) { e->y = F(9 * 16); }
        int hurt = 0, moved = 0, attacked = 0, x0 = PX(e->x), shots = 0;
        for (int i = 0; i < 500; i++) {
            game_update(0);
            if (G.p.hp < G.p.maxhp) { hurt++; G.p.hp = G.p.maxhp; }
            if (iabs(PX(e->x) - x0) > 6) moved = 1;
            if (e->st == ES_ATK || e->st == ES_WIND) attacked = 1;
            shots += count_pr(1) > 0;
            G.p.inv = 0; if (G.p.st == PS_HURT) G.p.st = PS_NORMAL;
        }
        CHECK(moved || attacked, "%s : ne bouge pas et n'attaque pas", nm[t]);
        if (t != ET_XBOW && t != ET_GREN) CHECK(hurt > 0 || t == ET_SLIME, "%s : n'a jamais blesse le joueur en 500 images", nm[t]);
        if (t == ET_XBOW || t == ET_GREN) CHECK(shots > 0, "%s : aucun tir", nm[t]);
        printf("    %-16s degats subis %2d fois, projectiles en vol %3d images\n", nm[t], hurt, shots);
        hurt_enemy(e, 99999, 1, 0, 0); CHECK(e->st == ES_DEAD, "%s : mort", nm[t]);
        if (t == ET_TOXIC) { int cl = 0; for (int i = 0; i < MAX_PR; i++) cl += G.pr[i].life && G.pr[i].type == PJ_CLOUD; CHECK(cl == 1, "putride : nuage de poison a sa mort"); }
    }
    /* butin : pieces, cellules, schemas, elites */
    new_game(7); G.sv.found = 3; long coins = 0, cells = 0; int kills = 0, bps = 0, hearts = 0;
    for (int n = 0; n < 400; n++) {
        clear_world(); G.bp_carry = 0; spawn_enemy(n % 4 == 0 ? ET_JAIL : ET_RAT, 6, 12, 1); Ent *e = newest(); if (!e) continue;
        G.run_gold = 0; G.cells_run = 0; hurt_enemy(e, 99999, 1, 0, 0); run(0, 60); kills++; coins += G.run_gold; cells += G.cells_run;
        hearts += find_pick(PK_HEART) != 0; bps += find_pick(PK_BLUE) != 0;
    }
    CHECK(coins > 0 && cells > 0, "butin : pieces %ld, cellules %ld", coins, cells);
    printf("    butin sur %d ennemis : %ld pieces, %ld cellules, %d coeurs, %d schemas\n", kills, coins, cells, hearts, bps);
    new_game(7); int el = 0, tot = 0;
    for (int n = 0; n < 3000; n++) { memset(G.en, 0, sizeof G.en); spawn_enemy(ET_JAIL, 6, 12, 4); Ent *e = newest(); tot++; if (e->elite) { el++; CHECK(e->hp > e->maxhp - 1 && e->maxhp > 36 * 188 / 100 * 2, "elite : PV x2,2"); } }
    CHECK(el > 100 && el < 400, "proportion d'elites en zone 5 : %d / %d", el, tot);
}

/* ---------------------------------------------------------------- T08 boss */
static void t_bosses(void) {
    GROUP("T08 boss : arene, attaques, mort, butin legendaire, victoire");
    for (int z = 2; z <= 4; z += 2) {
        new_game(50 + z); level_build(z); G.scene = SC_PLAY; settle(); clear_world();
        spawn_enemy(z == 2 ? ET_GOB : ET_KING, G.arena_x0 / 16 + 13, 12, z); Ent *b = newest(); G.boss_idx = (int16_t)(b - G.en); b->st = ES_SLEEP; G.boss_alive = 0;
        tp(G.arena_x0 + 3 * 16, 13 * 16); run(0, 3);
        CHECK(!G.arena_on, "arene : pas de verrouillage avant d'entrer");
        tp(G.arena_x0 + 6 * 16, 13 * 16); run(0, 3);
        CHECK(G.arena_on && G.boss_alive && G.map[5][G.arena_x0 / 16] == TL_GATE, "arene verrouillee (z=%d)", z);
        int seen[3] = { 0, 0, 0 }, waves = 0, spits = 0;
        for (int i = 0; i < 4000; i++) {
            game_update(0); G.p.hp = G.p.maxhp; G.p.inv = 0;
            if (b->st == ES_ATK || b->st == ES_AIR || b->st == ES_WIND) seen[b->aux % 3] = 1;
            for (int k = 0; k < MAX_PR; k++) if (G.pr[k].life) { waves += G.pr[k].type == PJ_WAVE; spits += G.pr[k].type == PJ_SPIT; }
        }
        CHECK(seen[0] && seen[1] && seen[2], "%s : %d types d'attaque vus", z == 2 ? "Gobeur" : "Roi", seen[0] + seen[1] + seen[2]);
        CHECK(waves > 0, "ondes de choc");
        if (z == 2) CHECK(spits > 0, "Gobeur : crachats");
        printf("    %s : attaques vues %d/3, ondes %d, crachats %d\n", z == 2 ? "Gobeur" : "Roi Fele", seen[0] + seen[1] + seen[2], waves, spits);
        b->hp = 1; hurt_enemy(b, 99999, 1, 0, 0); run(0, 70);
        CHECK(!G.boss_alive && G.map[5][G.arena_x0 / 16] == TL_EMPTY && G.map[G.door_ty][G.door_tx] == TL_DOOR, "boss mort : porte ouverte et sortie apparue");
        if (z == 2) { Pick *w = find_pick(PK_WPN); CHECK(w && w->q == 4, "Gobeur : arme legendaire (qualite L) lachee"); }
        else { CHECK(G.victory, "Roi : victoire declenchee"); run(0, 150); CHECK(G.scene == SC_WIN, "ecran de victoire"); }
    }
}

/* ---------------------------------------------------------------- T09 passages, PNJ, boutique, mutations */
static void t_passage(void) {
    GROUP("T09 passage : cellules, Passeur (ameliorations, schemas), Oracle (mutations), changement de zone");
    new_game(60); tp((G.nrooms - 1) * 320 + 16 * 16 + 8, 13 * 16); G.cells_run = 7; G.bp_carry = 1 << WP_HAMMER; G.run_gold = 100; G.p.hp = 50; G.p.flasks = 1;
    uint32_t c0 = G.sv.cells; run(0, 3);
    CHECK(G.passage == 1 && G.zone == 0, "entree en passage");
    CHECK(G.sv.cells == c0 + 7 && G.cells_run == 0, "cellules mises en lieu sur : %u", (unsigned)G.sv.cells);
    CHECK((G.sv.found >> WP_HAMMER) & 1, "schema livre");
    CHECK(G.p.hp > 50 && G.p.flasks == 2, "soin partiel (+30 %%) et fiole offerte : hp=%d fioles=%d", G.p.hp, G.p.flasks);
    int npc = 0; for (int i = 0; i < MAX_PK; i++) npc += G.pk[i].life && G.pk[i].kind == PK_NPC;
    CHECK(npc == 2, "2 PNJ presents (%d)", npc);
    CHECK(G.mut_pending && G.mut_off[0] != G.mut_off[1] && G.mut_off[1] != G.mut_off[2] && G.mut_off[0] < MU_COUNT, "3 mutations proposees distinctes");
    /* Passeur */
    G.sv.cells = 100; tp(6 * 16 + 8, 13 * 16); settle(); run(0, 2); run(K_OK, 1);
    CHECK(G.scene == SC_SHOP, "dialogue avec le Passeur");
    G.menu_sel = 0; run(0, 1); run(K_OK, 1); run(0, 1);
    CHECK(G.sv.up[UP_HP] == 1 && G.sv.cells == 90 && G.p.maxhp == 110, "amelioration achetee : vie niveau %d, cellules %u, PV max %d", G.sv.up[UP_HP], (unsigned)G.sv.cells, G.p.maxhp);
    uint8_t k[24], a[24]; int n = shop_rows(k, a); int row = -1; for (int i = 0; i < n; i++) if (k[i] == RW_BP && a[i] == WP_HAMMER) row = i;
    CHECK(row >= 0, "schema du marteau propose"); G.menu_sel = (int16_t)row; run(0, 1); run(K_OK, 1); run(0, 1);
    CHECK((G.sv.unlocked >> WP_HAMMER) & 1, "schema achete : marteau debloque");
    int lvl0 = G.p.w[G.p.cur].lvl, i0 = 0; for (int i = 0; i < n; i++) if (k[i] == RW_WLVL) i0 = i;
    n = shop_rows(k, a); for (int i = 0; i < n; i++) if (k[i] == RW_WLVL) i0 = i;
    G.menu_sel = (int16_t)i0; G.run_gold = 500; run(0, 1); run(K_OK, 1); run(0, 1);
    CHECK(G.p.w[G.p.cur].lvl == lvl0 + 1 && G.run_gold < 500, "niveau de l'arme +1 contre des pieces");
    G.sv.cells = 0; G.menu_sel = 1; run(0, 1); run(K_OK, 1); CHECK(G.sv.up[UP_DMG] == 0, "pas assez de cellules : achat refuse");
    run(K_BACK, 1); run(0, 1); CHECK(G.scene == SC_PLAY, "retour au passage");
    /* Oracle */
    tp(11 * 16 + 8, 13 * 16); settle(); run(0, 2); run(K_OK, 1); CHECK(G.scene == SC_MUT, "dialogue avec l'Oracle");
    int choice = G.mut_off[1]; G.menu_sel = 1; run(0, 1); run(K_OK, 1); run(0, 1);
    CHECK(G.mut_n == 1 && G.mut[0] == choice && !G.mut_pending && G.scene == SC_PLAY, "mutation choisie (%d)", choice);
    /* sortie vers la zone 2 */
    tp(17 * 16 + 8, 13 * 16); run(0, 4);
    CHECK(G.zone == 1 && !G.passage && G.mut_n == 1, "zone 2 atteinte, mutation conservee");
    /* 2e passage : une mutation de plus */
    tp((G.nrooms - 1) * 320 + 16 * 16 + 8, 13 * 16); clear_world(); run(0, 4);
    CHECK(G.passage && G.mut_pending, "2e passage : nouvelle mutation proposee");
    int offered_dup = 0; for (int i = 0; i < 3; i++) offered_dup += G.mut_off[i] == choice; CHECK(!offered_dup, "pas de doublon de mutation");
}

static void t_mutations(void) {
    GROUP("T10 mutations (valables pour la partie)");
    new_game(70); G.sv.unlocked = 0x3FF; equip(0, WP_SWORD, 1, 0); G.p.cur = 0; tp(5 * 16 + 8, 13 * 16); run(0, 3); settle();
    int base = hit_dmg(WP_SWORD, 1, 0, 0, 18, -1, 0);
    new_game(70); G.sv.unlocked = 0x3FF; equip(0, WP_SWORD, 1, 0); G.p.cur = 0; tp(5 * 16 + 8, 13 * 16); run(0, 3); settle();
    G.mut[0] = MU_REVENGE; G.mut_n = 1; G.p.inv = 0; hurt_player(10, PX(G.p.x) + 20, 0); CHECK(G.p.mt_veng > 0, "revanche armee apres un coup recu");
    settle(); G.p.mt_veng = 80; Ent *e = dummy(ET_JAIL, 18, -1); int hp0 = e->hp; run(K_OK, 2); for (int i = 0; i < 40; i++) { game_update(0); e->st = ES_REC; e->t = 30000; }
    CHECK(hp0 - e->hp == base * 160 / 100, "revanche : +60 %% (%d, attendu %d)", hp0 - e->hp, base * 160 / 100);
    new_game(71); G.mut[0] = MU_CARAPACE; G.mut_n = 1; G.p.inv = 0; hurt_player(20, 0, 0); CHECK(G.p.hp == G.p.maxhp - 16, "carapace : -20 %% (hp=%d)", G.p.hp);
    new_game(72); G.mut[0] = MU_INSTINCT; G.mut_n = 1; G.p.hp = 50; e = dummy(ET_JAIL, 20, -1); hurt_enemy(e, 99999, 1, 0, 0); CHECK(G.p.hp == 54, "instinct : +4 PV par ennemi tue (hp=%d)", G.p.hp);
    new_game(73); G.mut[0] = MU_GLUTTON; G.mut_n = 1; G.p.hp = 10; G.p.flasks = 1; settle(); G.p.hp = 10; game_update(K_FLASK);
    CHECK(G.p.hp == 10 + G.p.maxhp * 65 / 100, "gourmand : fiole +30 %% (hp=%d)", G.p.hp);
    new_game(74); G.mut[0] = MU_BLEED; G.mut_n = 1; equip(0, WP_SWORD, 1, 0); tp(5 * 16 + 8, 13 * 16); run(0, 3); settle();
    e = dummy(ET_JAIL, 18, -1); run(K_OK, 2); run(0, 30); CHECK(e->hp < 5000 - 12, "saignement : degats sur la duree (hp=%d)", e->hp);
    new_game(75); G.mut[0] = MU_COMBO; G.mut_n = 1; equip(0, WP_DAGGER, 1, 0); tp(5 * 16 + 8, 13 * 16); run(0, 3); settle();
    e = dummy(ET_JAIL, 18, -1); int h0 = e->hp; for (int i = 0; i < 4; i++) { run(K_OK, 2); for (int j = 0; j < 14; j++) { game_update(0); e->st = ES_REC; e->t = 30000; } }
    CHECK(h0 - e->hp > 4 * 7, "enchainement : les coups successifs s'amplifient (%d > %d)", h0 - e->hp, 4 * 7);
    new_game(76); G.mut[0] = MU_ADREN; G.mut_n = 1; tp(5 * 16 + 8, 13 * 16); run(0, 3); settle(); G.p.mt_adr = 90; run(K_R, 40); int x1 = PX(G.p.x);
    new_game(76); tp(5 * 16 + 8, 13 * 16); run(0, 3); settle(); run(K_R, 40); CHECK(x1 > PX(G.p.x), "adrenaline : plus vite (%d > %d)", x1, PX(G.p.x));
    new_game(77); G.mut[0] = MU_LUCK; G.mut_n = 1; long g1 = 0, g2 = 0;
    for (int n = 0; n < 200; n++) { clear_world(); G.run_gold = 0; spawn_enemy(ET_BRUTE, 6, 12, 0); hurt_enemy(newest(), 99999, 1, 0, 0); run(0, 50); g1 += G.run_gold; }
    G.mut_n = 0; for (int n = 0; n < 200; n++) { clear_world(); G.run_gold = 0; spawn_enemy(ET_BRUTE, 6, 12, 0); hurt_enemy(newest(), 99999, 1, 0, 0); run(0, 50); g2 += G.run_gold; }
    CHECK(g1 > g2, "chance : butin accru (%ld > %ld)", g1, g2);
}

/* ---------------------------------------------------------------- T11 sauvegarde */
static void t_save(void) {
    GROUP("T11 sauvegarde : ecriture, relecture, corruption, parametres");
    new_game(80); G.sv.cells = 1234; G.sv.up[UP_HP] = 3; G.sv.unlocked |= 1 << WP_BOW; G.sv.opt_fx = 0; G.sv.best_zone = 4; save_store();
    game_init(81);
    CHECK(G.sv.cells == 1234 && G.sv.up[UP_HP] == 3 && ((G.sv.unlocked >> WP_BOW) & 1) && G.sv.opt_fx == 0 && G.sv.best_zone == 4, "relecture fidele (cellules=%u)", (unsigned)G.sv.cells);
    FILE *f = fopen(SAVEF, "r+b"); fseek(f, 12, SEEK_SET); fputc(0x5A, f); fclose(f);
    game_init(82); CHECK(G.sv.cells == 0 && G.sv.opt_fx == 1, "sauvegarde corrompue detectee -> valeurs par defaut");
    remove(SAVEF); game_init(83); CHECK(G.sv.cells == 0 && G.save_ok, "premiere execution sans fichier");
    new_game(84); G.menu_sel = 0; G.scene = SC_OPTIONS; run(K_OK, 1); CHECK(G.sv.opt_fx == 0, "option effets basculee");
    printf("    taille de la sauvegarde : %d octets\n", (int)sizeof(Save));
}

/* ---------------------------------------------------------------- T12 rendu */
static uint16_t FB[SCR_W * SCR_H];
static void render(void) { for (int y = 0; y < SCR_H; y += STRIP_H) game_render_strip(FB + y * SCR_W, y); }
static int distinct(void) { int d = 0; uint16_t seen[64]; for (int i = 0; i < SCR_W * SCR_H && d < 64; i += 7) { int k; for (k = 0; k < d; k++) if (seen[k] == FB[i]) break; if (k == d) seen[d++] = FB[i]; } return d; }
static void t_render(void) {
    GROUP("T12 rendu : toutes les scenes, sans debordement (ASan) et non vides");
    static const char *nm[] = { "titre", "jeu", "pause", "boutique", "options", "aide", "mort", "victoire", "oracle" };
    int sc[] = { SC_TITLE, SC_PLAY, SC_PAUSE, SC_SHOP, SC_OPTIONS, SC_HELP, SC_DEAD, SC_WIN, SC_MUT };
    new_game(90); for (int i = 0; i < 9; i++) { G.scene = (uint8_t)sc[i]; G.in_run = 1; G.mut_pending = 1; G.mut_off[0] = 0; G.mut_off[1] = 1; G.mut_off[2] = 2; render(); CHECK(distinct() >= 4, "scene %s : image presque vide (%d couleurs)", nm[i], distinct()); }
    for (int z = 0; z < NZONES; z++) { level_build(z); G.scene = SC_PLAY; tp(1000, 13 * 16); run(0, 10); render(); CHECK(distinct() >= 8, "zone %d : %d couleurs", z + 1, distinct()); }
    /* aucun ecrasement hors de la bande : tampon de garde */
    static uint16_t guard[SCR_W * STRIP_H + 2 * SCR_W]; for (int i = 0; i < SCR_W * STRIP_H + 2 * SCR_W; i++) guard[i] = 0xBEEF;
    G.scene = SC_PLAY; level_build(0); for (int y = 0; y < SCR_H; y += STRIP_H) { game_render_strip(guard + SCR_W, y); }
    int ok = 1; for (int i = 0; i < SCR_W; i++) if (guard[i] != 0xBEEF || guard[SCR_W + SCR_W * STRIP_H + i] != 0xBEEF) ok = 0;
    CHECK(ok, "le rendu deborde de sa bande");
}

/* ---------------------------------------------------------------- T13 fuzz : touches aleatoires, invariants */
static uint32_t fz = 12345;
static uint32_t frnd(void) { fz = fz * 1664525u + 1013904223u; return fz >> 8; }
static void t_fuzz(void) {
    GROUP("T13 stabilite : 300 000 images de touches aleatoires (invariants verifies a chaque image)");
    long frames = 0; int bad = 0;
    for (int s = 0; s < 12; s++) {
        new_game(200 + s); G.sv.unlocked = 0x3FF; if (s & 1) level_build(s % NZONES);
        for (int i = 0; i < 25000; i++) {
            uint16_t k = (uint16_t)(frnd() & 0x1FF); if ((frnd() & 7) == 0) k = 0;
            if (G.scene == SC_PLAY && (frnd() % 900) == 0) { G.p.hp = G.p.maxhp; }
            game_update(k); frames++;
            if (G.scene == SC_PLAY || G.scene == SC_PAUSE) {
                int ok = G.p.x >= 0 && PX(G.p.x) <= G.lvw * TS && G.p.hp <= G.p.maxhp && G.p.hp >= 0 && G.p.cur < 2 && G.p.w[0].lvl >= 1 && G.p.w[0].lvl <= 16 && G.p.w[1].lvl <= 16
                    && G.mut_n <= 4 && G.zone < NZONES && PX(G.p.y) < 40 * TS;
                if (!ok && bad < 5) { bad++; CHECK(0, "invariant viole : x=%d hp=%d zone=%d scene=%d", PX(G.p.x), G.p.hp, G.zone, G.scene); }
            }
            if (G.scene == SC_DEAD || G.scene == SC_WIN) { game_update(K_OK); game_update(0); game_update(K_BACK); game_update(0); G.scene = SC_TITLE; game_update(K_OK); game_update(0); G.sv.unlocked = 0x3FF; }
            if ((i % 2500) == 0) render();
        }
    }
    CHECK(bad == 0, "invariants"); printf("    %ld images simulees sans plantage\n", frames);
}

/* ---------------------------------------------------------------- T14 robot : parties completes */
static int bot_jump_t, bot_tap, bot_stuck, bot_lastx;
static int good_wpn(int s) { int k = WPN[G.p.w[s].id].kind; return k == WK_MELEE || k == WK_DUAL; }
/* fight = 0 : ne cherche que le boss (invulnerable) ; fight = 1 : poursuit et attaque les ennemis proches */
static uint16_t bot_keys(int fight) {
    Player *p = &G.p; int px = PX(p->x), py = PX(p->y); uint16_t k = 0;
    if (G.scene != SC_PLAY) return K_BACK;
    Ent *t = 0; int best = 9999;
    if (fight) for (int i = 0; i < MAX_EN; i++) {
        Ent *e = &G.en[i];
        if (!e->alive || e->st == ES_DEAD || e->st == ES_SLEEP) continue;
        int dx = PX(e->x) - px, dy = PX(e->y) - py;
        if (iabs(dy) < (e->type == ET_BAT ? 80 : 40) && iabs(dx) < best && iabs(dx) < 110) { best = iabs(dx); t = e; }
    }
    int boss = G.arena_on && G.boss_alive && G.boss_idx >= 0 && G.en[G.boss_idx].alive;
    if (boss) { t = &G.en[G.boss_idx]; best = iabs(PX(t->x) - px); }
    int dir = t ? ((PX(t->x) > px) ? 1 : -1) : 1;
    int engaged = t && best < 34;
    if (boss && !good_wpn(p->cur) && good_wpn(p->cur ^ 1) && (G.frame & 1)) k |= K_SWAP;      /* prend l'arme de melee avant le boss */
    if (engaged) {
        if (p->dir != dir) k |= (dir > 0 ? K_R : K_L);
        else if (++bot_tap % 12 < 2) k |= K_OK;
    } else k |= (dir > 0 ? K_R : K_L);
    int ahead = px + dir * 13;
    int wall = is_solid(tile_at(ahead >> 4, (py - 8) >> 4)) || is_solid(tile_at(ahead >> 4, (py - 20) >> 4));
    int under = tile_at((px + dir * 18) >> 4, py >> 4), under2 = tile_at((px + dir * 34) >> 4, py >> 4);
    int pit = !(is_solid(under) || under == TL_PLAT) && p->ground;
    int spike = tile_at((px + dir * 14) >> 4, (py - 4) >> 4) == TL_SPIKE || tile_at((px + dir * 26) >> 4, (py - 4) >> 4) == TL_SPIKE;
    int pool = under == TL_TOXIC || under2 == TL_TOXIC;
    int need = wall || pit || spike || pool;
    if (p->x == (int32_t)bot_lastx && !engaged) bot_stuck++; else bot_stuck = 0;
    bot_lastx = (int)p->x;
    if (need && p->ground && !engaged) bot_jump_t = 15;
    if (bot_stuck > 40 && p->ground) bot_jump_t = 15;
    if (bot_jump_t > 0) { k |= K_U; bot_jump_t--; if (bot_jump_t == 8) k &= (uint16_t)~K_U; }
    if (!p->ground && p->vy > 10 && p->jumps < 2 && (need || bot_stuck > 40) && (G.frame & 1)) k |= K_U;
    return k;
}
static void t_bot(void) {
    GROUP("T14 robot : parties completes (5 zones, 2 boss, 4 passages) puis parties \"normales\"");
    int wins = 0, N = 40, passages = 0, bosses = 0; long tot = 0, worst = 0;
    for (int s = 1; s <= N; s++) {
        new_game((unsigned)(1000 + s * 7)); G.sv.unlocked = 0x3FF; bot_jump_t = bot_tap = bot_stuck = 0; bot_lastx = -1;
        long f = 0; int zmax = 0, pz = 0, prev_pass = 0, bz = 0, prev_arena = 0;
        while (f < 120000 && G.scene != SC_WIN && G.scene != SC_TITLE) {
            uint16_t k = bot_keys(0);
            if (G.scene == SC_PLAY) { G.p.hp = G.p.maxhp; if (G.p.st == PS_DEAD) G.p.st = PS_NORMAL; G.p.inv = 5; }
            game_update(k); f++;
            if (G.zone > zmax) zmax = G.zone;
            if (G.passage && !prev_pass) pz++;
            prev_pass = G.passage;
            if (G.arena_on && !prev_arena) bz++;
            prev_arena = G.arena_on;
        }
        passages += pz; bosses += bz;
        if (G.scene == SC_WIN) { wins++; tot += f; if (f > worst) worst = f; CHECK(pz == 4 && bz == 2, "graine %d : %d passages, %d arenes", s, pz, bz); }
        else { CHECK(0, "graine %d : bloque en zone %d apres %ld images (x=%d, passage=%d)", s, zmax + 1, f, PX(G.p.x), G.passage); }
    }
    CHECK(wins == N, "parties terminees : %d / %d", wins, N);
    if (wins) printf("    %d / %d parties finies (zone 1 -> victoire) ; duree moyenne %ld images (%.1f min de jeu), pire %ld ; %d passages et %d combats de boss traverses\n", wins, N, tot / wins, tot / wins / 30.0 / 60.0, worst, passages, bosses);
    int deaths = 0, zsum = 0, wn = 0, M = 40;
    for (int s = 1; s <= M; s++) {
        new_game((unsigned)(5000 + s * 11)); bot_jump_t = bot_tap = bot_stuck = 0; bot_lastx = -1;
        long f = 0;
        while (f < 60000 && G.scene != SC_WIN && G.scene != SC_DEAD && G.scene != SC_TITLE) { game_update(bot_keys(1)); f++; }
        if (G.scene == SC_DEAD) { deaths++; zsum += G.zone + 1; } else if (G.scene == SC_WIN) wn++;
    }
    printf("    robot NAIF sans invulnerabilite (ne sait ni esquiver ni parer) : %d / %d morts, zone moyenne de la mort %.1f, %d victoires\n", deaths, M, deaths ? (double)zsum / deaths : 0.0, wn);
}

/* ---------------------------------------------------------------- mesures */
static void t_perf(void) {
    GROUP("T15 mesures (PC, a titre indicatif)");
    new_game(300); level_build(2); tp(1000, 13 * 16); run(0, 10);
    clock_t c0 = clock(); for (int i = 0; i < 20000; i++) { game_update(i & 1 ? K_R : 0); G.p.hp = G.p.maxhp; } double up = (double)(clock() - c0) / CLOCKS_PER_SEC / 20000.0 * 1e6;
    c0 = clock(); for (int i = 0; i < 400; i++) render(); double rd = (double)(clock() - c0) / CLOCKS_PER_SEC / 400.0 * 1e6;
    printf("    mise a jour : %.1f us/image ; rendu plein ecran : %.0f us/image (PC, avec ASan : sans valeur absolue)\n", up, rd);
    printf("    sizeof(Game) = %d octets ; sizeof(Save) = %d ; bande de rendu = %d octets ; donnees de sprites = %d octets\n", (int)sizeof(Game), (int)sizeof(Save), SCR_W * STRIP_H * 2, (int)sizeof(SPRDATA));
}

int main(int argc, char **argv) {
    int quick = argc > 1 && argv[1][0] == 'q';
    t_rooms(); t_levels(); t_physics(); t_weapons(); t_shield(); t_items(); t_enemies(); t_bosses(); t_passage(); t_mutations(); t_save(); t_render();
    if (!quick) { t_fuzz(); t_bot(); }
    t_perf();
    printf("\nRESULTAT : %d verifications, %d echecs\n", n_run, n_fail);
    return n_fail ? 1 : 0;
}
