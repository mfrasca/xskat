
/*
    xskat - a card game for 1 to 3 players.
    Copyright (C) 1998  Gunter Gerhardt

    This program is free software; you can redistribute it freely.
    Use it at your own risk; there is NO WARRANTY.
*/

#define XDIAL_C

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include "defs.h"
#include "skat.h"
#include "ramsch.h"
#include "xio.h"
#include "irc.h"
#include "text.h"
#include "xdial.h"

#define INIT_DI(d)\
{\
  memcpy((VOID *)d[1],(VOID *)d[0],sizeof(d[0]));\
  memcpy((VOID *)d[2],(VOID *)d[0],sizeof(d[0]));\
  init_di(d[0]);\
  init_di(d[1]);\
  init_di(d[2]);\
}

VOID info_reiz()
{
  int sn;
  static char txt[3][40];

  for (sn=0;sn<numsp;sn++) {
    if (reizp<0) {
      dioptions[sn][11].str=OB_NONE;
    }
    else {
      dioptions[sn][11].str=txt[sn];
      sprintf(txt[sn],"%s %d",textarr[TX_GEREIZT_BIS_L],reizw[reizp]);
    }
  }
}

VOID info_spiel()
{
  int sn;
  static char txt[3][40];

  for (sn=0;sn<numsp;sn++) {
    dioptions[sn][12].str=txt[sn];
    sprintf(txt[sn],"%s %s",textarr[TX_GESPIELT_WIRD],
	    textarr[revolang?TX_REVOLUTION:TX_NULL+trumpf+1]);
  }
}

VOID info_stich(p,c,f)
int p,c,f;
{
  int sn;
  static char txt[3][3][20];

  for (sn=0;sn<numsp;sn++) {
    if (!f || sn==spieler) {
      dioptions[sn][14+p].str=txt[sn][p];
      txt[sn][p][0]=0;
      strcat(txt[sn][p],textarr[TX_KARO+(c>>3)]);
      strcat(txt[sn][p]," ");
      strcat(txt[sn][p],textarr[TX_AS+(c&7)]);
      dioptions[sn][13].str=textarr[f?TX_GEDRUECKT:TX_LETZTER_STICH];
    }
  }
}

VOID clear_info()
{
  int sn;

  for (sn=0;sn<numsp;sn++) {
    dioptions[sn][11].str=textarr[TX_GEREIZT_BIS_L];
    dioptions[sn][12].str=textarr[TX_GESPIELT_WIRD];
    dioptions[sn][13].str=textarr[TX_LETZTER_STICH];
    dioptions[sn][14].str=OB_NONE;
    dioptions[sn][15].str=OB_NONE;
    dioptions[sn][16].str=OB_NONE;
  }
}

VOID set_names(ob,idx)
OBJECT *ob;
int idx;
{
  int z,s;

  for (z=0;z<2;z++) {
    for (s=0;s<3;s++) {
      ob[idx+z*3+s].str=spnames[s][z];
    }
  }
}

int ismemb(w,ob)
Window w;
OBJECT *ob;
{
  int i;

  for (i=0;i<ob[0].spec;i++) {
    if (w==ob[i].win) return i?i:-1;
  }
  return 0;
}

VOID init_di(ob)
OBJECT *ob;
{
  int i;

  for (i=1;i<ob[0].spec;i++) {
    if (ob[i].str!=OB_NONE) {
      ob[i].str=textarr[(int)ob[i].str-OB_NONE-1];
    }
  }
}

VOID init_dials()
{
  INIT_DI(diende);
  INIT_DI(diterm);
  INIT_DI(dismlres);
  INIT_DI(diproto);
  INIT_DI(diliste);
  INIT_DI(dioptions);
  INIT_DI(dicopyr);
  INIT_DI(distrateg);
  INIT_DI(divarianten);
  INIT_DI(dibockevents);
  INIT_DI(diverzoegerung);
  INIT_DI(dimenubutton);
  INIT_DI(diweiter);
  init_di(dihand);
  init_di(digrandhand);
  init_di(dischieben);
  init_di(diverdoppelt);
  init_di(dibuben);
  init_di(diklopfen);
  init_di(dischenken);
  init_di(digeschenkt);
  init_di(diendeschenken);
  init_di(diwiederschenken);
  init_di(dinichtschenken);
  init_di(diloesch);
  init_di(dispiel);
  init_di(difehler);
  init_di(diueberr);
  init_di(dispitze);
  init_di(diansage);
  init_di(dikontra);
  init_di(direkontra);
  init_di(dikonre);
  init_di(didicht);
  init_di(diresult);
  init_di(diwieder);
}

VOID prverz(sn)
int sn;
{
  int n;
  char *gr="> ";
  char *em="";

  n=nimmstich[sn][0];
  if (n>=101) sprintf(diverzoegerung[sn][3].str,"%s",textarr[TX_MAUS_KLICK]);
  else {
    sprintf(diverzoegerung[sn][3].str,"%s%d.%d %s",
	    n<maxnimm()?gr:em,n/10,n%10,textarr[TX_SEKUNDEN]);
  }
}

int ob_disabled(ob,i)
OBJECT *ob;
int i;
{
  return ob[i].spec&OB_DISABLED ||
    (ob[0].next&OB_DISABLED && !(ob[i].spec&OB_EXIT));
}

