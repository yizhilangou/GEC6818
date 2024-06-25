#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include "uart.h"
#include "lcd.h"
#include "bmp.h"
#include "LED_CONTROL.h"

time_t start,end;

typedef struct {
    int fog;
    int LUX;
    int T;
    int P;
    int HUM;
    int H;
    pthread_mutex_t lock;
} SensorData;

SensorData sensorData = {0, 0, 0, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER};

void* mq2_getFog(void *arg)
{
    int mq2_fd = 0;
    ssize_t rsize = 0;
    unsigned char cmd[9] = { 0XFF,0X01,0X86,0X00,0X00,0X00,0X00,0X00,0X79};
    unsigned char buf[25] = {0};
    
    mq2_fd = init_serial("/dev/ttySAC2",9600);

    while(1)
    {
        write(mq2_fd, cmd, 9); 
        rsize = read(mq2_fd, buf, 9);
        if(rsize == 9)
        {
            int fog = ((unsigned int)buf[2] << 8) | buf[3];
            printf("Fog:%d\n", fog);

            // Update the shared data
            pthread_mutex_lock(&sensorData.lock);
            sensorData.fog = fog;
            pthread_mutex_unlock(&sensorData.lock);
        }
        sleep(1);
    }

    close(mq2_fd);
    return NULL;
}

void printGy39Data(unsigned char *recvdata, int n)
{
    int LUX, T, P, HUM, H;
    LUX = (recvdata[4]<<24 | recvdata[5]<<16 | recvdata[6]<<8 | recvdata[7]) / 100; 
    T = (recvdata[13]<<8 | recvdata[14]) / 100; 
    P = (recvdata[15]<<24 | recvdata[16]<<16 | recvdata[17]<<8 | recvdata[18]) / 100; 
    HUM = (recvdata[19]<<8 | recvdata[20]) / 100; 
    H = recvdata[21]<<8 | recvdata[22];
    printf("LUX:%d T:%d P:%d HUM:%d H:%d\n", LUX, T, P, HUM, H);

    // 更新数据
    pthread_mutex_lock(&sensorData.lock);
    sensorData.LUX = LUX;
    sensorData.T = T;
    sensorData.P = P;
    sensorData.HUM = HUM;
    sensorData.H = H;
    pthread_mutex_unlock(&sensorData.lock);
}

void* Gy39GetData(void *arg)
{
    int fd = init_serial("/dev/ttySAC1",9600);
    unsigned char cmd[3] = {0xA5,0x83,0x28};
    
    write(fd, cmd, 3);
    usleep(500000);
    unsigned char recvdata[24] = {0};
    int gy39_i = 0;
    int timeout = 0;
    while(1)
    {
        int r = read(fd, recvdata + gy39_i, 1);
        if(0 == r)
        {
            timeout++;
            usleep(1000);
            if(timeout > 1000) // Timeout
            {
                printf("time out\n");
                timeout = 0;
                break;
            }
            continue;
        }
        timeout = 0;
        gy39_i++;
        switch(gy39_i)
        {
            case 1: if(recvdata[0] != 0x5a) { gy39_i = 0; } break;
            case 2: if(recvdata[1] != 0x5a) { gy39_i = 0; } break;
            case 3: if(recvdata[2] != 0x15) { gy39_i = 0; } break;
            case 4: if(recvdata[3] != 0x04) { gy39_i = 0; } break;
            case 24: printGy39Data(recvdata, 24); gy39_i = 0; break; // Data received
            default:
                if(gy39_i > 24 || gy39_i < 0)
                {
                    gy39_i = 0;
                    sleep(1);
                }
                break;
        }       
    }
    return NULL;
}

void* updateDisplay(void *arg)
{
    lcd_init(); // 初始化 LCD

    while(1)
    {
        //同步机制：使用互斥锁来保护线程之间对共享数据的访问。
        pthread_mutex_lock(&sensorData.lock);// 锁
        // 读取共享数据
        int fog = sensorData.fog;
        int LUX = sensorData.LUX;
        int T = sensorData.T;
        int P = sensorData.P;
        int HUM = sensorData.HUM;
        int H = sensorData.H;

        pthread_mutex_unlock(&sensorData.lock);// 解锁
        
        if(start==0)
        {
            if(LUX>150)
            {
                beep_ctl(LED_OFF);
            }
            else{
                beep_ctl(LED_ON);
            }
        }
        else{
            time(&end);
            double second = difftime(end, start);
            if(second>180)
            {
                start=0;
            }
        }
		

        // 更新屏幕显示数据
		lcd_draw_bmp(0,0,"/test/background.bmp");
		lcd_draw_num(LUX,180,55,RED);
		lcd_draw_num(T,180,105,RED);
		lcd_draw_num(fog,180,165,RED);
		lcd_draw_num(P,180,215,RED);
		lcd_draw_num(HUM,180,270,RED);
		lcd_draw_num(H,180,330,RED);

        sleep(1); 
    }

    return NULL;
}

void* LED_CONTROL(void* arg)
{
    lcd_init(); // 初始化 LCD
    xy_read(&start);
}

int creat_pthread0() // gy39
{
    pthread_t pid;
    int ret = pthread_create(&pid, NULL, Gy39GetData, NULL);
    if (ret != 0)
    {
        printf("pthread_create error\n");
        return -1;
    }
    pthread_detach(pid);
    return 0;
}

int creat_pthread1() // mq2
{
    pthread_t pid;
    int ret = pthread_create(&pid, NULL, mq2_getFog, NULL);
    if (ret != 0)
    {
        printf("pthread_create error\n");
        return -1;
    }
    pthread_detach(pid);
    return 0;
}

int creat_pthread2() // 屏幕数据更新
{
    pthread_t pid;
    int ret = pthread_create(&pid, NULL, updateDisplay, NULL);
    if (ret != 0)
    {
        printf("pthread_create error\n");
        return -1;
    }
    pthread_detach(pid);
    return 0;
}

int create_pthread3() // 触摸屏
{
    pthread_t pid;
    int ret = pthread_create(&pid, NULL, LED_CONTROL, NULL);
    if (ret != 0)
    {
        printf("pthread_create error\n");
        return -1;
    }
    pthread_detach(pid);
    return 0;
}


int main()
{
    creat_pthread0();
    creat_pthread1();
    creat_pthread2();
    create_pthread3();
    while(1)
    {
        sleep(1);
    }
    return 0;
}




