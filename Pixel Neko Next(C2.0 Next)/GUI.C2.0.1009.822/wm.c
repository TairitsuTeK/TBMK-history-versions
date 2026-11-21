// wm.c - 完整版（含文件管理器）
#include "wm.h"
#include "font.h"
#include "version.h"
#include "port.h"
#include "calc.h"
#include "cpanel.h"
#include "filemanager.h"

static int wx=80, wy=52, ww=160, wh=112;
static int wx2=100, wy2=70, ww2=100, wh2=70;
static int ww2_def=100, wh2_def=70;
static int wcx=95, wcy=50, wcw=140, wch=120;
static int wcw_def=140, wch_def=120;
static int cpx=85, cpy=55, cpw=150, cph=110;
static int cpw_def=150, cph_def=110;
static int fmx=85, fmy=50, fmw=150, fmh=120;
static int fmw_def=150, fmh_def=120;
static int wv=0, cv=0, clv=0, cpv=0, fmv=0;
static int am=0, cm=0, clm=0, cpm=0, fmm=0;
static int asx=80, asy=52, csx=100, csy=70, clsx=95, clsy=50, cpsx=85, cpsy=55, fmsx=85, fmsy=50;
static int zo=0;
static int dragging=0, dgo=0, dgoy=0, dgw=0;
static int owx=80, owy=52, owx2=100, owy2=70, owcx=95, owcy=50, ocpx=85, ocpy=55, ofmx=85, ofmy=50;
static int okp=0, okbx=0, okby=0;
static int clp=0, clcx=0, clcy=0, clw=0;
static int mip=0, miw=0;
static int aip=0, cip=0, clip=0, cpip=0, fip=0;
static int resizing=0, rsdir=0;
static int rsox=0, rsoy=0, rsow=0, rsoh=0;
static int tick=0;
static int redraw=0;
static int need_mr=0;
static int restart_pressed=0;
static int restart_confirm=0;
static int restart_cancel_pressed=0, restart_ok_pressed=0;

static unsigned char cmos_read(unsigned char a){outb(0x70,a);return inb(0x71);}
static int bcd_to_int(unsigned char b){return(b>>4)*10+(b&0x0F);}
static void get_time(int*h,int*m,int*s){*h=bcd_to_int(cmos_read(0x04));*m=bcd_to_int(cmos_read(0x02));*s=bcd_to_int(cmos_read(0x00));}

static void ok_btn(unsigned char* v, int bx, int by, int p)
{
    for(int y=by;y<by+12;y++)for(int x=bx;x<bx+32;x++)v[y*320+x]=7;
    if(p){for(int x=bx;x<bx+32;x++)v[by*320+x]=0;for(int y=by;y<by+12;y++)v[y*320+bx]=0;
           for(int x=bx;x<bx+32;x++)v[(by+11)*320+x]=15;for(int y=by;y<by+12;y++)v[y*320+bx+31]=15;
           font_draw_text(v,bx+9,by+3,"OK",0);}
    else{for(int x=bx;x<bx+32;x++)v[by*320+x]=15;for(int y=by;y<by+12;y++)v[y*320+bx]=15;
          for(int x=bx;x<bx+32;x++)v[(by+11)*320+x]=0;for(int y=by;y<by+12;y++)v[y*320+bx+31]=0;
          font_draw_text(v,bx+8,by+2,"OK",0);}
}

static void cl_btn(unsigned char* v, int cx, int cy, int p)
{
    for(int y=cy;y<cy+12;y++)for(int x=cx;x<cx+16;x++)v[y*320+x]=7;
    if(p){for(int x=cx;x<cx+16;x++)v[cy*320+x]=0;for(int y=cy;y<cy+12;y++)v[y*320+cx]=0;
           for(int x=cx;x<cx+16;x++)v[(cy+11)*320+x]=15;for(int y=cy;y<cy+12;y++)v[y*320+cx+15]=15;
           v[(cy+2)*320+cx+5]=0;v[(cy+2)*320+cx+10]=0;v[(cy+3)*320+cx+6]=0;v[(cy+3)*320+cx+9]=0;
           v[(cy+4)*320+cx+7]=0;v[(cy+4)*320+cx+8]=0;v[(cy+5)*320+cx+7]=0;v[(cy+5)*320+cx+8]=0;
           v[(cy+6)*320+cx+6]=0;v[(cy+6)*320+cx+9]=0;v[(cy+7)*320+cx+5]=0;v[(cy+7)*320+cx+10]=0;}
    else{for(int x=cx;x<cx+16;x++)v[cy*320+x]=15;for(int y=cy;y<cy+12;y++)v[y*320+cx]=15;
          for(int x=cx;x<cx+16;x++)v[(cy+11)*320+x]=0;for(int y=cy;y<cy+12;y++)v[y*320+cx+15]=0;
          v[(cy+1)*320+cx+5]=0;v[(cy+1)*320+cx+10]=0;v[(cy+2)*320+cx+6]=0;v[(cy+2)*320+cx+9]=0;
          v[(cy+3)*320+cx+7]=0;v[(cy+3)*320+cx+8]=0;v[(cy+4)*320+cx+6]=0;v[(cy+4)*320+cx+9]=0;
          v[(cy+5)*320+cx+5]=0;v[(cy+5)*320+cx+10]=0;}
}

static void mi_btn(unsigned char* v, int mx, int my, int p)
{
    for(int y=my;y<my+12;y++)for(int x=mx;x<mx+16;x++)v[y*320+x]=7;
    if(p){for(int x=mx;x<mx+16;x++)v[my*320+x]=0;for(int y=my;y<my+12;y++)v[y*320+mx]=0;
           for(int x=mx;x<mx+16;x++)v[(my+11)*320+x]=15;for(int y=my;y<my+12;y++)v[y*320+mx+15]=15;
           for(int x=mx+5;x<mx+11;x++)v[(my+9)*320+x]=0;}
    else{for(int x=mx;x<mx+16;x++)v[my*320+x]=15;for(int y=my;y<my+12;y++)v[y*320+mx]=15;
          for(int x=mx;x<mx+16;x++)v[(my+11)*320+x]=0;for(int y=my;y<my+12;y++)v[y*320+mx+15]=0;
          for(int x=mx+5;x<mx+11;x++)v[(my+9)*320+x]=0;}
}

static void icon_hl(unsigned char* v, int x, int y, const char* label)
{
    for(int i=y;i<y+16&&i<200;i++)for(int j=x;j<x+16&&j<320;j++)v[i*320+j]=9;
    for(int i=y+2;i<y+12;i++)for(int j=x+6;j<x+10;j++)v[i*320+j]=15;
    for(int i=y+12;i<y+14;i++)for(int j=x+6;j<x+10;j++)v[i*320+j]=15;
    font_draw_text(v, x-3, y+18, label, 0);
}

