/**
 * Copyright (C) G2Labs Grzegorz Grzęda - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Grzegorz Grzęda <grzegorz.grzeda@g2labs.pl>, 03.2023
 */
#ifndef SSD1306_H
#define SSD1306_H

#include <stddef.h>
#include <stdint.h>
#include "generic-controller.h"
#include "generic-display.h"

generic_display_t* ssd1306_create(const generic_controller_t* controler);

#endif  // SSD1306_H