
/*
    xskat - a card game for 1 to 3 players.
    Copyright (C) 1998  Gunter Gerhardt

    This program is free software; you can redistribute it freely.
    Use it at your own risk; there is NO WARRANTY.
*/

#define SKAT_C

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "defs.h"
#include "skat.h"
#include "text.h"

int left(s)
int s;
{
  return (s+1)%3;
}

int right(s)
int s;
{
  return (s+2)%3;
}

int iscomp(s)
int s;
{
  return s>=numsp;
}

VOID swap(i,j)
int *i,*j;
{
  int h;

  h=*i;
  *i=*j;
  *j=h;
}

VOID setrnd(s,v)
long *s,v;
{
  *s=v<<1?v:-1;
}

int rndval(s,m)
long *s;
int m;
{
  register long h=*s;
  int i;

  for (i=0;i<7;i++) h=(h<<16)|((((h<<1)^(h<<4))>>16)&0xffff);
  *s=h;
  return h&m;
}

int rnd(m)
int m;
{
  return rndval(&seed[1],m);
}

VOID synerr(f,s)
FILE *f;
char *s;
{
  int c,l,n;

  fprintf(stderr,"Error in file %s",game_file);
  if (feof(f)) fputs(": unexpected EOF\n",stderr);
  else {
    fprintf(stderr," reading: '%s'\n",s);
    fputs("before:\n",stderr);
    l=3;
    n=200;
    while (l && n) {
      if ((c=fgetc(f))==EOF) l=0;
      else {
	if (c=='\n') l--;
	n--;
	fputc(c,stderr);
      }
    }
  }
  fclose(f);
  game_file=0;
}

int get_game()
{
  static int opened;
  static FILE *f;
  char s[100],*p;
  int n,cpos[4],cmax[4],state,rep;
  long num;

  if (predef==1) predef=0;
  if (!game_file) return 0;
  if (!opened) {
    if (!strcmp(game_file,"-")) {
      game_file="stdin";
      f=stdin;
    }
    else {
      f=fopen(game_file,"r");
    }
    if (!f) {
      fprintf(stderr,"Can't open file %s\n",game_file);
      exitus(1);
    }
    opened=1;
  }
  state=rep=0;
  while (state!=4) {
    if (fscanf(f," %99s",s)!=1) {
      if (!state) break;
      synerr(f,"");
      return 0;
    }
    else if (*s=='#') {
      do {
	fgets(s,99,f);
      } while (s[strlen(s)-1]!='\n' && s[0]);
    }
    else if (!strcmp(s,"random_seed")) {
      if (!state) state=2;
      else {
	synerr(f,s);
	return 0;
      }
    }
    else if (!strcmp(s,"repeat")) {
      if (!state) {
	if (rep) break;
	rewind(f);
	rep=1;
      }
      else {
	synerr(f,s);
	return 0;
      }
    }
    else {
      num=strtol(s,&p,10);
      if (p!=s+strlen(s)) {
	synerr(f,s);
	return 0;
      }
      switch (state) {
      case 0:
	n=0;
	cpos[0]=30;cmax[0]=2;
	cpos[1]=sager*10;cmax[1]=10;
	cpos[2]=geber*10;cmax[2]=10;
	cpos[3]=hoerer*10;cmax[3]=10;
	state=1;
      case 1:
	if (num<0 || num>3 || !cmax[num]) {
	  synerr(f,s);
	  return 0;
	}
	cards[cpos[num]]=n;
	cpos[num]++;
	cmax[num]--;
	n++;
	if (n==32) {
	  predef=1;
	  return 1;
	}
	break;
      case 2:
	predef=2;
	setrnd(&seed[0],savseed=num);
	state=3;
	break;
      case 3:
	gamenr=num;
	while (num-->0) {
	  for (n=0;n<32;n++) rndval(&seed[0],0);
	}
	if (fscanf(f," %99s",s)==1) {
	  if (toupper(*s)=='L') rotateby=-1;
	  else if (toupper(*s)=='R') rotateby=1;
	}
	state=4;
	break;
      }
    }
  }
  fclose(f);
  game_file=0;
  return 0;
}

VOID mischen()
{
  int i,j;
  static int savecards[32];

  if (wieder) {
    for (i=0;i<32;i++) cards[i]=savecards[i];
    if (wieder==1) {
      if (vorhandwn) rotateby=(rotateby+3)%3-1;
      for (i=0;i<10;i++) swap(&cards[i],&cards[10+i]);
      for (i=0;i<10;i++) swap(&cards[10+i],&cards[20+i]);
    }
    else if (wieder==3) {
      if (vorhandwn) rotateby=(rotateby+2)%3-1;
      for (i=0;i<10;i++) swap(&cards[i],&cards[20+i]);
      for (i=0;i<10;i++) swap(&cards[20+i],&cards[10+i]);
    }
    wieder=0;
  }
  else if (!get_game()) {
    for (i=0;i<32;i++) cards[i]=i;
    for (i=0;i<32;i++) swap(&cards[i],&cards[rndval(&seed[0],31)]);
    for (i=0;i<10;i++) swap(&cards[geber*10+i],&cards[i]);
    for (i=0;i<10;i++) swap(&cards[hoerer*10+i],&cards[geber==1?i:10+i]);
    if (rotateby<0) {
      for (i=0;i<10;i++) swap(&cards[i],&cards[10+i]);
      for (i=0;i<10;i++) swap(&cards[10+i],&cards[20+i]);
    }
    else if (rotateby>0) {
      for (i=0;i<10;i++) swap(&cards[i],&cards[20+i]);
      for (i=0;i<10;i++) swap(&cards[20+i],&cards[10+i]);
    }
    gamenr++;
  }
  for (i=0;i<32;i++) savecards[i]=cards[i];
  setrnd(&seed[1],seed[0]);
  for (i=0;i<32;i++) gespcd[i]=0;
  for (i=0;i<4;i++) gespfb[i]=0;
  butternok=0;
  for (i=0;i<3;i++) {
    for (j=0;j<5;j++) hatnfb[i][j]=0;
  }
}

int lower(c1,c2,n)
int c1,c2,n;
{
  int f1,f2,w1,w2;

  if (c1<0) return 1;
  if (c2<0) return 0;
  f1=c1>>3;
  f2=c2>>3;
  w1=c1&7;
  w2=c2&7;
  if (n) {
    if (sortw[f1]<sortw[f2]) return 1;
    if (sortw[f1]>sortw[f2]) return 0;
    if (w1==ZEHN) return w2<=BUBE;
    if (w2==ZEHN) return w1>BUBE;
    return w1>w2;
  }
  if (w2==BUBE) {
    if (w1!=BUBE) return 1;
    return f1<f2;
  }
  else {
    if (w1==BUBE) return 0;
    if (f2==trumpf && f1!=trumpf) return 1;
    if (f1==trumpf && f2!=trumpf) return 0;
    if (sortw[f1]<sortw[f2]) return 1;
    if (sortw[f1]>sortw[f2]) return 0;
    return w1>w2;
  }
}

VOID sort(sn)
int sn;
{
  int i,j,f=sn*10;
  int hatfb[4],fbsum,firstf,sptz;

  sortw[0]=0;
  sortw[1]=1;
  sortw[2]=2;
  sortw[3]=3;
  if (alternate[sn]) {
    hatfb[0]=hatfb[1]=hatfb[2]=hatfb[3]=0;
    for (i=f;i<f+10;i++) {
      if (cards[i]>=0 && ((cards[i]&7)!=BUBE || sort2[sn])) {
        hatfb[cards[i]>>3]=1;
      }
    }
    if (!sort2[sn] && trumpf>=0 && trumpf<4 && hatfb[trumpf]) {
      hatfb[trumpf]=0;
      firstf=trumpf;
    }
    else firstf=-1;
    fbsum=hatfb[0]+hatfb[1]+hatfb[2]+hatfb[3];
    if ((hatfb[0] || hatfb[1]) && (hatfb[2] || hatfb[3])) {
      switch (fbsum) {
      case 4:
        sortw[1]=2;
        sortw[2]=1;
        break;
      case 3:
        if (hatfb[0] && hatfb[1]) {
          sortw[0]=0;
          sortw[1]=2;
          sortw[2]=sortw[3]=1;
        }
        else {
          sortw[2]=0;
          sortw[3]=2;
          sortw[0]=sortw[1]=1;
        }
        break;
      case 2:
        if (firstf>1) {
          sortw[0]=sortw[1]=1;
          sortw[2]=sortw[3]=0;
        }
        break;
      }
    }
  }
  if (sn==spieler && spitzeang && !sort2[sn]) {
    sptz=(trumpf==4?BUBE:SIEBEN|trumpf<<3);
  }
  else sptz=-2;
  for (i=f;i<f+9;i++) {
    for (j=i+1;j<f+10;j++) {
      if (((cards[j]==sptz ||
	    lower(cards[i],cards[j],sort2[sn])) &&
	   cards[i]!=sptz)^sort1[sn]) {
	swap(&cards[i],&cards[j]);
      }
    }
  }
  sortw[0]=0;
  sortw[1]=1;
  sortw[2]=2;
  sortw[3]=3;
}

