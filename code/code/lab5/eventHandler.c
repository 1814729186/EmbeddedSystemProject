#include "gobang.h"

int failureButtonEvent(ButtonEvent* button){
	printf("failureButtonEvent\n");
	//清空按钮队列
	clearButtonList();
	initScreen();
	//绘制开始信息界面
	//开始游戏，绘制开始界面，状态0
	initGame();
	//绘制准备界面,添加准备按钮
	initButton(312,200,400,200,GREEN,"开始游戏",30,startButtonHandler);
	printf("state to state_0\n");
	game_state = state_0;
	return 0;
}

/*开始按钮事件处理*/
int startButtonHandler(ButtonEvent* button){
	printf("startButtonHandler\n");
	/*游戏状态:state_0-> state_01 or state_02 -> state_1*/
	/*修改游戏状态*/
	if(game_state==state_0){
		printf("state_0 to state_01\n");
		game_state=state_01;
		clearButtonList();
		//发送准备完成信息 1
		bluetooth_send(1,0,0,bluetooth_fd);
		/*清空按钮队列，显示准备完成字样*/
		initScreen();
		fb_draw_text(button->x_min,button->y_min,"等待另一玩家完成准备",20,COLOR_TEXT);
		fb_update();
	}
	else if(game_state==state_02){
		printf("state_02 to state_1\n");
		game_state=state_1;
		clearButtonList();
		//绘制棋子选择界面，并添加两个按钮
		drawSelect();
		//send message
		bluetooth_send(1,0,0,bluetooth_fd);
	}
	else{error("什么鬼啊，开始就出错？");}
	return 0;
}

//黑棋选择按钮响应
int selectBlack(ButtonEvent* button){
	/*颜色选中*/
	game_color = 1;
	draw_color = BLACK;
	dis_draw_color = WHITE;
	//选择黑棋通讯 3
	bluetooth_send(3,0,0,bluetooth_fd);
	//状态修改
	//state1 -> state2
	if(game_state == state_1){
		game_state = state_2;
		//黑棋按钮修改为"已选择"，清空按钮队列
		initButton(312,200,200,200,BLACK,"已选择",30,NULL);
		fb_update();
		printf("debug##");
		clearButtonList();
		printf("debug##");
	}
	//state5 -> state6
	else if(game_state == state_5){
		game_state = state_6;
		//绘制棋盘
		initScreen();
		drawChessBoard();
	}
	return 0;
}
//白棋选择按钮响应
int selectWhite(ButtonEvent* button){
	/*颜色选中*/
	game_color = 2;
	draw_color = WHITE;
	dis_draw_color = BLACK;
	//选择白棋通讯
	bluetooth_send(2,0,0,bluetooth_fd);
	//状态修改
	if(game_state==state_1){
		//白棋按钮修改为已选择，清空队列按钮
		printf("state_1 to state_3\n");
		game_state = state_3;
		//黑棋按钮修改为"已选择"，清空按钮队列
		clearButtonList();
		initScreen();
		fb_draw_text(312,200,"selected WHITE",20,COLOR_TEXT);
		fb_update();
		printf("selected white\n");
	}else if(game_state==state_4){
		game_state = state_7;
		//绘制棋盘
		clearButtonList();
		initScreen();
		drawChessBoard();
	}
	return 0;
}