VOID hndl_btevent(sn,bt)
int sn,bt;
{
  int i,j,c,ag,s1,s2,al,stg[2],sav,bb;
  OBJECT *ob;

  ob=actdial[sn];
  if (ob_disabled(ob,bt)) return;
  if ((i=bt+ob[bt].next)!=bt) {
    while (i!=bt) {
      if (ob[i].spec&OB_SELECTED) {
	ob[i].spec&=~OB_SELECTED;
	draw_di(sn,i);
      }
      i+=ob[i].next;
    }
    if (!(ob[bt].spec&OB_SELECTED)) {
      ob[bt].spec|=OB_SELECTED;
      draw_di(sn,bt);
    }
  }
  else {
    ob[bt].spec^=OB_SELECTED;
    draw_di(sn,bt);
  }
  if (ob[bt].spec&OB_EXIT) {
    waitt(100,1);
    if (ob==dihand) {
      remove_di(sn);
      if (bt==2) handsp=1;
      do_handok();
    }
    else if (ob==digrandhand) {
      remove_di(sn);
      if (bt==3) do_grandhand(sn);
      else next_grandhand(sn);
    }
    else if (ob==diende[sn]) {
      remove_di(sn);
      if (bt==2) {
	lost[sn]=1;
	XUnmapWindow(dpy[sn],win[sn]);
	XFlush(dpy[sn]);
	if (irc_play) exitus(0);
	finish(sn,0);
      }
      else if (phase==RESULT) phase=GEBEN;
    }
    else if (ob==diterm[sn]) {
      remove_di(sn);
      lost[sn]=1;
      XUnmapWindow(dpy[sn],win[sn]);
      XFlush(dpy[sn]);
      if (irc_play) exitus(0);
    }
    else if (ob==diloesch) {
      remove_di(sn);
      if (bt==2) {
	setsum(1);
	if (irc_play) irc_sendloeschen(sn);
	save_list();
      }
      di_liste(sn,1);
    }
    else if (ob==dispiel) {
      remove_di(sn);
      ag=0;
      if (dispiel[6].spec&OB_SELECTED ||
	  dispiel[13].spec&OB_SELECTED) trumpf=-1;
      else if (dispiel[2].spec&OB_SELECTED) trumpf=0;
      else if (dispiel[3].spec&OB_SELECTED) trumpf=1;
      else if (dispiel[4].spec&OB_SELECTED) trumpf=2;
      else if (dispiel[5].spec&OB_SELECTED) trumpf=3;
      else if (dispiel[7].spec&OB_SELECTED) trumpf=4;
      else {
	create_di(sn,dispiel);
	ag=1;
      }
      if (!ag && !handsp && trumpf!=-1 &&
	  (dispiel[8].spec&OB_SELECTED
	   || dispiel[9].spec&OB_SELECTED
	   || dispiel[10].spec&OB_SELECTED)) {
	create_di(sn,difehler);
	ag=1;
      }
      if (!ag && trumpf==-1 &&
	  reizw[reizp]>nullw[dispiel[13].spec&OB_SELECTED?4:
			     (dispiel[10].spec&OB_SELECTED?2:0)+handsp]) {
	diueberr[6].spec=revolution?OB_CENTERED:OB_HIDDEN;
	create_di(sn,diueberr);
	ag=1;
      }
      spitzeang=0;
      if (!ag && trumpf!=-1 && dispiel[11].spec&OB_SELECTED) {
	bb=0;
	for (i=0;i<(handsp?10:12);i++) {
	  c=i>=10?prot2.skat[1][i-10]:cards[spieler*10+i];
	  if (i<10 && c==(trumpf==4?BUBE:SIEBEN|trumpf<<3)) {
	    spitzeang=1;
	  }
	  if ((c&7)==BUBE) bb++;
	}
	if (!spitzeang || (bb==4 && trumpf==4)) {
	  dispitze[3].str=textarr[spitzeang?TX_SPITZE_F3:TX_SPITZE_F2];
	  create_di(sn,dispitze);
	  ag=1;
	}
      }
      if (!ag) {
	if (dispiel[13].spec&OB_SELECTED) revolang=ouveang=1;
	else if (dispiel[10].spec&OB_SELECTED) ouveang=schwang=schnang=1;
	else if (dispiel[9].spec&OB_SELECTED) schwang=schnang=1;
	else if (dispiel[8].spec&OB_SELECTED) schnang=1;
	if (trumpf==-1) schwang=schnang=0;
	di_ansage();
      }
    }
    else if (ob==difehler) {
      remove_di(sn);
      di_spiel();
    }
    else if (ob==diueberr) {
      remove_di(sn);
      di_spiel();
    }
    else if (ob==dispitze) {
      remove_di(sn);
      di_spiel();
    }
    else if (ob==diansage) {
      remove_di(sn);
      do_angesagt();
    }
    else if (ob==dikontra) {
      remove_di(sn);
      di_ktrnext(sn,bt==3);
    }
    else if (ob==direkontra) {
      remove_di(sn);
      di_ktrnext(sn,bt==3);
    }
    else if (ob==dikonre) {
      remove_di(sn);
      if (ktrnext>=0) {
	di_konre(ktrnext);
	ktrnext=-1;
      }
      else {
	do_angesagt();
      }
    }
    else if (ob==didicht) {
      remove_di(sn);
      if (didicht[3].spec&OB_SELECTED) {
	spielendscr();
      }
      else {
	ndichtw=1;
	phase=SPIELEN;
      }
    }
    else if (ob==diweiter[sn]) {
      remove_di(sn);
      di_weiter(0);
    }
    else if (ob==dischieben) {
      remove_di(sn);
      if (bt==2) {
	draw_skat();
	put_fbox(spieler,TX_FERTIG);
      }
      else {
	di_verdoppelt(0,0);
      }
    }
    else if (ob==diklopfen) {
      remove_di(sn);
      if (bt==3) {
	vmh=left(vmh);
	if (vmh) di_schieben();
	else start_ramsch();
      }
      else {
	di_verdoppelt(0,1);
      }
    }
    else if (ob==dischenken) {
      remove_di(sn);
      if (bt==3) {
	di_nichtschenken(sn);
      }
      else {
	di_geschenkt();
      }
    }
    else if (ob==digeschenkt || ob==diendeschenken) {
      remove_di(sn);
      if (bt==4) {
	schenkstufe++;
	if (schnang && schenkstufe==2) schenkstufe++;
	if (schenkstufe<3) schnang=1;
	else schwang=1;
	diwiederschenken[2].str=
	  textarr[schwang?TX_SCHW_ANGE:TX_SCHN_ANGE];
	di_wiederschenken(schenknext,1);
      }
      else {
	finishgame();
      }
    }
    else if (ob==diwiederschenken) {
      remove_di(sn);
      if (bt==4) {
	di_nichtschenken(sn);
      }
      else {
	if (schenknext!=-1) {
	  di_wiederschenken(schenknext,0);
	}
	else {
	  schenknext=sn;
	  sn=left(sn)==spieler?left(spieler):left(sn);
	  if (!iscomp(sn)) schenknext=sn;
	  di_geschenkt();
	}
      }
    }
    else if (ob==dinichtschenken) {
      remove_di(sn);
      if (schenkstufe==1) schenkstufe=0;
      phase=SPIELEN;
    }
    else if (ob==diverdoppelt) {
      remove_di(sn);
      di_verdoppelt(1,0);
    }
    else if (ob==dibuben) {
      remove_di(sn);
    }
    else if (ob==diresult) {
      remove_di(sn);
      if (handsp && sn==spieler) {
	putdesk(sn,desk[sn].playx+8*desk[sn].cardx,desk[sn].playy);
	putdesk(sn,desk[sn].playx+9*desk[sn].cardx,desk[sn].playy);
      }
      if (bt==16) di_ende(sn);
      else if (bt==18) di_proto(sn,1,0);
      else if (bt==17) di_wieder(sn);
      else phase=GEBEN;
    }
    else if (ob==diwieder) {
      remove_di(sn);
      vorhandwn=diwieder[10].spec&OB_SELECTED?1:0;
      wieder=bt-4;
      phase=GEBEN;
    }
    else if (ob==diproto[sn]) {
      remove_di(sn);
      if (bt==49) {
	protsort[sn]^=1;
	di_proto(sn,0,0);
      }
      else if (bt==47) {
	if ((sn && !irc_play) || !protsort[sn]) di_liste(sn,1);
	else {
	  di_proto(sn,1,1);
	  if (phase==RESULT) phase=GEBEN;
	}
      }
      else if (phase==RESULT) phase=GEBEN;
    }
    else if (ob==diliste[sn]) {
      remove_di(sn);
      if (bt==59) {
	if (splfirst[sn]>=12) {
	  splfirst[sn]-=12;
	  di_liste(sn,0);
	}
	else {
	  alist[sn]=(alist[sn]+1)%3;
	  if (!sn) save_opt();
	  di_liste(sn,0);
	}
      }
      else if (bt==61) {
	if (splfirst[sn]+12>=splstp) {
	  di_loesch(sn);
	}
	else {
	  if (splfirst[sn]+12<splstp) {
	    splfirst[sn]+=12;
	    di_liste(sn,0);
	  }
	  else {
	    di_liste(sn,1);
	  }
	}
      }
      else if (phase==RESULT) phase=GEBEN;
    }
    else if (ob==dioptions[sn]) {
      remove_di(sn);
      s1=dioptions[sn][4].spec&OB_SELECTED?1:0;
      al=dioptions[sn][6].spec&OB_SELECTED?1:0;
      s2=dioptions[sn][8].spec&OB_SELECTED?1:0;
      sav=sort1[0]+2*alternate[0];
      if (s1!=sort1[sn] || s2!=sort2[sn] || al!=alternate[sn]) {
	sort1[sn]=s1;
	sort2[sn]=s2;
	alternate[sn]=al;
	if (irc_play) irc_sendsort(sn);
	initscr(sn,1);
	if (sav!=sort1[0]+2*alternate[0] && !irc_play) save_opt();
      }
      if (bt==20) di_ende(sn);
      else if (bt==18 && splres) di_proto(sn,1,0);
      else if (bt==18) di_liste(sn,1);
      else if (bt==19) di_strateg(sn);
      else if (bt==17) {
	if (irc_play) irc_sendschenken(sn);
	di_schenken(sn);
      }
      else if (bt==2) di_copyr(sn);
    }
    else if (ob==dicopyr[sn]) {
      remove_di(sn);
      di_options(sn);
    }
    else if (ob==distrateg[sn]) {
      remove_di(sn);
      for (i=0;i<2;i++) stg[i]=strateg[i];
      for (i=-4;i<=4;i++) {
	if (distrateg[sn][i+10].spec&OB_SELECTED) {
	  stg[0]=i;
	}
	if (distrateg[sn][i+20].spec&OB_SELECTED) {
	  stg[1]=i;
	}
      }
      sav=0;
      for (i=0;i<2;i++) {
	if (stg[i]!=strateg[i]) {
	  strateg[i]=stg[i];
	  sav=1;
	}
      }
      if (distrateg[sn][26].spec&OB_SELECTED) {
	if (hints[sn]) {
	  hints[sn]=0;
	  if (phase==SPIELEN && sn==(ausspl+vmh)%3 && hintcard!=-1) {
	    show_hint(sn,0,0);
	  }
	  sav=1;
	}
      }
      else {
	if (!hints[sn]) {
	  hints[sn]=1;
	  if (phase==SPIELEN && sn==(ausspl+vmh)%3 && hintcard!=-1) {
	    show_hint(sn,hintcard,1);
	  }
	  sav=1;
	}
      }
      if (sav && !irc_play) save_opt();
      di_varianten(sn);
    }
    else if (ob==divarianten[sn]) {
      remove_di(sn);
      sav=((((((((playramsch
		  )*3+playkontra
		 )*3+playbock
		)*3+spitzezaehlt
	       )*2+playsramsch
	      )*2+resumebock
	     )*2+revolution
	    )*2+klopfen
	   )*2+schenken;
      for (i=0;i<3;i++) {
	if (divarianten[sn][3+i].spec&OB_SELECTED) {
	  playramsch=i;
	}
	if (divarianten[sn][10+i].spec&OB_SELECTED) {
	  playkontra=i;
	}
	if (divarianten[sn][14+i].spec&OB_SELECTED) {
	  playbock=i;
	}
	if (divarianten[sn][21+i].spec&OB_SELECTED) {
	  spitzezaehlt=i;
	}
      }
      playsramsch=!(divarianten[sn][7].spec&OB_SELECTED);
      resumebock=!(divarianten[sn][18].spec&OB_SELECTED);
      revolution=!(divarianten[sn][25].spec&OB_SELECTED);
      klopfen=!(divarianten[sn][28].spec&OB_SELECTED);
      schenken=!(divarianten[sn][31].spec&OB_SELECTED);
      if (sav!=((((((((playramsch
		       )*3+playkontra
		      )*3+playbock
		     )*3+spitzezaehlt
		    )*2+playsramsch
		   )*2+resumebock
		  )*2+revolution
		 )*2+klopfen
		)*2+schenken) {
	save_opt();
      }
      if (playbock) di_bockevents(sn);
      else di_verzoegerung(sn);
    }
    else if (ob==dibockevents[sn]) {
      remove_di(sn);
      sav=bockevents;
      bockevents=0;
      for (i=1,j=0;i<=BOCK_BEI_LAST;i*=2,j++) {
	if (dibockevents[sn][2+j].spec&OB_SELECTED) {
	  bockevents+=i;
	}
      }
      if (sav!=bockevents) save_opt();
      di_verzoegerung(sn);
    }
    else if (ob==diverzoegerung[sn]) {
      sav=nimmstich[sn][0]*2+fastdeal;
      diverzoegerung[sn][bt].spec&=~OB_SELECTED;
      switch (bt) {
      case 4:
	nimmstich[sn][0]=0;
	break;
      case 5:
	if (nimmstich[sn][0]==101) nimmstich[sn][0]--;
	else nimmstich[sn][0]-=10;
	break;
      case 6:
	nimmstich[sn][0]--;
	break;
      case 7:
	nimmstich[sn][0]++;
	if (irc_play && nimmstich[sn][0]>=101) nimmstich[sn][0]=100;
	break;
      case 8:
	nimmstich[sn][0]+=10;
	if (irc_play && nimmstich[sn][0]>=101) nimmstich[sn][0]=100;
	break;
      case 9:
	if (irc_play) nimmstich[sn][0]=100;
	else nimmstich[sn][0]=101;
	break;
      }
      if (nimmstich[sn][0]<0) nimmstich[sn][0]=0;
      else if (nimmstich[sn][0]>101) nimmstich[sn][0]=101;
      fastdeal=diverzoegerung[sn][11].spec&OB_SELECTED?1:0;
      if (sav!=nimmstich[sn][0]*2+fastdeal && !irc_play) save_opt();
      prverz(sn);
      if (bt==13) {
	remove_di(sn);
	di_menubutton(sn);
      }
      else {
	draw_di(sn,3);
	draw_di(sn,bt);
      }
    }
    else if (ob==dimenubutton[sn]) {
      remove_di(sn);
      sav=mbutton[sn];
      for (i=0;i<=5;i++) {
	if (dimenubutton[sn][2+i].spec&OB_SELECTED) {
	  mbutton[sn]=i;
	}
      }
      if (sav!=mbutton[sn] && !irc_play) save_opt();
    }
  }
}