VOID calc_rw(s)
int s;
{
  int i,c,f,tr,bb,as,ze,dk,stg;
  int b[4],t[4];

  maxrw[s]=0;
  b[0]=b[1]=b[2]=b[3]=0;
  t[0]=t[1]=t[2]=t[3]=0;
  bb=as=ze=dk=0;
  for (i=0;i<10;i++) {
    c=cards[10*s+i];
    if ((c&7)==BUBE) {
      b[c>>3]=1;
      bb++;
    }
    else t[c>>3]++;
  }
  tr=0;
  for (i=1;i<4;i++) {
    if (t[i]>=t[tr]) tr=i;
  }
  for (i=0;i<10;i++) {
    c=cards[10*s+i];
    if ((c&7)!=BUBE && c>>3!=tr) {
      switch (c&7) {
      case AS:as++;break;
      case ZEHN:ze++;break;
      default:dk+=cardw[c&7];
      }
    }
  }
  if ((bb+t[tr]==4 &&
       (
        (as==2 && ze>=2 && dk+10*ze>=24) ||
        (as>=3 && as+ze>=4)
	)) ||
      (bb+t[tr]==5 &&
       (
        (ze>=2 && dk+10*ze>=27 && (b[3] || b[2])) ||
        (as>=1 && ze>=1 && dk+10*ze>=14 && (b[3] || b[2])) ||
        (as>=2 && dk+10*ze>=6) ||
        (as>=3)
	)) ||
      (bb+t[tr]==6 &&
       (
        (dk+10*ze+11*as>=11)
	)) ||
      bb+t[tr]>=7
      ) {
    f=2;
    if (b[3]) {
      while (f<5 && b[4-f]) f++;
    }
    maxrw[s]=f*rwert[tr];
  }
  if (!maxrw[s]) testnull(s);
  if (!maxrw[s] && (b[3] || b[2] || bb==2) &&
      ((b[3] && b[2] && as>=2) ||
       (bb+t[tr]==4 && as>=1 && dk+10*ze+11*as>=27) ||
       (bb+t[tr]==5 && dk+10*ze+11*as>=18) ||
       (bb+t[tr]==5 && bb==2 && 10*ze+11*as>=20) ||
       (bb+t[tr]==6))) maxrw[s]=18;
  stg=strateg[numsp==0?s:numsp==1?s-1:0];
  if (stg<0 && rnd(3)<-stg && maxrw[s] &&
      maxrw[s]!=nullw[0] && maxrw[s]!=nullw[1] && maxrw[s]!=nullw[2] &&
      maxrw[s]!=nullw[3] && maxrw[s]!=nullw[4]) {
    maxrw[s]=17;
  }
  if (stg>0 && rnd(3)<stg && !maxrw[s] &&
      (b[3] || b[2] || b[1])) maxrw[s]=17;
}

VOID do_geben()
{
  int sn,i;

  sort2[0]=sort2[1]=sort2[2]=0;
  prot2.verdopp[0]=prot2.verdopp[1]=prot2.verdopp[2]=0;
  schnang=schwang=ouveang=spitzeang=revolang=0;
  ndichtw=0;
  hintcard=-1;
  for (sn=0;sn<numsp;sn++) calc_desk(sn);
  if (!wieder) {
    if (ramschspiele) {
      if (trumpf==4) geber=right(geber);
      else ramschspiele--;
    }
    else if (bockspiele) {
      bockspiele--;
      if (bockspiele%3==0 && playbock==2) {
	ramschspiele=3;
      }
    }
    bockspiele+=3*bockinc;
    geber=left(geber);
  }
  else if (!vorhandwn) {
    geber=left(geber+wieder);
  }
  bockinc=0;
  trumpf=-1;
  hoerer=ausspl=left(geber);
  sager=right(geber);
  mischen();
  setcurs();
  givecard(hoerer,0);
  givecard(sager,0);
  givecard(geber,0);
  givecard(-1,0);
  givecard(hoerer,1);
  givecard(sager,1);
  givecard(geber,1);
  givecard(hoerer,2);
  givecard(sager,2);
  givecard(geber,2);
  for (sn=0;sn<numsp;sn++) initscr(sn,1);
  for (i=0;i<3;i++) {
    lastmsaho[i]=0;
    sagte18[i]=0;
  }
  kontrastufe=0;
  schenkstufe=0;
  saho=1;
  reizp=0;
  clear_info();
  if (ramschspiele) {
    phase=ANSAGEN;
    di_grandhand(hoerer);
  }
  else if (playramsch>1) {
    init_ramsch();
  }
  else {
    putmark(hoerer);
    put_box(sager);
    put_box(hoerer);
    for (sn=numsp;sn<3;sn++) calc_rw(sn);
    phase=REIZEN;
  }
}

VOID do_sagen(s,w)
int s,w;
{
  char str[4];

  sprintf(str,"%d",w);
  b_text(s,str);
  inv_box(s,0,1);
  stdwait();
  inv_box(s,0,0);
  sagte18[s]=1;
}

VOID do_passen(s)
int s;
{
  b_text(s,textarr[TX_PASSE]);
  inv_box(s,1,1);
  stdwait();
  inv_box(s,1,0);
  rem_box(s);
}

VOID do_akzept(s)
int s;
{
  b_text(s,textarr[TX_JA]);
  inv_box(s,0,1);
  stdwait();
  inv_box(s,0,0);
  b_text(s," ");
  sagte18[s]=1;
}

VOID do_msagen(sn,w)
int sn,w;
{
  char str[4];

  if (lastmsaho[sn]==w) return;
  lastmsaho[sn]=w;
  sprintf(str,"%d",w);
  do_msaho(sn,str);
}

VOID do_mhoeren(sn)
int sn;
{
  if (lastmsaho[sn]==1) return;
  lastmsaho[sn]=1;
  do_msaho(sn,textarr[TX_JA]);
}

VOID do_entsch()
{
  int rw;

  rw=reizw[reizp];
  if (saho) {
    if (maxrw[sager]>=rw || (maxrw[sager]==17 && rw==18)) {
      do_sagen(sager,rw);
      saho=0;
      if (sager==hoerer) {
        spieler=sager;
	do_handspiel();
      }
    }
    else {
      do_passen(sager);
      if (sager==geber || sager==hoerer) {
        if (sager==hoerer) {
          reizp--;
	  do_handspiel();
        }
        else {
          if (reizp) {
            spieler=hoerer;
            reizp--;
	    do_handspiel();
          }
          else {
            rem_box(sager);
            sager=hoerer;
          }
        }
      }
      else {
        rem_box(sager);
        sager=geber;
        put_box(sager);
      }
    }
  }
  else {
    if (maxrw[hoerer]>=rw) {
      do_akzept(hoerer);
      reizp++;
      saho=1;
    }
    else {
      do_passen(hoerer);
      if (sager==geber) {
        spieler=sager;
	do_handspiel();
      }
      else {
        rem_box(hoerer);
        rem_box(sager);
        hoerer=sager;
        sager=geber;
        reizp++;
        saho=1;
        put_box(hoerer);
        put_box(sager);
      }
    }
  }
}

VOID do_reizen()
{
  while (phase==REIZEN &&
	 ((iscomp(sager) && saho) || (iscomp(hoerer) && !saho))) {
    do_entsch();
  }
  if (phase==REIZEN) {
    if (saho) do_msagen(sager,reizw[reizp]);
    else do_mhoeren(hoerer);
  }
}

VOID drueck(f,n)
int f,n;
{
  int i,j;

  do {
    for (i=trumpf!=5;i<8;i++) {
      if (inhand[f][i]) {
        inhand[f][i]=0;
        if (!gedr && cards[31]==(f<<3)+i) {
          swap(&cards[30],&cards[31]);
          break;
        }
        for (j=0;j<10;j++) {
          if (cards[spieler*10+j]==(f<<3)+i) {
            swap(&cards[30+gedr],&cards[10*spieler+j]);
            break;
          }
        }
        break;
      }
    }
    gedr++;
  } while (--n && gedr<2);
}

VOID truempfe()
{
  int i,c;

  for (c=0;c<2;c++) {
    if ((cards[30+c]&7)==BUBE || cards[30+c]>>3==trumpf) {
      for (i=0;i<10;i++) {
        if ((cards[10*spieler+i]&7)!=BUBE
          && cards[10*spieler+i]>>3!=trumpf) {
          swap(&cards[30+c],&cards[10*spieler+i]);
          break;
        }
      }
    }
  }
}

int tr_voll(sn,f)
int sn,f;
{
  int i,c,t,a,z;

  if (trumpf==-1 || trumpf==4) return f;
  t=a=z=0;
  for (i=0;i<10;i++) {
    c=cards[10*sn+i];
    if ((c&7)==BUBE || c>>3==trumpf) t++;
    else if ((c&7)==AS) a++;
    else if ((c&7)==ZEHN) z++;
  }
  if (f) {
    return t>7 || (t>6 && a+z);
  }
  return t>5 || (t>4 && a+z) || (t>3 && a>2);
}

