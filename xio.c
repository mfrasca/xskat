
/*
    xskat - a card game for 1 to 3 players.
    Copyright (C) 2004  Gunter Gerhardt

    This program is free software; you can redistribute it freely.
    Use it at your own risk; there is NO WARRANTY.

    Redistribution of modified versions is permitted
    provided that the following conditions are met:
    1. All copyright & permission notices are preserved.
    2.a) Only changes required for packaging or porting are made.
      or
    2.b) It is clearly stated who last changed the program.
         The program is renamed or
         the version number is of the form x.y.z,
         where x.y is the version of the original program
         and z is an arbitrary suffix.
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
#ifdef __EMX__ /* XFree OS/2 */
#include <sys/select.h>
#endif
#include <sys/utsname.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
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
  y+=(charh[sn]+dfont[sn]->ascent-dfont[sn]->descent)/2+1-gfx3d[sn];
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
		 64*desk[sn].f/desk[sn].q-8,charh[sn]-2);
  XFillRectangle(dpy[sn],bck[sn],gc[sn],x,y,
		 64*desk[sn].f/desk[sn].q-8,charh[sn]-2);
  change_gc(sn,fgpix[sn],gc);
}

VOID b_text(s,str)
int s;
tx_typ *str;
{
  int sn,x;

  for (sn=0;sn<numsp;sn++) {
    if (sn!=s) {
      x=s==left(sn)?desk[sn].cbox1x:desk[sn].cbox2x;
      clr_text(sn,x,desk[sn].cboxy);
      v_gtextc(sn,1,x,desk[sn].cboxy,desk[sn].cardw,str->t[lang[sn]]);
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
	   textarr[TX_PASSE].t[lang[sn]]);
}

VOID draw_skat(sn)
int sn;
{
  putcard(sn,cards[30],desk[sn].skatx,desk[sn].skaty);
  putcard(sn,cards[31],desk[sn].skatx+desk[sn].cardw,
	  desk[sn].skaty);
  skatopen=1;
}

VOID home_skat()
{
  int sn=spieler;

  homecard(sn,0,0);
  homecard(sn,0,1);
  umdrueck=skatopen=0;
  backopen[0]=backopen[1]=backopen[2]=1;
  spitzeopen=1;
}

VOID nimm_stich()
{
  int sn=ausspl,i;

  for (i=0;i<3;i++) {
    homecard(sn,1,i);
  }
  stichopen=0;
}

VOID drop_card(i,s)
int i,s;
{
  int sn,sna[3],x1[3],y1[3],x2[3],y2[3];
  static int l2r[3];

  if (stich==10) backopen[s]=0;
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
      if (s==spieler) spitzeopen=0;
    }
    else if (spitzeang && cards[i]==(trumpf==4?BUBE:SIEBEN|trumpf<<3)) {
      putback(sn,x1[sn],y1[sn]);
      spitzeopen=0;
      sptzmrk=1;
      putamark(sn,spieler);
    }
    if (!vmh) l2r[sn]=trickl2r[sn];
    x2[sn]=desk[sn].stichx+
      (l2r[sn]?vmh:s==left(sn)?0:s==sn?1:2)*desk[sn].cardw;
    y2[sn]=desk[sn].stichy;
  }
  movecard(numsp,sna,x1,y1,x2,y2);
  for (sn=0;sn<numsp;sn++) {
    putcard(sn,cards[i],x2[sn],desk[sn].stichy);
  }
  stcd[vmh]=cards[i];
  stichopen=vmh+1;
  gespcd[cards[i]]=2;
  if ((cards[i]&7)!=BUBE) gespfb[cards[i]>>3]++;
  cards[i]=-1;
  stdwait();
}

int query_err(d,e)
Display *d;
XErrorEvent *e;
{
  colerr=1;
  return 0;
}

int closest_col(sn,xcol)
int sn;
XColor *xcol;
{
  static int f[3];
  static XColor xcm[3][256];
  int i,j,k,pl;
  long r,g,b;
  int ps[9],sp;

  if (XAllocColor(dpy[sn],cmap[sn],xcol)) return 1;
  pl=desk[sn].plan;
  if (pl<2 || pl>8 || wpix[sn]>=1<<pl || bpix[sn]>=1<<pl) return 0;
  if (!f[sn]) {
    for (i=0;i<256;i++) xcm[sn][i].pixel=i;
    colerr=0;
    XSetErrorHandler(query_err);
    XQueryColors(dpy[sn],cmap[sn],xcm[sn],1<<pl);
    XSync(dpy[sn],0);
    XSetErrorHandler(NULL);
    if (colerr) return 0;
    f[sn]=1;
  }
  r=xcol->red;
  g=xcol->green;
  b=xcol->blue;
  for (sp=0;sp<(1<<pl)/32+1;sp++) {
    j=0;
    for (k=0;k<sp;k++) {
      if (ps[k]==j) {
	j++;
	k=-1;
      }
    }
    for (i=j+1;i<1<<pl;i++) {
      if (abs(xcm[sn][i].red-r)+
	  abs(xcm[sn][i].green-g)+
	  abs(xcm[sn][i].blue-b)<
	  abs(xcm[sn][j].red-r)+
	  abs(xcm[sn][j].green-g)+
	  abs(xcm[sn][j].blue-b)) {
	for (k=0;k<sp && ps[k]!=i;k++);
	if (k==sp) j=i;
      }
    }
    xcol->red=xcm[sn][j].red;
    xcol->green=xcm[sn][j].green;
    xcol->blue=xcm[sn][j].blue;
    if (XAllocColor(dpy[sn],cmap[sn],xcol)) return 1;
    ps[sp]=j;
  }
  return 0;
}

unsigned long get_col(sn,ucol,prog,col,defcol,defpix,xcol)
int sn;
char *ucol,*prog,*col,*defcol;
unsigned long defpix;
XColor *xcol;
{
  char *spec;

  spec=XGetDefault(dpy[sn],prog,col);
  if ((ucol && XParseColor(dpy[sn],cmap[sn],ucol,xcol) &&
       closest_col(sn,xcol)) ||
      (spec && XParseColor(dpy[sn],cmap[sn],spec,xcol) &&
       closest_col(sn,xcol)) ||
      (defcol && XParseColor(dpy[sn],cmap[sn],defcol,xcol) &&
       closest_col(sn,xcol))) {
    if (desk[sn].col>2) {
      color[sn][desk[sn].col++]=*xcol;
    }
    return xcol->pixel;
  }
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
  int ln;

  for (ln=0;ln<NUM_LANG;ln++) {
    extractnamln(sn,str,ln);
  }
}

VOID extractnamln(sn,str,ln)
int sn;
char *str;
int ln;
{
  char *eos;
  int z,s;

  spnames[sn][0][ln][0]=0;
  spnames[sn][1][ln][0]=0;
  if (!str) str="";
  if (!(eos=strchr(str,'@')) && !(eos=strchr(str,':'))) eos=str+strlen(str);
  for (z=0;z<2 && str!=eos;z++) {
    while (*str==' ' || *str=='~') str++;
    for (s=0;s<9 && str!=eos && *str!=' ' && *str!='~';s++,str++) {
      spnames[sn][z][ln][s]=*str;
    }
    spnames[sn][z][ln][s]=0;
  }
}

