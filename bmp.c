#include "bmp.h"
#include "lcd.h"


bmpinfo get_bmp_info(char* bmpname)
{
    int fd = open(bmpname,O_RDONLY);
    if(fd == -1)
    {
        printf("open bmp error :%s \n",bmpname);
        exit(-1);
    }
    bmpinfo bmp;
	bmp.bmpname = bmpname;
   
	lseek(fd, 0x02, SEEK_SET);
	read(fd, &bmp.size, 4);


	lseek(fd, 0x12, SEEK_SET);
	read(fd, &bmp.width, 4);

    lseek(fd, 0x16, SEEK_SET);
	read(fd, &bmp.height, 4);

	lseek(fd, 0x1c, SEEK_SET);
	read(fd, &bmp.bpp, 2);

	bmp.data = (char*)malloc(bmp.size - BMP_INFO_SIZE);
	lseek(fd, 0x36, SEEK_SET);
	read(fd, bmp.data, bmp.size-BMP_INFO_SIZE);
    close(fd);
    return bmp;
}

void lcd_draw_bmp( int x, int y,char* bmpname)
{
    bmpinfo bmp = get_bmp_info(bmpname);
    //Number of valid bytes
    int valid_types = abs(bmp.width) * (bmp.bpp / 8);
    //Padding bytes
    int pad_types = (valid_types % 4 == 0) ? 0 : (4 - (valid_types % 4));

    int color;
	char* p = bmp.data; 
    //printf("get bmpinfo success\n");
	for (int h = 0; h < abs(bmp.height); h++) 
	{
		for (int w = 0; w < abs(bmp.width); w++) 
		{
			char b = *(p++);
			char g = *(p++);
			char r = *(p++);
            char a = (bmp.bpp == 24) ? 0 : *(p++);
            color = a << 24 | r << 16 | g << 8 | b;
			int m =x+(bmp.width>0 ? w:(bmp.width-1-w));
			int n =y+(bmp.height<0 ? h:(bmp.height-1-h));
			lcd_draw_point(m, n, color);
		}
		p += pad_types;
    }
    free(bmp.data);
}
