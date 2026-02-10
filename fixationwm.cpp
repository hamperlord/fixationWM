#include "fixationwm.hpp"

xcb_connection_t *dpy;
xcb_generic_event_t *event;
xcb_screen_t *screen;

xcb_button_mask_t mouseLeft = XCB_BUTTON_MASK_1;
xcb_button_mask_t mouseMiddle = XCB_BUTTON_MASK_2;
xcb_button_mask_t mouseRight = XCB_BUTTON_MASK_3;

xcb_event_mask_t mouseMovement = XCB_EVENT_MASK_POINTER_MOTION;
xcb_event_mask_t mouseLeftMovement = XCB_EVENT_MASK_BUTTON_1_MOTION;
xcb_event_mask_t mouseMiddleMovement = XCB_EVENT_MASK_BUTTON_2_MOTION;
xcb_event_mask_t mouseRightMovement = XCB_EVENT_MASK_BUTTON_3_MOTION;
xcb_event_mask_t mouseButton = XCB_EVENT_MASK_BUTTON_PRESS;
xcb_event_mask_t mouseRelease = XCB_EVENT_MASK_BUTTON_RELEASE;

void grabMouse(xcb_connection_t *connection, xcb_window_t window);
void grabKeyboard(xcb_connection_t *connection, xcb_window_t window);
void grabInput(xcb_connection_t *connection, xcb_enter_notify_event_t *enterevent);
void mapWindow(xcb_connection_t *connection, xcb_map_request_event_t *mapevent);
void raiseWindow(xcb_connection_t *connection, xcb_window_t window);

void setup()
{
    int scr_num;
    dpy = xcb_connect(NULL, &scr_num);
    const xcb_setup_t *setup = xcb_get_setup(dpy);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

    if (xcb_connection_has_error(dpy))
    {
        printf("fixation failed.\n");
        exit(1);
    }
    else
        printf("fixation running.\n");

    for (int i = 0; i < scr_num; ++i)
    {
        xcb_screen_next(&iter);
    }

    screen = iter.data;

    uint32_t values[] = {
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
        mouseMovement |
        mouseButton |
        XCB_EVENT_MASK_ENTER_WINDOW};

    xcb_change_window_attributes(dpy, screen->root, XCB_CW_EVENT_MASK, values);

    xcb_flush(dpy);
}

void shutdown();

int main(int argc, char *argv[])
{
    setup();

    while (1)
    {
        event = xcb_wait_for_event(dpy);
        switch (event->response_type & ~0x80)
        {
        case XCB_MAP_REQUEST:
        {
            mapWindow(dpy, ((xcb_map_request_event_t *)event));
            xcb_flush(dpy);
            printf("window mapped");
            break;
        }
        case XCB_ENTER_NOTIFY:
        {
            printf("some window entered");
            xcb_window_t win = ((xcb_enter_notify_event_t *)event)->event;
            if (win != XCB_NONE)
            {
                // grabInput(dpy, (xcb_enter_notify_event_t *)event);
                printf("focus change\n");
                raiseWindow(dpy, win);
                xcb_flush(dpy);
            }
            break;
        }
        case XCB_BUTTON_PRESS:
        {

            break;
        }
        case XCB_KEY_PRESS:
        {
            xcb_keycode_t keypress = ((xcb_key_press_event_t *)event)->detail;
            printf("%u", keypress);
            switch (keypress)
            {
            case 8:
                break;
            default:
                break;
            }
            break;
        }
        default:
        {
            break;
        }
        }
        free(event);
    }

    shutdown();
}

void shutdown()
{
    xcb_flush(dpy);
    xcb_disconnect(dpy);
    if (xcb_connection_has_error(dpy))
        printf("fixation shut down.\n");
}

void mapWindow(xcb_connection_t *connection, xcb_map_request_event_t *mapevent)
{
    const uint32_t event_masks =
        XCB_EVENT_MASK_ENTER_WINDOW |
        XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_KEY_PRESS;
    const uint16_t value_masks =
        XCB_CONFIG_WINDOW_X |
        XCB_CONFIG_WINDOW_Y |
        XCB_CONFIG_WINDOW_WIDTH |
        XCB_CONFIG_WINDOW_HEIGHT;
    const uint32_t values[] = {
        200,
        200,
        ((screen->height_in_pixels) / 3),
        ((screen->width_in_pixels) / 3)};
    xcb_change_window_attributes(connection, mapevent->window, XCB_CW_EVENT_MASK, &event_masks);
    xcb_configure_window(connection, mapevent->window, value_masks, values);
    xcb_map_window(connection, mapevent->window);
    xcb_flush(dpy);
}

void raiseWindow(xcb_connection_t *connection, xcb_window_t window)
{
    const uint32_t values[] = {XCB_STACK_MODE_ABOVE};
    xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_STACK_MODE, values);
    xcb_set_input_focus(connection, XCB_INPUT_FOCUS_PARENT, window, XCB_CURRENT_TIME);
    xcb_flush(dpy);
    printf("raiseWindow ran.");
}

void grabMouse(xcb_connection_t *connection, xcb_window_t window)
{
    xcb_grab_pointer(connection, 1, window, mouseButton | mouseRelease | mouseMovement, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE, XCB_CURRENT_TIME);
    xcb_flush(dpy);
}

void grabKeyboard(xcb_connection_t *connection, xcb_window_t window)
{
    xcb_grab_keyboard(connection, 1, window, XCB_CURRENT_TIME, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    xcb_flush(dpy);
}

void grabInput(xcb_connection_t *connection, xcb_enter_notify_event_t *enterevent)
{
    grabMouse(connection, enterevent->event);
    grabKeyboard(connection, enterevent->event);
    xcb_flush(dpy);
    printf("grabInput ran.");
}