VOID hndl_events()
{
  int sn,b,x,y,i,opt,bt;
  XEvent event;
  char buf[100],*l;
  OBJECT *ob;

  if (!lost[0] && !lost[1] && !lost[2]) setcurs();
  waitt(50,1);
  for (sn=0;sn<numsp;sn++) {
    if (irc_play && irc_pos!=sn) continue;
    b=x=y=opt=0;
    while (!lost[sn] && XCheckMaskEvent(dpy[sn],~0,&event)) {
      ob=actdial[sn];
      switch (event.type) {
      case KeyPress:
	if ((i=XLookupString((XKeyEvent *)&event,buf,sizeof(buf)-1,
			     (KeySym *)0,(XComposeStatus *)0))) {
	  if (irc_state==IRC_PLAYING) {
	    irc_xinput(buf,i);
	  }
	  else if (!ob) {
	    b=sn+1;
	    opt=2;
	  }
	}
	break;
      case ButtonPress:
	if (ob) {
	  for (bt=1;bt<ob[0].spec;bt++) {
	    if (event.xbutton.window==ob[bt].win &&
		ob[bt].spec&(OB_BUTTON|OB_EXIT)) {
	      if (irc_play &&
		  ob!=diende[sn] &&
		  ob!=diterm[sn] &&
		  ob!=diproto[sn] &&
		  ob!=diliste[sn] &&
		  ob!=diloesch &&
		  ob!=dioptions[sn] &&
		  ob!=dicopyr[sn] &&
		  ob!=distrateg[sn] &&
		  ob!=divarianten[sn] &&
		  ob!=dibockevents[sn] &&
		  ob!=diverzoegerung[sn] &&
		  ob!=dimenubutton[sn]) irc_sendbtev(sn,bt);
	      hndl_btevent(sn,bt);
	      break;
	    }
	  }
	}
	else {
	  b=sn+1;
	  x=event.xbutton.x;
	  y=event.xbutton.y;
	  opt=(mbutton[sn] && mbutton[sn]==event.xbutton.button)?2
	    :!mbutton[sn]?1:0;
	}
	break;
      case Expose:
	if (event.xexpose.window==win[sn]) {
	  XCopyArea(dpy[sn],bck[sn],win[sn],gc[sn],
		    event.xexpose.x,event.xexpose.y,
		    event.xexpose.width,event.xexpose.height,
		    event.xexpose.x,event.xexpose.y);
	}
	else if (ob &&
		 (i=ismemb(event.xexpose.window,ob))) {
	  if (!event.xexpose.count) {
	    draw_di(sn,i<0?0:i);
	  }
	}
	else if (resdial[sn] &&
		 (i=ismemb(event.xexpose.window,resdial[sn]))) {
	  if (!event.xexpose.count) {
	    draw_dial(sn,i<0?0:i,resdial[sn]);
	  }
	}
	break;
      }
    }
    if (b) {
      hndl_button(b-1,x,y,opt,1);
    }
  }
  if (irc_play) {
    while ((l=irc_getline())) {
      irc_parse(l);
    }
    irc_talk((char *)0);
  }
}

VOID getob_xywhbd(sn,ob,i,x,y,w,h,bp)
int sn;
OBJECT *ob;
int i,*x,*y,*w,*h,*bp;
{
  int bd,ba,d3d;

  bd=ob[i].spec&OB_BUTTON?1:ob[i].spec&OB_EXIT?2:0;
  ba=gfx3d[sn]?2*bd:0;
  d3d=bd && gfx3d[sn]?1:0;
  *x=ob[i].x*charw[sn]-bd+d3d;
  *y=ob[i].y*charh[sn]-bd+d3d;
  *w=ob[i].w*charw[sn]-!!bd-d3d+ba;
  *h=ob[i].h*charh[sn]-!!bd-d3d+ba;
  *bp=bd;
}

VOID create_dial(sn,x,y,dy,ob)
int sn;
int x,y,dy;
OBJECT *ob;
{
  Window rt,wi;
  int i,w,h,bd;

  rt=XCreateSimpleWindow(dpy[sn],win[sn],x-1,y-dy-1,
			 ob[0].w*charw[sn],dy+ob[0].h*charh[sn],
			 1,fgpix[sn],bgpix[sn]);
  ob[0].y=dy;
  ob[0].win=rt;
  XSelectInput(dpy[sn],rt,ExposureMask);
  for (i=1;i<ob[0].spec;i++) {
    if (ob[i].spec&OB_HIDDEN) {
      ob[i].win=None;
      continue;
    }
    ob[i].spec&=~OB_SELECTED;
    getob_xywhbd(sn,ob,i,&x,&y,&w,&h,&bd);
    wi=XCreateSimpleWindow(dpy[sn],rt,x,y+dy,w,h,gfx3d[sn]?0:bd,
			   ob_disabled(ob,i)?bgpix[sn]:fgpix[sn],
			   bd && !ob_disabled(ob,i)?btpix[sn]:bgpix[sn]);
    ob[i].win=wi;
    XSelectInput(dpy[sn],wi,ExposureMask|
		 (ob[i].spec&(OB_BUTTON|OB_EXIT)?ButtonPressMask:0));
    XMapWindow(dpy[sn],wi);
  }
  XMapWindow(dpy[sn],rt);
}

VOID create_di(sn,ob)
int sn;
OBJECT *ob;
{
  int x,y;

  if (actdial[sn]) remove_di(sn);
  x=(desk[sn].w-ob[0].w*charw[sn])/2;
  y=(desk[sn].h-ob[0].h*charh[sn])/2;
  create_dial(sn,x,y,0,ob);
  actdial[sn]=ob;
}

VOID remove_dial(sn,ob)
int sn;
OBJECT *ob;
{
  XDestroyWindow(dpy[sn],ob[0].win);
}

VOID remove_di(sn)
int sn;
{
  remove_dial(sn,actdial[sn]);
  actdial[sn]=0;
}

VOID draw_3d(w,b,sn,x1,y1,x2,y2,rev)
Window w;
Pixmap b;
int sn,x1,y1,x2,y2,rev;
{
  XPoint pts[3];

  pts[0].x=(short)x1;
  pts[0].y=(short)y2;
  pts[1].x=(short)x1;
  pts[1].y=(short)y1;
  pts[2].x=(short)x2;
  pts[2].y=(short)y1;
  change_gc(sn,rev?b3dpix[sn]:w3dpix[sn],gc);
  XDrawLines(dpy[sn],w,gc[sn],pts,3,CoordModeOrigin);
  if (b!=None) XDrawLines(dpy[sn],b,gc[sn],pts,3,CoordModeOrigin);
  pts[0].x++;
  pts[1].x=(short)x2;
  pts[1].y=(short)y2;
  pts[2].y++;
  change_gc(sn,rev?w3dpix[sn]:b3dpix[sn],gc);
  XDrawLines(dpy[sn],w,gc[sn],pts,3,CoordModeOrigin);
  if (b!=None) XDrawLines(dpy[sn],b,gc[sn],pts,3,CoordModeOrigin);
  change_gc(sn,fgpix[sn],gc);
}

