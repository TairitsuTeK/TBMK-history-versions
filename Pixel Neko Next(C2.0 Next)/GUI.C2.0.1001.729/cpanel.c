#include "cpanel.h"
#include "font.h"
#include "mouse.h"

static int slider_val = 2;

void cpanel_init(void)
{
    slider_val = mouse_get_speed();
}

void cpanel_draw(unsigned char* v, int wx, int wy, int ww, int wh)
{
    font_draw_text(v, wx+8, wy+26, "Mouse Speed", 0);
    int tx = wx+20, ty = wy+50;
    // 轨道
    for(int y=ty; y<ty+8; y++) for(int x=tx; x<tx+100; x++) v[y*320+x]=7;
    for(int x=tx; x<tx+100; x++) v[ty*320+x]=0;
    for(int x=tx; x<tx+100; x++) v[(ty+7)*320+x]=0;
    for(int y=ty; y<ty+8; y++) {v[y*320+tx]=0; v[y*320+tx+99]=0;}
    // 5个档位标记
    for(int i=0;i<5;i++){int sx=tx+i*20+8;for(int y=ty-2;y<ty+10;y++)for(int x=sx;x<sx+4;x++)v[y*320+x]=0;}
    // 滑块
    int sx=tx+slider_val*20-4, sy=ty-3;
    for(int y=sy;y<sy+14;y++)for(int x=sx;x<sx+12;x++)v[y*320+x]=9;
    for(int x=sx;x<sx+12;x++)v[sy*320+x]=11;
    for(int y=sy;y<sy+14;y++)v[y*320+sx]=11;
    for(int x=sx;x<sx+12;x++)v[(sy+13)*320+x]=1;
    for(int y=sy;y<sy+14;y++)v[y*320+sx+11]=1;
    char buf[10]="Speed: ";
    buf[7]='0'+slider_val; font_draw_text(v,wx+20,wy+70,buf,0);
}

void cpanel_click(unsigned char* v, int mx, int my, int wx, int wy, int ww, int wh)
{
    int tx=wx+20, ty=wy+50;
    if(my>=ty-3 && my<ty+11 && mx>=tx && mx<tx+100){
        int nv=(mx-tx)/20;
        if(nv<0)nv=0; if(nv>4)nv=4;
        slider_val=nv;
        mouse_set_speed(slider_val);
    }
}