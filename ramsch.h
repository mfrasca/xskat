
/*
    xskat - a card game for 1 to 3 players.
    Copyright (C) 1998  Gunter Gerhardt

    This program is free software; you can redistribute it freely.
    Use it at your own risk; there is NO WARRANTY.
*/

#ifndef RAMSCH_H
#define RAMSCH_H

#undef EXTERN
#ifdef RAMSCH_C
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN int rswert[8]
#ifdef RAMSCH_C
=
{
  0,0,4,5,0,3,2,1
}
#endif
;

EXTERN int ggdmw[8]
#ifdef RAMSCH_C
=
{
  7,6,5,0,4,1,2,3
}
#endif
;

EXTERN int rstsum[3],rstich[3];
EXTERN int ggdurchm[3];

#endif /* RAMSCH_H */
