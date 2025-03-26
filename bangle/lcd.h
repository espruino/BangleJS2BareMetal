#include <stdint.h>
#include <stdbool.h>

uint8_t lcd_get_pixel(int x, int y);
void lcd_set_pixel(int x, int y, uint8_t col);
void lcd_flip();
void lcd_toggle(); // call this once a second to avoid (temporary) burn-in on the LCD
void lcd_set_backlight(bool on);
void lcd_init();

void lcd_clear(uint8_t col);
void lcd_print_char(char ch); // prints one character, doesn't flip
void lcd_print(char *ch); // prints text, doesn't flip
void lcd_println(char *ch); // prints a line and flips screen

void lcd_fill_rect(int x1, int y1, int x2, int y2, uint8_t col); // fill a rectangle, doesn't flip