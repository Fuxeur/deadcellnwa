/* eadk_stub.c - simule l'ecran et le clavier de la NumWorks pour tester main_eadk.c (boucle, bandes, touches) sur PC. */
#include <eadk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint16_t FB[320 * 240];
static uint64_t now_ms;
static int scans, pushes, full_frames, rows_seen;
uint64_t eadk_timing_millis(void) { return now_ms += 9; }
void eadk_timing_msleep(uint32_t ms) { now_ms += ms; }
bool eadk_display_wait_for_vblank(void) { return true; }
void eadk_display_push_rect(eadk_rect_t r, const eadk_color_t *p) {
    if (r.width != 320 || r.height != 16 || r.y % 16 || r.y > 224) { fprintf(stderr, "rectangle inattendu %d,%d %dx%d\n", r.x, r.y, r.width, r.height); exit(2); }
    memcpy(FB + r.y * 320, p, 320 * 16 * 2); pushes++;
    if (r.y == 224) full_frames++;
    rows_seen |= 1 << (r.y / 16);
}
eadk_keyboard_state_t eadk_keyboard_scan(void) {
    int n = scans++;
    if (n < 40) return 0;
    if (n < 43) return 1ull << eadk_key_ok;                       /* JOUER */
    if (n < 400) return 1ull << eadk_key_right;                   /* course vers la droite */
    if (n < 460) return (1ull << eadk_key_right) | (1ull << eadk_key_up);
    if (n < 470) return 1ull << eadk_key_ok;
    if (n < 700) return (n & 1) ? 1ull << eadk_key_back : 0;       /* esquives */
    if (n < 710) return 1ull << eadk_key_home;                    /* quitter */
    return 0;
}
void stub_report(const char *ppm) {
    FILE *f = fopen(ppm, "wb");
    printf("boucle principale : %d lectures clavier, %d bandes envoyees, %d images completes, bandes vues=0x%X (0x7FFF attendu)\n", scans, pushes, full_frames, rows_seen);
    if (!f) return;
    fprintf(f, "P6\n320 240\n255\n");
    for (int i = 0; i < 320 * 240; i++) { uint16_t c = FB[i]; fputc(((c >> 11) & 31) * 255 / 31, f); fputc(((c >> 5) & 63) * 255 / 63, f); fputc((c & 31) * 255 / 31, f); }
    fclose(f);
}

__attribute__((destructor)) static void at_exit_report(void) { stub_report("/tmp/eadk_last.ppm"); }
