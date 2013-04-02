
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

#define IRC_C

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include "defs.h"
#include "skat.h"
#include "xio.h"
#include "xdial.h"
#include "text.h"
#include "irc.h"

VOID irc_log(s,in)
char *s;
int in;
{
  static FILE *f;
  static int ini,nl;

  if (!irc_logfile) return;
  if (!ini) {
    f=strcmp(irc_logfile,"-")?fopen(irc_logfile,irc_logappend?"a":"w"):stdout;
    if (!f) {
      fprintf(stderr,"Can't write file %s\n",irc_logfile);
      irc_logfile=0;
      return;
    }
    ini=nl=1;
  }
  if (nl) {
    fputs(in?"<-":"->",f);
    nl=0;
  }
  fprintf(f,"%s",s);
  if (strchr(s,'\n')) {
    nl=1;
    fflush(f);
  }
}

VOID irc_out(s)
char *s;
{
  write(irc_o,s,strlen(s));
  irc_log(s,0);
}

VOID irc_print(s)
char *s;
{
  unsigned char c;

  while ((c=*s++)) {
    if (c>=' ' || c=='\b' || c=='\t' || c=='\n' || c=='\r') {
      while (fputc(c,stderr)==EOF);
    }
  }
  fflush(stderr);
}

VOID irc_printnl(s)
char *s;
{
  irc_print(s);
  irc_print("\n");
}

char *irc_getline()
{
  static char buf[1024];
  static int siz,rst;
  static ssize_t cnt;
  char *ptr;

  ptr=buf;
  if (rst) {
    while (cnt--) *ptr++=buf[++siz];
    cnt=siz=ptr-buf;
    rst=0;
    ptr=buf;
  }
  if (cnt<=0) {
    ptr=buf+siz;
    cnt=read(irc_i,ptr,sizeof(buf)-siz);
  }
  if (cnt>0) {
    while (cnt && *ptr!='\n') cnt--,ptr++;
    siz=ptr-buf;
    if (cnt) {
      cnt--;
      *ptr=0;
      rst=1;
      if (*buf!=':' || !(ptr=strchr(buf,' ')) || strncmp(ptr+1,"322",3)) {
	irc_log(buf,1);
	irc_log("\n",1);
      }
      return buf;
    }
    if (siz==sizeof(buf)) {
      cnt=siz=0;
    }
  }
  return 0;
}

int irc_xinput(s,l)
char *s;
int l;
{
  s[l]=0;
  switch (s[0]) {
  case '\r':
    if (!irc_inplen) return 0;
    s[0]='\n';
    break;
  case '\b':case 127:
    if (irc_inplen) {
      strcpy(s,"\b \b");
    }
    else {
      s[0]=l=0;
    }
    break;
  case ' ':
  case '\t':
    if (!irc_inplen) return 0;
    break;
  case 0x1B:
    return 0;
  }
  irc_print(s);
  if (s[0]=='\n') {
    irc_inpbuf[irc_inplen]=0;
    irc_talk(irc_inpbuf);
    irc_inplen=0;
  }
  else if (s[0]=='\b') {
    irc_inplen--;
  }
  else if (irc_inplen+l<sizeof(irc_inpbuf)){
    strcpy(irc_inpbuf+irc_inplen,s);
    irc_inplen+=l;
  }
  else {
    irc_inplen=0;
  }
  return 1;
}

int irc_match(cmd,s)
char *cmd;
char **s;
{
  int len;

  len=strlen(cmd);
  if (!strncmp(*s,cmd,len)) {
    *s+=len;
    return 1;
  }
  return 0;
}

VOID irc_sendnick(s)
char *s;
{
  irc_out("nick ");
  irc_out(s);
  irc_out("\n");
}

VOID irc_pr_ss(s1,s2)
char *s1,*s2;
{
  irc_print(s1);
  irc_print(" : ");
  irc_printnl(s2);
}

VOID irc_pr_bs(s)
char *s;
{
  irc_print(" ");
  irc_printnl(s);
}

VOID irc_pr_sd(s,d)
char *s;
int d;
{
  char buf[20];

  sprintf(buf,"%d",d);
  irc_print(s);
  irc_print(" : ");
  irc_printnl(buf);
}

VOID irc_pr_ramsch(val)
int val;
{
  irc_pr_ss(textarr[TX_RAMSCH_SPIELEN].t[lang[0]],
	    textarr[val==2?TX_IMMER:TX_NEIN-val].t[lang[0]]);
}

VOID irc_pr_sramsch(val)
int val;
{
  irc_pr_ss(textarr[TX_SCHIEBERAMSCH].t[lang[0]],
	    textarr[TX_NEIN-val].t[lang[0]]);
}

VOID irc_pr_rskatloser(val)
int val;
{
  irc_pr_ss(textarr[TX_SKAT_GEHT_AN].t[lang[0]],
	    textarr[val?TX_VERLIERER:TX_LETZTEN_STICH].t[lang[0]]);
}

VOID irc_pr_kontra(val)
int val;
{
  irc_pr_ss(textarr[TX_KONTRA_SAGEN].t[lang[0]],
	    textarr[val==2?TX_AB18:TX_NEIN-val].t[lang[0]]);
}

VOID irc_pr_bock(val)
int val;
{
  irc_pr_ss(textarr[TX_BOCK_RUNDEN].t[lang[0]],
	    textarr[val==2?TX_UND_RAMSCH:TX_NEIN-val].t[lang[0]]);
}

VOID irc_pr_resumebock(val)
int val;
{
  irc_pr_ss(textarr[TX_FORTSETZEN].t[lang[0]],
	    textarr[TX_NEIN-val].t[lang[0]]);
}