VOID usage()
{
  fprintf(stderr,"\
xskat [-display|-d display] [-geometry|-g geometry] [-font|-fn font]\n\
  [-iconic|-i] [-title|-T string] [-name prog] [-fg color] [-bg color]\n\
  [-bt color] [-3d] [-2d] [-3dtop color] [-3dbot color] [-mark color]\n\
  [-mb button] [-keyboard 0..2] [-tdelay sec] [-fastdeal] [-slowdeal]\n\
  [-help|-h] [-frenchcards] [-french4cards] [-germancards] [-german4cards]\n\
  [-color] [-mono] [-color1 color] .. [-color4 color] [-large] [-small]\n\
  [-up] [-down] [-alt] [-seq] [-list|-l filename] [-alist] [-nlist] [-tlist]\n\
  [-log filename] [-dolog] [-nolog] [-fmt] [-unfmt] [-game filename]\n\
  [-briefmsg] [-verbosemsg] [-trickl2r] [-notrickl2r]\n\
  [-lang language] [-start player#] [-s1 -4..4] [-s2 -4..4] [-s3 -4..4]\n\
  [-ramsch] [-noramsch] [-ramschonly] [-sramsch] [-nosramsch]\n\
  [-skattolast] [-skattoloser]\n\
  [-kontra] [-nokontra] [-kontra18] [-bock] [-nobock] [-bockramsch]\n\
  [-bockevents 1..255] [-resumebock] [-noresumebock]\n\
  [-spitze] [-spitze2] [-nospitze] [-revolution] [-norevolution]\n\
  [-klopfen] [-noklopfen] [-schenken] [-noschenken] [-hint] [-nohint]\n\
  [-newrules] [-oldrules] [-shortcut] [-noshortcut] [-askshortcut]\n\
  [-irc] [-noirc] [-irctelnet program] [-ircserver host]\n\
  [-ircport number] [-ircchannel name] [-ircnick name] [-ircuser name]\n\
  [-ircrealname name] [-ircpos number]\n\
  [-irclog file] [-irclogappend] [-irclogoverwrite]\n\
  [-auto #ofgames] [-opt filename] [-pk] [player@display...]\n\
After starting the game a mouse click or ESC/F1 will bring up a menu.\n\
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
  int sn,es=0;

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

VOID startirc(f)
int f;
{
  char *argv[100];
  int i,j;

  j=0;
  if (f) {
    argv[j++]="xterm";
    argv[j++]="-e";
  }
  for (i=j;i<90 && i-j<theargc;i++) {
    argv[i]=theargv[i-j];
  }
  if (f) {
    argv[i++]="-irc";
  }
  else {
    if (irc_telnetpid) {
      kill(irc_telnetpid,SIGHUP);
    }
  }
  argv[i]=0;
  execvp(argv[0],argv);
  fprintf(stderr,"%s not found\n",argv[0]);
  exitus(0);
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

int getcode(bpos,csiz,msk,thegif)
int *bpos,csiz,msk;
unsigned char *thegif;
{
  int pos;
  long c;

  pos=*bpos>>3;
  c=thegif[pos]+(thegif[pos+1]<<8);
  if (csiz>=8) c+=(long)thegif[pos+2]<<16;
  c>>=*bpos&7;
  *bpos+=csiz;
  return c&msk;
}

VOID decompgif(thedata,thepic,themap,cmapsize)
unsigned char *thedata,*thepic,**themap;
int *cmapsize;
{
  unsigned char b,*p;
  int i,bpos,cnt,csiz,isiz,cd,mc,cc,ec;
  int ac,pc,ic,ff,fc,lc,bmsk,msk;
  static int a[4096],e[4096],c[4097];
  static unsigned char thegif[138*88];

  bpos=cnt=pc=lc=0;
  thedata+=10;
  *cmapsize=1<<((*thedata++&7)+1);
  bmsk=*cmapsize-1;
  thedata+=2;
  *themap=thedata;
  thedata+=*cmapsize*3+10;
  ec=(cc=1<<(csiz=*thedata++))+1;
  fc=ff=cc+2;
  isiz=++csiz;
  msk=(mc=1<<csiz)-1;
  p=thegif;
  while ((b=*thedata++)) while (b--) *p++=*thedata++;
  cd=getcode(&bpos,csiz,msk,thegif);
  while (cd!=ec) {
    if (cd==cc) {
      msk=(mc=1<<(csiz=isiz))-1;
      fc=ff;
      ac=pc=cd=getcode(&bpos,csiz,msk,thegif);
      *thepic++=lc=ac&bmsk;
    }
    else {
      ac=ic=cd;
      if (ac>=fc) {
	ac=pc;
	c[cnt++]=lc;
      }
      while (ac>bmsk) {
	c[cnt++]=e[ac];
	ac=a[ac];
      }
      c[cnt]=lc=ac&bmsk;
      for (i=cnt;i>=0;i--) *thepic++=c[i];
      cnt=0;
      a[fc]=pc;
      e[fc++]=lc;
      pc=ic;
      if (fc>=mc && csiz<12) {
	msk=(1<<++csiz)-1;
	mc<<=1;
      }
    }
    cd=getcode(&bpos,csiz,msk,thegif);
  }
}

VOID drawimg(sn,c,f,w,x,y)
int sn,c,f,w,x,y;
{
  long i,j,k;
  int l,p,r,g,b,m,s,gr,tc,idx,ld,pd,an;
  static unsigned char thepic[138][88];
  static int fsdbuf[2][138][3];
  static int smlbuf[138][3];
  static int smcbuf[138][3];
  static XPoint xp[256][32];
  unsigned long pm[256];
  static int pc[16][16][16];
  unsigned char (*themap)[256][3];
  int xn[256];
  XColor xc,*xcp;
  int cmapsize;

  decompgif(f<0?backsd_gif[0]:blatt[sn]>=2?de_gif[f][w]:fr_gif[f][w],
	    (unsigned char *)thepic,(unsigned char **)&themap,&cmapsize);
  tc=desk[sn].plan>=12 && desk[sn].col>2 && desk[sn].large;
  xcp=&color[sn][0];
  if (!tc) {
    for (i=0;i<=0xffff;i+=0x1111) {
      for (j=0;j<=0xffff;j+=0x1111) {
	for (k=0;k<=0xffff;k+=0x1111) {
	  l=0;
	  ld=abs(xcp[l].red-i)+abs(xcp[l].green-j)+abs(xcp[l].blue-k);
	  for (p=1;p<desk[sn].col;p++) {
	    pd=abs(xcp[p].red-i)+abs(xcp[p].green-j)+abs(xcp[p].blue-k);
	    if (pd<ld) {
	      l=p;
	      ld=pd;
	    }
	  }
	  pc[i/0x1111][j/0x1111][k/0x1111]=l;
	}
      }
    }
  }
  xcp[desk[sn].col].pixel=wpix[sn];
  for (i=0;i<256;i++) {
    xn[i]=0;
  }
  if (tc) {
    for (i=0;i<cmapsize;i++) {
      xc.red=(*themap)[i][0];
      xc.green=(*themap)[i][1];
      xc.blue=(*themap)[i][2];
      if (blatt[sn]==3 && f==3 && xc.red>220 && xc.green<100 && xc.blue<100) {
	if (xc.blue<50) xc.blue=50;
	xc.green+=xc.blue+50;
	xc.blue=0;
      }
      if (xc.red==255 && xc.green==255 && xc.blue==255) {
	pm[i]=wpix[sn];
      }
      else {
	xc.red<<=8;
	xc.green<<=8;
	xc.blue<<=8;
	pm[i]=closest_col(sn,&xc)?xc.pixel:bpix[sn];
      }
    }
  }
  if (f<0 || blatt[sn]<2) {
    gr=f<0;
    m=0;
  }
  else {
    if (ggcards) {
      gr=ggde_flg[f][w]>>1;
      m=ggde_flg[f][w]&1;
    }
    else {
      gr=de_flg[f][w]>>1;
      m=de_flg[f][w]&1;
    }
  }
  idx=0;
  for (j=0;j<138;j++) {
    fsdbuf[idx][j][0]=fsdbuf[idx][j][1]=fsdbuf[idx][j][2]=0;
    smlbuf[j][0]=-1;
    smcbuf[j][0]=-1;
  }
  idx=1-idx;
  for (i=(desk[sn].large?87:86+(f<0));i>=0;i--) {
    s=f<0 && i>43?1:2;
    k=!i && m?desk[sn].large?88:58:0;
    if (!desk[sn].large && !((i+s)%3)) {
      for (j=(gr?137:68);j>=0;j--) {
	p=thepic[j][i];
	smlbuf[j][0]=(*themap)[p][0];
	smlbuf[j][1]=(*themap)[p][1];
	smlbuf[j][2]=(*themap)[p][2];
      }
      continue;
    }
    for (j=0;j<138;j++) {
      fsdbuf[idx][j][0]=fsdbuf[idx][j][1]=fsdbuf[idx][j][2]=0;
    }
    idx=1-idx;
    for (j=(gr?137:68);j>=0;j--) {
      p=thepic[j][i];
      if (!desk[sn].large && !((j+2)%3)) {
	if (j) {
	  smcbuf[j-1][0]=(*themap)[p][0];
	  smcbuf[j-1][1]=(*themap)[p][1];
	  smcbuf[j-1][2]=(*themap)[p][2];
	  fsdbuf[idx][j-1][0]+=fsdbuf[idx][j][0];
	  fsdbuf[idx][j-1][1]+=fsdbuf[idx][j][1];
	  fsdbuf[idx][j-1][2]+=fsdbuf[idx][j][2];
	}
	continue;
      }
      if (!tc) {
	r=(*themap)[p][0];
	g=(*themap)[p][1];
	b=(*themap)[p][2];
	an=1;
	if (smlbuf[j][0]>=0) {
	  r+=smlbuf[j][0];
	  g+=smlbuf[j][1];
	  b+=smlbuf[j][2];
	  smlbuf[j][0]=-1;
	  an++;
	}
	if (smcbuf[j][0]>=0) {
	  r+=smcbuf[j][0];
	  g+=smcbuf[j][1];
	  b+=smcbuf[j][2];
	  smcbuf[j][0]=-1;
	  an++;
	}
	if (an>1) {
	  r/=an;
	  g/=an;
	  b/=an;
	}
	if (blatt[sn]==3 && f==3 && r>220 && g<100 && b<100) {
	  if (b<50) b=50;
	  g+=b+50;
	  b=0;
	}
	if (r==255 && g==255 && b==255) {
	  p=desk[sn].col;
	  r=g=b=0;
	}
	else {
	  if (desk[sn].col<=2) {
	    if (r>=127) r=r*19/16;
	    if (g>=127) g=g*19/16;
	    if (b>=127) b=b*19/16;
	  }
	  r+=fsdbuf[idx][j][0];
	  if (r<0) r=0;
	  else if (r>255) r=255;
	  g+=fsdbuf[idx][j][1];
	  if (g<0) g=0;
	  else if (g>255) g=255;
	  b+=fsdbuf[idx][j][2];
	  if (b<0) b=0;
	  else if (b>255) b=255;
	  if (desk[sn].col<=2) {
	    if (r<127) r=r*11/16;
	    if (g<127) g=g*11/16;
	    if (b<127) b=b*11/16;
	  }
	  p=pc[r/0x11][g/0x11][b/0x11];
	  r-=xcp[p].red>>8;
	  g-=xcp[p].green>>8;
	  b-=xcp[p].blue>>8;
	}
      }
      else {
	r=g=b=0;
      }
      if (desk[sn].large) {
	xp[p][xn[p]].x=x+i+1;
	xp[p][xn[p]].y=y+j+1;
	xn[p]++;
	if (!gr) {
	  xp[p][xn[p]].x=x+88-i+m-k;
	  xp[p][xn[p]].y=y+138-j;
	  xn[p]++;
	}
      }
      else {
	xp[p][xn[p]].x=x+(i+s)*2/3-m+k;
	xp[p][xn[p]].y=y+(j+2)*2/3;
	xn[p]++;
	if (!gr) {
	  xp[p][xn[p]].x=x+59-(i+s)*2/3+m-k;
	  xp[p][xn[p]].y=y+93-(j+2)*2/3;
	  xn[p]++;
	}
      }
      if (j) {
	fsdbuf[idx][j-1][0]+=r*10/27;
	fsdbuf[idx][j-1][1]+=g*10/27;
	fsdbuf[idx][j-1][2]+=b*10/27;
	fsdbuf[1-idx][j-1][0]+=r-r*20/27;
	fsdbuf[1-idx][j-1][1]+=g-g*20/27;
	fsdbuf[1-idx][j-1][2]+=b-b*20/27;
      }
      fsdbuf[1-idx][j][0]+=r*10/27;
      fsdbuf[1-idx][j][1]+=g*10/27;
      fsdbuf[1-idx][j][2]+=b*10/27;
      if (xn[p]==32) {
	change_gc(sn,tc?pm[p]:xcp[p].pixel,gc);
	XDrawPoints(dpy[sn],cardpx[sn][c+1],gc[sn],xp[p],32,CoordModeOrigin);
	xn[p]=0;
      }
    }
  }
  for (p=0;p<256;p++) {
    if (xn[p]) {
      change_gc(sn,tc?pm[p]:xcp[p].pixel,gc);
      XDrawPoints(dpy[sn],cardpx[sn][c+1],gc[sn],xp[p],xn[p],CoordModeOrigin);
    }
  }
}

VOID create_card(sn,c)
int sn,c;
{
  int i,j,p,x,y,x1,y1,x2,y2,x3,y3,f,pf,upf,w,ww,hh;

  x=2;
  y=0;
  f=c>>3;
  pf=blatt[sn]==1?f==0?4:f==2?5:f:f;
  upf=desk[sn].col>pf && desk[sn].col>2;
  w=c&7;
  XFillRectangle(dpy[sn],cardpx[sn][c+1],gcbck[sn],0,0,
		 desk[sn].cardw,desk[sn].cardh);
  if (((KOENIG<=w && w<=BUBE) || c<0 || blatt[sn]>=2)) {
    drawimg(sn,c,c<0?c:f,w-(blatt[sn]>=2?0:KOENIG),x,y);
  }
  else {
    change_gc(sn,wpix[sn],gc);
    XFillRectangle(dpy[sn],cardpx[sn][c+1],gc[sn],x+1,y+1,
		   60*desk[sn].f/desk[sn].q-2,92*desk[sn].f/desk[sn].q);
  }
  change_gc(sn,bpix[sn],gc);
  if (desk[sn].large) {
    ww=89;
    hh=139;
  }
  else {
    ww=59;
    hh=93;
  }
  for (i=(desk[sn].large?8:5);i<(desk[sn].large?82:55);i++) {
    XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn], x+i, y);
    XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn], x+i, y+hh);
  }
  for (j=(desk[sn].large?8:5);j<(desk[sn].large?132:89);j++) {
    XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn], x,    y+j);
    XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn], x+ww, y+j);
  }
  for (i=0;i<(desk[sn].large?9:5);i++) {
    XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn],
	       x+frm[desk[sn].large][i][0],    y+frm[desk[sn].large][i][1]);
    XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn],
	       x+ww-frm[desk[sn].large][i][0], y+hh-frm[desk[sn].large][i][1]);
    XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn],
	       x+frm[desk[sn].large][i][0],    y+hh-frm[desk[sn].large][i][1]);
    XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn],
	       x+ww-frm[desk[sn].large][i][0], y+frm[desk[sn].large][i][1]);
  }
  change_gc(sn,bgpix[sn],gc);
  for (i=1;i<(desk[sn].large?5:3);i++) {
    for (j=1;j<(desk[sn].large?6:4)-i;j++) {
      XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn], x+i,    y+j);
      XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn], x+ww-i, y+hh-j);
      XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn], x+i,    y+hh-j);
      XDrawPoint(dpy[sn],cardpx[sn][c+1], gc[sn], x+ww-i, y+j);
    }
  }
  change_gc(sn,fgpix[sn],gc);
  if (c>=0) {
    if (blatt[sn]<2) {
      change_gcxor(sn,(upf?color[sn][pf].pixel:bpix[sn])^
		   wpix[sn]^bgpix[sn]);
      p=cnts[w];
      do {
	x1=f*16*desk[sn].f/desk[sn].q;
	y1=0;
	if (bigs[p+1]>70) x1+=64*desk[sn].f/desk[sn].q;
	x2=x1+16*desk[sn].f/desk[sn].q;
	y2=y1+16*desk[sn].f/desk[sn].q;
	x3=bigs[p++];
	y3=bigs[p++];
	if (!desk[sn].large) {
	  x3=x3*2/3+(KOENIG<=w && w<=BUBE && p-2!=cnts[w]?1:0);
	  y3=y3*2/3+(KOENIG<=w && w<=BUBE && p-2!=cnts[w]?1:0);
	}
	if (upf) {
	  XCopyPlane(dpy[sn],symbs[sn],cardpx[sn][c+1],gcxor[sn],
		     x1+2*desk[sn].cardw,y1,x2-x1,y2-y1,x+x3,y+y3,1);
	}
	else {
	  XCopyPlane(dpy[sn],symbs[sn],cardpx[sn][c+1],gcxor[sn],
		     x1,y1,x2-x1,y2-y1,x+x3,y+y3,1);
	}
      } while (p!=cnts[w+1]);
    }
    else {
      change_gcxor(sn,bpix[sn]^wpix[sn]^bgpix[sn]);
    }
    for (p=0;p<8;p+=2) {
      if (blatt[sn]<2) {
	x1=f*8*desk[sn].f/desk[sn].q;
	y1=16*desk[sn].f/desk[sn].q;
	if (smls[p+1]>50) x1+=32*desk[sn].f/desk[sn].q;
	x2=x1+8*desk[sn].f/desk[sn].q;
	y2=y1+8*desk[sn].f/desk[sn].q;
	if (KOENIG<=w && w<=BUBE) {
	  x3=smls[p];
	  y3=smls[p+1];
	}
	else {
	  x3=smlz[p];
	  y3=smlz[p+1]+(!desk[sn].large && p>3?1:0);
	}
	if (!desk[sn].large) {
	  x3=x3*2/3+(p&2?1:0);
	  y3=y3*2/3+(p>3?1:0);
	}
	if (upf) {
	  XCopyPlane(dpy[sn],symbs[sn],cardpx[sn][c+1],gcxor[sn],
		     x1+2*desk[sn].cardw,y1,x2-x1,y2-y1,x+x3,y+y3,1);
	}
	else {
	  XCopyPlane(dpy[sn],symbs[sn],cardpx[sn][c+1],gcxor[sn],
		     x1,y1,x2-x1,y2-y1,x+x3,y+y3,1);
	}
      }
      if (blatt[sn]<2) {
	x1=256+(w==AS?lang[sn]*20:w==ZEHN?p>3?23:0:w<=BUBE?
		(w-KOENIG+1)*5+lang[sn]*20:(p>3?31:8)+(w-NEUN)*5);
      }
      else {
	x1=256+(w==ZEHN?p>3?23:0:w<=BUBE?(w-KOENIG+1)*5+35:
		(p>3?31:8)+(w-NEUN)*5);
      }
      x1=x1*desk[sn].f/desk[sn].q;
      y1=(w!=ZEHN && w<=BUBE?p>3?7:0:14)+1;
      y1=y1*desk[sn].f/desk[sn].q;
      x2=x1+(w==ZEHN?8:5)*desk[sn].f/desk[sn].q;
      y2=y1+7*desk[sn].f/desk[sn].q;
      x3=smlc[p]-(w==ZEHN?p&2?4:2:0);
      y3=smlc[p+1];
      if (!desk[sn].large) {
	x3=x3*2/3+(w==ZEHN?p&2?0:2:0);
	y3=y3*2/3+(p>3?1:0);
      }
      if (blatt[sn]<2 || w!=AS) {
	XCopyPlane(dpy[sn],symbs[sn],cardpx[sn][c+1],gcxor[sn],
		   x1,y1,x2-x1,y2-y1,x+x3,y+y3,1);
      }
    }
  }
  change_gcxor(sn,fgpix[sn]);
}

