
/*
    xskat - a card game for 1 to 3 players.
    Copyright (C) 2000  Gunter Gerhardt

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

#define RAMSCH_C

#include "defs.h"
#include "skat.h"
#include "ramsch.h"

VOID start_ramsch()
{
  vmh=0;
  spieler=geber;
  save_skat(1);
  home_skat();
  remmark(0);
  phase=SPIELEN;
}

VOID init_ramsch()
{
  int sn;

  sramschstufe=0;
  trumpf=5;
  spieler=geber;
  reizp=-1;
  stich=1;
  handsp=0;
  vmh=0;
  ouveang=0;
  sort2[0]=sort2[1]=sort2[2]=0;
  prot2.sramsch=playsramsch;
  save_skat(0);
  info_reiz();
  info_spiel();
  for (sn=0;sn<numsp;sn++) {
    initscr(sn,1);
  }
  for (sn=0;sn<3;sn++) {
    rstsum[sn]=rstich[sn]=ggdurchm[sn]=0;
  }
  if (playsramsch || (ramschspiele && klopfen)) {
    phase=DRUECKEN;
    di_schieben();
  }
  else {
    start_ramsch();
  }
}

int zweibuben()
{
  int c0,c1,gespb;

  if (stich!=9 || possc!=2 ||
      ((c0=cards[possi[0]])&7)!=BUBE ||
      ((c1=cards[possi[1]])&7)!=BUBE) return 0;
  gespb=(gespcd[0<<3|BUBE]==2)+(gespcd[1<<3|BUBE]==2)+
    (gespcd[2<<3|BUBE]==2)+(gespcd[3<<3|BUBE]==2);
  if (!vmh || (vmh==1 && (stcd[0]&7)!=BUBE)) {
    if ((c0>>3)==3 || (c1>>3)==3) {
      if ((c0>>3)==0 || (c1>>3)==0) {
	if (gespb) {
	  playcd=(c1>>3)==0;
	}
	else {
	  playcd=(c1>>3)==3;
	}
      }
      else {
	playcd=(c0>>3)==3;
      }
      return 1;
    }
    if ((c0>>3)==2 || (c1>>3)==2) {
      if ((c0>>3)==0 || (c1>>3)==0) {
	if (gespb) {
	  playcd=(c1>>3)==0;
	}
	else {
	  playcd=(c1>>3)==2;
	}
      }
      return 1;
    }
    return 1;
  }
  if (vmh==1 || (vmh==2 && ((stcd[0]&7)!=BUBE)+((stcd[1]&7)!=BUBE)==1)) {
    if ((c0>>3)==3 || (c1>>3)==3) {
      if ((c0>>3)==0 || (c1>>3)==0) {
	if (gespb>1) {
	  playcd=(c1>>3)==0;
	}
	else {
	  playcd=(c1>>3)==3;
	}
	return 1;
      }
      if ((c0>>3)==1 || (c1>>3)==1) {
	if (gespb>1) {
	  playcd=(c1>>3)==1;
	}
	else {
	  if (stcd[0]==(2<<3|BUBE) || (vmh==2 && stcd[1]==(2<<3|BUBE))) {
	    playcd=(c1>>3)==1;
	  }
	  else {
	    playcd=(c1>>3)==3;
	  }
	}
	return 1;
      }
      return 1;
    }
    if ((c0>>3)==2 || (c1>>3)==2) {
      if ((c0>>3)==0 || (c1>>3)==0) {
	if (gespb>1) {
	  playcd=(c1>>3)==0;
	}
	else {
	  if (stcd[0]==(1<<3|BUBE) || (vmh==2 && stcd[1]==(1<<3|BUBE))) {
	    playcd=(c1>>3)==0;
	  }
	  else {
	    playcd=(c1>>3)==2;
	  }
	}
      }
      return 1;
    }
    return 1;
  }
  if ((stcd[0]&7)!=BUBE && (stcd[1]&7)!=BUBE) {
    playcd=(c1>>3)==3 || (c1>>3)==2;
  }
  else {
    playcd=(c1>>3)==1 || (c1>>3)==0;
  }
  return 1;
}

int bubeanspielen()
{
  int bb,nbb,j;

  bb=-1;
  nbb=0;
  for (j=0;j<possc;j++) {
    if ((cards[possi[j]]&7)==BUBE) {
      nbb++;
      if (cards[possi[j]]>>3) {
	bb=j;
      }
    }
  }
  if (nbb>1 || bb<0) return 0;
  for (j=0;j<4;j++) {
    if (gespcd[j<<3|BUBE]==2) {
      return 0;
    }
  }
  playcd=bb;
  return 1;
}

int sicher(fb,pc,le)
int fb,*pc,*le;
{
  int i,j,mkz,akz;
  int mk[7],ak[7],p[7];

  *le=0;
  if (hatnfb[left(ausspl+vmh)][fb]==1 && hatnfb[right(ausspl+vmh)][fb]==1) {
    return 1;
  }
  mkz=akz=0;
  for (i=7;i>=0;i--) {
    if (i==BUBE) continue;
    if (gespcd[fb<<3|i]!=2) {
      for (j=0;j<possc;j++) {
	if (cards[possi[j]]==(fb<<3|i)) break;
      }
      if (j<possc) {
	mk[mkz]=i;
	p[mkz]=j;
	mkz++;
      }
      else ak[akz++]=i;
    }
  }
  for (i=0;i<mkz && i<akz;i++) {
    if (mk[i]<ak[i]) break;
  }
  if (i<mkz && i<akz) {
    *pc=p[mkz>1?1:0];
    if ((cards[possi[*pc]]&7)<=ZEHN) {
      *pc=p[0];
    }
    if (mkz==1 && (cards[possi[*pc]]&7)>ZEHN) {
      *le=1;
    }
    return 0;
  }
  return 1;
}

VOID moeglklein()
{
  int pc,fb,fp,mgb,mgp;

  for (pc=1;pc<possc;pc++) {
    fb=cards[possi[pc]]>>3;
    fp=cards[possi[playcd]]>>3;
    mgb=(vmh ||
	 hatnfb[left(ausspl)][fb]!=1 ||
	 hatnfb[right(ausspl)][fb]!=1);
    mgp=(vmh ||
	 hatnfb[left(ausspl)][fp]!=1 ||
	 hatnfb[right(ausspl)][fp]!=1);
    if ((cards[possi[playcd]]&7)==BUBE) {
      if ((cards[possi[pc]]&7)==BUBE) {
	if (cards[possi[pc]]>>3>cards[possi[playcd]]>>3) {
	  playcd=pc;
	}
      }
      else if (mgb) {
	playcd=pc;
      }
    }
    else {
      if ((cards[possi[pc]]&7)!=BUBE) {
	if ((((cards[possi[pc]]&7)>(cards[possi[playcd]]&7)) &&
	     ((cards[possi[pc]]&7)!=ACHT ||
	      (cards[possi[playcd]]&7)!=DAME ||
	      !vmh ||
	      gespcd[(cards[possi[pc]]&~7)|NEUN]==2) &&
	     mgb) ||
	    !mgp) {
	  playcd=pc;
	}
      }
      else if (!mgp) {
	playcd=pc;
      }
    }
  }
}

VOID nimm_bube()
{
  int pc;

  if (stich>=7 ||
      cardw[stcd[0]&7]+cardw[stcd[1]&7]>4 ||
      (gespcd[BUBE]!=2 && gespcd[1<<3|BUBE]!=2 &&
       gespcd[2<<3|BUBE]!=2 && gespcd[3<<3|BUBE]!=2)) return;
  for (pc=0;pc<possc;pc++) {
    if (cards[possi[pc]]==(3<<3|BUBE) ||
	cards[possi[pc]]==(2<<3|BUBE)) {
      playcd=pc;
      return;
    }
  }
}

VOID moegldrunter(sc)
int sc;
{
  int pc,f,fb,pcl,le,fr,w1,w2,wc;

  fb=cards[possi[0]]>>3;
  for (pc=1;pc<possc;pc++) {
    if (cards[possi[pc]]>>3!=fb) break;
  }
  fr=pc!=possc;
  f=0;
  wc=30;
  for (pc=0;pc<possc;pc++) {
    if (higher(sc,cards[possi[pc]])) {
      if (f) {
	if (fr) {
	  if ((cards[possi[pc]]&7)==BUBE) {
	    w1=30+(cards[possi[pc]]>>3);
	  }
	  else {
	    w1=7-(cards[possi[pc]]&7);
	    if (!sicher(cards[possi[pc]]>>3,&pcl,&le)) {
	      w1+=10;
	    }
	  }
	  if ((cards[possi[playcd]]&7)==BUBE) {
	    w2=30+(cards[possi[playcd]]>>3);
	  }
	  else {
	    w2=7-(cards[possi[playcd]]&7);
	    if (!sicher(cards[possi[playcd]]>>3,&pcl,&le)) {
	      w2+=10;
	    }
	  }
	}
	else {
	  w1=7-(cards[possi[pc]]&7);
	  w2=7-(cards[possi[playcd]]&7);
	}
	if (w1>w2) {
	  playcd=pc;
	  wc=w1;
	}
      }
      else {
	playcd=pc;
	f=1;
      }
    }
  }
  if (!f) {
    moeglklein();
  }
  else if (fr && vmh==2 && wc<10) nimm_bube();
}

int ggdurchmarsch()
{
  int i,j,h,bb,sn;

  if (rstich[0]+rstich[1]+rstich[2]>1 ||
      (stcd[0]&7)==SIEBEN ||
      (vmh==2 && stich!=1 && !higher(stcd[0],stcd[1]))) return 0;
  sn=(ausspl+vmh)%3;
  for (i=3;i>=0;i--) {
    if (gespcd[bb=i<<3|BUBE]!=2) {
      for (i=0;i<10;i++) {
	if (cards[sn*10+i]==bb) return 0;
      }
      break;
    }
  }
  if (((stcd[0]&7)==BUBE &&
       ((hatnfb[left(ausspl)][4]==1 &&
	 hatnfb[right(ausspl)][4]==1) ||
	(gespcd[0<<3|BUBE]==2 && gespcd[1<<3|BUBE]==2 &&
	 gespcd[2<<3|BUBE]==2 && gespcd[3<<3|BUBE]==2))) ||
      (stcd[0]&7)<KOENIG) {
    ggdurchm[sn]=1;
  }
  if (!ggdurchm[sn]) return 0;
  j=h=0;
  if (vmh==2 && !higher(stcd[0],stcd[1])) {
    h=1;
  }
  for (i=0;i<possc;i++) {
    if (!higher(stcd[h],cards[possi[i]])) {
      if (!j || cardw[cards[possi[i]]&7]<cardw[cards[possi[j-1]]&7]) {
	j=i+1;
      }
    }
  }
  if (!j) {
    for (i=0;i<possc;i++) {
      if (!j || ggdmw[cards[possi[i]]&7]<ggdmw[cards[possi[j-1]]&7]) {
	j=i+1;
      }
    }
  }
  playcd=j-1;
  return 1;
}

VOID m_bvr()
{
  int fb,pc,le,lef,f;

  if (zweibuben()) return;
  if (bubeanspielen()) return;
  f=lef=0;
  for (fb=0;fb<4;fb++) {
    if (!sicher(fb,&pc,&le)) {
      if (f) {
	if (le>lef ||
	    (rswert[cards[possi[pc]]&7]>rswert[cards[possi[playcd]]&7] &&
	     le>=lef)) {
	  playcd=pc;
	  lef=le;
	}
      }
      else {
	playcd=pc;
	lef=le;
	f=1;
      }
    }
  }
  if (!f || (cards[possi[playcd]]&7)<=ZEHN) {
    playcd=0;
    moeglklein();
  }
}

VOID m_bmr()
{
  if (ggdurchmarsch()) return;
  if (zweibuben()) return;
  moegldrunter(stcd[0]);
}

VOID m_bhr()
{
  if (ggdurchmarsch()) return;
  if (zweibuben()) return;
  moegldrunter(higher(stcd[0],stcd[1])?stcd[0]:stcd[1]);
}

VOID m_bramsch()
{
  playcd=0;
  if (!vmh) m_bvr();
  else if (vmh==1) m_bmr();
  else m_bhr();
}

int unsich_fb(sn,s)
int sn,*s;
{
  int fb,pc,le,n;

  for (possc=0;possc<10;possc++) {
    possi[possc]=sn*10+possc;
  }
  n=0;
  for (fb=0;fb<4;fb++) {
    s[fb]=1;
    if (!sicher(fb,&pc,&le)) {
      s[fb]=0;
      n++;
    }
  }
  return n;
}

int comp_sramsch(sn)
int sn;
{
  int fb,n,i,j,c,ea,bb,dum;
  int p[4],t[4],s[4],o[4],b[4];

  dum=0;
  n=unsich_fb(sn,s);
  bb=b[0]=b[1]=b[2]=b[3]=0;
  for (i=0;i<10;i++) {
    c=cards[sn*10+i];
    if ((c&7)==BUBE) bb++,b[c>>3]=1;
  }
  if (ramschspiele && klopfen && !playsramsch) {
    if ((n<=3 && (!n || bb<=1)) ||
	(n<=2 && (!n || bb<=2))) {
      return di_verdoppelt(0,1);
    }
  }
  if (playsramsch) {
    if (sn==left(ausspl)) {
      if ((n<=3 && !bb) ||
	  (n==1 && bb<=1 && !b[3]) ||
	  !n) {
	return di_verdoppelt(0,0);
      }
    }
    else if ((n==3 && !bb) ||
	     (n==2 && bb<=1 && !b[3]) ||
	     (n==1 && bb<=2) ||
	     !n) {
      return di_verdoppelt(0,0);
    }
  }
  if (!playsramsch) return 0;
  for (fb=0;fb<4;fb++) {
    for (c=0;c<8;c++) inhand[fb][c]=0;
    p[fb]=t[fb]=0;
    o[fb]=fb;
  }
  if (((vmh && prot2.verdopp[right(ausspl+vmh)]!=1) ||
       (vmh==2 && prot2.verdopp[right(ausspl+vmh)]==1 &&
	prot2.verdopp[left(ausspl+vmh)]!=1)) &&
      (((cards[30]&7)>ZEHN && (cards[31]&7)>ZEHN) ||
       (cards[30]&7)==SIEBEN || (cards[31]&7)==SIEBEN) &&
      ((cards[30]&7)>=NEUN || (cards[31]&7)>=NEUN)) {
    ggdurchm[sn]=1;
  }
  for (i=0;i<2;i++) {
    for (j=0;j<10;j++) {
      if (cardw[cards[10*sn+j]&7]>cardw[cards[30+i]&7]) {
	swap(&cards[30+i],&cards[10*sn+j]);
      }
    }
    if ((cards[30+i]&7)==BUBE) {
      for (j=0;j<10;j++) {
	if ((cards[10*sn+j]&7)!=BUBE) {
	  swap(&cards[30+i],&cards[10*sn+j]);
	  break;
	}
      }
    }
  }
  for (i=0;i<10;i++) spcards[i]=cards[sn*10+i];
  spcards[10]=cards[30];
  spcards[11]=cards[31];
  bb=b[0]=b[1]=b[2]=b[3]=0;
  for (i=0;i<12;i++) {
    c=spcards[i];
    if ((c&7)!=BUBE) {
      p[c>>3]+=cardw[c&7];
      t[c>>3]++;
      inhand[c>>3][c&7]=1;
    }
    else bb++,b[c>>3]=1;
  }
  for (fb=0;fb<4;fb++) {
    for (i=fb+1;i<4;i++) {
      if (p[o[fb]]<p[o[i]]) {
	j=o[i];
	o[i]=o[fb];
	o[fb]=j;
      }
    }
  }
  gedr=0;
  ea=0;
  for (i=0;i<4;i++) {
    if (t[i]==1 && inhand[i][AS]) ea++;
  }
  if (ea<2) {
    for (i=0;i<4;i++) {
      if (t[i]==2 && inhand[i][AS] && inhand[i][ZEHN]) {
	drueck(i,2,&dum);
	break;
      }
    }
  }
  for (n=1;n<8 && gedr<2;n++) {
    for (j=0;j<4 && gedr<2;j++) {
      i=o[j];
      if (t[i]==n && !s[i]) {
	if (n==1) {
	  if (!inhand[i][ACHT]) {
	    drueck(i,1,&dum);
	  }
	}
	else if (n==2) {
	  if (inhand[i][SIEBEN] || inhand[i][ACHT]) drueck(i,1,&dum);
	  else drueck(i,2,&dum);
	}
	else if (n==3) {
	  switch (inhand[i][SIEBEN]+inhand[i][ACHT]+inhand[i][NEUN]) {
	  case 3:break;
	  case 2:drueck(i,1,&dum);break;
	  default:drueck(i,2,&dum);break;
	  }
	}
	else {
	  drueck(i,2,&dum);
	}
      }
    }
  }
  if (ramschspiele && klopfen && !ggdurchm[sn]) {
    n=unsich_fb(sn,s);
    if ((n<=3 && !bb) ||
	(n<=2 && (bb<=1 || (bb==2 && !b[3]))) ||
	(n<=1 && (bb<=2 || (bb==3 && b[0]))) ||
	!n) {
      return di_verdoppelt(0,1);
    }
  }
  return 0;
}

VOID ramsch_stich()
{
  rstsum[ausspl]+=cardw[stcd[0]&7]+cardw[stcd[1]&7]+cardw[stcd[2]&7];
  rstich[ausspl]=1;
  if (stich==10) {
    rskatsum=cardw[prot2.skat[1][0]&7]+cardw[prot2.skat[1][1]&7];
    if (!rskatloser) {
      rstsum[ausspl]+=rskatsum;
    }
  }
  if ((stcd[0]&7)==BUBE && (stcd[1]&7)!=BUBE && (stcd[2]&7)!=BUBE) {
    ggdurchm[0]=ggdurchm[1]=ggdurchm[2]=1;
  }
}

VOID ramsch_result()
{
  int maxn,i;

  stsum=rstsum[0];
  spieler=0;
  maxn=1;
  if (rstsum[1]>stsum) {
    stsum=rstsum[1];
    spieler=1;
  }
  else if (rstsum[1]==stsum) {
    spieler=2;
    maxn++;
  }
  if (rstsum[2]>stsum) {
    stsum=rstsum[2];
    spieler=2;
    maxn=1;
  }
  else if (rstsum[2]==stsum) {
    spieler=1-spieler;
    maxn++;
  }
  spgew=0;
  if (maxn==3) {
    spieler=spwert=stsum=0;
  }
  else {
    spwert=stsum;
    if (maxn==2) {
      stsum=120-2*stsum;
      spgew=1;
      if (rskatloser) {
	spwert+=rskatsum;
      }
    }
    else if (rskatloser) {
      stsum+=rskatsum;
      spwert+=rskatsum;
    }
  }
  nspwert=0;
  switch (rstich[0]+rstich[1]+rstich[2]) {
  case 1:
    nspwert=spwert=stsum=120;
    spgew=1;
    mes2=1;
    break;
  case 2:
    mes1=1;
    spwert*=2;
    break;
  }
  for (i=0;i<sramschstufe;i++) spwert*=2;
  if (bockspiele && !ramschspiele) spwert*=2;
}

int testgrandhand(sn)
int sn;
{
  int i,bb,as,zehn,b[4];

  bb=as=zehn=0;
  for (i=0;i<10;i++) {
    switch (cards[10*sn+i]&7) {
    case BUBE:bb++;break;
    case AS:as++;break;
    case ZEHN:zehn++;break;
    }
  }
  calc_inhand(sn);
  for (i=0;i<4;i++) b[i]=inhand[i][BUBE];
  return ((bb>=3 && as>=2 && as+zehn>=3) ||
	  (bb==4 && as>=2) ||
	  testgrand(bb,b,sn==hoerer));
}