static int in_ok(int mx,int my,int bx,int by){return mx>=bx&&mx<bx+32&&my>=by&&my<by+12;}
static int in_cl(int mx,int my,int cx,int cy){return mx>=cx&&mx<cx+16&&my>=cy&&my<cy+12;}
static int in_mi(int mx,int my,int wx,int wy){return mx>=wx&&mx<wx+16&&my>=wy&&my<wy+12;}
static int in_ic(int mx,int my,int x,int y){return mx>=x&&mx<x+16&&my>=y&&my<y+16;}
static int in_ta(int mx,int my){return mx>=85&&mx<135&&my>=5&&my<13;}
static int in_tc(int mx,int my){return mx>=140&&mx<190&&my>=5&&my<13;}
static int in_tcl(int mx,int my){return mx>=195&&mx<245&&my>=5&&my<13;}
static int in_tcp(int mx,int my){return mx>=238&&mx<296&&my>=5&&my<13;}
static int in_tfm(int mx,int my){return mx>=210&&mx<255&&my>=5&&my<13;}
static int rs_dir(int mx,int my,int wx,int wy,int ww,int wh)
{
    int c=12;
    if(mx<wx||mx>wx+ww||my<wy||my>wy+wh)return 0;
    if(mx>=wx&&mx<wx+c&&my>=wy&&my<wy+c)return 1;
    if(mx>=wx+ww-c&&mx<wx+ww&&my>=wy&&my<wy+c)return 2;
    if(mx>=wx&&mx<wx+c&&my>=wy+wh-c&&my<wy+wh)return 3;
    if(mx>=wx+ww-c&&mx<wx+ww&&my>=wy+wh-c&&my<wy+wh)return 4;
    return 0;
}

static void taskbar(unsigned char* v, int a, int c, int cl, int cp, int fm)
{
    for(int y=0;y<18;y++)for(int x=0;x<320;x++)v[y*320+x]=8;
    for(int x=0;x<320;x++)v[0*320+x]=15;for(int y=0;y<18;y++)v[y*320+0]=15;
    for(int x=0;x<320;x++)v[17*320+x]=0;for(int y=0;y<18;y++)v[y*320+319]=0;
    if(a)font_draw_text(v,85,5,"[About]",15);
    if(c)font_draw_text(v,140,5,"[Clock]",15);
    if(cl)font_draw_text(v,195,5,"[Calc]",15);
    if(cp)font_draw_text(v,238,5,"[CPanel]",15);
    if(fm)font_draw_text(v,210,5,"[Files]",15);
    int rx=284,ry=1;
    for(int y=ry;y<ry+16;y++)for(int x=rx;x<rx+16;x++)v[y*320+x]=4;
    for(int x=rx;x<rx+16;x++)v[ry*320+x]=12;for(int y=ry;y<ry+16;y++)v[y*320+rx]=12;
    for(int x=rx;x<rx+16;x++)v[(ry+15)*320+x]=0;for(int y=ry;y<ry+16;y++)v[y*320+rx+15]=0;
    for(int i=ry+3;i<ry+13;i++)for(int j=rx+5;j<rx+11;j++)v[i*320+j]=15;
    for(int i=ry+4;i<ry+12;i++)for(int j=rx+6;j<rx+10;j++)v[i*320+j]=4;
    for(int i=ry+2;i<ry+8;i++)for(int j=rx+7;j<rx+9;j++)v[i*320+j]=15;
}

static void window(unsigned char* v, int wx, int wy, const char* t, int ww, int wh, int rs)
{
    for(int y=wy+1;y<wy+wh+1&&y<200;y++)for(int x=wx+1;x<wx+ww+1&&x<320;x++)v[y*320+x]=0;
    for(int y=wy;y<wy+wh&&y<200;y++)for(int x=wx;x<wx+ww&&x<320;x++)
        v[y*320+x]=(y==wy||y==wy+wh-1||x==wx||x==wx+ww-1)?0:(y<wy+12)?1:15;
    cl_btn(v,wx+ww-18,wy+2,0);mi_btn(v,wx+ww-36,wy+2,0);
    font_draw_text(v,wx+5,wy+3,t,15);
    if(rs){int c=10;for(int i=0;i<c;i++)for(int j=0;j<=i;j++)
        {int px=wx+ww-c+j,py=wy+wh-c+i;if(px<320&&py<200)v[py*320+px]=8;}}
}

static void about_win(unsigned char* v, int wx, int wy)
{
    window(v,wx,wy,"About",160,112,0);
    int lx=wx+54,ly=wy+14;
    for(int y=ly+3;y<ly+18;y++)for(int x=lx-7;x<lx+48;x++)v[y*320+x]=1;
    for(int x=lx-4;x<lx+45;x++)v[(ly+3)*320+x]=9;
    for(int y=ly;y<ly+12;y++)for(int x=lx;x<lx+12;x++)v[y*320+x]=(y==ly||x==lx)?12:4;
    for(int y=ly;y<ly+12;y++)for(int x=lx+10;x<lx+22;x++)v[y*320+x]=(y==ly||x==lx+10)?10:2;
    for(int y=ly+7;y<ly+19;y++)for(int x=lx+19;x<lx+31;x++)v[y*320+x]=(y==ly+7||x==lx+19)?11:3;
    for(int y=ly+7;y<ly+19;y++)for(int x=lx+29;x<lx+41;x++)v[y*320+x]=(y==ly+7||x==lx+29)?13:5;
    font_draw_char(v,lx+4,ly+4,'T',15);font_draw_char(v,lx+14,ly+4,'B',15);
    font_draw_char(v,lx+23,ly+11,'M',15);font_draw_char(v,lx+33,ly+11,'K',15);
    int ly2=wy+40;for(int x=wx+4;x<wx+156;x++)v[ly2*320+x]=8;
    font_draw_text(v,wx+8,wy+46,VER_STRING1,0);font_draw_text(v,wx+8,wy+54,VER_STRING2,0);
    font_draw_text(v,wx+8,wy+64,VER_STRING3,8);ok_btn(v,wx+118,wy+90,0);
}

static void box(unsigned char* v, int x, int y, int w, int h)
{
    for(int i=x;i<x+w;i+=2){if(i<320&&y<200&&y>=0)v[y*320+i]=0;if(i<320&&y+h-1<200&&y+h-1>=0)v[(y+h-1)*320+i]=0;}
    for(int i=y;i<y+h;i+=2){if(x<320&&i<200&&i>=0)v[i*320+x]=0;if(x+w-1<320&&i<200&&i>=0)v[i*320+x+w-1]=0;}
}