VOID xinitwin(sn,argc,argv)
int sn,argc;
char **argv;
{
  Pixmap icon,iconmask;
  XClassHint classhint;
  XGCValues gcv;
  int i,gcvf;
  GC igc[3];

  win[sn]=XCreateSimpleWindow(dpy[sn],DefaultRootWindow(dpy[sn]),
			      desk[sn].x,desk[sn].y,desk[sn].w,desk[sn].h,
			      0,fgpix[sn],bgpix[sn]);
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
  symbs[sn]=XCreateBitmapFromData(dpy[sn],win[sn],(char *)symbs_bits,
				  symbs_width,symbs_height);
  if (symbs[sn]==None) nomem();
  if (desk[sn].col>3) {
    icon=XCreatePixmap(dpy[sn],win[sn],icon_width,icon_height,desk[sn].plan);
    iconmask=XCreatePixmap(dpy[sn],win[sn],icon_width,icon_height,1);
    if (icon==None || iconmask==None) nomem();
    gcv.foreground=0;
    gcv.background=0;
    igc[sn]=XCreateGC(dpy[sn],iconmask,gcvf,&gcv);
    XFillRectangle(dpy[sn],iconmask,igc[sn],0,0,icon_width,icon_height);
    change_gc(sn,1,igc);
    XCopyPlane(dpy[sn],symbs[sn],iconmask,igc[sn],128,0,16,16,1,1,1);
    XCopyPlane(dpy[sn],symbs[sn],iconmask,igc[sn],128+16,0,16,16,16,16,1);
    XCopyPlane(dpy[sn],symbs[sn],iconmask,igc[sn],128+32,0,16,16,16,1,1);
    XCopyPlane(dpy[sn],symbs[sn],iconmask,igc[sn],128+48,0,16,16,1,16,1);
    XFreeGC(dpy[sn],igc[sn]);
    wmhints.flags|=IconMaskHint;
    wmhints.icon_mask=iconmask;
    change_gc(sn,wpix[sn],gc);
    XFillRectangle(dpy[sn],icon,gc[sn],0,0,icon_width,icon_height);
    change_gcxor(sn,color[sn][0].pixel^wpix[sn]^bgpix[sn]);
    XCopyPlane(dpy[sn],symbs[sn],icon,gcxor[sn],128,0,16,16,1,1,1);
    change_gcxor(sn,color[sn][1].pixel^wpix[sn]^bgpix[sn]);
    XCopyPlane(dpy[sn],symbs[sn],icon,gcxor[sn],128+16,0,16,16,16,16,1);
    change_gcxor(sn,color[sn][2].pixel^wpix[sn]^bgpix[sn]);
    XCopyPlane(dpy[sn],symbs[sn],icon,gcxor[sn],128+32,0,16,16,16,1,1);
    change_gcxor(sn,color[sn][3].pixel^wpix[sn]^bgpix[sn]);
    XCopyPlane(dpy[sn],symbs[sn],icon,gcxor[sn],128+48,0,16,16,1,16,1);
    change_gcxor(sn,fgpix[sn]);
    change_gc(sn,fgpix[sn],gc);
  }
  else {
    icon=XCreateBitmapFromData(dpy[sn],DefaultRootWindow(dpy[sn]),
			       (char *)icon_bits,
			       icon_width,icon_height);
    if (icon==None) nomem();
  }
  classhint.res_name=prog_name;
  classhint.res_class=prog_name;
  XSetClassHint(dpy[sn],win[sn],&classhint);
  XSetStandardProperties(dpy[sn],win[sn],title[sn],title[sn],icon,
			 argv,argc,&szhints[sn]);
#ifdef PWinGravity
  szhints[sn].win_gravity=
    (geom_f[sn]&XNegative?
     (geom_f[sn]&YNegative?
    SouthEastGravity:NorthEastGravity):
     (geom_f[sn]&YNegative?
    SouthWestGravity:NorthWestGravity));
  szhints[sn].flags|=PWinGravity;
  XSetWMNormalHints(dpy[sn],win[sn],&szhints[sn]);
#endif
  if (szhints[sn].flags&USPosition) {
    XMoveWindow(dpy[sn],win[sn],szhints[sn].x,szhints[sn].y);
  }
  wmhints.flags|=IconPixmapHint;
  wmhints.icon_pixmap=icon;
  XSetWMHints(dpy[sn],win[sn],&wmhints);
  if (desk[sn].large) {
    XFreePixmap(dpy[sn],symbs[sn]);
    symbs[sn]=XCreateBitmapFromData(dpy[sn],win[sn],(char *)symbl_bits,
				    symbl_width,symbl_height);
    if (symbs[sn]==None) nomem();
  }
  bck[sn]=XCreatePixmap(dpy[sn],win[sn],desk[sn].w,desk[sn].h,desk[sn].plan);
  if (bck[sn]==None) nomem();
  for (i=0;i<33;i++) {
    cardpx[sn][i]=XCreatePixmap(dpy[sn],win[sn],
				desk[sn].cardw,desk[sn].cardh,
				desk[sn].plan);
    if (cardpx[sn][i]==None) nomem();
    create_card(sn,i-1);
  }
  XFillRectangle(dpy[sn],win[sn],gcbck[sn],0,0,desk[sn].w,desk[sn].h);
  XFillRectangle(dpy[sn],bck[sn],gcbck[sn],0,0,desk[sn].w,desk[sn].h);
  if (!irc_play || irc_pos==sn) {
    XSelectInput(dpy[sn],win[sn],ButtonPressMask|ExposureMask|
		 KeyPressMask|KeyReleaseMask);
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
    title[sn]=title[0];
    dfont[sn]=dfont[0];
    charw[sn]=charw[0];
    charh[sn]=charh[0];
    useoptfile[sn]=useoptfile[0];
    mbutton[sn]=mbutton[0];
    keyboard[sn]=keyboard[0];
    abkuerz[sn]=abkuerz[0];
    nimmstich[sn][0]=nimmstich[0][0];
    sort1[sn]=sort1[0];
    alternate[sn]=alternate[0];
    alist[sn]=alist[0];
    hints[sn]=hints[0];
    blatt[sn]=blatt[0];
    lang[sn]=lang[0];
    briefmsg[sn]=briefmsg[0];
    trickl2r[sn]=trickl2r[0];
    geom_f[sn]=geom_f[0];
    geom_x[sn]=geom_x[0];
    geom_y[sn]=geom_y[0];
    szhints[sn]=szhints[0];
  }
}

