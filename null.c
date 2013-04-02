
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

#define NULL_C

#include "defs.h"
#include "skat.h"
#include "null.h"

VOID init_null()
{
  int i;

  for (i=0;i<4;i++) {
    wirftabfb[i]=0;
    hattefb[i]=0;
    aussplfb[i]=0;
    nochinfb[i]=8;
  }
}

VOID testnull(sn)
int sn;
{
  int i,f,c;
  int a[4],l[4],n[4],m[4],h[4],s[4],mfb[4];

  naussplfb[sn]=-1;
  if (null_dicht(sn,1,&cards[30],(int *)0,mfb,(int *)0)) {
    for (i=0;i<4 && mfb[i];i++);
    if (sn!=ausspl || i<4) {
      if (sn==ausspl) naussplfb[sn]=i;
      maxrw[sn]=nullw[revolution?4:3];
      return;
    }
  }
  for (i=0;i<4;i++) a[i]=l[i]=n[i]=m[i]=h[i]=s[i]=0;
  f=1;
  for (i=0;i<10;i++) {
    c=cards[10*sn+i];
    a[c>>3]++;
    if ((c&7)>BUBE) l[c>>3]++;
    else if ((c&7)<BUBE && (c&7)!=ZEHN) h[c>>3]=1;
    else m[c>>3]=1;
    if ((c&7)==NEUN) n[c>>3]=1;
    if ((c&7)==SIEBEN) s[c>>3]=1;
  }
  for (i=0;i<4;i++) {
    if ((a[i] && l[i]!=a[i] && l[i]<2) ||
	(l[i]==1 && n[i]) ||
	(l[i]!=3 && !m[i] && h[i]) ||
	(a[i]>2 && !s[i])) f=0;
  }
  if (f) maxrw[sn]=nullw[1];
}

int kleiner_w(w1,w2)
int w1,w2;
{
  if (w1==ZEHN) return w2<=BUBE;
  if (w2==ZEHN) return w1>BUBE;
  return w1>w2;
}

int kleiner(i,j)
int i,j;
{
  return kleiner_w(cards[possi[i]]&7,cards[possi[j]]&7);
}

int hat(i)
int i;
{
  return !hatnfb[spieler][cards[possi[i]]>>3];
}

int n_anwert(c)
int c;
{
  int fb,i,m;

  fb=c>>3;
  if (hatnfb[spieler][fb]) return 0;
  for (i=AS;i<=SIEBEN;i=i==AS?KOENIG:i==BUBE?ZEHN:i==ZEHN?NEUN:i+1) {
    if (c==(fb<<3|i)) return 1;
    if (gespcd[fb<<3|i]!=2) break;
  }
  if ((c&7)==SIEBEN) {
    m=left(ausspl)!=spieler?left(ausspl):right(ausspl);
    if (hatnfb[m][fb]!=1 && nochinfb[fb]>4) return 2;
  }
  if (wirftabfb[fb]) return 5;
  if (aussplfb[fb]) return 3;
  if (hattefb[fb]) return 6;
  return 4;
}

int n_anspiel()
{
  int i,j,ci,cj,wi,wj;

  j=0;
  for (i=1;i<possc;i++) {
    ci=cards[possi[i]];
    cj=cards[possi[j]];
    wi=n_anwert(ci);
    wj=n_anwert(cj);
    if (wi>wj || (wi==wj && kleiner(i,j))) j=i;
  }
  return j;
}

int n_abwert(c)
int c;
{
  int fb,i,n;

  fb=c>>3;
  if ((c&7)>=ACHT) return 0;
  for (i=0;i<possc;i++) {
    if (cards[possi[i]]==(fb<<3|SIEBEN)) return 1;
  }
  if (hatnfb[spieler][fb]) return 2;
  if (aussplfb[fb]) return 3;
  if (wirftabfb[fb]) return 7;
  if (hattefb[fb]) return 6;
  n=0;
  for (i=0;i<possc;i++) {
    if (cards[possi[i]]>>3==fb) n++;
  }
  if (n<3) return 5;
  return 4;
}