static void icon(unsigned char* v, int x, int y, unsigned char bg, const char* lb)
{
    if(bg==1){
        for(int i=y;i<y+16;i++)for(int j=x;j<x+16;j++)v[i*320+j]=9;
        for(int j=x;j<x+16;j++)v[y*320+j]=11;for(int i=y;i<y+16;i++)v[i*320+x]=11;
        for(int j=x;j<x+16;j++)v[(y+15)*320+j]=1;for(int i=y;i<y+16;i++)v[i*320+x+15]=1;
        for(int i=y+3;i<y+7;i++)for(int j=x+6;j<x+10;j++)v[i*320+j]=15;
        for(int i=y+8;i<y+9;i++)for(int j=x+6;j<x+10;j++)v[i*320+j]=15;
        for(int i=y+10;i<y+12;i++)for(int j=x+7;j<x+9;j++)v[i*320+j]=15;
    }else if(bg==2){
        for(int i=y;i<y+16;i++)for(int j=x;j<x+16;j++)v[i*320+j]=8;
        for(int i=y+2;i<y+14;i++)for(int j=x+2;j<x+14;j++)v[i*320+j]=15;
        for(int j=x+2;j<x+14;j++){v[(y+2)*320+j]=0;v[(y+13)*320+j]=0;}
        for(int i=y+2;i<y+14;i++){v[i*320+x+2]=0;v[i*320+x+13]=0;}
        v[(y+8)*320+x+7]=0;v[(y+8)*320+x+8]=0;v[(y+8)*320+x+9]=0;
        v[(y+5)*320+x+7]=0;v[(y+6)*320+x+7]=0;v[(y+7)*320+x+7]=0;
        v[(y+8)*320+x+7]=4;
    }else if(bg==4){
        for(int i=y;i<y+16;i++)for(int j=x;j<x+16;j++)v[i*320+j]=7;
        for(int j=x;j<x+16;j++)v[y*320+j]=15;for(int i=y;i<y+16;i++)v[i*320+x]=15;
        for(int j=x;j<x+16;j++)v[(y+15)*320+j]=0;for(int i=y;i<y+16;i++)v[i*320+x+15]=0;
        for(int i=y+2;i<y+6;i++)for(int j=x+2;j<x+14;j++)v[i*320+j]=2;
        for(int j=x+2;j<x+14;j++)v[(y+2)*320+j]=0;
        for(int i=y+7;i<y+10;i++)for(int j=x+2;j<x+6;j++)v[i*320+j]=15;
        for(int i=y+7;i<y+10;i++)for(int j=x+7;j<x+11;j++)v[i*320+j]=15;
        for(int i=y+11;i<y+14;i++)for(int j=x+2;j<x+6;j++)v[i*320+j]=15;
        for(int i=y+11;i<y+14;i++)for(int j=x+7;j<x+11;j++)v[i*320+j]=15;
        for(int i=y+7;i<y+14;i++)for(int j=x+12;j<x+14;j++)v[i*320+j]=4;
        v[(y+3)*320+x+8]=0;v[(y+3)*320+x+9]=0;
        v[(y+3)*320+x+10]=0;v[(y+3)*320+x+11]=0;
    }else if(bg==6){
        for(int i=y;i<y+16;i++)for(int j=x;j<x+16;j++)v[i*320+j]=8;
        for(int j=x;j<x+16;j++)v[y*320+j]=15;
        for(int i=y;i<y+16;i++)v[i*320+x]=15;
        for(int j=x;j<x+16;j++)v[(y+15)*320+j]=0;
        for(int i=y;i<y+16;i++)v[i*320+x+15]=0;
        for(int j=x+3;j<x+13;j++)v[(y+4)*320+j]=15;
        for(int j=x+3;j<x+13;j++)v[(y+8)*320+j]=15;
        for(int j=x+3;j<x+13;j++)v[(y+12)*320+j]=15;
        for(int i=y+3;i<y+7;i++)for(int j=x+7;j<x+9;j++)v[i*320+j]=9;
        v[(y+5)*320+x+7]=11;v[(y+3)*320+x+7]=11;
    }else if(bg==5){
        for(int i=y;i<y+16;i++)for(int j=x;j<x+16;j++)v[i*320+j]=7;
        for(int j=x;j<x+16;j++)v[y*320+j]=15;for(int i=y;i<y+16;i++)v[i*320+x]=15;
        for(int j=x;j<x+16;j++)v[(y+15)*320+j]=0;for(int i=y;i<y+16;i++)v[i*320+x+15]=0;
        for(int i=y+3;i<y+13;i++)for(int j=x+3;j<x+13;j++)v[i*320+j]=6;
        for(int j=x+3;j<x+13;j++)v[(y+3)*320+j]=14;
        for(int i=y+3;i<y+5;i++)for(int j=x+3;j<x+7;j++)v[i*320+j]=14;
    }
    font_draw_text(v, x-4, y+18, lb, 0);
}

static void draw_restart_dialog(unsigned char* v)
{
    for(int y=70;y<120;y++)for(int x=100;x<280;x++)v[y*320+x]=8;
    for(int y=72;y<118;y++)for(int x=102;x<278;x++)v[y*320+x]=7;
    for(int x=102;x<278;x++){v[72*320+x]=15;v[117*320+x]=0;}
    for(int y=72;y<118;y++){v[y*320+102]=15;v[y*320+277]=0;}
    font_draw_text(v,125,80,"Restart?",0);font_draw_text(v,115,92,"Are you sure?",0);
    int bx=115,by=104;
    for(int y=by;y<by+12;y++)for(int x=bx;x<bx+55;x++)v[y*320+x]=restart_cancel_pressed?0:7;
    if(!restart_cancel_pressed){for(int x=bx;x<bx+55;x++)v[by*320+x]=15;for(int y=by;y<by+12;y++)v[y*320+bx]=15;
        for(int x=bx;x<bx+55;x++)v[(by+11)*320+x]=0;for(int y=by;y<by+12;y++)v[y*320+bx+54]=0;
        font_draw_text(v,bx+14,by+2,"Cancel",0);}
    else{for(int x=bx;x<bx+55;x++)v[by*320+x]=0;for(int y=by;y<by+12;y++)v[y*320+bx]=0;
        for(int x=bx;x<bx+55;x++)v[(by+11)*320+x]=15;for(int y=by;y<by+12;y++)v[y*320+bx+54]=15;
        font_draw_text(v,bx+15,by+3,"Cancel",0);}
    int rx=185,ry=104;
    for(int y=ry;y<ry+12;y++)for(int x=rx;x<rx+55;x++)v[y*320+x]=restart_ok_pressed?0:4;
    if(!restart_ok_pressed){for(int x=rx;x<rx+55;x++)v[ry*320+x]=12;for(int y=ry;y<ry+12;y++)v[y*320+rx]=12;
        for(int x=rx;x<rx+55;x++)v[(ry+11)*320+x]=0;for(int y=ry;y<ry+12;y++)v[y*320+rx+54]=0;
        font_draw_text(v,rx+20,ry+2,"OK",15);}
    else{for(int x=rx;x<rx+55;x++)v[ry*320+x]=0;for(int y=ry;y<ry+12;y++)v[y*320+rx]=0;
        for(int x=rx;x<rx+55;x++)v[(ry+11)*320+x]=12;for(int y=ry;y<ry+12;y++)v[y*320+rx+54]=12;
        font_draw_text(v,rx+21,ry+3,"OK",15);}
}

