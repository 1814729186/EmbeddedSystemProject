#include "common.h"
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>


static int LCD_FB_FD;
static int *LCD_FB_BUF = NULL;
static int *LCD_FB_FRONT, *LCD_FB_BACK;
struct fb_var_screeninfo LCD_FB_VAR;
static int DRAW_BUF[SCREEN_WIDTH*SCREEN_HEIGHT];

static struct area {
	int x1, x2, y1, y2;
} update_area = {0,0,0,0};

#define AREA_SET_EMPTY(pa) do {\
	(pa)->x1 = SCREEN_WIDTH;\
	(pa)->x2 = 0;\
	(pa)->y1 = SCREEN_HEIGHT;\
	(pa)->y2 = 0;\
} while(0)

void fb_init(char *dev)
{
	int fd;
	struct fb_fix_screeninfo fb_fix;
	struct fb_var_screeninfo fb_var;

	if(LCD_FB_BUF != NULL) return; /*already done*/

	//First: Open the device
	if((fd = open(dev, O_RDWR)) < 0){
		printf("Unable to open framebuffer %s, errno = %d\n", dev, errno);
		return;
	}
	if(ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix) < 0){
		printf("Unable to FBIOGET_FSCREENINFO %s\n", dev);
		return;
	}
	if(ioctl(fd, FBIOGET_VSCREENINFO, &fb_var) < 0){
		printf("Unable to FBIOGET_VSCREENINFO %s\n", dev);
		return;
	}

	printf("framebuffer info: bits_per_pixel=%u,size=(%d,%d),virtual_pos_size=(%d,%d)(%d,%d),line_length=%u,smem_len=%u\n",
		fb_var.bits_per_pixel, fb_var.xres, fb_var.yres, fb_var.xoffset, fb_var.yoffset,
		fb_var.xres_virtual, fb_var.yres_virtual, fb_fix.line_length, fb_fix.smem_len);

	//Second: mmap
	int *addr;
	addr = mmap(NULL, fb_fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if((int)addr == -1){
		printf("failed to mmap memory for framebuffer.\n");
		return;
	}

	if((fb_var.xoffset != 0) ||(fb_var.yoffset != 0))
	{
		fb_var.xoffset = 0;
		fb_var.yoffset = 0;
		if(ioctl(fd, FBIOPAN_DISPLAY, &fb_var) < 0) {
			printf("FBIOPAN_DISPLAY framebuffer failed\n");
		}
	}


	LCD_FB_FD = fd;
	LCD_FB_BUF = addr;
	LCD_FB_FRONT = addr;
	LCD_FB_BACK = addr + fb_var.xres*fb_var.yres;
	LCD_FB_VAR = fb_var;

	//set empty
	AREA_SET_EMPTY(&update_area);
	return;
}

static void _copy_area(int *dst, int *src, struct area *pa)
{
	int x, y, w, h;
	x = pa->x1; w = pa->x2-x;
	y = pa->y1; h = pa->y2-y;
	src += y*SCREEN_WIDTH + x;
	dst += y*SCREEN_WIDTH + x;
	while(h-- > 0){
		memcpy(dst, src, w*4);
		src += SCREEN_WIDTH;
		dst += SCREEN_WIDTH;
	}
}

static int _check_area(struct area *pa)
{
	if(pa->x2 == 0) return 0; //is empty

	if(pa->x1 < 0) pa->x1 = 0;
	if(pa->x2 > SCREEN_WIDTH) pa->x2 = SCREEN_WIDTH;
	if(pa->y1 < 0) pa->y1 = 0;
	if(pa->y2 > SCREEN_HEIGHT) pa->y2 = SCREEN_HEIGHT;

	if((pa->x2 > pa->x1) && (pa->y2 > pa->y1))
		return 1; //no empty

	//set empty
	AREA_SET_EMPTY(pa);
	return 0;
}

