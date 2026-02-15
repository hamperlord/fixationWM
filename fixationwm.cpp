#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#define MOD XCB_MOD_MASK_4

xcb_connection_t *dpy;
xcb_screen_t *screen;
xcb_generic_event_t *event;

void handlemouse(xcb_button_press_event_t *event);

void setup() {
  int screennum;
  dpy = xcb_connect(NULL, &screennum);
  if (xcb_connection_has_error(dpy))
    exit(1);

  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(dpy));

  for (int i = 0; i < screennum; i++)
    xcb_screen_next(&iter);

  screen = iter.data;

  uint32_t eventmask = {
      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_RESIZE_REDIRECT |
      XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_POINTER_MOTION |
      XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS};
  xcb_change_window_attributes(dpy, screen->root, XCB_CW_EVENT_MASK,
                               &eventmask);
}

int main() {
  setup();

  for (;;) {
    xcb_flush(dpy);

    event = xcb_wait_for_event(dpy);
    switch (event->response_type & ~0x80) {
    case XCB_MAP_REQUEST: {

      xcb_window_t map_request = ((xcb_map_request_event_t *)event)->window;
      xcb_map_window(dpy, map_request);
      int default_size[] = {screen->width_in_pixels / 3,
                            screen->height_in_pixels / 3};
      xcb_configure_window(dpy, map_request,
                           XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           default_size);
      break;
    }
    case XCB_BUTTON_PRESS:
      handlemouse((xcb_button_press_event_t *)event);
    }
  }

  return 0;
}

void handlemouse(xcb_button_press_event_t *event) {
  xcb_get_geometry_reply_t *geom;
  geom = xcb_get_geometry_reply(dpy, xcb_get_geometry(dpy, event->child), NULL);
  if (!geom)
    return;

  if (event->detail == 1) {
    xcb_window_t window = event->child;
    xcb_grab_pointer(dpy, 0, screen->root,
                     XCB_EVENT_MASK_POINTER_MOTION |
                         XCB_EVENT_MASK_BUTTON_RELEASE,
                     XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE,
                     XCB_NONE, XCB_CURRENT_TIME);

  } else {
    // nothing for now
  }

  int done = 0;
  while (!done) {
    xcb_flush(dpy);
    xcb_generic_event_t *event = xcb_wait_for_event(dpy);

    switch (event->response_type & ~0x80) {
    case XCB_MOTION_NOTIFY: {
      xcb_motion_notify_event_t *motion = ((xcb_motion_notify_event_t *)event);
      int oldx, oldy;
      oldx = motion->root_x;
      oldy = motion->root_y;

      printf("%d\n", oldx);
      printf("%d\n", oldy);
      break;
      free(event);
    }
    case XCB_BUTTON_RELEASE: {
      done = 1;
      break;
    }
    }
  }
  xcb_ungrab_pointer(dpy, XCB_CURRENT_TIME);
}
