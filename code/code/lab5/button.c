#include "gobang.h"

ButtonQueue buttonList;
/*初始化按钮队列*/
void initButtonList(){
	int i = 0;
	for(;i < 10;i ++) buttonList.buttons[i] = NULL;
	buttonList.tail = 0;
}

/*添加按钮到队尾*/
int addButtonToList(ButtonEvent * button){
	if(buttonList.tail==10) {error("添加按钮失败\n");return -1;}//添加失败
	buttonList.buttons[buttonList.tail++]=button;
	return 0;
}

/*移除队尾按钮*/
int deleteButtonFromList(){
	if(buttonList.tail==0) return -1;
	buttonList.tail--;
	if(buttonList.buttons[buttonList.tail]!=NULL) {
		free(buttonList.buttons[buttonList.tail]);
		buttonList.buttons[buttonList.tail] = NULL;
	}
	return 0;
}

/*清空按钮队列*/
void clearButtonList(){
	int i = 0;
	for(i = 0;i < buttonList.tail;i ++){
		if(buttonList.buttons[i]!=NULL){
			free(buttonList.buttons[i]);
			buttonList.buttons[i] = NULL;
		}
	}
	buttonList.tail=0;
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
	fb_draw_border(x,y,w,h,color);
	//绘制字符串到正中心
	//获得字符串长度
	int length=0;
	while(str[length]!='\0') length++;
	int text_x = x + (w - length * fontSize)/2;
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
	if(buttonList.tail==0){
		//无按钮信息存在
		return;
	}
	//打印存的按钮信息
	int i = 0;
	printf("buttonList:\n");
	for(i = 0;i < buttonList.tail;i++){
		printf("button%d:x_min:%d,x_max:%d,y_min:%d,y_max:%d,enable:%d\n",i,buttonList.buttons[i]->x_min,buttonList.buttons[i]->x_max,buttonList.buttons[i]->y_min,buttonList.buttons[i]->y_max,buttonList.buttons[i]->enable);
	}
	printf("button check begin:\n");
	for(i = buttonList.tail-1;i>=0;i--){
		printf("i=%d\n",i);
		if(buttonList.buttons[i]->enable==0) continue;
		if(x<buttonList.buttons[i]->x_max&&x>buttonList.buttons[i]->x_min&&y<buttonList.buttons[i]->y_max&&y>buttonList.buttons[i]->y_min){
			/*操作顺序：添加按钮--检测按钮--根据返回值调用按钮响应*/
			if(buttonList.buttons[i]->p==NULL) continue;
			if(buttonList.buttons[i]->p != NULL){
				(*(buttonList.buttons[i]->p))(buttonList.buttons[i]);
				//仅执行最末尾加入的按钮处理函数
				break;
			};
		}
	}
	printf("button check end\n");
}


