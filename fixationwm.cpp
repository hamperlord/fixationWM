#include <cstdint>
#include <cstdlib>
#include <xcb/xproto.h>

// macros
// #define

// function declarations
static void eventloop();
static void setup();
static void spawn();

// structs
struct window {
  char name[256];
  int x, y, w, h;
  int oldx, oldy, oldw, oldh;
};
struct screen {
  int pxw, pxh;
  int mmw, mmh;
};

int main(int argc, char *argv[]) { return EXIT_SUCCESS; }
