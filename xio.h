
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

#ifndef XIO_H
#define XIO_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "text.h"

#undef EXTERN
#ifdef XIO_C
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN tx_typ tspnames[3][2];
EXTERN int charw[3],charh[3];
EXTERN int lost[3];
EXTERN Display *dpy[3];
EXTERN Colormap cmap[3];
EXTERN Window win[3];
EXTERN unsigned long bpix[3],wpix[3],fgpix[3],bgpix[3],btpix[3];
EXTERN unsigned long b3dpix[3],w3dpix[3],mkpix[3];
EXTERN int gfx3d[3];
EXTERN GC gc[3],gcbck[3],gcxor[3];
EXTERN XFontStruct *dfont[3];
EXTERN int ggcards;
EXTERN Pixmap bck[3];
EXTERN Pixmap symbs[3];
EXTERN Pixmap cardpx[3][33];
EXTERN Cursor cursor[3][2];
EXTERN int actbtn[3];
EXTERN int skatopen,stichopen,spitzeopen,backopen[3];
EXTERN int ktrply,sptzmrk,schenkply;
EXTERN int revolsort,tauschcard,tauschdone,tauschply;
EXTERN long ticker;
EXTERN char *prog_name;
EXTERN char *disp_name[3];
EXTERN char *font_name;
EXTERN char *title[3];
EXTERN char *fg_col;
EXTERN char *bg_col;
EXTERN char *b3d_col;
EXTERN char *w3d_col;
EXTERN char *mk_col;
EXTERN char *bt_col;
EXTERN char *ccol[4];
EXTERN int nopre;
EXTERN int bwcol;
EXTERN int downup;
EXTERN int altseq;
EXTERN int geom_f[3],geom_x[3],geom_y[3];
EXTERN int colerr;
EXTERN XSizeHints szhints[3];
EXTERN XWMHints wmhints;
EXTERN struct
{
  int num,act;
  struct {
    int x1,y1,x2,y2,f;
  } p[21];
} selpos[3];
EXTERN struct
{
  int large;
  int x,y,w,h;
  int col,plan;
  int com1x,com2x,com1y,com2y;
  int playx,playy;
  int skatx,skaty;
  int stichx,stichy;
  int cbox1x,cbox2x,cboxy;
  int pboxx,pboxy;
  int cardx,cardw,cardh;
  int f,q;
} desk[3];
EXTERN XColor color[3][256]
#ifdef XIO_C
=
{{
  {0,0xffff,0x0000,0x0000},
  {0,0xffff,0x0000,0x0000},
  {0,0x0000,0x0000,0x0000},
  {0,0x0000,0x0000,0x0000},
  {0,0xff00,0xb400,0x0000},
  {0,0x0000,0xb400,0x0000}
}}
#endif
;

EXTERN int cnts[]
#ifdef XIO_C
=
{
  0,2,22,26,30,34,52,68,82
}
#endif
;

EXTERN int bigs[]
#ifdef XIO_C
=
{
  33,60,
  15,6, 51,6, 33,24, 15,43, 51,43, 15,77, 51,77, 33,93, 15,111, 51,111,
  7,9, 60,107,
  7,9, 60,107,
  7,9, 60,107,
  15,6, 51,6, 15,43, 51,43, 33,60, 15,77, 51,77, 15,111, 51,111,
  15,6, 51,6, 33,33, 15,60, 51,60, 33,87, 15,111, 51,111,
  15,6, 51,6, 33,33, 15,60, 51,60, 15,111, 51,111
}
#endif
;

EXTERN int smls[]
#ifdef XIO_C
=
{
  2,23, 77,23, 2,105, 77,105
}
#endif
;

EXTERN int smlz[]
#ifdef XIO_C
=
{
  4,16, 75,16, 4,112, 75,112
}
#endif
;

EXTERN int smlc[]
#ifdef XIO_C
=
{
  6,5, 78,5, 6,126, 78,126
}
#endif
;

EXTERN int frm[2][9][2]
#ifdef XIO_C
=
{
  {
    {1,4},{1,3},{2,2},
    {3,1},{4,1},{0,0},
    {0,0},{0,0},{0,0}
  },
  {
    {1,7},{1,6},{1,5},
    {2,4},{3,3},{4,2},
    {5,1},{6,1},{7,1}
  }
}
#endif
;

EXTERN int ramp[4][6]
#ifdef XIO_C
=
{
  {0, 70,100,150,180,255},
  {0, 70,150,180,255,255},
  {0, 70,180,255,255,255},
  {0,180,255,255,255,255}
}
#endif
;

#endif /* XIO_H */
