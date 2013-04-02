
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