int sage_kontra(sn)
int sn;
{
  return tr_voll(sn,0);
}

int sage_re(sn)
int sn;
{
  return tr_voll(sn,1);
}

int testgrand(bb,b,vh)
int bb,b[4],vh;
{
  int i,j,fl,ih,g3,g4;

  fl=g3=g4=0;
  for (i=0;i<4;i++) {
    ih=0;
    for (j=0;j<8;j++) {
      if (j!=BUBE && inhand[i][j]) ih++;
    }
    for (j=0;j<8;j++) {
      if (j!=BUBE) {
        if (inhand[i][j]) fl++;
        else if (7-ih>j) break;
      }
    }
    if ((ih>4) ||
	(ih>3 && (inhand[i][AS] || inhand[i][ZEHN]))) g4=1;
    if (ih>4 && (inhand[i][AS] || inhand[i][ZEHN])) g3=1;
    if (ih>3 && inhand[i][AS] && inhand[i][ZEHN]) g3=1;
  }
  return ((fl+bb>6) ||
	  (bb==4 && g4) ||
	  (bb==3 && (b[3] || vh) && g3));
}

VOID calc_inhand(sn)
int sn;
{
  int i,c;

  for (i=0;i<4;i++) {
    for (c=0;c<8;c++) inhand[i][c]=0;
  }
  for (i=0;i<10;i++) {
    c=cards[10*sn+i];
    if (c>=0) {
      inhand[c>>3][c&7]=1;
    }
  }
}

VOID calc_drueck()
{
  int i,j,c,f,bb,n,sp;
  int b[4],t[4],p[4],o[4];

  if (maxrw[spieler]==nullw[0] ||
      maxrw[spieler]==nullw[1] ||
      maxrw[spieler]==nullw[2] ||
      maxrw[spieler]==nullw[3] ||
      maxrw[spieler]==nullw[4]) {
    trumpf=-1;
    if (maxrw[spieler]!=nullw[0] && maxrw[spieler]!=nullw[2]) handsp=1;
    if (maxrw[spieler]>=nullw[3]) ouveang=1;
    if (maxrw[spieler]==nullw[4]) revolang=1;
    gedr=2;
    return;
  }
  for (i=0;i<4;i++) {
    b[i]=t[i]=p[i]=0;
    o[i]=i;
  }
  bb=0;
  for (i=0;i<4;i++) {
    for (c=0;c<8;c++) inhand[i][c]=0;
  }
  for (i=0;i<12;i++) {
    c=spcards[i];
    if ((c&7)==BUBE) {
      b[c>>3]=1;
      bb++;
    }
    else {
      p[c>>3]+=cardw[c&7];
      t[c>>3]++;
      inhand[c>>3][c&7]=1;
    }
  }
  for (i=1;i<4;i++) {
    if (inhand[i][ZEHN] && !inhand[i][AS] && !inhand[i][KOENIG]) {
      o[i]=0;
      o[0]=i;
      break;
    }
  }
  f=2;
  while (f<5 && b[4-f]==b[3]) f++;
  trumpf=0;
  while (f*rwert[trumpf]<reizw[reizp]) trumpf++;
  for (i=trumpf+1;i<4;i++) {
    if (t[i]>t[trumpf] || (t[i]==t[trumpf] && p[i]<=p[trumpf])) trumpf=i;
  }
  truempfe();
  for (n=1;n<8 && gedr<2;n++) {
    for (j=0;j<4 && gedr<2;j++) {
      i=o[j];
      if (t[i]==n && trumpf!=i) {
	if (inhand[i][AS]) {
	  if (!inhand[i][ZEHN] && n!=1) drueck(i,n==2?1:2);
	}
	else drueck(i,n==1?1:2);
      }
    }
  }
  if ((bb>2 || (b[3] && bb==2 && spieler==ausspl)) &&
      testgrand(bb,b,spieler==ausspl)) {
    trumpf=4;
  }
  if (spitzezaehlt &&
      ((trumpf<4 && inhand[trumpf][SIEBEN] &&
	(t[trumpf]+bb>=7 || (t[trumpf]+bb==6 && bb>=2))) ||
       (trumpf==4 && b[0] &&
	bb==3 && (spieler==ausspl || b[3]+b[2]==2)))) {
    sp=trumpf==4?BUBE:SIEBEN|trumpf<<3;
    if (cards[30]!=sp && cards[31]!=sp) {
      spitzeang=1;
    }
  }
  gespcd[cards[30]]=1;
  gespcd[cards[31]]=1;
}

VOID nextgame()
{
  if (numgames) {
    phase=GEBEN;
    numgames--;
    if (!numgames) {
      printf("%d ",sum[0][alist[0]]);
      printf("%d ",sum[1][alist[0]]);
      printf("%d\n",sum[2][alist[0]]);
      quit=1;
    }
  }
}

VOID save_skat(i)
int i;
{
  if (lower(cards[31],cards[30],0)) {
    swap(&cards[31],&cards[30]);
  }
  prot2.skat[i][0]=cards[30];
  prot2.skat[i][1]=cards[31];
}

int check_bockevents()
{
  int e,m,i,ns,s;

  e=0;
  if (bockevents&BOCK_BEI_60) {
    if (!spgew && stsum==60 && trumpf>=0 && trumpf<=4) e++;
  }
  if (bockevents&BOCK_BEI_GRANDHAND) {
    if (spgew && trumpf==4 && handsp) e++;
  }
  m=0;
  if (bockevents&BOCK_BEI_KONTRA) {
    if (!spgew && kontrastufe) e++,m=1;
  }
  if (!m && bockevents&BOCK_BEI_RE) {
    if (kontrastufe==2) e++;
  }
  for (i=0;i<3;i++) {
    ns=sum[i][alist[0]];
    s=ns<0?-ns:ns;
    if (ns!=prevsum[i][alist[0]] &&
	((bockevents&BOCK_BEI_NNN &&
	  s%10==s/10%10 && s%10==s/100%10) ||
	 (bockevents&BOCK_BEI_N00 &&
	  s%100==0))) e++;
  }
  m=0;
  if (bockevents&BOCK_BEI_72) {
    if (nspwert>=72) e++,m=1;
  }
  if (!m && bockevents&BOCK_BEI_96) {
    if (nspwert>=96) e++;
  }
  return e;
}

VOID update_list()
{
  int i;

  if (splstp>=360) {
    for (i=0;i<12;i++) {
      modsum(splsum,i,(int *)0,(int *)0,(int *)0,(int *)0);
    }
    for (i=12;i<splstp;i++) splist[i-12]=splist[i];
    splstp-=12;
  }
  splist[splstp].s=spieler;
  splist[splstp].r=trumpf==5;
  splist[splstp].d=trumpf==5 && stsum==120 && spgew;
  splist[splstp].e=spwert;
  splist[splstp].g=spgew;
  memcpy((VOID *)prevsum,(VOID *)sum,sizeof(sum));
  modsum(sum,splstp,(int *)0,(int *)0,(int *)0,(int *)0);
  splstp++;
}

VOID do_grandhand(sn)
int sn;
{
  handsp=1;
  trumpf=4;
  reizp=-1;
  spieler=sn;
  do_handspiel();
}

VOID set_prot()
{
  prot2.stichgem=stich-1;
  prot2.spieler=spieler;
  prot2.trumpf=trumpf;
  prot2.gereizt=reizp<0?0:reizw[reizp];
  prot2.gewonn=spgew;
  prot2.augen=stsum;
  prot2.handsp=handsp;
  prot2.ehsso=
    revolang?handsp?1:0:
    ouveang?trumpf==-1 && handsp?5:4:
    schwang?3:schnang?2:handsp?1:0;
  prot2.predef=predef;
  if (trumpf!=5) prot2.sramsch=0;
  prot2.savseed=savseed;
  prot2.gamenr=gamenr-1;
  prot2.rotateby=rotateby;
  prot2.spitze=spitzeang;
  prot2.revolution=revolang;
  prot2.schenken=schenkstufe;
}

VOID do_handspiel()
{
  int i;

  prot2.anspiel[0]=ausspl;
  prot2.gemacht[0]=-1;
  if (reizp<0 && !ramschspiele) {
    if (playramsch>0) {
      init_ramsch();
      return;
    }
    clr_desk(0);
    phase=GEBEN;
    stich=1;
    fill_st();
    trumpf=4;
    set_prot();
    save_skat(1);
    prot1=prot2;
    spwert=0;
    update_list();
    save_list();
    if (logging) di_proto(0,1,1);
    splres=1;
    nextgame();
    return;
  }
  info_reiz();
  drkcd=0;
  if (!ramschspiele) handsp=0;
  stsum=0;
  vmh=0;
  gedr=0;
  for (i=0;i<10;i++) spcards[i]=cards[spieler*10+i];
  save_skat(0);
  spcards[10]=cards[30];
  spcards[11]=cards[31];
  rem_box(sager);
  rem_box(hoerer);
  if (!iscomp(spieler) && !ramschspiele) {
    phase=HANDSPIEL;
    di_hand();
  }
  else do_handok();
}

