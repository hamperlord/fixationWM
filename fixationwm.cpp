#include "fixationwm.hpp"

xcb_connection_t *dpy;
xcb_generic_event_t *event;
xcb_screen_t *screen;

void grabMouse();
void grabKeyboard();
void grabInput();
void mapWindow(xcb_connection_t *connection, xcb_map_request_event_t *mapevent);

void setup()
{
    int screen_num;
    dpy = xcb_connect(NULL, &screen_num);
    if (xcb_connection_has_error(dpy))
        exit(1);
    else
        printf("fixation connected to display.\n");

    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(dpy));

    for (int i = 0; i < screen_num; i++)
        xcb_screen_next(&iter);

    screen = iter.data;
}

void shutdown();

int main(int argc, char *argv[])
{
    setup();
    shutdown();

    for (;;)
    {
        event = xcb_wait_for_event(dpy);
        switch (event->response_type & ~0x80)
        {
        case XCB_MAP_NOTIFY:
            mapWindow(dpy, ((xcb_map_request_event_t *)event));
            break;
        default:
            break;
        }
    }
}

void shutdown()
{
    xcb_disconnect(dpy);
    if (xcb_connection_has_error(dpy))
        printf("fixation shut down.\n");
}

void mapWindow(xcb_connection_t *connection, xcb_map_request_event_t *mapevent)
{
    const uint32_t value_masks[] = {};
    const uint32_t value_list[] = {};
    xcb_change_window_attributes(connection, mapevent->window, *value_masks, value_list);
    xcb_map_window(connection, mapevent->window);
}

void grabMouse()
{
}

void grabKeyboard()
{
}

void grabInput()
{
    grabMouse();
    grabKeyboard();
}