VOID irc_pr_spitze(val)
int val;
{
  irc_pr_ss(textarr[TX_SPITZE].t[lang[0]],
	    textarr[val==2?TX_PLUS2:TX_NEIN-val].t[lang[0]]);
}

VOID irc_pr_revolution(val)
int val;
{
  irc_pr_ss(textarr[TX_REVOLUTION].t[lang[0]],
	    textarr[TX_NEIN-val].t[lang[0]]);
}

VOID irc_pr_klopfen(val)
int val;
{
  irc_pr_ss(textarr[TX_KLOPFEN].t[lang[0]],
	    textarr[TX_NEIN-val].t[lang[0]]);
}

VOID irc_pr_schenken(val)
int val;
{
  irc_pr_ss(textarr[TX_SCHENKEN].t[lang[0]],
	    textarr[TX_NEIN-val].t[lang[0]]);
}

VOID irc_pr_oldrules(val)
int val;
{
  irc_pr_ss(textarr[TX_ALTE_REGELN].t[lang[0]],
	    textarr[TX_NEIN-val].t[lang[0]]);
}

VOID irc_pr_bockevents(val)
int val;
{
  irc_print(textarr[TX_BOCK_EREIGNISSE_T].t[lang[0]]+1);
  irc_printnl(":");
  if (val&BOCK_BEI_60) {
    irc_pr_bs(textarr[TX_VERLOREN_MIT_60].t[lang[0]]);
  }
  if (val&BOCK_BEI_GRANDHAND) {
    irc_pr_bs(textarr[TX_GRAND_HAND_GEWONNEN].t[lang[0]]);
  }
  if (val&BOCK_BEI_KONTRA) {
    irc_pr_bs(textarr[TX_ERFOLGREICHER_KONTRA].t[lang[0]]);
  }
  if (val&BOCK_BEI_RE) {
    irc_pr_bs(textarr[TX_KONTRA_RE_ANGESAGT].t[lang[0]]);
  }
  if (val&BOCK_BEI_NNN) {
    irc_pr_bs(textarr[TX_NNN_IN_SPIELLISTE].t[lang[0]]);
  }
  if (val&BOCK_BEI_N00) {
    irc_pr_bs(textarr[TX_N00_IN_SPIELLISTE].t[lang[0]]);
  }
  if (val&BOCK_BEI_72) {
    irc_pr_bs(textarr[TX_SPIELWERT_72].t[lang[0]]);
  }
  if (val&BOCK_BEI_96) {
    irc_pr_bs(textarr[TX_SPIELWERT_96].t[lang[0]]);
  }
}

VOID irc_pr_alist(val)
int val;
{
  irc_pr_ss(textarr[TX_SPIELLISTE].t[lang[0]],
	    textarr[val==2?TX_TURNIER:val?TX_ALTERNATIV:TX_NORMAL].t[lang[0]]);
}

VOID irc_pr_start(val)
int val;
{
  irc_pr_sd(textarr[TX_GEBER].t[lang[0]],right(val)?right(val):3);
}

VOID irc_pr_s1(val)
int val;
{
  irc_pr_sd(textarr[TX_SPIELSTAERKE].t[lang[0]],val);
}

VOID irc_showrules(aplayramsch,aplaysramsch,aplaykontra,aplaybock,
		   aresumebock,aspitzezaehlt,arevolution,aklopfen,
		   aschenken,abockevents,ageber,aalist,astrateg,
		   aoldrules,arskatloser)
int aplayramsch,aplaysramsch,aplaykontra,aplaybock;
int aresumebock,aspitzezaehlt,arevolution,aklopfen;
int aschenken,abockevents,ageber,aalist,astrateg;
int aoldrules,arskatloser;
{
  int f=1;

  if (aplayramsch) {
    f=0;
    irc_pr_ramsch(aplayramsch);
    irc_pr_sramsch(aplaysramsch);
    irc_pr_rskatloser(arskatloser);
  }
  if (aplaykontra) {
    f=0;
    irc_pr_kontra(aplaykontra);
  }
  if (aplaybock && (abockevents&(2*BOCK_BEI_LAST-1))) {
    f=0;
    irc_pr_bock(aplaybock);
    irc_pr_resumebock(aresumebock);
    irc_pr_bockevents(abockevents);
  }
  if (aspitzezaehlt) {
    f=0;
    irc_pr_spitze(aspitzezaehlt);
  }
  if (arevolution) {
    f=0;
    irc_pr_revolution(arevolution);
  }
  if (aklopfen) {
    f=0;
    irc_pr_klopfen(aklopfen);
  }
  if (aschenken) {
    f=0;
    irc_pr_schenken(aschenken);
  }
  if (aoldrules) {
    f=0;
    irc_pr_oldrules(aoldrules);
  }
  if (f) {
    irc_printnl(textarr[TX_OFFIZIELLE_REGELN].t[lang[0]]);
  }
  irc_pr_alist(aalist);
  irc_pr_start(ageber);
  irc_pr_s1(astrateg);
}

VOID irc_sendrules()
{
  char buf[1024];

  sprintf(buf,"notice %s :/rules %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
	  irc_channel,
	  playramsch,playsramsch,playkontra,playbock,
	  resumebock,spitzezaehlt,revolution,klopfen,
	  schenken,bockevents,geber,alist[0],strateg[0],oldrules,rskatloser);
  irc_out(buf);
}

VOID irc_incidx(idx,sn)
int *idx,sn;
{
  if (++idx[sn]>=100) idx[sn]=0;
}

VOID irc_sync(q)
char *q;
{
  int idx[3],n=0;

  sscanf(q,"%d,%d,%d\n",&idx[0],&idx[1],&idx[2]);
  while (idx[irc_pos]!=irc_histidx[irc_pos]) {
    irc_out("notice ");
    irc_out(irc_channel);
    irc_out(irc_history[irc_pos][idx[irc_pos]]);
    irc_incidx(idx,irc_pos);
    if (++n>3) {
      n=0;
      sleep(3);
    }
  }
}

