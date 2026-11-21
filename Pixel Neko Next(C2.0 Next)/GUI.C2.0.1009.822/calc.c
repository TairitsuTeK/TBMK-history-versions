// calc.c - 温暖配色版（独立模块，无wm函数）
#include "calc.h"
#include "font.h"

static int display = 0;
static int op = 0;
static int stored = 0;
static int new_number = 1;

static int btn_x(int col) { return col * 26 + 8; }
static int btn_y(int row) { return row * 18 + 42; }

static int btn_color(const char* text)
{
    if(text[0]=='C') return 3;
    if(text[0]=='=') return 2;
    if(text[0]=='+'||text[0]=='-'||text[0]=='*'||text[0]=='/') return 1;
    return 0;
}

static void draw_btn(unsigned char* v, int wx, int wy, int col, int row, const char* text, int pressed)
{
    int bx = wx + btn_x(col), by = wy + btn_y(row);
    int c = btn_color(text);
    unsigned char bg = (c==0)?7:(c==1)?6:(c==2)?2:4;
    unsigned char hl = (c==0)?15:(c==1)?14:(c==2)?10:12;
    unsigned char sh = (c==0)?0:(c==1)?3:(c==2)?0:0;

    for(int y=by; y<by+16; y++) for(int x=bx; x<bx+24; x++) v[y*320+x]=bg;
    if(pressed){
        for(int x=bx; x<bx+24; x++) v[by*320+x]=sh;
        for(int y=by; y<by+16; y++) v[y*320+bx]=sh;
        for(int x=bx; x<bx+24; x++) v[(by+15)*320+x]=hl;
        for(int y=by; y<by+16; y++) v[y*320+bx+23]=hl;
    }else{
        for(int x=bx; x<bx+24; x++) v[by*320+x]=hl;
        for(int y=by; y<by+16; y++) v[y*320+bx]=hl;
        for(int x=bx; x<bx+24; x++) v[(by+15)*320+x]=sh;
        for(int y=by; y<by+16; y++) v[y*320+bx+23]=sh;
    }
    int tw = 0; while(text[tw]) tw++;
    int tx = bx + (24 - tw*6) / 2;
    int ty = by + (16 - 7) / 2;
    int fg = (c==3||c==2)?15:0;
    font_draw_text(v, tx, ty, text, fg);
}

static int in_btn(int mx, int my, int wx, int wy, int col, int row)
{
    int bx = wx + btn_x(col), by = wy + btn_y(row);
    return mx>=bx && mx<bx+24 && my>=by && my<by+16;
}

void calc_init(void)
{
    display = 0;
    op = 0;
    stored = 0;
    new_number = 1;
}

void calc_draw(unsigned char* v, int wx, int wy, int ww, int wh)
{
    for(int y=wy+15; y<wy+38; y++) for(int x=wx+4; x<wx+ww-4; x++) v[y*320+x]=15;
    for(int x=wx+4; x<wx+ww-4; x++){v[(wy+15)*320+x]=0; v[(wy+37)*320+x]=0;}
    for(int y=wy+15; y<wy+38; y++){v[y*320+wx+4]=0; v[y*320+wx+ww-5]=0;}

    if(display == -1){
        font_draw_text(v, wx+8, wy+24, "Error", 0);
    }else{
        char buf[16];
        int n = display, i = 0;
        if(n < 0){ buf[i++] = '-'; n = -n; }
        if(n == 0) buf[i++] = '0';
        while(n > 0 && i < 15){ buf[i++] = '0' + n % 10; n /= 10; }
        buf[i] = 0;
        int start = (display < 0) ? 1 : 0;
        int end = i - 1;
        while(start < end){ char t = buf[start]; buf[start] = buf[end]; buf[end] = t; start++; end--; }
        font_draw_text(v, wx+ww-8-i*6, wy+24, buf, 0);
    }

    draw_btn(v,wx,wy,0,0,"7",0); draw_btn(v,wx,wy,1,0,"8",0); draw_btn(v,wx,wy,2,0,"9",0); draw_btn(v,wx,wy,3,0,"/",0);
    draw_btn(v,wx,wy,0,1,"4",0); draw_btn(v,wx,wy,1,1,"5",0); draw_btn(v,wx,wy,2,1,"6",0); draw_btn(v,wx,wy,3,1,"*",0);
    draw_btn(v,wx,wy,0,2,"1",0); draw_btn(v,wx,wy,1,2,"2",0); draw_btn(v,wx,wy,2,2,"3",0); draw_btn(v,wx,wy,3,2,"-",0);
    draw_btn(v,wx,wy,0,3,"0",0); draw_btn(v,wx,wy,1,3,"C",0); draw_btn(v,wx,wy,2,3,"=",0); draw_btn(v,wx,wy,3,3,"+",0);
}

void calc_click(unsigned char* v, int mx, int my, int wx, int wy, int ww, int wh)
{
    int digits = (ww - 8) / 6;
    if(digits > 9) digits = 9;
    int max_val = 1;
    for(int i = 0; i < digits; i++) max_val *= 10;

    if(display == -1){
        if(in_btn(mx,my,wx,wy,1,3)){ display=0; op=0; stored=0; new_number=1; }
        return;
    }

    for(int digit=0; digit<=9; digit++){
        int col = (digit==0)?0:((digit-1)%3);
        int row = (digit==0)?3:(2-(digit-1)/3);
        if(in_btn(mx,my,wx,wy,col,row)){
            if(new_number){ display=0; new_number=0; }
            else if(display >= max_val / 10) return;
            display = display * 10 + digit;
            return;
        }
    }
    if(in_btn(mx,my,wx,wy,3,3)){ op=1; stored=display; new_number=1; return; }
    if(in_btn(mx,my,wx,wy,3,2)){ op=2; stored=display; new_number=1; return; }
    if(in_btn(mx,my,wx,wy,3,1)){ op=3; stored=display; new_number=1; return; }
    if(in_btn(mx,my,wx,wy,3,0)){ op=4; stored=display; new_number=1; return; }
    if(in_btn(mx,my,wx,wy,2,3)){
        int result = 0;
        if(op==1) result = stored + display;
        else if(op==2) result = stored - display;
        else if(op==3) result = stored * display;
        else if(op==4 && display!=0) result = stored / display;
        if(result >= max_val || result <= -max_val/10){ display = -1; op=0; new_number=1; return; }
        display = result;
        op=0; new_number=1;
        return;
    }
    if(in_btn(mx,my,wx,wy,1,3)){
        display=0; op=0; stored=0; new_number=1;
        return;
    }
}