VOID draw_dial(sn,i,ob)
int sn,i;
OBJECT *ob;
{
  int x,y,w,h,l,bd,sel;

  if (!ob) return;
  if (!i) {
    w=ob[0].w*charw[sn];
    h=ob[0].h*charh[sn]+ob[0].y;
    XClearWindow(dpy[sn],ob[0].win);
    if (gfx3d[sn]) {
      draw_3d(ob[0].win,None,sn,0,0,w-1,h-1,0);
      draw_3d(ob[0].win,None,sn,1,1,w-2,h-2,0);
    }
    else {
      change_gc(sn,btpix[sn],gc);
      XDrawRectangle(dpy[sn],ob[0].win,gc[sn],0,0,w-1,h-1);
      XDrawRectangle(dpy[sn],ob[0].win,gc[sn],1,1,w-3,h-3);
      change_gc(sn,fgpix[sn],gc);
      XDrawRectangle(dpy[sn],ob[0].win,gc[sn],2,2,w-5,h-5);
    }
  }
  else {
    if (ob[i].spec&OB_HIDDEN) return;
    sel=ob[i].spec&OB_SELECTED;
    XClearWindow(dpy[sn],ob[i].win);
    if (ob[i].str!=OB_NONE && (l=strlen(ob[i].str))) {
      x=y=0;
      if (ob[i].spec&(OB_BUTTON|OB_EXIT|OB_CENTERED)) {
	x=(ob[i].w*charw[sn]-XTextWidth(dfont[sn],ob[i].str,l))/2;
	y=(ob[i].h-1)*charh[sn]/2;
      }
      else if (ob[i].spec&OB_RIGHT) {
	x=ob[i].w*charw[sn]-XTextWidth(dfont[sn],ob[i].str,l);
      }
      if (gfx3d[sn]) {
	if (ob[i].spec&OB_EXIT) x+=2,y+=2;
	else if (ob[i].spec&OB_BUTTON) x++,y++;
      }
      XDrawString(dpy[sn],ob[i].win,gc[sn],x,y+dfont[sn]->ascent,
		  ob[i].str,l);
      if (ob[i].spec&OB_BOLD) {
	XDrawString(dpy[sn],ob[i].win,gc[sn],x+1,y+dfont[sn]->ascent,
		    ob[i].str,l);
      }
      if (ob[i].spec&OB_UNDERLINED && l) {
	XDrawLine(dpy[sn],ob[i].win,gc[sn],x,ob[i].h*charh[sn]-2,
		  x+XTextWidth(dfont[sn],ob[i].str,l),ob[i].h*charh[sn]-2);
      }
    }
    if (gfx3d[sn]) {
      getob_xywhbd(sn,ob,i,&x,&y,&w,&h,&bd);
      if (!bd && sel) bd=1;
      if (sel || !ob_disabled(ob,i)) {
	if (bd) {
	  draw_3d(ob[i].win,None,sn,0,0,w-1,h-1,sel);
	  if (bd==2) draw_3d(ob[i].win,None,sn,1,1,w-2,h-2,sel);
	}
      }
    }
    else if (sel) {
      bd=ob[i].spec&(OB_BUTTON|OB_EXIT) && !ob_disabled(ob,i);
      if (bd) change_gcxor(sn,btpix[sn]^fgpix[sn]^bgpix[sn]);
      XFillRectangle(dpy[sn],ob[i].win,gcxor[sn],0,0,
		     ob[i].w*charw[sn],ob[i].h*charh[sn]);
      if (bd) change_gcxor(sn,fgpix[sn]);
    }
  }
}

VOID draw_di(sn,idx)
int sn,idx;
{
  draw_dial(sn,idx,actdial[sn]);
}

VOID refresh()
{
  int sn,i;
  XEvent event;

  for (sn=0;sn<numsp;sn++) {
    if (lost[sn]) continue;
    if (resdial[sn]) {
      for (i=0;i<resdial[sn][0].spec;i++) {
	if (resdial[sn][i].win) {
	  while (XCheckTypedWindowEvent(dpy[sn],resdial[sn][i].win,
					Expose,&event)) {
	    if (!event.xexpose.count) {
	      draw_dial(sn,i,resdial[sn]);
	    }
	  }
	}
      }
    }
    if (actdial[sn]) {
      for (i=0;i<actdial[sn][0].spec;i++) {
	if (actdial[sn][i].win) {
	  while (XCheckTypedWindowEvent(dpy[sn],actdial[sn][i].win,
					Expose,&event)) {
	    if (!event.xexpose.count) {
	      draw_dial(sn,i,actdial[sn]);
	    }
	  }
	}
      }
    }
    while (XCheckTypedWindowEvent(dpy[sn],win[sn],Expose,&event)) {
      XCopyArea(dpy[sn],bck[sn],win[sn],gc[sn],event.xexpose.x,event.xexpose.y,
		event.xexpose.width,event.xexpose.height,
		event.xexpose.x,event.xexpose.y);
    }
    XFlush(dpy[sn]);
  }
}

VOID prspnam(txt,sn)
char *txt;
int sn;
{
  strcpy(txt,spnames[sn][0]);
  if (spnames[sn][1][0]) {
    strcat(txt," ");
    strcat(txt,spnames[sn][1]);
  }
}

VOID di_info(sn,th)
int sn,th;
{
  int s,x,y,w;
  char txt[20];
  char clr[100];

  if (numsp==1) return;
  w=3*desk[sn].cardw/XTextWidth(dfont[sn]," ",1)+1;
  if (w>99) w=99;
  clr[w]=0;
  while (w) clr[--w]=' ';
  for (s=0;s<3;s++) {
    if (s!=sn) {
      prspnam(txt,s);
      x=(s==left(sn)?desk[sn].com1x
	 :desk[sn].com2x)+30*desk[sn].f/desk[sn].q;
      y=(s==left(sn)?desk[sn].com1y
	 :desk[sn].com2y)+130*desk[sn].f/desk[sn].q;
      if (th<-1) {
	v_gtext(sn,x,y,0,clr);
	continue;
      }
      v_gtext(sn,x,y,0,txt);
      y+=charh[sn];
      if ((phase==SPIELEN || phase==SCHENKEN) &&
	  stich==1 && s==spieler && trumpf!=5) {
	v_gtext(sn,x,y,0,clr);
	strcpy(txt,textarr[TX_SPIELT]);
	strcat(txt,textarr[revolang?TX_REVOLUTION:TX_NULL+trumpf+1]);
	v_gtext(sn,x,y,0,txt);
	y+=charh[sn];
	v_gtext(sn,x,y,0,clr);
	if (ouveang && !revolang) {
	  if (handsp && trumpf==-1) {
	    v_gtext(sn,x,y,0,textarr[TX_OUVE_HAND]);
	  }
	  else {
	    v_gtext(sn,x,y,0,textarr[TX_OUVE]);
	  }
	}
	else if (schwang) {
	  v_gtext(sn,x,y,0,textarr[TX_SCHW_ANGE]);
	}
	else if (schnang) {
	  v_gtext(sn,x,y,0,textarr[TX_SCHN_ANGE]);
	}
	else if (handsp && !revolang) {
	  v_gtext(sn,x,y,0,textarr[TX_HAND]);
	}
      }
      else {
	if (th>=0) {
	  v_gtext(sn,x,y,0,clr);
	  if (s==th) {
	    v_gtext(sn,x,y,0,textarr[TX_UEBERLEGT]);
	  }
	  v_gtext(sn,x,y+charh[sn],0,clr);
	}
      }
    }
  }
}

VOID di_hand()
{
  create_di(spieler,dihand);
}

VOID next_grandhand(sn)
int sn;
{
  sn=left(sn);
  if (sn==hoerer) {
    ktrply=-1;
    init_ramsch();
  }
  else {
    di_grandhand(sn);
  }
}

VOID di_grandhand(sn)
int sn;
{
  if (iscomp(sn)) {
    if (testgrandhand(sn)) do_grandhand(sn);
    else next_grandhand(sn);
  }
  else {
    ktrply=sn;
    create_di(sn,digrandhand);
  }
}

VOID di_term(sn,s)
int sn,s;
{
  static char txt[20];

  if (lost[sn]) return;
  diterm[sn][2].str=txt;
  prspnam(txt,s);
  create_di(sn,diterm[sn]);
}

VOID di_ende(sn)
int sn;
{
  create_di(sn,diende[sn]);
}

VOID di_loesch(sn)
int sn;
{
  create_di(sn,diloesch);
}