VOID irc_sendsync(f)
int f;
{
  char buf[1024];

  sprintf(buf,"notice %s :/sync%s %d,%d,%d\n",
	  irc_channel,
	  f?"":"ed",irc_histidx[0],irc_histidx[1],irc_histidx[2]);
  irc_out(buf);
  irc_tick=ticker;
}

VOID irc_checksync()
{
  if (irc_state==IRC_PLAYING &&
      ticker-irc_tick>(30+5*irc_pos)*1000 &&
      !lost[0] && !lost[1] && !lost[2]) {
    irc_sendsync(1);
  }
}

VOID irc_alarm()
{
  if (irc_bell) {
    XBell(dpy[0],100);
    XFlush(dpy[0]);
  }
}

VOID irc_talk(msg)
char *msg;
{
  static int ini,len;
  static char buf[1024],channel[1024],nick[IRC_NICK_LEN+1];
  int i,c;
  char *p,plb[80];

  irc_checksync();
  irc_checkhist();
  if (!ini) {
    if (fcntl(fileno(stdin),F_SETFL,
	      fcntl(fileno(stdin),F_GETFL)|O_NONBLOCK)<0) {
      fprintf(stderr,"Fcntl stdin failed\n");
      exitus(1);
    }
    ini=1;
  }
  while (msg || (c=getchar())!=EOF) {
    if (msg) {
      len=strlen(msg);
      c='\n';
      p=msg;
    }
    else p=buf;
    if (len>=sizeof(buf)) len=0;
    if (c=='\n') {
      if (len) {
	buf[len]=0;
	if (irc_match("/join ",&p)) {
	  irc_out("privmsg ");
	  irc_out(irc_channel);
	  irc_out(" :--> ");
	  irc_out(p);
	  irc_out("\n");
	  irc_out("part ");
	  irc_out(irc_channel);
	  irc_out("\n");
	  strcpy(channel,p);
	  irc_channel=channel;
	  irc_out("join ");
	  irc_out(irc_channel);
	  irc_out("\n");
	}
	else if (irc_match("/who",&p)) {
	  irc_out("who ");
	  irc_out(irc_channel);
	  irc_out("\n");
	}
	else if (irc_match("/list",&p)) {
	  if (*p==' ') {
	    strncpy(irc_substr,p+1,sizeof(irc_substr)-1);
	  }
	  else {
	    strcpy(irc_substr,"xskat");
	  }
	  irc_out("list\n");
	}
	else if (irc_match("/sync",&p)) {
	  irc_sendsync(1);
	}
	else if (irc_match("/bell",&p)) {
	  irc_bell^=1;
	  irc_alarm();
	}
	else if (irc_match("/nick",&p)) {
	  if (*p++==' ') {
	    strncpy(nick,p,IRC_NICK_LEN);
	    nick[IRC_NICK_LEN]=0;
	    irc_sendnick(nick);
	  }
	  else {
	    irc_printnl(irc_nick);
	  }
	}
	else if (irc_match("/pos ",&p)) {
	  if (irc_state==IRC_TALK) {
	    irc_pos=atoi(p);
	    irc_pos--;
	    if (irc_pos<0 || irc_pos>2) irc_pos=-1;
	    irc_pr_sd(textarr[TX_POSITION].t[lang[0]],irc_pos+1);
	  }
	}
	else if (irc_match("/ramsch ",&p)) {
	  if (irc_state==IRC_TALK) {
	    playramsch=atoi(p);
	    if (playramsch<0) playramsch=0;
	    else if (playramsch>2) playramsch=2;
	    irc_pr_ramsch(playramsch);
	  }
	}
	else if (irc_match("/sramsch ",&p)) {
	  if (irc_state==IRC_TALK) {
	    playsramsch=istrue(p);
	    irc_pr_sramsch(playsramsch);
	  }
	}
	else if (irc_match("/skattoloser ",&p)) {
	  if (irc_state==IRC_TALK) {
	    rskatloser=istrue(p);
	    irc_pr_rskatloser(rskatloser);
	  }
	}
	else if (irc_match("/kontra ",&p)) {
	  if (irc_state==IRC_TALK) {
	    playkontra=atoi(p);
	    if (playkontra<0) playkontra=0;
	    else if (playkontra>2) playkontra=2;
	    irc_pr_kontra(playkontra);
	  }
	}
	else if (irc_match("/bock ",&p)) {
	  if (irc_state==IRC_TALK) {
	    playbock=atoi(p);
	    if (playbock<0) playbock=0;
	    else if (playbock>2) playbock=2;
	    irc_pr_bock(playbock);
	  }
	}
	else if (irc_match("/resumebock ",&p)) {
	  if (irc_state==IRC_TALK) {
	    resumebock=istrue(p);
	    irc_pr_resumebock(resumebock);
	  }
	}
	else if (irc_match("/spitze ",&p)) {
	  if (irc_state==IRC_TALK) {
	    spitzezaehlt=atoi(p);
	    if (spitzezaehlt<0) spitzezaehlt=0;
	    else if (spitzezaehlt>2) spitzezaehlt=2;
	    irc_pr_spitze(spitzezaehlt);
	  }
	}
	else if (irc_match("/revolution ",&p)) {
	  if (irc_state==IRC_TALK) {
	    revolution=istrue(p);
	    irc_pr_revolution(revolution);
	  }
	}
	else if (irc_match("/klopfen ",&p)) {
	  if (irc_state==IRC_TALK) {
	    klopfen=istrue(p);
	    irc_pr_klopfen(klopfen);
	  }
	}
	else if (irc_match("/schenken ",&p)) {
	  if (irc_state==IRC_TALK) {
	    schenken=istrue(p);
	    irc_pr_schenken(schenken);
	  }
	}
	else if (irc_match("/oldrules ",&p)) {
	  if (irc_state==IRC_TALK) {
	    oldrules=istrue(p);
	    irc_pr_oldrules(oldrules);
	  }
	}
	else if (irc_match("/bockevents ",&p)) {
	  if (irc_state==IRC_TALK) {
	    bockevents=atoi(p);
	    irc_pr_bockevents(bockevents);
	  }
	}
	else if (irc_match("/alist ",&p)) {
	  if (irc_state==IRC_TALK) {
	    alist[0]=istrue(p);
	    irc_pr_alist(alist[0]);
	  }
	}
	else if (irc_match("/tlist ",&p)) {
	  if (irc_state==IRC_TALK) {
	    alist[0]=istrue(p)?2:0;
	    irc_pr_alist(alist[0]);
	  }
	}
	else if (irc_match("/start ",&p)) {
	  if (irc_state==IRC_TALK) {
	    geber=atoi(p);
	    if (geber<1 || geber>3) geber=0;
	    else geber=left(geber);
	    irc_pr_start(geber);
	  }
	}
	else if (irc_match("/s1 ",&p)) {
	  if (irc_state==IRC_TALK) {
	    strateg[0]=atoi(p);
	    if (strateg[0]<-4) strateg[0]=-4;
	    else if (strateg[0]>0) strateg[0]=0;
	    irc_pr_s1(strateg[0]);
	  }
	}
	else if (irc_match("/default",&p)) {
	  if (irc_state==IRC_TALK) {
	    playramsch=playsramsch=playkontra=playbock=resumebock=0;
	    spitzezaehlt=revolution=klopfen=schenken=bockevents=oldrules=0;
	    rskatloser=0;
	    irc_printnl(textarr[TX_OFFIZIELLE_REGELN].t[lang[0]]);
	  }
	}
	else if (irc_match("/rules",&p)) {
	  if (irc_state==IRC_TALK) {
	    irc_showrules(playramsch,playsramsch,playkontra,playbock,
			  resumebock,spitzezaehlt,revolution,klopfen,
			  schenken,bockevents,geber,alist[0],strateg[0],
			  oldrules,rskatloser);
	    irc_sendrules();
	  }
	}
	else if (irc_match("/go",&p)) {
	  if (irc_state==IRC_TALK || irc_state==IRC_SERVER) {
	    if (!strcmp(irc_channel,"#xskat")) {
	      irc_printnl(textarr[TX_NICHT_HIER].t[lang[0]]);
	    }
	    else {
	      irc_clients=0;
	      irc_2player=*p=='2';
	      irc_out("notice ");
	      irc_out(irc_channel);
	      irc_out(" :/server ");
	      irc_out(textarr[TX_XSKAT].t[lang[0]]);
	      irc_out("\n");
	      sprintf(plb,textarr[TX_WARTEN_AUF_SPIELER_N].t[lang[0]],2);
	      irc_printnl(plb);
	      irc_state=IRC_SERVER;
	    }
	  }
	}
	else if (irc_match("/quote ",&p)) {
	  irc_out(p);
	  irc_out("\n");
	}
	else if (irc_match("/help",&p)) {
	  for (i=TX_IRC_HELP01;i<=TX_IRC_HELPXX;i++) {
	    irc_printnl(textarr[i].t[lang[0]]);
	  }
	}
	else if (irc_match("/quit",&p)) {
	  exitus(1);
	}
	else if (*p=='/') {
	  irc_printnl(textarr[TX_UNBEKANNTES_KOMMANDO].t[lang[0]]);
	}
	else {
	  if (!strcmp(p,"go") || !strcmp(p,"go2")) {
	    sprintf(plb,textarr[TX_SOLLTE_DAS_GO_X_SEIN].t[lang[0]],p+2);
	    irc_printnl(plb);
	  }
	  else {
	    irc_out("privmsg ");
	    irc_out(irc_channel);
	    irc_out(" :");
	    irc_out(p);
	    irc_out("\n");
	  }
	}
	len=0;
      }
    }
    else {
      buf[len++]=c;
    }
    msg=0;
  }
}

