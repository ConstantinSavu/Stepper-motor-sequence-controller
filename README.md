# Stepper Motor Sequence Controller

## Project Description
This project involves programming a Raspberry Pi Pico to control a stepper motor, executing a specific sequence of rotations[^2^][2]. Input is provided through a 4x4 matrix keypad, and the sequence is displayed on an LCD.

## Hardware Description
Key components used in this project include:
- Raspberry Pi Pico H
- 42HB34F08AB Stepper Motor
- A4988 Stepper Motor Driver
- 1602 LCD with I2C interface
- 4x4 button matrix

## Software Description
The software is divided into four parts:
1. Button Matrix Software
2. LCD Display Software
3. Stepper Driver Software
4. Overall Control Software

Each part is responsible for managing its respective hardware component, with the overall control software integrating all parts.

## Problems and Solutions
Challenges encountered during the project were addressed using serial port printing and minicom on the host machine for debugging.

## Conclusions
The project met all requirements, providing valuable experience in using the Raspberry Pi Pico for data acquisition and device control, and enhancing problem-solving skills.
