#include <iostream>
#include <X11/Xlib.h>
#include <X11/X.h>

int main() {
    Display *dpy = XOpenDisplay(NULL);

    std::cout << XDisplayHeight(dpy, 0) << " x " << XDisplayWidth(dpy, 0);

    return 0;
}