VOID do_handok()
{
  if (iscomp(spieler) || handsp) {
    home_skat();
    if (iscomp(spieler) && !handsp) calc_drueck();
    stsum=cardw[cards[30]&7]+cardw[cards[31]&7];
    save_skat(1);
  }
  if (!iscomp(spieler) && !handsp) {
    draw_skat();
    put_fbox(spieler,TX_DRUECKEN);
    phase=DRUECKEN;
  }
  else do_ansagen();
}

VOID do_ansagen()
{
  int i,c,bb;

  phase=ANSAGEN;
  bb=kannspitze=0;
  for (i=0;i<(handsp?10:12);i++) {
    c=i>=10?prot2.skat[1][i-10]:cards[spieler*10+i];
    if ((c&7)==BUBE) bb++;
    if (i<10) {
      if ((c&7)==SIEBEN) {
	kannspitze=1;
	break;
      }
      if (c==BUBE) {
	kannspitze=2;
      }
    }
  }
  if (kannspitze==2) {
    kannspitze=bb!=4;
  }
  if (!iscomp(spieler) && !ramschspiele) {
    di_spiel();
  }
  else {
    remmark(1);
    di_ansage();
  }
}

VOID karobube()
{
  int s,i,n,k,c;

  karobubeanz=0;
  if (trumpf<0 || trumpf>3) return;
  for (s=0;s<3;s++) {
    if (s==spieler) continue;
    n=k=0;
    for (i=0;i<10;i++) {
      c=cards[s*10+i];
      if ((c&7)==BUBE || c>>3==trumpf) {
	n++;
	if ((c&7)<KOENIG) n=9;
	if (c==BUBE) k=1;
	else if ((c&7)==BUBE) n=9;
      }
    }
    if (k && n==2) {
      karobubeanz=1;
    }
  }
}

int karobubespielen()
{
  int i;

  if (!karobubeanz) return 0;
  for (i=0;i<possc;i++) {
    if (cards[possi[i]]==BUBE) {
      playcd=i;
      return 1;
    }
  }
  return 0;
}

VOID do_angesagt()
{
  if (!iscomp(spieler)) {
    remmark(1);
  }
  stich=1;
  schwz=1;
  nullv=0;
  spitzeok=0;
  info_spiel();
  sort2[0]=sort2[1]=sort2[2]=trumpf==-1;
  if (revolang) revolutiondist();
  if (revolang && numsp && (numsp>1 || iscomp(spieler))) revolutionscr();
  else spielphase();
}

VOID spielphase()
{
  int sn,c,i;

  phase=SPIELEN;
  sptruempfe=0;
  for (i=0;i<10;i++) {
    c=cards[spieler*10+i];
    if ((c&7)==BUBE || c>>3==trumpf) sptruempfe++;
  }
  karobube();
  if (ouveang) {
    for (sn=0;sn<numsp;sn++) {
      di_info(sn,-2);
      calc_desk(sn);
    }
  }
  for (sn=0;sn<numsp;sn++) initscr(sn,1);
}

int higher(c1,c2)
int c1,c2;
{
  int f1,w1,f2,w2;

  if (c2==-1) return 1;
  f1=c1>>3;
  w1=c1&7;
  f2=c2>>3;
  w2=c2&7;
  if (trumpf==-1) {
    if (f1==f2) {
      if (w1==ZEHN) return w2>BUBE;
      if (w2==ZEHN) return w1<=BUBE;
      return w1<w2;
    }
    return 1;
  }
  if (w1==BUBE) {
    if (w2==BUBE) return f1>f2;
    else return 1;
  }
  if (w2==BUBE) return 0;
  if (f1==f2) return w1<w2;
  if (f2==trumpf) return 0;
  return 1;
}

VOID calc_result()
{
  int i,c,f;
  int b[4],s[8];

  mes1=mes2=mes3=mes4=0;
  if (trumpf==5) {
    ramsch_result();
    return;
  }
  if (trumpf==-1) {
    spwert=nullw[revolang?4:(ouveang?2:0)+handsp];
    if (nullv) {
      spgew=0;
      if (!handsp) spwert*=2;
      nspwert=0;
    }
    else {
      spgew=1;
      nspwert=spwert;
    }
    for (i=0;i<kontrastufe;i++) spwert*=2;
    if (bockspiele) spwert*=2;
    return;
  }
  if (stich==1 && schenkstufe) {
    stsum=61;
    schwz=0;
    nullv=1;
  }
  b[0]=b[1]=b[2]=b[3]=0;
  s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=0;
  for (i=0;i<12;i++) {
    c=spcards[i];
    if ((c&7)==BUBE) b[c>>3]=1;
    else if (c>>3==trumpf) s[c&7]=1;
  }
  s[BUBE]=s[NEUN];
  s[NEUN]=s[ACHT];
  s[ACHT]=s[SIEBEN];
  f=1;
  while (f<4 && b[3-f]==b[3]) f++;
  if (f==4 && trumpf!=4) {
    while (f<11 && s[f-4]==b[3]) f++;
  }
  f++;
  if (handsp) f++;
  if (stsum>=90 || schnang || stsum<=30) f++;
  if (schnang) f++;
  if (schwz || schwang || !nullv) f++;
  if (schwang) f++;
  if (ouveang) f++;
  if (spitzeang) f+=spitzezaehlt;
  if (trumpf==4 && ouveang) spwert=(f-1)*36;
  else spwert=f*rwert[trumpf];
  if ((stsum>60 &&
       spwert>=reizw[reizp] &&
       (stsum>=90 || !schnang) &&
       (schwz || !schwang) &&
       (spitzeok || !spitzeang))
      || stich==1) {
    spgew=1;
    nspwert=spwert;
  }
  else {
    if (spwert<reizw[reizp]) mes1=1;
    else if (schnang && stsum<90) mes2=1;
    else if (schwang && !schwz) mes3=1;
    else if (spitzeang && !spitzeok) mes4=1;
    spgew=0;
    if (spwert<reizw[reizp]) {
      spwert=((reizw[reizp]-1)/rwert[trumpf]+1)*rwert[trumpf];
    }
    if (!handsp) spwert*=2;
    nspwert=0;
  }
  for (i=0;i<kontrastufe;i++) spwert*=2;
  if (bockspiele && !ramschspiele) spwert*=2;
}

VOID get_next()
{
  int s;

  prot2.anspiel[stich-1]=ausspl;
  prot2.stiche[stich-1][ausspl]=stcd[0];
  prot2.stiche[stich-1][left(ausspl)]=stcd[1];
  prot2.stiche[stich-1][right(ausspl)]=stcd[2];
  if (trumpf==-1) null_stich();
  if (higher(stcd[0],stcd[1])) {
    if (higher(stcd[0],stcd[2])) s=0;
    else s=2;
  }
  else {
    if (higher(stcd[1],stcd[2])) s=1;
    else s=2;
  }
  ausspl=(ausspl+s)%3;
  prot2.gemacht[stich-1]=ausspl;
  if (spitzeang && stich==10 && ausspl==spieler &&
      stcd[s]==(trumpf==4?BUBE:SIEBEN|trumpf<<3)) {
    spitzeok=1;
  }
  if (trumpf==5) {
    ramsch_stich();
    return;
  }
  if (spieler==ausspl) {
    if (butternok==1) butternok=2;
    stsum+=cardw[stcd[0]&7]+cardw[stcd[1]&7]+cardw[stcd[2]&7];
    nullv=1;
  }
  else {
    if (butternok!=2) butternok=0;
    schwz=0;
  }
}

VOID save_list()
{
  FILE *f;
  int i;

  if (!list_file) return;
  f=fopen(list_file,"w");
  if (!f) {
    fprintf(stderr,"Can't write file %s\n",list_file);
    return;
  }
  fprintf(f,"%d %d %d\n",splsum[0][0],splsum[1][0],splsum[2][0]);
  fprintf(f,"a %d %d %d\n",splsum[0][1],splsum[1][1],splsum[2][1]);
  fprintf(f,"t %d %d %d\n",splsum[0][2],splsum[1][2],splsum[2][2]);
  fprintf(f,"b %d %d %d\n",bockspiele,bockinc,ramschspiele);
  for (i=0;i<splstp;i++) {
    fprintf(f,"e %d %d %d %d %d\n",
	    splist[i].s,splist[i].r,splist[i].d,
	    splist[i].e,splist[i].g);
  }
  fclose(f);
}

VOID setsum(clr)
int clr;
{
  int i,j;

  splstp=0;
  for (i=0;i<3;i++) {
    splfirst[i]=0;
    for (j=0;j<3;j++) {
      if (clr) sum[i][j]=0;
      splsum[i][j]=sum[i][j];
    }
  }
}

