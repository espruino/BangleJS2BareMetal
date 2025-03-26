#include <stdint.h>
#include <stdbool.h>

typedef enum {
  TG_NONE,
  TG_SWIPE_DOWN,
  TG_SWIPE_UP,
  TG_SWIPE_LEFT,
  TG_SWIPE_RIGHT,
  TG_TAP
} touchGesture;

void touch_init(void (*_touch_cb)(int x, int y, bool touch), void (*_touch_gesture_cb)(int x, int y, touchGesture gesture));
