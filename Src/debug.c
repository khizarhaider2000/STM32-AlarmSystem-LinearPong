/*
 * debug.c
 *
 *  Created on: Sep 25, 2025
 *      Author: shaid029
 */
#include "stm32l552xx.h"
#include <stdio.h>

int __io_putchar(int ch) {
    ITM_SendChar(ch);   // macro provided by CMSIS
    return ch;
}
