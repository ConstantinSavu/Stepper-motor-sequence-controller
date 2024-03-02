#include "pico/stdlib.h"
#include "buffer.h"

#include "hardware/timer.h"
#include "keypad4x4.h"
#include "string.h"

#include <stdio.h>
#include "hardware/i2c.h"
#include "my_lcd_1602_i2c.h"

#define GPIO_INPUT false
#define GPIO_OUTPUT true


#define EXECUTE_KEY         'A'
#define CHANGE_DELAY_KEY    'B'
#define CLEAR_KEY           'C'
#define DELETE_KEY          'D'
#define LEFT_KEY            'L'
#define RIGHT_KEY           'R'

#define EXECUTE_BIT 0
#define DELETE_BIT  1
#define CHANGED_BIT 2

#define PIN_SDA 4
#define PIN_SCL 5

#define PIN_STEP 17
#define PIN_DIR 16

#define DELAY_BETWEEN_STEPS_US 2000

uint columns[4] = {7, 6, 3, 2}; 
uint rows[4] = {15, 14, 11, 10};

volatile uint32_t time = 0;
const int delay_time_in_ms = 150;
circular_buffer *cb;
const int cb_capacity = 1024;
int movent_delay_in_ms = 1000;

char matrix[16] = {
    '1', '2' , '3', 'A',
    '4', '5' , '6', 'B',
    '7', '8' , '9', 'C',
    'L', '0' , 'R', 'D'
};

inline int32_t bit_set(int32_t number, int32_t n) {
    return number | ((int32_t)1 << n);
}

inline int32_t bit_clear(int32_t number, int32_t n) {
    return number & ~((int32_t)1 << n);
}

inline int32_t bit_toggle(int32_t number, int32_t n) {
    return number ^ ((int32_t)1 << n);
}

inline bool bit_get(int32_t number, int32_t n) {
    return (number >> n) & (int32_t)1;
}

inline int32_t bit_set_to(int32_t number, int32_t n, bool x) {
    return (number & ~((int32_t)1 << n)) | ((int32_t)x << n);
}

void get_key_callback(uint gpio, uint32_t events){
    
    char key;
    int res;

    if ((to_ms_since_boot(get_absolute_time())-time) > delay_time_in_ms) {
        
        time = to_ms_since_boot(get_absolute_time());
        key = keypad_get_key_from_column(gpio);
        
        if(key == 0){
            return;
        }
        
        if(cb == NULL){
            res = cb_init(cb, cb_capacity, sizeof(char));

            if(res == -1){
                printf("Unable to allocate buffer\n");
            }
        }

        cb_push_back(cb, &key);
        
    }
    

}

void init_stepper_pins(){

    gpio_init(PIN_STEP);
    gpio_init(PIN_DIR);

    gpio_set_dir(PIN_STEP, GPIO_OUTPUT);
    gpio_set_dir(PIN_DIR, GPIO_OUTPUT);

}

void move_stepper(unsigned int steps, unsigned int delay_between_steps_us, unsigned int delay_before_complete_ms){

    sleep_ms(delay_before_complete_ms);
    for(int i = 0; i < steps; i++)
	{
		gpio_put(PIN_STEP, 1);
		sleep_us(delay_between_steps_us);
		gpio_put(PIN_STEP, 0);
		sleep_us(delay_between_steps_us);
	}
}

uint32_t transfer_in_local_buffer(char* local_buffer){
    
    char key;
    
    int res;

    uint32_t return_value = 0;

    while(cb -> count != 0){
        res = cb_pop_front(cb, &key);
        if(res == -1){
            printf("Error in popping\n");
            return -1;
        }

        if(key == CLEAR_KEY){
            local_buffer[0] = '\0';
            return_value = bit_set(return_value, DELETE_BIT);

        }
        else if(key == DELETE_KEY){
            
            if(strlen(local_buffer) - 1 < 0){
                continue;
            }

            local_buffer[strlen(local_buffer) - 1] = '\0';
            return_value = bit_set(return_value, DELETE_BIT);
        }
        else{

            char addkey[2];
            
            addkey[0] = key;
            addkey[1] = '\0';

            
            strcat(local_buffer, addkey);
            return_value = bit_set(return_value, CHANGED_BIT);

            if(key == EXECUTE_KEY){
                return_value = bit_set(return_value, EXECUTE_BIT);
            }
        }
    }

    return return_value;
}

