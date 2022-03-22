#include "gobang.h"

int main(int argc,char**argv){
	//初始化设备
	printf("debug0\n");
	fb_init("/dev/graphics/fb0");
	font_init("/data/local/font.ttc");
	printf("debug1\n");
	touch_fd = touch_init("/dev/input/event3");
	task_add_file(touch_fd, touch_event_cb);
	printf("init\n");
	bluetooth_fd = bluetooth_tty_init("/dev/rfcomm0");
	if(bluetooth_fd == -1) return 0;
	task_add_file(bluetooth_fd, bluetooth_tty_event_cb);
	//开始游戏，绘制开始界面，状态0
	initGame();
	//list init
	memset(&buttonList,0,sizeof(ButtonQueue));
	//绘制准备界面,添加准备按钮
	initButton(312,200,400,200,GREEN,"开始游戏",30,startButtonHandler);
	
	printf("debug2\n");
	task_loop();

}