VOID modsum(sm,p,as,ae,ar,ad)
int sm[3][3],p,*as,*ae,*ar,*ad;
{
  int s,e,r,d;

  s=splist[p].s;
  r=splist[p].r;
  d=splist[p].d;
  e=splist[p].e;
  if (!splist[p].g) e=-e;
  if (e<=0 || !r || d) {
    sm[s][0]+=e;
    sm[s][2]+=e;
  }
  if (e<0) {
    sm[s][1]-=e;
    if (!r) {
      sm[s][2]-=50;
      sm[left(s)][2]+=40;
      sm[right(s)][2]+=40;
    }
  }
  else {
    if (r && !d) {
      sm[left(s)][0]-=e;
      sm[right(s)][0]-=e;
      sm[left(s)][2]-=e;
      sm[right(s)][2]-=e;
    }
    sm[left(s)][1]+=e;
    sm[right(s)][1]+=e;
    if (!r && e) {
      sm[s][2]+=50;
    }
  }
  if (as) *as=s;
  if (ae) *ae=e;
  if (ar) *ar=r;
  if (ad) *ad=d;
}

VOID read_opt_srpk(f)
FILE *f;
{
  int i,v,va[3];

  if (fscanf(f,"s %d %d %d\n",&va[0],&va[1],&va[2])==3) {
    for (i=0;i<3;i++) {
      if (!stgset[i]) {
	if (va[i]<-4) va[i]=-4;
	else if (va[i]>4) va[i]=4;
	strateg[i]=va[i];
      }
    }
  }
  if (fscanf(f,"r %d\n",&v)==1) {
    if (!ramschset) {
      if (v<0) v=0;
      else if (v>2) v=2;
      playramsch=v;
    }
  }
  if (fscanf(f,"p %d\n",&v)==1) {
    if (!sramschset) {
      playsramsch=!!v;
    }
  }
  if (fscanf(f,"k %d\n",&v)==1) {
    if (!kontraset) {
      if (v<0) v=0;
      else if (v>2) v=2;
      playkontra=v;
    }
  }
}

VOID read_opt()
{
  FILE *f;
  int v,va[3],i;

  if (!opt_file) return;
  f=fopen(opt_file,"r");
  if (!f) return;
  read_opt_srpk(f);
  if (fscanf(f,"b %d\n",&v)==1) {
    if (!bockset) {
      if (v<0) v=0;
      else if (v>2) v=2;
      playbock=v;
    }
  }
  if (fscanf(f,"e %d\n",&v)==1) {
    if (!bockeventsset) {
      bockevents=v;
    }
  }
  if (fscanf(f,"c %d\n",&v)==1) {
    if (!resumebockset) {
      resumebock=!!v;
    }
  }
  if (fscanf(f,"d %d %d %d\n",&va[0],&va[1],&va[2])==3) {
    for (i=0;i<3;i++) {
      if (useoptfile[i] && !downupset[i]) {
	sort1[i]=!!va[i];
      }
    }
  }
  if (fscanf(f,"a %d %d %d\n",&va[0],&va[1],&va[2])==3) {
    for (i=0;i<3;i++) {
      if (useoptfile[i] && !altseqset[i]) {
	alternate[i]=!!va[i];
      }
    }
  }
  if (fscanf(f,"t %d %d %d\n",&va[0],&va[1],&va[2])==3) {
    for (i=0;i<3;i++) {
      if (useoptfile[i] && !tdelayset[i]) {
	if (va[i]<0) va[i]=7;
	else if (va[i]>101) va[i]=101;
	nimmstich[i][0]=va[i];
      }
    }
  }
  if (fscanf(f,"l %d %d %d\n",&va[0],&va[1],&va[2])==3) {
    for (i=0;i<3;i++) {
      if (useoptfile[i] && !alistset[i]) {
	if (va[i]<0) va[i]=0;
	else if (va[i]>2) va[i]=2;
	alist[i]=va[i];
      }
    }
  }
  if (fscanf(f,"m %d %d %d\n",&va[0],&va[1],&va[2])==3) {
    for (i=0;i<3;i++) {
      if (useoptfile[i] && !mbuttonset[i]) {
	if (va[i]<0 || va[i]>5) va[i]=0;
	mbutton[i]=va[i];
      }
    }
  }
  if (fscanf(f,"s %d\n",&v)==1) {
    if (!spitzezaehltset) {
      spitzezaehlt=v;
    }
  }
  if (fscanf(f,"r %d\n",&v)==1) {
    if (!revolutionset) {
      revolution=!!v;
    }
  }
  if (fscanf(f,"k %d\n",&v)==1) {
    if (!klopfenset) {
      klopfen=!!v;
    }
  }
  if (fscanf(f,"s %d\n",&v)==1) {
    if (!schenkenset) {
      schenken=!!v;
    }
  }
  if (fscanf(f,"f %d\n",&v)==1) {
    if (!fastdealset) {
      fastdeal=!!v;
    }
  }
  if (fscanf(f,"h %d %d %d\n",&va[0],&va[1],&va[2])==3) {
    for (i=0;i<3;i++) {
      if (useoptfile[i] && !hintsset[i]) {
	hints[i]=!!va[i];
      }
    }
  }
  fclose(f);
}

VOID save_opt()
{
  FILE *f;

  if (!opt_file) return;
  f=fopen(opt_file,"w");
  if (!f) {
    fprintf(stderr,"Can't write file %s\n",opt_file);
    return;
  }
  fprintf(f,"s %d %d %d\n",strateg[0],strateg[1],strateg[2]);
  fprintf(f,"r %d\n",playramsch);
  fprintf(f,"p %d\n",playsramsch);
  fprintf(f,"k %d\n",playkontra);
  fprintf(f,"b %d\n",playbock);
  fprintf(f,"e %d\n",bockevents);
  fprintf(f,"c %d\n",resumebock);
  fprintf(f,"d %d %d %d\n",sort1[0],sort1[1],sort1[2]);
  fprintf(f,"a %d %d %d\n",alternate[0],alternate[1],alternate[2]);
  fprintf(f,"t %d %d %d\n",nimmstich[0][0],nimmstich[1][0],nimmstich[2][0]);
  fprintf(f,"l %d %d %d\n",alist[0],alist[1],alist[2]);
  fprintf(f,"m %d %d %d\n",mbutton[0],mbutton[1],mbutton[2]);
  fprintf(f,"s %d\n",spitzezaehlt);
  fprintf(f,"r %d\n",revolution);
  fprintf(f,"k %d\n",klopfen);
  fprintf(f,"s %d\n",schenken);
  fprintf(f,"f %d\n",fastdeal);
  fprintf(f,"h %d %d %d\n",hints[0],hints[1],hints[2]);
  fclose(f);
}

VOID read_list()
{
  FILE *f;
  int c,i,j;

  if (!list_file) return;
  f=fopen(list_file,"r");
  if (!f) return;
  splstp=0;
  if (fscanf(f,"%d %d %d\n",&splsum[0][0],&splsum[1][0],&splsum[2][0])!=3) {
    splsum[0][0]=splsum[1][0]=splsum[2][0]=0;
    fclose(f);
    return;
  }
  if (fscanf(f,"a %d %d %d\n",&splsum[0][1],&splsum[1][1],&splsum[2][1])!=3) {
    splsum[0][1]=splsum[1][1]=splsum[2][1]=0;
  }
  if (fscanf(f,"t %d %d %d\n",&splsum[0][2],&splsum[1][2],&splsum[2][2])!=3) {
    splsum[0][2]=splsum[1][2]=splsum[2][2]=0;
  }
  read_opt_srpk(f);
  if (fscanf(f,"b %d %d %d\n",&bockspiele,&bockinc,&ramschspiele)!=3) {
    bockspiele=bockinc=ramschspiele=0;
  }
  for (i=0;i<3;i++) {
    for (j=0;j<3;j++) {
      sum[i][j]=splsum[i][j];
    }
  }
  do {
    if (fscanf(f,"%d\n",&i)==1) {
      splist[splstp].s=i/10000;
      splist[splstp].r=(i-splist[splstp].s*10000)/4000;
      splist[splstp].d=(i-splist[splstp].s*10000-splist[splstp].r*4000)/2000;
      splist[splstp].e=i%2000>>1;
      splist[splstp].g=i&1;
      modsum(sum,splstp,(int *)0,(int *)0,(int *)0,(int *)0);
      splstp++;
    }
    else if (fscanf(f,"e %d %d %d %d %d\n",
		    &splist[splstp].s,&splist[splstp].r,&splist[splstp].d,
		    &splist[splstp].e,&splist[splstp].g)==5) {
      modsum(sum,splstp,(int *)0,(int *)0,(int *)0,(int *)0);
      splstp++;
    }
    else {
      while ((c=getc(f))!=EOF && c!='\n');
    }
  } while (!feof(f) && splstp<360);
  fclose(f);
}