int closecol(x,r)
int x,*r;
{
  int i;

  for (i=1;x>r[i];i++);
  return i-(r[i]-x>x-r[i-1]);
}

VOID find_cardcol(bm,r,col)
unsigned char *bm;
int *r,col[6][6][6];
{
  int i,s;

  bm+=10;
  s=1<<((*bm++&7)+1);
  bm+=2;
  for (i=0;i<s;i++) {
    col[closecol(bm[0],r)][closecol(bm[1],r)][closecol(bm[2],r)]=1;
    bm+=3;
  }
}

VOID card_colors(sn)
int sn;
{
  int c,i,j,k,ncol;
  int col[6][6][6];
  unsigned long p;
  XColor xc;

  if (desk[sn].col<=2) {
    return;
  }
  else if (desk[sn].plan>=8) {
    c=0;
  }
  else if (desk[sn].plan>=7) {
    c=1;
  }
  else if (desk[sn].plan>=6) {
    c=2;
  }
  else {
    c=3;
  }
  for (;c<4;c++) {
    for (i=0;i<6-c;i++) {
      for (j=0;j<6-c;j++) {
	for (k=0;k<6-c;k++) {
	  col[i][j][k]=0;
	}
      }
    }
    for (i=0;i<4;i++) {
      for (j=0;j<3;j++) {
	find_cardcol(fr_gif[i][j],ramp[c],col);
      }
      for (j=0;j<8;j++) {
	find_cardcol(de_gif[i][j],ramp[c],col);
      }
    }
    find_cardcol(backsd_gif[0],ramp[c],col);
    ncol=0;
    for (i=0;i<6-c;i++) {
      for (j=0;j<6-c;j++) {
	for (k=0;k<6-c;k++) {
	  if (col[i][j][k]) {
	    xc.red=ramp[c][i]<<8;
	    xc.green=ramp[c][j]<<8;
	    xc.blue=ramp[c][k]<<8;
	    if (closest_col(sn,&xc)) {
	      color[sn][desk[sn].col+ncol++]=xc;
	    }
	    else {
	      i=j=k=7;
	    }
	  }
	}
      }
    }
    if (i<7) {
      desk[sn].col+=ncol;
      return;
    }
    for (i=0;i<ncol;i++) {
      p=color[sn][desk[sn].col+i].pixel;
      for (j=0;j<desk[sn].col && color[sn][j].pixel!=p;j++);
      if (j==desk[sn].col) {
	for (j=i+1;j<ncol && color[sn][desk[sn].col+j].pixel!=p;j++);
	if (j==ncol) XFreeColors(dpy[sn],cmap[sn],&p,1,0);
      }
    }
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
  int i,len,cw,nw,ln;
  unsigned int w,h;
  struct passwd *pwd;

  if (sn) {
    font_name=fg_col=bg_col=bt_col=0;
    w3d_col=b3d_col=mk_col=0;
    bwcol=gfx3d[sn]=desk[sn].large=-1;
    extractnam(sn,disp_name[sn]);
    if (strchr(disp_name[sn],'@')) {
      disp_name[sn]=strchr(disp_name[sn],'@')+1;
    }
  }
  else {
    extractnam(sn,getenv("LOGNAME"));
    strcpy(usrname[0],spnames[0][0][0]);
    strcpy(usrname[1],spnames[0][1][0]);
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
  for (i=0;i<4;i++) {
    if (sn || !ccol[i] ||
	!XParseColor(dpy[sn],cmap[sn],ccol[i],&color[sn][i])) {
      sprintf(cbuf,"color%d",i+1);
      res=XGetDefault(dpy[sn],prog_name,cbuf);
      if (res) {
	XParseColor(dpy[sn],cmap[sn],res,&color[sn][i]);
      }
    }
  }
  if (bwcol<0) {
    res=XGetDefault(dpy[sn],prog_name,"color");
    bwcol=!res || istrue(res);
  }
  if (bwcol && desk[sn].plan>1) {
    for (i=0;i<6 && closest_col(sn,&color[sn][i]);i++);
    if (i!=6) {
      if (i==4) color[sn][i++]=color[sn][0];
      if (i==5) color[sn][i++]=color[sn][2];
      else i=0;
    }
  }
  else i=0;
  color[sn][i].pixel=wpix[sn];
  color[sn][i].red=color[sn][i].green=color[sn][i].blue=0xffff;
  i++;
  color[sn][i].pixel=bpix[sn];
  color[sn][i].red=color[sn][i].green=color[sn][i].blue=0;
  i++;
  desk[sn].col=i;
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
  card_colors(sn);
  if (desk[sn].large<0) {
    res=XGetDefault(dpy[sn],prog_name,"large");
    if (res) desk[sn].large=istrue(res);
    if (desk[sn].large<0) {
      desk[sn].large=XDisplayWidth(dpy[sn],scr)>960 &&
	XDisplayHeight(dpy[sn],scr)>600;
    }
  }
  calc_desk(sn);
  if (!title[sn] &&
      !(title[sn]=XGetDefault(dpy[sn],prog_name,"title"))) title[sn]=prog_name;
  if (!geom_f[sn] &&
      (res=XGetDefault(dpy[sn],prog_name,"geometry"))) {
    geom_f[sn]=XParseGeometry(res,&geom_x[sn],&geom_y[sn],&w,&h);
  }
  if (!font_name &&
      !(font_name=XGetDefault(dpy[sn],prog_name,"font"))) {
    font_name=desk[sn].large?"10x20":"9x15";
  }
  if (!(dfont[sn]=XLoadQueryFont(dpy[sn],font_name))) {
    fprintf(stderr,"Font %s not found\n",font_name);
    exitus(1);
  }
  charw[sn]=dfont[sn]->max_bounds.width;
  charh[sn]=dfont[sn]->max_bounds.ascent+dfont[sn]->max_bounds.descent+1;
  if (desk[sn].large && charh[sn]<21) charh[sn]=21;
  else if (charh[sn]<16) charh[sn]=16;
  charh[sn]+=gfx3d[sn]+(charh[sn]&1);
  if (sn) {
    res=XGetDefault(dpy[sn],prog_name,"useoptfile");
    useoptfile[sn]=res && istrue(res);
  }
  else {
    useoptfile[sn]=1;
  }
  if (!mbuttonset[sn] &&
      (res=XGetDefault(dpy[sn],prog_name,"menubutton"))) {
    mbutton[sn]=atoi(res);
  }
  if (mbutton[sn]<0 || mbutton[sn]>5) mbutton[sn]=0;
  if (!keyboardset[sn] &&
      (res=XGetDefault(dpy[sn],prog_name,"keyboard"))) {
    keyboard[sn]=atoi(res);
  }
  if (keyboard[sn]<0 || keyboard[sn]>2) keyboard[sn]=1;
  if (!abkuerzset[sn] &&
      (res=XGetDefault(dpy[sn],prog_name,"shortcut"))) {
    abkuerz[sn]=atoi(res);
    if (abkuerz[sn]<0 || abkuerz[sn]>2) abkuerz[sn]=1;
  }
  if (!tdelayset[sn] &&
      (res=XGetDefault(dpy[sn],prog_name,"tdelay"))) {
    nimmstich[sn][0]=(int)(atof(res)*10+.5);
  }
  if (nimmstich[sn][0]<0) nimmstich[sn][0]=7;
  else if (nimmstich[sn][0]>101) nimmstich[sn][0]=101;
  if (!downupset[sn]) {
    res=XGetDefault(dpy[sn],prog_name,"down");
    downup=!res || istrue(res);
  }
  sort1[sn]=!downup;
  if (!altseqset[sn]) {
    res=XGetDefault(dpy[sn],prog_name,"alt");
    if (!res && sn) altseq=alternate[0];
    else altseq=res && istrue(res);
  }
  alternate[sn]=!!altseq;
  if (!alistset[sn]) {
    res=XGetDefault(dpy[sn],prog_name,"alist");
    if (res) {
      alist[sn]=istrue(res);
    }
    else {
      res=XGetDefault(dpy[sn],prog_name,"tlist");
      if (!res && sn) alist[sn]=alist[0];
      else alist[sn]=res && istrue(res)?2:0;
    }
  }
  if ((res=XGetDefault(dpy[sn],prog_name,"alias"))) {
    extractnam(sn,res);
    if (!sn) {
      strcpy(usrname[0],spnames[0][0][0]);
      strcpy(usrname[1],spnames[0][1][0]);
    }
  }
  if (!hintsset[sn]) {
    res=XGetDefault(dpy[sn],prog_name,"hint");
    if (!res && sn) hints[sn]=hints[0];
    else hints[sn]=res && istrue(res);
  }
  if (!blattset[sn]) {
    res=XGetDefault(dpy[sn],prog_name,"cards");
    if (res) {
      blatt[sn]=atoi(res);
      if (blatt[sn]<0 || blatt[sn]>3) blatt[sn]=1;
    }
    else if (sn) {
      blatt[sn]=blatt[0];
    }
  }
  if (!langset[sn]) {
    res=XGetDefault(dpy[sn],prog_name,"language");
    if (!res && sn) lang[sn]=lang[0];
    else lang[sn]=langidx(res);
  }
  if (!briefmsgset[sn]) {
    res=XGetDefault(dpy[sn],prog_name,"briefmsg");
    if (!res && sn) briefmsg[sn]=briefmsg[0];
    else briefmsg[sn]=res && istrue(res);
  }
  if (!trickl2rset[sn]) {
    res=XGetDefault(dpy[sn],prog_name,"trickl2r");
    if (!res && sn) trickl2r[sn]=trickl2r[0];
    else trickl2r[sn]=res && istrue(res);
  }
  if (!sn) {
    if (!game_file) {
      if (!getdeffn(prog_name,&game_file,"game","")) {
	game_file=0;
      }
    }
    logdef=!prot_file && getdeffn(prog_name,&prot_file,"log","xskat.log");
    if (logging<0) {
      res=XGetDefault(dpy[sn],prog_name,"dolog");
      logging=(res && istrue(res)) || (!res && logdef);
    }
    if (unformatted<0) {
      res=XGetDefault(dpy[sn],prog_name,"formatted");
      unformatted=!(res && istrue(res));
    }
    if (!opt_file) {
      getdeffn(prog_name,&opt_file,"opt","xskat.opt");
    }
    if (!ramschset && (res=XGetDefault(dpy[sn],prog_name,"ramsch"))) {
      playramsch=atoi(res);
      if (playramsch<0) playramsch=0;
      else if (playramsch>2) playramsch=2;
    }
    if (!sramschset && (res=XGetDefault(dpy[sn],prog_name,"sramsch"))) {
      playsramsch=istrue(res);
    }
    if (!rskatloserset && (res=XGetDefault(dpy[sn],prog_name,"skattoloser"))) {
      rskatloser=istrue(res);
    }
    if (!kontraset && (res=XGetDefault(dpy[sn],prog_name,"kontra"))) {
      playkontra=atoi(res);
      if (playkontra<0) playkontra=0;
      else if (playkontra>2) playkontra=2;
    }
    if (!bockset && (res=XGetDefault(dpy[sn],prog_name,"bock"))) {
      playbock=atoi(res);
      if (playbock<0) playbock=0;
      else if (playbock>2) playbock=2;
    }
    if (!bockeventsset &&
	(res=XGetDefault(dpy[sn],prog_name,"bockevents"))) {
      bockevents=atoi(res);
    }
    if (!resumebockset &&
	(res=XGetDefault(dpy[sn],prog_name,"resumebock"))) {
      resumebock=istrue(res);
    }
    if (!spitzezaehltset &&
	(res=XGetDefault(dpy[sn],prog_name,"spitze"))) {
      spitzezaehlt=atoi(res);
      if (spitzezaehlt<0) spitzezaehlt=0;
      else if (spitzezaehlt>2) spitzezaehlt=2;
    }
    if (!revolutionset &&
	(res=XGetDefault(dpy[sn],prog_name,"revolution"))) {
      revolution=istrue(res);
    }
    if (!klopfenset &&
	(res=XGetDefault(dpy[sn],prog_name,"klopfen"))) {
      klopfen=istrue(res);
    }
    if (!schenkenset &&
	(res=XGetDefault(dpy[sn],prog_name,"schenken"))) {
      schenken=istrue(res);
    }
    if (!oldrulesset &&
	(res=XGetDefault(dpy[sn],prog_name,"oldrules"))) {
      oldrules=istrue(res);
    }
    if (irc_play<0) {
      res=XGetDefault(dpy[sn],prog_name,"irc");
      irc_play=res && istrue(res);
    }
    if (irc_logappend<0) {
      res=XGetDefault(dpy[sn],prog_name,"irclogappend");
      irc_logappend=res && istrue(res);
    }
    if (!list_file) {
      getdeffn(prog_name,&list_file,
	       irc_play?"irclist":"list",
	       irc_play?"xskat.irc":"xskat.lst");
    }
    if (!cards_file) {
      getdeffn(prog_name,&cards_file,"cardsfile","xskat.cards");
    }
    if (!irc_host &&
	!(irc_host=XGetDefault(dpy[sn],prog_name,"ircserver")) &&
	!(irc_host=getenv("IRCSERVER"))) {
      irc_host=irc_defaulthost;
    }
    strncpy(irc_hostname,irc_host,35);
    irc_hostname[35]=0;
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
    }
    if (geber<0) {
      if ((res=XGetDefault(dpy[sn],prog_name,"start"))) {
	geber=atoi(res);
      }
      if (geber<1 || geber>3) geber=0;
      else geber=left(geber);
    }
    for (i=0;i<2;i++) {
      if (!stgset[i]) {
	stgs[1]=i+'1';
	if ((res=XGetDefault(dpy[sn],prog_name,stgs))) {
	  strateg[i]=atoi(res);
	}
      }
      if (strateg[i]<-4) strateg[i]=-4;
      else if (strateg[i]>0) strateg[i]=0;
    }
    if (prot_file && !*prot_file) prot_file=0;
    if (opt_file && !*opt_file) opt_file=0;
    if (game_file && !*game_file) game_file=0;
    if (list_file && !*list_file) list_file=0;
    if (irc_logfile && !*irc_logfile) irc_logfile=0;
    if (cards_file && !*cards_file) cards_file=0;
  }
  if ((geom_f[sn]&(XValue|YValue))==(XValue|YValue)) {
    szhints[sn].x=geom_f[sn]&XNegative?
      XDisplayWidth(dpy[sn],scr)+geom_x[sn]-desk[sn].w:geom_x[sn];
    szhints[sn].y=geom_f[sn]&YNegative?
      XDisplayHeight(dpy[sn],scr)+geom_y[sn]-desk[sn].h:geom_y[sn];
    szhints[sn].flags|=USPosition;
  }
  szhints[sn].flags|=PMinSize|PMaxSize;
  szhints[sn].min_width=szhints[sn].max_width=desk[sn].w;
  szhints[sn].min_height=szhints[sn].max_height=desk[sn].h;
  cw=1;
  for (ln=0;ln<NUM_LANG;ln++) {
    for (i=0;i<TX_NUM_TX;i++) {
      if ((len=strlen(textarr[i].t[ln]))>3 &&
	  (nw=(XTextWidth(dfont[sn],textarr[i].t[ln],len)+len-1)/len)>cw) {
	cw=nw;
      }
    }
  }
  if (cw<charw[sn]) charw[sn]=cw;
  if (charw[sn]<7) charw[sn]=7;
  for (ln=0;ln<NUM_LANG;ln++) {
    if (!spnames[sn][0][ln][0]) {
      sprintf(spnames[sn][0][ln],textarr[TX_SPIELER_N].t[ln],sn+1);
    }
  }
}

VOID xstoreres()
{
  char buf[256];
  XColor xcol;

  if (!XGetDefault(dpy[0],prog_name,"font") && font_name) {
    sprintf(buf,"%.99s.%s:%.99s\n",prog_name,"font",font_name);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"title") && title[0]) {
    sprintf(buf,"%.99s.%s:%.99s\n",prog_name,"title",title[0]);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"foreground")) {
    xcol.pixel=fgpix[0];
    XQueryColor(dpy[0],cmap[0],&xcol);
    sprintf(buf,"%.99s.%s:#%04x%04x%04x\n",prog_name,"foreground",
	    xcol.red,xcol.green,xcol.blue);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"background")) {
    xcol.pixel=bgpix[0];
    XQueryColor(dpy[0],cmap[0],&xcol);
    sprintf(buf,"%.99s.%s:#%04x%04x%04x\n",prog_name,"background",
	    xcol.red,xcol.green,xcol.blue);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"mark")) {
    xcol.pixel=mkpix[0];
    XQueryColor(dpy[0],cmap[0],&xcol);
    sprintf(buf,"%.99s.%s:#%04x%04x%04x\n",prog_name,"mark",
	    xcol.red,xcol.green,xcol.blue);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"menubutton")) {
    sprintf(buf,"%.99s.%s:%d\n",prog_name,"menubutton",mbutton[0]);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"keyboard")) {
    sprintf(buf,"%.99s.%s:%d\n",prog_name,"keyboard",keyboard[0]);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"tdelay")) {
    sprintf(buf,"%.99s.%s:%f\n",prog_name,"tdelay",nimmstich[0][0]/10.0);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"cards")) {
    sprintf(buf,"%.99s.%s:%d\n",prog_name,"cards",blatt[0]);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"large")) {
    sprintf(buf,"%.99s.%s:%d\n",prog_name,"large",desk[0].large);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"down")) {
    sprintf(buf,"%.99s.%s:%d\n",prog_name,"down",!sort1[0]);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"alt")) {
    sprintf(buf,"%.99s.%s:%d\n",prog_name,"alt",alternate[0]);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"briefmsg")) {
    sprintf(buf,"%.99s.%s:%d\n",prog_name,"briefmsg",briefmsg[0]);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"trickl2r")) {
    sprintf(buf,"%.99s.%s:%d\n",prog_name,"trickl2r",trickl2r[0]);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"language")) {
    sprintf(buf,"%.99s.%s:%.99s\n",prog_name,"language",idxlang(lang[0]));
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"hint")) {
    sprintf(buf,"%.99s.%s:%d\n",prog_name,"hint",hints[0]);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"shortcut")) {
    sprintf(buf,"%.99s.%s:%d\n",prog_name,"shortcut",abkuerz[0]);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"alias")) {
    sprintf(buf,"%.99s.%s:%.9s %.9s\n",prog_name,"alias",
	    spnames[0][0][0],spnames[0][1][0]);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  if (!XGetDefault(dpy[0],prog_name,"ready")) {
    sprintf(buf,"%.99s.%s:%d\n",prog_name,"ready",1);
    XChangeProperty(dpy[0],DefaultRootWindow(dpy[0]),
		    XA_RESOURCE_MANAGER,XA_STRING,8,PropModeAppend,
		    buf,strlen(buf));
  }
  XCloseDisplay(dpy[0]);
  strcpy(buf,"+");
  strcat(buf,lanip[0]);
  if (strchr(buf,':')) *strchr(buf,':')=0;
  execlp("xhost","xhost",buf,NULL);
  fprintf(stderr,"xhost not found\n");
}

