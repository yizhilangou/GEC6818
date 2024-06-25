#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "lcd.h"
#include "bmp.h"
#define TOUCH_PATH "/dev/input/event0"
enum LED_STATE{LED_OFF,LED_ON};

int xy_read();
int beep_ctl( int on_off );


#endif