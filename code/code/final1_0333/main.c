#include "gobang.h"

ButtonQueue buttonList;
/*棋盘*/
// 0--none 1--black 2--write
int board[16][16];
/*绘制函数*/
int circleFunc[18];
/*游戏状态*/
enum State game_state;
/*本机棋子颜色 :  1 - black ;2 - white */
int game_color;
int draw_color;
int dis_draw_color;//对手棋子颜色

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
	return 0;
}


/*清空按钮队列*/
void clearButtonList(){
	int i = 0;
	for(i = 0;i < 10;i ++){
		if(buttonList.buttons[i]!=NULL){
			free(buttonList.buttons[i]);
			buttonList.buttons[i] = NULL;
		}
	}
	buttonList.tail=0;
}
/*添加按钮到队尾*/
int addButtonToList(ButtonEvent * button){
	if(buttonList.tail==10) return -1;//添加失败
	buttonList.buttons[buttonList.tail++]=button;
	return 0;
}	

/*按钮生成*/
int initButton(int x,int y,int w,int h,int color,char str[],int fontSize,int (*fp)(struct ButtonEvent* button)){
	//添加按钮
	ButtonEvent* button = (ButtonEvent*)malloc(sizeof(ButtonEvent));
	button->x_min=x;
	button->y_min=y;
	button->x_max=x+w;
	button->y_max=y+h;
	button->p=fp;
	button->enable = 1;
	//绘制按钮信息
	fb_draw_rect(x,y,w,h,COLOR_BACKGROUND);
	fb_draw_border(x,y,w,h,color);
	//绘制字符串到正中心
	//获得字符串长度
	int length=0;
	while(str[length]!='\0') length++;
	int text_x = x + (w - length/3 * fontSize)/2;
	int text_y = y + (h + fontSize)/2;
	//绘制字体
	fb_draw_text(text_x,text_y,str,fontSize,color);
	//添加到按钮队列中
	addButtonToList(button);
	fb_update();
	return 0;
}

/*按钮触发检测，调用触发的函数*/
void buttonListClicked(int x,int y){
	
	int i = 0;
	printf("buttonList:\n");
	for(i = 0;i < buttonList.tail;i++){
		printf("button%d:x_min:%d,x_max:%d,y_min:%d,y_max:%d,enable:%d\n",i,buttonList.buttons[i]->x_min,buttonList.buttons[i]->x_max,buttonList.buttons[i]->y_min,buttonList.buttons[i]->y_max,buttonList.buttons[i]->enable);
	}
	printf("button check\n");

	for(i = buttonList.tail-1;i>=0;i--){
		printf("i=%d\n",i);
		if(buttonList.buttons[i]->enable==0) continue;
		if(x<buttonList.buttons[i]->x_max&&x>buttonList.buttons[i]->x_min&&y<buttonList.buttons[i]->y_max&&y>buttonList.buttons[i]->y_min){
			/*操作顺序：添加按钮--检测按钮--根据返回值调用按钮响应*/
			if(buttonList.buttons[i]->p==NULL) continue;
			(*(buttonList.buttons[i]->p))(buttonList.buttons[i]);
			break;
		}
	}
	printf("button check end\n");
}


/*五子棋相关函数封装*/

/*总游戏初始化*/
void initGame(){
	//初始化棋盘
	int i,j;
	for(i = 0;i < 16;i ++){
		for(j = 0;j < 16;j ++)	board[i][j]=0;
	}
	//初始化状态
	game_state = state_0;
	//清空按钮队列
	clearButtonList();
	cauculateCircleFunc();
}

/*初始化显示窗口*/
void initScreen(){
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
	fb_update();
}


