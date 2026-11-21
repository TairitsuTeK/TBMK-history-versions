#include "calc.h"
#include "font.h"

static int display = 0;
static int op = 0;
static int stored = 0;
static int new_number = 1;

static int btn_x(int col) { return col * 24 + 6; }
static int btn_y(int row) { return row * 16 + 38; }

static void draw_btn(unsigned char* v, int wx, int wy, int col, int row, const char* text, int pressed)
{
    int bx = wx + btn_x(col), by = wy + btn_y(row);
    for(int y=by; y<by+14; y++) for(int x=bx; x<bx+22; x++) v[y*320+x]=7;
    if(pressed){
        for(int x=bx; x<bx+22; x++) v[by*320+x]=0;
        for(int y=by; y<by+14; y++) v[y*320+bx]=0;
    }else{
        for(int x=bx; x<bx+22; x++) v[by*320+x]=15;
        for(int y=by; y<by+14; y++) v[y*320+bx]=15;
        for(int x=bx; x<bx+22; x++) v[(by+13)*320+x]=0;
        for(int y=by; y<by+14; y++) v[y*320+bx+21]=0;
    }
    font_draw_text(v, bx+3, by+3, text, 0);
}

static int in_btn(int mx, int my, int wx, int wy, int col, int row)
{
    int bx = wx + btn_x(col), by = wy + btn_y(row);
    return mx>=bx && mx<bx+22 && my>=by && my<by+14;
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
    for(int y=wy+15; y<wy+33; y++) for(int x=wx+4; x<wx+ww-4; x++) v[y*320+x]=15;
    for(int x=wx+4; x<wx+ww-4; x++){v[(wy+15)*320+x]=0; v[(wy+32)*320+x]=0;}
    for(int y=wy+15; y<wy+33; y++){v[y*320+wx+4]=0; v[y*320+wx+ww-5]=0;}

    char buf[8];
    int n = display, i = 0;
    if(n==0) buf[i++]='0';
    else while(n>0 && i<7){buf[i++]='0'+n%10; n/=10;}
    buf[i]=0;
    for(int j=0;j<i/2;j++){char t=buf[j]; buf[j]=buf[i-1-j]; buf[i-1-j]=t;}
    font_draw_text(v, wx+ww-8-i*6, wy+22, buf, 0);

    draw_btn(v,wx,wy,0,0,"7",0); draw_btn(v,wx,wy,1,0,"8",0); draw_btn(v,wx,wy,2,0,"9",0); draw_btn(v,wx,wy,3,0,"/",0);
    draw_btn(v,wx,wy,0,1,"4",0); draw_btn(v,wx,wy,1,1,"5",0); draw_btn(v,wx,wy,2,1,"6",0); draw_btn(v,wx,wy,3,1,"*",0);
    draw_btn(v,wx,wy,0,2,"1",0); draw_btn(v,wx,wy,1,2,"2",0); draw_btn(v,wx,wy,2,2,"3",0); draw_btn(v,wx,wy,3,2,"-",0);
    draw_btn(v,wx,wy,0,3,"0",0); draw_btn(v,wx,wy,1,3,"C",0); draw_btn(v,wx,wy,2,3,"=",0); draw_btn(v,wx,wy,3,3,"+",0);
}

void calc_click(unsigned char* v, int mx, int my, int wx, int wy, int ww, int wh)
{
    for(int digit=0; digit<=9; digit++){
        int col = (digit==0)?0:((digit-1)%3);
        int row = (digit==0)?3:(2-(digit-1)/3);
        if(in_btn(mx,my,wx,wy,col,row)){
            if(new_number){display=0; new_number=0;}
            display = display*10 + digit;
            if(display>999999) display=999999;
            return;
        }
    }
    if(in_btn(mx,my,wx,wy,3,3)){op=1; stored=display; new_number=1; return;}
    if(in_btn(mx,my,wx,wy,3,2)){op=2; stored=display; new_number=1; return;}
    if(in_btn(mx,my,wx,wy,3,1)){op=3; stored=display; new_number=1; return;}
    if(in_btn(mx,my,wx,wy,3,0)){op=4; stored=display; new_number=1; return;}
    if(in_btn(mx,my,wx,wy,2,3)){
        if(op==1) display = stored + display;
        else if(op==2) display = stored - display;
        else if(op==3) display = stored * display;
        else if(op==4 && display!=0) display = stored / display;
        op=0; new_number=1;
        return;
    }
    if(in_btn(mx,my,wx,wy,1,3)){
        display=0; op=0; stored=0; new_number=1;
        return;
    }
}