static void draw_others(unsigned char* v, int dgw)
{
    if(zo==0){if(dgw!=4&&cpv&&!cpm){window(v,cpx,cpy,"Control Panel",cpw,cph,0);cpanel_draw(v,cpx,cpy,cpw,cph);}
        if(dgw!=3&&clv&&!clm){window(v,wcx,wcy,"Calc",wcw,wch,0);calc_draw(v,wcx,wcy,wcw,wch);}
        if(dgw!=2&&cv&&!cm){window(v,wx2,wy2,"Clock",ww2,wh2,1);int h,m,s;get_time(&h,&m,&s);char b[9];
            b[0]='0'+h/10;b[1]='0'+h%10;b[2]=':';b[3]='0'+m/10;b[4]='0'+m%10;b[5]=':';b[6]='0'+s/10;b[7]='0'+s%10;b[8]=0;
            font_draw_text(v,wx2+ww2/2-18,wy2+wh2/2-4,b,0);}
        if(dgw!=1&&wv&&!am)about_win(v,wx,wy);
        if(dgw!=5&&fmv&&!fmm){window(v,fmx,fmy,"Files",fmw,fmh,0);fm_draw(v,fmx,fmy,fmw,fmh);}}
    else if(zo==1){if(dgw!=4&&cpv&&!cpm){window(v,cpx,cpy,"Control Panel",cpw,cph,0);cpanel_draw(v,cpx,cpy,cpw,cph);}
        if(dgw!=3&&clv&&!clm){window(v,wcx,wcy,"Calc",wcw,wch,0);calc_draw(v,wcx,wcy,wcw,wch);}
        if(dgw!=1&&wv&&!am)about_win(v,wx,wy);
        if(dgw!=2&&cv&&!cm){window(v,wx2,wy2,"Clock",ww2,wh2,1);int h,m,s;get_time(&h,&m,&s);char b[9];
            b[0]='0'+h/10;b[1]='0'+h%10;b[2]=':';b[3]='0'+m/10;b[4]='0'+m%10;b[5]=':';b[6]='0'+s/10;b[7]='0'+s%10;b[8]=0;
            font_draw_text(v,wx2+ww2/2-18,wy2+wh2/2-4,b,0);}
        if(dgw!=5&&fmv&&!fmm){window(v,fmx,fmy,"Files",fmw,fmh,0);fm_draw(v,fmx,fmy,fmw,fmh);}}
    else if(zo==2){if(dgw!=4&&cpv&&!cpm){window(v,cpx,cpy,"Control Panel",cpw,cph,0);cpanel_draw(v,cpx,cpy,cpw,cph);}
        if(dgw!=1&&wv&&!am)about_win(v,wx,wy);
        if(dgw!=2&&cv&&!cm){window(v,wx2,wy2,"Clock",ww2,wh2,1);int h,m,s;get_time(&h,&m,&s);char b[9];
            b[0]='0'+h/10;b[1]='0'+h%10;b[2]=':';b[3]='0'+m/10;b[4]='0'+m%10;b[5]=':';b[6]='0'+s/10;b[7]='0'+s%10;b[8]=0;
            font_draw_text(v,wx2+ww2/2-18,wy2+wh2/2-4,b,0);}
        if(dgw!=3&&clv&&!clm){window(v,wcx,wcy,"Calc",wcw,wch,0);calc_draw(v,wcx,wcy,wcw,wch);}
        if(dgw!=5&&fmv&&!fmm){window(v,fmx,fmy,"Files",fmw,fmh,0);fm_draw(v,fmx,fmy,fmw,fmh);}}
    else if(zo==3){if(dgw!=1&&wv&&!am)about_win(v,wx,wy);
        if(dgw!=2&&cv&&!cm){window(v,wx2,wy2,"Clock",ww2,wh2,1);int h,m,s;get_time(&h,&m,&s);char b[9];
            b[0]='0'+h/10;b[1]='0'+h%10;b[2]=':';b[3]='0'+m/10;b[4]='0'+m%10;b[5]=':';b[6]='0'+s/10;b[7]='0'+s%10;b[8]=0;
            font_draw_text(v,wx2+ww2/2-18,wy2+wh2/2-4,b,0);}
        if(dgw!=3&&clv&&!clm){window(v,wcx,wcy,"Calc",wcw,wch,0);calc_draw(v,wcx,wcy,wcw,wch);}
        if(dgw!=4&&cpv&&!cpm){window(v,cpx,cpy,"Control Panel",cpw,cph,0);cpanel_draw(v,cpx,cpy,cpw,cph);}
        if(dgw!=5&&fmv&&!fmm){window(v,fmx,fmy,"Files",fmw,fmh,0);fm_draw(v,fmx,fmy,fmw,fmh);}}
    else{if(dgw!=1&&wv&&!am)about_win(v,wx,wy);
        if(dgw!=2&&cv&&!cm){window(v,wx2,wy2,"Clock",ww2,wh2,1);int h,m,s;get_time(&h,&m,&s);char b[9];
            b[0]='0'+h/10;b[1]='0'+h%10;b[2]=':';b[3]='0'+m/10;b[4]='0'+m%10;b[5]=':';b[6]='0'+s/10;b[7]='0'+s%10;b[8]=0;
            font_draw_text(v,wx2+ww2/2-18,wy2+wh2/2-4,b,0);}
        if(dgw!=3&&clv&&!clm){window(v,wcx,wcy,"Calc",wcw,wch,0);calc_draw(v,wcx,wcy,wcw,wch);}
        if(dgw!=4&&cpv&&!cpm){window(v,cpx,cpy,"Control Panel",cpw,cph,0);cpanel_draw(v,cpx,cpy,cpw,cph);}
        if(dgw!=5&&fmv&&!fmm){window(v,fmx,fmy,"Files",fmw,fmh,0);fm_draw(v,fmx,fmy,fmw,fmh);}}
}