char *irc_copyname(t,f)
char *t,*f;
{
  char *p;

  p=strchr(f,'~');
  *p=0;
  strcpy(t,f);
  return p+1;
}

int irc_senderok(s)
char *s;
{
  switch (irc_pos) {
  case 0:return !strcmp(irc_conf[1].nick,s)?2:
    !irc_2player && !strcmp(irc_conf[2].nick,s)?3:0;
  case 1:return !strcmp(irc_conf[0].nick,s)?1:
    !irc_2player && !strcmp(irc_conf[2].nick,s)?3:0;
  default:return !strcmp(irc_conf[0].nick,s)?1:
    !strcmp(irc_conf[1].nick,s)?2:0;
  }
}

VOID irc_histsave(sn,s)
int sn;
char *s;
{
  irc_out(s);
  s=strchr(strchr(s,' ')+1,' ');
  if (irc_history[sn][irc_histidx[sn]]) {
    free(irc_history[sn][irc_histidx[sn]]);
  }
  if (!(irc_history[sn][irc_histidx[sn]]=(char *)malloc(strlen(s)+1))) {
    nomem();
  }
  strcpy(irc_history[sn][irc_histidx[sn]],s);
  irc_incidx(irc_histidx,sn);
}

VOID irc_sendbtev(sn,bt)
int sn,bt;
{
  char buf[1024];

  sprintf(buf,"notice %s :/cmd%02dbt %d\n",
	  irc_channel,irc_histidx[sn],bt);
  irc_histsave(sn,buf);
}

