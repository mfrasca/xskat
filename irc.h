
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

#ifndef IRC_H
#define IRC_H

#include <sys/types.h>

#undef EXTERN
#ifdef IRC_C
#define EXTERN
#else
#define EXTERN extern
#endif

#define IRC_NICK_LEN 30

enum {
  IRC_OFFLINE,
  IRC_TALK,
  IRC_SERVER,
  IRC_PLAYING
};

EXTERN pid_t irc_telnetpid;
EXTERN int irc_i,irc_o;
EXTERN int irc_state,irc_clients;
EXTERN int irc_play;
EXTERN int irc_2player,irc_pos;
EXTERN int irc_port;
EXTERN int irc_bell;
EXTERN int irc_logappend;
EXTERN int irc_histidx[3],irc_histcnt[3];
EXTERN int irc_inplen;
EXTERN long irc_tick;
EXTERN char *irc_telnet,*irc_host,*irc_channel;
EXTERN char *irc_nick,*irc_user,*irc_realname;
EXTERN char *irc_logfile;
EXTERN char *irc_history[3][100];
EXTERN char irc_substr[100];
EXTERN char irc_inpbuf[1024];
EXTERN struct {
  char nick[IRC_NICK_LEN+1];
  int sort1,alternate,large;
  int nimmstich,pos,abkuerz;
  char spnames[2][10];
} irc_conf[3];
EXTERN char irc_defaulthost[]
#ifdef IRC_C
=DEFAULT_IRC_SERVER
#endif
;
#endif /* IRC_H */
