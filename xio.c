
/*
    xskat - a card game for 1 to 3 players.
    Copyright (C) 1998  Gunter Gerhardt

    This program is free software; you can redistribute it freely.
    Use it at your own risk; there is NO WARRANTY.
*/

#define XIO_C

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include "defs.h"
#include "skat.h"
#include "bitmaps.h"
#include "xio.h"
#include "irc.h"
#include "text.h"

VOID change_gc(sn,fg,gcp)
int sn;
unsigned long fg;
GC *gcp;
{
  XGCValues gcv;

  gcv.foreground=fg;
  XChangeGC(dpy[sn],gcp[sn],GCForeground,&gcv);
}

VOID change_gcbg(sn,bg,gcp)
int sn;
unsigned long bg;
GC *gcp;
{
  XGCValues gcv;

  gcv.background=bg;
  XChangeGC(dpy[sn],gcp[sn],GCBackground,&gcv);
}

VOID change_gcxor(sn,fg)
int sn;
unsigned long fg;
{
  change_gc(sn,fg^bgpix[sn],gcxor);
}

int istrue(s)
char *s;
{
  char h[5];
  int i;

  for (i=0;i<4 && *s;i++,s++) {
    h[i]=tolower(*s);
  }
  h[i]=0;
  return h[0]=='0'?0:h[0]=='1'?1:!strcmp(h,"true");
}

VOID v_gtextnc(sn,n,c,x,y,w,t)
int sn,n,c,x,y,w;
char *t;
{
  int l;

  l=strlen(t);
  if (n) x+=(w-XTextWidth(dfont[sn],t,l))/2;
  y+=(20*desk[sn].f/desk[sn].q-charh[sn])/2+dfont[sn]->ascent+gfx3d[sn];
  if (c) {
    change_gcbg(sn,btpix[sn],gc);
  }
  XDrawImageString(dpy[sn],win[sn],gc[sn],x,y,t,l);
  XDrawImageString(dpy[sn],bck[sn],gc[sn],x,y,t,l);
  if (c) {
    change_gcbg(sn,bgpix[sn],gc);
  }
}

VOID v_gtextc(sn,c,x,y,w,t)
int sn,c,x,y,w;
char *t;
{
  v_gtextnc(sn,1,c,x,y,w,t);
}

VOID v_gtext(sn,x,y,w,t)
int sn,x,y,w;
char *t;
{
  v_gtextc(sn,0,x,y,w,t);
}

VOID clr_text(sn,x,y)
int sn,x,y;
{
  x+=4;
  y++;
  change_gc(sn,btpix[sn],gc);
  XFillRectangle(dpy[sn],win[sn],gc[sn],x,y,
		 64*desk[sn].f/desk[sn].q-8,18*desk[sn].f/desk[sn].q-2);
  XFillRectangle(dpy[sn],bck[sn],gc[sn],x,y,
		 64*desk[sn].f/desk[sn].q-8,18*desk[sn].f/desk[sn].q-2);
  change_gc(sn,fgpix[sn],gc);
}

VOID b_text(s,str)
int s;
char *str;
{
  int sn,x;

  for (sn=0;sn<numsp;sn++) {
    if (sn!=s) {
      x=s==left(sn)?desk[sn].cbox1x:desk[sn].cbox2x;
      clr_text(sn,x,desk[sn].cboxy);
      v_gtextc(sn,1,x,desk[sn].cboxy,desk[sn].cardw,str);
    }
  }
}

VOID do_msaho(sn,str)
int sn;
char *str;
{
  clr_text(sn,desk[sn].pboxx,desk[sn].pboxy);
  v_gtextc(sn,1,desk[sn].pboxx,desk[sn].pboxy,desk[sn].cardw,str);
  clr_text(sn,desk[sn].pboxx+desk[sn].cardw,desk[sn].pboxy);
  v_gtextc(sn,1,desk[sn].pboxx+desk[sn].cardw,desk[sn].pboxy,desk[sn].cardw,
	   textarr[TX_PASSE]);
}

VOID draw_skat()
{
  int sn=spieler;

  putcard(sn,cards[30],desk[sn].skatx,desk[sn].skaty);
  putcard(sn,cards[31],desk[sn].skatx+desk[sn].cardw,
	  desk[sn].skaty);
}

VOID home_skat()
{
  int sn=spieler;

  homecard(sn,0,0);
  homecard(sn,0,1);
}

VOID nimm_stich()
{
  int sn=ausspl,i;

  for (i=0;i<3;i++) {
    homecard(sn,1,i);
  }
}

VOID drop_card(i,s)
int i,s;
{
  int sn,sna[3],x1[3],y1[3],x2[3],y2[3];

  for (sn=0;sn<numsp;sn++) {
    sna[sn]=sn;
    if (s==left(sn)) {
      x1[sn]=desk[sn].com1x;
      y1[sn]=desk[sn].com1y;
    }
    else {
      x1[sn]=desk[sn].com2x;
      y1[sn]=desk[sn].com2y;
    }
    if (sn==s || (ouveang && s==spieler)) {
      x1[sn]=desk[sn].playx+(i%10)*desk[sn].cardx;
      if (sn==s) y1[sn]=desk[sn].playy;
      putdesk(sn,x1[sn],y1[sn]);
    }
    else if (stich==10) {
      putdesk(sn,x1[sn],y1[sn]);
    }
    else if (spitzeang && cards[i]==(trumpf==4?BUBE:SIEBEN|trumpf<<3)) {
      putback(sn,x1[sn],y1[sn]);
      sptzmrk=1;
      putamark(sn,spieler);
    }
    x2[sn]=desk[sn].stichx+vmh*desk[sn].cardw;
    y2[sn]=desk[sn].stichy;
  }
  movecard(numsp,sna,x1,y1,x2,y2);
  for (sn=0;sn<numsp;sn++) {
    putcard(sn,cards[i],desk[sn].stichx+vmh*desk[sn].cardw,desk[sn].stichy);
  }
  stcd[vmh]=cards[i];
  gespcd[cards[i]]=2;
  if ((cards[i]&7)!=BUBE) gespfb[cards[i]>>3]++;
  cards[i]=-1;
  stdwait();
}

VOID create_colcards(sn,cwidth,cheight,cpixs,cbits,bwpix,pm)
int sn,cwidth,cheight;
char *cpixs;
unsigned char *cbits;
Pixmap bwpix,*pm;
{
  Pixmap pmc;
  int i,j,pl,m0,m1,m2,m3,of,ix;

  *pm=XCreatePixmap(dpy[sn],win[sn],cwidth,cheight/4,desk[sn].plan);
  if (*pm==None) return;
  XFillRectangle(dpy[sn],*pm,gcbck[sn],0,0,cwidth,cheight/4);
  change_gcxor(sn,color[sn][4].pixel);
  XCopyPlane(dpy[sn],bwpix,*pm,gcxor[sn],0,0,desk[sn].cardw,desk[sn].cardh,
	     0,0,1);
  XCopyPlane(dpy[sn],bwpix,*pm,gcxor[sn],0,0,desk[sn].cardw,desk[sn].cardh,
	     desk[sn].cardw,0,1);
  XCopyPlane(dpy[sn],bwpix,*pm,gcxor[sn],0,0,desk[sn].cardw,desk[sn].cardh,
	     2*desk[sn].cardw,0,1);
  XCopyPlane(dpy[sn],bwpix,*pm,gcxor[sn],0,0,desk[sn].cardw,desk[sn].cardh,
	     3*desk[sn].cardw,0,1);
  of=cheight/4*cwidth/8;
  for (pl=0;pl<16;pl++) {
    m0=pl&1?0:0xff;
    m1=pl&2?0:0xff;
    m2=pl&4?0:0xff;
    m3=pl&8?0:0xff;
    for (i=0;i<cheight/4;i++) {
      for (j=0;j<cwidth/8;j++) {
	ix=i*cwidth/8+j;
	cpixs[ix]=
	  (cbits[ix]^m0)&
	  (cbits[ix+of]^m1)&
	  (cbits[ix+of+of]^m2)&
	  (cbits[ix+of+of+of]^m3);
      }
    }
    pmc=XCreateBitmapFromData(dpy[sn],win[sn],cpixs,
			      cwidth,cheight/4);
    if (pmc==None) {
      *pm=None;
      return;
    }
    change_gcxor(sn,color[sn][pl+4].pixel^color[sn][4].pixel^bgpix[sn]);
    XCopyPlane(dpy[sn],pmc,*pm,gcxor[sn],
	       0,0,cwidth,cheight/4,0,0,1);
    XFreePixmap(dpy[sn],pmc);
  }
  change_gcxor(sn,fgpix[sn]);
}

VOID alloc_colors(sn)
int sn;
{
  int i;

  for (i=0;i<20 && XAllocColor(dpy[sn],cmap[sn],&color[sn][i]);i++);
  desk[sn].col=i+2;
  for (;i<20;i++) {
    color[sn][i].pixel=bpix[sn];
  }
}

unsigned long get_col(sn,ucol,prog,col,defcol,defpix,xcol)
int sn;
char *ucol,*prog,*col,*defcol;
unsigned long defpix;
XColor *xcol;
{
  char *spec;

  if (ucol && XParseColor(dpy[sn],cmap[sn],ucol,xcol) &&
      XAllocColor(dpy[sn],cmap[sn],xcol)) return xcol->pixel;
  spec=XGetDefault(dpy[sn],prog,col);
  if (spec && XParseColor(dpy[sn],cmap[sn],spec,xcol) &&
      XAllocColor(dpy[sn],cmap[sn],xcol)) return xcol->pixel;
  if (defcol && XParseColor(dpy[sn],cmap[sn],defcol,xcol) &&
      XAllocColor(dpy[sn],cmap[sn],xcol)) return xcol->pixel;
  xcol->pixel=defpix;
  XQueryColor(dpy[sn],cmap[sn],xcol);
  return defpix;
}

