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
	state_0 = 0, /*开始阶段*/
	state_01 = 10, /*完成准备等待对手机准备*/
	state_02 = 20, /*对手机完成准备，等待本机准备*/
	state_1 = 1,	/*棋色选择*/
	state_2 = 2,
	state_3 = 3,
	state_4 = 4,
	state_5 = 5,
	state_6 = 6,
	state_7 = 7,
	state_8 = 8,
	state_9 = 9
};
/*棋盘*/
// 0--none 1--black 2--write
extern int board[16][16];
/*绘制函数*/
extern int circleFunc[18];
/*游戏状态*/
extern enum State game_state;
/*本机棋子颜色 :  1 - black ;2 - white */
extern int game_color;
extern int draw_color;
extern int dis_draw_color;//对手棋子颜色

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


/*初始化函数：蓝牙设备初始化*/
int bluetooth_tty_init(const char *dev);

/*初始化按钮队列*/
void initButtonList();

/*清空按钮队列*/
void clearButtonList();

/*添加按钮到队尾*/
int addButtonToList(ButtonEvent * button);

/*移除队尾按钮*/
int deleteButtonFromList();

/*按钮生成*/
int initButton(int x,int y,int w,int h,int color,char str[],int fontSize,int (*fp)(struct ButtonEvent* button));

/*按钮触发检测，调用触发的函数*/
void buttonListClicked(int x,int y);


/*总游戏初始化*/
void initGame();

/*初始化显示窗口*/
void initScreen();

/*初始化棋盘绘制*/
void drawChessBoard();

/*棋盘绘制*/
void initChessboard();

/*绘制节点函数值计算*/
void cauculateCircleFunc();

/*落子绘制，x,y为棋盘坐标*/
void drawPiece(int x,int y,int color);

/*棋盘检测*/
int checkBoard(int x,int y);

/*触摸板事件控制模块，根据触摸位置和状态信息作出反应*/
void touch_event_cb(int fd);

/*五种信息类型：
	1 : 准备信息 1
	2 : 选择白棋 2
	3 : 选择黑棋 3
	4 : 落子坐标 4
	5 : 胜利信息 5
*/
/*蓝牙信息接收模块*/
void bluetooth_tty_event_cb(int fd);

/*蓝牙信息发送*/
void bluetooth_send(int ctype,int cx,int cy,int fd);

int failureButtonEvent(ButtonEvent* button);

/*开始按钮事件处理*/
int startButtonHandler(ButtonEvent* button);

void drawSelect();

//黑棋选择按钮响应
int selectBlack(ButtonEvent* button);
//白棋选择按钮响应
int selectWhite(ButtonEvent* button);

void error(char str[]);



