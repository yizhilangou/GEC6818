#include "lcd.h"

int lcdfd = 0;   //保存LCD屏幕文件的文件描述符
int * plcd = NULL;   //像素点
int lcd_init(void)
{
    lcdfd = open("/dev/fb0",O_RDWR);
	if(-1 == lcdfd)
	{
		perror("open lcd error");
		return -1;
	}
    /* 将LCD屏映射到内存空间 =》Frame Buffer 帧缓冲设备*/
	plcd = (int*)mmap(NULL,//系统自动选择映射地址
						800*480*32/8,	//映射空间大小（单位 字节），屏幕800*480个像素点，每个像素点32位（32/8个字节）
						PROT_READ|PROT_WRITE,//具备读写权限
						MAP_SHARED,//共享映射，对映射内存空间的操作会被同步到文件
						lcdfd,//指定映射的文件
						0); //从文件头开始映射
	if(MAP_FAILED == plcd)
	{
		perror("mmap lcd error");
		close(lcdfd);
		return -1;
	}

}

int lcd_uninit(void)
{
    /* 收尾：解除映射，关闭屏幕文件 */
	munmap(plcd,800*480*32/8);
	close(lcdfd);
}

int lcd_draw_point(int x,int y,int color)
{
    *(plcd+800*y+x) = color;
}

//清屏
int lcd_draw_clear(void)
{
    int red = RED;
    /* 通过操作映射内存地址，来实现对文件内容 的访问: plcd+y*800+x 就是（x,y）像素点的坐标 */
    for(int y=0;y<480;y++)
    {
         for(int x=0;x<800;x++)
        {
    		*(plcd+y*800+x) = WHITE;	//让（x，y）像素点显示为 红色
        }
    }
}

//画圆
int lcd_draw_circle(int x ,int y ,int r,int color)
{
    int i, j;
    for (j = 0; j < 480; j++)
    {
        for (i = 0; i < 800; i++)
        {
            if ((i-x)*(i-x) + (j-y)*(j-y) <= r*r)
            {
                lcd_draw_point(i, j, color);
            }
        }
    }
}

//画四叶草
int lcd_draw_FourLeafClover(void)
{
    //r = 100   
    //x1:100 y1:100 x2:100+100*sqrt(2) y2: 100 
    //x3:100 y3:100+100*sqrt(2) x4:100+100*sqrt(2) y4:100*sqrt(2)
    int red = RED;
    int green = GREEN;
    int yellow = YELLOW;
    int white = WHITE;
    int blue = BLUE;
    int i, j;
    for (j = 0; j < 480; j++)
    {
        for (i = 0; i < 800; i++)
        {
   
            if (((i-100)*(i-100) + (j-100)*(j-100) <= 10000) &&
                ((i-100-100*sqrt(2))*(i-100-100*sqrt(2)) + (j-100)*(j-100) <= 10000) 
            )
            {
                lcd_draw_point(i, j, red);
            }
            else if(((i-100)*(i-100) + (j-100)*(j-100) <= 10000) &&
                    ((i-100)*(i-100) + (j-100-100*sqrt(2))*(j-100-100*sqrt(2)) <= 10000)   
            )
            {
                lcd_draw_point(i, j, green);
            }
             else if(((i-100-100*sqrt(2))*(i-100-100*sqrt(2)) + (j-100)*(j-100) <= 10000)  &&
                    ((i-100-100*sqrt(2))*(i-100-100*sqrt(2)) + (j-100-100*sqrt(2))*(j-100-100*sqrt(2)) <= 10000)   
            )
            {
                lcd_draw_point(i, j, yellow);
            }
            else if(  ((i-100)*(i-100) + (j-100-100*sqrt(2))*(j-100-100*sqrt(2)) <= 10000)  &&
                ((i-100-100*sqrt(2))*(i-100-100*sqrt(2)) + (j-100-100*sqrt(2))*(j-100-100*sqrt(2)) <= 10000)  
            )
            {
                lcd_draw_point(i, j, white);
            }
            else{
                lcd_draw_point(i, j, blue);
            }

        }
    }
}

// 画彩虹
int lcd_draw_Rainbow(void)
{
    int red = RED;
    int grenn = GREEN;
    int yellow = YELLOW;
    int blue = BLUE;
    lcd_draw_circle(400,0,400,red);
    lcd_draw_circle(400,0,300,grenn);
    lcd_draw_circle(400,0,200,yellow);
    lcd_draw_circle(400,0,100,blue);
}

void lcd_draw_word(unsigned char word16[],int w,int h,int x0,int y0,int color)
{
	int i,j;
	
	for(j=0;j<w*h/8;j++) //遍历一个字节对应的所有 十六进制码
    {
        /*
        	j的变化对显示位置的影响
        */
        for(i=7;i>=0;i--) //遍历一个十六进制码的每一个bit
        {
            if(word16[j]>>i & 1)//判断该bit位是否需要上色
            {
                //x+(7-i) 将十六进制码 与其对应的像素点的横坐标确定下来
                //+(j%5)*8 每一行有40个像素点，由5个字节来表示  (此处的5已经用 w/8替换了，w/8表示一行需要几个字节)
                lcd_draw_point(x0+(7-i)+(j%(w/8))*8,y0+j/(w/8),color); //将对应像素点显示一个相应的颜色

            }
        }
    }
}

void lcd_draw_num(int num,int x0,int y0,int color)
{
	int i = 0;
	int j = 0;
	int value = num;
	unsigned char ch[16] = {0};

	/* 将整数的每个数位都取出来,存入到数组 */
	while(abs(num))
	{
		ch[i++] = abs(num)%10;
		num /= 10;
		
	}
	if(value < 0)
	{
		ch[i++] = 10;
	}
	
	
	//由于存入数组的数据与整数本身的数位是倒过来的
	for(j=i-1;j>=0;j--)
	{
		//digit[ch[i]] 就是以取出来的 数位为下表，找到该数对应的 字模
		// printf("%d\n",ch[j]);
		lcd_draw_word(digit[ch[j]],24,35,x0+(i-1-j)*24,y0,color);
	}
}
