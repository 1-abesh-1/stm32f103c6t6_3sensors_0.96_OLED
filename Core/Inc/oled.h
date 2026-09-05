#ifndef OLED_H
#define OLED_H

#include "main.h"
#include <stdint.h>

#define OLED_WIDTH   128
#define OLED_HEIGHT   64

#define OLED_ADDR    (0x3C << 1)

/* Initialize OLED */
void OLED_Init(void);

/* Clear display buffer */
void OLED_Clear(void);

/* Update physical OLED from buffer */
void OLED_Update(void);

/* Set text cursor */
void OLED_SetCursor(uint8_t x, uint8_t y);

/* Write one character */
void OLED_WriteChar(char c);

/* Write a string */
void OLED_WriteString(const char *str);

#endif