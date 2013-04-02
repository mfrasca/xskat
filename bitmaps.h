
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

#endif /* BITMAPS_H */
