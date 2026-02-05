#include "WindowManager.hpp"

xcb_connection_t *dpy;
xcb_screen_t *screen;

void setup(void) {
    /* she's in love with the concept, as if we're all just how she imagined /lyr
    (but she just can't fucking code!) */

    int scr_num;
    dpy = xcb_connect(NULL, &scr_num);
}