VOID calc_desk(sn)
int sn;
{
  desk[sn].x=0;
  desk[sn].y=0;
  desk[sn].cardw=desk[sn].large?96:64;
  desk[sn].cardh=desk[sn].large?141:94;
  desk[sn].f=desk[sn].large?3:1;
  desk[sn].q=desk[sn].large?2:1;
  desk[sn].w=10*desk[sn].cardw;
  desk[sn].h=desk[sn].large?600:400;
  desk[sn].com1x=desk[sn].x+desk[sn].cardw;
  desk[sn].com2x=desk[sn].x+desk[sn].w-2*desk[sn].cardw;
  desk[sn].com1y=desk[sn].com2y=desk[sn].y+3;
  if (ouveang && sn!=spieler) {
    if (spieler==left(sn)) {
      desk[sn].com2y+=desk[sn].cardh+11*desk[sn].f/desk[sn].q;
    }
    else {
      desk[sn].com1y+=desk[sn].cardh+11*desk[sn].f/desk[sn].q;
    }
  }
  desk[sn].cardx=(desk[sn].w-desk[sn].cardw)/9;
  desk[sn].playx=desk[sn].x+(desk[sn].w-desk[sn].cardw-9*desk[sn].cardx)/2;
  desk[sn].playy=desk[sn].y+desk[sn].h-desk[sn].cardh-2;
  desk[sn].skatx=desk[sn].x+(desk[sn].w-2*desk[sn].cardw)/2;
  desk[sn].skaty=desk[sn].y+desk[sn].cardh+51*desk[sn].f/desk[sn].q;
  desk[sn].stichx=desk[sn].x+(desk[sn].w-3*desk[sn].cardw)/2;
  desk[sn].stichy=desk[sn].skaty;
  desk[sn].cbox1x=desk[sn].com1x;
  desk[sn].cbox2x=desk[sn].com2x;
  desk[sn].cboxy=desk[sn].com1y+desk[sn].cardh+11*desk[sn].f/desk[sn].q;
  desk[sn].pboxx=desk[sn].skatx;
  desk[sn].pboxy=desk[sn].skaty+desk[sn].cardh+13*desk[sn].f/desk[sn].q;
}

VOID extractnam(sn,str)
int sn;
char *str;
{
  char *eos;
  int z,s;

  spnames[sn][0][0]=0;
  spnames[sn][1][0]=0;
  if (!str) str="";
  if (!(eos=strchr(str,'@')) && !(eos=strchr(str,':'))) eos=str+strlen(str);
  for (z=0;z<2 && str!=eos;z++) {
    while (*str==' ' || *str=='-') str++;
    for (s=0;s<9 && str!=eos && *str!=' ' && *str!='-';s++,str++) {
      spnames[sn][z][s]=*str=='~'?'-':*str;
    }
    spnames[sn][z][s]=0;
  }
}

VOID usage()
{
  fprintf(stderr,"\
xskat [-display|-d display] [-geometry|-g geometry] [-font|-fn font]\n\
  [-iconic|-i] [-title|-T string] [-name prog] [-fg color] [-bg color]\n\
  [-bt color] [-3d] [-2d] [-3dtop color] [-3dbot color] [-mark color]\n\
  [-mb button] [-tdelay sec] [-fastdeal] [-slowdeal] [-help|-h]\n\
  [-color] [-mono] [-color1 color] .. [-color20 color] [-large] [-small]\n\
  [-up] [-down] [-alt] [-seq] [-list|-l filename] [-alist] [-nlist] [-tlist]\n\
  [-log filename] [-dolog] [-nolog] [-fmt] [-unfmt] [-game filename]\n\
  [-lang language] [-start player#] [-s1 -4..4] [-s2 -4..4] [-s3 -4..4]\n\
  [-ramsch] [-noramsch] [-ramschonly] [-sramsch] [-nosramsch]\n\
  [-kontra] [-nokontra] [-kontra18] [-bock] [-nobock] [-bockramsch]\n\
  [-bockevents 1..255] [-resumebock] [-noresumebock]\n\
  [-spitze] [-spitze2] [-nospitze] [-revolution] [-norevolution]\n\
  [-klopfen] [-noklopfen] [-schenken] [-noschenken] [-hint] [-nohint]\n\
  [-irc] [-noirc] [-irctelnet program] [-ircserver host]\n\
  [-ircport number] [-ircchannel name] [-ircnick name] [-ircuser name]\n\
  [-ircrealname name] [-ircpos number] [-irclog file]\n\
  [-auto #ofgames] [-opt filename] [player@display...]\n\
After starting the game a mouse click will bring up a menu.\n\
");
}

VOID invopt(opt)
char *opt;
{
  fprintf(stderr,"Invalid option %s\n",opt);
  usage();
  exitus(1);
}

VOID nomem()
{
  fprintf(stderr,"Out of memory\n");
  exitus(1);
}

VOID finish(sn,ex)
int sn,ex;
{
  int s;

  quit=1;
  for (s=0;s<numsp;s++) {
    di_term(s,sn);
  }
  for (s=numsp;s<3;s++) {
    lost[s]=1;
  }
  while (!lost[0] || !lost[1] || !lost[2]) {
    hndl_events();
  }
  if (ex) exitus(1);
}

int ioerr(d)
Display *d;
{
  int sn,es;

  for (sn=0;sn<numsp;sn++) {
    if (dpy[sn]==d) lost[es=sn]=1;
  }
  if (irc_play) exitus(0);
  finish(es,1);
  return 0;
}

VOID exitus(n)
int n;
{
  if (irc_telnetpid) {
    kill(irc_telnetpid,SIGHUP);
  }
  exit(n);
}

int getdeffn(prog_name,pfn,res,suf)
char *prog_name,**pfn,*res,*suf;
{
  char *home,*fn,*rfn;
  int r;

  home=getenv("HOME");
  fn=XGetDefault(dpy[0],prog_name,res);
  if (fn && !strncmp(fn,"~/",2)) {
    if (home && (rfn=malloc(strlen(home)+strlen(fn)))) {
      strcpy(rfn,home);
      strcat(rfn,fn+1);
      fn=rfn;
    }
  }
  r=1;
  if (!fn) {
    r=0;
    if (home && (fn=malloc(strlen(home)+3+strlen(suf)))) {
      strcpy(fn,home);
      strcat(fn,"/.");
      strcat(fn,suf);
    }
    else {
      fn=suf;
    }
  }
  *pfn=fn;
  return r;
}

VOID logit()
{
#ifdef LOGDIR
  FILE *f;
  char logf[1000];
  struct passwd *pwd;
  struct utsname uts;

  sprintf(logf,"%s/%lu",LOGDIR,(unsigned long)getuid());
  f=fopen(logf,"w");
  if (f) {
    if (pwd=getpwuid(getuid())) {
      if (pwd->pw_name) fputs(pwd->pw_name,f);
      if (pwd->pw_gecos) fprintf(f,"(%s)",pwd->pw_gecos);
    }
    if (!uname(&uts)) {
      fprintf(f,"@%s",uts.nodename);
#ifndef __hpux
      fprintf(f,".%s",uts.domainname);
#endif
      fprintf(f,"(%s",uts.sysname);
      fprintf(f,",%s)",uts.machine);
    }
    fputc('\n',f);
    fclose(f);
  }
#endif
}

VOID xinitwin(sn)
int sn;
{
  Pixmap icon;
  XClassHint classhint;
  XGCValues gcv;
  int gcvf;

  icon=XCreateBitmapFromData(dpy[sn],DefaultRootWindow(dpy[sn]),icon_bits,
			     icon_width,icon_height);
  if (icon==None) nomem();
  win[sn]=XCreateSimpleWindow(dpy[sn],DefaultRootWindow(dpy[sn]),
			      desk[sn].x,desk[sn].y,desk[sn].w,desk[sn].h,
			      0,fgpix[sn],bgpix[sn]);
  classhint.res_name=prog_name;
  classhint.res_class=prog_name;
  XSetClassHint(dpy[sn],win[sn],&classhint);
  XSetStandardProperties(dpy[sn],win[sn],title,title,icon,
			 (char **)0,0,&szhints);
  wmhints.flags|=IconPixmapHint;
  wmhints.icon_pixmap=icon;
  XSetWMHints(dpy[sn],win[sn],&wmhints);
  cursor[sn][0]=XCreateFontCursor(dpy[sn],XC_hand2);
  cursor[sn][1]=XCreateFontCursor(dpy[sn],XC_watch);
  XDefineCursor(dpy[sn],win[sn],cursor[sn][1]);
  gcvf=GCGraphicsExposures|GCForeground|GCBackground;
  gcv.graphics_exposures=False;
  gcv.font=dfont[sn]->fid;
  gcv.foreground=fgpix[sn];
  gcv.background=bgpix[sn];
  gc[sn]=XCreateGC(dpy[sn],win[sn],gcvf|GCFont,&gcv);
  gcv.foreground=bgpix[sn];
  gcv.background=fgpix[sn];
  gcbck[sn]=XCreateGC(dpy[sn],win[sn],gcvf,&gcv);
  gcv.foreground=fgpix[sn]^bgpix[sn];
  gcv.background=0;
  gcv.function=GXxor;
  gcxor[sn]=XCreateGC(dpy[sn],win[sn],gcvf|GCFunction,&gcv);
  if (desk[sn].large) {
    bwcards[sn]=XCreateBitmapFromData(dpy[sn],win[sn],bwcardl_bits,
				      bwcardl_width,bwcardl_height);
  }
  else {
    bwcards[sn]=XCreateBitmapFromData(dpy[sn],win[sn],bwcards_bits,
				      bwcards_width,bwcards_height);
  }
  if (bwcards[sn]==None) nomem();
  if (desk[sn].col>2) {
    alloc_colors(sn);
    if (desk[sn].large) {
      create_colcards(sn,colcardl_width,colcardl_height,
		      colcardl_pixs,colcardl_bits,bwcards[sn],
		      &colcards[sn]);
    }
    else {
      create_colcards(sn,colcards_width,colcards_height,
		      colcards_pixs,colcards_bits,bwcards[sn],
		      &colcards[sn]);
    }
    if (colcards[sn]==None) nomem();
  }
  bck[sn]=XCreatePixmap(dpy[sn],win[sn],desk[sn].w,desk[sn].h,desk[sn].plan);
  if (bck[sn]==None) nomem();
  XFillRectangle(dpy[sn],win[sn],gcbck[sn],0,0,desk[sn].w,desk[sn].h);
  XFillRectangle(dpy[sn],bck[sn],gcbck[sn],0,0,desk[sn].w,desk[sn].h);
  if (!irc_play || irc_pos==sn) {
    XSelectInput(dpy[sn],win[sn],ButtonPressMask|ExposureMask|KeyPressMask);
    XMapWindow(dpy[sn],win[sn]);
  }
}


VOID xinitplayers()
{
  int sn;

  for (sn=1;sn<3;sn++) {
    disp_name[sn]=disp_name[0];
    dpy[sn]=dpy[0];
    cmap[sn]=cmap[0];
    desk[sn]=desk[0];
    bpix[sn]=bpix[0];
    wpix[sn]=wpix[0];
    bgpix[sn]=bgpix[0];
    fgpix[sn]=fgpix[0];
    btpix[sn]=btpix[0];
    mkpix[sn]=mkpix[0];
    w3dpix[sn]=w3dpix[0];
    b3dpix[sn]=b3dpix[0];
    gfx3d[sn]=gfx3d[0];
    memcpy((VOID *)color[sn],(VOID *)color[0],sizeof(color[0]));
    dfont[sn]=dfont[0];
    charw[sn]=charw[0];
    charh[sn]=charh[0];
    mbutton[sn]=mbutton[0];
    hints[sn]=hints[0];
  }
}

