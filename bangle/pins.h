#define BTN1_PININDEX 17/* D17 */

#define LCD_WIDTH 176
#define LCD_HEIGHT 176
#define LCD_BPP 4
#define LCD_BL 8/* D8 */
#define LCD_DISP 7/* D7 */
#define LCD_EXTCOMIN 6/* D6 */
#define LCD_SPI_CS 5/* D5 */
#define LCD_SPI_MOSI 27/* D27 */
#define LCD_SPI_SCK 26/* D26 */
#define LCD_ROWHEADER 2
#define LCD_STRIDE (LCD_ROWHEADER+((LCD_WIDTH*LCD_BPP+7)>>3)) // data in required BPP, plus 2 bytes LCD command

#define VIBRATE_PIN 19/* D19 */

#define HEARTRATE_PIN_SDA 24/* D24 */
#define HEARTRATE_PIN_SCL 32/* D32 */
#define HEARTRATE_PIN_EN 21/* D21 */
#define HEARTRATE_PIN_INT 22/* D22 */
#define HEARTRATE_ADDR 51

#define BAT_PIN_CHARGING 23/* D23 */
#define BAT_PIN_VOLTAGE 3/* D3 */

#define GPS_PIN_EN 29/* D29 */
#define GPS_PIN_RX 30/* D30 */
#define GPS_PIN_TX 31/* D31 */

// KX023
#define ACCEL_ADDR 30
#define ACCEL_PIN_SDA 38/* D38 */
#define ACCEL_PIN_SCL 37/* D37 */

#define MAG_ADDR 12
#define MAG_PIN_SDA 44/* D44 */
#define MAG_PIN_SCL 45/* D45 */

#define PRESSURE_ADDR 118
#define PRESSURE_PIN_SDA 47/* D47 */
#define PRESSURE_PIN_SCL 2/* D2 */

// CST816D
#define TOUCH_ADDR 21
#define TOUCH_PIN_SDA 33/* D33 */
#define TOUCH_PIN_SCL 34/* D34 */
#define TOUCH_PIN_RST 35/* D35 */
#define TOUCH_PIN_IRQ 36/* D36 */


#define SPIFLASH_PAGESIZE 4096
#define SPIFLASH_LENGTH 8388608
#define SPIFLASH_PIN_CS 14/* D14 */
#define SPIFLASH_PIN_SCK 16/* D16 */
#define SPIFLASH_PIN_MOSI 15/* D15 */
#define SPIFLASH_PIN_MISO 13/* D13 */