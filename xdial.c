
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

#define XDIAL_C

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include "defs.h"
#include "skat.h"
#include "ramsch.h"
#include "xio.h"
#include "irc.h"
#include "text.h"
#include "xdial.h"

#ifndef XK_KP_Tab
#define XK_KP_Tab           0xFF89
#endif
#ifndef XK_KP_Enter
#define XK_KP_Enter         0xFF8D
#endif
#ifndef XK_KP_Left
#define XK_KP_Left          0xFF96
#endif
#ifndef XK_KP_Up
#define XK_KP_Up            0xFF97
#endif
#ifndef XK_KP_Right
#define XK_KP_Right         0xFF98
#endif
#ifndef XK_KP_Down
#define XK_KP_Down          0xFF99
#endif
#ifndef XK_3270_BackTab
#define XK_3270_BackTab     0xFD05
#endif
#ifndef XK_ISO_Left_Tab
#define XK_ISO_Left_Tab     0xFE20
#endif
#ifndef hpXK_BackTab
#define hpXK_BackTab    0x1000FF74
#endif
#ifndef hpXK_KP_BackTab
#define hpXK_KP_BackTab 0x1000FF75
#endif
#ifndef osfXK_BackTab
#define osfXK_BackTab   0x1004FF07
#endif
#ifndef osfXK_Left
#define osfXK_Left      0x1004FF51
#endif
#ifndef osfXK_Up
#define osfXK_Up        0x1004FF52
#endif
#ifndef osfXK_Right
#define osfXK_Right     0x1004FF53
#endif
#ifndef osfXK_Down
#define osfXK_Down      0x1004FF54
#endif

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
  int ln,sn;
  static char txt[NUM_LANG][40];
  static tx_typ tt;

  for (ln=0;ln<NUM_LANG;ln++) tt.t[ln]=txt[ln];
  for (sn=0;sn<numsp;sn++) {
    if (reizp<0 || ramschspiele) {
      dioptions[sn][11].str=OB_NONE;
    }
    else {
      dioptions[sn][11].str=&tt;
      for (ln=0;ln<NUM_LANG;ln++) {
	sprintf(txt[ln],"%s %d",textarr[TX_GEREIZT_BIS_L].t[ln],reizw[reizp]);
      }
    }
  }
}

int trumpf_idx(sn,tr)
int sn,tr;
{
  return blatt[sn]>=2 && tr>=0 && tr<=3?
    TX_SCHELLEN+tr:TX_NULL+tr+1;
}

VOID info_spiel()
{
  int bl,ln,sn;
  static char txt[2][3][NUM_LANG][40];

  for (sn=0;sn<numsp;sn++) {
    dioptions[sn][12].str=&spielstr[0][sn];
    for (bl=0;bl<2;bl++) {
      for (ln=0;ln<NUM_LANG;ln++) {
	spielstr[bl][sn].t[ln]=txt[bl][sn][ln];
	sprintf(txt[bl][sn][ln],"%s %s",textarr[TX_GESPIELT_WIRD].t[ln],
		textarr[revolang?TX_REVOLUTION:
		       bl && trumpf>=0 && trumpf<=3?
		       TX_SCHELLEN+trumpf:TX_NULL+trumpf+1].t[ln]);
      }
    }
  }
}

VOID info_stich(p,c)
int p,c;
{
  int bl,ln,sn;
  static char txt[2][3][3][NUM_LANG][20];

  for (sn=0;sn<numsp;sn++) {
    for (bl=0;bl<2;bl++) {
      dioptions[sn][14+p].str=&stichstr[0][sn][p];
      for (ln=0;ln<NUM_LANG;ln++) {
	stichstr[bl][sn][p].t[ln]=txt[bl][sn][p][ln];
	strcpy(txt[bl][sn][p][ln],
	       textarr[(bl?TX_SCHELLEN:TX_KARO)+(c>>3)].t[ln]);
	strcat(txt[bl][sn][p][ln]," ");
	strcat(txt[bl][sn][p][ln],
	       textarr[(bl?TX_ASD:TX_AS)+(c&7)].t[ln]);
      }
    }
  }
}