VOID di_ansage()
{
  static char txt[30];
  int i,ktr;

  ktrply=-1;
  diansage[2].str=dikontra[2].str=dikonre[2].str=txt;
  strcpy(txt,textarr[revolang?TX_REVOLUTION:TX_NULL+trumpf+1]);
  if (trumpf==-1) {
    if (!revolang) {
      if (ouveang) {
	strcat(txt," ");
	strcat(txt,textarr[TX_OUVE]);
      }
      if (handsp) {
	strcat(txt," ");
	strcat(txt,textarr[TX_HAND]);
      }
    }
  }
  else {
    if (ouveang) {
      strcat(txt," ");
      strcat(txt,textarr[TX_OUVE]);
    }
    else if (schwang) {
      strcat(txt," ");
      strcat(txt,textarr[TX_SCHWARZ]);
    }
    else if (schnang) {
      strcat(txt," ");
      strcat(txt,textarr[TX_SCHNEIDER]);
    }
    else if (handsp) {
      strcat(txt," ");
      strcat(txt,textarr[TX_HAND]);
    }
  }
  ktr=0;
  ktrnext=-1;
  if (playkontra) {
    for (i=0;i<3;i++) {
      if (i!=spieler && (playkontra==1 || sagte18[i])) {
	if (!ktr) {
	  ktr=1;
	  ktrsag=i;
	  ktrnext=i;
	}
	else if ((sagte18[i] && (i==sager || i==hoerer)) ||
		 !sagte18[ktrsag]) {
	  ktrsag=i;
	}
	else {
	  ktrnext=i;
	}
      }
    }
  }
  if (numsp==1 && iscomp(spieler) && !ktr) {
    create_di(0,diansage);
  }
  else if (ktr) {
    di_kontra(ktrsag);
  }
  else do_angesagt();
}

VOID di_kontra(sn)
int sn;
{
  ktrply=sn;
  ktrsag=sn;
  sort2[sn]=sort2[spieler]=trumpf==-1;
  if (!iscomp(spieler)) initscr(spieler,1);
  if (iscomp(sn)) {
    di_ktrnext(sn,sage_kontra(sn));
  }
  else {
    initscr(sn,1);
    create_di(sn,dikontra);
  }
}

VOID di_rekontra(sn)
int sn;
{
  static char txt[30];

  ktrply=-1;
  direkontra[2].str=txt;
  strcpy(txt,textarr[TX_VON]);
  prspnam(txt+strlen(txt),sn);
  if (iscomp(spieler)) {
    di_ktrnext(sn,sage_re(spieler));
  }
  else {
    create_di(spieler,direkontra);
  }
}

VOID di_konre(sn)
int sn;
{
  static char txt[30];

  ktrply=sn;
  dikonre[3].str=txt;
  strcpy(txt,textarr[TX_MIT_KONTRA]);
  if (kontrastufe==2) {
    strcat(txt," & ");
    strcat(txt,textarr[TX_RE]);
  }
  create_di(sn,dikonre);
}

VOID di_ktrnext(sn,f)
int sn,f;
{
  if (kontrastufe==1) {
    if (f) {
      kontrastufe=2;
      prot2.verdopp[spieler]=2;
      ktrnext=left(ktrsag)==spieler?right(ktrsag):left(ktrsag);
      if (iscomp(ktrnext)) ktrnext=-1;
      if (iscomp(ktrsag)) ktrsag=ktrnext,ktrnext=-1;
    }
    else {
      ktrnext=-1;
      ktrsag=left(ktrsag)==spieler?right(ktrsag):left(ktrsag);
      if (iscomp(ktrsag)) ktrsag=ktrnext;
    }
    if (ktrsag>=0) {
      di_konre(ktrsag);
    }
    else {
      do_angesagt();
    }
  }
  else if (f) {
    kontrastufe=1;
    prot2.verdopp[sn]=2;
    di_rekontra(sn);
  }
  else if (ktrnext>=0 && ktrnext!=sn) {
    di_kontra(ktrnext);
    ktrnext=-1;
  }
  else {
    if (numsp==1 && iscomp(spieler) && playkontra==2 && !sagte18[0]) {
      create_di(0,diansage);
    }
    else do_angesagt();
  }
}

VOID di_dicht()
{
  phase=SPIELDICHT;
  if (iscomp(spieler)) {
    spielendscr();
  }
  else {
    didicht[1].str=textarr[trumpf==-1?TX_NULL_DICHT:TX_REST_BEI_MIR];
    create_di(spieler,didicht);
  }
}

VOID di_weiter(ini)
int ini;
{
  int x,y,sn;
  static int num;

  if (!numsp || !ini) {
    if (!numsp || !--num) {
      finishgame();
    }
    return;
  }
  num=numsp;
  for (sn=0;sn<numsp;sn++) {
    x=(desk[sn].w-diweiter[sn][0].w*charw[sn])/2;
    y=(desk[sn].playy+desk[sn].skaty+desk[sn].cardh+
       -diweiter[sn][0].h*charh[sn])/2;
    if (actdial[sn]) remove_di(sn);
    create_dial(sn,x,y,0,diweiter[sn]);
    actdial[sn]=diweiter[sn];
  }
}

VOID di_klopfen(sn)
int sn;
{
  create_di(sn,diklopfen);
}

VOID di_schenken(sn)
int sn;
{
  int msp;

  if (schenkstufe) return;
  phase=SCHENKEN;
  schenkstufe=1;
  schenknext=sn;
  msp=left(sn)==spieler?left(spieler):left(sn);
  if (iscomp(msp)) {
    if (kontrastufe) di_nichtschenken(msp);
    else di_geschenkt();
  }
  else {
    create_di(msp,dischenken);
    schenkply=msp;
  }
}

VOID di_geschenkt()
{
  if (iscomp(spieler)) {
    finishgame();
  }
  else {
    digeschenkt[4].str=textarr[schnang?TX_SCHWARZ:TX_SCHNEIDER];
    create_di(spieler,schwang || trumpf==-1?diendeschenken:digeschenkt);
    schenkply=spieler;
  }
}

VOID di_wiederschenken(sn,f)
int sn,f;
{
  int msp;

  msp=left(sn)==spieler?left(spieler):left(sn);
  schenknext=f && !iscomp(msp)?msp:-1;
  create_di(sn,diwiederschenken);
  schenkply=sn;
}

VOID di_nichtschenken(sn)
int sn;
{
  sn=left(sn)==spieler?left(spieler):left(sn);
  if (!iscomp(sn)) {
    create_di(sn,dinichtschenken);
    schenkply=sn;
  }
  else {
    phase=SPIELEN;
  }
}

VOID di_schieben()
{
  int sn;

  do {
    if (vmh) save_skat(vmh+1);
    sn=(ausspl+vmh)%3;
    spieler=sn;
    if (iscomp(sn)) {
      if (comp_sramsch(sn)) {
	return;
      }
      vmh=left(vmh);
    }
    else {
      create_di(sn,playsramsch?dischieben:diklopfen);
      return;
    }
  } while (vmh);
  start_ramsch();
}

int di_verdoppelt(f,kl)
int f,kl;
{
  int sn;
  static int verd1,verd2;
  static char txt[20];

  if (!f) {
    diverdoppelt[3].str=textarr[kl?TX_KLOPFT:TX_SCHIEBT];
    prspnam(txt,spieler);
    diverdoppelt[2].str=txt;
    verd1=verd2=-1;
    for (sn=0;sn<numsp;sn++) {
      if (sn!=spieler) {
	if (verd1==-1) verd1=sn;
	else verd2=sn;
      }
    }
    sramschstufe++;
    prot2.verdopp[spieler]=kl+1;
  }
  if (verd1!=-1) {
    spieler=verd1;
    create_di(spieler,diverdoppelt);
    verd1=verd2;
    verd2=-1;
    return 1;
  }
  if (!iscomp(spieler)) {
    vmh=left(vmh);
    if (vmh) di_schieben();
    else start_ramsch();
  }
  return 0;
}

VOID di_buben()
{
  create_di(spieler,dibuben);
}

VOID di_spiel()
{
  static char txt[33];

  ktrply=-1;
  sprintf(txt,textarr[TX_GEREIZT_BIS_N],reizw[reizp]);
  dispiel[14].str=txt;
  dispiel[11].spec=spitzezaehlt && kannspitze?OB_BUTTON:OB_HIDDEN;
  dispiel[13].spec=revolution?OB_BUTTON:OB_HIDDEN;
  dispiel[7].next=revolution?6:-5;
  create_di(spieler,dispiel);
}