static void draw_all(unsigned char* v)
{
    if(zo==0){if(cpv&&!cpm){window(v,cpx,cpy,"Control Panel",cpw,cph,0);cpanel_draw(v,cpx,cpy,cpw,cph);}
        if(clv&&!clm){window(v,wcx,wcy,"Calc",wcw,wch,0);calc_draw(v,wcx,wcy,wcw,wch);}
        if(cv&&!cm){window(v,wx2,wy2,"Clock",ww2,wh2,1);int h,m,s;get_time(&h,&m,&s);char b[9];
            b[0]='0'+h/10;b[1]='0'+h%10;b[2]=':';b[3]='0'+m/10;b[4]='0'+m%10;b[5]=':';b[6]='0'+s/10;b[7]='0'+s%10;b[8]=0;
            font_draw_text(v,wx2+ww2/2-18,wy2+wh2/2-4,b,0);}
        if(wv&&!am)about_win(v,wx,wy);
        if(fmv&&!fmm){window(v,fmx,fmy,"Files",fmw,fmh,0);fm_draw(v,fmx,fmy,fmw,fmh);}}
    else if(zo==1){if(cpv&&!cpm){window(v,cpx,cpy,"Control Panel",cpw,cph,0);cpanel_draw(v,cpx,cpy,cpw,cph);}
        if(clv&&!clm){window(v,wcx,wcy,"Calc",wcw,wch,0);calc_draw(v,wcx,wcy,wcw,wch);}
        if(wv&&!am)about_win(v,wx,wy);
        if(cv&&!cm){window(v,wx2,wy2,"Clock",ww2,wh2,1);int h,m,s;get_time(&h,&m,&s);char b[9];
            b[0]='0'+h/10;b[1]='0'+h%10;b[2]=':';b[3]='0'+m/10;b[4]='0'+m%10;b[5]=':';b[6]='0'+s/10;b[7]='0'+s%10;b[8]=0;
            font_draw_text(v,wx2+ww2/2-18,wy2+wh2/2-4,b,0);}
        if(fmv&&!fmm){window(v,fmx,fmy,"Files",fmw,fmh,0);fm_draw(v,fmx,fmy,fmw,fmh);}}
    else if(zo==2){if(cpv&&!cpm){window(v,cpx,cpy,"Control Panel",cpw,cph,0);cpanel_draw(v,cpx,cpy,cpw,cph);}
        if(wv&&!am)about_win(v,wx,wy);
        if(cv&&!cm){window(v,wx2,wy2,"Clock",ww2,wh2,1);int h,m,s;get_time(&h,&m,&s);char b[9];
            b[0]='0'+h/10;b[1]='0'+h%10;b[2]=':';b[3]='0'+m/10;b[4]='0'+m%10;b[5]=':';b[6]='0'+s/10;b[7]='0'+s%10;b[8]=0;
            font_draw_text(v,wx2+ww2/2-18,wy2+wh2/2-4,b,0);}
        if(clv&&!clm){window(v,wcx,wcy,"Calc",wcw,wch,0);calc_draw(v,wcx,wcy,wcw,wch);}
        if(fmv&&!fmm){window(v,fmx,fmy,"Files",fmw,fmh,0);fm_draw(v,fmx,fmy,fmw,fmh);}}
    else if(zo==3){if(wv&&!am)about_win(v,wx,wy);
        if(cv&&!cm){window(v,wx2,wy2,"Clock",ww2,wh2,1);int h,m,s;get_time(&h,&m,&s);char b[9];
            b[0]='0'+h/10;b[1]='0'+h%10;b[2]=':';b[3]='0'+m/10;b[4]='0'+m%10;b[5]=':';b[6]='0'+s/10;b[7]='0'+s%10;b[8]=0;
            font_draw_text(v,wx2+ww2/2-18,wy2+wh2/2-4,b,0);}
        if(clv&&!clm){window(v,wcx,wcy,"Calc",wcw,wch,0);calc_draw(v,wcx,wcy,wcw,wch);}
        if(cpv&&!cpm){window(v,cpx,cpy,"Control Panel",cpw,cph,0);cpanel_draw(v,cpx,cpy,cpw,cph);}
        if(fmv&&!fmm){window(v,fmx,fmy,"Files",fmw,fmh,0);fm_draw(v,fmx,fmy,fmw,fmh);}}
    else{if(wv&&!am)about_win(v,wx,wy);
        if(cv&&!cm){window(v,wx2,wy2,"Clock",ww2,wh2,1);int h,m,s;get_time(&h,&m,&s);char b[9];
            b[0]='0'+h/10;b[1]='0'+h%10;b[2]=':';b[3]='0'+m/10;b[4]='0'+m%10;b[5]=':';b[6]='0'+s/10;b[7]='0'+s%10;b[8]=0;
            font_draw_text(v,wx2+ww2/2-18,wy2+wh2/2-4,b,0);}
        if(clv&&!clm){window(v,wcx,wcy,"Calc",wcw,wch,0);calc_draw(v,wcx,wcy,wcw,wch);}
        if(cpv&&!cpm){window(v,cpx,cpy,"Control Panel",cpw,cph,0);cpanel_draw(v,cpx,cpy,cpw,cph);}
        if(fmv&&!fmm){window(v,fmx,fmy,"Files",fmw,fmh,0);fm_draw(v,fmx,fmy,fmw,fmh);}}
    if(restart_confirm) draw_restart_dialog(v);
}

void wm_init(void){wx=80;wy=52;ww=160;wh=112;wx2=100;wy2=70;ww2=100;wh2=70;ww2_def=100;wh2_def=70;
    wcx=95;wcy=50;wcw=140;wch=120;wcw_def=140;wch_def=120;
    cpx=85;cpy=55;cpw=150;cph=110;cpw_def=150;cph_def=110;
    fmx=85;fmy=50;fmw=150;fmh=120;fmw_def=150;fmh_def=120;
    calc_init();cpanel_init();fm_init();redraw=1;}

void wm_draw(unsigned char* v){if(redraw){for(int i=0;i<320*200;i++)v[i]=7;
    taskbar(v,am||wv,cm||cv,clm||clv,cpm||cpv,fmm||fmv);
    icon(v,20,164,1,"About");icon(v,60,164,2,"Clock");icon(v,100,164,4,"Calc");icon(v,140,164,6,"CPanel");icon(v,180,164,5,"Files");
    draw_all(v);font_draw_text(v,200,192,"Pixel Neko Next Beta",8);}redraw=0;}

