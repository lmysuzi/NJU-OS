#include <game.h>

#define KEY_W 30
#define KEY_A 43
#define KEY_S 44
#define KEY_D 45
#define KEYNAME(key) \
  [AM_KEY_##key] = #key,
static const char *key_names[] = {
  AM_KEYS(KEYNAME)
};

extern Ball ball;


int print_key() {
  AM_INPUT_KEYBRD_T event = { .keycode = AM_KEY_NONE };
  ioe_read(AM_INPUT_KEYBRD, &event);
  if (event.keycode != AM_KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
    return event.keycode;
  }
  return 0;
}