VOID list_fun(sn)
int sn;
{
  int i,j,k,s,e,r,d,curr[3][3],cp;
  static char txt[3][13][4][10];

  for (i=0;i<3;i++) {
    for (j=0;j<3;j++) {
      curr[i][j]=splsum[i][j];
    }
  }
  cp=alist[sn];
  for (j=0;j<splfirst[sn];j++) {
    modsum(curr,j,(int *)0,(int *)0,(int *)0,(int *)0);
  }
  for (k=0;k<3;k++) {
    diliste[sn][8+k].str=txt[sn][0][k];
    sprintf(txt[sn][0][k],"%d",curr[k][cp]);
  }
  for (i=splfirst[sn],j=1;j<13 && i<splstp;i++,j++) {
    modsum(curr,i,&s,&e,&r,&d);
    for (k=0;k<4;k++) {
      diliste[sn][7+4*j+k].str=txt[sn][j][k];
      sprintf(txt[sn][j][k],"%d",k==3?e>0 && r && !d?-e:e:curr[k][cp]);
    }
    if ((cp==1 || (r && !d)) && e>0) {
      diliste[sn][7+4*j+s].str=OB_NONE;
    }
    else if (cp!=2 || r || e>=0) {
      if (e==0) s=4;
      for (k=0;k<3;k++) {
	if (k!=s) diliste[sn][7+4*j+k].str=OB_NONE;
      }
    }
  }
  for (;j<13;j++) {
    for (k=0;k<4;k++) {
      diliste[sn][7+4*j+k].str=OB_NONE;
    }
  }
}

VOID di_liste(sn,ini)
int sn,ini;
{
  static char ltyp[3][10];

  sprintf(ltyp[sn],"<%d>",alist[sn]+1);
  set_names(diliste[sn],2);
  if (ini) splfirst[sn]=((splstp>0?splstp-1:0)/12)*12;
  diliste[sn][59].str=splfirst[sn]>=12?"<-":ltyp[sn];
  diliste[sn][59].spec=splfirst[sn]<12 && irc_play
    ?OB_DISABLED|OB_EXIT:OB_EXIT;
  diliste[sn][61].str=splfirst[sn]+12>=splstp?textarr[TX_LOESCHEN]:"->";
  diliste[sn][61].spec=splfirst[sn]+12<splstp ||
    ((splstp || (irc_play &&
		 (sum[0][0] || sum[0][1] || sum[0][2] ||
		  sum[1][0] || sum[1][1] || sum[1][2] ||
		  sum[2][0] || sum[2][1] || sum[2][2]))) &&
     ((!irc_play && !sn) || (irc_play &&
			     (phase==REIZEN ||
			      (phase==SPIELEN && sn==(ausspl+vmh)%3)))) &&
     ini)?OB_EXIT:OB_HIDDEN;
  list_fun(sn);
  create_di(sn,diliste[sn]);
}

VOID pformat(f,spec,txt,fil)
FILE *f;
int spec;
char *txt;
int fil;
{
  int i,l;

  i=12;
  if (spec&OB_CENTERED) {
    l=(12-strlen(txt))/2;
    i-=l;
    while (l-->0) fputc(' ',f);
  }
  while (*txt) {
    if (unformatted) {
      if (spec&OB_BOLD) fprintf(f,"%c",toupper(*txt));
      else if (spec&OB_UNDERLINED && *txt==' ') fputc('_',f);
      else fputc(*txt,f);
    }
    else {
      if (spec&OB_BOLD) fprintf(f,"%c\b%c",*txt,*txt);
      else if (spec&OB_UNDERLINED) fprintf(f,"_\b%c",*txt);
      else fputc(*txt,f);
    }
    txt++;
    i--;
  }
  if (fil) while (i-->0) fputc(' ',f);
}

VOID prot_fun(sn,f)
int sn;
FILE *f;
{
  int tr,e,i,j,s,stiche[10][3];
  static char txt[3][10][3][20];

  tr=trumpf;
  trumpf=prot1.trumpf;
  for (s=0;s<3;s++) {
    for (i=0;i<10;i++) stiche[i][s]=prot1.stiche[i][s];
    for (i=(protsort[sn]?0:prot1.stichgem);i<9;i++) {
      for (j=i+1;j<10;j++) {
	if (lower(stiche[i][s],stiche[j][s],trumpf==-1)) {
	  swap(&stiche[i][s],&stiche[j][s]);
        }
      }
    }
  }
  trumpf=tr;
  for (i=0;i<10;i++) {
    for (s=0;s<3;s++) {
      if (protsort[sn]) {
        e=prot1.trumpf!=-1 &&
	  (stiche[i][s]>>3==prot1.trumpf || (stiche[i][s]&7)==BUBE)
	    ?e=OB_BOLD:OB_NONE;
      }
      else {
	if (i && prot1.stichgem<=i) {
	  e=prot1.gewonn && prot1.stichgem?OB_NONE:OB_HIDDEN;
	}
	else {
	  e=prot1.anspiel[i]==s?OB_UNDERLINED:OB_NONE;
	  if (prot1.gemacht[i]==s) e|=OB_BOLD;
	}
      }
      diproto[sn][8+3*i+s].spec=e;
      diproto[sn][8+3*i+s].str=txt[sn][i][s];
      if (prot1.stichgem || protsort[sn]) {
	if (prot1.spitze &&
	    stiche[i][s]==(prot1.trumpf==4?BUBE:SIEBEN|prot1.trumpf<<3)) {
	  strcpy(txt[sn][i][s],textarr[TX_SPITZE_PROT]);
	}
	else {
	  strcpy(txt[sn][i][s],textarr[TX_KARO+(stiche[i][s]>>3)]);
	  strcat(txt[sn][i][s],textarr[TX_A+(stiche[i][s]&7)]);
	}
      }
      else {
	if (e==OB_UNDERLINED) strcpy(txt[sn][i][s]," ");
	else txt[sn][i][s][0]=0;
	strcat(txt[sn][i][s],
	       textarr[prot1.schenken?
		       prot1.spieler==s?TX_AKZEPTIERT:TX_SCHENKEN:TX_PASSE]);
	if (e==OB_UNDERLINED) strcat(txt[sn][i][s]," ");
      }
    }
    if (f && diproto[sn][8+3*i].spec!=OB_HIDDEN) {
      fprintf(f,"  ");
      for (s=0;s<3;s++) {
	pformat(f,diproto[sn][8+3*i+s].spec,txt[sn][i][s],1);
      }
      fprintf(f,"\n");
    }
  }
}

VOID im_skat(s,i)
char *s;
int i;
{
  strcpy(s,textarr[TX_KARO+(prot1.skat[i][0]>>3)]);
  strcat(s,textarr[TX_A+(prot1.skat[i][0]&7)]);
  strcat(s,",");
  strcat(s,textarr[TX_KARO+(prot1.skat[i][1]>>3)]);
  strcat(s,textarr[TX_A+(prot1.skat[i][1]&7)]);
}