VOID read_cards()
{
  FILE *f;
  int b,cd,cdst,bufidx,buflen=200000;
  char *buf,line[20];

  if (!cards_file) return;
  f=fopen(cards_file,"r");
  if (!f) return;
  buf=malloc(buflen);
  if (!buf) nomem();
  bufidx=0;
  for (cd=0;cd<sizeof(map_gif)/sizeof(map_gif[0]);cd++) {
    while (fscanf(f,"unsigned char %4s",line)!=1) {
      if (feof(f)) {
	fclose(f);
	fprintf(stderr,"Error reading cards from %s\n",cards_file);
	return;
      }
      fscanf(f,"%*s ");
    }
    if (strncmp(map_gif[cd].name,line,strlen(map_gif[cd].name))) {
      cd--;
      continue;
    }
    cdst=bufidx;
    do {
      if (bufidx<buflen && fscanf(f,"%i",&b)==1) {
	buf[bufidx++]=b;
      }
    } while (fgetc(f)!='}' && !feof(f));
    if (bufidx>=buflen || feof(f) ||
	strcmp(buf+cdst,"GIF87aX") ||
	(buf[cdst+8]!=(char)0x45 && buf[cdst+8]!=(char)0x8a)) {
      cd--;
      bufidx=cdst;
      continue;
    }
    *(map_gif[cd].pos)=buf+cdst;
    ggcards=0;
  }
  fclose(f);
}

