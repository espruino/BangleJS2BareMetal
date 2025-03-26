#include "io.h"
#include "pins.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_wdt.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpiote.h"
#include "nrfx_gpiote.h"

void io_init(void (*btn_callback)()) {
  // pin change driver
  nrf_drv_gpiote_init();
  // actual inputs
  nrf_gpio_cfg_input(BTN1_PININDEX, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(BAT_PIN_CHARGING, NRF_GPIO_PIN_PULLUP);
  if (btn_callback)
    io_watch_pin(BTN1_PININDEX, btn_callback);
}


bool io_is_button_pressed() {
  return !nrf_gpio_pin_read(BTN1_PININDEX);
}

bool io_is_charging() {
  return !nrf_gpio_pin_read(BAT_PIN_CHARGING);
}

void io_kick_watchdog() {
  NRF_WDT->RR[0] = 0x6E524635;
}

void io_watch_pin(int pin, void (*callback)()) {
  nrf_drv_gpiote_in_config_t gpiote_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(1/* high accuracy */);
  gpiote_config.is_watcher = true; // stop this resetting the input state
  nrf_drv_gpiote_in_init(pin, &gpiote_config, callback);
  nrf_drv_gpiote_in_event_enable(pin, true);
}