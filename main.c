// SSD1306 Display Example - Display an image on SSD1306 OLED
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "atari_mono2_page.h"  // Atari image in page format

// I2C Configuration
#define I2C_INSTANCE i2c0
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define I2C_FREQ 400000  // 400kHz

// SSD1306 Configuration
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_ADDRESS 0x3C  // Common address, may be 0x3D on some displays

// Global display variable like working project
ssd1306_t display;

int main() {
    // Initialize I2C - exactly like working project
    i2c_init(I2C_INSTANCE, I2C_FREQ);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Initialize SSD1306 display - exactly like working project
    ssd1306_init(&display, SSD1306_WIDTH, SSD1306_HEIGHT, SSD1306_ADDRESS, I2C_INSTANCE);
    
    // Clear the display
    ssd1306_clear(&display);
    
    // Display the Atari image (120x64) - already inverted, no need to invert display
    // Image is 120 pixels wide, display is 128 pixels wide, so center it with 4px offset
    ssd1306_draw_page_image(&display, atari_mono2_page, ATARI_MONO2_PAGE_WIDTH, ATARI_MONO2_PAGE_HEIGHT, 4, 0);
    
    ssd1306_show(&display);

    while (true) {
        tight_loop_contents();
    }
}
