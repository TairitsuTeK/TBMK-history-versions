#include "port.h"
#include "mouse.h"
#include "wm.h"

void kernel_main(void)
{
    unsigned char* v = (unsigned char*)0xA0000;
    int mx, my, mb, last_mx = -1, last_my = -1, last_mb = 0;

    wm_init();
    wm_draw(v);
    mouse_init();

    while (1) {
        if (inb(0x64) & 0x01) {
            unsigned char st = inb(0x64);
            if (st & 0x20) mouse_handle_byte(inb(0x60));
            else inb(0x60);
        }
        mouse_get(&mx, &my, &mb);
        int mb_press = (mb & 1) && !(last_mb & 1);
        int mb_release = !(mb & 1) && (last_mb & 1);
        last_mb = mb;

        wm_update(v, mx, my, mb, mb_press, mb_release);
        wm_draw(v);

        if (wm_need_mouse_reset()) {
            mouse_reset();
            last_mx = -1; last_my = -1;
        }

        if (!wm_is_dragging() && (mx != last_mx || my != last_my)) {
            mouse_draw(v);
            last_mx = mx; last_my = my;
        }
    }
}