/*初始化棋盘绘制*/
void drawChessBoard(){
	fb_draw_rect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,COLOR_BACKGROUND);
	initChessboard();
	fb_update();
}
/*棋盘绘制*/
void initChessboard(){
	int x = 242,y=30;
	int i;
	printf("begin--\n");
	for(i = 0;i < 16;i++){
		printf("i=%d\t",i);		
		fb_draw_line(x,y+36*i,x+540,y+36*i,COLOR_LINE);
		fb_draw_line(x+36*i,y,x+36*i,y+540,COLOR_LINE);
	}
	//fb_draw_border(241,29,542,542,ORANGE);
}
/*绘制节点函数值计算*/
void cauculateCircleFunc(){
	int i=0;
	for(;i < 18;i++){
		circleFunc[i]=(int)(sqrt(17*17-i*i)+0.5);
	}
	return;
}
/*落子绘制，x,y为棋盘坐标*/
void drawPiece(int x,int y,int color){
	if(x<0||x>15||y<0||y>15) return;
	board[x][y] = color;
	
	x = 242+36*x;
	y = 30+36*y;
	int i=0;
	for(;i<18;i++){
		fb_draw_line(x+i,y-circleFunc[i],x+i,y+circleFunc[i],color);
		fb_draw_line(x-i,y-circleFunc[i],x-i,y+circleFunc[i],color);
	}
	fb_update();
}
/*棋盘检测*/
int checkBoard(int x,int y){
	int curPieceColor = board[x][y];
	int numOfPieces=1;
	int i=x,j=y;
	//left to right
	while(i<15&&board[++i][j]==curPieceColor) numOfPieces++;
	//right to left
	i=x;
	while(i>0&&board[--i][j]==curPieceColor) numOfPieces++;
	if(numOfPieces == 5) return curPieceColor;
	
	//up to down
	i=x;numOfPieces=1;
	while(j<15&&board[i][++j]==curPieceColor) numOfPieces++;
	j=y;
	while(j>0&&board[i][--j]==curPieceColor) numOfPieces++;
	if(numOfPieces == 5) return curPieceColor;
	
	//left-up to right-down
	j=y;numOfPieces=1;
	while(j>0&&i>0&&board[--i][--j]==curPieceColor) numOfPieces++;
	i=x;j=y;
	while(j<15&&i<15&&board[++i][++j]==curPieceColor) numOfPieces++;
	if(numOfPieces == 5) return curPieceColor;
	
	//left-down to right-up
	i=x;j=y;numOfPieces=1;
	while(j>0&&i<15&&board[++i][--j]==curPieceColor) numOfPieces++;
	i=x;j=y;
	while(j<15&&i>0&&board[--i][++j]==curPieceColor) numOfPieces++;
	if(numOfPieces == 5) return curPieceColor;
	return 0;
}

/*触摸板事件控制模块，根据触摸位置和状态信息作出反应*/
void touch_event_cb(int fd){
	int type,x,y,finger;
	int no_x,no_y;
	type = touch_read(fd, &x,&y,&finger);
	switch(type){
	case TOUCH_PRESS:
		//按钮检测
		printf("%d,%d\n",x,y);
		printf("game_state=%d\n",game_state);

		if(game_state == state_6){
			printf("game_color=%d\n",game_color);
			/*获得坐标位置信息，发送信息*/
			no_x = (x-(242-18))/36;
			no_y = (y-(30-18))/36;
			if(no_x<0||no_x>15||no_y<0||no_y>15) break;
			if(board[no_x][no_y]!=0) break;
			/*绘制*/
			drawPiece(no_x,no_y,draw_color);
			fb_update();
			if(checkBoard(no_x,no_y)!=0){
				/*发送信息，状态修改*/
				bluetooth_send(5,no_x,no_y,bluetooth_fd);
				game_state = state_8;
				//添加按钮，本机获胜
				//打印本机失败并创建重新开始的按钮
					initButton(312,200,400,200,GREEN,"获胜",30,failureButtonEvent);
					fb_update();
			}else{
				/*状态修改*/
				game_state = state_7;
				
				/*发送坐标信息*/
				bluetooth_send(4,no_x,no_y,bluetooth_fd);
			}
		}else if(game_state == state_7){}
		else {
			printf("checked\n");
			buttonListClicked(x,y);
		}
		
		
		break;
	case TOUCH_ERROR:
		printf("close touch fd\n");
		break;
	default:
		return;
	}
	
	
	
}