VOID clear_info()
{
  int sn;

  for (sn=0;sn<numsp;sn++) {
    dioptions[sn][11].str=&textarr[TX_GEREIZT_BIS_L];
    dioptions[sn][12].str=&textarr[TX_GESPIELT_WIRD];
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
      ob[idx+z*3+s].str=&tspnames[s][z];
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
      ob[i].str=&textarr[(int)ob[i].str-OB_NONE-1];
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
  INIT_DI(dicopyralt);
  INIT_DI(digrafik);
  INIT_DI(distrateg);
  INIT_DI(divarianten);
  INIT_DI(diramschopts);
  INIT_DI(dibockevents);
  INIT_DI(digeschwindigkeit);
  INIT_DI(digui);
  INIT_DI(diweiter);
  INIT_DI(dikontra);
  INIT_DI(dikonre);
  INIT_DI(diinput);
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
  init_di(direkontra);
  init_di(didicht);
  init_di(diresult);
  init_di(diwiederweiter);
  init_di(diwieder);
  init_di(dimehrspieler);
  init_di(dilanspiel);
  init_di(dieigenertisch);
  init_di(dianderertisch);
  init_di(diwarteauf);
  init_di(diirc);
}

VOID prverz(sn)
int sn;
{
  int ln,n;
  char *gr="> ";
  char *em="";

  n=nimmstich[sn][0];
  for (ln=0;ln<NUM_LANG;ln++) {
    if (n>=101) sprintf(digeschwindigkeit[sn][3].str->t[ln],"%s",
			textarr[TX_MAUS_KLICK].t[ln]);
    else {
      sprintf(digeschwindigkeit[sn][3].str->t[ln],"%s%d.%d %s",
	      n<maxnimm()?gr:em,n/10,n%10,textarr[TX_SEKUNDEN].t[ln]);
    }
  }
}

int ob_disabled(ob,i)
OBJECT *ob;
int i;
{
  return ob[i].spec&OB_DISABLED ||
    (ob[0].next&OB_DISABLED && !(ob[i].spec&OB_EXIT));
}

VOID cleanip(sn)
int sn;
{
  int i;

  if (!inputbuf[sn][0]) {
    strcpy(inputbuf[sn],"127.0.0.1");
  }
  else {
    i=0;
    while (inputbuf[sn][i]) {
      if (inputbuf[sn][i]==' ') inputbuf[sn][i]='.';
      i++;
    }
  }
}

VOID cleanirchost(sn)
int sn;
{
  int i;

  if (!inputbuf[sn][0]) {
    strncpy(inputbuf[sn],irc_host,35);
    inputbuf[sn][35]=0;
  }
  i=0;
  while (inputbuf[sn][i]) {
    if (inputbuf[sn][i]==' ') inputbuf[sn][i]='.';
    i++;
  }
}

VOID cleanname(sn)
int sn;
{
  int i;

  i=0;
  while (inputbuf[sn][i]) {
    if (inputbuf[sn][i]==' ') inputbuf[sn][i]='_';
    i++;
  }
}

VOID hndl_btevent(sn,bt)
int sn,bt;
{
  int i,j,c,ag,s1,s2,al,stg[2],sav,bb;
  OBJECT *ob;
  static int ircagain;

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
      else {
	if (ircagain) {
	  startirc(0);
	}
	if (phase==RESULT) di_resultdi(sn);
      }
    }
    else if (ob==diterm[sn]) {
      remove_di(sn);
      if (irc_play) {
	ircagain=1;
	di_ende(sn);
      }
      else {
	lost[sn]=1;
	XUnmapWindow(dpy[sn],win[sn]);
	XFlush(dpy[sn]);
      }
    }
    else if (ob==diloesch) {
      remove_di(sn);
      if (bt==2) {
	setsum(1);
	if (irc_play) irc_sendloeschen(sn);
	save_list();
	if (phase==RESULT) di_resultdi(sn);
      }
      else di_liste(sn,1);
    }
    else if (ob==dispiel) {
      remove_di(sn);
      if (bt==14) {
	cards[30]=prot2.skat[1][0];
	cards[31]=prot2.skat[1][1];
	umdrueck=1;
	do_druecken();
	return;
      }
      ag=0;
      if (dispiel[6].spec&OB_SELECTED ||
	  dispiel[13].spec&OB_SELECTED) trumpf=-1;
      else if (dispiel[2].spec&OB_SELECTED) trumpf=0;
      else if (dispiel[3].spec&OB_SELECTED) trumpf=1;
      else if (dispiel[4].spec&OB_SELECTED) trumpf=2;
      else if (dispiel[5].spec&OB_SELECTED) trumpf=3;
      else trumpf=4;
      if (!handsp && trumpf!=-1 &&
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
	  dispitze[3].str=&textarr[spitzeang?TX_SPITZE_F3:TX_SPITZE_F2];
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
    else if (ob==dikontra[sn]) {
      remove_di(sn);
      di_ktrnext(sn,bt==3);
    }
    else if (ob==direkontra) {
      remove_di(sn);
      di_ktrnext(sn,bt==3);
    }
    else if (ob==dikonre[sn]) {
      remove_di(sn);
      if (ktrnext>=0) {
	di_konre(ktrnext);
	ktrnext=-1;
      }
      else {
	do_angesagt();
      }
    }
    else if (ob==diinput[sn]) {
      if (bt==3) {
	inputbuf[sn][0]='_';
	inputbuf[sn][1]=0;
	diinput[sn][3].spec&=~OB_SELECTED;
	if (actbtn[sn]!=4) {
	  actbtn[sn]=4;
	  draw_di(sn,4);
	}
	draw_di(sn,3);
	draw_di(sn,2);
      }
      else {
	inputbuf[sn][strlen(inputbuf[sn])-1]=0;
	switch (inputdi[sn]) {
	case 0:
	  cleanip(sn);
	  save_opt();
	  di_eigenertisch(sn);
	  break;
	case 1:
	  cleanip(sn);
	  save_opt();
	  di_anderertisch(sn);
	  break;
	case 2:
	  cleanirchost(sn);
	  save_opt();
	  di_irc(sn);
	  break;
	case 3:
	  cleanname(sn);
	  set_conames();
	  save_opt();
	  di_strateg(sn);
	  break;
	case 4:
	  cleanname(sn);
	  set_conames();
	  save_opt();
	  di_grafik(sn);
	  break;
	}
      }
    }
    else if (ob==didicht) {
      remove_di(sn);
      if (didicht[3].spec&OB_SELECTED ||
	  didicht[5].spec&OB_SELECTED) {
	spielendscr();
	if (didicht[5].spec&OB_SELECTED) {
	  abkuerz[sn]=2;
	  save_opt();
	}
      }
      else if (didicht[4].spec&OB_SELECTED ||
	       didicht[6].spec&OB_SELECTED) {
	ndichtw=1;
	phase=SPIELEN;
	if (didicht[6].spec&OB_SELECTED) {
	  abkuerz[sn]=0;
	  save_opt();
	}
      }
    }
    else if (ob==diweiter[sn]) {
      remove_di(sn);
      di_weiter(0);
    }
    else if (ob==diwiederweiter) {
      remove_di(sn);
      clr_desk(0);
      skatopen=0;
      if (bt==1) di_wieder(sn,0);
      else phase=GEBEN;
    }
    else if (ob==dischieben) {
      remove_di(sn);
      if (bt==2) {
	draw_skat(spieler);
	put_fbox(spieler,TX_FERTIG);
	drbut=spieler+1;
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
	  &textarr[schwang?TX_SCHW_ANGE:TX_SCHN_ANGE];
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
      clr_desk(0);
      if (bt==20) di_ende(sn);
      else if (bt==22) di_proto(sn,1,0);
      else if (bt==21) di_wieder(sn,1);
      else phase=GEBEN;
    }
    else if (ob==diwieder) {
      remove_di(sn);
      vorhandwn=diwieder[10].spec&OB_SELECTED?1:0;
      if (bt==11) {
	di_resultdi(sn);
      }
      else {
	if (diwieder[5].spec&OB_SELECTED) {
	  wieder=1;
	}
	else if (diwieder[6].spec&OB_SELECTED) {
	  wieder=2;
	}
	else {
	  wieder=3;
	}
	phase=GEBEN;
      }
    }
    else if (ob==diproto[sn]) {
      remove_di(sn);
      if (bt==49) {
	protsort[sn]^=1;
	di_proto(sn,0,0);
	actbtn[sn]=49;
      }
      else if (bt==47) {
	if ((sn && !irc_play) || !protsort[sn]) di_liste(sn,1);
	else {
	  di_proto(sn,1,1);
	  if (phase==RESULT) di_resultdi(sn);
	}
      }
      else if (phase==RESULT) di_resultdi(sn);
    }
    else if (ob==diliste[sn]) {
      remove_di(sn);
      if (bt==62) {
	splfirst[sn]-=12;
	di_liste(sn,0);
	actbtn[sn]=splfirst[sn]?62:65;
      }
      else if (bt==63) {
	alist[sn]=(alist[sn]+1)%3;
	if (!sn) save_opt();
	di_liste(sn,splfirst[sn]+12>=splstp);
	actbtn[sn]=63;
      }
      else if (bt==65) {
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
	  actbtn[sn]=splfirst[sn]+12<splstp?65:62;
	}
      }
      else if (phase==RESULT) di_resultdi(sn);
    }
    else if (ob==dioptions[sn]) {
      remove_di(sn);
      s1=dioptions[sn][4].spec&OB_SELECTED?1:0;
      al=dioptions[sn][6].spec&OB_SELECTED?1:0;
      s2=dioptions[sn][8].spec&OB_SELECTED?1:0;
      sav=sort1[sn]+2*alternate[sn];
      if (s1!=sort1[sn] || s2!=sort2[sn] || al!=alternate[sn]) {
	sort1[sn]=s1;
	sort2[sn]=s2;
	alternate[sn]=al;
	if (irc_play) irc_sendsort(sn);
	initscr(sn,1);
	if (sav!=sort1[sn]+2*alternate[sn] && !irc_play) save_opt();
      }
      if (bt==21) di_ende(sn);
      else if (bt==19 && splres) di_proto(sn,1,0);
      else if (bt==19) di_liste(sn,1);
      else if (bt==20) di_grafik(sn);
      else if (bt==17) {
	if (irc_play) irc_sendschenken(sn);
	di_schenken(sn);
      }
      else if (bt==18) di_mehrspieler(sn);
      else if (bt==2) di_copyr(sn);
    }
    else if (ob==dicopyr[sn] || ob==dicopyralt[sn]) {
      remove_di(sn);
      if (dlhintseen!=2 || firstgame) {
	di_options(!sn && firstgame?-1:sn);
      }
      if (!sn) dlhintseen=1;
    }
    else if (ob==digrafik[sn]) {
      sav=blatt[sn]+4*lang[sn];
      ag=0;
      if (!(digrafik[sn][8+lang[sn]].spec&OB_SELECTED)) {
	remove_di(sn);
	lang[sn]^=1;
	if (phase==REIZEN) {
	  lastmsaho[sn]=0;
	  if (saho) do_msagen(sager,reizw[reizp]);
	  else do_mhoeren(hoerer);
	}
	if (drbut==sn+1) {
	  put_fbox(sn,TX_DRUECKEN);
	}
	ag=1;
      }
      if (!(digrafik[sn][3+blatt[sn]].spec&OB_SELECTED) ||
	  (sav/4!=lang[sn] && blatt[sn]<2)) {
	if (!ag) remove_di(sn);
	blatt[sn]=(digrafik[sn][3].spec&OB_SELECTED?0:
		   digrafik[sn][4].spec&OB_SELECTED?1:
		   digrafik[sn][5].spec&OB_SELECTED?2:3);
	for (i=0;i<33;i++) {
	  create_card(sn,i-1);
	}
	ag=1;
      }
      if (ag) {
	initscr(sn,2);
	setcurs(sn+1);
	if (resdial[sn]) {
	  XUnmapWindow(dpy[sn],resdial[sn][0].win);
	  XMapWindow(dpy[sn],resdial[sn][0].win);
	}
	set_conames();
	if (sav!=blatt[sn]+4*lang[sn] && !irc_play) save_opt();
	di_grafik(sn);
      }
      if (bt==11) {
	di_input(sn,TX_NAME,4,usrname[0],9);
      }
      else if (bt==12) {
	di_input(sn,TX_NAME,4,usrname[1],9);
      }
      else if (bt==13) {
	set_conames();
	if (sav!=blatt[sn]+4*lang[sn] && !irc_play) save_opt();
	di_strateg(sn);
      }
    }
    else if (ob==distrateg[sn]) {
      remove_di(sn);
      for (i=0;i<2;i++) stg[i]=strateg[i];
      for (i=-4;i<=0;i++) {
	if (distrateg[sn][i+8].spec&OB_SELECTED) {
	  stg[0]=i;
	}
	if (distrateg[sn][i+14].spec&OB_SELECTED) {
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
      if (distrateg[sn][16].spec&OB_SELECTED) {
	if (hints[sn]) {
	  hints[sn]=0;
	  if (hintcard[0]!=-1) {
	    if (phase==SPIELEN && sn==(ausspl+vmh)%3) {
	      show_hint(sn,0,0);
	    }
	    else if (phase==DRUECKEN && sn==spieler) {
	      show_hint(sn,0,0);
	      show_hint(sn,1,0);
	    }
	  }
	  sav=1;
	}
      }
      else {
	if (!hints[sn]) {
	  hints[sn]=1;
	  if (hintcard[0]!=-1) {
	    if (phase==SPIELEN && sn==(ausspl+vmh)%3) {
	      show_hint(sn,0,1);
	    }
	    else if (phase==DRUECKEN && sn==spieler) {
	      show_hint(sn,0,1);
	      show_hint(sn,1,1);
	    }
	  }
	  sav=1;
	}
      }
      if (bt>=19 && bt<=22) {
	di_input(sn,TX_NAME,3,conames[1-(bt&1)][(bt-19)/2],9);
      }
      else {
	set_conames();
	if (sav && !irc_play) save_opt();
	di_varianten(sn);
      }
    }
    else if (ob==divarianten[sn]) {
      remove_di(sn);
      sav=(((((((playramsch
		 )*3+playkontra
		)*3+playbock
	       )*3+spitzezaehlt
	      )*2+revolution
	     )*2+klopfen
	    )*2+schenken
	   )*2+oldrules;
      for (i=0;i<3;i++) {
	if (divarianten[sn][3+i].spec&OB_SELECTED) {
	  playramsch=i;
	}
	if (divarianten[sn][7+i].spec&OB_SELECTED) {
	  playkontra=i;
	}
	if (divarianten[sn][11+i].spec&OB_SELECTED) {
	  playbock=i;
	}
	if (divarianten[sn][15+i].spec&OB_SELECTED) {
	  spitzezaehlt=i;
	}
      }
      revolution=!(divarianten[sn][19].spec&OB_SELECTED);
      klopfen=!(divarianten[sn][22].spec&OB_SELECTED);
      schenken=!(divarianten[sn][25].spec&OB_SELECTED);
      oldrules=!(divarianten[sn][28].spec&OB_SELECTED);
      if (sav!=(((((((playramsch
		      )*3+playkontra
		     )*3+playbock
		    )*3+spitzezaehlt
		   )*2+revolution
		  )*2+klopfen
		 )*2+schenken
		)*2+oldrules) {
	save_opt();
      }
      if (playramsch) di_ramschopts(sn);
      else if (playbock) di_bockevents(sn);
      else di_geschwindigkeit(sn);
    }
    else if (ob==diramschopts[sn]) {
      remove_di(sn);
      sav=playsramsch*2+rskatloser;
      playsramsch=!(diramschopts[sn][3].spec&OB_SELECTED);
      rskatloser=!(diramschopts[sn][6].spec&OB_SELECTED);
      if (sav!=playsramsch*2+rskatloser) save_opt();
      if (playbock) di_bockevents(sn);
      else di_geschwindigkeit(sn);
    }
    else if (ob==dibockevents[sn]) {
      remove_di(sn);
      sav=bockevents*2+resumebock;
      bockevents=0;
      for (i=1,j=0;i<=BOCK_BEI_LAST;i*=2,j++) {
	if (dibockevents[sn][2+j].spec&OB_SELECTED) {
	  bockevents+=i;
	}
      }
      resumebock=!(dibockevents[sn][11].spec&OB_SELECTED);
      if (sav!=bockevents*2+resumebock) save_opt();
      di_geschwindigkeit(sn);
    }
    else if (ob==digeschwindigkeit[sn]) {
      sav=(nimmstich[sn][0]*3+abkuerz[sn])*2+fastdeal;
      digeschwindigkeit[sn][bt].spec&=~OB_SELECTED;
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
      fastdeal=digeschwindigkeit[sn][11].spec&OB_SELECTED?1:0;
      abkuerz[sn]=(digeschwindigkeit[sn][14].spec&OB_SELECTED?0:
		   digeschwindigkeit[sn][15].spec&OB_SELECTED?1:2);
      if (sav!=(nimmstich[sn][0]*3+abkuerz[sn])*2+fastdeal &&
	  !irc_play) save_opt();
      prverz(sn);
      if (bt==17) {
	remove_di(sn);
	if (!irc_play && numsp==1) di_mehrspieler(sn);
	else di_eingabe(sn);
      }
      else {
	draw_di(sn,3);
	draw_di(sn,bt);
      }
    }
    else if (ob==dimehrspieler) {
      if (bt==2) di_lanspiel(sn);
      else if (bt==3) di_irc(sn);
      else if (bt==4) {
	manpage("xskat");
	dimehrspieler[bt].spec&=~OB_SELECTED;
	draw_di(sn,bt);
      }
      else di_eingabe(sn);
    }
    else if (ob==dilanspiel) {
      if (bt==2) di_eigenertisch(sn);
      else if (bt==3) di_anderertisch(sn);
      else di_mehrspieler(sn);
    }
    else if (ob==dieigenertisch) {
      sav=laninvite[0]*2+laninvite[1];
      laninvite[0]=!(dieigenertisch[4].spec&OB_SELECTED);
      laninvite[1]=!(dieigenertisch[9].spec&OB_SELECTED);
      if (sav!=laninvite[0]*2+laninvite[1]) save_opt();
      if (bt==13) di_lanspiel(sn);
      else if (!laninvite[0] && !laninvite[1]) {
	di_eigenertisch(sn);
      }
      else if (bt==6) {
	di_input(sn,TX_RECHNER_IP,0,lanip[1],35);
      }
      else if (bt==11) {
	di_input(sn,TX_RECHNER_IP,0,lanip[2],35);
      }
      else di_warteauf(sn,0,0,0);
    }
    else if (ob==dianderertisch) {
      if (bt==4) {
	di_input(sn,TX_RECHNER_IP,1,lanip[0],35);
      }
      else if (bt==7) {
	manpage("xhost");
	dianderertisch[bt].spec&=~OB_SELECTED;
	draw_di(sn,bt);
      }
      else if (bt==9) {
	xstoreres();
	exitus(0);
      }
      else di_lanspiel(sn);
    }
    else if (ob==diwarteauf) {
      di_eigenertisch(sn);
    }
    else if (ob==diirc) {
      if (bt==7) di_mehrspieler(sn);
      else if (bt==3) {
	di_input(sn,TX_RECHNER_IP,2,irc_hostname,35);
      }
      else if (bt==6) {
	manpage("xskat");
	diirc[bt].spec&=~OB_SELECTED;
	draw_di(sn,bt);
      }
      else {
	if (!fork()) startirc(1);
	exitus(0);
      }
    }
    else if (ob==digui[sn]) {
      remove_di(sn);
      sav=(((mbutton[sn]
	     )*3+keyboard[sn]
	    )*2+briefmsg[sn]
	   )*2+trickl2r[sn];
      for (i=0;i<=5;i++) {
	if (digui[sn][3+i].spec&OB_SELECTED) {
	  mbutton[sn]=i;
	}
      }
      for (i=0;i<3;i++) {
	if (digui[sn][10+i].spec&OB_SELECTED) {
	  keyboard[sn]=i;
	}
      }
      briefmsg[sn]=!(digui[sn][14].spec&OB_SELECTED);
      trickl2r[sn]=!(digui[sn][17].spec&OB_SELECTED);
      if (sav!=(((mbutton[sn]
		  )*3+keyboard[sn]
		 )*2+briefmsg[sn]
		)*2+trickl2r[sn] && !irc_play) save_opt();
    }
  }
}

VOID button_press(sn,bt,ob)
int sn,bt;
OBJECT *ob;
{
  if (irc_play &&
      ob!=diende[sn] &&
      ob!=diterm[sn] &&
      ob!=diproto[sn] &&
      ob!=diliste[sn] &&
      ob!=diloesch &&
      ob!=dioptions[sn] &&
      ob!=dicopyr[sn] &&
      ob!=dicopyralt[sn] &&
      ob!=digrafik[sn] &&
      ob!=distrateg[sn] &&
      ob!=divarianten[sn] &&
      ob!=diramschopts[sn] &&
      ob!=dibockevents[sn] &&
      ob!=digeschwindigkeit[sn] &&
      ob!=digui[sn]) irc_sendbtev(sn,bt);
  hndl_btevent(sn,bt);
}

VOID draw_wedge(w,bp,sn,f,x,y,s)
Drawable w;
unsigned long bp;
int sn,f,x,y,s;
{
  int i,a,b,c;

  a=s&1?-1:1;
  b=s&2?-1:1;
  c=s&4?desk[sn].large?20:13:1;
  change_gc(sn,f?mkpix[sn]:bp,gc);
  for (i=0;i<=6;i++) {
    XDrawPoint(dpy[sn],w,gc[sn],x,y+b*(2*i));
    XDrawPoint(dpy[sn],w,gc[sn],x+a*1,y+b*(2*i+1));
  }
  for (i=0;i<=c;i++) {
    XDrawPoint(dpy[sn],w,gc[sn],x+a*(2*i+2),y);
    XDrawPoint(dpy[sn],w,gc[sn],x+a*(2*i+1),y+b*1);
  }
  change_gc(sn,f?wpix[sn]:bp,gc);
  for (i=0;i<=6;i++) {
    XDrawPoint(dpy[sn],w,gc[sn],x,y+b*(2*i+1));
    XDrawPoint(dpy[sn],w,gc[sn],x+a*1,y+b*(2*i));
  }
  for (i=0;i<=c;i++) {
    XDrawPoint(dpy[sn],w,gc[sn],x+a*(2*i+1),y);
    XDrawPoint(dpy[sn],w,gc[sn],x+a*(2*i+2),y+b*1);
  }
  change_gc(sn,fgpix[sn],gc);
}

VOID draw_actbtn(sn,f)
int sn,f;
{
  OBJECT *ob;
  unsigned long bp;
  int bt,d,e,w,h;

  if (f && !keyboard[sn]) actbtn[sn]=0;
  ob=actdial[sn];
  bt=actbtn[sn];
  if (bt && ob) {
    bp=gfx3d[sn] || !(ob[bt].spec&OB_SELECTED)?btpix[sn]:fgpix[sn];
    d=gfx3d[sn]?ob[bt].spec&OB_EXIT?2:1:0;
    e=gfx3d[sn]?ob[bt].spec&OB_EXIT?1:2:2;
    w=ob[bt].w*charw[sn];
    h=ob[bt].h*charh[sn];
    draw_wedge(ob[bt].win,bp,sn,f,d,d,0);
    draw_wedge(ob[bt].win,bp,sn,f,w-e,d,1);
    draw_wedge(ob[bt].win,bp,sn,f,d,h-e,2);
    draw_wedge(ob[bt].win,bp,sn,f,w-e,h-e,3);
  }
}

VOID draw_selpos(sn,f)
int sn,f;
{
  unsigned long bp;
  int p,s;

  if (selpos[sn].num) {
    p=selpos[sn].act;
    if (selpos[sn].p[p].f&1) {
      bp=btpix[sn];
      s=0;
    }
    else if (selpos[sn].p[p].f&2) {
      bp=bgpix[sn];
      s=4;
    }
    else {
      return;
    }
    draw_wedge(win[sn],bp,sn,f,selpos[sn].p[p].x1,selpos[sn].p[p].y1,s);
    draw_wedge(win[sn],bp,sn,f,selpos[sn].p[p].x2,selpos[sn].p[p].y1,s+1);
    draw_wedge(win[sn],bp,sn,f,selpos[sn].p[p].x1,selpos[sn].p[p].y2,s+2);
    draw_wedge(win[sn],bp,sn,f,selpos[sn].p[p].x2,selpos[sn].p[p].y2,s+3);
    draw_wedge(bck[sn],bp,sn,f,selpos[sn].p[p].x1,selpos[sn].p[p].y1,s);
    draw_wedge(bck[sn],bp,sn,f,selpos[sn].p[p].x2,selpos[sn].p[p].y1,s+1);
    draw_wedge(bck[sn],bp,sn,f,selpos[sn].p[p].x1,selpos[sn].p[p].y2,s+2);
    draw_wedge(bck[sn],bp,sn,f,selpos[sn].p[p].x2,selpos[sn].p[p].y2,s+3);
  }
}

VOID new_selpos(sn,dir)
int sn,dir;
{
  draw_selpos(sn,0);
  selpos[sn].act=(selpos[sn].act+dir+selpos[sn].num)%selpos[sn].num;
  draw_selpos(sn,1);
}

VOID new_actbtn(sn,dir)
int sn,dir;
{
  OBJECT *ob;
  int bt;

  if (selpos[sn].num) {
    new_selpos(sn,dir);
    return;
  }
  draw_actbtn(sn,0);
  ob=actdial[sn];
  bt=actbtn[sn];
  if (bt && ob) {
    if (!dir) {
      if (ob[bt].next!=OB_NONE) {
	actbtn[sn]+=ob[bt].next;
      }
      if (ob==dispiel) {
	actbtn[sn]=12;
      }
    }
    else {
      for (;;) {
	bt+=dir;
	if (bt==ob[0].spec) bt=1;
	else if (!bt) bt=ob[0].spec-1;
	if (ob[bt].spec&(OB_BUTTON|OB_EXIT) &&
	    !ob_disabled(ob,bt) &&
	    !(ob[bt].spec&OB_HIDDEN) &&
	    (!(ob[bt].spec&OB_SELECTED) ||
	     ob[bt].next==OB_NONE)) {
	  actbtn[sn]=bt;
	  break;
	}
      }
    }
  }
  draw_actbtn(sn,1);
}

VOID set_selpos(sn)
int sn;
{
  int i,j,k,n;
  static int tc[3],ph[3];
  static long gn[3];

  if (selpos[sn].num || actdial[sn] || !keyboard[sn]) return;
  if (selpos[sn].act==-1 || !(selpos[sn].p[selpos[sn].act].f&4) ||
      gn[sn]!=gamenr || ph[sn]!=phase) {
    selpos[sn].act=0;
    n=1;
  }
  else n=0;
  if (phase==REIZEN) {
    if ((saho && sn==sager) || (!saho && sn==hoerer)) {
      selpos[sn].num=2;
      for (i=0;i<2;i++) {
	selpos[sn].p[i].x1=desk[sn].pboxx+i*desk[sn].cardw+4;
	selpos[sn].p[i].y1=desk[sn].pboxy+1;
	selpos[sn].p[i].x2=desk[sn].pboxx+(i+1)*desk[sn].cardw-5;
	selpos[sn].p[i].y2=desk[sn].pboxy+charh[sn]-gfx3d[sn]-1;
	selpos[sn].p[i].f=1;
      }
    }
  }
  else if (phase==DRUECKEN) {
    if (sn==spieler) {
      if (n) {
	selpos[sn].act=6;
      }
      if (hints[sn] && hintcard[0]!=-1) {
	selpos[sn].act=0;
	if (hintcard[0]<30) selpos[sn].act=hintcard[0]-10*sn+1;
	if (hintcard[1]<30) selpos[sn].act=hintcard[1]-10*sn+1;
      }
      selpos[sn].num=11;
      i=0;
      selpos[sn].p[i].x1=desk[sn].pboxx+24*desk[sn].f/desk[sn].q+4;
      selpos[sn].p[i].y1=desk[sn].pboxy+1;
      selpos[sn].p[i].x2=desk[sn].pboxx+104*desk[sn].f/desk[sn].q-5;
      selpos[sn].p[i].y2=desk[sn].pboxy+charh[sn]-gfx3d[sn]-1;
      selpos[sn].p[i].f=1;
      for (i=1;i<11;i++) {
	selpos[sn].p[i].x1=desk[sn].playx+(i-1)*desk[sn].cardx;
	selpos[sn].p[i].y1=desk[sn].playy-2;
	selpos[sn].p[i].x2=desk[sn].playx+i*desk[sn].cardx-1-desk[sn].large*2;
	selpos[sn].p[i].y2=desk[sn].playy+desk[sn].cardh+1-desk[sn].large;
	selpos[sn].p[i].f=2|4;
      }
    }
  }
  else if (phase==SPIELEN) {
    if (sn==(ausspl+vmh)%3) {
      computer();
      if (actdial[sn] || keyboard[sn]!=2) return;
      calc_poss(sn);
      selpos[sn].num=possc;
      i=0;
      for (k=0;k<10;k++) {
	for (j=0;j<possc;j++) {
	  if (possi[j]%10==k) {
	    if (hints[sn] && hintcard[0]==possi[j]) {
	      selpos[sn].act=i;
	    }
	    selpos[sn].p[i].x1=desk[sn].playx+k*desk[sn].cardx;
	    selpos[sn].p[i].y1=desk[sn].playy-2;
	    selpos[sn].p[i].x2=desk[sn].playx+(k+1)*desk[sn].cardx-
	      1-desk[sn].large*2;
	    selpos[sn].p[i].y2=desk[sn].playy+desk[sn].cardh+1-desk[sn].large;
	    selpos[sn].p[i].f=2;
	    i++;
	    break;
	  }
	}
      }
    }
  }
  else if (phase==NIMMSTICH) {
    if (nimmstich[sn][1]) {
      selpos[sn].num=1;
      selpos[sn].p[0].f=0;
      selpos[sn].p[0].x1=0;
      selpos[sn].p[0].y1=0;
    }
  }
  else if (phase==REVOLUTION) {
    if (sn==tauschply) {
      if (tauschcard!=-1) {
	j=tauschcard%10;
	if (tauschcard/10!=sn) j+=10;
      }
      else j=-1;
      if (!n) {
	if (tc[sn]!=-1 && selpos[sn].act>=tc[sn]+1) {
	  selpos[sn].act++;
	}
	if (j!=-1) {
	  if (selpos[sn].act>j+1) {
	    selpos[sn].act--;
	  }
	  else if (selpos[sn].act==j+1) {
	    selpos[sn].act=j%10*2+10-j+(j/10);
	  }
	}
      }
      selpos[sn].num=20+(j==-1);
      i=0;
      selpos[sn].p[i].x1=desk[sn].pboxx+24*desk[sn].f/desk[sn].q+4;
      selpos[sn].p[i].y1=desk[sn].pboxy+1;
      selpos[sn].p[i].x2=desk[sn].pboxx+104*desk[sn].f/desk[sn].q-5;
      selpos[sn].p[i].y2=desk[sn].pboxy+charh[sn]-gfx3d[sn]-1;
      selpos[sn].p[i].f=1;
      i++;
      for (k=0;k<20;k++) {
	if (k==j) continue;
	selpos[sn].p[i].x1=desk[sn].playx+k%10*desk[sn].cardx;
	selpos[sn].p[i].y1=(k>9?desk[sn].skaty:desk[sn].playy)-2;
	selpos[sn].p[i].x2=desk[sn].playx+(k%10+1)*desk[sn].cardx-
	  1-desk[sn].large*2;
	selpos[sn].p[i].y2=(k>9?desk[sn].skaty:desk[sn].playy)+
	  desk[sn].cardh+1-desk[sn].large;
	selpos[sn].p[i].f=2|4;
	i++;
      }
      tc[sn]=j;
    }
  }
  draw_selpos(sn,1);
  gn[sn]=gamenr;
  ph[sn]=phase;
}

VOID del_selpos(sn)
int sn;
{
  draw_selpos(sn,0);
  selpos[sn].num=0;
}

VOID manpage(subj)
char *subj;
{
  if (!fork()) {
    execlp("xterm","xterm","-e","man",subj,NULL);
    fprintf(stderr,"xterm not found\n");
    exitus(0);
  }
}

VOID polldisps()
{
  static int tim;
  Display *d;
  char dn[2][80];
  int i,ok,s[2];
  char *argv[100];

  if (++tim>20) tim=0;
  if (tim) return;
  ok=1;
  for (i=0;i<2;i++) {
    if ((s[i]=laninvite[i])) {
      strcpy(dn[i],lanip[i+1]);
      if (!strchr(dn[i],':')) {
	strcat(dn[i],":0");
      }
      if ((d=XOpenDisplay(dn[i]))) {
	if (!XGetDefault(d,prog_name,"ready")) ok=0;
	else s[i]=0;
	XCloseDisplay(d);
      }
      else ok=0;
    }
  }
  if (ok) {
    for (i=0;i<90 && i<theargc;i++) {
      argv[i]=theargv[i];
    }
    if (laninvite[0]) argv[i++]=dn[0];
    if (laninvite[1]) argv[i++]=dn[1];
    argv[i]=0;
    execvp(argv[0],argv);
    fprintf(stderr,"%s not found\n",argv[0]);
    exitus(0);
  }
  di_warteauf(0,1,s[0],s[1]);
}

VOID hndl_events()
{
  int sn,b,x,y,i,opt,bt;
  XEvent event;
  KeySym keysym;
  char buf[100],*l;
  OBJECT *ob;
  static int sl,sr;

  if (!lost[0] && !lost[1] && !lost[2]) setcurs(0);
  waitt(50,1);
  for (sn=0;sn<numsp;sn++) {
    if (irc_play && irc_pos!=sn) continue;
    b=x=y=opt=0;
    set_selpos(sn);
    while (!lost[sn] && XCheckMaskEvent(dpy[sn],~0,&event)) {
      ob=actdial[sn];
      switch (event.type) {
      case KeyRelease:
	keysym=XLookupKeysym((XKeyEvent *)&event,0);
	if (keysym==XK_Shift_L) sl=0;
	else if (keysym==XK_Shift_R) sr=0;
	break;
      case KeyPress:
	i=XLookupString((XKeyEvent *)&event,buf,sizeof(buf)-1,
			&keysym,(XComposeStatus *)0);
	if (keysym==XK_Shift_L) sl=1;
	else if (keysym==XK_Shift_R) sr=1;
	if (keysym==XK_F6 && pkoption!=1) {
	  if (!pkoption) pkoption=2;
	  else if (pkoption!=4) pkoption++;
	}
	if (!ob &&
	    (keysym==XK_Escape ||
	     keysym==XK_F1)) {
	  b=sn+1;
	  opt=2;
	}
	else if (i && irc_state==IRC_PLAYING && irc_xinput(buf,i));
	else if (ob || selpos[sn].num) {
	  if (ob==diinput[sn] &&
	      ((i==1 && (unsigned int)buf[0]>=' ') ||
	       keysym==XK_BackSpace || keysym==XK_Delete)) {
	    buf[1]=0;
	    if (keysym==XK_BackSpace || keysym==XK_Delete) {
	      if (strlen(inputbuf[sn])>1) {
		inputbuf[sn][strlen(inputbuf[sn])-2]='_';
		inputbuf[sn][strlen(inputbuf[sn])-1]=0;
	      }
	    }
	    else if (strlen(inputbuf[sn])-1<inputlen[sn]) {
	      inputbuf[sn][strlen(inputbuf[sn])-1]=0;
	      strcat(inputbuf[sn],buf);
	      strcat(inputbuf[sn],"_");
	    }
	    if (actbtn[sn]!=4) {
	      actbtn[sn]=4;
	      draw_di(sn,3);
	      draw_di(sn,4);
	    }
	    draw_di(sn,2);
	  }
	  else if ((keysym==XK_Tab && !sr && !sl) ||
	      (keysym==XK_KP_Tab && !sr && !sl) ||
	      keysym==XK_Right ||
	      keysym==XK_Down ||
	      keysym==XK_KP_Right ||
	      keysym==XK_KP_Down ||
	      keysym==osfXK_Right ||
	      keysym==osfXK_Down) {
	    new_actbtn(sn,1);
	  }
	  else if ((keysym==XK_Tab && sr+sl) ||
		   (keysym==XK_KP_Tab && sr+sl) ||
		   keysym==XK_3270_BackTab ||
		   keysym==XK_ISO_Left_Tab ||
		   keysym==hpXK_BackTab ||
		   keysym==hpXK_KP_BackTab ||
		   keysym==osfXK_BackTab ||
		   keysym==XK_Left ||
		   keysym==XK_Up ||
		   keysym==XK_KP_Left ||
		   keysym==XK_KP_Up ||
		   keysym==osfXK_Left ||
		   keysym==osfXK_Up) {
	    new_actbtn(sn,-1);
	  }
	  else if (keysym==XK_Return ||
		   keysym==XK_KP_Enter ||
		   keysym==XK_space ||
		   keysym==XK_Escape ||
		   keysym==XK_F1) {
	    if (ob && actbtn[sn]) {
	      button_press(sn,actbtn[sn],ob);
	      new_actbtn(sn,0);
	    }
	    else if (selpos[sn].num) {
	      i=selpos[sn].act;
	      del_selpos(sn);
	      hndl_button(sn,selpos[sn].p[i].x1+2,selpos[sn].p[i].y1+2,0,1);
	    }
	  }
	}
	break;
      case ButtonPress:
	if (ob) {
	  for (bt=1;bt<ob[0].spec;bt++) {
	    if (event.xbutton.window==ob[bt].win &&
		ob[bt].spec&(OB_BUTTON|OB_EXIT)) {
	      button_press(sn,bt,ob);
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
      set_selpos(sn);
    }
    if (b) {
      del_selpos(sn);
      hndl_button(b-1,x,y,opt,1);
    }
    computer();
  }
  if (actdial[0]==diwarteauf) {
    polldisps();
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

VOID findlastex(sn)
int sn;
{
  int i;
  OBJECT *ob;

  actbtn[sn]=0;
  ob=actdial[sn];
  for (i=ob[0].spec-1;i;i--) {
    if (ob[i].spec&OB_EXIT &&
	!ob_disabled(ob,i)) {
      actbtn[sn]=i;
      return;
    }
  }
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
  findlastex(sn);
  del_selpos(sn);
}

VOID create_diopt(sn,ob)
int sn;
OBJECT *ob;
{
  int x,y;

  if (actdial[sn]) remove_di(sn);
  x=(desk[sn].w-ob[0].w*charw[sn])/2;
  y=(desk[sn].h+12*charh[sn])/2-ob[0].h*charh[sn];
  create_dial(sn,x,y,0,ob);
  actdial[sn]=ob;
  findlastex(sn);
  del_selpos(sn);
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
  char *str;

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
    if (ob[i].str!=OB_NONE && (l=strlen(str=ob[i].str->t[lang[sn]]))) {
      x=y=0;
      if (ob[i].spec&(OB_BUTTON|OB_EXIT|OB_CENTERED)) {
	x=(ob[i].w*charw[sn]-XTextWidth(dfont[sn],str,l))/2;
	y=(ob[i].h-1)*charh[sn]/2;
      }
      else if (ob[i].spec&OB_RIGHT) {
	x=ob[i].w*charw[sn]-XTextWidth(dfont[sn],str,l);
      }
      if (gfx3d[sn]) {
	if (ob[i].spec&OB_EXIT) x+=2,y+=2;
	else if (ob[i].spec&OB_BUTTON) x++,y++;
      }
      XDrawString(dpy[sn],ob[i].win,gc[sn],x,
		  y+(charh[sn]+dfont[sn]->ascent-dfont[sn]->descent)/2-
		  gfx3d[sn],str,l);
      if (ob[i].spec&OB_BOLD) {
	XDrawString(dpy[sn],ob[i].win,gc[sn],x+1,
		    y+(charh[sn]+dfont[sn]->ascent-dfont[sn]->descent)/2-
		    gfx3d[sn],str,l);
      }
      if (ob[i].spec&OB_UNDERLINED && l) {
	XDrawLine(dpy[sn],ob[i].win,gc[sn],x,
		  y+(charh[sn]+dfont[sn]->ascent-dfont[sn]->descent)/2-
		  gfx3d[sn]+1,
		  x+XTextWidth(dfont[sn],str,l),
		  y+(charh[sn]+dfont[sn]->ascent-dfont[sn]->descent)/2-
		  gfx3d[sn]+1);
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
  draw_actbtn(sn,1);
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

VOID prspnam(txt,sn,ln)
char *txt;
int sn,ln;
{
  strcpy(txt,spnames[sn][0][ln]);
  if (spnames[sn][1][ln][0]) {
    strcat(txt," ");
    strcat(txt,spnames[sn][1][ln]);
  }
}

VOID di_info(sn,th)
int sn,th;
{
  int s,x,y,w;
  char txt[20];
  char clr[100];

  w=3*desk[sn].cardw/XTextWidth(dfont[sn]," ",1)+1;
  if (w>99) w=99;
  clr[w]=0;
  while (w) clr[--w]=' ';
  for (s=0;s<3;s++) {
    if (s!=sn) {
      prspnam(txt,s,lang[sn]);
      x=(s==left(sn)?desk[sn].com1x
	 :desk[sn].com2x)+30*desk[sn].f/desk[sn].q;
      y=(s==left(sn)?desk[sn].com1y
	 :desk[sn].com2y)+130*desk[sn].f/desk[sn].q;
      v_gtext(sn,x,y,0,clr);
      if (th<-1) {
	v_gtext(sn,x,y+charh[sn],0,clr);
	continue;
      }
      if (numsp==1) {
	if (phase>=SPIELEN && phase<=NIMMSTICH && s==spieler && trumpf!=5) {
	  prspnam(txt,s,lang[sn]);
	}
	else {
	  *txt=0;
	}
      }
      if (numsp!=1 || !briefmsg[sn]) {
	v_gtext(sn,x,y,0,txt);
      }
      y+=charh[sn];
      if ((phase==SPIELEN || phase==SCHENKEN) &&
	  (numsp!=1 || !briefmsg[sn]) &&
	  (stich==1 || (s!=th && !briefmsg[sn])) && s==spieler && trumpf!=5) {
	v_gtext(sn,x,y,0,clr);
	strcpy(txt,textarr[TX_SPIELT].t[lang[sn]]);
	strcat(txt,textarr[revolang?TX_REVOLUTION:trumpf_idx(sn,trumpf)].
	       t[lang[sn]]);
	v_gtext(sn,x,y,0,txt);
	y+=charh[sn];
	v_gtext(sn,x,y,0,clr);
	if (ouveang && !revolang) {
	  if (handsp && trumpf==-1) {
	    v_gtext(sn,x,y,0,textarr[TX_OUVE_HAND].t[lang[sn]]);
	  }
	  else {
	    v_gtext(sn,x,y,0,textarr[TX_OUVE].t[lang[sn]]);
	  }
	}
	else if (schwang) {
	  v_gtext(sn,x,y,0,textarr[TX_SCHW_ANGE].t[lang[sn]]);
	}
	else if (schnang) {
	  v_gtext(sn,x,y,0,textarr[TX_SCHN_ANGE].t[lang[sn]]);
	}
	else if (handsp && !revolang) {
	  v_gtext(sn,x,y,0,textarr[TX_HAND].t[lang[sn]]);
	}
      }
      else if (th>=0) {
	v_gtext(sn,x,y,0,clr);
	if (s==th) {
	  v_gtext(sn,x,y,0,textarr[TX_UEBERLEGT].t[lang[sn]]);
	}
	else if (phase>=REIZEN && phase<=ANSAGEN && !briefmsg[sn]) {
	  v_gtext(sn,x,y,0,textarr[ausspl==s?TX_VORHAND:
				   (ausspl+1)%3==s?TX_MITTELHAND:
				   TX_HINTERHAND].t[lang[sn]]);
	}
	v_gtext(sn,x,y+charh[sn],0,clr);
      }
      else if (phase>ANSAGEN) {
	v_gtext(sn,x,y,0,clr);
      }
    }
  }
}

VOID di_hand()
{
  int ln;
  static char txt[NUM_LANG][33];
  static tx_typ tt;

  dihand[4].str=&tt;
  for (ln=0;ln<NUM_LANG;ln++) {
    tt.t[ln]=txt[ln];
    sprintf(txt[ln],textarr[TX_GEREIZT_BIS_N].t[ln],reizw[reizp]);
  }
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
    actbtn[sn]--;
  }
}

VOID di_term(sn,s)
int sn,s;
{
  int ln;
  static char txt[3][NUM_LANG][20];
  static tx_typ tt[3];

  if (lost[sn]) return;
  diterm[sn][2].str=&tt[sn];
  for (ln=0;ln<NUM_LANG;ln++) {
    tt[sn].t[ln]=txt[sn][ln];
    prspnam(txt[sn][ln],s,ln);
  }
  create_di(sn,diterm[sn]);
}

VOID di_ende(sn)
int sn;
{
  create_di(sn,diende[sn]);
  actbtn[sn]--;
}

VOID di_loesch(sn)
int sn;
{
  create_di(sn,diloesch);
  actbtn[sn]--;
}

VOID di_ansage()
{
  int ln,sn,i,ktr;
  static char txt[3][NUM_LANG][30];
  static tx_typ tt[3];

  ktrply=-1;
  diansage[2].str=&tt[0];
  for (sn=0;sn<numsp;sn++) {
    dikontra[sn][2].str=dikonre[sn][2].str=&tt[sn];
    for (ln=0;ln<NUM_LANG;ln++) {
      tt[sn].t[ln]=txt[sn][ln];
      strcpy(txt[sn][ln],textarr[revolang?TX_REVOLUTION:trumpf_idx(sn,trumpf)].
	     t[ln]);
      if (trumpf==-1) {
	if (!revolang) {
	  if (ouveang) {
	    strcat(txt[sn][ln]," ");
	    strcat(txt[sn][ln],textarr[TX_OUVE].t[ln]);
	  }
	  if (handsp) {
	    strcat(txt[sn][ln]," ");
	    strcat(txt[sn][ln],textarr[TX_HAND].t[ln]);
	  }
	}
      }
      else {
	if (ouveang) {
	  strcat(txt[sn][ln]," ");
	  strcat(txt[sn][ln],textarr[TX_OUVE].t[ln]);
	}
	else if (schwang) {
	  strcat(txt[sn][ln]," ");
	  strcat(txt[sn][ln],textarr[TX_SCHWARZ].t[ln]);
	}
	else if (schnang) {
	  strcat(txt[sn][ln]," ");
	  strcat(txt[sn][ln],textarr[TX_SCHNEIDER].t[ln]);
	}
	else if (handsp) {
	  strcat(txt[sn][ln]," ");
	  strcat(txt[sn][ln],textarr[TX_HAND].t[ln]);
	}
      }
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
  if (numsp==1 && iscomp(spieler) && briefmsg[0] && !ktr) {
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
    create_di(sn,dikontra[sn]);
  }
}

VOID di_rekontra(sn)
int sn;
{
  int ln;
  static char txt[NUM_LANG][30];
  static tx_typ tt;

  ktrply=-1;
  direkontra[2].str=&tt;
  for (ln=0;ln<NUM_LANG;ln++) {
    tt.t[ln]=txt[ln];
    strcpy(txt[ln],textarr[TX_VON].t[ln]);
    prspnam(txt[ln]+strlen(txt[ln]),sn,ln);
  }
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
  int ln;
  static char txt[NUM_LANG][30];
  static tx_typ tt;

  ktrply=sn;
  dikonre[sn][3].str=&tt;
  for (ln=0;ln<NUM_LANG;ln++) {
    tt.t[ln]=txt[ln];
    strcpy(txt[ln],textarr[TX_MIT_KONTRA].t[ln]);
    if (kontrastufe==2) {
      strcat(txt[ln]," & ");
      strcat(txt[ln],textarr[TX_RE].t[ln]);
    }
  }
  create_di(sn,dikonre[sn]);
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
    if (numsp==1 && iscomp(spieler) && briefmsg[0] &&
	playkontra==2 && !sagte18[0]) {
      create_di(0,diansage);
    }
    else do_angesagt();
  }
}

VOID di_dicht()
{
  int sn;

  phase=SPIELDICHT;
  if (iscomp(spieler)) {
    for (sn=0;sn<numsp;sn++) {
      if (abkuerz[sn]) {
	spielendscr();
	return;
      }
    }
  }
  else {
    if (abkuerz[spieler]==1) {
      didicht[1].str=&textarr[trumpf==-1?TX_NULL_DICHT:TX_REST_BEI_MIR];
      if (irc_play) {
	didicht[0].h=7;
	didicht[5].spec=didicht[6].spec=OB_HIDDEN;
      }
      create_di(spieler,didicht);
      actbtn[spieler]-=irc_play?1:3;
      return;
    }
    else if (abkuerz[spieler]==2) {
      spielendscr();
      return;
    }
  }
  ndichtw=1;
  phase=SPIELEN;
}

VOID di_weiter(ini)
int ini;
{
  int x,y,sn;
  static int num;

  skatopen=0;
  if (!numsp || !ini) {
    if (!numsp || !--num) {
      if (reizp>=0) finishgame();
      else {
	clr_desk(0);
	phase=GEBEN;
      }
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
    findlastex(sn);
    del_selpos(sn);
  }
}

VOID di_wiederweiter(sn)
int sn;
{
  int x,y;

  x=(desk[sn].w-diwiederweiter[0].w*charw[sn])/2;
  y=(desk[sn].playy+desk[sn].skaty+desk[sn].cardh+
     -diwiederweiter[0].h*charh[sn])/2;
  if (actdial[sn]) remove_di(sn);
  create_dial(sn,x,y,0,diwiederweiter);
  actdial[sn]=diwiederweiter;
  findlastex(sn);
  del_selpos(sn);
}

VOID di_klopfen(sn)
int sn;
{
  create_di(sn,diklopfen);
  actbtn[sn]--;
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
    actbtn[msp]--;
    schenkply=msp;
  }
}

VOID di_geschenkt()
{
  if (iscomp(spieler)) {
    finishgame();
  }
  else {
    digeschenkt[4].str=&textarr[schnang?TX_SCHWARZ:TX_SCHNEIDER];
    create_di(spieler,schwang || trumpf==-1?diendeschenken:digeschenkt);
    if (!schwang && trumpf!=-1) actbtn[spieler]--;
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
  actbtn[sn]--;
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
      actbtn[sn]--;
      return;
    }
  } while (vmh);
  start_ramsch();
}

int di_verdoppelt(f,kl)
int f,kl;
{
  int ln,sn;
  static int verd1,verd2;
  static char txt[NUM_LANG][20];
  static tx_typ tt;

  if (!f) {
    diverdoppelt[3].str=&textarr[kl?TX_KLOPFT:TX_SCHIEBT];
    for (ln=0;ln<NUM_LANG;ln++) {
      tt.t[ln]=txt[ln];
      prspnam(txt[ln],spieler,ln);
    }
    diverdoppelt[2].str=&tt;
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
  dibuben[2].str=&textarr[blatt[spieler]>=2?TX_UNTER_NICHT:TX_BUBEN_NICHT];
  create_di(spieler,dibuben);
}

VOID di_spiel()
{
  int ln,i,j,a[4];
  static char txt[NUM_LANG][33];
  static tx_typ tt,tzur;

  ktrply=-1;
  dispiel[14].str=&tzur;
  dispiel[15].str=&tt;
  for (ln=0;ln<NUM_LANG;ln++) {
    tzur.t[ln]="<-";
    tt.t[ln]=txt[ln];
    sprintf(txt[ln],textarr[TX_GEREIZT_BIS_N].t[ln],reizw[reizp]);
  }
  dispiel[11].spec=spitzezaehlt && kannspitze?OB_BUTTON:OB_HIDDEN;
  dispiel[13].spec=revolution?OB_BUTTON:OB_HIDDEN;
  dispiel[7].next=revolution?6:-5;
  for (i=0;i<4;i++) {
    dispiel[2+i].str=&textarr[(blatt[spieler]>=2?TX_SCHELLEN:TX_KARO)+i];
  }
  create_di(spieler,dispiel);
  a[0]=a[1]=a[2]=a[3]=0;
  for (i=0;i<10;i++) {
    if ((cards[10*spieler+i]&7)!=BUBE) a[cards[10*spieler+i]>>3]++;
  }
  j=3;
  for (i=2;i>=0;i--) {
    if (a[i]>a[j]) j=i;
  }
  dispiel[j+2].spec|=OB_SELECTED;
  actbtn[spieler]=12;
}

VOID list_fun(sn)
int sn;
{
  int i,j,k,s,e,r,d,curr[3][3],cgv[3][2],cp,ln;
  static char txt[3][13][4][10];
  static tx_typ tt[3][13][4];

  for (i=0;i<3;i++) {
    for (j=0;j<3;j++) {
      curr[i][j]=splsum[i][j];
    }
    for (j=0;j<2;j++) {
      cgv[i][j]=sgewoverl[i][j];
    }
    for (j=0;j<13;j++) {
      for (k=0;k<4;k++) {
	for (ln=0;ln<NUM_LANG;ln++) {
	  tt[i][j][k].t[ln]=txt[i][j][k];
	}
      }
    }
  }
  cp=alist[sn];
  for (j=0;j<splfirst[sn];j++) {
    modsum(curr,cgv,j,(int *)0,(int *)0,(int *)0,(int *)0);
  }
  for (k=0;k<3;k++) {
    diliste[sn][8+k].str=&tt[sn][0][k];
    sprintf(txt[sn][0][k],"%d",curr[k][cp]);
  }
  for (i=splfirst[sn],j=1;j<13 && i<splstp;i++,j++) {
    modsum(curr,cgv,i,&s,&e,&r,&d);
    for (k=0;k<4;k++) {
      diliste[sn][7+4*j+k].str=&tt[sn][j][k];
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
  for (k=0;k<3;k++) {
    sprintf(diliste[sn][59+k].str->t[0],"%d/%d",cgv[k][0],cgv[k][1]);
  }
}

VOID di_delliste()
{
  if (irc_play) di_liste(irc_pos,1);
  else if (numsp>1) di_liste(0,1);
}

VOID di_liste(sn,ini)
int sn,ini;
{
  int ln;
  static char txt[3][10];
  static char spt[3][3][40];
  static tx_typ tt1[3],tt2,tt3,tt4[3];

  for (ln=0;ln<NUM_LANG;ln++) {
    tt1[sn].t[ln]=txt[sn];
    tt2.t[ln]="<-";
    tt3.t[ln]="->";
    tt4[0].t[ln]=spt[sn][0];
    tt4[1].t[ln]=spt[sn][1];
    tt4[2].t[ln]=spt[sn][2];
  }
  sprintf(txt[sn],"<%d>",alist[sn]+1);
  set_names(diliste[sn],2);
  if (ini) splfirst[sn]=((splstp>0?splstp-1:0)/12)*12;
  diliste[sn][59].str=&tt4[0];
  diliste[sn][60].str=&tt4[1];
  diliste[sn][61].str=&tt4[2];
  diliste[sn][62].str=&tt2;
  diliste[sn][62].spec=splfirst[sn]>=12?OB_EXIT:OB_HIDDEN;
  diliste[sn][63].str=&tt1[sn];
  diliste[sn][63].spec=irc_play?OB_DISABLED|OB_EXIT:OB_EXIT;
  diliste[sn][65].str=splfirst[sn]+12>=splstp?&textarr[TX_LOESCHEN]:&tt3;
  diliste[sn][65].spec=splfirst[sn]+12<splstp ||
    ((splstp || (irc_play &&
		 (sum[0][0] || sum[0][1] || sum[0][2] ||
		  sum[1][0] || sum[1][1] || sum[1][2] ||
		  sum[2][0] || sum[2][1] || sum[2][2]))) &&
     ((!irc_play && !sn) || (irc_play &&
			     (phase==GEBEN || phase==REIZEN ||
			      (phase==SPIELEN && sn==(ausspl+vmh)%3)))) &&
     ini)?OB_EXIT:OB_HIDDEN;
  list_fun(sn);
  create_di(sn,diliste[sn]);
  actbtn[sn]=64;
}

int ger_toupper(c)
int c;
{
  switch (c) {
  case 'ä':return 'Ä';
  case 'ö':return 'Ö';
  case 'ü':return 'Ü';
  default:return toupper(c);
  }
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
      if (spec&OB_BOLD) fprintf(f,"%c",ger_toupper(*txt));
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
  int ln,tr,e,i,j,s,stiche[10][3];
  static char txt[3][10][3][NUM_LANG][20];
  static tx_typ tt[3][10][3];

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
	    ?OB_BOLD:OB_NONE;
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
      diproto[sn][8+3*i+s].str=&tt[sn][i][s];
      for (ln=0;ln<NUM_LANG;ln++) {
	tt[sn][i][s].t[ln]=txt[sn][i][s][ln];
	if (prot1.stichgem || protsort[sn]) {
	  if (prot1.spitze &&
	      stiche[i][s]==(prot1.trumpf==4?BUBE:SIEBEN|prot1.trumpf<<3)) {
	    strcpy(txt[sn][i][s][ln],textarr[TX_SPITZE_PROT].t[ln]);
	  }
	  else {
	    strcpy(txt[sn][i][s][ln],
		   textarr[(blatt[sn]>=2?TX_SCHELLEN:TX_KARO)+
			   (stiche[i][s]>>3)].t[ln]);
	    strcat(txt[sn][i][s][ln],
		   textarr[(blatt[sn]>=2?TX_AD:TX_A)+
			   (stiche[i][s]&7)].t[ln]);
	  }
	}
	else {
	  if (e==OB_UNDERLINED) strcpy(txt[sn][i][s][ln]," ");
	  else txt[sn][i][s][ln][0]=0;
	  strcat(txt[sn][i][s][ln],
		 textarr[prot1.schenken?
			 prot1.spieler==s?TX_AKZEPTIERT:TX_SCHENKEN:TX_PASSE].
		 t[ln]);
	  if (e==OB_UNDERLINED) strcat(txt[sn][i][s][ln]," ");
	}
      }
    }
    if (f && diproto[sn][8+3*i].spec!=OB_HIDDEN) {
      fprintf(f,"  ");
      for (s=0;s<3;s++) {
	pformat(f,diproto[sn][8+3*i+s].spec,txt[sn][i][s][lang[sn]],1);
      }
      fprintf(f,"\n");
    }
  }
}

VOID im_skat(sn,ln,s,i)
int sn,ln;
char *s;
int i;
{
  strcpy(s,textarr[(blatt[sn]>=2?TX_SCHELLEN:TX_KARO)+(prot1.skat[i][0]>>3)].
	 t[ln]);
  strcat(s,textarr[(blatt[sn]>=2?TX_AD:TX_A)+(prot1.skat[i][0]&7)].t[ln]);
  strcat(s,",");
  strcat(s,textarr[(blatt[sn]>=2?TX_SCHELLEN:TX_KARO)+(prot1.skat[i][1]>>3)].
	 t[ln]);
  strcat(s,textarr[(blatt[sn]>=2?TX_AD:TX_A)+(prot1.skat[i][1]&7)].t[ln]);
}

VOID di_proto(sn,ini,log)
int sn,ini,log;
{
  static char txt[3][NUM_LANG][40],aug[3][NUM_LANG][20];
  static char imski[3][NUM_LANG][40],imskw[3][NUM_LANG][40];
  static char vhschob[3][NUM_LANG][40],mhschob[3][NUM_LANG][40];
  static char bis[3][10];
  static tx_typ ttxt[3],taug[3];
  static tx_typ timski[3],timskw[3];
  static tx_typ tvhschob[3],tmhschob[3];
  static tx_typ tbis[3],tzur,tvor;
  char *hd="----------------------------------------\n";
  char *tl="========================================\n";
  char hdbuf[100],spn[12];
  int ln,s,p,u1,u2,u3;
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
	strcat(spn,spnames[s][0][lang[0]]);
	strcat(spn," ");
	p=OB_CENTERED|OB_UNDERLINED;
      }
      else {
	strcpy(spn,spnames[s][0][lang[0]]);
	p=OB_CENTERED;
      }
      pformat(f,p,spn,1);
    }
    fputc('\n',f);
    for (s=0;s<3;s++) {
      if (prot1.verdopp[s]==2 && spnames[s][1][0]) {
	strcpy(spn," ");
	strcat(spn,spnames[s][1][lang[0]]);
	strcat(spn," ");
	p=OB_CENTERED|OB_UNDERLINED;
      }
      else {
	strcpy(spn,spnames[s][1][lang[0]]);
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
  diproto[sn][44].str=&ttxt[sn];
  for (ln=0;ln<NUM_LANG;ln++) {
    ttxt[sn].t[ln]=txt[sn][ln];
    if (prot1.trumpf==5 && prot1.augen==0 && !prot1.gewonn) {
      strcpy(txt[sn][ln],textarr[TX_NIEMAND].t[ln]);
    }
    else {
      prspnam(txt[sn][ln],prot1.spieler,ln);
    }
    if (prot1.trumpf==5) {
      strcat(txt[sn][ln],textarr[prot1.gewonn?TX_GEWANN:TX_VERLOR].t[ln]);
      strcat(txt[sn][ln],textarr[TX_DEN_RAMSCH].t[ln]);
    }
    else {
      strcat(txt[sn][ln],textarr[TX_SPIELTE].t[ln]);
      strcat(txt[sn][ln],textarr[prot1.revolution?TX_REVOLUTION:
				 trumpf_idx(sn,prot1.trumpf)].t[ln]);
    }
  }
  diproto[sn][38].str=&textarr[TX_IM_SKAT_IST];
  diproto[sn][38].spec=OB_NONE;
  diproto[sn][39].str=&timski[sn];
  diproto[sn][39].spec=OB_NONE;
  diproto[sn][40].str=&textarr[TX_IM_SKAT_WAR];
  diproto[sn][40].spec=OB_NONE;
  diproto[sn][41].str=&timskw[sn];
  diproto[sn][41].spec=OB_NONE;
  diproto[sn][42].str=&textarr[TX_GEREIZT_BIS];
  diproto[sn][42].spec=OB_NONE;
  diproto[sn][43].str=&tbis[sn];
  diproto[sn][43].spec=OB_NONE;
  for (ln=0;ln<NUM_LANG;ln++) {
    tzur.t[ln]="<-";
    tvor.t[ln]="->";
    timskw[sn].t[ln]=imskw[sn][ln];
    timski[sn].t[ln]=imski[sn][ln];
    tvhschob[sn].t[ln]=vhschob[sn][ln];
    tmhschob[sn].t[ln]=mhschob[sn][ln];
    tbis[sn].t[ln]=bis[sn];
    taug[sn].t[ln]=aug[sn][ln];
    sprintf(aug[sn][ln],textarr[TX_N_AUGEN].t[ln],prot1.augen);
    im_skat(sn,ln,imskw[sn][ln],0);
    im_skat(sn,ln,imski[sn][ln],1);
    im_skat(sn,ln,vhschob[sn][ln],2);
    im_skat(sn,ln,mhschob[sn][ln],3);
  }
  sprintf(bis[sn],"%d",prot1.gereizt);
  u1=u2=u3=OB_NONE;
  if (prot1.sramsch) {
    p=protsort[sn];
    if (prot1.verdopp[prot1.anspiel[0]]==1) u1=OB_UNDERLINED;
    if (prot1.verdopp[left(prot1.anspiel[0])]==1) u2=OB_UNDERLINED;
    if (prot1.verdopp[right(prot1.anspiel[0])]==1) u3=OB_UNDERLINED;
  }
  else {
    p=protsort[sn] && !prot1.handsp?0:prot1.ehsso;
  }
  if (p) {
    if (prot1.sramsch) {
      diproto[sn][38].str=&textarr[TX_VH_SCHOB];
      diproto[sn][38].spec=u1;
      diproto[sn][39].str=&tvhschob[sn];
      diproto[sn][40].str=&textarr[TX_MH_SCHOB];
      diproto[sn][40].spec=u2;
      diproto[sn][41].str=&tmhschob[sn];
      diproto[sn][42].str=&textarr[TX_HH_SCHOB];
      diproto[sn][42].spec=u3;
      diproto[sn][43].str=&timski[sn];
    }
    else if (prot1.trumpf==5) {
      diproto[sn][40].spec=OB_HIDDEN;
      diproto[sn][41].spec=OB_HIDDEN;
      diproto[sn][42].spec=OB_HIDDEN;
      diproto[sn][43].spec=OB_HIDDEN;
    }
    else {
      diproto[sn][40].str=&textarr[TX_HAND_GESP+p-1];
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
      diproto[sn][45].str=&textarr[TX_GEWONNEN];
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
    diproto[sn][45].str=&textarr[prot1.gewonn?TX_GEWONNEN:TX_VERLOREN];
    diproto[sn][46].str=OB_NONE;
  }
  else if (prot1.trumpf==5 && prot1.augen==0 && !prot1.gewonn) {
    diproto[sn][45].str=diproto[sn][46].str=OB_NONE;
  }
  else {
    diproto[sn][45].str=&textarr[prot1.gewonn?TX_GEWO_MIT:TX_VERL_MIT];
    diproto[sn][46].str=&taug[sn];
  }
  diproto[sn][49].str=protsort[sn]?&tzur:&tvor;
  prot_fun(sn,f);
  if (log) {
    strcpy(hdbuf,hd);
    if (prot1.predef!=1) {
      sprintf(hdbuf+4+irc_play," random_seed %ld %ld %d%s",
	      prot1.savseed+11*irc_play,prot1.gamenr,
	      right(prot1.anspiel[0])+1,
	      prot1.rotateby<0?" L":prot1.rotateby>0?" R":"");
      hdbuf[strlen(hdbuf)]=' ';
    }
    fputs(hdbuf,f);
    protsort[sn]=1;
    prot_fun(sn,f);
    fputs(hd,f);
    fprintf(f,"%s %s\n",textarr[TX_IM_SKAT_IST].t[lang[sn]],
	    imski[sn][lang[sn]]);
    if (prot1.stichgem || prot1.schenken) {
      if (prot1.trumpf!=5) {
	fprintf(f,"%s %s\n",textarr[TX_IM_SKAT_WAR].t[lang[sn]],
		imskw[sn][lang[sn]]);
	if (prot1.gereizt) {
	  fprintf(f,"%s %s\n",textarr[TX_GEREIZT_BIS].t[lang[sn]],
		  bis[sn]);
	}
      }
      else if (prot1.sramsch) {
	fprintf(f,"%s %s\n",textarr[TX_IM_SKAT_WAR].t[lang[sn]],
		imskw[sn][lang[sn]]);
	pformat(f,u1,textarr[TX_VH_SCHOB].t[lang[sn]],0);
	fprintf(f," %s\n",vhschob[sn][lang[sn]]);
	pformat(f,u2,textarr[TX_MH_SCHOB].t[lang[sn]],0);
	fprintf(f," %s\n",mhschob[sn][lang[sn]]);
	pformat(f,u3,textarr[TX_HH_SCHOB].t[lang[sn]],0);
	fprintf(f," %s\n",imski[sn][lang[sn]]);
      }
      fprintf(f,"%s\n",txt[sn][lang[sn]]);
      if (diproto[sn][40].str!=&textarr[TX_IM_SKAT_WAR] && prot1.trumpf!=5) {
	fprintf(f,"%s\n",diproto[sn][40].str->t[lang[sn]]);
      }
      if (prot1.trumpf!=5 || prot1.augen!=0 || prot1.gewonn) {
	fprintf(f,"%s",diproto[sn][45].str->t[lang[sn]]);
	if (diproto[sn][46].str!=OB_NONE) {
	  fprintf(f," %s",diproto[sn][46].str->t[lang[sn]]);
	}
	fputc('\n',f);
      }
    }
    fputs(tl,f);
    if (f!=stdout) fclose(f);
  }
  else {
    diproto[sn][47].str=
      &textarr[(!sn || irc_play) && protsort[sn]?TX_SPEICHERN:TX_SPIELLISTE];
    create_di(sn,diproto[sn]);
    actbtn[sn]=48;
  }
}

VOID di_resultdi(sn)
int sn;
{
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

VOID di_result(be)
int be;
{
  int ln,sn,i,x,y,sx,sy;
  static int ini,smlh;
  static char sa[NUM_LANG][30],sw[NUM_LANG][40],sg[NUM_LANG][40];
  static char su[3][3][10],txt[NUM_LANG][30],spt[3][40];
  static tx_typ tsa,tsw,tsg,tsu[3][3],ttxt,tsp[3];

  sx=sy=0;
  for (sn=0;sn<numsp;sn++) {
    if (handsp || sn!=spieler || trumpf==5) {
      i=trumpf==5 && sn==ausspl;
      x=desk[sn].playx+8*desk[sn].cardx;
      sx=x-charw[sn];
      if (sn==spieler || numsp>1 || trumpf==5) {
	y=desk[sn].playy;
	sy=y+desk[sn].cardh-charh[sn];
      }
      else {
	y=desk[sn].y+3;
	sy=y;
	if (spieler==left(sn)) {
	  x=desk[sn].playx;
	  sx=x+2*desk[sn].cardw+charw[sn];
	}
      }
      putcard(sn,prot2.skat[i][0],x,y);
      putcard(sn,prot2.skat[i][1],x+desk[sn].cardx,y);
    }
  }
  diresult[2].str=
    (trumpf==5?
     (mes1?&textarr[TX_JUNGFRAU]:
      mes2?&textarr[TX_DURCHMARSCH]:
      OB_NONE)
     :
     (mes1?&textarr[TX_UEBERREIZT]:
      mes2?&textarr[TX_SCHNEIDERFREI]:
      mes3?&textarr[TX_NICHT_SCHWARZ]:
      mes4?&textarr[TX_SPITZE_VERLOREN]:
      OB_NONE));
  diresult[3].str=&tsg;
  diresult[4].str=&tsa;
  diresult[5].str=&tsw;
  diresult[16].str=&tsp[0];
  diresult[17].str=&tsp[1];
  diresult[18].str=&tsp[2];
  diresult[19].str=be?&ttxt:OB_NONE;
  for (ln=0;ln<NUM_LANG;ln++) {
    tsg.t[ln]=sg[ln];
    tsa.t[ln]=sa[ln];
    tsw.t[ln]=sw[ln];
    ttxt.t[ln]=txt[ln];
    tsp[0].t[ln]=spt[0];
    tsp[1].t[ln]=spt[1];
    tsp[2].t[ln]=spt[2];
    if (trumpf==5 && spwert==0) {
      strcpy(sg[ln],textarr[TX_NIEMAND].t[ln]);
    }
    else {
      prspnam(sg[ln],spieler,ln);
    }
    strcat(sg[ln]," ");
    strcat(sg[ln],textarr[spgew?TX_GEWINNT:TX_VERLIERT].t[ln]);
    if (trumpf==-1) {
      strcpy(sa[ln],textarr[TX_DAS_NULLSPIEL].t[ln]);
    }
    else if (trumpf==5) {
      strcpy(sa[ln],textarr[TX_DEN_RAMSCH].t[ln]);
    }
    else {
      if (stich==1) {
	sa[ln][0]=0;
      }
      else if ((spgew && schwz) || !nullv) {
	strcpy(sa[ln],textarr[TX_DAS_SPIEL_SCHWARZ].t[ln]);
      }
      else {
	sprintf(sa[ln],textarr[TX_MIT_N_ZU_M_AUGEN].t[ln],stsum,120-stsum);
      }
    }
    sprintf(sw[ln],"%s %d.",textarr[TX_DER_SPIELWERT_IST].t[ln],
	    spgew && (trumpf!=5 || stsum==120)?spwert:-spwert);
    if (be>1) sprintf(txt[ln],textarr[TX_N_BOCK_EREIGNISSE].t[ln],be);
    else strcpy(txt[ln],textarr[TX_BOCK_EREIGNIS].t[ln]);
  }
  for (i=0;i<3;i++) {
    sprintf(diresult[16+i].str->t[0],"%d/%d",cgewoverl[i][0],cgewoverl[i][1]);
  }
  for (sn=0;sn<numsp;sn++) {
    for (i=0;i<3;i++) {
      sprintf(su[sn][i],"%d",sum[i][alist[sn]]);
      diresult[12+i].str=&tsu[sn][i];
      for (ln=0;ln<NUM_LANG;ln++) {
	tsu[sn][i].t[ln]=su[sn][i];
      }
    }
    if (numsp>1) {
      set_names(dismlres[sn],5);
      dismlres[sn][1].str=diresult[2].str;
      dismlres[sn][2].str=diresult[3].str;
      dismlres[sn][3].str=diresult[4].str;
      dismlres[sn][4].str=diresult[5].str;
      dismlres[sn][11].str=diresult[12].str;
      dismlres[sn][12].str=diresult[13].str;
      dismlres[sn][13].str=diresult[14].str;
      dismlres[sn][14].str=diresult[16].str;
      dismlres[sn][15].str=diresult[17].str;
      dismlres[sn][16].str=diresult[18].str;
      dismlres[sn][17].str=diresult[19].str;
      if (!ini) {
	smlh=dismlres[sn][0].h;
	ini=1;
      }
      if (be) {
	dismlres[sn][17].spec&=~OB_HIDDEN;
	dismlres[sn][0].h=smlh+1;
      }
      else {
	dismlres[sn][17].spec|=OB_HIDDEN;
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
      if (sx && sy) {
	if ((trumpf==5 && sn==ausspl) || handsp) {
	  if (sx>desk[sn].w/2) {
	    sx-=(strlen(textarr[TX_IM_SKAT_IST].t[lang[sn]])+0)*
	      charw[sn];
	  }
	  v_gtextnc(sn,0,0,sx,sy,0,textarr[TX_IM_SKAT_IST].t[lang[sn]]);
	}
	else {
	  if (sx>desk[sn].w/2) {
	    sx-=(strlen(textarr[TX_URSPRUENG_SKAT].t[lang[sn]])+0)*
	      charw[sn];
	  }
	  v_gtextnc(sn,0,0,sx,sy,0,textarr[TX_URSPRUENG_SKAT].t[lang[sn]]);
	}
      }
      set_names(diresult,6);
      di_resultdi(sn);
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
  int i,f,ln;
  static char txt[3][NUM_LANG][20];
  static tx_typ tt[3];

  f=sn<0;
  if (f) {
    sn=0;
  }
  else {
    dioptions[sn][23].spec=OB_HIDDEN;
    dioptions[sn][24].spec=OB_HIDDEN;
  }
  if (!sn) firstgame=0;
  for (i=0;i<3;i++) {
    if (dioptions[sn][14+i].str!=OB_NONE) {
      dioptions[sn][14+i].str=&stichstr[blatt[sn]>=2][sn][i];
    }
  }
  if (dioptions[sn][12].str!=&textarr[TX_GESPIELT_WIRD]) {
    dioptions[sn][12].str=&spielstr[blatt[sn]>=2][sn];
  }
  dioptions[sn][10].str=bockspiele?&tt[sn]:OB_NONE;
  for (ln=0;ln<NUM_LANG;ln++) {
    tt[sn].t[ln]=txt[sn][ln];
    sprintf(txt[sn][ln],"%s %d",textarr[TX_BOCK_SPIELE].t[ln],bockspiele);
  }
  dioptions[sn][19].spec=splstp||splres||
    (irc_play && (sum[0][0] || sum[0][1] || sum[0][2] ||
		  sum[1][0] || sum[1][1] || sum[1][2] ||
		  sum[2][0] || sum[2][1] || sum[2][2]))
      ?OB_EXIT:OB_HIDDEN;
  dioptions[sn][19].str=&textarr[splres?TX_PROTOKOLL:TX_SPIELLISTE];
  dioptions[sn][17].spec=
    schenken && !schenkstufe && trumpf<=4 && stich==1 &&
      phase==SPIELEN && sn!=spieler && (ausspl+vmh)%3==sn
	?OB_EXIT:OB_HIDDEN;
  dioptions[sn][18].spec=OB_HIDDEN;
  if (f) {
    dioptions[sn][23].spec=OB_CENTERED|OB_BOLD;
    dioptions[sn][24].spec=OB_CENTERED|OB_BOLD;
  }
  else if (numsp==1 && phase!=SPIELEN && !irc_play) {
    dioptions[sn][18].spec=OB_EXIT;
  }
  dioptions[sn][14].spec=
    dioptions[sn][15].spec=
      dioptions[sn][16].spec=
	(dioptions[sn][17].spec==OB_EXIT ||
	 dioptions[sn][18].spec==OB_EXIT)
	  ?OB_HIDDEN:OB_NONE;
  create_di(sn,dioptions[sn]);
  dioptions[sn][5-sort1[sn]].spec|=OB_SELECTED;
  dioptions[sn][7-alternate[sn]].spec|=OB_SELECTED;
  dioptions[sn][9-sort2[sn]].spec|=OB_SELECTED;
}

VOID di_copyr(sn)
int sn;
{
  if (!sn && !dlhintseen) dlhintseen=2;
  if (ggcards) {
    create_di(sn,dicopyr[sn]);
  }
  else {
    create_di(sn,dicopyralt[sn]);
  }
}

VOID di_grafik(sn)
int sn;
{
  static tx_typ tt[2];
  int ln;
  char buf[40];

  sprintf(buf,"%s %s",usrname[0],usrname[1]);
  extractnamln(3,buf,0);
  if (!spnames[0][0][0][0]) {
    strcpy(usrname[0],"?");
    strcpy(usrname[1],"");
  }
  else {
    strcpy(usrname[0],spnames[0][0][0]);
    strcpy(usrname[1],spnames[0][1][0]);
  }
  for (ln=0;ln<NUM_LANG;ln++) {
    tt[0].t[ln]=usrname[0];
    tt[1].t[ln]=usrname[1];
  }
  digrafik[sn][11].str=&tt[0];
  digrafik[sn][12].str=&tt[1];
  if (irc_play || numsp>1) {
    digrafik[sn][10].spec=
      digrafik[sn][11].spec=
	digrafik[sn][12].spec=OB_HIDDEN;
  }
  create_diopt(sn,digrafik[sn]);
  digrafik[sn][3+blatt[sn]].spec|=OB_SELECTED;
  digrafik[sn][8+lang[sn]].spec|=OB_SELECTED;
}

VOID di_strateg(sn)
int sn;
{
  static tx_typ tt[3][4];
  int i,dis,ln;
  char buf[40];

  dis=irc_play || numsp>2?OB_DISABLED:OB_NONE;
  for (i=0;i<5;i++) {
    distrateg[sn][i+4].spec|=dis;
  }
  dis=irc_play || numsp>1?OB_HIDDEN:OB_NONE;
  for (i=0;i<5;i++) {
    distrateg[sn][i+10].spec|=dis;
  }
  dis=irc_play || numsp>1?OB_HIDDEN:OB_NONE;
  distrateg[sn][3].spec=distrateg[sn][9].spec=dis;
  ln=lang[sn];
  sprintf(buf,"%s %s",conames[0][0],conames[0][1]);
  extractnamln(3,buf,ln);
  if (!spnames[3][0][ln][0]) {
    strcpy(conames[0][0],textarr[TX_COMPUTER].t[ln]);
    strcpy(conames[0][1],textarr[TX_LINKS].t[ln]);
  }
  else {
    strcpy(conames[0][0],spnames[3][0][ln]);
    strcpy(conames[0][1],spnames[3][1][ln]);
  }
  sprintf(buf,"%s %s",conames[1][0],conames[1][1]);
  extractnamln(3,buf,ln);
  if (!spnames[3][0][ln][0]) {
    strcpy(conames[1][0],textarr[TX_COMPUTER].t[ln]);
    strcpy(conames[1][1],textarr[TX_RECHTS].t[ln]);
  }
  else {
    strcpy(conames[1][0],spnames[3][0][ln]);
    strcpy(conames[1][1],spnames[3][1][ln]);
  }
  tt[sn][0].t[ln]=conames[0][0];
  tt[sn][1].t[ln]=conames[1][0];
  tt[sn][2].t[ln]=conames[0][1];
  tt[sn][3].t[ln]=conames[1][1];
  distrateg[sn][19].str=&tt[sn][0];
  distrateg[sn][20].str=&tt[sn][1];
  distrateg[sn][21].str=&tt[sn][2];
  distrateg[sn][22].str=&tt[sn][3];
  dis=irc_play || numsp>1?OB_HIDDEN:OB_NONE;
  for (i=0;i<5;i++) {
    distrateg[sn][18+i].spec|=dis;
  }
  create_diopt(sn,distrateg[sn]);
  distrateg[sn][strateg[0]+8].spec|=OB_SELECTED;
  distrateg[sn][strateg[1]+14].spec|=OB_SELECTED;
  distrateg[sn][hints[sn]+16].spec|=OB_SELECTED;
}

VOID di_varianten(sn)
int sn;
{
  divarianten[sn][0].next=irc_play?OB_DISABLED:OB_NONE;
  create_diopt(sn,divarianten[sn]);
  divarianten[sn][3+playramsch].spec|=OB_SELECTED;
  divarianten[sn][7+playkontra].spec|=OB_SELECTED;
  divarianten[sn][11+playbock].spec|=OB_SELECTED;
  divarianten[sn][15+spitzezaehlt].spec|=OB_SELECTED;
  divarianten[sn][19+revolution].spec|=OB_SELECTED;
  divarianten[sn][22+klopfen].spec|=OB_SELECTED;
  divarianten[sn][25+schenken].spec|=OB_SELECTED;
  divarianten[sn][28+oldrules].spec|=OB_SELECTED;
}

VOID di_ramschopts(sn)
int sn;
{
  diramschopts[sn][0].next=irc_play?OB_DISABLED:OB_NONE;
  create_diopt(sn,diramschopts[sn]);
  diramschopts[sn][3+playsramsch].spec|=OB_SELECTED;
  diramschopts[sn][6+rskatloser].spec|=OB_SELECTED;
}

VOID di_bockevents(sn)
int sn;
{
  int i,j;

  dibockevents[sn][0].next=irc_play?OB_DISABLED:OB_NONE;
  create_diopt(sn,dibockevents[sn]);
  for (i=1,j=0;i<=BOCK_BEI_LAST;i*=2,j++) {
    if (bockevents&i) {
      dibockevents[sn][2+j].spec|=OB_SELECTED;
    }
  }
  dibockevents[sn][11+resumebock].spec|=OB_SELECTED;
}

VOID di_geschwindigkeit(sn)
int sn;
{
  static char txt[3][NUM_LANG][20];
  static tx_typ tt[3],tkl[3],tgr[3];
  int i,dis,ln;

  digeschwindigkeit[sn][3].str=&tt[sn];
  for (ln=0;ln<NUM_LANG;ln++) {
    tt[sn].t[ln]=txt[sn][ln];
  }
  prverz(sn);
  dis=irc_play && nimmstich[sn][0]>=101?OB_DISABLED|OB_EXIT:OB_EXIT;
  for (i=0;i<3;i++) {
    for (ln=0;ln<NUM_LANG;ln++) {
      tkl[i].t[ln]="<<<"+i;
      tgr[i].t[ln]=">>>"+2-i;
    }
    digeschwindigkeit[sn][4+i].str=&tkl[i];
    digeschwindigkeit[sn][4+i].spec=dis;
    digeschwindigkeit[sn][7+i].str=&tgr[i];
    digeschwindigkeit[sn][7+i].spec=dis;
    digeschwindigkeit[sn][14+i].spec=irc_play?OB_DISABLED|OB_BUTTON:OB_BUTTON;
  }
  create_diopt(sn,digeschwindigkeit[sn]);
  digeschwindigkeit[sn][12-fastdeal].spec|=OB_SELECTED;
  digeschwindigkeit[sn][14+abkuerz[sn]].spec|=OB_SELECTED;
}

VOID di_mehrspieler(sn)
int sn;
{
  create_diopt(sn,dimehrspieler);
}

VOID di_lanspiel(sn)
int sn;
{
  create_di(sn,dilanspiel);
}

VOID di_eigenertisch(sn)
int sn;
{
  static tx_typ tt1,tt2;
  int ln;

  for (ln=0;ln<NUM_LANG;ln++) {
    tt1.t[ln]=lanip[1];
    tt2.t[ln]=lanip[2];
  }
  dieigenertisch[6].str=&tt1;
  dieigenertisch[11].str=&tt2;
  create_di(sn,dieigenertisch);
  if (!laninvite[0] && !laninvite[1]) laninvite[0]=1;
  dieigenertisch[4-laninvite[0]].spec|=OB_SELECTED;
  dieigenertisch[9-laninvite[1]].spec|=OB_SELECTED;
}

VOID di_anderertisch(sn)
int sn;
{
  static tx_typ tt0;
  int ln;

  for (ln=0;ln<NUM_LANG;ln++) {
    tt0.t[ln]=lanip[0];
  }
  dianderertisch[4].str=&tt0;
  create_di(sn,dianderertisch);
}

VOID di_warteauf(sn,u,s2,s3)
int sn,u,s2,s3;
{
  static tx_typ tt2,tt3;
  static char txt2[80],txt3[80];
  static int st2,st3;
  int ln;

  if (u) {
    if (st2==s2 && st3==s3) return;
    st2=s2;
    st3=s3;
  }
  else {
    st2=laninvite[0];
    st3=laninvite[1];
  }
  sprintf(txt2,textarr[TX_WARTEN_AUF_SPIELER_N].t[lang[sn]],2);
  sprintf(txt3,textarr[TX_WARTEN_AUF_SPIELER_N].t[lang[sn]],3);
  for (ln=0;ln<NUM_LANG;ln++) {
    tt2.t[ln]=txt2;
    tt3.t[ln]=txt3;
  }
  diwarteauf[2].str=&tt2;
  diwarteauf[3].str=&tt3;
  diwarteauf[2].spec=st2?OB_CENTERED:OB_HIDDEN;
  diwarteauf[3].spec=st3?OB_CENTERED:OB_HIDDEN;
  create_di(sn,diwarteauf);
}

VOID di_irc(sn)
int sn;
{
  static tx_typ tt;
  int ln;

  for (ln=0;ln<NUM_LANG;ln++) {
    tt.t[ln]=irc_hostname;
  }
  diirc[3].str=&tt;
  create_di(sn,diirc);
}

VOID di_eingabe(sn)
int sn;
{
  static char txt[5][2];
  static tx_typ tt[5];
  int i,ln;

  for (i=0;i<5;i++) {
    for (ln=0;ln<NUM_LANG;ln++) {
      tt[i].t[ln]=txt[i];
    }
    sprintf(txt[i],"%d",i+1);
    digui[sn][4+i].str=&tt[i];
  }
  create_diopt(sn,digui[sn]);
  digui[sn][3+mbutton[sn]].spec|=OB_SELECTED;
  digui[sn][10+keyboard[sn]].spec|=OB_SELECTED;
  digui[sn][14+briefmsg[sn]].spec|=OB_SELECTED;
  digui[sn][17+trickl2r[sn]].spec|=OB_SELECTED;
}

VOID di_wieder(sn,f)
int sn,f;
{
  if (f) {
    diwieder[11].spec&=~OB_HIDDEN;
  }
  else {
    diwieder[11].spec|=OB_HIDDEN;
  }
  create_di(sn,diwieder);
  diwieder[6].spec|=OB_SELECTED;
  diwieder[9+vorhandwn].spec|=OB_SELECTED;
}

VOID di_input(sn,ti,di,buf,len)
int sn,ti,di;
char *buf;
int len;
{
  static tx_typ tt[3],tx[3];
  static char txt[3][80];
  int ln;

  if (len>35) {
    buf[35]=0;
    len=35;
  }
  strcat(buf,"_");
  inputdi[sn]=di;
  inputbuf[sn]=buf;
  inputlen[sn]=len;
  sprintf(txt[sn]," %s ",textarr[ti].t[lang[sn]]);
  for (ln=0;ln<NUM_LANG;ln++) {
    tt[sn].t[ln]=buf;
    tx[sn].t[ln]=txt[sn];
  }
  diinput[sn][1].str=&tx[sn];
  diinput[sn][2].str=&tt[sn];
  create_di(sn,diinput[sn]);
}
