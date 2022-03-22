#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../common/common.h"
#define DEBUG

#define COLOR_BACKGROUND	FB_COLOR(0xff,0xff,0xff)
#define COLOR_RED FB_COLOR(0xff, 0, 0)
static int touch_fd;
static void touch_event_cb(int fd);
static int BtnHit(int x, int y);
static void BtnCall();

int main(int argc, char *argv[])
{
	fb_init("/dev/graphics/fb0");
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
	fb_draw_rect(50,50,50,50,COLOR_RED);
	fb_update();
	srand(time(0));
	//打开多点触摸设备文件, 返回文件fd
	touch_fd = touch_init("/dev/input/event3");
	//添加任务, 当touch_fd文件可读时, 会自动调用touch_event_cb函数
	task_add_file(touch_fd, touch_event_cb);
	
	task_loop(); //进入任务循环
	return 0;
}

static void touch_event_cb(int fd)
{
	static int oldX[5] = { 0, 0, 0, 0, 0 };
	static int oldY[5] = { 0, 0, 0, 0, 0 };
	static int color[5];
	static int active[5] = { 0, 0, 0, 0, 0 };
	static int oldFinger = -1;
	int type,x,y,finger;
	type = touch_read(fd, &x,&y,&finger);
	switch(type){
	case TOUCH_PRESS:
		#ifdef DEBUG
		printf("TOUCH_PRESS：x=%d,y=%d,finger=%d\n",x,y,finger);
		#endif
		oldFinger = finger;
		oldX[finger] = x;
		oldY[finger] = y;
		color[finger] = FB_COLOR((rand() & 0xff), (rand() & 0xff), (rand() & 0xff));
		active[finger] = 1;
		break;
	case TOUCH_MOVE:
		#ifdef DEBUG
		printf("TOUCH_MOVE：x=%d,y=%d,finger=%d\n",x,y,finger);
		#endif
		if (active[finger])
		{
			fb_draw_line(oldX[finger], oldY[finger], x, y, color[finger]);
		}
		oldX[finger] = x;
		oldY[finger] = y;
		break;
	case TOUCH_RELEASE:
		if (oldFinger == finger && BtnHit(oldX[finger], oldY[finger]) && BtnHit(x, y)) { BtnCall(); }
		active[finger] = 0;
		#ifdef DEBUG
		printf("TOUCH_RELEASE：x=%d,y=%d,finger=%d\n",x,y,finger);
		#endif
		break;
	case TOUCH_ERROR:
		printf("close touch fd\n");
		close(fd);
		task_delete_file(fd);
		break;
	default:
		return;
	}
	fb_update();
	return;
}

static int BtnHit(int x, int y)
{
	if (x >= 50 && x <= 100 && y >= 50 && y <= 100) { return 1; }
	return 0;
}
static void BtnCall()
{
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
	fb_draw_rect(50,50,50,50,COLOR_RED);
}