VOID fill_st()
{
  int i,j,s,c,sc;

  for (s=0;s<3;s++) {
    if (vmh>=1 && s==ausspl) {
      sc=stcd[0];
    }
    else if (vmh==2 && s==left(ausspl)) {
      sc=stcd[1];
    }
    else {
      sc=-1;
    }
    i=stich-1;
    for (j=0;j<10;j++) {
      c=cards[10*s+j];
      if (c<0 && sc>=0) {
	c=sc;
	sc=-1;
      }
      if (c>=0) prot2.stiche[i++][s]=c;
    }
  }
}

int maxnimm()
{
  int i,m;

  m=nimmstich[0][0];
  for (i=1;i<numsp;i++) {
    if (nimmstich[i][0]>m) m=nimmstich[i][0];
  }
  return m;
}

VOID next_stich()
{
  int nd;

  if (maxnimm()<101) {
    waitt(maxnimm()*100,2);
  }
  info_stich(0,stcd[0],0);
  info_stich(1,stcd[1],0);
  info_stich(2,stcd[2],0);
  nimm_stich();
  vmh=0;
  stich++;
  nd=0;
  if (stich==11 ||
      (trumpf==-1 &&
       (nullv ||
	(!ndichtw && stich<10 &&
	 (nd=null_dicht(spieler,handsp,&prot2.skat[1][0],
			(int *)0,(int *)0,(int *)0)))))) {
    if (nd) di_dicht();
    else finishgame();
  }
}

VOID finishgame()
{
  int i,s;

  if (stich<11) {
    if (trumpf<0 || trumpf>4) fill_st();
    else {
      while (stich!=11) {
	s=(ausspl+vmh)%3;
	calc_poss(s);
	make_best(s);
	i=possi[playcd];
	stcd[vmh]=cards[i];
	cards[i]=-1;
	if (vmh==2) {
	  get_next();
	  vmh=0;
	  stich++;
	}
	else vmh++;
      }
    }
  }
  calc_result();
  set_prot();
  prot1=prot2;
  update_list();
  if (playbock) bockinc=check_bockevents();
  save_list();
  if (logging) di_proto(0,1,1);
  splres=1;
  clr_desk(0);
  phase=RESULT;
  di_result(bockinc);
  nextgame();
}

VOID do_next()
{
  int sn;

  if (vmh==2) {
    get_next();
    for (sn=0;sn<numsp;sn++) {
      if (nimmstich[sn][0]>=101) {
	nimmstich[sn][1]=1;
	phase=NIMMSTICH;
      }
    }
    if (phase==NIMMSTICH) return;
    next_stich();
  }
  else vmh++;
}

VOID calc_poss(s)
int s;
{
  int i,j,f1,w1,f2,w2;

  possc=0;
  for (i=0;i<10;i++) {
    if (cards[s*10+i]>=0) possi[possc++]=s*10+i;
  }
  if (vmh) {
    f1=stcd[0]>>3;
    w1=stcd[0]&7;
    if (trumpf!=-1 && w1==BUBE) f1=trumpf;
    i=j=0;
    do {
      f2=cards[possi[i]]>>3;
      w2=cards[possi[i]]&7;
      if (trumpf!=-1 && w2==BUBE) f2=trumpf;
      if (f1==f2) possi[j++]=possi[i];
    } while (++i<possc);
    if (j) possc=j;
    else hatnfb[s][f1>4?4:f1]=1;
  }
}

VOID c_high(f,h)
int f;
int *h;
{
  int i,j;

  h[0]=h[1]=h[2]=h[3]=h[4]=-1;
  for (i=0;i<4;i++) {
    for (j=0;j<8;j++) {
      if (j==BUBE) j++;
      if (gespcd[i<<3|j]<f) {
        h[i]=i<<3|j;
        break;
      }
    }
  }
  for (i=3;i>=0;i--) {
    if (gespcd[i<<3|BUBE]<f) {
      h[trumpf]=i<<3|BUBE;
      break;
    }
  }
}

VOID calc_high(f,s)
int f,s;
{
  int i,gespsav[32];

  c_high(f,high);
  if (!s) return;
  for (i=0;i<32;i++) gespsav[i]=gespcd[i];
  for (i=0;i<5;i++) {
    if (high[i]>=0) gespcd[high[i]]=2;
  }
  c_high(f,shigh);
  for (i=0;i<32;i++) gespcd[i]=gespsav[i];
}

int zweihoechste(ci)
int ci;
{
  int i,tr,trdr,cj;

  calc_high(1,1);
  if (ci!=high[trumpf]) return 0;
  for (i=0;i<possc;i++) {
    cj=cards[possi[i]];
    if (cj==shigh[trumpf]) break;
  }
  tr=0;
  for (i=0;i<possc;i++) {
    if (cards[possi[i]]>>3==trumpf || (cards[possi[i]]&7)==BUBE) {
      tr++;
    }
  }
  if (trumpf<4) trdr=7-gespfb[trumpf];
  else trdr=0;
  for (i=0;i<4;i++) if (!gespcd[i<<3|BUBE]) trdr++;
  return ci!=cj && cj==shigh[trumpf] && trdr-tr<=1;
}

int ignorieren()
{
  int mi,fb,i,ih,k[8];

  mi=right(ausspl);
  fb=stcd[0]>>3;
  if ((stcd[0]&7)==BUBE) fb=trumpf;
  ih=0;
  for (i=0;i<8;i++) k[i]=0;
  for (i=0;i<possc;i++) {
    if (cards[possi[i]]>>3==fb) {
      ih++;
      k[cards[possi[i]]&7]=1;
    }
  }
  if (ih==2) {
    k[BUBE]=k[NEUN];
    for (i=AS;i<=DAME;i++) {
      if (k[i] && k[i+1]) return 0;
    }
  }
  return fb!=trumpf && !cardw[stcd[0]&7] && hatnfb[mi][fb]!=1 && ih<3;
}

int gegenbuben()
{
  int s,i;

  for (s=0;s<3;s++) {
    if (s==spieler) continue;
    for (i=0;i<10;i++) {
      if (cards[s*10+i]>=0 && (cards[s*10+i]&7)==BUBE) return 1;
    }
  }
  return 0;
}

int uebernehmen(p,h)
int p,h;
{
  int i,j,ci,cj,wi,wj,fb,is;

  is=(ausspl+vmh)%3==spieler;
  j=0;
  calc_inhand((ausspl+vmh)%3);
  for (i=0;i<possc;i++) {
    ci=cards[possi[i]];
    if (!higher(stcd[p],ci)) {
      if (j) {
        cj=cards[possi[j-1]];
        wi=cardw[ci&7];
        wj=cardw[cj&7];
        if (is) {
	  if (h) {
	    calc_high(1,1);
	    fb=wj==2?trumpf:cj>>3;
	    if (cj==high[fb] &&
		shigh[fb]>=0 &&
		!inhand[shigh[fb]>>3][shigh[fb]&7]) {
	      j=i+1;
	      continue;
	    }
	    fb=wi==2?trumpf:ci>>3;
	    if (ci==high[fb] &&
		shigh[fb]>=0 &&
		!inhand[shigh[fb]>>3][shigh[fb]&7]) continue;
	  }
          if (wj==11 && !gegenbuben()) continue;
          if (wi==11 && cj>>3!=trumpf) {
            j=i+1;
            continue;
          }
        }
        if (wi==10) wi=12-h*2;
        if (wj==10) wj=12-h*2;
        if (wi==2 && wj==2) {
	  if (trumpf==4 && is) {
	    wi=ci>>3;
	    wj=cj>>3;
	  }
	  else {
	    wi=cj>>3;
	    wj=ci>>3;
	  }
        }
        else {
          if (wi==2) wi=5-h*6;
          if (wj==2) wj=5-h*6;
        }
	if (is) {
	  if (!h && zweihoechste(ci)) j=i+1;
	  else {
	    if ((h || !zweihoechste(cj)) && ((wi<wj)^h)) j=i+1;
	  }
	}
	else {
	  if ((wi<wj)^h) j=i+1;
	}
      }
      else j=i+1;
    }
  }
  if (j) {
    cj=cards[possi[j-1]];
    wj=cardw[cj&7];
    if (!h && wj==10 && gespcd[(cj&0x18)|AS]<!is+1) j=0;
    else playcd=j-1;
  }
  return j!=0;
}

VOID schmieren()
{
  int i,j,wi,wj,ci,cj,aw[4];

  j=0;
  aw[0]=aw[1]=aw[2]=aw[3]=9;
  for (i=0;i<possc;i++) {
    ci=cards[possi[i]];
    if ((ci&7)==ZEHN) aw[ci>>3]=11;
  }
  for (i=1;i<possc;i++) {
    wi=cardw[(ci=cards[possi[i]])&7];
    wj=cardw[(cj=cards[possi[j]])&7];
    if (wi==2) wi=-1;
    else if (ci>>3==trumpf && cj>>3!=trumpf) wi=1;
    else if (wi==11) wi=aw[ci>>3];
    if (wj==2) wj=-1;
    else if (cj>>3==trumpf && ci>>3!=trumpf) wj=1;
    else if (wj==11) wj=aw[cj>>3];
    if (wi>wj) j=i;
  }
  playcd=j;
}