VOID irc_sendxyev(sn,x,y,opt)
int sn,x,y,opt;
{
  char buf[1024];

  sprintf(buf,"notice %s :/cmd%02dxy %d,%d,%d\n",
	  irc_channel,irc_histidx[sn],x,y,opt);
  irc_histsave(sn,buf);
}

VOID irc_sendsort(sn)
int sn;
{
  char buf[1024];

  sprintf(buf,"notice %s :/cmd%02dso %d,%d,%d\n",
	  irc_channel,irc_histidx[sn],
	  sort1[sn],sort2[sn],alternate[sn]);
  irc_histsave(sn,buf);
}

VOID irc_sendschenken(sn)
int sn;
{
  char buf[1024];

  sprintf(buf,"notice %s :/cmd%02dsc\n",
	  irc_channel,irc_histidx[sn]);
  irc_histsave(sn,buf);
}

VOID irc_sendloeschen(sn)
int sn;
{
  char buf[1024];

  sprintf(buf,"notice %s :/cmd%02dcl\n",
	  irc_channel,irc_histidx[sn]);
  irc_histsave(sn,buf);
}

VOID irc_addcmd(sn,idx,p)
int sn,idx;
char *p;
{
  if (irc_history[sn][idx]) free(irc_history[sn][idx]);
  else irc_histcnt[sn]++;
  if (!(irc_history[sn][idx]=(char *)malloc(strlen(p)+1))) nomem();
  strcpy(irc_history[sn][idx],p);
}

int irc_getcmd(sn,fh,q)
int sn,fh;
char *q;
{
  int idx,bt,x,y,opt,s1,s2,a,ok;
  char *p=q;

  ok=0;
  sscanf(q,"%d",&idx);
  q+=2;
  if (idx!=irc_histidx[sn]) {
    if (irc_histidx[sn]-idx+(idx>irc_histidx[sn]?100:0)>50) {
      if (!irc_histcnt[sn]) irc_sendsync(1);
      else irc_addcmd(sn,idx,p);
    }
    return ok;
  }
  if (irc_match("bt ",&q)) {
    sscanf(q,"%d\n",&bt);
    if (actdial[sn]) {
      irc_incidx(irc_histidx,sn);
      hndl_btevent(sn,bt);
      ok=1;
    }
  }
  else if (irc_match("xy ",&q)) {
    sscanf(q,"%d,%d,%d\n",&x,&y,&opt);
    if (hndl_button(sn,x,y,opt,0)) {
      irc_incidx(irc_histidx,sn);
      ok=1;
    }
  }
  else if (irc_match("so ",&q)) {
    sscanf(q,"%d,%d,%d\n",&s1,&s2,&a);
    irc_incidx(irc_histidx,sn);
    ok=1;
    sort1[sn]=s1;
    sort2[sn]=s2;
    alternate[sn]=a;
    initscr(sn,1);
  }
  else if (irc_match("sc",&q)) {
    irc_incidx(irc_histidx,sn);
    ok=1;
    di_schenken(sn);
  }
  else if (irc_match("cl",&q)) {
    irc_incidx(irc_histidx,sn);
    ok=1;
    setsum(1);
    save_list();
  }
  if (!ok && !fh) irc_addcmd(sn,idx,p);
  return ok;
}

VOID irc_checkhist()
{
  int ok,sn,idx;

  do {
    ok=0;
    for (sn=0;sn<numsp;sn++) {
      if (sn==irc_pos) continue;
      if (irc_history[sn][idx=irc_histidx[sn]]) {
	if (irc_getcmd(sn,1,irc_history[sn][idx])) {
	  free(irc_history[sn][idx]);
	  irc_history[sn][idx]=0;
	  irc_histcnt[sn]--;
	  ok=1;
	}
      }
    }
  } while (ok);
}

VOID irc_getrules(q)
char *q;
{
  int aplayramsch,aplaysramsch,aplaykontra,aplaybock;
  int aresumebock,aspitzezaehlt,arevolution,aklopfen;
  int aschenken,abockevents,ageber,aalist,astrateg;
  int aoldrules,arskatloser;

  sscanf(q,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
	 &aplayramsch,&aplaysramsch,&aplaykontra,&aplaybock,
	 &aresumebock,&aspitzezaehlt,&arevolution,&aklopfen,
	 &aschenken,&abockevents,&ageber,&aalist,&astrateg,
	 &aoldrules,&arskatloser);
  irc_showrules(aplayramsch,aplaysramsch,aplaykontra,aplaybock,
		aresumebock,aspitzezaehlt,arevolution,aklopfen,
		aschenken,abockevents,ageber,aalist,astrateg,
		aoldrules,arskatloser);
}