VOID di_proto(sn,ini,log)
int sn,ini,log;
{
  static char txt[3][40],imski[3][40],imskw[3][40],bis[3][10],aug[3][20];
  static char vhschob[3][40],mhschob[3][40];
  char *hd="----------------------------------------\n";
  char *tl="========================================\n";
  char hdbuf[100],spn[12];
  int s,p,u1,u2,u3;
  FILE *f;

  if (!prot_file) log=0;
  if (log) {
    f=strcmp(prot_file,"-")?fopen(prot_file,"a"):stdout;
    if (!f) {
      fprintf(stderr,"Can't write file %s\n",prot_file);
      return;
    }
    for (s=0;s<3;s++) {
      if (prot1.verdopp[s]==2) {
	strcpy(spn," ");
	strcat(spn,spnames[s][0]);
	strcat(spn," ");
	p=OB_CENTERED|OB_UNDERLINED;
      }
      else {
	strcpy(spn,spnames[s][0]);
	p=OB_CENTERED;
      }
      pformat(f,p,spn,1);
    }
    fputc('\n',f);
    for (s=0;s<3;s++) {
      if (prot1.verdopp[s]==2 && spnames[s][1][0]) {
	strcpy(spn," ");
	strcat(spn,spnames[s][1]);
	strcat(spn," ");
	p=OB_CENTERED|OB_UNDERLINED;
      }
      else {
	strcpy(spn,spnames[s][1]);
	p=OB_CENTERED;
      }
      pformat(f,p,spn,1);
    }
    fprintf(f,"\n%s",hd);
  }
  else f=0;
  if (ini) protsort[sn]=0;
  set_names(diproto[sn],2);
  for (p=0;p<2;p++) {
    for (s=0;s<3;s++) {
      diproto[sn][2+p*3+s].spec=prot1.verdopp[s]==2?
	OB_CENTERED|OB_UNDERLINED:OB_CENTERED;
    }
  }
  diproto[sn][44].str=txt[sn];
  if (prot1.trumpf==5 && prot1.augen==0 && !prot1.gewonn) {
    strcpy(txt[sn],textarr[TX_NIEMAND]);
  }
  else {
    prspnam(txt[sn],prot1.spieler);
  }
  if (prot1.trumpf==5) {
    strcat(txt[sn],textarr[prot1.gewonn?TX_GEWANN:TX_VERLOR]);
    strcat(txt[sn],textarr[TX_DEN_RAMSCH]);
  }
  else {
    strcat(txt[sn],textarr[TX_SPIELTE]);
    strcat(txt[sn],textarr[prot1.revolution?TX_REVOLUTION:
			   TX_NULL+prot1.trumpf+1]);
  }
  diproto[sn][38].str=textarr[TX_IM_SKAT_IST];
  diproto[sn][38].spec=OB_NONE;
  diproto[sn][39].str=imski[sn];
  diproto[sn][39].spec=OB_NONE;
  diproto[sn][40].str=textarr[TX_IM_SKAT_WAR];
  diproto[sn][40].spec=OB_NONE;
  diproto[sn][41].str=imskw[sn];
  diproto[sn][41].spec=OB_NONE;
  diproto[sn][42].str=textarr[TX_GEREIZT_BIS];
  diproto[sn][42].spec=OB_NONE;
  diproto[sn][43].str=bis[sn];
  diproto[sn][43].spec=OB_NONE;
  im_skat(imskw[sn],0);
  im_skat(imski[sn],1);
  im_skat(vhschob[sn],2);
  im_skat(mhschob[sn],3);
  sprintf(bis[sn],"%d",prot1.gereizt);
  if (prot1.sramsch) {
    p=protsort[sn];
    u1=prot1.verdopp[prot1.anspiel[0]]==1?OB_UNDERLINED:OB_NONE;
    u2=prot1.verdopp[left(prot1.anspiel[0])]==1?OB_UNDERLINED:OB_NONE;
    u3=prot1.verdopp[right(prot1.anspiel[0])]==1?OB_UNDERLINED:OB_NONE;
  }
  else {
    p=protsort[sn] && !prot1.handsp?0:prot1.ehsso;
  }
  if (p) {
    if (prot1.sramsch) {
      diproto[sn][38].str=textarr[TX_VH_SCHOB];
      diproto[sn][38].spec=u1;
      diproto[sn][39].str=vhschob[sn];
      diproto[sn][40].str=textarr[TX_MH_SCHOB];
      diproto[sn][40].spec=u2;
      diproto[sn][41].str=mhschob[sn];
      diproto[sn][42].str=textarr[TX_HH_SCHOB];
      diproto[sn][42].spec=u3;
      diproto[sn][43].str=imski[sn];
    }
    else if (prot1.trumpf==5) {
      diproto[sn][40].spec=OB_HIDDEN;
      diproto[sn][41].spec=OB_HIDDEN;
      diproto[sn][42].spec=OB_HIDDEN;
      diproto[sn][43].spec=OB_HIDDEN;
    }
    else {
      diproto[sn][40].str=textarr[TX_HAND_GESP+p-1];
      diproto[sn][41].spec=OB_HIDDEN;
      if (!prot1.gereizt) {
	diproto[sn][42].spec=OB_HIDDEN;
	diproto[sn][43].spec=OB_HIDDEN;
      }
    }
  }
  else {
    if (prot1.trumpf==5) {
      if (!prot1.sramsch) {
	diproto[sn][40].spec=OB_HIDDEN;
	diproto[sn][41].spec=OB_HIDDEN;
      }
      diproto[sn][42].spec=OB_HIDDEN;
      diproto[sn][43].spec=OB_HIDDEN;
    }
    else if (!prot1.gereizt) {
      diproto[sn][42].spec=OB_HIDDEN;
      diproto[sn][43].spec=OB_HIDDEN;
    }
  }
  if (!prot1.stichgem) {
    if (prot1.schenken) {
      diproto[sn][45].str=textarr[TX_GEWONNEN];
      diproto[sn][46].str=OB_NONE;
    }
    else {
      diproto[sn][42].spec=OB_HIDDEN;
      diproto[sn][40].str=OB_NONE;
      diproto[sn][41].str=OB_NONE;
      diproto[sn][43].str=OB_NONE;
      diproto[sn][44].str=OB_NONE;
      diproto[sn][45].str=OB_NONE;
      diproto[sn][46].str=OB_NONE;
    }
  }
  else if (prot1.trumpf==-1) {
    diproto[sn][45].str=textarr[prot1.gewonn?TX_GEWONNEN:TX_VERLOREN];
    diproto[sn][46].str=OB_NONE;
  }
  else if (prot1.trumpf==5 && prot1.augen==0 && !prot1.gewonn) {
    diproto[sn][45].str=diproto[sn][46].str=OB_NONE;
  }
  else {
    diproto[sn][45].str=textarr[prot1.gewonn?TX_GEWO_MIT:TX_VERL_MIT];
    diproto[sn][46].str=aug[sn];
    sprintf(aug[sn],textarr[TX_AUGEN],prot1.augen);
  }
  diproto[sn][49].str=protsort[sn]?"<-":"->";
  prot_fun(sn,f);
  if (log) {
    strcpy(hdbuf,hd);
    if (prot1.predef!=1) {
      sprintf(hdbuf+4+irc_play," random_seed %ld %ld%s",
	      prot1.savseed+11*irc_play,prot1.gamenr,
	      prot1.rotateby<0?" L":prot1.rotateby>0?" R":"");
      hdbuf[strlen(hdbuf)]=' ';
    }
    fputs(hdbuf,f);
    protsort[sn]=1;
    prot_fun(sn,f);
    fputs(hd,f);
    fprintf(f,"%s %s\n",textarr[TX_IM_SKAT_IST],imski[sn]);
    if (prot1.stichgem || prot1.schenken) {
      if (prot1.trumpf!=5) {
	fprintf(f,"%s %s\n",textarr[TX_IM_SKAT_WAR],imskw[sn]);
	if (prot1.gereizt) {
	  fprintf(f,"%s %s\n",textarr[TX_GEREIZT_BIS],bis[sn]);
	}
      }
      else if (prot1.sramsch) {
	fprintf(f,"%s %s\n",textarr[TX_IM_SKAT_WAR],imskw[sn]);
	pformat(f,u1,textarr[TX_VH_SCHOB],0);
	fprintf(f," %s\n",vhschob[sn]);
	pformat(f,u2,textarr[TX_MH_SCHOB],0);
	fprintf(f," %s\n",mhschob[sn]);
	pformat(f,u3,textarr[TX_HH_SCHOB],0);
	fprintf(f," %s\n",imski[sn]);
      }
      fprintf(f,"%s\n",txt[sn]);
      if (diproto[sn][40].str!=textarr[TX_IM_SKAT_WAR] && prot1.trumpf!=5) {
	fprintf(f,"%s\n",diproto[sn][40].str);
      }
      if (prot1.trumpf!=5 || prot1.augen!=0 || prot1.gewonn) {
	fprintf(f,"%s",diproto[sn][45].str);
	if (diproto[sn][46].str!=OB_NONE) {
	  fprintf(f," %s",diproto[sn][46].str);
	}
	fputc('\n',f);
      }
    }
    fputs(tl,f);
    if (f!=stdout) fclose(f);
  }
  else {
    diproto[sn][47].str=
      textarr[(!sn || irc_play) && protsort[sn]?TX_SPEICHERN:TX_SPIELLISTE];
    create_di(sn,diproto[sn]);
  }
}

