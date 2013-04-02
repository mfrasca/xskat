
/*
    xskat - a card game for 1 to 3 players.
    Copyright (C) 1998  Gunter Gerhardt

    This program is free software; you can redistribute it freely.
    Use it at your own risk; there is NO WARRANTY.
*/

#ifndef NULL_H
#define NULL_H

#undef EXTERN
#ifdef NULL_C
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN int wirftabfb[4],hattefb[4],aussplfb[4],nochinfb[4];
EXTERN int naussplfb[3];

#endif /* NULL_H */