VOID set_conames()
{
  int ln,sn;
  char buf[40];

  if (!usrname[0][0]) strcpy(usrname[0],"~");
  if (!usrname[1][0]) strcpy(usrname[1],"~");
  for (ln=0;ln<NUM_LANG;ln++) {
    if (!strcmp(conames[0][0],textarr[TX_COMPUTER].t[ln]) &&
	!strcmp(conames[0][1],textarr[TX_LINKS].t[ln])) {
      strcpy(conames[0][0],"~");
      strcpy(conames[0][1],"~");
    }
    if (!strcmp(conames[1][0],textarr[TX_COMPUTER].t[ln]) &&
	!strcmp(conames[1][1],textarr[TX_RECHTS].t[ln])) {
      strcpy(conames[1][0],"~");
      strcpy(conames[1][1],"~");
    }
  }
  for (sn=0;sn<2;sn++) {
    if (!conames[sn][0][0]) strcpy(conames[sn][0],"~");
    if (!conames[sn][1][0]) strcpy(conames[sn][1],"~");
  }
  for (ln=0;ln<NUM_LANG;ln++) {
    switch (numsp) {
    case 0:
      for (sn=0;sn<3;sn++) {
	if (!spnames[sn][0][ln][0]) {
	  sprintf(spnames[sn][0][ln],"%s%d",textarr[TX_COMPUTER].t[ln],sn+1);
	}
      }
      break;
    case 1:
      sprintf(buf,"%s %s",usrname[0],usrname[1]);
      extractnamln(0,buf,ln);
      sprintf(buf,"%s %s",conames[0][0],conames[0][1]);
      extractnamln(1,buf,ln);
      if (!spnames[1][0][ln][0]) {
	strcpy(spnames[1][0][ln],textarr[TX_COMPUTER].t[ln]);
	strcpy(spnames[1][1][ln],textarr[TX_LINKS].t[ln]);
      }
      sprintf(buf,"%s %s",conames[1][0],conames[1][1]);
      extractnamln(2,buf,ln);
      if (!spnames[2][0][ln][0]) {
	strcpy(spnames[2][0][ln],textarr[TX_COMPUTER].t[ln]);
	strcpy(spnames[2][1][ln],textarr[TX_RECHTS].t[ln]);
      }
      break;
    case 2:
      if (!irc_play) {
	sprintf(buf,"%s %s",usrname[0],usrname[1]);
	extractnamln(0,buf,ln);
      }
      sprintf(buf,"%s %s",conames[1][0],conames[1][1]);
      extractnamln(2,buf,ln);
      if (!spnames[2][0][ln][0]) {
	strcpy(spnames[2][0][ln],textarr[TX_COMPUTER].t[ln]);
      }
      break;
    case 3:
      if (!irc_play) {
	sprintf(buf,"%s %s",usrname[0],usrname[1]);
	extractnamln(0,buf,ln);
      }
      break;
    }
  }
}

