#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../common/common.h"
#define DEBUG

#define COLOR_BACKGROUND	FB_COLOR(0xff,0xff,0xff)
#define COLOR_RED FB_COLOR(0xff, 0, 0)
#define COLOR_CYAN FB_COLOR(0x10, 0x2b, 0x6a)
#define COLOR_BROWN FB_COLOR(0x84, 0x39, 0x0)

static int touch_fd;
static void touch_event_cb(int fd);
static int BtnHit(int x, int y);
static void BtnCall();
/* PENs */
static void dot_r3(int x, int y, int color);
static void dot_r4(int x, int y, int color);
static void square_a2(int x, int y, int color);
static void fancy_1(int x, int y, int color);
static void dot_rx(int x, int y, int color);

/* PEN Collection */
static void(*pen[5])(int x, int y, int color) = {
	dot_r3, dot_r4, square_a2, fancy_1, dot_rx
};
static int rx = 8;
enum { PEN_R3, PEN_R4, PEN_A2, PEN_X1, PEN_RX };
enum { BTN_CLS = 1, BTN_CHG };

int main(int argc, char *argv[])
{
	fb_init("/dev/graphics/fb0");
	font_init("/data/local/font.ttc");
	
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
	fb_draw_rect(40,40,70,5,COLOR_RED);
	fb_draw_rect(35,40,5,50,COLOR_RED);

	fb_image *img;
	img = fb_read_png_image("/data/local/clear.png");
	fb_draw_image(50, 150, img, 0);
	
	char buf[3];
	if (rx >= 10) { buf[0] = rx / 10 + '0'; buf[1] = rx % 10 + '0'; buf[2] = '\0'; }
	else { buf[0] = rx + '0'; buf[1] = '\0'; }
	fb_draw_text(50, 99, buf, 64, COLOR_BROWN);
	fb_update();

	fb_free_image(img);

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
		break;
	case TOUCH_MOVE:
		#ifdef DEBUG
		printf("TOUCH_MOVE：x=%d,y=%d,finger=%d\n",x,y,finger);
		#endif
		/*
		int distance = abs(oldX[finger] - x) + abs(oldY[finger] - y);
		if (distance > SCREEN_WIDTH) { rx = 0; }
		else { rx = 12 - distance / (SCREEN_WIDTH >> 6); }
		*/
		fb_draw_line(oldX[finger], oldY[finger], x, y, color[finger]);
		oldX[finger] = x;
		oldY[finger] = y;
		break;
	case TOUCH_RELEASE:
		if (oldFinger == finger) { BtnCall(BtnHit(x, y)); }
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
	if (x >= 50 && x <= 100 && y >= 50 && y <= 100) { return BTN_CHG; }
	if (x >= 50 && x <= 100 && y >= 150 && y <= 200) { return BTN_CLS; }
	return 0;
}
static void BtnCall(int cmd)
{
	char buf[32];
	fb_image *img;
	switch(cmd)
	{
	case BTN_CLS:
		fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
		fb_draw_rect(40,40,70,5,COLOR_RED);
		fb_draw_rect(35,40,5,50,COLOR_RED);

		img = fb_read_png_image("/data/local/clear.png");
		fb_draw_image(50, 150, img, 0);

		if (rx >= 10) { buf[0] = rx / 10 + '0'; buf[1] = rx % 10 + '0'; buf[2] = '\0'; }
		else { buf[0] = rx + '0'; buf[1] = '\0'; }
		fb_draw_text(50, 99, buf, 64, COLOR_BROWN);
		fb_update();

		fb_free_image(img);
		break;
	case BTN_CHG:
		rx >= 66 ? (rx = 1) : (++rx);
		if (rx >= 10) { buf[0] = rx / 10 + '0'; buf[1] = rx % 10 + '0'; buf[2] = '\0'; }
		else { buf[0] = rx + '0'; buf[1] = '\0'; }
		fb_draw_rect(40,40,75,60,COLOR_BACKGROUND);
		fb_draw_rect(40,40,70,5,COLOR_RED);
		fb_draw_rect(35,40,5,50,COLOR_RED);		
		fb_draw_text(50, 99, buf, 64, COLOR_BROWN);
		fb_update();
		break;
	default: break;
	}
}