int n_abwerfen()
{
  int i,j,ci,cj,wi,wj;

  j=0;
  for (i=1;i<possc;i++) {
    ci=cards[possi[i]];
    cj=cards[possi[j]];
    wi=n_abwert(ci);
    wj=n_abwert(cj);
    if (wi>wj || (wi==wj && !kleiner(i,j))) j=i;
  }
  return j;
}

int minmax(f)
int f;
{
  int i,j,hi,hj;

  j=0;
  for (i=1;i<possc;i++) {
    hi=hat(i);
    hj=hat(j);
    if ((hi && !hj) || (kleiner(i,j)^f && (hi || !hj))) j=i;
  }
  return j;
}

int minmaxfb(f,fb)
{
  int i,j=-1;

  for (i=0;i<possc;i++) {
    if ((j<0 && cards[possi[i]]>>3==fb) ||
	(cards[possi[i]]>>3==fb && kleiner(i,j)^f)) j=i;
  }
  return j<0?0:j;
}

int drunter(f)
int f;
{
  int i,j;

  j=0;
  for (i=1;i<possc;i++) {
    if (higher(cards[possi[j]],cards[possi[i]])) j=i;
  }
  for (i=0;i<possc;i++) {
    if (higher(stcd[f],cards[possi[i]]) &&
      higher(cards[possi[i]],cards[possi[j]])) j=i;
  }
  return j;
}

int drunterdrue()
{
  int i,w,fb;

  fb=stcd[0]>>3;
  i=-1;
  for (w=stcd[0]&7;w>=AS;w=w==NEUN?ZEHN:w==ZEHN?BUBE:w==KOENIG?AS:w-1) {
    if (i<0) {
      i=0;
      continue;
    }
    for (i=0;i<possc;i++) {
      if (cards[possi[i]]==(fb<<3|w)) return i;
    }
    if (gespcd[fb<<3|w]!=2) break;
  }
  return drunter(0);
}

VOID m_nsp()
{
  int ufb;

  if (!vmh) {
    if (naussplfb[spieler]==-1) {
      playcd=minmax(0);
    }
    else {
      playcd=minmaxfb(0,naussplfb[spieler]);
    }
  }
  else if (hatnfb[spieler][stcd[0]>>3]) {
    if (null_dicht(spieler,handsp,&prot2.skat[1][0],&ufb,(int *)0,(int *)0)) {
      playcd=minmax(1);
    }
    else {
      playcd=minmaxfb(1,ufb);
    }
  }
  else playcd=drunter(vmh==2?!higher(stcd[0],stcd[1]):0);
}

VOID m_nns(s)
int s;
{
  int sga;

  if (revolang && spieler!=ausspl) {
    playcd=minmax(0);
    return;
  }
  sga=spieler==ausspl;
  if (!vmh) playcd=n_anspiel();
  else if (hatnfb[s][stcd[0]>>3]) playcd=n_abwerfen();
  else if (vmh==1) {
    if (sga) playcd=drunter(0);
    else playcd=drunterdrue();
  }
  else if (higher(stcd[0],stcd[1])^sga) {
    playcd=minmax(1);
  }
  else {
    playcd=minmax(0);
    if (!higher(stcd[!sga],cards[possi[playcd]])) {
      playcd=minmax(1);
    }
  }
}

VOID null_stich()
{
  int i,fb1,fb2;

  for (i=0;i<3;i++) {
    nochinfb[stcd[i]>>3]--;
  }
  fb1=stcd[0]>>3;
  if (ausspl!=spieler) {
    fb2=stcd[(spieler-ausspl+3)%3]>>3;
    if (fb1!=fb2) {
      wirftabfb[fb2]=1;
    }
    else {
      hattefb[fb2]=1;
    }
  }
  else {
    aussplfb[fb1]=1;
    hattefb[fb1]=1;
  }
}

VOID null_sort(arr,cnt)
int *arr,cnt;
{
  int i,swp;

  do {
    swp=0;
    for (i=0;i<cnt-1;i++) {
      if (kleiner_w(arr[i+1],arr[i])) {
	swap(&arr[i],&arr[i+1]);
	swp=1;
      }
    }
  } while (swp);
}

