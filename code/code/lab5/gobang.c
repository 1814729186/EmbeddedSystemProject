#include "gobang.h"

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

/*总游戏初始化*/
void initGame(){
	//初始化棋盘
	int i,j;
	for(i = 0;i < 16;i ++){
		//初始化游戏窗口并显示棋盘状态
		for(j = 0;j < 16;j ++)	{printf("%d ",board[i][j]);board[i][j]=0;}
		printf("\n");
	}
	//初始化状态
	game_state = state_0;
	//清空按钮队列
	clearButtonList();
	//计算绘制信息
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
	fb_draw_border(241,29,542,542,ORANGE);
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
	//left to rigte
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

void drawSelect(){
	initScreen();
	//绘制棋子选择界面，并添加两个按钮
	initButton(312,200,200,200,BLACK,"黑色",30,selectBlack);
	initButton(512,200,200,200,WHITE,"白色",30,selectWhite);
	fb_update();
}
void error(char str[]){
	printf("error: %s",str);
	exit(-1);
}