VOID di_result(be)
int be;
{
  int sn;
  static char sa[30],sw[40],sg[40],su[3][3][10],txt[30];
  static int ini,smlh;

  sn=-1;
  if (trumpf==5 && !iscomp(ausspl)) {
    sn=ausspl;
  }
  else if (handsp && !iscomp(spieler)) {
    sn=spieler;
  }
  if (sn>=0) {
    putcard(sn,prot2.skat[1][0],desk[sn].playx+8*desk[sn].cardx,
	    desk[sn].playy);
    putcard(sn,prot2.skat[1][1],desk[sn].playx+9*desk[sn].cardx,
	    desk[sn].playy);
  }
  diresult[2].str=
    (trumpf==5?
     (mes1?textarr[TX_JUNGFRAU]:
      mes2?textarr[TX_DURCHMARSCH]:
      OB_NONE)
     :
     (mes1?textarr[TX_UEBERREIZT]:
      mes2?textarr[TX_SCHNEIDERFREI]:
      mes3?textarr[TX_NICHT_SCHWARZ]:
      mes4?textarr[TX_SPITZE_VERLOREN]:
      OB_NONE));
  diresult[3].str=sg;
  if (trumpf==5 && spwert==0) {
    strcpy(sg,textarr[TX_NIEMAND]);
  }
  else {
    prspnam(sg,spieler);
  }
  strcat(sg," ");
  strcat(sg,textarr[spgew?TX_GEWINNT:TX_VERLIERT]);
  diresult[4].str=sa;
  if (trumpf==-1) {
    strcpy(sa,textarr[TX_DAS_NULLSPIEL]);
  }
  else if (trumpf==5) {
    strcpy(sa,textarr[TX_DEN_RAMSCH]);
  }
  else {
    if (stich==1) {
      *sa=0;
    }
    else if ((spgew && schwz) || !nullv) {
      strcpy(sa,textarr[TX_DAS_SPIEL_SCHWARZ]);
    }
    else {
      sprintf(sa,textarr[TX_MIT_AUGEN],stsum,120-stsum);
    }
  }
  diresult[5].str=sw;
  sprintf(sw,"%s %d.",textarr[TX_DER_SPIELWERT_IST],
	  spgew && (trumpf!=5 || stsum==120)?spwert:-spwert);
  diresult[15].str=be?txt:OB_NONE;
  if (be>1) sprintf(txt,textarr[TX_BOCK_EREIGNISSE],be);
  else strcpy(txt,textarr[TX_BOCK_EREIGNIS]);
  for (sn=0;sn<numsp;sn++) {
    sprintf(su[sn][0],"%d",sum[0][alist[sn]]);
    sprintf(su[sn][1],"%d",sum[1][alist[sn]]);
    sprintf(su[sn][2],"%d",sum[2][alist[sn]]);
    diresult[12].str=su[sn][0];
    diresult[13].str=su[sn][1];
    diresult[14].str=su[sn][2];
    if (numsp>1) {
      set_names(dismlres[sn],5);
      dismlres[sn][1].str=diresult[2].str;
      dismlres[sn][2].str=diresult[3].str;
      dismlres[sn][3].str=diresult[4].str;
      dismlres[sn][4].str=diresult[5].str;
      dismlres[sn][11].str=diresult[12].str;
      dismlres[sn][12].str=diresult[13].str;
      dismlres[sn][13].str=diresult[14].str;
      dismlres[sn][14].str=diresult[15].str;
      if (!ini) {
	smlh=dismlres[sn][0].h;
	ini=1;
      }
      if (be) {
	dismlres[sn][14].spec&=~OB_HIDDEN;
	dismlres[sn][0].h=smlh+1;
      }
      else {
	dismlres[sn][14].spec|=OB_HIDDEN;
	dismlres[sn][0].h=smlh;
      }
      di_delres(sn);
      resdial[sn]=dismlres[sn];
      if (actdial[sn]) remove_di(sn);
      create_dial(sn,(desk[sn].w-resdial[sn][0].w*charw[sn])/2,
		  5,3,resdial[sn]);
      dismlres[sn][11].spec|=spieler==0?OB_SELECTED:OB_NONE;
      dismlres[sn][12].spec|=spieler==1?OB_SELECTED:OB_NONE;
      dismlres[sn][13].spec|=spieler==2?OB_SELECTED:OB_NONE;
      if (spgew &&
	  (alist[sn]==1 || (trumpf==5 && stsum!=120))) {
	dismlres[sn][11].spec^=OB_SELECTED;
	dismlres[sn][12].spec^=OB_SELECTED;
	dismlres[sn][13].spec^=OB_SELECTED;
      }
      if (trumpf==5 && spwert==0) {
	dismlres[sn][11].spec=dismlres[sn][12].spec=dismlres[sn][13].spec&=
	  ~OB_SELECTED;
      }
      phase=GEBEN;
    }
    else {
      set_names(diresult,6);
      create_di(sn,diresult);
      diresult[12].spec|=spieler==0?OB_SELECTED:OB_NONE;
      diresult[13].spec|=spieler==1?OB_SELECTED:OB_NONE;
      diresult[14].spec|=spieler==2?OB_SELECTED:OB_NONE;
      if (spgew &&
	  (alist[sn]==1 || (trumpf==5 && stsum!=120))) {
	diresult[12].spec^=OB_SELECTED;
	diresult[13].spec^=OB_SELECTED;
	diresult[14].spec^=OB_SELECTED;
      }
      if (trumpf==5 && spwert==0) {
	diresult[12].spec=diresult[13].spec=diresult[14].spec&=~OB_SELECTED;
      }
    }
  }
}

VOID di_delres(sn)
int sn;
{
  if (resdial[sn]) {
    remove_dial(sn,resdial[sn]);
    resdial[sn]=0;
  }
}

VOID di_options(sn)
int sn;
{
  static char txt[3][20];

  dioptions[sn][10].str=bockspiele?txt[sn]:OB_NONE;
  sprintf(txt[sn],"%s %d",textarr[TX_BOCK_SPIELE],bockspiele);
  dioptions[sn][18].spec=splstp||splres||
    (irc_play && (sum[0][0] || sum[0][1] || sum[0][2] ||
		  sum[1][0] || sum[1][1] || sum[1][2] ||
		  sum[2][0] || sum[2][1] || sum[2][2]))
      ?OB_EXIT:OB_HIDDEN;
  dioptions[sn][18].str=textarr[splres?TX_PROTOKOLL:TX_SPIELLISTE];
  dioptions[sn][17].spec=
    schenken && !schenkstufe && trumpf<=4 && stich==1 &&
      phase==SPIELEN && sn!=spieler && (ausspl+vmh)%3==sn
	?OB_EXIT:OB_HIDDEN;
  dioptions[sn][14].spec=
    dioptions[sn][15].spec=
      dioptions[sn][16].spec=
	dioptions[sn][17].spec==OB_EXIT?OB_HIDDEN:OB_NONE;
  create_di(sn,dioptions[sn]);
  dioptions[sn][5-sort1[sn]].spec|=OB_SELECTED;
  dioptions[sn][7-alternate[sn]].spec|=OB_SELECTED;
  dioptions[sn][9-sort2[sn]].spec|=OB_SELECTED;
}

VOID di_copyr(sn)
int sn;
{
  create_di(sn,dicopyr[sn]);
}

VOID di_strateg(sn)
int sn;
{
  static char nums[9][3],ini[3];
  int i,dis;

  if (!ini[sn]) {
    for (i=-4;i<=4;i++) {
      sprintf(nums[i+4],"%d",i);
      distrateg[sn][i+10].str=distrateg[sn][i+20].str=nums[i+4];
    }
    dis=irc_play?OB_DISABLED|OB_BUTTON:OB_BUTTON;
    for (i=0;i<9;i++) {
      distrateg[sn][i+6].spec=distrateg[sn][i+16].spec=dis;
    }
    if (numsp>1) {
      if (!sn) distrateg[sn][5].str=textarr[TX_RECHTS];
      for (i=0;i<10;i++) {
	distrateg[sn][i+15].spec=OB_HIDDEN;
      }
      for (i=25;i<29;i++) {
	distrateg[sn][i].y-=numsp>2?4:1;
      }
      distrateg[sn][0].h-=numsp>2?4:1;
      if (numsp>2) {
	for (i=2;i<15;i++) {
	  distrateg[sn][i].spec=OB_HIDDEN;
	}
	distrateg[sn][0].w-=10;
	distrateg[sn][1].w-=10;
	distrateg[sn][28].x-=5;
      }
    }
    ini[sn]=1;
  }
  create_di(sn,distrateg[sn]);
  distrateg[sn][strateg[0]+10].spec|=OB_SELECTED;
  distrateg[sn][strateg[1]+20].spec|=OB_SELECTED;
  distrateg[sn][hints[sn]+26].spec|=OB_SELECTED;
}

VOID di_varianten(sn)
int sn;
{
  divarianten[sn][0].next=irc_play?OB_DISABLED:OB_NONE;
  create_di(sn,divarianten[sn]);
  divarianten[sn][3+playramsch].spec|=OB_SELECTED;
  divarianten[sn][7+playsramsch].spec|=OB_SELECTED;
  divarianten[sn][10+playkontra].spec|=OB_SELECTED;
  divarianten[sn][14+playbock].spec|=OB_SELECTED;
  divarianten[sn][18+resumebock].spec|=OB_SELECTED;
  divarianten[sn][21+spitzezaehlt].spec|=OB_SELECTED;
  divarianten[sn][25+revolution].spec|=OB_SELECTED;
  divarianten[sn][28+klopfen].spec|=OB_SELECTED;
  divarianten[sn][31+schenken].spec|=OB_SELECTED;
}

VOID di_bockevents(sn)
int sn;
{
  int i,j;

  dibockevents[sn][0].next=irc_play?OB_DISABLED:OB_NONE;
  create_di(sn,dibockevents[sn]);
  for (i=1,j=0;i<=BOCK_BEI_LAST;i*=2,j++) {
    if (bockevents&i) {
      dibockevents[sn][2+j].spec|=OB_SELECTED;
    }
  }
}

VOID di_verzoegerung(sn)
int sn;
{
  static char txt[3][20];
  char *kl="<<<";
  char *gr=">>>";
  int i,dis;

  diverzoegerung[sn][3].str=txt[sn];
  prverz(sn);
  dis=irc_play && nimmstich[sn][0]>=101?OB_DISABLED|OB_EXIT:OB_EXIT;
  for (i=0;i<3;i++) {
    diverzoegerung[sn][4+i].str=kl+i;
    diverzoegerung[sn][4+i].spec=dis;
    diverzoegerung[sn][7+i].str=gr+2-i;
    diverzoegerung[sn][7+i].spec=dis;
  }
  create_di(sn,diverzoegerung[sn]);
  diverzoegerung[sn][12-fastdeal].spec|=OB_SELECTED;
}

VOID di_menubutton(sn)
int sn;
{
  static char txt[5][2];
  int i;

  create_di(sn,dimenubutton[sn]);
  dimenubutton[sn][2+mbutton[sn]].spec|=OB_SELECTED;
  for (i=0;i<5;i++) {
    sprintf(txt[i],"%d",i+1);
    dimenubutton[sn][3+i].str=txt[i];
  }
}

VOID di_wieder(sn)
int sn;
{
  create_di(sn,diwieder);
  diwieder[9+vorhandwn].spec|=OB_SELECTED;
}