void analyze_buffer(char* buffer){
    char *local_copy;
    local_copy = malloc(sizeof(char) * (strlen(buffer) + 3));

    strcpy(local_copy, buffer);
    int index = 0;
    int length = strlen(local_copy);
    int amount = 0;

    char current_command[100];

    for(index = 0; index < length; index++)
    {
        if( local_copy[index] != LEFT_KEY   &&
            local_copy[index] != RIGHT_KEY  &&
            local_copy[index] != CHANGE_DELAY_KEY 
        ){
            continue;
        }
        

        if(sscanf(local_copy + index + 1, "%d", &amount) != 1){

            printf("Invalid command, could not scan argument!\n");
            continue;

        }

        if(local_copy[index] == LEFT_KEY){
            gpio_put(PIN_DIR, false);
            printf("Moving left with: %d\n", amount);
            sprintf(current_command, "L: %d\0", amount);
            
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_string("EXECUTING");

            lcd_set_cursor(1, 0);
            lcd_string(current_command);
            
            move_stepper(amount, DELAY_BETWEEN_STEPS_US, movent_delay_in_ms);
        }
        else if(local_copy[index] == RIGHT_KEY){
            gpio_put(PIN_DIR, true);
            printf("Moving right with: %d\n", amount);
            sprintf(current_command, "R: %d\0", amount);

            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_string("EXECUTING");

            lcd_set_cursor(1, 0);
            lcd_string(current_command);

            move_stepper(amount, DELAY_BETWEEN_STEPS_US, movent_delay_in_ms);

        }
        else if(local_copy[index] == CHANGE_DELAY_KEY){
            printf("Change delay to: %d ms\n", amount);
            movent_delay_in_ms = amount;
        }
        
    }

    free(local_copy);
}




int startup(){
    int res;
    time = to_ms_since_boot(get_absolute_time());
    stdio_init_all();

    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    lcd_init();
    lcd_clear();

    keypad_init(columns, rows, matrix);
    printf("Starting\n");

    init_stepper_pins();

    cb = malloc(sizeof(circular_buffer));

    if(cb == NULL){
        printf("Unable to allocate buffer\n");
        return 0;
    }

    res = cb_init(cb, cb_capacity, sizeof(char));
    
    if(res == -1){
        printf("Unable to allocate buffer\n");
        return 0;
    }

    for (int i = 0; i < 4; i++) {
        gpio_set_irq_enabled_with_callback(columns[i], GPIO_IRQ_EDGE_RISE , true, &get_key_callback);
    }

    return 1;

}


int main() {
    
    int res;

    res = startup();
    
    if(!res){
        printf("Startup failed\n");
        return -1;
    }

    char local_buffer[1024] = {'\0'};
    char command[1024] = {'\0'};
    
    uint32_t return_value = 0;
    char * ptr;

    
    
    while(1){
        
        
        return_value = transfer_in_local_buffer(local_buffer);
        
        
        if(return_value != 0){
            printf("%s\n", local_buffer);
            lcd_clear();
            lcd_set_cursor(0, 0);
            int offset = 0;
            if(strlen(local_buffer) > MAX_CHARS){
                offset = strlen(local_buffer) - MAX_CHARS;
            }
            lcd_string(local_buffer + offset);
        }
        ptr = strchr(local_buffer, EXECUTE_KEY);
        while(ptr){
            
            strncpy(command, local_buffer, ptr - local_buffer);
            command[ptr - local_buffer] = '\0';
            printf("EXECUTING %s\n", command);
            
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_string("EXECUTING");

            lcd_set_cursor(1, 0);
            lcd_string(command);
            
            analyze_buffer(command);

            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_string("FINISHED EXECUTING");

            lcd_set_cursor(1, 0);
            lcd_string(command);

            int remaining_length = strlen(ptr + 1);

            if(remaining_length == 0){
                local_buffer[0] = '\0';
                local_buffer[1] = '\0';   
            }
            else{
                strcpy(local_buffer, ptr+1);
                local_buffer[remaining_length - 1] = '\0';
            }
            
            ptr = strchr(local_buffer, EXECUTE_KEY);
        }
        
        
        
        
    }

    cb_free(cb);
    
}