void fb_update(void)
{
	if(_check_area(&update_area) == 0) return; //is empty
	_copy_area(LCD_FB_FRONT, DRAW_BUF, &update_area);
	AREA_SET_EMPTY(&update_area); //set empty
	return;
}

/*======================================================================*/

static void * _begin_draw(int x, int y, int w, int h)
{
	int x2 = x+w;
	int y2 = y+h;
	if(update_area.x1 > x) update_area.x1 = x;
	if(update_area.y1 > y) update_area.y1 = y;
	if(update_area.x2 < x2) update_area.x2 = x2;
	if(update_area.y2 < y2) update_area.y2 = y2;
	return DRAW_BUF;
}

void fb_draw_pixel(int x, int y, int color)
{
	//printf("x=%d,y=%d\n", x, y);
	if(x<0 || y<0 || x>=SCREEN_WIDTH || y>=SCREEN_HEIGHT) return;
	int *buf = _begin_draw(x,y,1,1);
/*---------------------------------------------------*/
	*(buf + y*SCREEN_WIDTH + x) = color;
/*---------------------------------------------------*/
	return;
}

void fb_draw_rect(int x, int y, int w, int h, int color)
{
	if(x < 0) { w += x; x = 0;}
	if(x+w > SCREEN_WIDTH) { w = SCREEN_WIDTH-x;}
	if(y < 0) { h += y; y = 0;}
	if(y+h >SCREEN_HEIGHT) { h = SCREEN_HEIGHT-y;}
	if(w<=0 || h<=0) return;
	int *buf = _begin_draw(x,y,w,h);
/*---------------------------------------------------*/
	//printf("you need implement fb_draw_rect()\n"); exit(0);
	int i, j;
	for (i = 0; i < h; ++i)
	{
		for (j = 0; j < w; ++j)
		{
			buf[(i + y) * SCREEN_WIDTH + j + x] = color;
		}
	}
/*---------------------------------------------------*/
	return;
}

void fb_draw_line(int x1, int y1, int x2, int y2, int color)
{
/*---------------------------------------------------*/
	//printf("you need implement fb_draw_line()\n"); exit(0);
	//printf("line from (%d,%d) to (%d,%d)\n", x1, y1, x2, y2);
	int dx,             // difference in x's
		dy,             // difference in y's
		dx2,            // dx,dy * 2
		dy2,
		x_inc,          // amount in pixel space to move during drawing
		y_inc,          // amount in pixel space to move during drawing
		error,          // the discriminant i.e. error i.e. decision variable
		index;          // used for looping 
	// compute horizontal and vertical deltas
	dx = x2 - x1;
	dy = y2 - y1;
	// test which direction the line is going in i.e. slope angle
	if (dx >= 0)
	{
		x_inc = 1;
	} // end if line is moving right
	else
	{
		x_inc = -1;
		dx = -dx;  // need absolute value
	} // end else moving left
	// test y component of slope
	if (dy >= 0)
	{
		y_inc = 1;
	} // end if line is moving down
	else
	{
		y_inc = -1;
		dy = -dy;  // need absolute value
	} // end else moving up
	// compute (dx,dy) * 2
	dx2 = dx << 1;
	dy2 = dy << 1;
	int beg_x = x1 < x2 ? x1 : x2;
	int beg_y = y1 < y2 ? y1 : y2;
	int* buf = _begin_draw(beg_x, beg_y, dx + 1, dy + 1);
	int x = x1, y = y1;	//indexes for ploting pixels
	// now based on which delta is greater we can draw the line
	if (dx > dy)
	{
		// initialize error term
		error = dy2 - dx;
		// draw the line
		for (index = 0; index <= dx; index++)
		{
			// set the pixel
			//*buf = color;
			//*(buf + y*SCREEN_WIDTH + x) = color;
			fb_draw_pixel(x ,y, color);
			// test if error has overflowed
			if (error >= 0)
			{
				error -= dx2;
				// move to next line
				y += y_inc;
			} // end if error overflowed
			// adjust the error term
			error += dy2;
			// move to the next pixel
			x += x_inc;
		} // end for
	} // end if |k| <= 1
	else
	{
		// initialize error term
		error = dx2 - dy;
		// draw the line
		for (index = 0; index <= dy; index++)
		{
			// set the pixel
			//*(buf + y*SCREEN_WIDTH + x) = color;
			fb_draw_pixel(x ,y, color);
			// test if error overflowed
			if (error >= 0)
			{
				error -= dy2;
				// move to next line
				x += x_inc;
			} // end if error overflowed
		 	// adjust the error term
			error += dx2;
			// move to the next pixel
			y += y_inc;
		} // end for
	} // end else |k| > 1

/*---------------------------------------------------*/
	return;
}