int einstechen()
{
  int ci;

  if (!cardw[stcd[0]&7] || !uebernehmen(0,0)) return 0;
  ci=cards[possi[playcd]];
  if ((ci&7)<=ZEHN || (ci&7)==BUBE) return 0;
  if (ci>>3==trumpf) return 1;
  return 0;
}

int niedrighoch(f)
int f;
{
  int i,j,ok,gespsav[32];

  for (i=0;i<32;i++) gespsav[i]=gespcd[i];
  ok=j=0;
  do {
    calc_high(1,0);
    if (ok) ok=2;
    for (i=0;i<possc;i++) {
      if (cards[possi[i]]==high[f]) {
	j++;
	if (f!=trumpf || j<3) {
	  ok=1;
	  playcd=i;
	  gespcd[cards[possi[i]]]=2;
	}
      }
    }
  } while (ok==1);
  for (i=0;i<32;i++) gespcd[i]=gespsav[i];
  return ok;
}

int ueberdoerfer()
{
  int i,j;

  if (trumpf==4 || sptruempfe>4) return 0;
  calc_high(1,0);
  for (i=0;i<possc;i++) {
    for (j=0;j<4;j++) {
      if (j!=trumpf &&
	  cards[possi[i]]==high[j]) {
	playcd=i;
	return 1;
      }
    }
  }
  return 0;
}

int trumpfausspielen()
{
  int i,j,k,g1,g2,tr,trdr,wi,wj;

  g1=left(spieler);
  g2=right(spieler);
  if (!hatnfb[g1][trumpf] ||
    !hatnfb[g2][trumpf]) {
    if (niedrighoch(trumpf)) return 1;
  }
  calc_high(1,0);
  tr=wj=0;
  j=-1;
  for (i=0;i<possc;i++) {
    if (cards[possi[i]]>>3==trumpf || (cards[possi[i]]&7)==BUBE) {
      tr++;
      wi=cardw[cards[possi[i]]&7];
      if (wi==2) wi=-1;
      if (j<0 || wi<wj) {
        j=i;
        wj=wi;
      }
    }
  }
  k=possc;
  if (trumpf<4) {
    trdr=7-gespfb[trumpf];
    if (wj!=-1 && (hatnfb[g1][trumpf] || hatnfb[g2][trumpf])) {
      calc_inhand(spieler);
      for (i=SIEBEN;i>=DAME;i--) {
	if (i==BUBE) continue;
	if (!gespcd[trumpf<<3|i] && !inhand[trumpf][i]) {
	  for (;i>=KOENIG;i--) {
	    if (i==BUBE) continue;
	    if (inhand[trumpf][i]) {
	      for (k=0;k<possc;k++) {
		if (cards[possi[k]]==(trumpf<<3|i)) {
		  break;
		}
	      }
	      break;
	    }
	  }
	  break;
	}
      }
    }
  }
  else trdr=0;
  for (i=0;i<4;i++) if (!gespcd[i<<3|BUBE]) trdr++;
  if ((tr>2 && (trumpf!=4 || trdr-tr)) || (tr>1 && trdr-tr && trdr-tr<=2)) {
    playcd=k!=possc && (trdr-tr==2 || !cardw[cards[possi[k]]&7])?k:j;
    return 1;
  }
  for (i=0;i<possc;i++) {
    for (j=0;j<4;j++) {
      if (j!=trumpf && cards[possi[i]]==high[j]) {
	if ((cards[possi[i]]&7)==AS) playcd=i;
        else niedrighoch(j);
        return 1;
      }
    }
  }
  return 0;
}

int hochausspielen()
{
  int i,j;

  if (stich<=3) return 0;
  calc_high(2,0);
  for (i=0;i<possc;i++) {
    for (j=0;j<4;j++) {
      if (j!=trumpf &&
	  cards[possi[i]]==high[j] &&
	  !hatnfb[spieler][j]) {
	playcd=i;
	return 1;
      }
    }
  }
  return 0;
}

VOID schenke()
{
  int i,j,ci,cj,wi,wj,iw,jw;

  j=0;
  for (i=1;i<possc;i++) {
    ci=cards[possi[i]];
    cj=cards[possi[j]];
    wi=cardw[iw=(ci&7)];
    wj=cardw[jw=(cj&7)];
    if (wi==2) wi=5;
    if (wj==2) wj=5;
    if (wi==5 && wj==5) {
      wi=ci>>3;
      wj=cj>>3;
    }
    else {
      if (!wi && !gespcd[(ci&~7)|AS] && zehnblank(ci)) wi+=5;
      if (!wj && !gespcd[(cj&~7)|AS] && zehnblank(cj)) wj+=5;
    }
    if ((ci&7)!=BUBE && ci>>3==trumpf) wi+=5;
    if ((cj&7)!=BUBE && cj>>3==trumpf) wj+=5;
    if (wi<wj || (wi==wj && iw>=NEUN && jw>=NEUN && iw>jw)) j=i;
  }
  playcd=j;
}

int zehnblank(ci)
int ci;
{
  int i,f,n,z,a,cj;

  f=ci>>3;
  n=z=a=0;
  for (i=0;i<possc;i++) {
    cj=cards[possi[i]];
    if (cj>>3==f) {
      n++;
      if ((cj&7)==ZEHN) z=1;
      else if ((cj&7)==AS) a=1;
    }
  }
  return z && !a && n==2 && !hatnfb[spieler][f];
}

int fabwerfen()
{
  int i,fb,ci,n[4];

  fb=stcd[0]>>3;
  if (!hatnfb[spieler][fb] ||
      cardw[stcd[0]&7]+cardw[stcd[1]&7]>4) return 0;
  n[0]=n[1]=n[2]=n[3]=0;
  for (i=0;i<possc;i++) {
    ci=cards[possi[i]];
    if ((ci&7)!=BUBE && ci>>3!=trumpf) {
      n[ci>>3]++;
    }
  }
  calc_high(1,0);
  for (i=0;i<possc;i++) {
    ci=cards[possi[i]];
    fb=ci>>3;
    if ((ci&7)!=BUBE && fb!=trumpf && cardw[ci&7]<=4 &&
	n[fb]==1 && ci!=high[fb]) {
      playcd=i;
      return 1;
    }
  }
  return 0;
}

VOID abwerfen(f)
int f;
{
  int i,j,k,ci,cj,wi,wj,mi,wio,wjo,h;
  int n[4];

  j=0;
  for (i=1;i<possc;i++) {
    ci=cards[possi[i]];
    cj=cards[possi[j]];
    wi=cardw[ci&7];
    wj=cardw[cj&7];
    wio=wi;
    wjo=wj;
    if (wi==2) wi=5;
    if (wj==2) wj=5;
    if (wi==5 && wj==5) {
      wi=ci>>3;
      wj=cj>>3;
    }
    else {
      if (wi==5 || ci>>3==trumpf) wi+=5;
      if (wj==5 || cj>>3==trumpf) wj+=5;
      if (!wi && zehnblank(ci)) wi+=5;
      if (!wj && zehnblank(cj)) wj+=5;
      if (f) {
	if (trumpf==4) {
	  if ((ci&7)!=BUBE && hatnfb[spieler][ci>>3]) wi-=30;
	  if ((cj&7)!=BUBE && hatnfb[spieler][cj>>3]) wj-=30;
	}
	else {
	  mi=spieler==left(ausspl)?2:1;
	  wio=wi;
	  wjo=wj;
	  if (!hatnfb[spieler][ci>>3] || wi>4) wi+=8;
	  if (!hatnfb[spieler][cj>>3] || wj>4) wj+=8;
	  if (mi==2 && hatnfb[(ausspl+mi)%3][trumpf]!=1) {
	    h=0;
	    if (hatnfb[(ausspl+mi)%3][ci>>3]==1 && wio<=4) wi-=30,h++;
	    if (hatnfb[(ausspl+mi)%3][cj>>3]==1 && wjo<=4) wj-=30,h++;
	    if (h==2) swap(&wi,&wj);
	  }
	}
        if (wi==wj && stich<=3 && ci>>3!=cj>>3) {
          n[0]=n[1]=n[2]=n[3]=0;
          for (k=0;k<possc;k++) {
            if ((cards[possi[k]]&7)!=BUBE) {
              n[cards[possi[k]]>>3]++;
            }
          }
          if (n[ci>>3]<n[cj>>3]) wi=0;
          if (spieler==left(ausspl)) swap(&wi,&wj);
        }
      }
      else {
	if (possc==2 && ((stcd[0]&7)==BUBE || stcd[0]>>3==trumpf) &&
	    (wio==2 || wjo==2) && (wio==11 || wjo==11)) {
	  if (wio==11) wi=1,wj=2;
	  else wi=2,wj=1;
	  if (gespcd[BUBE]==2 || ci==BUBE || cj==BUBE ||
	      gespcd[2<<3|BUBE]!=2 || gespcd[3<<3|BUBE]!=2) swap(&wi,&wj);
	}
	else {
	  if ((ci&7)==BUBE || (!hatnfb[spieler][ci>>3] && wi>=4)) wi+=8;
	  if ((cj&7)==BUBE || (!hatnfb[spieler][cj>>3] && wj>=4)) wj+=8;
	}
      }
    }
    if (wi<wj) j=i;
  }
  playcd=j;
}

