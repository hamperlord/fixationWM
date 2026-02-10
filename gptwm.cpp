// fixationwm.cpp
// single-file XCB window manager

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define MOD XCB_MOD_MASK_4 // Super key
#define WORKSPACES 4

struct Client
{
    xcb_window_t win;
    int x, y, w, h;
    bool floating;
    bool fullscreen;
    int workspace;
    Client *next;
};

xcb_connection_t *dpy;
xcb_screen_t *scr;
xcb_window_t root;

Client *clients = nullptr;
Client *focused = nullptr;

int current_ws = 0;
bool running = true;

Client *find_client(xcb_window_t w)
{
    for (Client *c = clients; c; c = c->next)
        if (c->win == w)
            return c;
    return nullptr;
}

void focus(Client *c)
{
    if (!c)
        return;
    focused = c;
    xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, c->win, XCB_CURRENT_TIME);
    uint32_t values[] = {XCB_STACK_MODE_ABOVE};
    xcb_configure_window(dpy, c->win, XCB_CONFIG_WINDOW_STACK_MODE, values);
}

void tile()
{
    int count = 0;
    for (Client *c = clients; c; c = c->next)
        if (!c->floating && !c->fullscreen && c->workspace == current_ws)
            count++;

    if (!count)
        return;

    int i = 0;
    for (Client *c = clients; c; c = c->next)
    {
        if (c->floating || c->fullscreen || c->workspace != current_ws)
            continue;

        int w = scr->width_in_pixels / count;
        int h = scr->height_in_pixels;

        uint32_t vals[4] = {
            (uint32_t)(i * w),
            0,
            (uint32_t)w,
            (uint32_t)h};

        uint16_t mask =
            XCB_CONFIG_WINDOW_X |
            XCB_CONFIG_WINDOW_Y |
            XCB_CONFIG_WINDOW_WIDTH |
            XCB_CONFIG_WINDOW_HEIGHT;

        xcb_configure_window(dpy, c->win, mask, vals);
        i++;
    }
}

void map_request(xcb_map_request_event_t *e)
{
    Client *c = find_client(e->window);
    if (!c)
    {
        c = (Client *)calloc(1, sizeof(Client));
        c->win = e->window;
        c->floating = false;
        c->fullscreen = false;
        c->workspace = current_ws;
        c->next = clients;
        clients = c;
    }

    uint32_t mask =
        XCB_EVENT_MASK_ENTER_WINDOW |
        XCB_EVENT_MASK_FOCUS_CHANGE |
        XCB_EVENT_MASK_PROPERTY_CHANGE;

    xcb_change_window_attributes(dpy, c->win, XCB_CW_EVENT_MASK, &mask);
    xcb_map_window(dpy, c->win);
    focus(c);
    tile();
}

void unmap(xcb_unmap_notify_event_t *e)
{
    Client **pp = &clients;
    while (*pp)
    {
        if ((*pp)->win == e->window)
        {
            Client *dead = *pp;
            *pp = dead->next;
            free(dead);
            break;
        }
        pp = &(*pp)->next;
    }
    focused = nullptr;
    tile();
}

void motion(xcb_motion_notify_event_t *e)
{
    Client *c = find_client(e->event);
    if (!c || !c->floating)
        return;

    uint32_t vals[2] = {(uint32_t)e->root_x, (uint32_t)e->root_y};
    uint16_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
    xcb_configure_window(dpy, c->win, mask, vals);
}

void button(xcb_button_press_event_t *e)
{
    Client *c = find_client(e->event);
    if (!c)
        return;

    focus(c);

    if (e->detail == 1)
    { // left click -> toggle floating
        c->floating = !c->floating;
        tile();
    }

    if (e->detail == 3)
    { // right click -> fullscreen
        c->fullscreen = !c->fullscreen;

        if (c->fullscreen)
        {
            uint32_t vals[4] = {
                0, 0,
                scr->width_in_pixels,
                scr->height_in_pixels};
            uint16_t mask =
                XCB_CONFIG_WINDOW_X |
                XCB_CONFIG_WINDOW_Y |
                XCB_CONFIG_WINDOW_WIDTH |
                XCB_CONFIG_WINDOW_HEIGHT;
            xcb_configure_window(dpy, c->win, mask, vals);
        }
        else
        {
            tile();
        }
    }
}

void keypress(xcb_key_press_event_t *e)
{
    // crude keycodes (typical US layout)
    if (!(e->state & MOD))
        return;

    switch (e->detail)
    {

    case 36: // Super+Enter -> tile
        tile();
        break;

    case 24: // Super+Q -> close focused
        if (focused)
            xcb_kill_client(dpy, focused->win);
        break;

    case 65: // Super+Space -> toggle floating
        if (focused)
        {
            focused->floating = !focused->floating;
            tile();
        }
        break;

    case 10:
    case 11:
    case 12:
    case 13:
    { // Super+1..4 workspace
        int ws = e->detail - 10;
        if (ws >= WORKSPACES)
            break;
        current_ws = ws;

        for (Client *c = clients; c; c = c->next)
        {
            if (c->workspace == ws)
                xcb_map_window(dpy, c->win);
            else
                xcb_unmap_window(dpy, c->win);
        }
        tile();
        break;
    }
    }
}

void setup()
{
    int screen_nbr;
    dpy = xcb_connect(nullptr, &screen_nbr);
    if (xcb_connection_has_error(dpy))
    {
        printf("cannot connect X\n");
        exit(1);
    }

    const xcb_setup_t *setup = xcb_get_setup(dpy);
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
    for (int i = 0; i < screen_nbr; i++)
        xcb_screen_next(&it);
    scr = it.data;
    root = scr->root;

    uint32_t mask =
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
        XCB_EVENT_MASK_ENTER_WINDOW |
        XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_KEY_PRESS;

    xcb_change_window_attributes(dpy, root, XCB_CW_EVENT_MASK, &mask);

    xcb_grab_key(dpy, 1, root, MOD, 36, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    xcb_grab_key(dpy, 1, root, MOD, 24, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    xcb_grab_key(dpy, 1, root, MOD, 65, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

    for (int i = 0; i < WORKSPACES; i++)
        xcb_grab_key(dpy, 1, root, MOD, 10 + i, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

    xcb_flush(dpy);
}

int main()
{
    setup();

    while (running)
    {
        xcb_generic_event_t *ev = xcb_wait_for_event(dpy);
        if (!ev)
            break;

        switch (ev->response_type & ~0x80)
        {
        case XCB_MAP_REQUEST:
            map_request((xcb_map_request_event_t *)ev);
            break;
        case XCB_UNMAP_NOTIFY:
            unmap((xcb_unmap_notify_event_t *)ev);
            break;
        case XCB_BUTTON_PRESS:
            button((xcb_button_press_event_t *)ev);
            break;
        case XCB_MOTION_NOTIFY:
            motion((xcb_motion_notify_event_t *)ev);
            break;
        case XCB_KEY_PRESS:
            keypress((xcb_key_press_event_t *)ev);
            break;
        }

        free(ev);
        xcb_flush(dpy);
    }

    xcb_disconnect(dpy);
    return 0;
}
