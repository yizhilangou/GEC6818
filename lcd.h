#ifndef __LCD_H__
#define __LCD_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include <stdlib.h>
#include "zimu.h"

#define BLACK 0x000000 
#define WHITE 0xFFFFFF
#define RED  0xFF0000
#define GREEN 0x00FF00
#define YELLOW 0xFFFF00
#define BLUE  0x0000FF

extern int lcdfd; //保存LCD屏幕文件的文件描述符
extern int *plcd; //保存LCD屏幕映射内存首地址

int lcd_init(void); //初始化LCD屏：打开和映射LCD屏幕文件
int lcd_uninit(void);//释放LCD屏：接触映射，关闭LCD屏幕文件
int lcd_draw_point(int x,int y,int color);	//画点函数，用来将坐标为(x,y)的像素点显示为color色
int lcd_draw_clear(void);//清屏函数，
int lcd_draw_FourLeafClover(void);//在屏幕上显示一个四叶草（取上下左右四个圆两两的交集）
int lcd_draw_Rainbow(void);//在屏幕上显示一个彩虹
int lcd_draw_circle(int x ,int y ,int r,int color);
void lcd_draw_word(unsigned char word16[],int w,int h,int x0,int y0,int color);
void lcd_draw_num(int num,int x0,int y0,int color);

#endif