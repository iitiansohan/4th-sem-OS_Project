#include <stdint.h>
#include <stdbool.h>

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define KEYBOARD_PORT 0x60

#define RED_BG_GREEN_TEXT 0x4A
#define BLUE_BG_WHITE_TEXT 0x1F

__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,
    0x0,
    -(0x1BADB002 + 0x0)
};

static uint16_t* terminal_buffer;
static uint8_t terminal_row;
static uint8_t terminal_column;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

void terminal_initialize(uint8_t color) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_buffer = (uint16_t*) VGA_ADDRESS;
    for (uint8_t y = 0; y < VGA_HEIGHT; y++) {
        for (uint8_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = (uint16_t)(' ' | (color << 8));
        }
    }
}

void clear_screen(uint8_t color) {
    for (uint8_t y = 0; y < VGA_HEIGHT; y++) {
        for (uint8_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = (uint16_t)(' ' | (color << 8));
        }
    }
}

void terminal_putchar(char c, uint8_t x, uint8_t y, uint8_t color) {
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        terminal_buffer[y * VGA_WIDTH + x] = (uint16_t)(c | (color << 8));
    }
}

void terminal_writestring(const char* str, uint8_t x, uint8_t y, uint8_t color) {
    for (uint8_t i = 0; str[i] != '\0' && x + i < VGA_WIDTH; i++) {
        terminal_putchar(str[i], x + i, y, color);
    }
}

char read_key() {
    if ((inb(0x64) & 0x01) == 0) return 0;
    uint8_t scancode = inb(KEYBOARD_PORT);
    switch (scancode) {
        case 0x12: return 'e';
        case 0x1E: return 'a';
        case 0x23: return 'h';
        case 0x32: return 'm';
        case 0x1F: return 's';
        default: return 0;
    }
}

char wait_for_key() {
    char key = 0;
    while (!(key = read_key()));
    return key;
}

uint8_t hours = 0, minutes = 0, seconds = 0;
bool alarm_set = false;
uint8_t alarm_hours = 0, alarm_minutes = 0, alarm_seconds = 0;

void delay() {
    for (volatile int i = 0; i < 100000000; i++);
}

void delay_seconds_with_countdown(int seconds) {
    for (int i = seconds; i > 0; i--) {
        char countdown_str[20] = "Starting in ";
        countdown_str[12] = '0' + (i / 10);
        countdown_str[13] = '0' + (i % 10);
        countdown_str[14] = '.';
        countdown_str[15] = '.';
        countdown_str[16] = '.';
        countdown_str[17] = '\0';

        terminal_writestring(countdown_str, 28, 20, RED_BG_GREEN_TEXT);
        for (volatile int j = 0; j < 20000000; j++);
    }
}

// BCD to Binary conversion
uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd & 0x0F);
}

uint8_t read_cmos(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

void read_rtc_time() {
    hours = bcd_to_binary(read_cmos(0x04));
    minutes = bcd_to_binary(read_cmos(0x02));
    seconds = bcd_to_binary(read_cmos(0x00));
}

void display_time() {
    read_rtc_time(); // Read hardware RTC

    char time_str[9] = {
        '0' + (hours / 10), '0' + (hours % 10), ':',
        '0' + (minutes / 10), '0' + (minutes % 10), ':',
        '0' + (seconds / 10), '0' + (seconds % 10), '\0'
    };
    terminal_writestring("Current Time:", 0, 0, BLUE_BG_WHITE_TEXT);
    terminal_writestring(time_str, 0, 1, BLUE_BG_WHITE_TEXT);

    if (alarm_set && hours == alarm_hours && minutes == alarm_minutes && seconds == alarm_seconds) {
        terminal_writestring("⏰ Alarm! Wake up! ⏰", 0, 2, BLUE_BG_WHITE_TEXT);
        alarm_set = false;
    }
}

void adjust_time() {
    terminal_writestring("Time Adjust: h=+Hour, m=+Min, s=+Sec, e=Exit", 0, 4, BLUE_BG_WHITE_TEXT);
    while (1) {
        char key = wait_for_key();
        if (key == 'e') break;
        if (key == 'h') hours = (hours + 1) % 24;
        else if (key == 'm') minutes = (minutes + 1) % 60;
        else if (key == 's') seconds = (seconds + 1) % 60;

        char time_str[9] = {
            '0' + (hours / 10), '0' + (hours % 10), ':',
            '0' + (minutes / 10), '0' + (minutes % 10), ':',
            '0' + (seconds / 10), '0' + (seconds % 10), '\0'
        };
        terminal_writestring("Current Time:", 0, 0, BLUE_BG_WHITE_TEXT);
        terminal_writestring(time_str, 0, 1, BLUE_BG_WHITE_TEXT);
    }
    terminal_writestring("                                        ", 0, 4, BLUE_BG_WHITE_TEXT);
}

void set_alarm() {
    terminal_writestring("Set Alarm: h=+Hour, m=+Min, s=+Sec, e=Exit", 0, 5, BLUE_BG_WHITE_TEXT);
    uint8_t h = 0, m = 0, s = 0;
    while (1) {
        char key = wait_for_key();
        if (key == 'e') break;
        if (key == 'h') h = (h + 1) % 24;
        else if (key == 'm') m = (m + 1) % 60;
        else if (key == 's') s = (s + 1) % 60;

        char alarm_str[9] = {
            '0' + (h / 10), '0' + (h % 10), ':',
            '0' + (m / 10), '0' + (m % 10), ':',
            '0' + (s / 10), '0' + (s % 10), '\0'
        };
        terminal_writestring("Alarm Time:", 0, 6, BLUE_BG_WHITE_TEXT);
        terminal_writestring(alarm_str, 0, 7, BLUE_BG_WHITE_TEXT);
    }
    alarm_hours = h;
    alarm_minutes = m;
    alarm_seconds = s;
    alarm_set = true;
    terminal_writestring("Alarm Set!", 0, 8, BLUE_BG_WHITE_TEXT);
    delay();
    for (int i = 5; i <= 8; i++) {
        terminal_writestring("                                        ", 0, i, BLUE_BG_WHITE_TEXT);
    }
}

void draw_welcome_screen() {
    terminal_writestring(" __        __   _                            _          _____ _____  _____ ", 5, 3, RED_BG_GREEN_TEXT);
    terminal_writestring(" \\ \\      / /__| | ___ ___  _ __ ___   ___  | |_ ___   |_   _| ____||_   _|", 5, 4, RED_BG_GREEN_TEXT);
    terminal_writestring("  \\ \\ /\\ / / _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\ | __/ _ \\    | | |  _|     | |  ", 5, 5, RED_BG_GREEN_TEXT);
    terminal_writestring("   \\ V  V /  __/ | (_| (_) | | | | | |  __/ | || (_) |   | | | |___    | |  ", 5, 6, RED_BG_GREEN_TEXT);
    terminal_writestring("    \\_/\\_/ \\___|_|\\___\\___/|_| |_| |_|\\___|  \\__\\___/    |_| |_____|   |_|  ", 5, 7, RED_BG_GREEN_TEXT);
}

void kernel_main(void) {
    terminal_initialize(RED_BG_GREEN_TEXT);
    draw_welcome_screen();
    delay_seconds_with_countdown(100);
    clear_screen(BLUE_BG_WHITE_TEXT);

    terminal_writestring("Press 'e' to adjust time, 'a' to set alarm", 0, 3, BLUE_BG_WHITE_TEXT);

    while (1) {
        display_time();
        delay();
        char key = read_key();
        if (key == 'e') adjust_time();
        else if (key == 'a') set_alarm();
    }
}