/*五种信息类型：
	1 : 准备信息 1
	2 : 选择白棋 2
	3 : 选择黑棋 3
	4 : 落子坐标 4
	5 : 胜利信息 5
*/
/*蓝牙信息接收模块*/
void bluetooth_tty_event_cb(int fd)
{
	Message message;
	int n;
	n = myRead_nonblock(fd, &message, sizeof(Message));
	if(n <= 0) {
		printf("close bluetooth tty fd\n");
		//task_delete_file(fd);
		//close(fd);
		exit(0);
		return;
	}
	//接收信息
	printf("get message:type:%d\n",message.type);
	switch(message.type){
		
		case 1:
			switch(game_state){
				case state_0: /*状态0 接收信息*/
					/*对手完成准备，本机状态改变*/
					printf("state_0 to state_02\n");
					game_state = state_02;
					break;
				case state_01:
					/*完成准备，绘制选择界面*/
					printf("state_01 to state_1\n");
					game_state = state_1;	
					drawSelect();
					break;
				default:
					error("游戏状态转换，发送消息接收信息type1");
			}
			break;
		case 2:
			switch(game_state){
				case state_1:
					printf("state_1 to state_5\n");
					game_state = state_5;
					/*白棋被选择*/
					/*设置被选择信息，并将白棋选择按钮使能关闭*/
					buttonList.buttons[1]->enable=0;
					initButton(512,200,200,200,WHITE,"已被选择",30,NULL);
					fb_update();
					break;
				case state_2:
					/*清空按钮队列，绘制棋盘，转入状态6*/
					printf("state_2 to state_6\n");
					game_state = state_6;
					clearButtonList();
					drawChessBoard();
					break;
				default:
					error("游戏状态转换，接收消息type2");
			}
			break;
		case 3:
			switch(game_state){
				case state_1:
					printf("state_1 to state_4\n");
					game_state = state_4;
					/*黑棋被选择*/
					/*设置被选择信息，并将黑棋选择按钮使能关闭*/
					buttonList.buttons[0]->enable=0;
					initButton(312,200,200,200,BLACK,"已被选择",30,NULL);
					fb_update();
					break;
					
				case state_3:
					/*清空按钮队列，绘制棋盘，转入状态7*/
					printf("state_3 to state_7\n");
					game_state = state_7;
					clearButtonList();
					drawChessBoard();
				break;
				
				default:
					error("游戏状态转换，接收消息type3");
			}
			break;
		case 4:
			switch(game_state){
				case state_7:
					printf("state_7 to state_6\n");
					game_state = state_6;
					/*分析相关信息*/
					/*绘制棋子*/
					drawPiece(message.x,message.y,dis_draw_color);	//队首棋颜色绘制
				
					break;
				default:
					error("游戏状态转换，接收消息type4");
			}
			break;
		case 5:
			switch(game_state){
				case state_7:
					printf("state_7 to state_9\n");
					game_state = state_9;
					//打印本机失败并创建重新开始的按钮
					initButton(312,200,400,200,RED,"失败",30,failureButtonEvent);
					fb_update();
				break;
				default:
					error("游戏状态转换，接收消息type5");
			}
			break;
		default:
					error("游戏状态转换，消息类型不正确");
	}
	return;
}
/*蓝牙信息发送*/
void bluetooth_send(int ctype,int cx,int cy,int fd){
	//char buf[sizeof(Message)];
	Message *message = (Message*)malloc(sizeof(Message));
	message->type=ctype;
	message->x=cx;
	message->y=cy;
	myWrite_nonblock(fd, message, sizeof(Message));
	printf("send message:type:%d,size:%d\n",message->type,sizeof(Message));
	free(message);
}

int failureButtonEvent(ButtonEvent* button){
	//清空按钮队列
	clearButtonList();
	initScreen();
	//绘制开始信息界面
	//开始游戏，绘制开始界面，状态0
	initGame();
	//绘制准备界面,添加准备按钮
	initButton(312,200,400,200,GREEN,"开始游戏",30,startButtonHandler);
	game_state = state_0;
	return 0;
}
/*开始按钮事件处理*/
int startButtonHandler(ButtonEvent* button){
	printf("startButtonHandler");
	/*游戏状态:state_0-> state_01 or state_02 -> state_1*/
	/*修改游戏状态*/
	if(game_state==state_0){
		printf("state_0 to state_01");
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
		printf("state_02 to state_1");
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
void drawSelect(){
	initScreen();
	//绘制棋子选择界面，并添加两个按钮
	initButton(312,200,200,200,BLACK,"黑色",30,selectBlack);
	initButton(512,200,200,200,WHITE,"白色",30,selectWhite);
	fb_update();
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
		clearButtonList();
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
		game_state = state_3;
		//黑棋按钮修改为"已选择"，清空按钮队列
		initButton(512,200,200,200,BLACK,"已选择",30,NULL);
		fb_update();
		clearButtonList();
	}else if(game_state==state_4){
		game_state = state_7;
		//绘制棋盘
		initScreen();
		drawChessBoard();
	}
	return 0;
}


void error(char str[]){
	printf("error: %s",str);
	exit(-1);
}


