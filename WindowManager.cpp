#include "WindowManager.hpp"

xcb_connection_t *dpy;
xcb_generic_event_t *event;
xcb_screen_t *screen;

void connect() {
    int scr_num;
    dpy = xcb_connect(NULL, &scr_num);
    const xcb_setup_t *setup = xcb_get_setup (dpy);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (setup);

    if (xcb_connection_has_error(dpy)) {
        printf("Connection Failed.");
        exit(1); 
    } else printf("Connection running.");

    // FROM XCB DOCS

    // we want the screen at index screenNum of the iterator
    for (int i = 0; i < scr_num; ++i) {
        xcb_screen_next (&iter);
    }

    screen = iter.data;
}

void setup() {

    uint32_t values[] = {
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY   |
        XCB_EVENT_MASK_POINTER_MOTION        |
        XCB_EVENT_MASK_BUTTON_PRESS
    };

    xcb_change_window_attributes(dpy, screen->root, XCB_CW_EVENT_MASK, values);

    connect();

    /* report */

    printf ("\n");
    printf ("Information for screen %" PRIu32 ":\n", screen->root);
    printf ("  width         : %" PRIu16 "\n", screen->width_in_pixels);
    printf ("  height        : %" PRIu16 "\n", screen->height_in_pixels);
    printf ("  white pixel   : %" PRIu32 "\n", screen->white_pixel);
    printf ("  black pixel   : %" PRIu32 "\n", screen->black_pixel);
    printf ("\n");

    // Grab inputs

    xcb_grab_pointer(dpy, 1, screen->root, XCB_EVENT_MASK_BUTTON_PRESS, 
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, screen->root, XCB_NONE, XCB_CURRENT_TIME);
    

    // Create mouse cursor

    xcb_cursor_t cursor = xcb_generate_id(dpy);
    xcb_cursor_t *pCursor = &cursor;
    xcb_font_t cfont = xcb_generate_id(dpy);
    xcb_open_font(dpy, cfont, strlen("cursor"), "cursor");
    xcb_create_glyph_cursor(dpy, cursor, cfont, cfont, 58, 58 + 1, 0, 0, 0, 0, 0, 0);
    xcb_change_window_attributes(dpy, screen->root, XCB_CW_CURSOR, pCursor);

    xcb_flush(dpy);
}

int main() {

    setup();

    // main event loop
    for (;;) {
        xcb_flush(dpy);

        event = xcb_wait_for_event(dpy);

        switch (event->response_type & ~0x80)
        {
        case XCB_MAP_REQUEST: {
            //im too lazy rn
            break; }
        case XCB_MOTION_NOTIFY: {
            xcb_query_pointer_cookie_t pointerCookie = xcb_query_pointer(dpy, screen->root);
            xcb_query_pointer_reply_t *pointerReply = xcb_query_pointer_reply(dpy, pointerCookie, nullptr);
            std::cout << "X: " << pointerReply->root_x << "\nY: " << pointerReply->root_y << "\n";
            free(pointerReply);
            break; }
        default: {
            break; }
        }

        free(event);
    }

    return 0;
}