VOID irc_getserverconf(q)
char *q;
{
  static int i,f;
  char plb[80];
  int ln;

  if (irc_match("1 ",&q)) {
    q=irc_copyname(irc_conf[i].nick,q);
    q=irc_copyname(spnames[i][0][0],q);
    q=irc_copyname(spnames[i][1][0],q);
    for (ln=1;ln<NUM_LANG;ln++) {
      strcpy(spnames[i][0][ln],spnames[i][0][0]);
      strcpy(spnames[i][1][ln],spnames[i][1][0]);
    }
    if (!i) {
      strcpy(usrname[0],spnames[0][0][0]);
      strcpy(usrname[1],spnames[0][1][0]);
    }
    sscanf(q,"%d,%d,%d,%d,%d,%d\n",
	   &sort1[i],&alternate[i],&desk[i].large,&alist[i],
	   &nimmstich[i][0],&abkuerz[i]);
    calc_desk(i);
    if (!strcmp(irc_conf[i].nick,irc_nick)) {
      irc_pos=i;
      f=1;
    }
    else if (nimmstich[i][0]<101) {
      nimmstich[i][0]=0;
    }
    numsp=++i;
    irc_2player=numsp==2;
  }
  else if (f && irc_match("2 ",&q)) {
    sscanf(q,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%ld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
	   &playramsch,&playsramsch,&playkontra,&playbock,&resumebock,
	   &spitzezaehlt,&revolution,&klopfen,&schenken,
	   &bockevents,&geber,&alist[0],&strateg[0],&oldrules,&rskatloser,
	   &savseed,&bockspiele,&bockinc,&ramschspiele,
	   &sum[0][0],&sum[0][1],&sum[0][2],
	   &sum[1][0],&sum[1][1],&sum[1][2],
	   &sum[2][0],&sum[2][1],&sum[2][2],
	   &cgewoverl[0][0],&cgewoverl[1][0],&cgewoverl[2][0],
	   &cgewoverl[0][1],&cgewoverl[1][1],&cgewoverl[2][1]);
    savseed-=11;
    setrnd(&seed[0],savseed);
    setsum(0);
    if (!irc_2player) {
      sprintf(plb,textarr[TX_VERBUNDEN_MIT_SPIELER_N].t[lang[0]],3);
      irc_printnl(plb);
    }
    irc_state=IRC_PLAYING;
  }
  else i=0;
}

VOID irc_putserverconf()
{
  char buf[1024];
  int i;

  for (i=0;i<2+!irc_2player;i++) {
    sprintf(buf,"notice %s :/svconf1 %s~%s~%s~%d,%d,%d,%d,%d,%d\n",
	    irc_channel,
	    irc_conf[i].nick,spnames[i][0][0],spnames[i][1][0],
	    irc_conf[i].sort1,irc_conf[i].alternate,irc_conf[i].large,
	    alist[0],irc_conf[i].nimmstich,irc_conf[i].abkuerz);
    irc_out(buf);
  }
  sprintf(buf,"notice %s :/svconf2 %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%ld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
	  irc_channel,
	  playramsch,playsramsch,playkontra,playbock,resumebock,
	  spitzezaehlt,revolution,klopfen,schenken,
	  bockevents,geber,alist[0],strateg[0],oldrules,rskatloser,
	  savseed+11,bockspiele,bockinc,ramschspiele,
	  sum[0][0],sum[0][1],sum[0][2],
	  sum[1][0],sum[1][1],sum[1][2],
	  sum[2][0],sum[2][1],sum[2][2],
	  cgewoverl[0][0],cgewoverl[1][0],cgewoverl[2][0],
	  cgewoverl[0][1],cgewoverl[1][1],cgewoverl[2][1]);
  irc_out(buf);
  irc_state=IRC_PLAYING;
}

VOID irc_putclientconf()
{
  char buf[1024];

  sprintf(buf,"notice %s :/clconf %s~%s~%d,%d,%d,%d,%d,%d\n",
	  irc_channel,
	  spnames[0][0][0],spnames[0][1][0],
	  sort1[0],alternate[0],desk[0].large,
	  nimmstich[0][0],irc_pos,abkuerz[0]);
  irc_out(buf);
}

VOID irc_setpos()
{
  int i,j,p[3];

  for (i=0;i<3;i++) {
    p[i]=-1;
    for (j=0;j<numsp;j++) {
      if (irc_conf[j].pos==i) {
	if (i==2 && numsp==2) {
	  irc_conf[j].pos=-1;
	}
	else {
	  p[i]=j++;
	  for (;j<numsp;j++) {
	    if (irc_conf[j].pos==i) irc_conf[j].pos=-1;
	  }
	}
      }
    }
  }
  for (i=j=0;i<3;i++) {
    if (p[i]==-1) {
      for (;j<numsp;j++) {
	if (irc_conf[j].pos==-1) {
	  irc_conf[j].pos=i;
	  break;
	}
      }
    }
  }
}

VOID irc_getclientconf(s,q)
char *s,*q;
{
  int i,ln;
  char plb[80];

  i=irc_clients;
  strcpy(irc_conf[i].nick,s);
  q=irc_copyname(irc_conf[i].spnames[0],q);
  q=irc_copyname(irc_conf[i].spnames[1],q);
  sscanf(q,"%d,%d,%d,%d,%d,%d\n",
	 &irc_conf[i].sort1,&irc_conf[i].alternate,
	 &irc_conf[i].large,&irc_conf[i].nimmstich,
	 &irc_conf[i].pos,&irc_conf[i].abkuerz);
  irc_clients=++i;
  sprintf(plb,textarr[TX_VERBUNDEN_MIT_SPIELER_N].t[lang[0]],i+1);
  irc_printnl(plb);
  if (!irc_2player && i<2) {
    sprintf(plb,textarr[TX_WARTEN_AUF_SPIELER_N].t[lang[0]],i+2);
    irc_printnl(plb);
  }
  if (i==1+!irc_2player) {
    numsp=i+1;
    irc_conf[i].pos=irc_pos;
    irc_setpos();
    irc_pos=irc_conf[i].pos;
    for (i=2;i>=0;i--) {
      if (irc_conf[0].pos==i) irc_conf[i]=irc_conf[0];
      else if (!irc_2player && irc_conf[1].pos==i) irc_conf[i]=irc_conf[1];
    }
    i=irc_pos;
    strcpy(irc_conf[i].nick,irc_nick);
    irc_conf[i].sort1=sort1[0];
    irc_conf[i].alternate=alternate[0];
    irc_conf[i].large=desk[0].large;
    irc_conf[i].nimmstich=nimmstich[0][0];
    irc_conf[i].abkuerz=abkuerz[0];
    strcpy(irc_conf[i].spnames[0],spnames[0][0][0]);
    strcpy(irc_conf[i].spnames[1],spnames[0][1][0]);
    for (i=0;i<numsp;i++) {
      sort1[i]=irc_conf[i].sort1;
      alternate[i]=irc_conf[i].alternate;
      desk[i].large=irc_conf[i].large;
      alist[i]=alist[0];
      nimmstich[i][0]=i!=irc_pos && irc_conf[i].nimmstich<101
	?0:irc_conf[i].nimmstich;
      abkuerz[i]=irc_conf[i].abkuerz;
      for (ln=0;ln<NUM_LANG;ln++) {
	strcpy(spnames[i][0][ln],irc_conf[i].spnames[0]);
	strcpy(spnames[i][1][ln],irc_conf[i].spnames[1]);
      }
      calc_desk(i);
    }
    irc_putserverconf();
  }
}