int wm_is_dragging(void){return dragging;}
int wm_need_mouse_reset(void){int r=need_mr;need_mr=0;return r;}

void wm_update(unsigned char* v, int mx, int my, int mb, int mb_press, int mb_release)
{
    redraw=0;tick++;
    if(cv&&!cm&&!dragging&&!resizing&&!restart_confirm&&!clp&&!okp&&!mip&&tick>50000){tick=0;redraw=1;}

    if(restart_confirm){
        if(mb_press && mx>=115 && mx<170 && my>=104 && my<116){restart_cancel_pressed=1;need_mr=1;}
        if(mb_release && restart_cancel_pressed){restart_cancel_pressed=0;
            if(mx>=115 && mx<170 && my>=104 && my<116){restart_confirm=0;redraw=1;}else{redraw=1;}}
        if(mb_press && mx>=185 && mx<240 && my>=104 && my<116){restart_ok_pressed=1;need_mr=1;}
        if(mb_release && restart_ok_pressed){restart_ok_pressed=0;
            if(mx>=185 && mx<240 && my>=104 && my<116){outb(0x64,0xFE);}else{redraw=1;}}
        if(redraw) need_mr=1;return;
    }

    if(mb_press && mx>=284 && mx<300 && my>=1 && my<17){
        restart_pressed=1;
        int rx=284,ry=1;
        for(int y=ry;y<ry+16;y++)for(int x=rx;x<rx+16;x++)v[y*320+x]=4;
        for(int x=rx;x<rx+16;x++)v[ry*320+x]=0;for(int y=ry;y<ry+16;y++)v[y*320+rx]=0;
        for(int x=rx;x<rx+16;x++)v[(ry+15)*320+x]=12;for(int y=ry;y<ry+16;y++)v[y*320+rx+15]=12;
        for(int i=ry+3;i<ry+13;i++)for(int j=rx+5;j<rx+11;j++)v[i*320+j]=15;
        for(int i=ry+4;i<ry+12;i++)for(int j=rx+6;j<rx+10;j++)v[i*320+j]=4;
        for(int i=ry+2;i<ry+8;i++)for(int j=rx+7;j<rx+9;j++)v[i*320+j]=15;
        need_mr=1;
    }
    if(mb_release && restart_pressed){restart_pressed=0;
        if(mx>=284 && mx<300 && my>=1 && my<17){restart_confirm=1;redraw=1;}else{redraw=1;}}

    if(mb_press&&!wv&&!am&&in_ic(mx,my,20,164)){aip=1;zo=0;icon_hl(v,20,164,"About");need_mr=1;}
    if(mb_release&&aip){aip=0;if(in_ic(mx,my,20,164)){wv=1;am=0;redraw=1;}else{redraw=1;}}
    if(mb_press&&!cv&&!cm&&in_ic(mx,my,60,164)){cip=1;zo=1;icon_hl(v,60,164,"Clock");need_mr=1;}
    if(mb_release&&cip){cip=0;if(in_ic(mx,my,60,164)){cv=1;cm=0;ww2=ww2_def;wh2=wh2_def;redraw=1;}else{redraw=1;}}
    if(mb_press&&!clv&&!clm&&in_ic(mx,my,100,164)){clip=1;zo=2;icon_hl(v,100,164,"Calc");need_mr=1;}
    if(mb_release&&clip){clip=0;if(in_ic(mx,my,100,164)){clv=1;clm=0;wcw=wcw_def;wch=wch_def;calc_init();redraw=1;}else{redraw=1;}}
    if(mb_press&&!cpv&&!cpm&&in_ic(mx,my,140,164)){cpip=1;zo=3;icon_hl(v,140,164,"CPanel");need_mr=1;}
    if(mb_release&&cpip){cpip=0;if(in_ic(mx,my,140,164)){cpv=1;cpm=0;cpw=cpw_def;cph=cph_def;cpanel_init();redraw=1;}else{redraw=1;}}
    if(mb_press&&!fmv&&!fmm&&in_ic(mx,my,180,164)){fip=1;zo=4;icon_hl(v,180,164,"Files");need_mr=1;}
    if(mb_release&&fip){fip=0;if(in_ic(mx,my,180,164)){fmv=1;fmm=0;fmw=fmw_def;fmh=fmh_def;fm_init();redraw=1;}else{redraw=1;}}

    if(mb_press&&in_ta(mx,my)){if(am){am=0;wv=1;wx=asx;wy=asy;}else if(wv){am=1;wv=0;asx=wx;asy=wy;}zo=0;redraw=1;}
    if(mb_press&&in_tc(mx,my)){if(cm){cm=0;cv=1;wx2=csx;wy2=csy;ww2=ww2_def;wh2=wh2_def;}else if(cv){cm=1;cv=0;csx=wx2;csy=wy2;}zo=1;redraw=1;}
    if(mb_press&&in_tcl(mx,my)){if(clm){clm=0;clv=1;wcx=clsx;wcy=clsy;wcw=wcw_def;wch=wch_def;calc_init();}else if(clv){clm=1;clv=0;clsx=wcx;clsy=wcy;}zo=2;redraw=1;}
    if(mb_press&&in_tcp(mx,my)){if(cpm){cpm=0;cpv=1;cpx=cpsx;cpy=cpsy;cpw=cpw_def;cph=cph_def;cpanel_init();}else if(cpv){cpm=1;cpv=0;cpsx=cpx;cpsy=cpy;}zo=3;redraw=1;}
    if(mb_press&&in_tfm(mx,my)){if(fmm){fmm=0;fmv=1;fmx=fmsx;fmy=fmsy;fmw=fmw_def;fmh=fmh_def;fm_init();}else if(fmv){fmm=1;fmv=0;fmsx=fmx;fmsy=fmy;}zo=4;redraw=1;}

    if(mb_press&&!dragging&&!okp&&!clp&&!mip&&!resizing){
        int new_zo=zo;
        if(zo==4&&fmv&&!fmm&&mx>=fmx&&mx<fmx+fmw&&my>=fmy&&my<fmy+fmh)new_zo=4;
        else if(zo==4&&cpv&&!cpm&&mx>=cpx&&mx<cpx+cpw&&my>=cpy&&my<cpy+cph)new_zo=3;
        else if(zo==4&&clv&&!clm&&mx>=wcx&&mx<wcx+wcw&&my>=wcy&&my<wcy+wch)new_zo=2;
        else if(zo==4&&cv&&!cm&&mx>=wx2&&mx<wx2+ww2&&my>=wy2&&my<wy2+wh2)new_zo=1;
        else if(zo==4&&wv&&!am&&mx>=wx&&mx<wx+ww&&my>=wy&&my<wy+wh)new_zo=0;
        else if(zo==3&&fmv&&!fmm&&mx>=fmx&&mx<fmx+fmw&&my>=fmy&&my<fmy+fmh)new_zo=4;
        else if(zo==3&&cpv&&!cpm&&mx>=cpx&&mx<cpx+cpw&&my>=cpy&&my<cpy+cph)new_zo=3;
        else if(zo==3&&clv&&!clm&&mx>=wcx&&mx<wcx+wcw&&my>=wcy&&my<wcy+wch)new_zo=2;
        else if(zo==3&&cv&&!cm&&mx>=wx2&&mx<wx2+ww2&&my>=wy2&&my<wy2+wh2)new_zo=1;
        else if(zo==3&&wv&&!am&&mx>=wx&&mx<wx+ww&&my>=wy&&my<wy+wh)new_zo=0;
        else if(zo==2&&fmv&&!fmm&&mx>=fmx&&mx<fmx+fmw&&my>=fmy&&my<fmy+fmh)new_zo=4;
        else if(zo==2&&cpv&&!cpm&&mx>=cpx&&mx<cpx+cpw&&my>=cpy&&my<cpy+cph)new_zo=3;
        else if(zo==2&&clv&&!clm&&mx>=wcx&&mx<wcx+wcw&&my>=wcy&&my<wcy+wch)new_zo=2;
        else if(zo==2&&cv&&!cm&&mx>=wx2&&mx<wx2+ww2&&my>=wy2&&my<wy2+wh2)new_zo=1;
        else if(zo==2&&wv&&!am&&mx>=wx&&mx<wx+ww&&my>=wy&&my<wy+wh)new_zo=0;
        else if(zo==1&&fmv&&!fmm&&mx>=fmx&&mx<fmx+fmw&&my>=fmy&&my<fmy+fmh)new_zo=4;
        else if(zo==1&&cpv&&!cpm&&mx>=cpx&&mx<cpx+cpw&&my>=cpy&&my<cpy+cph)new_zo=3;
        else if(zo==1&&clv&&!clm&&mx>=wcx&&mx<wcx+wcw&&my>=wcy&&my<wcy+wch)new_zo=2;
        else if(zo==1&&cv&&!cm&&mx>=wx2&&mx<wx2+ww2&&my>=wy2&&my<wy2+wh2)new_zo=1;
        else if(zo==1&&wv&&!am&&mx>=wx&&mx<wx+ww&&my>=wy&&my<wy+wh)new_zo=0;
        else if(zo==0&&fmv&&!fmm&&mx>=fmx&&mx<fmx+fmw&&my>=fmy&&my<fmy+fmh)new_zo=4;
        else if(zo==0&&cpv&&!cpm&&mx>=cpx&&mx<cpx+cpw&&my>=cpy&&my<cpy+cph)new_zo=3;
        else if(zo==0&&clv&&!clm&&mx>=wcx&&mx<wcx+wcw&&my>=wcy&&my<wcy+wch)new_zo=2;
        else if(zo==0&&cv&&!cm&&mx>=wx2&&mx<wx2+ww2&&my>=wy2&&my<wy2+wh2)new_zo=1;
        else if(zo==0&&wv&&!am&&mx>=wx&&mx<wx+ww&&my>=wy&&my<wy+wh)new_zo=0;
        if(new_zo!=zo){zo=new_zo;redraw=1;}
    }

    int bx=wx+ww-42,by=wy+90;
    if(mb_press&&wv&&!am&&in_ok(mx,my,bx,by)){okp=1;okbx=bx;okby=by;ok_btn(v,bx,by,1);need_mr=1;}
    if(mb_release&&okp){okp=0;ok_btn(v,okbx,okby,0);if(in_ok(mx,my,okbx,okby)){wv=0;am=0;redraw=1;}else{redraw=1;}}

    if(mb_press&&!dragging&&!okp&&!clp&&!mip&&!resizing){
        if(wv&&!am){int cx=wx+ww-18,cy=wy+2;if(in_cl(mx,my,cx,cy)){clp=1;clw=1;clcx=cx;clcy=cy;cl_btn(v,cx,cy,1);need_mr=1;}}
        if(!clp&&cv&&!cm){int cx2=wx2+ww2-18,cy2=wy2+2;if(in_cl(mx,my,cx2,cy2)){clp=1;clw=2;clcx=cx2;clcy=cy2;cl_btn(v,cx2,cy2,1);need_mr=1;}}
        if(!clp&&clv&&!clm){int cx3=wcx+wcw-18,cy3=wcy+2;if(in_cl(mx,my,cx3,cy3)){clp=1;clw=3;clcx=cx3;clcy=cy3;cl_btn(v,cx3,cy3,1);need_mr=1;}}
        if(!clp&&cpv&&!cpm){int cx4=cpx+cpw-18,cy4=cpy+2;if(in_cl(mx,my,cx4,cy4)){clp=1;clw=4;clcx=cx4;clcy=cy4;cl_btn(v,cx4,cy4,1);need_mr=1;}}
        if(!clp&&fmv&&!fmm){int cx5=fmx+fmw-18,cy5=fmy+2;if(in_cl(mx,my,cx5,cy5)){clp=1;clw=5;clcx=cx5;clcy=cy5;cl_btn(v,cx5,cy5,1);need_mr=1;}}
    }
    if(mb_release&&clp){cl_btn(v,clcx,clcy,0);if(in_cl(mx,my,clcx,clcy)){
        if(clw==1){wv=0;am=0;}else if(clw==2){cv=0;cm=0;}else if(clw==3){clv=0;clm=0;}else if(clw==4){cpv=0;cpm=0;}else{fmv=0;fmm=0;}redraw=1;}else{redraw=1;}clp=0;clw=0;}

    if(mb_press&&!dragging&&!okp&&!clp&&!mip&&!resizing){
        if(wv&&!am){int mx2=wx+ww-36,my2=wy+2;if(in_mi(mx,my,mx2,my2)){mip=1;miw=1;mi_btn(v,mx2,my2,1);need_mr=1;}}
        if(!mip&&cv&&!cm){int mx3=wx2+ww2-36,my3=wy2+2;if(in_mi(mx,my,mx3,my3)){mip=1;miw=2;mi_btn(v,mx3,my3,1);need_mr=1;}}
        if(!mip&&clv&&!clm){int mx4=wcx+wcw-36,my4=wcy+2;if(in_mi(mx,my,mx4,my4)){mip=1;miw=3;mi_btn(v,mx4,my4,1);need_mr=1;}}
        if(!mip&&cpv&&!cpm){int mx5=cpx+cpw-36,my5=cpy+2;if(in_mi(mx,my,mx5,my5)){mip=1;miw=4;mi_btn(v,mx5,my5,1);need_mr=1;}}
        if(!mip&&fmv&&!fmm){int mx6=fmx+fmw-36,my6=fmy+2;if(in_mi(mx,my,mx6,my6)){mip=1;miw=5;mi_btn(v,mx6,my6,1);need_mr=1;}}
    }
    if(mb_release&&mip){if(miw==1){am=1;wv=0;asx=wx;asy=wy;}else if(miw==2){cm=1;cv=0;csx=wx2;csy=wy2;}
        else if(miw==3){clm=1;clv=0;clsx=wcx;clsy=wcy;}else if(miw==4){cpm=1;cpv=0;cpsx=cpx;cpsy=cpy;}else{fmm=1;fmv=0;fmsx=fmx;fmsy=fmy;}redraw=1;mip=0;}

    if(mb_press&&cv&&!cm&&!dragging&&!resizing){
        int d=rs_dir(mx,my,wx2,wy2,ww2,wh2);
        if(d){resizing=1;rsdir=d;rsox=wx2;rsoy=wy2;rsow=ww2;rsoh=wh2;}
    }
    if(mb&1&&resizing){
        int nx=wx2,ny=wy2,nw=ww2,nh=wh2;
        if(rsdir==1||rsdir==3){nx=mx;nw=rsox+rsow-mx;}if(rsdir==2||rsdir==4){nw=mx-wx2;}
        if(rsdir==1||rsdir==2){ny=my;nh=rsoy+rsoh-my;}if(rsdir==3||rsdir==4){nh=my-wy2;}
        if(nw<60){if(rsdir==1||rsdir==3)nx=rsox+rsow-60;nw=60;}
        if(nh<40){if(rsdir==1||rsdir==2)ny=rsoy+rsoh-40;nh=40;}
        if(nw!=ww2||nh!=wh2||nx!=wx2||ny!=wy2){wx2=nx;wy2=ny;ww2=nw;wh2=nh;redraw=1;}
    }
    if(mb_release&&resizing){resizing=0;redraw=1;}

    if(mb_press&&clv&&!clm&&!clp&&!mip){calc_click(v,mx,my,wcx,wcy,wcw,wch);redraw=1;}
    if(mb_press&&cpv&&!cpm&&!clp&&!mip){cpanel_click(v,mx,my,cpx,cpy,cpw,cph);redraw=1;}
    if(mb_press&&fmv&&!fmm&&!clp&&!mip){fm_click(v,mx,my,fmx,fmy,fmw,fmh);redraw=1;}

    if(mb&1&&!okp&&!clp&&!mip&&!resizing){
        if(!dragging){
            if(wv&&!am&&mx>=wx&&mx<wx+ww&&my>=wy&&my<wy+12){zo=0;dragging=1;dgw=1;dgo=mx-wx;dgoy=my-wy;owx=wx;owy=wy;}
            else if(cv&&!cm&&mx>=wx2&&mx<wx2+ww2&&my>=wy2&&my<wy2+12){zo=1;dragging=1;dgw=2;dgo=mx-wx2;dgoy=my-wy2;owx2=wx2;owy2=wy2;}
            else if(clv&&!clm&&mx>=wcx&&mx<wcx+wcw&&my>=wcy&&my<wcy+12){zo=2;dragging=1;dgw=3;dgo=mx-wcx;dgoy=my-wcy;owcx=wcx;owcy=wcy;}
            else if(cpv&&!cpm&&mx>=cpx&&mx<cpx+cpw&&my>=cpy&&my<cpy+12){zo=3;dragging=1;dgw=4;dgo=mx-cpx;dgoy=my-cpy;ocpx=cpx;ocpy=cpy;}
            else if(fmv&&!fmm&&mx>=fmx&&mx<fmx+fmw&&my>=fmy&&my<fmy+12){zo=4;dragging=1;dgw=5;dgo=mx-fmx;dgoy=my-fmy;ofmx=fmx;ofmy=fmy;}
        }
        if(dragging){
            int nwx=mx-dgo,nwy=my-dgoy;
            int nww=dgw==1?ww:dgw==2?ww2:dgw==3?wcw:dgw==4?cpw:fmw;
            int nwh=dgw==1?wh:dgw==2?wh2:dgw==3?wch:dgw==4?cph:fmh;
            if(nwx<0)nwx=0;if(nwy<0)nwy=0;if(nwx+nww>320)nwx=320-nww;if(nwy+nwh>200)nwy=200-nwh;
            int*cwx=dgw==1?&wx:dgw==2?&wx2:dgw==3?&wcx:dgw==4?&cpx:&fmx;
            int*cwy=dgw==1?&wy:dgw==2?&wy2:dgw==3?&wcy:dgw==4?&cpy:&fmy;
            if(nwx!=*cwx||nwy!=*cwy){
                int ox=dgw==1?owx:dgw==2?owx2:dgw==3?owcx:dgw==4?ocpx:ofmx;
                int oy=dgw==1?owy:dgw==2?owy2:dgw==3?owcy:dgw==4?ocpy:ofmy;
                int ow=dgw==1?ww:dgw==2?ww2:dgw==3?wcw:dgw==4?cpw:fmw;
                int oh=dgw==1?wh:dgw==2?wh2:dgw==3?wch:dgw==4?cph:fmh;
                for(int y2=oy;y2<oy+oh+2&&y2<200;y2++)for(int x2=ox;x2<ox+ow+2&&x2<320;x2++)v[y2*320+x2]=7;
                taskbar(v,am||wv,cm||cv,clm||clv,cpm||cpv,fmm||fmv);
                icon(v,20,164,1,"About");icon(v,60,164,2,"Clock");icon(v,100,164,4,"Calc");icon(v,140,164,6,"CPanel");icon(v,180,164,5,"Files");
                draw_others(v, dgw);box(v,nwx,nwy,nww,nwh);
                if(dgw==1){owx=nwx;owy=nwy;}else if(dgw==2){owx2=nwx;owy2=nwy;}
                else if(dgw==3){owcx=nwx;owcy=nwy;}else if(dgw==4){ocpx=nwx;ocpy=nwy;}else{ofmx=nwx;ofmy=nwy;}
                *cwx=nwx;*cwy=nwy;
            }
        }
    }else{if(dragging){dragging=0;dgw=0;redraw=1;}}
    if(redraw) need_mr=1;
}