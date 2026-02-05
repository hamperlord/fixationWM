#include "WindowManager.hpp"

int main() {
    std::cout << "Initializing fixationWM..." << std::endl;

    int screenNum;
    xcb_connection_t* connection =
    xcb_connect(NULL, NULL);

    /* I took this from the XCB docs lel */
    const xcb_setup_t      *setup  = xcb_get_setup (connection);
    xcb_screen_iterator_t   iter   = xcb_setup_roots_iterator (setup);
    xcb_screen_t           *screen = iter.data;

    if (xcb_connection_has_error(connection)) {
        printf("Connection to Xorg server failed.\n");
    }
    else {
        printf("Connection to Xorg server established.\n");
    }

    xcb_map_request_event_t* map_request;

    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(connection))) {
        switch (event->response_type & ~0x80)
        {
        case XCB_EXPOSE:
            printf ("Oop!");
            break;
        case XCB_MAP_REQUEST:
            xcb_window_t window = xcb_generate_id(connection);
            xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0, 0, 
                              500, 500, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, 
                              0, NULL);
        }
    }
    

    KillFixation(connection);

    return 0;
}