/**
 * Copyright (C) G2Labs Grzegorz Grzęda - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Grzegorz Grzęda <grzegorz.grzeda@g2labs.pl>, 03.2023
 */
#include "ssd1306.h"
#include <stdlib.h>
#include <string.h>

#define SSD1306_SCREEN_WIDTH (128)
#define SSD1306_SCREEN_HEIGHT (64)
#define SSD1306_SCREEN_COLOR (GENERIC_DISPLAY_COLOR_1B)

#define ROWS (SSD1306_SCREEN_WIDTH)
#define PAGES (SSD1306_SCREEN_HEIGHT / 8)

#define SSD1306_INITIALIZATION_CMDS                                          \
    {                                                                        \
        0xAE,     /* Set display OFF */                                      \
            0xD5, /* Set Display Clock Divide Ratio / OSC Frequency */       \
            0x80, /* Display Clock Divide Ratio / OSC Frequency */           \
            0xA8, /* Set Multiplex Ratio */                                  \
            0x3F, /* Multiplex Ratio for 128x64 (64-1) */                    \
            0xD3, /* Set Display Offset */                                   \
            0x00, /* Display Offset */                                       \
            0x40, /* Set Display Start Line */                               \
            0x8D, /* Set Charge Pump */                                      \
            0x14, /* Charge Pump (0x10 External, 0x14 Internal DC/DC) */     \
            0xA1, /* Set Segment Re-Map */                                   \
            0xC8, /* Set Com Output Scan Direction */                        \
            0xDA, /* Set COM Hardware Configuration */                       \
            0x12, /* COM Hardware Configuration */                           \
            0x81, /* Set Contrast */                                         \
            0xCF, /* Contrast */                                             \
            0xD9, /* Set Pre-Charge Period */                                \
            0xF1, /* Set Pre-Charge Period (0x22 External, 0xF1 Internal) */ \
            0xDB, /* Set VCOMH Deselect Level */                             \
            0x40, /* VCOMH Deselect Level */                                 \
            0xA4, /* Set all pixels OFF */                                   \
            0xA6, /* Set display not inverted */                             \
            0xAF, /* Set display On */                                       \
            0x20, /* Set address mode to horizontal */                       \
            0x00,                                                            \
    }
#define SSD1306_ADDRESSING_CMDS                      \
    {                                                \
        0x21,                       /* set column */ \
            0x00, (ROWS - 1), 0x22, /* set page */   \
            0x00, (PAGES - 1),                       \
    }
#define SSD1306_FILL_SECTION_BLACK (0)
#define SSD1306_FILL_SECTION_WHITE (0xFF)

#define SSD1306_BIT_FROM_Y(y) ((y) & 0x07)
#define SSD1306_GET_BYTE_FROM_BUFFER(buffer, x, y) ((buffer)[((((uint32_t)(y)) & 0xF8) << 4) + (x)])
#define SSD1306_SET_BYTE_TO_BUFFER(buffer, x, y, value) ((buffer)[((((uint32_t)(y)) & 0xF8) << 4) + (x)] = (value))

typedef struct ssd1306 {
    const generic_controller_t* controler;
    uint8_t buffer[ROWS * PAGES];
} ssd1306_t;

static void ssd1306_reset(generic_display_t* display) {
    if (!display || !display->context) {
        return;
    }
    ssd1306_t* ssd1306 = (ssd1306_t*)display->context;
    ssd1306->controler->reset(ssd1306->controler);
    char cmds[] = SSD1306_INITIALIZATION_CMDS;
    ssd1306->controler->send_command(ssd1306->controler, cmds, sizeof(cmds));
}

static void ssd1306_update(generic_display_t* display) {
    if (!display || !display->context) {
        return;
    }
    ssd1306_t* ssd1306 = (ssd1306_t*)display->context;
    uint8_t cmds[] = SSD1306_ADDRESSING_CMDS;
    ssd1306->controler->send_command(ssd1306->controler, cmds, sizeof(cmds));
    ssd1306->controler->send_data(ssd1306->controler, ssd1306->buffer, sizeof(ssd1306->buffer));
}

static void ssd1306_fill(generic_display_t* display, uint32_t color) {
    if (!display || !display->context) {
        return;
    }
    ssd1306_t* ssd1306 = (ssd1306_t*)display->context;
    memset(ssd1306->buffer, color ? SSD1306_FILL_SECTION_WHITE : SSD1306_FILL_SECTION_BLACK, sizeof(ssd1306->buffer));
}

static void ssd1306_pixel(generic_display_t* display, uint32_t x, uint32_t y, uint32_t color) {
    if (!display || !display->context || (x >= (ROWS)) || (y >= (PAGES * 8))) {
        return;
    }
    ssd1306_t* ssd1306 = (ssd1306_t*)display->context;
    uint8_t disp_y = SSD1306_BIT_FROM_Y(y);
    uint8_t byte = SSD1306_GET_BYTE_FROM_BUFFER(ssd1306->buffer, x, y);
    if (color) {
        byte |= (1 << disp_y);
    } else {
        byte &= (uint8_t)(~(1 << disp_y));
    }
    SSD1306_SET_BYTE_TO_BUFFER(ssd1306->buffer, x, y, byte);
}
static void ssd1306_line(generic_display_t* display,
                         uint32_t x0,
                         uint32_t y0,
                         uint32_t x1,
                         uint32_t y1,
                         uint32_t color) {
    if (!display || ((x0 == x1) && (y0 == y1))) {
        return;
    }
    uint32_t sorted_x0 = ((x1 > x0) ? x0 : x1);
    uint32_t sorted_x1 = ((x1 > x0) ? x1 : x0);
    uint32_t sorted_y0 = ((x1 > x0) ? y0 : y1);
    uint32_t sorted_y1 = ((x1 > x0) ? y1 : y0);

    float dx = (float)(sorted_x1 - sorted_x0);
    float dy = (float)(sorted_y1 - sorted_y0);
    if (dx > dy) {
        float a = dy / dx;
        for (uint32_t x = sorted_x0; x < sorted_x1; x++) {
            uint32_t y = sorted_y0 + (uint32_t)(a * (float)(x - sorted_x0));
            display->pixel(display, x, y, color);
        }
    } else {
        float a = dx / dy;
        for (uint32_t y = sorted_y0; y < sorted_y1; y++) {
            uint32_t x = sorted_x0 + (uint32_t)(a * (float)(y - sorted_y0));
            display->pixel(display, x, y, color);
        }
    }
}

static void ssd1306_destroy(generic_display_t* display) {
    if (!display) {
        return;
    }
    free(display->context);
    free(display);
}

generic_display_t* ssd1306_create(const generic_controller_t* controler) {
    if (!controler) {
        return NULL;
    }
    ssd1306_t* display = calloc(1, sizeof(*display));
    if (!display) {
        return NULL;
    }
    generic_display_t* gd = calloc(1, sizeof(*gd));
    if (!gd) {
        free(display);
        return NULL;
    }
    display->controler = controler;
    gd->context = display;
    gd->info.color_type = SSD1306_SCREEN_COLOR;
    gd->info.height = SSD1306_SCREEN_HEIGHT;
    gd->info.width = SSD1306_SCREEN_WIDTH;

    gd->reset = ssd1306_reset;
    gd->update = ssd1306_update;
    gd->fill = ssd1306_fill;
    gd->pixel = ssd1306_pixel;
    gd->line = ssd1306_line;
    gd->destroy = ssd1306_destroy;

    return gd;
}