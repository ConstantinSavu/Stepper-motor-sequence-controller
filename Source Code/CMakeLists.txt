

add_library(buffer buffer.c)
add_library(keypad4x4 keypad4x4.c)
add_library(my_lcd_1602_i2c my_lcd_1602_i2c.c)
target_link_libraries(keypad4x4 pico_stdlib hardware_timer)
target_link_libraries(my_lcd_1602_i2c pico_stdlib hardware_i2c)



add_executable(proiect_MES proiect_MES.c)
# pull in common dependencies
target_link_libraries(proiect_MES buffer my_lcd_1602_i2c keypad4x4 hardware_i2c pico_stdlib)








# enable usb output, disable uart output
pico_enable_stdio_usb(proiect_MES 1)
pico_enable_stdio_uart(proiect_MES 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(proiect_MES)

# add url via pico_set_program_url
example_auto_set_url(proiect_MES)
