#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../common/common.h"

/*颜色信息定义*/
#define COLOR_BACKGROUND	FB_COLOR(0xaf,0xaf,0xaf)
#define COLOR_TEXT		FB_COLOR(0x0,0x0,0x0)
#define COLOR_LINE 		FB_COLOR(0x0,0x0,0x0)
#define RED		FB_COLOR(255,0,0)
#define ORANGE	FB_COLOR(255,165,0)
#define YELLOW	FB_COLOR(255,255,0)
#define GREEN	FB_COLOR(0,255,0)
#define CYAN	FB_COLOR(0,127,255)
#define BLUE	FB_COLOR(0,0,255)
#define PURPLE	FB_COLOR(139,0,255)
#define WHITE   FB_COLOR(255,255,255)
#define BLACK   FB_COLOR(0,0,0)

/**
* 五子棋联机通信游戏头文件支持
*/

/*硬件信息*/
static int touch_fd;
static int bluetooth_fd;

/*游戏状态*/
enum State{
	state_0, /*开始阶段*/
	state_01, /*完成准备等待对手机准备*/
	state_02, /*对手机完成准备，等待本机准备*/
	state_1,	/*棋色选择*/
	state_2,
	state_3,
	state_4,
	state_5,
	state_6,
	state_7,
	state_8,
	state_9
};
/*棋盘*/
// 0--none 1--black 2--write
extern int board[][16];
/*绘制函数*/
extern int circleFunc[];
/*游戏状态*/
extern enum State game_state;
/*本机棋子颜色 :  1 - black ;2 - white */
extern int game_color;
extern int draw_color;
extern int dis_draw_color;//对手棋子颜色

/*初始化函数：蓝牙设备初始化*/
static int bluetooth_tty_init(const char *dev)
{
	int fd = open(dev, O_RDWR|O_NOCTTY|O_NONBLOCK); /*非阻塞模式*/
	if(fd < 0){
		printf("bluetooth_tty_init open %s error(%d): %s\n", dev, errno, strerror(errno));
		return -1;
	}
	return fd;
}
/*按钮事件结构体与按钮事件响应函数指针*/
typedef struct ButtonEvent{
	int enable; //使能
	//坐标信息
	int x_min;
	int y_min;
	int x_max;
	int y_max;
	int (*p)(struct ButtonEvent* button); //函数指针
}ButtonEvent;

/*按钮队列，最多仅允许存在10个按钮*/
typedef struct ButtonQueue{
	int tail;
	ButtonEvent *buttons[10];
}ButtonQueue;
/*屏幕中存在的按钮队列*/
extern ButtonQueue buttonList;

/*蓝牙通信结构体*/
typedef struct Message{
	int type;
	int x;
	int y;
}Message;

void clearButtonList();
int addButtonToList(ButtonEvent * button);
int initButton(int x,int y,int w,int h,int color,char str[],int fontSize,int (*fp)(struct ButtonEvent* button));
void buttonListClicked(int x,int y);
void initGame();
void initScreen();
void drawChessBoard();
void initChessboard();
void cauculateCircleFunc();
void drawPiece(int x,int y,int color);
int checkBoard(int x,int y);
void touch_event_cb(int fd);
void bluetooth_tty_event_cb(int fd);
void bluetooth_send(int ctype,int cx,int cy,int fd);
int failureButtonEvent(ButtonEvent* button);
int startButtonHandler(ButtonEvent* button);
void drawSelect();
int selectBlack(ButtonEvent* button);
int selectWhite(ButtonEvent* button);
void error(char str[]);



