#include "gobang.h"

/*初始化函数：蓝牙设备初始化*/
int bluetooth_tty_init(const char *dev)
{
	int fd = open(dev, O_RDWR|O_NOCTTY|O_NONBLOCK); /*非阻塞模式*/
	if(fd < 0){
		printf("bluetooth_tty_init open %s error(%d): %s\n", dev, errno, strerror(errno));
		return -1;
	}
	return fd;
}

/*触摸板事件控制模块，根据触摸位置和状态信息作出反应*/
void touch_event_cb(int fd){
	int type,x,y,finger;
	int no_x,no_y;
	type = touch_read(fd, &x,&y,&finger);
	switch(type){
	case TOUCH_PRESS:
		//触摸检测
		printf("%d,%d\n",x,y);
		printf("game_state=%d\n",game_state);	//当前状态信息
		if(game_state == state_6){
			printf("game_color:%d\n",game_color);
			/*获得坐标位置信息，发送信息*/
			no_x = (x-(242-18))/36;
			no_y = (y-(30-18))/36;
			
			if(no_x<0||no_x>15||no_y<0||no_y>15) break;
			/*本地绘制*/
			drawPiece(no_x,no_y,draw_color);
			
			fb_update();
			
			if(checkBoard(no_x,no_y)!=0){
				/*发送信息，状态修改*/
				bluetooth_send(5,no_x,no_y,bluetooth_fd);
				game_state = state_8;
				//添加按钮，本机获胜
				//打印本机失败并创建重新开始的按钮
				clearButtonList();
				initButton(312,200,400,200,GREEN,"获胜",30,failureButtonEvent);
				fb_update();
			}else{
				/*状态修改*/
				game_state = state_7;
				/*发送坐标信息*/
				bluetooth_send(4,no_x,no_y,bluetooth_fd);
				fb_update();
			}
		}else if(game_state==state_7){printf("非本机轮次");}
		else{
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

/*蓝牙信息接收模块*/
void bluetooth_tty_event_cb(int fd){
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
	printf("get message:type:%d,no_x:%d,no_y:%d\n",message.type,message.x,message.y);
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
					//buttonList.buttons[0]->enable=0;
					printf("debug#");
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
					drawPiece(message.x,message.y,dis_draw_color);	//棋颜色绘制
				
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
					clearButtonList();
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
	Message message;
	message.type=ctype;
	message.x=cx;
	message.y=cy;
	myWrite_nonblock(fd, &message, sizeof(Message));
	printf("send message:type:%d,size:%d",message.type,sizeof(Message));
}