VOID xinit(argc,argv)
int argc;
char *argv[];
{
  int ln,sn,i;
  unsigned int w,h;

  logit();
  signal(SIGPIPE,SIG_IGN);
  signal(SIGHUP,exitus);
  signal(SIGINT,exitus);
  signal(SIGTERM,exitus);
  numsp=1;
  disp_name[0]=font_name=fg_col=bg_col=bt_col=0;
  w3d_col=b3d_col=mk_col=0;
  for (i=0;i<4;i++) ccol[i]=0;
  geber=logging=unformatted=bwcol=gfx3d[0]=desk[0].large=-1;
  irc_play=irc_pos=irc_port=irc_logappend=-1;
  irc_telnet=irc_host=irc_channel=irc_nick=0;
  irc_user=irc_realname=irc_logfile=0;
  nimmstich[0][0]=nimmstich[1][0]=nimmstich[2][0]=7;
  keyboard[0]=keyboard[1]=keyboard[2]=1;
  abkuerz[0]=abkuerz[1]=abkuerz[2]=1;
  trickl2r[0]=trickl2r[1]=trickl2r[2]=1;
  blatt[0]=blatt[1]=blatt[2]=1;
  prog_name=strrchr(argv[0],'/');
  if (prog_name) prog_name++;
  else prog_name=argv[0];
#ifdef __EMX__ /* XFree OS/2 */
  _remext(prog_name);
#endif
  while (argc>1) {
    if (!strcmp(argv[1],"-help") || !strcmp(argv[1],"-h")) {
      usage();
      exitus(0);
    }
    else if (!strcmp(argv[1],"-nopre")) {
      nopre=1;
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
    else if (!strcmp(argv[1],"-frenchcards")) {
      blatt[0]=0;
      blattset[0]=1;
    }
    else if (!strcmp(argv[1],"-french4cards")) {
      blatt[0]=1;
      blattset[0]=1;
    }
    else if (!strcmp(argv[1],"-germancards")) {
      blatt[0]=2;
      blattset[0]=1;
    }
    else if (!strcmp(argv[1],"-german4cards")) {
      blatt[0]=3;
      blattset[0]=1;
    }
    else if (!strcmp(argv[1],"-noshortcut")) {
      abkuerz[0]=0;
      abkuerzset[0]=1;
    }
    else if (!strcmp(argv[1],"-askshortcut")) {
      abkuerz[0]=1;
      abkuerzset[0]=1;
    }
    else if (!strcmp(argv[1],"-shortcut")) {
      abkuerz[0]=2;
      abkuerzset[0]=1;
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
    else if (!strcmp(argv[1],"-skattolast")) {
      rskatloser=0;
      rskatloserset=1;
    }
    else if (!strcmp(argv[1],"-skattoloser")) {
      rskatloser=1;
      rskatloserset=1;
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
    else if (!strcmp(argv[1],"-newrules")) {
      oldrules=0;
      oldrulesset=1;
    }
    else if (!strcmp(argv[1],"-oldrules")) {
      oldrules=1;
      oldrulesset=1;
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
    else if (!strcmp(argv[1],"-briefmsg")) {
      briefmsg[0]=1;
      briefmsgset[0]=1;
    }
    else if (!strcmp(argv[1],"-verbosemsg")) {
      briefmsg[0]=0;
      briefmsgset[0]=1;
    }
    else if (!strcmp(argv[1],"-trickl2r")) {
      trickl2r[0]=1;
      trickl2rset[0]=1;
    }
    else if (!strcmp(argv[1],"-notrickl2r")) {
      trickl2r[0]=0;
      trickl2rset[0]=1;
    }
    else if (!strcmp(argv[1],"-irc")) {
      irc_play=1;
    }
    else if (!strcmp(argv[1],"-noirc")) {
      irc_play=0;
    }
    else if (!strcmp(argv[1],"-irclogappend")) {
      irc_logappend=1;
      irc_play=1;
    }
    else if (!strcmp(argv[1],"-irclogoverwrite")) {
      irc_logappend=0;
      irc_play=1;
    }
    else if (!strcmp(argv[1],"-pk")) {
      pkoption=1;
    }
    else if ((argv[1][0]!='-') && numsp<3) {
      disp_name[numsp++]=argv[1];
    }
    else if (argc>2) {
      if (!strcmp(argv[1],"-display") || !strcmp(argv[1],"-d")) {
	disp_name[0]=argv[2];
      }
      else if (!strcmp(argv[1],"-geometry") || !strcmp(argv[1],"-g")) {
	geom_f[0]=XParseGeometry(argv[2],&geom_x[0],&geom_y[0],&w,&h);
      }
      else if (!strcmp(argv[1],"-name") || !strcmp(argv[1],"-n")) {
	prog_name=argv[2];
      }
      else if (!strcmp(argv[1],"-title") || !strcmp(argv[1],"-T")) {
	title[0]=argv[2];
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
	if (i<=4) ccol[i-1]=argv[2];
      }
      else if (!strcmp(argv[1],"-mb")) {
	mbutton[0]=atoi(argv[2]);
	mbuttonset[0]=1;
      }
      else if (!strcmp(argv[1],"-keyboard")) {
	keyboard[0]=atoi(argv[2]);
	keyboardset[0]=1;
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
	lang[0]=langidx(argv[2]);
	langset[0]=1;
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
	irc_hostset=1;
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
  init_text();
  if (numgames) {
    numsp=irc_play=0;
    if (!langset[0]) lang[0]=langidx((char *)0);
    lang[2]=lang[1]=lang[0];
    if (geber<0) geber=0;
    if (logging<0) logging=0;
    if (unformatted<0) unformatted=1;
    for (i=0;i<3;i++) {
      if (strateg[i]<-4) strateg[i]=-4;
      else if (strateg[i]>0) strateg[i]=0;
    }
  }
  else {
    xinitres(0);
  }
  read_list();
  if (irc_play) {
    setsum(0);
    read_opt();
    set_conames();
    irc_init();
  }
  else {
    read_opt();
    for (sn=1;sn<numsp;sn++) {
      xinitres(sn);
    }
    read_opt();
  }
  for (sn=0;sn<3;sn++) {
    if (!lanip[sn][0]) {
      strcpy(lanip[sn],"127.0.0.1");
    }
  }
  if (de_gif[3][0][8]==0x45) {
    ggcards=1;
  }
  read_cards();
  for (sn=0;sn<numsp;sn++) {
    selpos[sn].act=-1;
    xinitwin(sn,sn?0:theargc,sn?(char **)0:theargv);
  }
  init_dials();
  for (ln=0;ln<NUM_LANG;ln++) {
    for (sn=0;sn<3;sn++) {
      tspnames[sn][0].t[ln]=spnames[sn][0][ln];
      tspnames[sn][1].t[ln]=spnames[sn][1][ln];
    }
  }
  set_conames();
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

VOID drawcard(sn,c,x,y)
int sn,c,x,y;
{
  XCopyArea(dpy[sn],cardpx[sn][c+1],win[sn],gc[sn],0,0,
	    desk[sn].cardw,desk[sn].cardh,x,y);
  XCopyArea(dpy[sn],cardpx[sn][c+1],bck[sn],gc[sn],0,0,
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

  xyarr[0]=(c<30?desk[sn].playx:desk[sn].skatx)+
    (c%10)*desk[sn].cardx+7*desk[sn].f/desk[sn].q;
  xyarr[1]=(c<30?desk[sn].playy:desk[sn].skaty)
    -5*desk[sn].f/desk[sn].q;
  xyarr[2]=xyarr[0]+desk[sn].cardx-16*desk[sn].f/desk[sn].q+1;
  xyarr[3]=xyarr[1];
  XDrawLine(dpy[sn],win[sn],gcp[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
  XDrawLine(dpy[sn],bck[sn],gcp[sn],xyarr[0],xyarr[1],xyarr[2],xyarr[3]);
}

VOID show_hint(sn,c,d)
int sn,c,d;
{
  static int lm[3][2];

  if (lm[sn][c]) {
    hint_line(sn,lm[sn][c]-1,gcbck);
  }
  if (d) {
    change_gc(sn,mkpix[sn],gc);
    hint_line(sn,hintcard[c],gc);
    change_gc(sn,fgpix[sn],gc);
    lm[sn][c]=hintcard[c]+1;
  }
  else {
    lm[sn][c]=0;
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
  if (!umdrueck) movecard(numsp,sna,x1,y1,x2,y2);
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
  int i,x,y,c0,c1;

  if (phase==WEITER || phase==REVOLUTION) return;
  if (sor) {
    c0=-1;
    c1=-1;
    if (hintcard[0]!=-1) {
      if (phase==SPIELEN) {
	c0=cards[hintcard[0]];
      }
      else if (phase==DRUECKEN) {
	if (hintcard[0]<30) c0=cards[hintcard[0]];
	if (hintcard[1]<30) c1=cards[hintcard[1]];
      }
    }
    if (sor!=2) sort(sn);
    else {
      if (skatopen) draw_skat(spieler);
      if (phase==SPIELEN || phase==NIMMSTICH) {
	for (i=0;i<stichopen;i++) {
	  putcard(sn,stcd[i],desk[sn].stichx+i*desk[sn].cardw,desk[sn].stichy);
	}
      }
    }
    for (i=0;i<10;i++) {
      if (c0>=0 && c0==cards[sn*10+i]) hintcard[0]=sn*10+i;
      if (c1>=0 && c1==cards[sn*10+i]) hintcard[1]=sn*10+i;
      if (hintcard[0]!=-1 && !iscomp(sn) && hints[sn]) {
	if (phase==SPIELEN && sn==(ausspl+vmh)%3) {
	  show_hint(sn,0,1);
	}
	else if (phase==DRUECKEN && sn==spieler) {
	  show_hint(sn,0,1);
	  show_hint(sn,1,1);
	}
      }
      putcard(sn,cards[sn*10+i],
	      desk[sn].playx+i*desk[sn].cardx,desk[sn].playy);
    }
  }
  if (phase!=ANSAGEN) {
    di_info(sn,-1);
    if (predef && (!ouveang || sn==spieler) && (sn || !nopre)) {
      x=desk[sn].w/2;
      y=desk[sn].y+2*charh[sn];
      v_gtext(sn,x,y,0,textarr[TX_VORDEFINIERTES_SPIEL].t[lang[sn]]);
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
      if (backopen[spieler==left(sn)?left(spieler):left(sn)]) putback(sn,x,y);
    }
  }
  else if (spitzeang && sn!=spieler && spitzeopen) {
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
    if (sn!=spieler || abkuerz[sn]==2)  {
      x=desk[sn].skatx+desk[sn].cardx;
      y1=(desk[sn].skaty+desk[sn].com1y+desk[sn].cardh-charh[sn])/2;
      v_gtext(sn,x,y1,0,textarr[trumpf==-1?TX_NULL_DICHT:TX_REST_BEI_MIR].
	      t[lang[sn]]);
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
    v_gtext(sn,x,y,0,textarr[TX_KARTEN_AUSTAUSCHEN].t[lang[sn]]);
    mi=left(sn)==spieler?left(spieler):left(sn);
    x=desk[sn].playx+10*desk[sn].f/desk[sn].q;
    v_gtextnc(sn,0,0,x,desk[sn].com1y+desk[sn].cardh+1,
	      0,textarr[spieler==ausspl?TX_VORHAND:
			spieler==left(ausspl)?TX_MITTELHAND:
			TX_HINTERHAND].t[lang[sn]]);
    v_gtextnc(sn,0,0,x,desk[sn].skaty-charh[sn]-1,
	      0,textarr[mi==ausspl?TX_VORHAND:
			mi==left(ausspl)?TX_MITTELHAND:
			TX_HINTERHAND].t[lang[sn]]);
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
  xy[2]=x+w-3;xy[3]=y+charh[sn]-gfx3d[sn]+1;
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
    h=charh[sn]-gfx3d[sn]-1;
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
	   textarr[t].t[lang[sn]]);
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
  h=charh[sn]-gfx3d[sn]-1;
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

  if (x<=desk[sn].pboxx+60*desk[sn].f/desk[sn].q) b=0;
  else b=desk[sn].cardw;
  if (x>=desk[sn].pboxx+3*desk[sn].f/desk[sn].q+b &&
      x<=desk[sn].pboxx+60*desk[sn].f/desk[sn].q+b &&
      y>=desk[sn].pboxy+1 &&
      y<=desk[sn].pboxy+16*desk[sn].f/desk[sn].q) {
    di_delres(sn);
    if (b) maxrw[sn]=0;
    else {
      maxrw[sn]=999-1;
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
    if (hintcard[0]==10*sn+c) {
      hintcard[0]=drkcd+30;
      if (hints[sn]) show_hint(sn,0,0);
    }
    else if (hintcard[0]==drkcd+30) hintcard[0]=10*sn+c;
    if (hintcard[1]==10*sn+c)  {
      hintcard[1]=drkcd+30;
      if (hints[sn]) show_hint(sn,1,0);
    }
    else if (hintcard[1]==drkcd+30) hintcard[1]=10*sn+c;
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
  if (x>=desk[sn].pboxx+27*desk[sn].f/desk[sn].q &&
      x<=desk[sn].pboxx+100*desk[sn].f/desk[sn].q &&
      y>=desk[sn].pboxy+1 &&
      y<=desk[sn].pboxy+16*desk[sn].f/desk[sn].q) inv_fbox(spieler,1);
  else return 0;
  if (hints[sn] && hintcard[0]!=-1) {
    show_hint(sn,0,0);
    show_hint(sn,1,0);
  }
  stdwait();
  inv_fbox(spieler,0);
  if (trumpf==5 && (((cards[30]&7)==BUBE) || ((cards[31]&7)==BUBE))) {
    di_buben();
    return 1;
  }
  rem_fbox(spieler);
  drbut=0;
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
  if (x>=desk[sn].pboxx+27*desk[sn].f/desk[sn].q &&
      x<=desk[sn].pboxx+100*desk[sn].f/desk[sn].q &&
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
	di_eingabe(sn);
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

VOID setcurs(f)
int f;
{
  int x,y,w,sn,snn,newsn=-1;
  char clr[100];
  static int actsn=-1,wsn=-1,wtime;
  static int ur[3];

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
      if (stich==1 && trumpf==5 && !ur[sn]) {
	di_info(sn,-1);
	ur[sn]=1;
      }
      if (stich==2) ur[sn]=0;
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
      if (numsp==1) newsn=0;
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
  if (f && actsn>=0) {
    if (phase!=WEITER && phase!=REVOLUTION) di_info(f-1,actsn);
  }
  if (actsn==-1) wtime=0;
  if (phase==REIZEN && bockspiele && !ramschspiele) wtime=15*1000;
  if (!wtime || wtime>=15*1000) {
    if (wsn!=-1 && (actsn!=wsn ||
		    wtime==16*1000 ||
		    (phase!=REIZEN && phase!=SPIELEN && phase!=NIMMSTICH))) {
      x=desk[wsn].w/2;
      y=(desk[wsn].pboxy+desk[wsn].playy)/2;
      w=5*desk[wsn].cardw/XTextWidth(dfont[wsn]," ",1)+1;
      if (w>99) w=99;
      clr[w]=0;
      while (w) clr[--w]=' ';
      v_gtext(wsn,x,y,0,clr);
      wsn=-1;
      if (wtime==16*1000) wtime-=1500;
      if (phase!=REIZEN && phase!=SPIELEN && phase!=NIMMSTICH) wtime=0;
    }
    if (wtime==15*1000 &&
	(phase==REIZEN || phase==SPIELEN || phase==NIMMSTICH)) {
      wsn=actsn;
      x=desk[wsn].w/2;
      y=(desk[wsn].pboxy+desk[wsn].playy)/2;
      if (phase==REIZEN) {
	if (bockspiele && !ramschspiele) {
	  v_gtext(wsn,x,y,0,textarr[TX_BOCK_SPIEL].t[lang[wsn]]);
	}
      }
      else if (numsp!=1) {
	v_gtext(wsn,x,y,0,textarr[TX_DU_BIST_DRAN].t[lang[wsn]]);
      }
    }
  }
  wtime+=50;
}
