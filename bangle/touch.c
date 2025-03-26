#include "io.h"
#include "pins.h"
#include "touch.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_wdt.h"

void (*touch_cb)(int x, int y, bool touch) = NULL;
void (*touch_gesture_cb)(int x, int y, touchGesture gesture) = NULL;

// ============================================================= super hacky software i2c for touch
#define TOUCH_I2C_TIMEOUT 100000
/// write pin
void wr(int pin, bool state) {
  if (state) {
    nrf_gpio_pin_set(pin); nrf_gpio_cfg_output(pin);
    nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);
  } else {
    nrf_gpio_pin_clear(pin);
    nrf_gpio_cfg_output(pin);
  }
}
/// read pin
bool rd(int pin) {
  return nrf_gpio_pin_read(pin);
}
void dly() {
  volatile int i;
  for (i=0;i<10;i++);
}
bool touch_i2c_started = false;
/// start bit
void i2c_start() {
  if (touch_i2c_started) {
    // reset
    wr(TOUCH_PIN_SDA, 1);
    dly();
    wr(TOUCH_PIN_SCL, 1);
    int timeout = TOUCH_I2C_TIMEOUT;
    while (!rd(TOUCH_PIN_SCL) && --timeout); // clock stretch
    //if (!timeout) err("Timeout (start)");
    dly();
  }
  //if (!rd(TOUCH_PIN_SDA)) err("Arbitration (start)");
  wr(TOUCH_PIN_SDA, 0);
  dly();
  wr(TOUCH_PIN_SCL, 0);
  dly();
  touch_i2c_started = true;
}
/// stop bit
void i2c_stop() {
  wr(TOUCH_PIN_SDA, 0);
  dly();
  wr(TOUCH_PIN_SCL, 1);
  int timeout = TOUCH_I2C_TIMEOUT;
  while (!rd(TOUCH_PIN_SCL) && --timeout); // clock stretch
  //if (!timeout) err("Timeout (stop)");
  dly();
  wr(TOUCH_PIN_SDA, 1);
  dly();
  //if (!rd(TOUCH_PIN_SDA)) err("Arbitration (stop)");
  dly();
  touch_i2c_started = false;
}
/// write bit
void i2c_wr_bit(bool b) {
  wr(TOUCH_PIN_SDA, b);
  dly();
  wr(TOUCH_PIN_SCL, 1);
  dly();
  int timeout = TOUCH_I2C_TIMEOUT;
  while (!rd(TOUCH_PIN_SCL) && --timeout); // clock stretch
  //if (!timeout) err("Timeout (wr)");
  wr(TOUCH_PIN_SCL, 0);
  wr(TOUCH_PIN_SDA, 1); // stop forcing SDA (needed?)
}
/// read bit
bool i2c_rd_bit() {
  wr(TOUCH_PIN_SDA, 1); // stop forcing SDA
  dly();
  wr(TOUCH_PIN_SCL, 1); // stop forcing SDA
  int timeout = TOUCH_I2C_TIMEOUT;
  while (!rd(TOUCH_PIN_SCL) && --timeout); // clock stretch
  //if (!timeout) err("Timeout (rd)");
  dly();
  bool b = rd(TOUCH_PIN_SDA);
  wr(TOUCH_PIN_SCL, 0);
  return b;
}
/// write byte, true on ack, false on nack
bool i2c_wr(uint8_t data) {
  int i;
  for (i=0;i<8;i++) {
    i2c_wr_bit(data&128);
    data <<= 1;
  }
  return !i2c_rd_bit();
}
/// read byte
uint8_t i2c_rd(bool nack) {
  int i;
  int data = 0;
  for (i=0;i<8;i++)
    data = (data<<1) | (i2c_rd_bit()?1:0);
  i2c_wr_bit(nack);
  return data;
}
/// Write to I2C register
void touch_write(int addr, int data) {
  int iaddr = 0x15;
  i2c_start();
  i2c_wr(iaddr<<1);
  i2c_wr(addr);
  i2c_wr(data);
  i2c_stop();
  wr(TOUCH_PIN_SDA, 1);
  wr(TOUCH_PIN_SCL, 1);
}
/// Read from I2C register
void touch_read(int addr, int cnt, unsigned char *data) {
  int iaddr = 0x15;
  i2c_start();
  i2c_wr(iaddr<<1);
  i2c_wr(addr);
  i2c_start();
  i2c_wr(1|(iaddr<<1));
  for (int i=0;i<cnt;i++) {
    data[i] = i2c_rd(i==(cnt-1));
  }
  i2c_stop();
  wr(TOUCH_PIN_SDA, 1);
  wr(TOUCH_PIN_SCL, 1);
}
// =============================================================
void touch_irq_callback() {
  unsigned char buf[6];
  touch_read(1, 6, buf);
  int gesture = buf[0]; // gesture
  int touchPts = buf[1];
  int x = buf[3], y = buf[5];
  static int lastGesture = 0;
  // forward button pressed based on swipe
  if (gesture != lastGesture) {
    lastGesture = gesture;
    if (touch_gesture_cb) touch_gesture_cb(x,y,gesture);
  }
  static int lastX = 0, lastY = 0, lastT = 0;
  if (lastX!=x || lastY!=y || lastT!=touchPts) {
    lastX = x;
    lastY = y;
    lastT = touchPts;
    if (touch_cb) touch_cb(x,y,touchPts);
  }
}

void touch_init(void (*_touch_cb)(int x, int y, bool touch), void (*_touch_gesture_cb)(int x, int y, touchGesture gesture)) {
  touch_cb = _touch_cb;
  touch_gesture_cb = _touch_gesture_cb;

  nrf_gpio_cfg_input(TOUCH_PIN_IRQ, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_pin_set(TOUCH_PIN_RST);
  nrf_gpio_cfg_output(TOUCH_PIN_RST);
  // Set I2C default state to on
  nrf_gpio_pin_set(TOUCH_PIN_SDA);
  nrf_gpio_pin_set(TOUCH_PIN_SCL);
  nrf_gpio_cfg_output(TOUCH_PIN_SDA);
  nrf_gpio_cfg_output(TOUCH_PIN_SCL);

  io_watch_pin(TOUCH_PIN_IRQ, touch_irq_callback);
  nrf_gpio_cfg_input(TOUCH_PIN_IRQ, NRF_GPIO_PIN_PULLUP);
}

void touch_on() {
  // pulse reset to turn on
  nrf_gpio_pin_clear(TOUCH_PIN_RST);
  nrf_gpio_cfg_output(TOUCH_PIN_RST);
  nrf_delay_us(1000); // delay 1ms
  nrf_gpio_pin_set(TOUCH_PIN_RST);
  nrf_delay_us(1000);
}


void touch_off() {
  // write 0xE5,0x03 to sleep touch
  touch_write(0xE5,0x03);
}