int buttern()
{
  int fb,mi,se;

  se=left(ausspl);
  mi=spieler==ausspl?right(ausspl):ausspl;
  fb=stcd[0]>>3;
  if ((stcd[0]&7)==BUBE) fb=trumpf;
  if (!hatnfb[se][fb]) return 0;
  calc_high(2,0);
  if (spieler==ausspl) {
    if (fb==4 ||
	higher(stcd[0],high[fb]) ||
	(hatnfb[mi][fb]==1 && hatnfb[mi][trumpf]==1)) return 0;
    if (butternok) return 0;
    butternok=1;
    return 1;
  }
  if (higher(stcd[0],high[trumpf]) && higher(stcd[0],high[fb])) return 1;
  if (higher(stcd[0],high[fb]) && !hatnfb[spieler][fb]) {
    return 1;
  }
  return 0;
}

int hatas()
{
  int f,as,i;

  f=stcd[0]>>3;
  if ((stcd[0]&7)==BUBE || possc>2 ||
      cardw[stcd[0]&7]>4 || hatnfb[spieler][f] || f==trumpf) return 0;
  as=f<<3|AS;
  for (i=0;i<possc;i++) {
    if (cards[possi[i]]==as) {
      playcd=i;
      return 1;
    }
  }
  return 0;
}

int schnippeln(f)
int f;
{
  int fb,i,j,k,as,hi;

  fb=stcd[0]>>3;
  if ((stcd[0]&7)==BUBE ||
      (stcd[f]&7)==BUBE ||
      fb==trumpf || stcd[f]>>3==trumpf ||
      (f && fb!=stcd[1]>>3) ||
      gespcd[fb<<3|ZEHN]==2 ||
      gespfb[fb]>3) {
    return 0;
  }
  as=0;
  for (i=0;i<possc;i++) {
    if (cards[possi[i]]==(fb<<3|AS)) as=i+1;
    if (cards[possi[i]]==(fb<<3|ZEHN)) return 0;
  }
  if (!as) return 0;
  possi[as-1]=possi[--possc];
  j=k=0;
  for (i=1;i<possc;i++) {
    if (cards[possi[i]]<cards[possi[j]]) j=i;
    if (cards[possi[i]]>cards[possi[k]]) k=i;
  }
  hi=f?higher(stcd[0],stcd[1])^(spieler==ausspl):cards[possi[j]]<stcd[0];
  playcd=hi?j:k;
  return 1;
}

VOID nichtspitze()
{
  int sp,i;

  if (spitzeang) {
    sp=trumpf==4?BUBE:SIEBEN|trumpf<<3;
    for (i=0;i<possc;i++) {
      if (cards[possi[i]]==sp) {
	possc--;
	for (;i<possc;i++) {
	  possi[i]=possi[i+1];
	}
	return;
      }
    }
  }
}

int spitzefangen()
{
  int i,c,t;

  if (!spitzeang || stich!=9) return 0;
  t=-1;
  for (i=0;i<possc;i++) {
    if (((c=cards[possi[i]])&7)==BUBE || c>>3==trumpf) {
      if (t!=-1) return 0;
      t=i;
    }
  }
  if (t==-1) return 0;
  playcd=t?0:1;
  return 1;
}

int restbeimir()
{
  int c,h,i,j,k,s[2];

  if (stich==10 || vmh || ausspl!=spieler || trumpf<0 || trumpf>4) return 0;
  s[0]=left(spieler);
  s[1]=right(spieler);
  if (!hatnfb[s[0]][trumpf] ||
      !hatnfb[s[1]][trumpf]) return 0;
  for (i=0;i<4;i++) {
    if (i==trumpf) continue;
    h=SIEBEN+1;
    for (j=0;j<2;j++) {
      for (k=0;k<10;k++) {
	if ((c=cards[s[j]*10+k])>=0 &&
	    c>>3==i && (c&7)!=BUBE && (c&7)<h) h=c&7;
      }
    }
    for (k=0;k<10;k++) {
      if ((c=cards[spieler*10+k])>=0 &&
	  c>>3==i && (c&7)!=BUBE && (c&7)>h) return 0;
    }
  }
  return 1;
}

VOID m_bvsp()
{
  if (ueberdoerfer()) return;
  if (!trumpfausspielen()) schenke();
}

VOID m_bmsp()
{
  if (!uebernehmen(0,0)) schenke();
}

VOID m_bhsp()
{
  if (fabwerfen()) return;
  if (!uebernehmen(!higher(stcd[0],stcd[1]),1)) schenke();
}

VOID m_bvns()
{
  if (spitzefangen()) return;
  if (spieler==left(ausspl) && karobubespielen()) return;
  if (!hochausspielen()) abwerfen(1);
}

VOID m_bmns()
{
  if (spitzefangen()) return;
  if (karobubespielen()) return;
  if (spieler==ausspl) {
    if (schnippeln(0) || (!ignorieren() && uebernehmen(0,1))) return;
  }
  else {
    if (einstechen() || hatas()) return;
  }
  if (buttern()) schmieren();
  else abwerfen(0);
}

VOID m_bhns()
{
  if (spitzefangen()) return;
  if (karobubespielen()) return;
  if (schnippeln(1)) return;
  if (higher(stcd[0],stcd[1])^(spieler!=ausspl)) {
    if (!uebernehmen(spieler!=ausspl,1)) abwerfen(0);
  }
  else {
    schmieren();
  }
}

VOID m_bsp()
{
  playcd=0;
  nichtspitze();
  if (!vmh) m_bvsp();
  else if (vmh==1) m_bmsp();
  else m_bhsp();
}

VOID m_bns()
{
  playcd=0;
  if (!vmh) m_bvns();
  else if (vmh==1) m_bmns();
  else m_bhns();
}

VOID make_best(s)
int s;
{
  if (possc==1) playcd=0;
  else if (trumpf>4) {
    m_bramsch();
  }
  else if (trumpf>=0) {
    if (s==spieler) m_bsp();
    else m_bns();
  }
  else {
    if (s==spieler) m_nsp();
    else m_nns(s);
  }
}

VOID adjfb(s,v)
int s,v;
{
  int i,c,n;
  int fb[5];

  fb[0]=fb[1]=fb[2]=fb[3]=fb[4]=0;
  n=handsp && s!=spieler?12:10;
  for (i=0;i<n;i++) {
    if ((c=i<10?cards[10*s+i]:prot2.skat[0][i-10])>=0) {
      if (trumpf!=-1 && (c&7)==BUBE) fb[trumpf]=1;
      else fb[c>>3]=1;
    }
  }
  for (i=0;i<5;i++) {
    if (!fb[i]) {
      if (hatnfb[s][i]!=1) hatnfb[s][i]=v;
    }
  }
}

VOID do_spielen()
{
  int s,i;
  static int sp,lvmh;

  if (phase!=SPIELEN) {
    sp=0;
    return;
  }
  if (trumpf==-1 && stich==1) init_null();
  while (phase==SPIELEN) {
    s=(ausspl+vmh)%3;
    if (iscomp(s)) sp=0;
    else {
      if (sp==s+1 && lvmh==vmh) return;
      sp=s+1;
    }
    lvmh=vmh;
    if (s==spieler && trumpf!=5) {
      adjfb(left(spieler),2);
      adjfb(right(spieler),2);
      for (i=0;i<5;i++) {
	if (!hatnfb[left(spieler)][i] || !hatnfb[right(spieler)][i]) {
	  if (hatnfb[left(spieler)][i]==2) hatnfb[left(spieler)][i]=0;
	  if (hatnfb[right(spieler)][i]==2) hatnfb[right(spieler)][i]=0;
	}
      }
    }
    if (ouveang) {
      adjfb(spieler,1);
    }
    calc_poss(s);
    make_best(s);
    hintcard=possi[playcd];
    if (sp && hints[s]) {
      show_hint(s,hintcard,1);
    }
    if (!ndichtw && restbeimir()) {
      di_dicht();
      return;
    }
    if (sp) return;
    drop_card(possi[playcd],s);
    do_next();
  }
}

VOID computer()
{
  if (quit) return;
  if (phase==GEBEN) do_geben();
  if (phase==REIZEN) do_reizen();
  do_spielen();
}

VOID play()
{
  if (!resumebock) {
    bockspiele=bockinc=ramschspiele=0;
  }
  phase=GEBEN;
  do {
    computer();
    computer();
    hndl_events();
  } while (!quit);
}

VOID main(argc,argv)
int argc;
char *argv[];
{
  setrnd(&seed[0],savseed=time((time_t *)0));
  xinit(argc,argv);
  play();
  exitus(0);
}