VOID irc_changenick(send)
int send;
{
  static char nick[IRC_NICK_LEN+1];
  static long rseed;
  static int ini;

  if (!ini) {
    rseed=seed[0];
    ini=1;
  }
  sprintf(nick,"xskat%04d",rndval(&rseed,8191));
  if (send) irc_sendnick(nick);
  if (!send || irc_state==IRC_OFFLINE) irc_nick=nick;
}

VOID irc_checknick()
{
  if (!strlen(irc_nick)) {
    irc_changenick(0);
  }
}

VOID irc_nickchanged(oldnick,newnick)
char *oldnick,*newnick;
{
  static char nick[IRC_NICK_LEN+1];
  int i;

  if (*newnick==':') newnick++;
  if (!strcmp(oldnick,irc_nick)) {
    strncpy(nick,newnick,IRC_NICK_LEN);
    nick[IRC_NICK_LEN]=0;
    irc_nick=nick;
  }
  if (irc_state==IRC_PLAYING) {
    for (i=0;i<numsp;i++) {
      if (!strcmp(oldnick,irc_conf[i].nick)) {
	strncpy(irc_conf[i].nick,newnick,IRC_NICK_LEN);
	irc_conf[i].nick[IRC_NICK_LEN]=0;
      }
    }
  }
}

VOID irc_msg(nick,q)
char *nick,*q;
{
  int i,sn;
  char nam[20];

  irc_alarm();
  if (irc_inplen) {
    for (i=0;i<irc_inplen;i++) {
      irc_print("\b \b");
    }
  }
  irc_print("<");
  irc_print(nick);
  if (irc_state==IRC_PLAYING && (sn=irc_senderok(nick))) {
    prspnam(nam,sn-1,lang[sn-1]);
    if (strcmp(nick,nam)) {
      irc_print("/");
      irc_print(nam);
    }
  }
  irc_print("> ");
  irc_printnl(q);
  if (irc_inplen) {
    irc_inpbuf[irc_inplen]=0;
    irc_print(irc_inpbuf);
  }
}

