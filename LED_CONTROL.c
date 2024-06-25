#include "LED_CONTROL.h"
#include <time.h>

int touch_fd;

int st=0;

int beep_ctl( int on_off )
{
    //打开文件
    int fd = open("/sys/kernel/gec_ctrl/led_all",O_RDWR);

    if(fd == -1)
    {
        printf("sorry , open beep failed\n");
        return -1;
    }
    write( fd, &on_off, 4 );
    //关闭文件
    close(fd);
}

int xy_read(time_t *t)
{
    //1.打开触摸屏  
    touch_fd = open(TOUCH_PATH,O_RDONLY);//只读打开
    if(touch_fd<0)
    {
        perror("open fail");
        return 0;
    }

    int x_read = -1;
    int y_read = -1;

    //定义一个结构体叫ev
    struct input_event ev;

    //一直读取触摸板信息
    while(1)
    {
        //读取屏幕
        read(touch_fd, &ev, sizeof(struct input_event));//第三个参数ev
        
        if(ev.type == EV_KEY && ev.code == BTN_TOUCH)
        {
            if(ev.value == 1)
            {
                printf("down\n");
                if(x_read>=900&&x_read<=1000&&y_read>=500&&y_read<=600)
                {
                    time(t);
                    printf(ctime(t));
                    if(st==0)
                    {
                        printf("lED ON");
                        beep_ctl(LED_ON);
                        st=1;
                    }else{
                        printf("LED OFF");
                        beep_ctl(LED_OFF);
                        st=0;
                    }
                }
            }
            else
            {
                printf("up\n");
            }
        }

        if(ev.type == EV_ABS)   //触摸事件 
        {
            if(ev.code == ABS_X)
            {
                x_read = ev.value;   //x轴
            }
            if(ev.code == ABS_Y)		//y轴
            {
                y_read = ev.value;
            }
            printf("(%d,%d)\n", x_read, y_read);
            
        }    
    }
}