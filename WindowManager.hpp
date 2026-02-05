#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>

#define Shift XCB_MOD_MASK_SHIFT
#define CapsLock XCB_MOD_MASK_LOCK
#define Control XCB_MOD_MASK_CONTROL
#define Super XCB_MOD_MASK_4

uint64_t InputMasks[] = {
    Shift,
    CapsLock,
    Control,
    Super
};

void getKeypress(xcb_connection_t* connection) {
    xcb_generic_event_t *event;
    xcb_expose_event_t *expose;
    while ( (event = xcb_poll_for_event (connection)) ) {
        
    }
}

void KillFixation(xcb_connection_t* connection) {
    xcb_disconnect(connection);
    std::cout << "Connection ended.";
}