void fb_draw_circle(int x, int y, int r, int color)
{
	int tx = 0, ty = r, d = 3 - 2 * r;

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

void fb_draw_line_pen(int x1, int y1, int x2, int y2, void (*pen)(int x, int y, int color), int color)
{
	/*---------------------------------------------------*/
	//printf("you need implement fb_draw_line()\n"); exit(0);
	int dx,             // difference in x's
		dy,             // difference in y's
		dx2,            // dx,dy * 2
		dy2,
		x_inc,          // amount in pixel space to move during drawing
		y_inc,          // amount in pixel space to move during drawing
		error,          // the discriminant i.e. error i.e. decision variable
		index;          // used for looping 
	// compute horizontal and vertical deltas
	dx = x2 - x1;
	dy = y2 - y1;
	// test which direction the line is going in i.e. slope angle
	if (dx >= 0)
	{
		x_inc = 1;
	} // end if line is moving right
	else
	{
		x_inc = -1;
		dx = -dx;  // need absolute value
	} // end else moving left
	// test y component of slope
	if (dy >= 0)
	{
		y_inc = 1;
	} // end if line is moving down
	else
	{
		y_inc = -1;
		dy = -dy;  // need absolute value
	} // end else moving up
	// compute (dx,dy) * 2
	dx2 = dx << 1;
	dy2 = dy << 1;
	int beg_x = x1 < x2 ? x1 : x2;
	int beg_y = y1 < y2 ? y1 : y2;
	//int* buf = _begin_draw(beg_x, beg_y, dx, dy);
	int x = x1, y = y1;	//indexes for ploting pixels
	// now based on which delta is greater we can draw the line
	if (dx > dy)
	{
		// initialize error term
		error = dy2 - dx;
		// draw the line
		for (index = 0; index <= dx; index++)
		{
			// set the pixel
			//*(buf + y * SCREEN_WIDTH + x) = color;
			(*pen)(x, y, color);
			// test if error has overflowed
			if (error >= 0)
			{
				error -= dx2;
				// move to next line
				y += y_inc;
			} // end if error overflowed
			// adjust the error term
			error += dy2;
			// move to the next pixel
			x += x_inc;
		} // end for
	} // end if |k| <= 1
	else
	{
		// initialize error term
		error = dx2 - dy;
		// draw the line
		for (index = 0; index <= dy; index++)
		{
			// set the pixel
			//*(buf + y * SCREEN_WIDTH + x) = color;
			(*pen)(x, y, color);
			// test if error overflowed
			if (error >= 0)
			{
				error -= dy2;
				// move to next line
				x += x_inc;
			} // end if error overflowed
			// adjust the error term
			error += dx2;
			// move to the next pixel
			y += y_inc;
		} // end for
	} // end else |k| > 1

/*---------------------------------------------------*/
	return;
}

void fb_draw_image(int x, int y, fb_image *image, int color)
{
	if(image == NULL) return;

	int ix = 0; //image x
	int iy = 0; //image y
	int w = image->pixel_w; //draw width
	int h = image->pixel_h; //draw height

	if(x<0) {w+=x; ix-=x; x=0;}
	if(y<0) {h+=y; iy-=y; y=0;}
	
	if(x+w > SCREEN_WIDTH) {
		w = SCREEN_WIDTH - x;
	}
	if(y+h > SCREEN_HEIGHT) {
		h = SCREEN_HEIGHT - y;
	}
	if((w <= 0)||(h <= 0)) return;

	int *buf = _begin_draw(x,y,w,h);
/*---------------------------------------------------------------*/
	char *dst = (char *)(buf + y*SCREEN_WIDTH + x);
	char *src = image->content + iy*image->line_byte + ix*4;
/*---------------------------------------------------------------*/

	char alpha;
	char R1,G1,B1;
	int i,j;
	char *p,*pic_addr;
	if(image->color_type == FB_COLOR_RGB_8880) /*lab3: jpg*/
	{
		/*get a line of colors and copy them to buffer*/
		int bytes = (image->line_byte >= 4 * w) ? w * 4 : image->line_byte;
		for(i=0;i<h;i++){
			memcpy(dst+i*SCREEN_WIDTH*4,src+i*image->line_byte,bytes);
		}
		return;
	}
	if(image->color_type == FB_COLOR_RGBA_8888) /*lab3: png*/
	{
		for(i = 0;i < h;i++){
			for(j = 0;j < w;j++){
				p = (char*)(buf + (i+y)*SCREEN_WIDTH + j+x);//addr
				//图像信息
				pic_addr = image->content + i*image->line_byte + j*4;
				alpha = *(char*)(pic_addr+3);
				R1 = *(char*)(pic_addr+2);
				G1 =*(char*)(pic_addr+1);
				R1 =*pic_addr;
				switch(alpha){
					case 0:break;
					case 255:p[0]=B1;p[1]=G1;p[2]=R1;break;
					default:
						p[0] += (((B1 - p[0]) * alpha) >> 8);
                        			p[1] += (((G1 - p[1]) * alpha) >> 8);
                       				p[2] += (((R1 - p[2]) * alpha) >> 8);
				}
			}
		}

		return;
	}
	if(image->color_type == FB_COLOR_ALPHA_8) /*lab3: font*/
	{
		//get r,g,b
		R1 = (color&0xff0000)>>16;
		G1 = (color&0xff00)>>8;
		B1 = color&0xff;
		for(i = 0;i < h;i++){
			for(j = 0;j < w;j++){
				p = (char*)(buf + (i + y) * SCREEN_WIDTH + x + j);
				//get Alpha
				alpha = *(char*)(image->content + i * image->line_byte + j);
				switch (alpha) {
				case 0:break;
				case 255:p[0]=B1;p[1]=G1;p[2]=R1;break;
				default:
					p[0] += (((B1 - p[0]) * alpha) >> 8);
					p[1] += (((G1 - p[1]) * alpha) >> 8);
					p[2] += (((R1 - p[2]) * alpha) >> 8);
				}
			}		
		}
		return;
	}
/*---------------------------------------------------------------*/
	return;
}

void fb_draw_border(int x, int y, int w, int h, int color)
{
	if(w<=0 || h<=0) return;
	fb_draw_rect(x, y, w, 1, color);
	if(h > 1) {
		fb_draw_rect(x, y+h-1, w, 1, color);
		fb_draw_rect(x, y+1, 1, h-2, color);
		if(w > 1) fb_draw_rect(x+w-1, y+1, 1, h-2, color);
	}
}

/** draw a text string **/
void fb_draw_text(int x, int y, char *text, int font_size, int color)
{
	fb_image *img;
	fb_font_info info;
	int i=0;
	int len = strlen(text);
	while(i < len)
	{
		img = fb_read_font_image(text+i, font_size, &info);
		if(img == NULL) break;
		fb_draw_image(x+info.left, y-info.top, img, color);
		fb_free_image(img);

		x += info.advance_x;
		i += info.bytes;
	}
	return;
}

