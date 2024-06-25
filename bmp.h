#ifndef __BMP_H__
#define __BMP_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

#define BMP_INFO_SIZE 54    //DIB &BMPMAP

typedef struct bmpinfo
{
    char* bmpname;          
    int size;               
    int width;             
    int height;             
    short bpp;      //color depth         
    char* data;     //address of the first data address       
}bmpinfo;

bmpinfo get_bmp_info(char* bmpname);
void lcd_draw_bmp( int x, int y,char* bmpname);

#endif