VOID xinitres(sn)
int sn;
{
  static char stgs[]="s1";
  char cbuf[8];
  char *res;
  int logdef;
  int scr;
  XColor fgcol,nocol;
  unsigned long borw;
  int i,len,cw,nw;
  struct passwd *pwd;

  if (sn) {
    font_name=title=fg_col=bg_col=bt_col=0;
    w3d_col=b3d_col=mk_col=0;
    bwcol=downup=altseq=alist[sn]=-1;
    extractnam(sn,disp_name[sn]);
    if (strchr(disp_name[sn],'@')) {
      disp_name[sn]=strchr(disp_name[sn],'@')+1;
    }
  }
  else {
    extractnam(sn,getenv("LOGNAME"));
    memcpy((VOID *)color[1],(VOID *)color[0],sizeof(color[0]));
    memcpy((VOID *)color[2],(VOID *)color[0],sizeof(color[0]));
  }
  if (!(dpy[sn]=XOpenDisplay(disp_name[sn]))) {
    fprintf(stderr,"Can't open display %s\n",XDisplayName(disp_name[sn]));
    exitus(1);
  }
  XSetIOErrorHandler(ioerr);
  scr=DefaultScreen(dpy[sn]);
  cmap[sn]=DefaultColormap(dpy[sn],scr);
  desk[sn].plan=DefaultDepth(dpy[sn],scr);
  bpix[sn]=BlackPixel(dpy[sn],scr);
  wpix[sn]=WhitePixel(dpy[sn],scr);
  fgpix[sn]=get_col(sn,fg_col,prog_name,"foreground",NULL,bpix[sn],&fgcol);
  borw=(long)fgcol.red+fgcol.green+fgcol.blue<0x1E000L?wpix[sn]:bpix[sn];
  if (gfx3d[sn]<0) {
    res=XGetDefault(dpy[sn],prog_name,"3d");
    gfx3d[sn]=(!res && desk[sn].plan>1) || (res && istrue(res));
  }
  if (gfx3d[sn]) {
    bgpix[sn]=get_col(sn,bg_col,prog_name,"3dbackground",
		      desk[sn].plan>1?"#cccccccccccc":
		      borw==wpix[sn]?"white":"black",
		      borw,&nocol);
    btpix[sn]=get_col(sn,bt_col,prog_name,"3dbutton",
		      desk[sn].plan>1?"#dddddddddddd":
		      borw==wpix[sn]?"white":"black",
		      borw,&nocol);
    w3dpix[sn]=get_col(sn,w3d_col,prog_name,"3dtop",NULL,wpix[sn],&nocol);
    b3dpix[sn]=get_col(sn,b3d_col,prog_name,"3dbot",NULL,bpix[sn],&nocol);
  }
  else {
    bgpix[sn]=get_col(sn,bg_col,prog_name,"background",NULL,borw,&nocol);
    btpix[sn]=get_col(sn,bt_col,prog_name,"button",NULL,borw,&nocol);
  }
  mkpix[sn]=get_col(sn,mk_col,prog_name,"mark",
		    desk[sn].plan>1?"#ffff00000000":"black",
		    bpix[sn],&nocol);
  for (i=0;i<20;i++) {
    if (sn || !ccol[i] ||
	!XParseColor(dpy[sn],cmap[sn],ccol[i],&color[sn][i])) {
      sprintf(cbuf,"color%d",i+1);
      res=XGetDefault(dpy[sn],prog_name,cbuf);
      if (res) {
	XParseColor(dpy[sn],cmap[sn],res,&color[sn][i]);
      }
    }
  }
  if (desk[sn].large<0) {
    res=XGetDefault(dpy[sn],prog_name,"large");
    if (res) desk[sn].large=istrue(res);
    if (desk[sn].large<0) {
      desk[sn].large=XDisplayWidth(dpy[sn],scr)>960 &&
	XDisplayHeight(dpy[sn],scr)>600;
    }
  }
  calc_desk(sn);
  if (!title &&
      !(title=XGetDefault(dpy[sn],prog_name,"title"))) title=prog_name;
  if (!font_name &&
      !(font_name=XGetDefault(dpy[sn],prog_name,"font"))) {
    font_name=desk[sn].large?"10x20":"9x15";
  }
  if (!(dfont[sn]=XLoadQueryFont(dpy[sn],font_name))) {
    fprintf(stderr,"Font %s not found\n",font_name);
    exitus(1);
  }
  charw[sn]=dfont[sn]->max_bounds.width;
  charh[sn]=dfont[sn]->max_bounds.ascent+dfont[sn]->max_bounds.descent+
    gfx3d[sn]+1;
  if (!sn && !opt_file) {
    getdeffn(prog_name,&opt_file,"opt","xskat.opt");
  }
  if (opt_file && !*opt_file) opt_file=0;
  if (!sn && !prot_file) {
    logdef=getdeffn(prog_name,&prot_file,"log","xskat.log");
  }
  if (prot_file && !*prot_file) prot_file=0;
  if (!sn && logging<0) {
    res=XGetDefault(dpy[sn],prog_name,"dolog");
    logging=(res && istrue(res)) || (!res && logdef);
  }
  if (!sn && unformatted<0) {
    res=XGetDefault(dpy[sn],prog_name,"formatted");
    unformatted=!(res && istrue(res));
  }
  if (sn) {
    res=XGetDefault(dpy[sn],prog_name,"useoptfile");
    useoptfile[sn]=res && istrue(res);
  }
  else {
    useoptfile[sn]=1;
  }
  if (!sn && !game_file) {
    if (!getdeffn(prog_name,&game_file,"game","")) {
      game_file=0;
    }
  }
  if (game_file && !*game_file) game_file=0;
  if (!mbuttonset[sn]) {
    res=XGetDefault(dpy[sn],prog_name,"menubutton");
    if (res) {
      mbutton[sn]=atoi(res);
    }
  }
  if (mbutton[sn]<0 || mbutton[sn]>5) mbutton[sn]=0;
  res=XGetDefault(dpy[sn],prog_name,"tdelay");
  if (res && !tdelayset[sn]) {
    nimmstich[sn][0]=(int)(atof(res)*10+.5);
  }
  if (nimmstich[sn][0]<0) nimmstich[sn][0]=7;
  else if (nimmstich[sn][0]>101) nimmstich[sn][0]=101;
  if (bwcol<0) {
    res=XGetDefault(dpy[sn],prog_name,"color");
    if (res) bwcol=istrue(res);
    else bwcol=(DefaultVisual(dpy[sn],scr)->class!=StaticGray &&
		DefaultVisual(dpy[sn],scr)->class!=GrayScale);
  }
  desk[sn].col=bwcol?3:2;
  if (downup<0) {
    res=XGetDefault(dpy[sn],prog_name,"down");
    if (res) {
      downup=istrue(res);
    }
  }
  sort1[sn]=!downup;
  if (altseq<0) {
    res=XGetDefault(dpy[sn],prog_name,"alt");
    if (res) {
      altseq=istrue(res);
    }
  }
  alternate[sn]=!!altseq;
  if (alist[sn]<0) {
    res=XGetDefault(dpy[sn],prog_name,"alist");
    if (res) {
      alist[sn]=istrue(res);
    }
    else {
      res=XGetDefault(dpy[sn],prog_name,"tlist");
      alist[sn]=res && istrue(res)?2:0;
    }
  }
  if ((res=XGetDefault(dpy[sn],prog_name,"alias"))) {
    extractnam(sn,res);
  }
  if (!sn) {
    if (lang<0) {
      lang=langidx(XGetDefault(dpy[sn],prog_name,"language"),1);
    }
    init_text();
  }
  if (!sn && !ramschset && (res=XGetDefault(dpy[sn],prog_name,"ramsch"))) {
    playramsch=atoi(res);
    if (playramsch<0) playramsch=0;
    else if (playramsch>2) playramsch=2;
  }
  if (!sn && !sramschset && (res=XGetDefault(dpy[sn],prog_name,"sramsch"))) {
    playsramsch=istrue(res);
  }
  if (!sn && !kontraset && (res=XGetDefault(dpy[sn],prog_name,"kontra"))) {
    playkontra=atoi(res);
    if (playkontra<0) playkontra=0;
    else if (playkontra>2) playkontra=2;
  }
  if (!sn && !bockset && (res=XGetDefault(dpy[sn],prog_name,"bock"))) {
    playbock=atoi(res);
    if (playbock<0) playbock=0;
    else if (playbock>2) playbock=2;
  }
  if (!sn && !bockeventsset &&
      (res=XGetDefault(dpy[sn],prog_name,"bockevents"))) {
    bockevents=atoi(res);
  }
  if (!sn && !resumebockset &&
      (res=XGetDefault(dpy[sn],prog_name,"resumebock"))) {
    resumebock=istrue(res);
  }
  if (!sn && !spitzezaehltset &&
      (res=XGetDefault(dpy[sn],prog_name,"spitze"))) {
    spitzezaehlt=atoi(res);
    if (spitzezaehlt<0) spitzezaehlt=0;
    else if (spitzezaehlt>2) spitzezaehlt=2;
  }
  if (!sn && !revolutionset &&
      (res=XGetDefault(dpy[sn],prog_name,"revolution"))) {
    revolution=istrue(res);
  }
  if (!sn && !klopfenset &&
      (res=XGetDefault(dpy[sn],prog_name,"klopfen"))) {
    klopfen=istrue(res);
  }
  if (!sn && !schenkenset &&
      (res=XGetDefault(dpy[sn],prog_name,"schenken"))) {
    schenken=istrue(res);
  }
  if (!hintsset[sn] &&
      (res=XGetDefault(dpy[sn],prog_name,"hint"))) {
    hints[sn]=istrue(res);
  }
  if (hints[sn]<0) hints[sn]=0;
  if (!sn) {
    if (irc_play<0 &&
	(res=XGetDefault(dpy[sn],prog_name,"irc"))) {
      irc_play=istrue(res);
    }
    if (irc_play<0) irc_play=0;
    if (!list_file) {
      getdeffn(prog_name,&list_file,
	       irc_play?"irclist":"list",
	       irc_play?"xskat.irc":"xskat.lst");
    }
    if (list_file && !*list_file) list_file=0;
    if (irc_play) {
      game_file=0;
      if (irc_pos<0 &&
	  (res=XGetDefault(dpy[sn],prog_name,"ircpos"))) {
	irc_pos=atoi(res);
      }
      irc_pos--;
      if (irc_pos<0 || irc_pos>2) irc_pos=-1;
      if (!irc_telnet &&
	  !(irc_telnet=XGetDefault(dpy[sn],prog_name,"irctelnet"))) {
	irc_telnet="telnet";
      }
      if (!irc_host &&
	  !(irc_host=XGetDefault(dpy[sn],prog_name,"ircserver")) &&
	  !(irc_host=getenv("IRCSERVER"))) {
	irc_host=irc_defaulthost;
      }
      if (irc_port<0 &&
	  ((res=XGetDefault(dpy[sn],prog_name,"ircport")) ||
	   (res=getenv("IRCPORT")))) {
	irc_port=atoi(res);
      }
      if (irc_port<0) irc_port=6667;
      if (!irc_channel &&
	  !(irc_channel=XGetDefault(dpy[sn],prog_name,"ircchannel"))) {
	irc_channel="#xskat";
      }
      if (!irc_nick &&
	  !(irc_nick=XGetDefault(dpy[sn],prog_name,"ircnick")) &&
	  !(irc_nick=getenv("IRCNICK")) &&
	  !(irc_nick=getenv("IRCUSER")) &&
	  !(irc_nick=getenv("LOGNAME"))) {
	irc_changenick(0);
      }
      if (!irc_user &&
	  !(irc_user=XGetDefault(dpy[sn],prog_name,"ircuser")) &&
	  !(irc_user=getenv("IRCUSER")) &&
	  !(irc_user=getenv("LOGNAME"))) {
	irc_user="XSkat";
      }
      if (!irc_realname &&
	  !(irc_realname=XGetDefault(dpy[sn],prog_name,"ircrealname")) &&
	  !(irc_realname=getenv("IRCNAME"))) {
	if ((pwd=getpwuid(getuid())) && pwd->pw_gecos) {
	  irc_realname=pwd->pw_gecos;
	  if ((res=strchr(irc_realname,','))) *res=0;
	}
	if (!irc_realname || !*irc_realname) {
	  irc_realname="XSkat player";
	}
      }
      if (!irc_logfile) {
	getdeffn(prog_name,&irc_logfile,"irclog","xskat.ilg");
      }
      if (irc_logfile && !*irc_logfile) irc_logfile=0;
    }
  }
  if (!sn && geber<0) {
    if ((res=XGetDefault(dpy[sn],prog_name,"start"))) {
      geber=atoi(res);
    }
    if (geber<1 || geber>3) geber=0;
    else geber=left(geber);
  }
  if (!sn) {
    for (i=0;i<2;i++) {
      if (!stgset[i]) {
	stgs[1]=i+'1';
	if ((res=XGetDefault(dpy[sn],prog_name,stgs))) {
	  strateg[i]=atoi(res);
	}
      }
      if (strateg[i]<-4) strateg[i]=-4;
      else if (strateg[i]>4) strateg[i]=4;
    }
  }
  if ((geom_f&(XValue|YValue))==(XValue|YValue)) {
    szhints.x=geom_f&XNegative?
      XDisplayWidth(dpy[sn],scr)-geom_x-desk[sn].w:geom_x;
    szhints.y=geom_f&YNegative?
      XDisplayHeight(dpy[sn],scr)-geom_y-desk[sn].h:geom_y;
    szhints.flags|=USPosition;
  }
  szhints.flags|=PMinSize|PMaxSize;
  szhints.min_width=szhints.max_width=desk[sn].w;
  szhints.min_height=szhints.max_height=desk[sn].h;
  cw=1;
  for (i=0;i<TX_LAST;i++) {
    if ((len=strlen(textarr[i]))>3 &&
	(nw=(XTextWidth(dfont[sn],textarr[i],len)+len-1)/len)>cw) {
      cw=nw;
    }
  }
  if (cw<charw[sn]) charw[sn]=cw;
  if (!spnames[sn][0][0]) {
    sprintf(spnames[sn][0],textarr[TX_SPIELER],sn+1);
  }
}

