
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

#ifndef SKAT_H
#define SKAT_H

#undef EXTERN
#ifdef SKAT_C
#define EXTERN
#else
#define EXTERN extern
#endif

#define AS 0
#define ZEHN 1
#define KOENIG 2
#define DAME 3
#define BUBE 4
#define NEUN 5
#define ACHT 6
#define SIEBEN 7

#define BOCK_BEI_60         1
#define BOCK_BEI_GRANDHAND  2
#define BOCK_BEI_KONTRA     4
#define BOCK_BEI_RE         8
#define BOCK_BEI_NNN       16
#define BOCK_BEI_N00       32
#define BOCK_BEI_72        64
#define BOCK_BEI_96       128
#define BOCK_BEI_LAST     128

enum {
 GEBEN,
 REIZEN,
 HANDSPIEL,
 DRUECKEN,
 ANSAGEN,
 REVOLUTION,
 SPIELEN,
 SCHENKEN,
 NIMMSTICH,
 SPIELDICHT,
 WEITER,
 RESULT
};

EXTERN int nullw[]
#ifdef SKAT_C
=
{
  23,35,46,59,92
}
#endif
;

EXTERN int rwert[]
#ifdef SKAT_C
=
{
  9,10,11,12,24
}
#endif
;

EXTERN int reizw[]
#ifdef SKAT_C
=
{
  18,20,22,23,24,27,30,33,35,36,40,44,45,46,48,50,
  54,55,59,60,63,66,70,72,77,80,81,84,88,90,96,99,
  100,108,110,117,120,121,126,130,132,135,140,143,
  144,150,153,154,156,160,162,165,168,170,171,176,
  180,187,189,190,192,198,200,204,207,209,210,216,
  220,228,240,264,999
}
#endif
;

EXTERN int cardw[]
#ifdef SKAT_C
=
{
  11,10,4,3,2,0,0,0
}
#endif
;

EXTERN int sortw[]
#ifdef SKAT_C
=
{
  0,1,2,3
}
#endif
;

EXTERN int numsp,numgames;
EXTERN int strateg[3],stgset[3];
EXTERN int hints[3],hintsset[3],hintcard[2];
EXTERN int blatt[3],blattset[3];
EXTERN int lang[3],langset[3];
EXTERN int maxrw[3],sort1[3],sort2[3],alternate[3],splfirst[3];
EXTERN int downupset[3],altseqset[3],tdelayset[3],alistset[3];
EXTERN int mbutton[3],mbuttonset[3];
EXTERN int keyboard[3],keyboardset[3];
EXTERN int abkuerz[3],abkuerzset[3];
EXTERN int briefmsg[3],briefmsgset[3];
EXTERN int trickl2r[3],trickl2rset[3];
EXTERN int useoptfile[3];
EXTERN int lastmsaho[3],protsort[3],hatnfb[3][5];
EXTERN int splsum[3][3],sum[3][3],prevsum[3][3],alist[3];
EXTERN int sgewoverl[3][2],cgewoverl[3][2];
EXTERN int nimmstich[3][2],playramsch,ramschset;
EXTERN int playsramsch,sramschset,sramschstufe;
EXTERN int rskatloser,rskatloserset,rskatsum;
EXTERN int playkontra,kontraset,kontrastufe;
EXTERN int playbock,bockset,bockevents,bockeventsset;
EXTERN int bockspiele,ramschspiele,bockinc;
EXTERN int resumebock,resumebockset;
EXTERN int spitzezaehlt,spitzezaehltset;
EXTERN int revolution,revolutionset;
EXTERN int klopfen,klopfenset;
EXTERN int schenken,schenkenset,schenkstufe;
EXTERN int oldrules,oldrulesset;
EXTERN int fastdeal,fastdealset;
EXTERN int sagte18[3];
EXTERN int cards[32],gespcd[32];
EXTERN int quit,phase,geber,hoerer,sager,spieler;
EXTERN int saho,reizp,gedr,vmh,stich,ausspl;
EXTERN int possi[10],possc,stcd[3];
EXTERN int trumpf,handsp,stsum,astsum,gstsum,spcards[12],kannspitze;
EXTERN int spgew,spwert,schwz,nullv,spitzeok,nspwert;
EXTERN int schnang,schwang,ouveang,spitzeang,revolang;
EXTERN struct { int s,r,d,e,g; } splist[360];
EXTERN int splstp,splres;
EXTERN int gespfb[4],high[5],shigh[5];
EXTERN int inhand[4][8];
EXTERN int playcd,drkcd;
EXTERN int mes1,mes2,mes3,mes4;
EXTERN int butternok,sptruempfe;
EXTERN int predef,logging,unformatted;
EXTERN long seed[2],savseed,gamenr,rotateby;
EXTERN char *list_file,*game_file,*prot_file,*opt_file,*cards_file;
EXTERN int wieder,vorhandwn;
EXTERN int karobubeanz;
EXTERN int ndichtw;
EXTERN int firstgame,dlhintseen;
EXTERN int drbut,umdrueck;
EXTERN int pkoption;
EXTERN char lanip[3][40];
EXTERN int laninvite[2];
EXTERN char irc_hostname[40];
EXTERN int irc_hostset;
EXTERN int theargc;
EXTERN char **theargv;
EXTERN struct
{
  int stichgem,spieler,trumpf,gereizt,gewonn,augen,ehsso,predef,sramsch;
  int stiche[10][3],anspiel[10],gemacht[10],skat[4][2],verdopp[3];
  long savseed,gamenr;
  int rotateby,spitze,revolution,schenken,handsp;
} prot1,prot2;

#endif /* SKAT_H */
