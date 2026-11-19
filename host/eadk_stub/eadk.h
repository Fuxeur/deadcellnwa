/* eadk.h FACTICE : ne sert qu'a verifier que main_eadk.c compile et boucle correctement sur PC.
   Le vrai en-tete est fourni par la chaine officielle NumWorks (nwlink) lors de la compilation du .nwa. */
#ifndef EADK_STUB_H
#define EADK_STUB_H
#include <stdint.h>
#include <stdbool.h>
typedef uint16_t eadk_color_t;
typedef struct { uint16_t x, y; } eadk_point_t;
typedef struct { uint16_t x, y, width, height; } eadk_rect_t;
typedef uint64_t eadk_keyboard_state_t;
typedef enum { eadk_key_left = 0, eadk_key_up = 1, eadk_key_down = 2, eadk_key_right = 3, eadk_key_ok = 4, eadk_key_back = 5, eadk_key_home = 6,
    eadk_key_on_off = 8, eadk_key_shift = 12, eadk_key_alpha = 13, eadk_key_xnt = 14, eadk_key_var = 15, eadk_key_toolbox = 16, eadk_key_backspace = 17,
    eadk_key_exe = 52 } eadk_key_t;
void eadk_display_push_rect(eadk_rect_t rect, const eadk_color_t *pixels);
bool eadk_display_wait_for_vblank(void);
eadk_keyboard_state_t eadk_keyboard_scan(void);
static inline bool eadk_keyboard_key_down(eadk_keyboard_state_t state, eadk_key_t key) { return (state >> key) & 1; }
uint64_t eadk_timing_millis(void);
void eadk_timing_msleep(uint32_t ms);
#endif