VOID xinit(argc,argv)
int argc;
char *argv[];
{
  int sn,i;
  unsigned int w,h;

  logit();
  signal(SIGPIPE,SIG_IGN);
  signal(SIGHUP,exitus);
  signal(SIGINT,exitus);
  signal(SIGTERM,exitus);
  numsp=1;
  disp_name[0]=font_name=title=fg_col=bg_col=bt_col=0;
  w3d_col=b3d_col=mk_col=0;
  for (i=0;i<20;i++) ccol[i]=0;
  geom_f=0;
  bwcol=downup=altseq=lang=geber=logging=unformatted=alist[0]=-1;
  gfx3d[0]=gfx3d[1]=gfx3d[2]=-1;
  hints[0]=hints[1]=hints[2]=-1;
  irc_play=irc_pos=irc_port=-1;
  irc_telnet=irc_host=irc_channel=irc_nick=0;
  irc_user=irc_realname=irc_logfile=0;
  alternate[0]=alternate[1]=alternate[2]=1;
  nimmstich[0][0]=nimmstich[1][0]=nimmstich[2][0]=7;
  desk[0].large=desk[1].large=desk[2].large=-1;
  prog_name=strrchr(argv[0],'/');
  if (prog_name) prog_name++;
  else prog_name=argv[0];
  wmhints.flags=0;
  szhints.flags=0;
  while (argc>1) {
    if (!strcmp(argv[1],"-help") || !strcmp(argv[1],"-h")) {
      usage();
      exitus(0);
    }
    else if (!strcmp(argv[1],"-color")) {
      bwcol=1;
    }
    else if (!strcmp(argv[1],"-mono")) {
      bwcol=0;
    }
    else if (!strcmp(argv[1],"-3d")) {
      gfx3d[0]=1;
    }
    else if (!strcmp(argv[1],"-2d")) {
      gfx3d[0]=0;
    }
    else if (!strcmp(argv[1],"-iconic") || !strcmp(argv[1],"-i")) {
      wmhints.flags|=StateHint;
      wmhints.initial_state=IconicState;
    }
    else if (!strcmp(argv[1],"-down")) {
      downup=1;
      downupset[0]=1;
    }
    else if (!strcmp(argv[1],"-up")) {
      downup=0;
      downupset[0]=1;
    }
    else if (!strcmp(argv[1],"-alt")) {
      altseq=1;
      altseqset[0]=1;
    }
    else if (!strcmp(argv[1],"-seq")) {
      altseq=0;
      altseqset[0]=1;
    }
    else if (!strcmp(argv[1],"-dolog")) {
      logging=1;
    }
    else if (!strcmp(argv[1],"-nolog")) {
      logging=0;
    }
    else if (!strcmp(argv[1],"-fmt")) {
      unformatted=0;
    }
    else if (!strcmp(argv[1],"-unfmt")) {
      unformatted=1;
    }
    else if (!strcmp(argv[1],"-large")) {
      desk[0].large=1;
    }
    else if (!strcmp(argv[1],"-small")) {
      desk[0].large=0;
    }
    else if (!strcmp(argv[1],"-alist")) {
      alist[0]=1;
      alistset[0]=1;
    }
    else if (!strcmp(argv[1],"-nlist")) {
      alist[0]=0;
      alistset[0]=1;
    }
    else if (!strcmp(argv[1],"-tlist")) {
      alist[0]=2;
      alistset[0]=1;
    }
    else if (!strcmp(argv[1],"-noramsch")) {
      playramsch=0;
      ramschset=1;
    }
    else if (!strcmp(argv[1],"-ramsch")) {
      playramsch=1;
      ramschset=1;
    }
    else if (!strcmp(argv[1],"-ramschonly")) {
      playramsch=2;
      ramschset=1;
    }
    else if (!strcmp(argv[1],"-sramsch")) {
      playsramsch=1;
      sramschset=1;
    }
    else if (!strcmp(argv[1],"-nosramsch")) {
      playsramsch=0;
      sramschset=1;
    }
    else if (!strcmp(argv[1],"-nokontra")) {
      playkontra=0;
      kontraset=1;
    }
    else if (!strcmp(argv[1],"-kontra")) {
      playkontra=1;
      kontraset=1;
    }
    else if (!strcmp(argv[1],"-kontra18")) {
      playkontra=2;
      kontraset=1;
    }
    else if (!strcmp(argv[1],"-nobock")) {
      playbock=0;
      bockset=1;
    }
    else if (!strcmp(argv[1],"-bock")) {
      playbock=1;
      bockset=1;
    }
    else if (!strcmp(argv[1],"-bockramsch")) {
      playbock=2;
      bockset=1;
    }
    else if (!strcmp(argv[1],"-resumebock")) {
      resumebock=1;
      resumebockset=1;
    }
    else if (!strcmp(argv[1],"-noresumebock")) {
      resumebock=0;
      resumebockset=1;
    }
    else if (!strcmp(argv[1],"-spitze")) {
      spitzezaehlt=1;
      spitzezaehltset=1;
    }
    else if (!strcmp(argv[1],"-spitze2")) {
      spitzezaehlt=2;
      spitzezaehltset=1;
    }
    else if (!strcmp(argv[1],"-nospitze")) {
      spitzezaehlt=0;
      spitzezaehltset=1;
    }
    else if (!strcmp(argv[1],"-revolution")) {
      revolution=1;
      revolutionset=1;
    }
    else if (!strcmp(argv[1],"-norevolution")) {
      revolution=0;
      revolutionset=1;
    }
    else if (!strcmp(argv[1],"-klopfen")) {
      klopfen=1;
      klopfenset=1;
    }
    else if (!strcmp(argv[1],"-noklopfen")) {
      klopfen=0;
      klopfenset=1;
    }
    else if (!strcmp(argv[1],"-schenken")) {
      schenken=1;
      schenkenset=1;
    }
    else if (!strcmp(argv[1],"-noschenken")) {
      schenken=0;
      schenkenset=1;
    }
    else if (!strcmp(argv[1],"-fastdeal")) {
      fastdeal=1;
      fastdealset=1;
    }
    else if (!strcmp(argv[1],"-slowdeal")) {
      fastdeal=0;
      fastdealset=1;
    }
    else if (!strcmp(argv[1],"-hint")) {
      hints[0]=1;
      hintsset[0]=1;
    }
    else if (!strcmp(argv[1],"-nohint")) {
      hints[0]=0;
      hintsset[0]=1;
    }
    else if (!strcmp(argv[1],"-irc")) {
      irc_play=1;
    }
    else if (!strcmp(argv[1],"-noirc")) {
      irc_play=0;
    }
    else if ((argv[1][0]!='-') && numsp<3) {
      disp_name[numsp++]=argv[1];
    }
    else if (argc>2) {
      if (!strcmp(argv[1],"-display") || !strcmp(argv[1],"-d")) {
	disp_name[0]=argv[2];
      }
      else if (!strcmp(argv[1],"-geometry") || !strcmp(argv[1],"-g")) {
	geom_f=XParseGeometry(argv[2],&geom_x,&geom_y,&w,&h);
      }
      else if (!strcmp(argv[1],"-name") || !strcmp(argv[1],"-n")) {
	prog_name=argv[2];
      }
      else if (!strcmp(argv[1],"-title") || !strcmp(argv[1],"-T")) {
	title=argv[2];
      }
      else if (!strcmp(argv[1],"-fg")) {
	fg_col=argv[2];
      }
      else if (!strcmp(argv[1],"-bg")) {
	bg_col=argv[2];
      }
      else if (!strcmp(argv[1],"-bt")) {
	bt_col=argv[2];
      }
      else if (!strcmp(argv[1],"-3dtop")) {
	w3d_col=argv[2];
      }
      else if (!strcmp(argv[1],"-3dbot")) {
	b3d_col=argv[2];
      }
      else if (!strcmp(argv[1],"-mark")) {
	mk_col=argv[2];
      }
      else if (!strncmp(argv[1],"-color",6) &&
	       (i=atoi(argv[1]+6))>=1 && i<=20) {
	ccol[i-1]=argv[2];
      }
      else if (!strcmp(argv[1],"-mb")) {
	mbutton[0]=atoi(argv[2]);
	mbuttonset[0]=1;
      }
      else if (!strcmp(argv[1],"-tdelay")) {
	nimmstich[0][0]=(int)(atof(argv[2])*10+.5);
	tdelayset[0]=1;
      }
      else if (!strcmp(argv[1],"-font") || !strcmp(argv[1],"-fn")) {
	font_name=argv[2];
      }
      else if (!strcmp(argv[1],"-list") || !strcmp(argv[1],"-l")) {
	list_file=argv[2];
      }
      else if (!strcmp(argv[1],"-bockevents")) {
	bockevents=atoi(argv[2]);
	bockeventsset=1;
      }
      else if (!strcmp(argv[1],"-opt")) {
	opt_file=argv[2];
      }
      else if (!strcmp(argv[1],"-log")) {
	prot_file=argv[2];
	logging=1;
      }
      else if (!strcmp(argv[1],"-game")) {
	game_file=argv[2];
      }
      else if (!strcmp(argv[1],"-lang")) {
	lang=langidx(argv[2],0);
      }
      else if (!strcmp(argv[1],"-start")) {
	geber=atoi(argv[2]);
	if (geber<1 || geber>3) geber=0;
	else geber=left(geber);
      }
      else if (!strcmp(argv[1],"-auto")) {
	numgames=atoi(argv[2]);
	if (numgames<1) numgames=1;
      }
      else if (!strcmp(argv[1],"-s1")) {
	strateg[0]=atoi(argv[2]);
	stgset[0]=1;
      }
      else if (!strcmp(argv[1],"-s2")) {
	strateg[1]=atoi(argv[2]);
	stgset[1]=1;
      }
      else if (!strcmp(argv[1],"-s3")) {
	strateg[2]=atoi(argv[2]);
	stgset[2]=1;
      }
      else if (!strcmp(argv[1],"-irctelnet")) {
	irc_telnet=argv[2];
	irc_play=1;
      }
      else if (!strcmp(argv[1],"-ircserver")) {
	irc_host=argv[2];
	irc_play=1;
      }
      else if (!strcmp(argv[1],"-ircport")) {
	irc_port=atoi(argv[2]);
	irc_play=1;
      }
      else if (!strcmp(argv[1],"-ircchannel")) {
	irc_channel=argv[2];
	irc_play=1;
      }
      else if (!strcmp(argv[1],"-ircnick")) {
	irc_nick=argv[2];
	irc_play=1;
      }
      else if (!strcmp(argv[1],"-ircuser")) {
	irc_user=argv[2];
	irc_play=1;
      }
      else if (!strcmp(argv[1],"-ircrealname")) {
	irc_realname=argv[2];
	irc_play=1;
      }
      else if (!strcmp(argv[1],"-ircpos")) {
	irc_pos=atoi(argv[2]);
	irc_play=1;
      }
      else if (!strcmp(argv[1],"-irclog")) {
	irc_logfile=argv[2];
	irc_play=1;
      }
      else {
	invopt(argv[1]);
      }
      argc--;argv++;
    }
    else {
      invopt(argv[1]);
    }
    argc--;argv++;
  }
  if (numgames) {
    numsp=irc_play=0;
    if (lang<0) lang=langidx((char *)0,1);
    init_text();
    if (geber<0) geber=0;
    if (logging<0) logging=0;
    if (unformatted<0) unformatted=1;
    if (alist[0]<0) alist[0]=0;
    for (i=0;i<3;i++) {
      if (strateg[i]<-4) strateg[i]=-4;
      else if (strateg[i]>4) strateg[i]=4;
    }
  }
  else {
    xinitres(0);
  }
  read_list();
  if (irc_play) {
    setsum(0);
    read_opt();
    irc_init();
  }
  else {
    for (sn=1;sn<numsp;sn++) {
      xinitres(sn);
    }
    read_opt();
  }
  for (sn=0;sn<numsp;sn++) {
    xinitwin(sn);
  }
  init_dials();
  switch (numsp) {
  case 0:
    for (sn=0;sn<3;sn++) {
      sprintf(spnames[sn][0],"%s%d",textarr[TX_COMPUTER],sn+1);
    }
    break;
  case 1:
    strcpy(spnames[1][0],textarr[TX_COMPUTER]);
    strcpy(spnames[1][1],textarr[TX_LINKS]);
    strcpy(spnames[2][0],textarr[TX_COMPUTER]);
    strcpy(spnames[2][1],textarr[TX_RECHTS]);
    break;
  case 2:
    strcpy(spnames[2][0],textarr[TX_COMPUTER]);
    break;
  }
  if (!irc_play) save_opt();
}