VOID irc_parse(s)
char *s;
{
  char *p,*q,nick[IRC_NICK_LEN+1];
  char plb[80];
  int i,sn,nickerr,colon;
  static int ini;
  static char mynick[IRC_NICK_LEN+1];

  nickerr=0;
  colon=*s==':';
  if (colon) {
    s++;
    strncpy(nick,s,IRC_NICK_LEN);
    nick[IRC_NICK_LEN]=0;
    for (i=0,p=s;i<IRC_NICK_LEN && *p && *p!=' ' && *p!='!';i++,p++);
    nick[*p=='!'?p++-s:0]=0;
    p=strchr(p,' ');
    if (p) {
      p++;
      if ((q=strchr(p,' ')) &&
	  (q=strchr(q+1,' ')) &&
	  (q=strchr(q+1,':'))) q++;
      if (q && irc_match("NOTICE",&p)) {
	if (irc_match("/server ",&q)) {
	  if (irc_state==IRC_SERVER) {
	    irc_printnl(textarr[TX_NUR_EINER_SOLLTE_GO_SAGEN].t[lang[0]]);
	    irc_out("notice ");
	    irc_out(irc_channel);
	    irc_out(" :/noserver\n");
	    irc_state=IRC_TALK;
	  }
	  else if (irc_state==IRC_TALK) {
	    irc_out("notice ");
	    irc_out(irc_channel);
	    irc_out(" :/client ");
	    irc_out(textarr[TX_XSKAT].t[lang[0]]);
	    irc_out("\n");
	    if (!irc_match(textarr[TX_XSKAT].t[lang[0]],&q)) {
	      irc_pr_ss(textarr[TX_VERSCHIEDENE_VERSIONEN].t[lang[0]],q);
	    }
	    else {
	      sprintf(plb,textarr[TX_VERBUNDEN_MIT_SPIELER_N].t[lang[0]],2);
	      irc_printnl(plb);
	      irc_putclientconf();
	    }
	  }
	}
	else if (irc_match("/noserver",&q)) {
	  if (irc_state==IRC_SERVER) {
	    irc_printnl(textarr[TX_NUR_EINER_SOLLTE_GO_SAGEN].t[lang[0]]);
	    irc_state=IRC_TALK;
	  }
	}
	else if (irc_match("/client ",&q)) {
	  if (irc_state==IRC_SERVER) {
	    if (!irc_match(textarr[TX_XSKAT].t[lang[0]],&q)) {
	      irc_pr_ss(textarr[TX_VERSCHIEDENE_VERSIONEN].t[lang[0]],q);
	      irc_state=IRC_TALK;
	    }
	  }
	}
	else if (irc_match("/clconf ",&q)) {
	  if (irc_state==IRC_SERVER) {
	    irc_getclientconf(nick,q);
	  }
	}
	else if (irc_match("/svconf",&q)) {
	  if (irc_state==IRC_TALK || irc_state==IRC_SERVER) {
	    irc_state=IRC_TALK;
	    irc_getserverconf(q);
	  }
	}
	else if (irc_match("/cmd",&q)) {
	  irc_tick=ticker;
	  if (irc_state==IRC_PLAYING && (sn=irc_senderok(nick))) {
	    irc_getcmd(sn-1,0,q);
	  }
	}
	else if (irc_match("/sync",&q)) {
	  if (irc_state==IRC_PLAYING && irc_senderok(nick)) {
	    if (!irc_match("ed",&q)) {
	      irc_sendsync(0);
	    }
	    irc_sync(q);
	  }
	}
	else if (irc_match("/rules",&q)) {
	  if (irc_state==IRC_TALK) {
	    irc_print("<");
	    irc_print(nick);
	    irc_printnl(">");
	    irc_getrules(q);
	  }
	}
	else {
	  irc_msg(nick,q);
	}
      }
      else if (q && irc_match("PRIVMSG",&p)) {
	irc_msg(nick,q);
      }
      else if (irc_match("JOIN",&p)) {
	if (!strncmp(irc_nick,nick,9) && !ini) {
	  strcpy(mynick,nick);
	  irc_nick=mynick;
	  irc_out("mode ");
	  irc_out(irc_nick);
	  irc_out(" +i\n");
	  for (i=TX_IRC_INTRO1;i<=TX_IRC_INTROX;i++) {
	    irc_printnl(textarr[i].t[lang[0]]);
	  }
	  ini=1;
	}
	else {
	  irc_alarm();
	  irc_printnl(s);
	}
      }
      else if (irc_match("NICK ",&p)) {
	irc_printnl(s);
	irc_nickchanged(nick,p);
      }
      else if (irc_match("QUIT",&p)) {
	if (irc_state==IRC_PLAYING && (sn=irc_senderok(nick)) &&
	    !lost[0] && !lost[1] && !lost[2]) {
	  lost[sn-1]=1;
	  finish(sn-1,0);
	}
      }
      else if (irc_match("432",&p) || irc_match("433",&p)) {
	nickerr=1;
	irc_printnl(s);
	if (irc_state==IRC_OFFLINE) irc_changenick(1);
      }
      else if (irc_match("322",&p)) {
	if ((p=strchr(p+1,' ')) &&
	    (!irc_substr[0] ||
	     strstr(p,irc_substr))) {
	  p++;
	  if (*p==':') p++;
	  irc_printnl(p);
	  irc_log(s-1,1);
	  irc_log("\n",1);
	}
      }
      else if (*p>='0' && *p<='9') {
	if ((p=strchr(p,' ')) &&
	    (p=strchr(p+1,' '))) {
	  p++;
	  if (*p==':') p++;
	  irc_printnl(p);
	}
      }
      else {
	irc_printnl(s);
      }
    }
    else {
      irc_printnl(s);
    }
  }
  else if (irc_match("PING",&s)) {
    p=strchr(s,':');
    if (p) {
      irc_out("PONG ");
      irc_out(p+1);
      irc_out("\n");
    }
  }
  else if (p=s,irc_match("Connection closed",&s)) {
    irc_printnl(p);
    exitus(1);
  }
  else {
    irc_printnl(s);
  }
  if (irc_state==IRC_OFFLINE && colon && !nickerr) {
    irc_out("join ");
    irc_out(irc_channel);
    irc_out("\n");
    irc_state=IRC_TALK;
  }
}

VOID irc_connect()
{
  int pipe1fd[2];
  int pipe2fd[2];
  char buf[20];

#ifdef SIGTSTP
  signal(SIGTSTP,SIG_IGN);
#endif
  pipe(pipe1fd);
  pipe(pipe2fd);
  if (fcntl(pipe1fd[0],F_SETFL,
	    fcntl(pipe1fd[0],F_GETFL)|O_NONBLOCK)<0) {
    fprintf(stderr,"Fcntl pipe failed\n");
    exitus(1);
  }
  if ((irc_telnetpid=fork())<0) {
    fprintf(stderr,"Fork failed\n");
    exitus(1);
  }
  if (!irc_telnetpid) {
    if (dup2(pipe2fd[0],fileno(stdin))<0 ||
	dup2(pipe1fd[1],fileno(stdout))<0 ||
	dup2(fileno(stdout),fileno(stderr))<0) {
      fprintf(stderr,"Dup failed\n");
      exitus(1);
    }
    close(pipe1fd[0]);
    close(pipe1fd[1]);
    close(pipe2fd[0]);
    close(pipe2fd[1]);
    sprintf(buf,"%d",irc_port);
    execlp(irc_telnet,irc_telnet,irc_hostname,buf,NULL);
    fprintf(stderr,"Exec '%s' failed\n",irc_telnet);
    exitus(1);
  }
  close(pipe1fd[1]);
  close(pipe2fd[0]);
  irc_i=pipe1fd[0];
  irc_o=pipe2fd[1];
}

VOID irc_init()
{
  char *l;

  irc_state=IRC_OFFLINE;
  xinitplayers();
  irc_connect();
  irc_checknick();
  irc_print("*** ");
  irc_print(textarr[TX_XSKAT].t[lang[0]]);
  irc_printnl(" ***");
  irc_printnl(textarr[TX_README_IRC].t[lang[0]]);
  irc_print("Login ");
  irc_print(irc_user);
  irc_print(",");
  irc_print(irc_nick);
  irc_print(",");
  irc_printnl(irc_realname);
  irc_out("user ");
  irc_out(irc_user);
  irc_out(" xskat xskat :");
  irc_out(irc_realname);
  irc_out("\n");
  irc_sendnick(irc_nick);
  for (;;) {
    while ((l=irc_getline())) {
      irc_parse(l);
      if (irc_state==IRC_PLAYING) {
	irc_tick=ticker;
	return;
      }
    }
    irc_talk((char *)0);
    waitt(100,0);
  }
}