int null_dicht(sn,hnd,cd,ufb,mfb,sfb)
int sn,hnd,*cd,*ufb,*mfb,*sfb;
{
  int i,c,fb,sp[8],spc,ns[8],nsc,cnt,sfbc[4],el,*mpt;

  for (fb=0;fb<4;fb++) {
    mpt=mfb?&mfb[fb]:&el;
    spc=nsc=0;
    for (i=0;i<(hnd?32:30);i++) {
      if (i>=30) c=cd[i-30];
      else c=cards[i];
      if (c!=-1 && c>>3==fb) {
	if (sn*10<=i && i<=sn*10+9) sp[spc++]=c&7;
	else ns[nsc++]=c&7;
      }
    }
    if (sfb) {
      el=fb;
      sfbc[el]=spc;
      for (i=0;i<fb;i++) {
	if (sfbc[el]<sfbc[sfb[i]]) {
	  swap(&el,&sfb[i]);
	}
      }
      sfb[fb]=el;
    }
    if (spc) {
      if (nsc) {
	null_sort(sp,spc);
	null_sort(ns,nsc);
	cnt=nsc<spc?nsc:spc;
	for (i=0;i<cnt;i++) {
	  if (kleiner_w(ns[i],sp[i])) {
	    if (ufb) *ufb=fb;
	    return 0;
	  }
	}
	if (nsc<3 && hnd) *mpt=1;
	else if (spc>1 && nsc>1) {
	  *mpt=0;
	  for (i=1;i<cnt;i++) {
	    if (kleiner_w(ns[i-1],sp[i])) {
	      *mpt=1;
	      break;
	    }
	  }
	}
	else *mpt=0;
      }
      else *mpt=1;
    }
    else *mpt=2;
  }
  return 1;
}

VOID revolutiondist()
{
  int i,j,k,p,c,sn,mi,fb,ufb,cnt;
  int cd[4][8],cdc[4],mfb[4],ct[3],sfb[4];

  sn=spieler==ausspl?left(spieler):ausspl;
  mi=left(sn)==spieler?left(spieler):left(sn);
  if (null_dicht(spieler,0,(int *)0,&ufb,mfb,sfb)) {
    for (fb=0;fb<4 && mfb[fb]!=1;fb++);
    if (spieler!=ausspl || fb==4) return;
    ct[0]=ct[1]=ct[2]=0;
    for (fb=0;fb<4;fb++) {
      if (mfb[sfb[fb]]==1) {
	p=ct[sn]<ct[mi]?sn:mi;
	for (j=0,k=sn;j<2 && ct[p]!=10;j++,k=mi) {
	  for (i=0;i<10 && ct[p]!=10;i++) {
	    c=cards[10*k+i];
	    if (c>>3==sfb[fb]) {
	      swap(&cards[10*k+i],&cards[10*p+ct[p]]);
	      ct[p]++;
	    }
	  }
	}
      }
    }
    return;
  }
  cdc[0]=cdc[1]=cdc[2]=cdc[3]=0;
  cnt=0;
  for (j=0,k=sn;j<2;j++,k=mi) {
    for (i=0;i<10;i++) {
      c=cards[10*k+i];
      if (c>>3==ufb) {
	swap(&cards[10*k+i],&cards[10*sn+cnt]);
	cnt++;
      }
    }
  }
  for (j=0,k=sn;j<2;j++,k=mi) {
    for (i=0;i<10;i++) {
      c=cards[10*k+i];
      cd[c>>3][cdc[c>>3]++]=c&7;
    }
  }
  for (fb=0;fb<4;fb++) {
    null_sort(cd[fb],cdc[fb]);
  }
  fb=0;
  while (cnt<10) {
    while (fb==ufb || !cdc[fb]) fb=(fb+1)%4;
    for (j=0,k=sn;j<2;j++,k=mi) {
      for (i=0;i<10;i++) {
	c=cards[10*k+i];
	if (c==(fb<<3|cd[fb][cdc[fb]-1])) {
	  swap(&cards[10*k+i],&cards[10*sn+cnt]);
	  cnt++;
	  cdc[fb]--;
	  fb=(fb+1)%4;
	  i=10;
	  j=2;
	}
      }
    }
  }
}