VOID waitt(t,f)
int t,f;
{
  struct timeval timeout;
  int sn;

  if (numgames) return;
  ticker+=t;
  if (!irc_play || irc_state==IRC_PLAYING) {
    for (sn=0;sn<numsp;sn++) {
      if (!lost[sn]) {
	if (!f) {
	  XFlush(dpy[sn]);
	}
	else {
	  XSync(dpy[sn],0);
	}
      }
    }
  }
  timeout.tv_sec=t/1000;
  timeout.tv_usec=(t%1000)*1000L;
  select(0,NULL,NULL,NULL,&timeout);
  if (f>1) refresh();
}

VOID stdwait()
{
  waitt(700,2);
}

VOID backgr(sn,x,y,w,h)
int sn,x,y,w,h;
{
  XFillRectangle(dpy[sn],bck[sn],gcbck[sn],x,y,w,h);
  XFillRectangle(dpy[sn],win[sn],gcbck[sn],x,y,w,h);
}

VOID putdesk(sn,x,y)
int sn,x,y;
{
  backgr(sn,x,y,desk[sn].cardw,desk[sn].cardh);
}

VOID drawcard(sn,i,x,y)
int sn,i,x,y;
{
  int p,x1,y1,x2,y2,x3,y3,f,w,dxy,dx,dy,fc;
  unsigned long wp,bp;

  x+=2;
  f=i>>3;
  w=i&7;
  fc=desk[sn].col>21;
  wp=fc?color[sn][4].pixel:wpix[sn];
  bp=fc?color[sn][19].pixel:bpix[sn];
  if (fc && ((KOENIG<=w && w<=BUBE) || i<0)) {
    x1=i>=0?(w-KOENIG+1)*desk[sn].cardw:0;
    XCopyArea(dpy[sn],colcards[sn],bck[sn],gc[sn],x1,0,
	      desk[sn].cardw,desk[sn].cardh,x,y);
  }
  else {
    x1=(i>=0?KOENIG<=w && w<=BUBE?(w-KOENIG+3):2:1)*desk[sn].cardw;
    XFillRectangle(dpy[sn],bck[sn],gcbck[sn],x,y,
		   desk[sn].cardw,desk[sn].cardh);
    change_gcxor(sn,wp);
    XCopyPlane(dpy[sn],bwcards[sn],bck[sn],gcxor[sn],0,0,
	       desk[sn].cardw,desk[sn].cardh,x,y,1);
    change_gcxor(sn,bp^wp^bgpix[sn]);
    XCopyPlane(dpy[sn],bwcards[sn],bck[sn],gcxor[sn],x1,0,
	       desk[sn].cardw,desk[sn].cardh,x,y,1);
  }
  if (i>=0) {
    change_gcxor(sn,(desk[sn].col>f+2?color[sn][f].pixel:bpix[sn])^
		 wp^bgpix[sn]);
    p=cnts[w];
    if (w==BUBE && f==1) dxy=dx=-1;
    else dxy=dx=0;
    do {
      x1=f*16*desk[sn].f/desk[sn].q;
      y1=96*desk[sn].f/desk[sn].q;
      if (bigs[p+1]>39) x1+=64*desk[sn].f/desk[sn].q;
      x2=x1+16*desk[sn].f/desk[sn].q;
      y2=y1+16*desk[sn].f/desk[sn].q;
      dxy=-dxy;
      x3=(bigs[p++]-2)*desk[sn].f/desk[sn].q+dxy+dx;
      y3=bigs[p++]*desk[sn].f/desk[sn].q+dxy;
      dx=0;
      if (desk[sn].col>f+2) {
	XCopyPlane(dpy[sn],bwcards[sn],bck[sn],gcxor[sn],
		   x1+2*desk[sn].cardw,y1,x2-x1,y2-y1,x+x3,y+y3,1);
      }
      else {
	XCopyPlane(dpy[sn],bwcards[sn],bck[sn],gcxor[sn],
		   x1,y1,x2-x1,y2-y1,x+x3,y+y3,1);
      }
    } while (p!=cnts[w+1]);
    for (p=0;p<8;p+=2) {
      x1=f*8*desk[sn].f/desk[sn].q;
      y1=112*desk[sn].f/desk[sn].q;
      if (smls[p+1]>11) x1+=32*desk[sn].f/desk[sn].q;
      x2=x1+8*desk[sn].f/desk[sn].q;
      y2=y1+8*desk[sn].f/desk[sn].q;
      if (f>0 && f<3 && p<3) dy=-1;
      else dy=0;
      x3=(smls[p]-2)*desk[sn].f/desk[sn].q;
      y3=smls[p+1]*desk[sn].f/desk[sn].q+dy;
      if (desk[sn].col>f+2) {
	XCopyPlane(dpy[sn],bwcards[sn],bck[sn],gcxor[sn],
		   x1+2*desk[sn].cardw,y1,x2-x1,y2-y1,x+x3,y+y3,1);
      }
      else {
	XCopyPlane(dpy[sn],bwcards[sn],bck[sn],gcxor[sn],
		   x1,y1,x2-x1,y2-y1,x+x3,y+y3,1);
      }
      x1=256+(w==AS?lang*20:w==ZEHN?p>3?23:0:w<=BUBE?(w-KOENIG+1)*5+lang*20:
	      (p>3?31:8)+(w-NEUN)*5);
      x1=x1*desk[sn].f/desk[sn].q;
      y1=95+(w!=ZEHN && w<=BUBE?p>3?7:0:14);
      y1=y1*desk[sn].f/desk[sn].q;
      x2=x1+(w==ZEHN?8:5)*desk[sn].f/desk[sn].q;
      y2=y1+7*desk[sn].f/desk[sn].q;
      x3=(smlc[p]-(w==ZEHN?p&2?2:1:0))*desk[sn].f/desk[sn].q+
	(w!=ZEHN && p&2 && desk[sn].f>1);
      y3=smlc[p+1]*desk[sn].f/desk[sn].q;
      XCopyPlane(dpy[sn],bwcards[sn],bck[sn],gcxor[sn],
		 x1,y1,x2-x1,y2-y1,x+x3,y+y3,1);
    }
  }
  change_gcxor(sn,fgpix[sn]);
  XCopyArea(dpy[sn],bck[sn],win[sn],gc[sn],x,y,
	    desk[sn].cardw,desk[sn].cardh,x,y);
}

