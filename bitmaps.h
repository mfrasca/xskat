
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

#ifndef BITMAPS_H
#define BITMAPS_H

#undef EXTERN
#ifdef BITMAPS_C
#define EXTERN
#else
#define EXTERN extern
#endif

#define icon_width 32
#define icon_height 32
extern unsigned char icon_bits[];

#define symbs_width 384
#define symbs_height 26
extern unsigned char symbs_bits[];

#define symbl_width 576
#define symbl_height 40
extern unsigned char symbl_bits[];

extern unsigned char back_gif[];
extern unsigned char heb_gif[];
extern unsigned char hed_gif[];
extern unsigned char hek_gif[];
extern unsigned char kab_gif[];
extern unsigned char kad_gif[];
extern unsigned char kak_gif[];
extern unsigned char krb_gif[];
extern unsigned char krd_gif[];
extern unsigned char krk_gif[];
extern unsigned char pib_gif[];
extern unsigned char pid_gif[];
extern unsigned char pik_gif[];
extern unsigned char e10_gif[];
extern unsigned char e7_gif[];
extern unsigned char e8_gif[];
extern unsigned char e9_gif[];
extern unsigned char ea_gif[];
extern unsigned char ek_gif[];
extern unsigned char eo_gif[];
extern unsigned char eu_gif[];
extern unsigned char g10_gif[];
extern unsigned char g7_gif[];
extern unsigned char g8_gif[];
extern unsigned char g9_gif[];
extern unsigned char ga_gif[];
extern unsigned char gk_gif[];
extern unsigned char go_gif[];
extern unsigned char gu_gif[];
extern unsigned char h10_gif[];
extern unsigned char h7_gif[];
extern unsigned char h8_gif[];
extern unsigned char h9_gif[];
extern unsigned char ha_gif[];
extern unsigned char hk_gif[];
extern unsigned char ho_gif[];
extern unsigned char hu_gif[];
extern unsigned char s10_gif[];
extern unsigned char s7_gif[];
extern unsigned char s8_gif[];
extern unsigned char s9_gif[];
extern unsigned char sa_gif[];
extern unsigned char sk_gif[];
extern unsigned char so_gif[];
extern unsigned char su_gif[];

EXTERN unsigned char *fr_gif[4][3]
#ifdef BITMAPS_C
= {
  {kak_gif,kad_gif,kab_gif},
  {hek_gif,hed_gif,heb_gif},
  {pik_gif,pid_gif,pib_gif},
  {krk_gif,krd_gif,krb_gif}
}
#endif
;

EXTERN unsigned char *de_gif[4][8]
#ifdef BITMAPS_C
= {
  {sa_gif,s10_gif,sk_gif,so_gif,su_gif,s9_gif,s8_gif,s7_gif},
  {ha_gif,h10_gif,hk_gif,ho_gif,hu_gif,h9_gif,h8_gif,h7_gif},
  {ga_gif,g10_gif,gk_gif,go_gif,gu_gif,g9_gif,g8_gif,g7_gif},
  {ea_gif,e10_gif,ek_gif,eo_gif,eu_gif,e9_gif,e8_gif,e7_gif}
}
#endif
;

EXTERN unsigned char *backsd_gif[1]
#ifdef BITMAPS_C
= {
  back_gif
}
#endif
;

EXTERN unsigned char ggde_flg[4][8]
#ifdef BITMAPS_C
= {
  {1,0,0,0,0,1,1,1},
  {1,0,0,0,0,1,1,1},
  {1,1,0,0,0,1,1,1},
  {1,1,0,0,0,1,1,1}
}
#endif
;

EXTERN unsigned char de_flg[4][8]
#ifdef BITMAPS_C
= {
  {0,0,0,0,0,0,0,0},
  {1,0,0,0,0,1,1,0},
  {0,0,0,0,0,1,1,1},
  {2,0,0,0,0,0,1,0}
}
#endif
;

EXTERN struct
{
  char *name;
  unsigned char **pos;
} map_gif[45]
#ifdef BITMAPS_C
= {
  {"back",&backsd_gif[0]},
  {"heb",&fr_gif[1][2]},
  {"hed",&fr_gif[1][1]},
  {"hek",&fr_gif[1][0]},
  {"kab",&fr_gif[0][2]},
  {"kad",&fr_gif[0][1]},
  {"kak",&fr_gif[0][0]},
  {"krb",&fr_gif[3][2]},
  {"krd",&fr_gif[3][1]},
  {"krk",&fr_gif[3][0]},
  {"pib",&fr_gif[2][2]},
  {"pid",&fr_gif[2][1]},
  {"pik",&fr_gif[2][0]},
  {"e10",&de_gif[3][1]},
  {"e7",&de_gif[3][7]},
  {"e8",&de_gif[3][6]},
  {"e9",&de_gif[3][5]},
  {"ea",&de_gif[3][0]},
  {"ek",&de_gif[3][2]},
  {"eo",&de_gif[3][3]},
  {"eu",&de_gif[3][4]},
  {"g10",&de_gif[2][1]},
  {"g7",&de_gif[2][7]},
  {"g8",&de_gif[2][6]},
  {"g9",&de_gif[2][5]},
  {"ga",&de_gif[2][0]},
  {"gk",&de_gif[2][2]},
  {"go",&de_gif[2][3]},
  {"gu",&de_gif[2][4]},
  {"h10",&de_gif[1][1]},
  {"h7",&de_gif[1][7]},
  {"h8",&de_gif[1][6]},
  {"h9",&de_gif[1][5]},
  {"ha",&de_gif[1][0]},
  {"hk",&de_gif[1][2]},
  {"ho",&de_gif[1][3]},
  {"hu",&de_gif[1][4]},
  {"s10",&de_gif[0][1]},
  {"s7",&de_gif[0][7]},
  {"s8",&de_gif[0][6]},
  {"s9",&de_gif[0][5]},
  {"sa",&de_gif[0][0]},
  {"sk",&de_gif[0][2]},
  {"so",&de_gif[0][3]},
  {"su",&de_gif[0][4]}
}
#endif
;

#endif /* BITMAPS_H */
