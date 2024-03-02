#ifndef KEYPAD4X4_H
#define KEYPAD4X4_H

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdio.h>

void keypad_init(uint columns[4], uint rows[4], char matrix_values[16]);

char keypad_get_key_from_column(uint column);

#endif