VOID putcard(sn,i,x,y)
int sn,i,x,y;
{
  if (i<0) putdesk(sn,x,y);
  else drawcard(sn,i,x,y);
}

VOID putback(sn,x,y)
int sn,x,y;
{
  drawcard(sn,-1,x,y);
}

VOID hint_line(sn,c,gcp)
int sn,c;
GC *gcp;
{
  int xyarr[4];

  xyarr[0]=desk[sn].playx+(c%10)*desk[sn].cardx+7*desk[sn].f/desk[sn].q;
  xyarr[1]=desk[sn].playy-5*desk[sn].f/desk[sn].q;
  xyarr[2]=xyarr[0]+desk[sn].cardx-16*desk[sn].f/desk[sn].q+1;
  xyarr[3]=xyarr[1];
  XDrawLine(dpy[sn],win[sn],gcp[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
  XDrawLine(dpy[sn],bck[sn],gcp[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
}

VOID show_hint(sn,c,d)
int sn,c,d;
{
  static int lm[3];

  if (lm[sn]) {
    hint_line(sn,lm[sn]-1,gcbck);
  }
  if (d) {
    change_gc(sn,mkpix[sn],gc);
    hint_line(sn,c,gc);
    change_gc(sn,fgpix[sn],gc);
    lm[sn]=c+1;
  }
  else {
    lm[sn]=0;
  }
}

VOID putamark(sn,s)
int sn,s;
{
  int a,b,xp1,xp2,yp,xyarr[4];

  if (sn==s || ouveang || (spitzeang && !sptzmrk)) return;
  a=48*desk[sn].f/desk[sn].q-4;
  b=48*desk[sn].f/desk[sn].q-2;
  change_gc(sn,mkpix[sn],gc);
  xp1=desk[sn].com1x+32*desk[sn].f/desk[sn].q;
  xp2=desk[sn].com2x+32*desk[sn].f/desk[sn].q;
  yp=desk[sn].com1y+2*desk[sn].f/desk[sn].q;
  xyarr[0]=(s==left(sn)?xp1:xp2)-a/2;
  xyarr[1]=yp;
  xyarr[2]=xyarr[0]+a-4*desk[sn].f/desk[sn].q+3;
  xyarr[3]=xyarr[1];
  XDrawLine(dpy[sn],win[sn],gc[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
  XDrawLine(dpy[sn],bck[sn],gc[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
  xyarr[1]=yp+89*desk[sn].f/desk[sn].q;
  xyarr[3]=xyarr[1];
  XDrawLine(dpy[sn],win[sn],gc[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
  XDrawLine(dpy[sn],bck[sn],gc[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
  xyarr[0]=(s==left(sn)?xp1:xp2)-b/2;
  xyarr[1]=yp+1;
  xyarr[2]=xyarr[0]+b-4*desk[sn].f/desk[sn].q+3;
  xyarr[3]=xyarr[1];
  XDrawLine(dpy[sn],win[sn],gc[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
  XDrawLine(dpy[sn],bck[sn],gc[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
  xyarr[1]=yp+88*desk[sn].f/desk[sn].q;
  xyarr[3]=xyarr[1];
  XDrawLine(dpy[sn],win[sn],gc[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
  XDrawLine(dpy[sn],bck[sn],gc[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
  change_gc(sn,fgpix[sn],gc);
}

VOID putmark(s)
int s;
{
  int sn;

  for (sn=0;sn<numsp;sn++) {
    putamark(sn,s);
  }
}

VOID remmark(f)
int f;
{
  int sn;

  for (sn=0;sn<numsp;sn++) {
    putback(sn,desk[sn].com1x,desk[sn].com1y);
    putback(sn,desk[sn].com2x,desk[sn].com2y);
    if (f) putamark(sn,spieler);
  }
}

VOID movecard(nn,sn,x1,y1,x2,y2)
int nn,sn[],x1[],y1[],x2[],y2[];
{
  int dx[3],dy[3],i,j,n=8;

  for (i=0;i<nn;i++) {
    dx[i]=x2[i]-x1[i];
    dy[i]=y2[i]-y1[i];
  }
  for (i=0;i<n;i++) {
    for (j=0;j<nn;j++) {
      XDrawRectangle(dpy[sn[j]],win[sn[j]],gcxor[sn[j]],
		     x1[j]+i*dx[j]/n,y1[j]+i*dy[j]/n,
		     desk[sn[j]].cardw,desk[sn[j]].cardh);
    }
    waitt(9,0);
    for (j=0;j<nn;j++) {
      XDrawRectangle(dpy[sn[j]],win[sn[j]],gcxor[sn[j]],
		     x1[j]+i*dx[j]/n,y1[j]+i*dy[j]/n,
		     desk[sn[j]].cardw,desk[sn[j]].cardh);
    }
  }
  refresh();
}

VOID homecard(s,n,m)
int s,n,m;
{
  int sn,sna[3],x1[3],y1[3],x2[3],y2[3];

  for (sn=0;sn<numsp;sn++) {
    sna[sn]=sn;
    x2[sn]=desk[sn].x+desk[sn].w;
    if (s!=sn) {
      y2[sn]=desk[sn].com1y;
      if (s==left(sn)) x2[sn]=desk[sn].x-desk[sn].cardw;
    }
    else y2[sn]=desk[sn].playy;
    x1[sn]=(n?desk[sn].stichx:desk[sn].skatx)+m*desk[sn].cardw;
    y1[sn]=n?desk[sn].stichy:desk[sn].skaty;
    putdesk(sn,x1[sn],y1[sn]);
  }
  movecard(numsp,sna,x1,y1,x2,y2);
}

VOID givecard(s,n)
int s,n;
{
  int sn,sna[3],x1[3],y1[3],x2[3],y2[3];

  sptzmrk=0;
  for (sn=0;sn<numsp;sn++) {
    sna[sn]=sn;
    if (s<0) {
      x1[sn]=desk[sn].skatx;
      y1[sn]=desk[sn].skaty;
    }
    else if (s!=sn) {
      if (s==left(sn)) x1[sn]=desk[sn].com1x;
      else x1[sn]=desk[sn].com2x;
      y1[sn]=desk[sn].com1y;
    }
    else {
      if (!n) x1[sn]=desk[sn].playx;
      else if (n==1) x1[sn]=desk[sn].playx+3*desk[sn].cardx;
      else x1[sn]=desk[sn].playx+7*desk[sn].cardx;
      y1[sn]=desk[sn].playy;
    }
    x2[sn]=desk[sn].x+desk[sn].w;
    if (geber!=sn) {
      y2[sn]=desk[sn].com1y;
      if (geber==left(sn)) x2[sn]=desk[sn].x-desk[sn].cardw;
    }
    else y2[sn]=desk[sn].playy;
  }
  if (!fastdeal) movecard(numsp,sna,x2,y2,x1,y1);
  for (sn=0;sn<numsp;sn++) {
    putback(sn,x1[sn],y1[sn]);
    if (s==hoerer) putamark(sn,s);
    if (s==sn) {
      putback(sn,x1[sn]+desk[sn].cardx,y1[sn]);
      putback(sn,x1[sn]+2*desk[sn].cardx,y1[sn]);
      if (n==1) putback(sn,x1[sn]+3*desk[sn].cardx,y1[sn]);
    }
    else if (s<0) {
      putback(sn,x1[sn]+desk[sn].cardw,y1[sn]);
    }
  }
  if (!fastdeal) waitt(300,2);
}

VOID initscr(sn,sor)
int sn,sor;
{
  int i,x,y,c;

  if (phase==WEITER || phase==REVOLUTION) return;
  if (sor) {
    if (phase==SPIELEN && hintcard!=-1) c=cards[hintcard];
    else c=-1;
    sort(sn);
    for (i=0;i<10;i++) {
      if (c>=0 && c==cards[sn*10+i]) hintcard=sn*10+i;
      if (phase==SPIELEN && !iscomp(sn) && sn==(ausspl+vmh)%3 &&
	  hints[sn] && hintcard!=-1) {
	show_hint(sn,hintcard,1);
      }
      putcard(sn,cards[sn*10+i],
	      desk[sn].playx+i*desk[sn].cardx,desk[sn].playy);
    }
  }
  if (phase!=ANSAGEN) {
    di_info(sn,-1);
    if (predef && (!ouveang || sn==spieler)) {
      x=desk[sn].w/2;
      y=desk[sn].y+2*charh[sn];
      v_gtext(sn,x,y,0,textarr[TX_VORDEFINIERTES_SPIEL]);
    }
  }
  if (phase!=ANSAGEN && ouveang) {
    if (sn==spieler) {
      for (sn=0;sn<numsp;sn++) {
	if (sn!=spieler) initscr(sn,0);
      }
    }
    else {
      y=spieler==left(sn)?desk[sn].com1y:desk[sn].com2y;
      for (i=0;i<10;i++) {
	putcard(sn,cards[spieler*10+i],desk[sn].playx+i*desk[sn].cardx,y);
      }
      x=spieler==left(sn)?desk[sn].com2x:desk[sn].com1x;
      y=spieler==left(sn)?desk[sn].com2y:desk[sn].com1y;
      putback(sn,x,y);
    }
  }
  else if (spitzeang && sn!=spieler) {
    x=spieler==left(sn)?desk[sn].com1x:desk[sn].com2x;
    y=spieler==left(sn)?desk[sn].com1y:desk[sn].com2y;
    putcard(sn,trumpf==4?BUBE:SIEBEN|trumpf<<3,x,y);
  }
}

VOID spielendscr()
{
  int sn,s,i,d,x,y1,y2,sav[3];

  clr_desk(0);
  for (sn=0;sn<numsp;sn++) {
    di_info(sn,-2);
    for (s=0;s<3;s++) {
      sav[0]=alternate[s];
      sav[1]=sort1[s];
      sav[2]=sort2[s];
      alternate[s]=0;
      sort1[s]=sort1[sn];
      sort2[s]=sort2[sn];
      sort(s);
      alternate[s]=sav[0];
      sort1[s]=sav[1];
      sort2[s]=sav[2];
    }
    y1=desk[sn].com1y;
    y2=desk[sn].skaty;
    if (left(sn)!=spieler) swap(&y1,&y2);
    d=sort1[sn]?stich-1:0;
    for (i=0;i<11-stich;i++) {
      x=desk[sn].playx+(stich-1+2*i)*desk[sn].cardx/2;
      putcard(sn,cards[sn*10+i+d],x,desk[sn].playy);
      s=left(sn);
      putcard(sn,cards[s*10+i+d],x,y1);
      s=left(s);
      putcard(sn,cards[s*10+i+d],x,y2);
    }
    if (sn!=spieler)  {
      x=desk[sn].skatx+desk[sn].cardx;
      y1=(desk[sn].skaty+desk[sn].com1y+desk[sn].cardh-charh[sn])/2;
      v_gtext(sn,x,y1,0,textarr[trumpf==-1?TX_NULL_DICHT:TX_REST_BEI_MIR]);
    }
  }
  phase=WEITER;
  di_weiter(1);
}

VOID revolutionsort(sp)
int sp;
{
  int sn,s,i,x,y1,y2,sav[3];

  for (s=0;s<3;s++) {
    sav[0]=alternate[s];
    sav[1]=sort1[s];
    sav[2]=sort2[s];
    alternate[s]=0;
    sort1[s]=revolsort;
    sort2[s]=1;
    sort(s);
    alternate[s]=sav[0];
    sort1[s]=sav[1];
    sort2[s]=sav[2];
  }
  for (sn=0;sn<numsp;sn++) {
    if (sn==spieler) continue;
    y1=desk[sn].com1y;
    y2=desk[sn].skaty;
    if (left(sn)!=spieler) swap(&y1,&y2);
    for (i=0;i<10;i++) {
      x=desk[sn].playx+i*desk[sn].cardx;
      putcard(sn,cards[sn*10+i],x,desk[sn].playy);
      s=left(sn);
      if (sp || s!=spieler) putcard(sn,cards[s*10+i],x,y1);
      s=left(s);
      if (sp || s!=spieler) putcard(sn,cards[s*10+i],x,y2);
    }
  }
}

VOID revolutionscr()
{
  int sn,mi,x,y,f;

  tauschcard=-1;
  tauschdone=0;
  revolsort=iscomp(spieler)?sort1[0]:sort1[spieler];
  clr_desk(1);
  f=1;
  for (sn=0;sn<numsp;sn++) {
    if (sn==spieler) continue;
    di_info(sn,-2);
    x=desk[sn].skatx+desk[sn].cardx;
    y=(desk[sn].skaty+desk[sn].com1y+desk[sn].cardh-charh[sn])/2;
    v_gtext(sn,x,y,0,textarr[TX_KARTEN_AUSTAUSCHEN]);
    mi=left(sn)==spieler?left(spieler):left(sn);
    x=desk[sn].playx+10*desk[sn].f/desk[sn].q;
    v_gtextnc(sn,0,0,x,desk[sn].com1y+desk[sn].cardh+1,
	      0,textarr[spieler==ausspl?TX_VORHAND:
			spieler==left(ausspl)?TX_MITTELHAND:
			TX_HINTERHAND]);
    v_gtextnc(sn,0,0,x,desk[sn].skaty-charh[sn]-1,
	      0,textarr[mi==ausspl?TX_VORHAND:
			mi==left(ausspl)?TX_MITTELHAND:
			TX_HINTERHAND]);
    if (f) {
      tauschply=sn;
      put_fbox(sn,TX_FERTIG);
      f=0;
    }
  }
  revolutionsort(1);
  phase=REVOLUTION;
}

VOID clr_desk(nsp)
int nsp;
{
  int sn;

  for (sn=0;sn<numsp;sn++) {
    if (!nsp || sn!=spieler) {
      backgr(sn,desk[sn].x,desk[sn].y,desk[sn].w,desk[sn].h);
      di_info(sn,3);
    }
  }
  if (!nsp && ouveang) {
    for (sn=0;sn<numsp;sn++) {
      if (sn!=spieler) di_info(sn,-2);
    }
    ouveang=0;
    for (sn=0;sn<numsp;sn++) {
      calc_desk(sn);
      if (sn!=spieler) di_info(sn,3);
    }
    ouveang=1;
  }
}

VOID draw_box(sn,x,y,w)
int sn,x,y,w;
{
  int xy[4];

  xy[0]=x+2;xy[1]=y-1;
  xy[2]=x+w-3;xy[3]=y+18*desk[sn].f/desk[sn].q;
  if (gfx3d[sn]) {
    draw_3d(win[sn],bck[sn],sn,xy[0],xy[1],xy[2],xy[3],0);
  }
  else {
    XDrawRectangle(dpy[sn],win[sn],gc[sn],xy[0],xy[1],xy[2]-xy[0],xy[3]-xy[1]);
    XDrawRectangle(dpy[sn],bck[sn],gc[sn],xy[0],xy[1],xy[2]-xy[0],xy[3]-xy[1]);
  }
  xy[0]++;xy[1]++;
  xy[2]--;xy[3]--;
  if (gfx3d[sn]) {
    draw_3d(win[sn],bck[sn],sn,xy[0],xy[1],xy[2],xy[3],0);
  }
  else {
    XDrawRectangle(dpy[sn],win[sn],gc[sn],xy[0],xy[1],xy[2]-xy[0],xy[3]-xy[1]);
    XDrawRectangle(dpy[sn],bck[sn],gc[sn],xy[0],xy[1],xy[2]-xy[0],xy[3]-xy[1]);
  }
  xy[0]++;xy[1]++;
  change_gc(sn,btpix[sn],gc);
  XFillRectangle(dpy[sn],win[sn],gc[sn],xy[0],xy[1],xy[2]-xy[0],xy[3]-xy[1]);
  XFillRectangle(dpy[sn],bck[sn],gc[sn],xy[0],xy[1],xy[2]-xy[0],xy[3]-xy[1]);
  change_gc(sn,fgpix[sn],gc);
}

VOID put_box(s)
int s;
{
  int sn;

  for (sn=0;sn<numsp;sn++) {
    if (s!=sn) {
      if (s==left(sn)) draw_box(sn,desk[sn].cbox1x,desk[sn].cboxy,
				desk[sn].cardw);
      else draw_box(sn,desk[sn].cbox2x,desk[sn].cboxy,desk[sn].cardw);
    }
    else {
      draw_box(sn,desk[sn].pboxx,desk[sn].pboxy,desk[sn].cardw);
      draw_box(sn,desk[sn].pboxx+desk[sn].cardw,desk[sn].pboxy,desk[sn].cardw);
    }
  }
}

VOID rem_box(s)
int s;
{
  int sn;

  for (sn=0;sn<numsp;sn++) {
    if (s!=sn) {
      if (s==left(sn)) backgr(sn,desk[sn].cbox1x,desk[sn].cboxy-5,
			      66*desk[sn].f/desk[sn].q,
			      28*desk[sn].f/desk[sn].q);
      else backgr(sn,desk[sn].cbox2x,desk[sn].cboxy-5,
		  desk[sn].cardw,28*desk[sn].f/desk[sn].q);
    }
    else {
      backgr(sn,desk[sn].pboxx,desk[sn].pboxy-5,
	     desk[sn].cardw,28*desk[sn].f/desk[sn].q);
      backgr(sn,desk[sn].pboxx+desk[sn].cardw,desk[sn].pboxy-5,
	     desk[sn].cardw,28*desk[sn].f/desk[sn].q);
    }
  }
}

VOID inv_box(s,c,rev)
int s,c,rev;
{
  int sn,x,y,w,h;

  for (sn=0;sn<numsp;sn++) {
    x=(s!=sn
       ?s==left(sn)
       ?desk[sn].cbox1x
       :desk[sn].cbox2x
       :desk[sn].pboxx+(c?desk[sn].cardw:0))+4;
    y=(s!=sn?desk[sn].cboxy:desk[sn].pboxy)+1;
    w=64*desk[sn].f/desk[sn].q-8;
    h=18*desk[sn].f/desk[sn].q-2;
    if (gfx3d[sn]) {
      draw_3d(win[sn],bck[sn],sn,x-1,y-1,x+w,y+h,rev);
      draw_3d(win[sn],bck[sn],sn,x-2,y-2,x+w+1,y+h+1,rev);
    }
    else {
      change_gcxor(sn,btpix[sn]^fgpix[sn]^bgpix[sn]);
      XFillRectangle(dpy[sn],win[sn],gcxor[sn],x,y,w,h);
      XFillRectangle(dpy[sn],bck[sn],gcxor[sn],x,y,w,h);
      change_gcxor(sn,fgpix[sn]);
    }
  }
}

VOID put_fbox(sn,t)
int sn,t;
{
  draw_box(sn,desk[sn].pboxx+24*desk[sn].f/desk[sn].q,desk[sn].pboxy,
	   80*desk[sn].f/desk[sn].q);
  v_gtextc(sn,1,desk[sn].pboxx+24*desk[sn].f/desk[sn].q,desk[sn].pboxy,
	   80*desk[sn].f/desk[sn].q,
	   textarr[t]);
}

VOID rem_fbox(sn)
int sn;
{
  backgr(sn,desk[sn].pboxx+24*desk[sn].f/desk[sn].q,desk[sn].pboxy-5,
	 80*desk[sn].f/desk[sn].q,28*desk[sn].f/desk[sn].q);
}

VOID inv_fbox(sn,rev)
int sn,rev;
{
  int x,y,w,h;

  x=desk[sn].pboxx+24*desk[sn].f/desk[sn].q+4;
  y=desk[sn].pboxy+1;
  w=80*desk[sn].f/desk[sn].q-8;
  h=18*desk[sn].f/desk[sn].q-2;
  if (gfx3d[sn]) {
    draw_3d(win[sn],bck[sn],sn,x-1,y-1,x+w,y+h,rev);
    draw_3d(win[sn],bck[sn],sn,x-2,y-2,x+w+1,y+h+1,rev);
  }
  else {
    change_gcxor(sn,btpix[sn]^fgpix[sn]^bgpix[sn]);
    XFillRectangle(dpy[sn],win[sn],gcxor[sn],x,y,w,h);
    XFillRectangle(dpy[sn],bck[sn],gcxor[sn],x,y,w,h);
    change_gcxor(sn,fgpix[sn]);
  }
}

int card_at(sn,x,y,zw)
int sn,x,y,zw;
{
  int c,s;

  zw=(zw && y>=desk[sn].skaty && y<desk[sn].skaty+desk[sn].cardh);
  if (y>=desk[sn].playy || zw) {
    x-=desk[sn].playx;
    if (x<0) return 0;
    c=x/desk[sn].cardx;
    if (c>9) c=9;
    s=zw?left(sn)==spieler?left(spieler):left(sn):sn;
    if (cards[10*s+c]>=0) return c+1+(zw?10:0);
  }
  return 0;
}

int hndl_reizen(sn,x,y)
int sn,x,y;
{
  int b;

  if (x<=desk[sn].pboxx+56*desk[sn].f/desk[sn].q) b=0;
  else b=desk[sn].cardw;
  if (x>=desk[sn].pboxx+7*desk[sn].f/desk[sn].q+b &&
      x<=desk[sn].pboxx+56*desk[sn].f/desk[sn].q+b &&
      y>=desk[sn].pboxy+1 &&
      y<=desk[sn].pboxy+16*desk[sn].f/desk[sn].q) {
    di_delres(sn);
    if (b) maxrw[sn]=0;
    else {
      maxrw[sn]=215;
    }
    do_entsch();
    return 1;
  }
  return 0;
}

int hndl_druecken(sn,x,y)
int sn,x,y;
{
  int c,sna[1],x1[1],y1[1],x2[1],y2[1];

  c=card_at(sn,x,y,0);
  if (c) {
    c--;
    swap(&cards[10*sn+c],&cards[drkcd+30]);
    sna[0]=sn;
    x1[0]=desk[sn].playx+c*desk[sn].cardx;
    y1[0]=desk[sn].playy;
    x2[0]=desk[sn].skatx+drkcd*desk[sn].cardw;
    y2[0]=desk[sn].skaty;
    movecard(1,sna,x1,y1,x2,y2);
    putcard(sn,cards[drkcd+30],x2[0],y2[0]);
    initscr(sn,1);
    drkcd=1-drkcd;
    return 1;
  }
  if (x>=desk[sn].pboxx+31*desk[sn].f/desk[sn].q &&
      x<=desk[sn].pboxx+96*desk[sn].f/desk[sn].q &&
      y>=desk[sn].pboxy+1 &&
      y<=desk[sn].pboxy+16*desk[sn].f/desk[sn].q) inv_fbox(spieler,1);
  else return 0;
  stdwait();
  inv_fbox(spieler,0);
  if (trumpf==5 && (((cards[30]&7)==BUBE) || ((cards[31]&7)==BUBE))) {
    di_buben();
    return 1;
  }
  rem_fbox(spieler);
  if (trumpf==5) {
    putback(sn,desk[sn].skatx,desk[sn].skaty);
    putback(sn,desk[sn].skatx+desk[sn].cardw,desk[sn].skaty);
    if (ramschspiele && klopfen) {
      di_klopfen(spieler);
    }
    else {
      vmh=left(vmh);
      if (vmh) di_schieben();
      else start_ramsch();
    }
    return 1;
  }
  home_skat();
  save_skat(1);
  info_stich(0,cards[30],1);
  info_stich(1,cards[31],1);
  for (c=0;c<2;c++) {
    stsum+=cardw[cards[c+30]&7];
    gespcd[cards[c+30]]=1;
    cards[c+30]=-1;
  }
  gedr=2;
  do_ansagen();
  return 1;
}

int hndl_tauschen(sn,x,y)
int sn,x,y;
{
  int c,c1,zw,mi,d[2],sna[2],x1[2],y1[2],x2[2],y2[2];

  mi=left(sn)==spieler?left(spieler):left(sn);
  c=card_at(sn,x,y,1);
  if (c) {
    zw=c>10;
    c=(c-1)%10;
    c1=tauschcard!=-1?tauschcard%10:0;
    x1[0]=desk[sn].playx+c*desk[sn].cardx;
    y1[0]=zw?desk[sn].skaty:desk[sn].playy;
    x2[0]=desk[sn].playx+c1*desk[sn].cardx;
    y2[0]=zw?desk[sn].playy:desk[sn].skaty;
    d[0]=(zw?4:-4)*desk[sn].f/desk[sn].q;
    if (!iscomp(mi)) {
      x1[1]=desk[mi].playx+c*desk[mi].cardx;
      y1[1]=zw?desk[mi].playy:desk[mi].skaty;
      x2[1]=desk[mi].playx+c1*desk[mi].cardx;
      y2[1]=zw?desk[mi].skaty:desk[mi].playy;
      d[1]=(zw?4:-4)*desk[mi].f/desk[mi].q;
    }
    if (tauschcard!=-1) {
      if ((zw?mi:sn)==tauschcard/10) {
	if (c==c1) return 1;
	putdesk(sn,x2[0],y1[0]+d[0]);
	putcard(sn,cards[tauschcard],x2[0],y1[0]);
	if (!iscomp(mi)) {
	  putdesk(mi,x2[1],y1[1]-d[1]);
	  putcard(mi,cards[tauschcard],x2[1],y1[1]);
	}
      }
      else {
	swap(&cards[tauschcard],&cards[10*(zw?mi:sn)+c]);
	sna[0]=sn;
	if (!iscomp(mi)) sna[1]=mi;
	movecard(iscomp(mi)?1:2,sna,x1,y1,x2,y2);
	putdesk(sn,x2[0],y2[0]-d[0]);
	if (!iscomp(mi)) putdesk(mi,x2[1],y2[1]+d[1]);
	revolutionsort(0);
	tauschcard=-1;
	if (tauschdone) {
	  tauschdone=0;
	}
	return 1;
      }
    }
    tauschcard=10*(zw?mi:sn)+c;
    putdesk(sn,x1[0],y1[0]);
    putcard(sn,cards[tauschcard],x1[0],y1[0]+d[0]);
    if (!iscomp(mi)) {
      putdesk(mi,x1[1],y1[1]);
      putcard(mi,cards[tauschcard],x1[1],y1[1]-d[1]);
    }
    return 1;
  }
  if (x>=desk[sn].pboxx+31*desk[sn].f/desk[sn].q &&
      x<=desk[sn].pboxx+96*desk[sn].f/desk[sn].q &&
      y>=desk[sn].pboxy+1 &&
      y<=desk[sn].pboxy+16*desk[sn].f/desk[sn].q) inv_fbox(sn,1);
  else return 0;
  stdwait();
  inv_fbox(sn,0);
  rem_fbox(sn);
  tauschdone|=left(sn)==spieler?1:2;
  if (iscomp(mi) || tauschdone==3) {
    clr_desk(1);
    spielphase();
  }
  else {
    tauschply=mi;
    put_fbox(mi,TX_FERTIG);
  }
  return 1;
}

int hndl_spielen(sn,x,y)
int sn,x,y;
{
  int i,c;

  c=card_at(sn,x,y,0);
  if (c) {
    di_delres(sn);
    c--;
    calc_poss(sn);
    for (i=0;i<possc;i++) {
      if (10*sn+c==possi[i]) {
	if (hints[sn]) show_hint(sn,0,0);
        drop_card(10*sn+c,sn);
        do_next();
        break;
      }
    }
    return 1;
  }
  return 0;
}

int hndl_nimmstich(sn)
int sn;
{
  nimmstich[sn][1]=0;
  phase=SPIELEN;
  for (sn=0;sn<numsp;sn++) {
    if (nimmstich[sn][1]) {
      phase=NIMMSTICH;
    }
  }
  if (phase==SPIELEN) {
    next_stich();
  }
  return 1;
}

int hndl_button(sn,x,y,opt,send)
int sn,x,y,opt,send;
{
  int ok=0;
  static int errcnt[3];

  if (opt==2) {
    di_options(sn);
    errcnt[sn]=0;
    return 0;
  }
  if (phase==REIZEN) {
    if ((saho && sn==sager) || (!saho && sn==hoerer)) ok=hndl_reizen(sn,x,y);
  }
  else if (phase==DRUECKEN) {
    if (sn==spieler) ok=hndl_druecken(sn,x,y);
  }
  else if (phase==SPIELEN) {
    if (sn==(ausspl+vmh)%3) ok=hndl_spielen(sn,x,y);
  }
  else if (phase==NIMMSTICH) {
    if (nimmstich[sn][1]) ok=hndl_nimmstich(sn);
  }
  else if (phase==REVOLUTION) {
    if (sn==tauschply) ok=hndl_tauschen(sn,x,y);
  }
  if (!ok) {
    if (irc_play && sn!=irc_pos) return ok;
    if (opt==1) {
      di_options(sn);
      errcnt[sn]=0;
    }
    else {
      errcnt[sn]++;
      if (errcnt[sn]>=3) {
	di_menubutton(sn);
	errcnt[sn]=0;
      }
    }
  }
  else {
    errcnt[sn]=0;
    if (send && irc_play) irc_sendxyev(sn,x,y,opt);
  }
  return ok;
}

VOID setcurs()
{
  int x,y,w,sn,snn,newsn=-1;
  char clr[100];
  static int actsn=-1,wsn=-1,wtime;

  for (sn=0;sn<numsp;sn++) {
    switch (phase) {
    case REIZEN:
      if ((saho && sn==sager) || (!saho && sn==hoerer)) newsn=sn;
      break;
    case HANDSPIEL:
    case DRUECKEN:
    case ANSAGEN:
    case RESULT:
      if (phase==ANSAGEN && ktrply>=0) newsn=ktrply;
      else if (sn==spieler) newsn=sn;
      if (numsp==1) newsn=0;
      break;
    case SPIELEN:
      if (sn==(ausspl+vmh)%3) newsn=sn;
      break;
    case SCHENKEN:
      newsn=schenkply;
      break;
    case NIMMSTICH:
      for (snn=0;snn<numsp;snn++) {
	if (nimmstich[snn][1]) newsn=snn;
      }
      break;
    case SPIELDICHT:
      newsn=spieler;
      break;
    case WEITER:
      newsn=-1;
      break;
    case REVOLUTION:
      newsn=tauschply;
      break;
    }
  }
  if (newsn!=actsn) {
    wtime=0;
    if (actsn>=0) XDefineCursor(dpy[actsn],win[actsn],cursor[actsn][1]);
    actsn=newsn;
    if (actsn>=0) {
      for (sn=0;sn<numsp;sn++) {
	if (phase!=WEITER && phase!=REVOLUTION) di_info(sn,actsn);
      }
      XDefineCursor(dpy[actsn],win[actsn],cursor[actsn][0]);
    }
  }
  if (numsp==1) return;
  if (actsn==-1) wtime=0;
  if (!wtime || wtime>=15*1000) {
    if (wsn!=-1 && (actsn!=wsn ||
		    wtime==16*1000 ||
		    (phase!=SPIELEN && phase!=NIMMSTICH))) {
      x=desk[wsn].w/2;
      y=desk[wsn].playy-2*charh[wsn];
      w=5*desk[wsn].cardw/XTextWidth(dfont[wsn]," ",1)+1;
      if (w>99) w=99;
      clr[w]=0;
      while (w) clr[--w]=' ';
      v_gtext(wsn,x,y,0,clr);
      wsn=-1;
      if (wtime==16*1000) wtime-=1500;
      if (phase!=SPIELEN && phase!=NIMMSTICH) wtime=0;
    }
    if (wtime==15*1000 && (phase==SPIELEN || phase==NIMMSTICH)) {
      wsn=actsn;
      x=desk[wsn].w/2;
      y=desk[wsn].playy-2*charh[wsn];
      v_gtext(wsn,x,y,0,textarr[TX_DU_BIST_DRAN]);
    }
  }
  wtime+=50;
}
