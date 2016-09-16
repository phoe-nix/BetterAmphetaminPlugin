/*
	PSP VSH 24bpp text bliter
*/
#include <psp2/types.h>
#include <psp2/display.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "blit.h"
#include "font_chn.c"

#define ALPHA_BLEND 1
#define FONTX2 1

//extern unsigned char msx[];

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int pwidth, pheight, bufferwidth, pixelformat;
static unsigned int* vram32;

static uint32_t fcolor = 0x00ffffff;
static uint32_t bcolor = 0xff000000;

#if ALPHA_BLEND
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static uint32_t adjust_alpha(uint32_t col)
{
	uint32_t alpha = col>>24;
	uint8_t mul;
	uint32_t c1,c2;

	if(alpha==0)    return col;
	if(alpha==0xff) return col;

	c1 = col & 0x00ff00ff;
	c2 = col & 0x0000ff00;
	mul = (uint8_t)(255-alpha);
	c1 = ((c1*mul)>>8)&0x00ff00ff;
	c2 = ((c2*mul)>>8)&0x0000ff00;
	return (alpha<<24)|c1|c2;
}
#endif

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//int blit_setup(int sx,int sy,const char *msg,int fg_col,int bg_col)
int blit_setup(void)
{
	SceDisplayFrameBuf param;
	param.size = sizeof(SceDisplayFrameBuf);
	sceDisplayGetFrameBuf(&param, SCE_DISPLAY_SETBUF_IMMEDIATE);

	pwidth = param.width;
	pheight = param.height;
	vram32 = param.base;
	bufferwidth = param.pitch;
	pixelformat = param.pixelformat;

	if( (bufferwidth==0) || (pixelformat!=0)) return -1;

	fcolor = 0x00ffffff;
	bcolor = 0xff000000;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// blit text
/////////////////////////////////////////////////////////////////////////////
void blit_set_color(int fg_col,int bg_col)
{
	fcolor = fg_col;
	bcolor = bg_col;
}

/////////////////////////////////////////////////////////////////////////////
// blit text
/////////////////////////////////////////////////////////////////////////////

int blit_string(int sx,int sy,const char *msg)
{
	int x,y,p;
	int offset,font_offset;
	char code1,code2;
	unsigned char *font;
	uint32_t fg_col,bg_col;

#if ALPHA_BLEND
	uint32_t col,c1,c2;
	uint32_t alpha;

	fg_col = adjust_alpha(fcolor);
	bg_col = adjust_alpha(bcolor);
#else
	fg_col = fcolor;
	bg_col = bcolor;
#endif

//Kprintf("MODE %d WIDTH %d\n",pixelformat,bufferwidth);
	if( (bufferwidth==0) || (pixelformat!=0)) return -1;

	x=0;
	while(msg[x] && x<(pwidth/16))
	{
		code1 = msg[x];
		
		if (code1 < 128)
		{
			code2 = 0;
			font = &msx_asc[code1 * 12];
			for(y=0;y<12;y++,font++)
			{
				for(p=0;p<6;p++)
				{
#if FONTX2
					offset = (sy+(y*2))*bufferwidth + sx+x*12+p*2;
#else
					offset = (sy+y)*bufferwidth + sx+x*6+p;
#endif

#if ALPHA_BLEND
					col = (*font & (0x80 >> p)) ? fg_col : bg_col;
					alpha = col>>24;
					if(alpha==0)
					{
						vram32[offset] = col;
						vram32[offset + 1] = col;
						vram32[offset + bufferwidth] = col;
						vram32[offset + bufferwidth + 1] = col;
					}
					else if(alpha!=0xff)
					{
						c2 = vram32[offset];
						c1 = c2 & 0x00ff00ff;
						c2 = c2 & 0x0000ff00;
						c1 = ((c1*alpha)>>8)&0x00ff00ff;
						c2 = ((c2*alpha)>>8)&0x0000ff00;
						uint32_t color = (col&0xffffff) + c1 + c2;
						vram32[offset] = color;
						vram32[offset + 1] = color;
						vram32[offset + bufferwidth] = color;
						vram32[offset + bufferwidth + 1] = color;
					}
#else
					uint32_t color = (*font & 0x80) ? fg_col : bg_col;
					vram32[offset] = color;
					vram32[offset + 1] = color;
					vram32[offset + bufferwidth] = color;
					vram32[offset + bufferwidth + 1] = color;
#endif
				}
			}
			x++;
		}
		else
		{
			code2 = msg[x+1];

			{
				if( (code1 >= 0xA1) && (code1 <= 0xFE) && (code2 >= 0xA1) && (code2 <= 0xFE) )
					font_offset = 24 + ((94*(code1-0xA1)+(code2-0xA1))*24);
				else
					font_offset = 0;

				if(font_offset+24 > sizeof(msx_chs))
					font_offset = 0;

				font = &msx_chs[ font_offset ];
			}

			for(y=0;y<12;y++,font+=2)
			{
				for(p=0;p<12;p++)
				{
#if ALPHA_BLEND
					if (p < 8)
						col = (*font & (128 >> p)) ? fg_col : bg_col;
					else
						col = (*(font+1) & (128 >> (p-8))) ? fg_col : bg_col;

					alpha = col>>24;
#if FONTX2
					offset = (sy+(y*2))*bufferwidth + sx+x*12+p*2;
#else
					offset = (sy+y)*bufferwidth + sx+x*6+p;
#endif
					if(alpha==0)
					{
						vram32[offset] = col;
						vram32[offset + 1] = col;
						vram32[offset + bufferwidth] = col;
						vram32[offset + bufferwidth + 1] = col;
					}
					else if(alpha!=0xff)
					{
						c2 = vram32[offset];
						c1 = c2 & 0x00ff00ff;
						c2 = c2 & 0x0000ff00;
						c1 = ((c1*alpha)>>8)&0x00ff00ff;
						c2 = ((c2*alpha)>>8)&0x0000ff00;
						uint32_t color = (col&0xffffff) + c1 + c2;
						vram32[offset] = color;
						vram32[offset + 1] = color;
						vram32[offset + bufferwidth] = color;
						vram32[offset + bufferwidth + 1] = color;
					}
#else
				
					if (p < 8)
					uint32_t color = (*(font+1) & 0x80 >> p)) ? fg_col : bg_col;
					else
					uint32_t color = (*(font+1) & 0x80 >> (p-8))) ? fg_col : bg_col;
					vram32[offset] = color;
					vram32[offset + 1] = color;
					vram32[offset + bufferwidth] = color;
					vram32[offset + bufferwidth + 1] = color;
#endif
				}
			}
			x+=2;
		}
	}
	return x;
}


int blit_string_ctr(int sy,const char *msg)
{
	int sx = 960/2-strlen(msg)*(16/2);
	return blit_string(sx,sy,msg);
}

int blit_stringf(int sx, int sy, const char *msg, ...)
{
	va_list list;
	char string[512];

	va_start(list, msg);
	vsprintf(string, msg, list);
	va_end(list);

	return blit_string(sx, sy, string);
}
