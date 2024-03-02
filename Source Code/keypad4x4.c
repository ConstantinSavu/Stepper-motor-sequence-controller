#include "keypad4x4.h"


#define GPIO_INPUT false
#define GPIO_OUTPUT true

uint _columns[4];
uint _rows[4];
char _matrix_values[16];
uint8_t column_dict[28] = {-1};
uint32_t columns_mask = 0;
uint32_t rows_mask = 0;

void keypad_init(uint columns[4], uint rows[4], char matrix_values[16]) {

    for (int i = 0; i < 16; i++) {
        _matrix_values[i] = matrix_values[i];
    }

    for (int i = 0; i < 4; i++) {

        _columns[i] = columns[i];
        _rows[i] = rows[i];

        gpio_init(_columns[i]);
        gpio_init(_rows[i]);

        gpio_set_dir(_columns[i], GPIO_INPUT);
        gpio_set_dir(_rows[i], GPIO_OUTPUT);

        gpio_pull_down(_columns[i]);

        gpio_put(_rows[i], true);

        column_dict[_columns[i]] = i;

        columns_mask |= 1 << _columns[i];
        rows_mask |= 1 << _rows[i];

    }
}

char keypad_get_key_from_column(uint column) {
    
    bool status;
    uint8_t row_index = -1;

    for (int i = 0; i < 4; i++) {

        gpio_put(_rows[i], false);
        busy_wait_us(1000);
        status = gpio_get(column);
        gpio_put(_rows[i], true);

        //Inverted logic
        if(!status){
            row_index = i;
            break;
        }
    }

    if(row_index == -1){
        return 0;
    }
    
    return (char)_matrix_values[row_index * 4 + column_dict[column]];
}

