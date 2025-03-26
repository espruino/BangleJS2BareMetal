#include <stdint.h>
#include <stdbool.h>

void io_init(void (*btn_callback)());
bool io_is_button_pressed();
bool io_is_charging();
void io_kick_watchdog();

void io_watch_pin(int pin, void (*callback)());