/* PEN for draw line */
static void dot_r3(int x, int y, int color)
{
	int tx = 0, ty = 3, d = 3 - 2 * 3;

	while (tx < ty)
	{
		// 小于 45 度横线
		fb_draw_line(x - ty, y - tx, x + ty, y - tx, color);
		if (tx != 0) // 防止水平线重复绘制
		{
			fb_draw_line(x - ty, y + tx, x + ty, y + tx, color);
		}

		if (d < 0)   // 取上面的点
		{
			d += 4 * tx + 6;
		}
		else        // 取下面的点
		{
			// 大于 45 度横线
			fb_draw_line(x - tx, y - ty, x + tx, y - ty, color);
			fb_draw_line(x - tx, y + ty, x + tx, y + ty, color);
			d += 4 * (tx - ty) + 10;
			ty--;
		}
		tx++;
	}
	if (tx == ty)    // 45 度横线
	{
		fb_draw_line(x - ty, y - tx, x + ty, y - tx, color);
		fb_draw_line(x - ty, y + tx, x + ty, y + tx, color);
	}
}

static void dot_r4(int x, int y, int color)
{
	int tx = 0, ty = 4, d = 3 - 2 * 4;

	while (tx < ty)
	{
		// 小于 45 度横线
		fb_draw_line(x - ty, y - tx, x + ty, y - tx, color);
		if (tx != 0) // 防止水平线重复绘制
		{
			fb_draw_line(x - ty, y + tx, x + ty, y + tx, color);
		}

		if (d < 0)   // 取上面的点
		{
			d += 4 * tx + 6;
		}
		else        // 取下面的点
		{
			// 大于 45 度横线
			fb_draw_line(x - tx, y - ty, x + tx, y - ty, color);
			fb_draw_line(x - tx, y + ty, x + tx, y + ty, color);
			d += 4 * (tx - ty) + 10;
			ty--;
		}
		tx++;
	}
	if (tx == ty)    // 45 度横线
	{
		fb_draw_line(x - ty, y - tx, x + ty, y - tx, color);
		fb_draw_line(x - ty, y + tx, x + ty, y + tx, color);
	}
}

static void square_a2(int x, int y, int color)
{
	int i, j;
	for (i = 0; i < 2; ++i)
	{
		for (j = 0; j < 2; ++j)
		{
			fb_draw_pixel(x + i, y + j, color);
		}
	}
}

static void fancy_1(int x, int y, int color)
{
	fb_draw_pixel(x - 1, y - 1, color);
	fb_draw_pixel(x, y, color);
	fb_draw_pixel(x + 1, y + 1, color);
	fb_draw_pixel(x + 1, y + 2, color);
}

static void dot_rx(int x, int y, int color)
{
	int tx = 0, ty = rx, d = 3 - 2 * rx;

	while (tx < ty)
	{
		// 小于 45 度横线
		fb_draw_line(x - ty, y - tx, x + ty, y - tx, color);
		if (tx != 0) // 防止水平线重复绘制
		{
			fb_draw_line(x - ty, y + tx, x + ty, y + tx, color);
		}

		if (d < 0)   // 取上面的点
		{
			d += 4 * tx + 6;
		}
		else        // 取下面的点
		{
			// 大于 45 度横线
			fb_draw_line(x - tx, y - ty, x + tx, y - ty, color);
			fb_draw_line(x - tx, y + ty, x + tx, y + ty, color);
			d += 4 * (tx - ty) + 10;
			ty--;
		}
		tx++;
	}
	if (tx == ty)    // 45 度横线
	{
		fb_draw_line(x - ty, y - tx, x + ty, y - tx, color);
		fb_draw_line(x - ty, y + tx, x + ty, y + tx, color);
	}
}
