
/*
    xskat - a card game for 1 to 3 players.
    Copyright (C) 1998  Gunter Gerhardt

    This program is free software; you can redistribute it freely.
    Use it at your own risk; there is NO WARRANTY.
*/

#define TEXT_C

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "text.h"

#define VERSION   " X S K A T   3.0 "
#define IRC_VERS  "1.0"
#define COPYRIGHT "Copyright 1998 © Gunter Gerhardt"
#define EMAIL     "(gerhardt@draeger.com)"
#define HOMEPAGE  "XSkat Home Page"
#define URL       "http://www.gulu.net/xskat/"

static char *ger_text[]={
  "Null","Karo","Herz","Pik","Kreuz","Grand","Ramsch",
  "As","10","König","Dame","Bube","9","8","7",
  " A"," 10"," K"," D"," B"," 9"," 8"," 7",
  "Hand gespielt","Schneider angesagt","Schwarz angesagt",
  "Ouvert gespielt","Ouvert Hand gespielt",
  "Passe","Ja","Nein","Spieler%d","Computer","links","rechts",
  "Drücken","spielt ","ouvert Hand","ouvert","Hand","Ouvert",
  "überlegt","Gereizt bis: %d","Löschen"," spielte ",
  "Gewonnen"," gewann ","Verloren"," verlor ","%d Augen",
  "Überreizt !","Gegner nicht Schneider !","Gegner nicht schwarz !",
  "Eine Jungfrau !","Durchmarsch !",
  "Niemand","gewinnt","verliert","den Ramsch",
  "das Nullspiel.","das Spiel schwarz !","mit %d zu %d Augen.",
  " Hand ? "," Spiel beenden ? "," Spielende ","beendet das Spiel","Aha",
  " Löschen ? "," Spiel ","Schneider","Schwarz","Spielen"," Hinweis ",
  "Nur bei Handspielen kann Schneider",
  "oder schwarz angesagt werden.",
  "Ouvert schließt schwarz angesagt",
  "ein (außer bei Null natürlich).",
  "Du hast höher gereizt als der",
  "Wert des angesagten Spiels !",
  "Null:23  Null ouvert:46",
  "Null Hand:35  Null ouvert Hand:59",
  "Revolution:92",
  " Angesagt ist "," Resultat ","Der Spielwert ist","Ende",
  "Protokoll","Weiter"," Protokoll ",
  "Im Skat ist:",
  "Im Skat war:",
  "Gereizt bis:",
  "Gewonnen mit:",
  "Verloren mit:",
  "Spielliste"," Spielliste ",VERSION,
  COPYRIGHT,EMAIL,HOMEPAGE,URL,
  "Sortieren","Aufwärts","Abwärts","Alternierend","Sequentiell",
  "Normal",
  "Gereizt bis:","Gespielt wird:","Letzter Stich:","Gedrückt:",
  "Du bist dran !","Vordefiniertes Spiel","Speichern",
  "Dieses Programm ist freie Software;",
  "es kann frei verbreitet werden.",
  "Verwendung auf eigenes Risiko;",
  "es gibt KEINE GARANTIE.",
  "Optionen"," Computer ","Strategie","passe","sage 18",
  "Vorschläge"," Varianten ",
  "Ramsch spielen","Immer",
  "Schieberamsch"," Skat aufnehmen ? ","Fertig",
  "Buben dürfen nicht geschoben werden.",
  " Spielwert verdoppelt ","nimmt den Skat nicht auf.","klopft !",
  "VH schob:","MH schob:","HH schob:",
  "Kontra sagen","mit Kontra","ab 18","Kontra"," Kontra ! ","Re","von ",
  "Wiederholen"," Spiel wiederholen ","mit den Karten von","mir",
  "Vorhand wechselt:",
  " Verzögerung ","Nimm Stich nach:","Sekunden","Maus-Klick",
  "Bock-Runden","+Ramsch","fortsetzen",
  " Bock-Ereignisse ","Bock-Ereignis","%d Bock-Ereignisse",
  "Bock-Spiele:"," Grand Hand ? ",
  "Verloren mit 60 Augen","Grand Hand gewonnen",
  "Erfolgreicher Kontra","Kontra & Re angesagt",
  "NNN Punkte in Spielliste","N00 Punkte in Spielliste",
  "Spielwert ist >= +72","Spielwert ist >= +96",
  " Menü-Button ","Jeder",
  "Spitze","zählt 2","Spitze verloren !","Spitze !",
  "Spitze nicht erlaubt !",
  "Niedrigster Trumpf nicht auf der Hand.",
  "Grand mit 4 Buben.",
  " Null ist dicht ! "," Rest bei mir ! ","Karten aufdecken ?",
  "Revolution","Revolution: Karten austauschen !",
  "Vorhand","Mittelhand","Hinterhand",
  "Klopfen"," Klopfen ","Spielwert verdoppeln ?",
  "Schenken"," Schenken ","Spiel verloren geben ?","Geschenk annehmen ?",
  "Mitspieler lehnt ab.","Gegner geben auf.","Annehmen",
  "Geben","Schnell","Langsam",
  IRC_VERS,"Verschiedene Versionen",
  "Nicht auf diesem Kanal !",
  "\n Zum Spielen mit /join #xskatXYZ auf einen freien Kanal wechseln.",
  " Wenn alle Mitspieler auf diesem Kanal sind, muß einer /go eingeben,",
  " bzw. /go2, wenn nur 2 Spieler teilnehmen.",
  " Zeilen, die nicht mit / beginnen, werden als Nachricht verschickt.\n",
  "Offizielle Regeln",
  "Position","Alternativ","Turnier","Geber",
  "Unbekanntes Kommando (versuche /help)",
  "/join kanal - wechsle auf einen anderen Kanal",
  "/go, /go2 - starte Spiel mit 3 bzw. 2 Spielern",
  "/nick name, /nick - ändere Namen oder zeige ihn an",
  "/who - wer ist auf dem aktuellen Kanal ?",
  "/list, /list text - zeige Kanäle (die xskat/text enthalten)",
  "/pos n - setze Position auf 1, 2 oder 3",
  "/bell - schalte Alarmton ein/aus",
  "/sync - synchronisiere sofort",
  "/quote command args - für Spezialisten",
  "/default - aktiviere offizielle Regeln",
  "/ramsch n, /sramsch b, /kontra n, /bock n, /resumebock n,",
  " /spitze n, /revolution b, /klopfen b, /schenken b,",
  " /bockevents n, /alist b, /tlist b, /start n, /s1 n",
  " - ändere Regeln (n ist eine Zahl, b ist true/false)",
  "/rules - zeige allen Mitspielern aktuelle Regeln an"
};

static char *eng_text[]={
  "Null","Diamond","Heart","Spade","Club","Grand","Ramsch",
  "Ace","10","King","Queen","Jack","9","8","7",
  " A"," 10"," K"," Q"," J"," 9"," 8"," 7",
  "Played Hand","Schneider announced","Schwarz announced",
  "Played ouvert","Played ouvert Hand",
  "Pass","Yes","No","Player%d","Computer","left","right",
  "Done","plays ","ouvert Hand","ouvert","Hand","Ouvert",
  "thinks","You have bid: %d","Clear"," played ",
  "Won"," won ","Lost"," lost ","%d points",
  "Overbid !","Opponents not Schneider !","Opponents not schwarz !",
  "One virgin !","Forced march !",
  "Nobody","wins","loses","the Ramsch",
  "the Null game.","the game schwarz !","with %d vs %d points.",
  " Hand ? "," Quit game ? "," Game over ","quits the game","OK",
  " Clear ? "," Game ","Schneider","Schwarz","Play"," Reminder ",
  "Only when playing Hand you may",
  "say Schneider or schwarz.",
  "Ouvert includes schwarz",
  "(except when playing Null).",
  "The value of your game",
  "is not high enough !",
  "Null:23  Null ouvert:46",
  "Null Hand:35  Null ouvert Hand:59",
  "Revolution:92",
  " Playing "," Result ","The value of the game is","Quit",
  "Log","Continue"," Log ",
  "Skat is:",
  "Skat was:",
  "Last bid:",
  "Won with:",
  "Lost with:",
  "Game list"," Game list ",VERSION,
  COPYRIGHT,EMAIL,HOMEPAGE,URL,
  "Sort","Up","Down","Alternating","Sequential",
  "Normal",
  "Bidding:","Playing:","Last trick:","Skat:",
  "It's your turn !","predefined game","Save",
  "This program is free software;",
  "you can redistribute it freely.",
  "Use it at your own risk;",
  "there is NO WARRANTY.",
  "Options"," Computer ","Strategy","pass","say 18",
  "Hints"," Variations ",
  "Play Ramsch","Always",
  "Schieberamsch"," Pick up Skat ? ","Done",
  "It's not allowed to pass on jacks.",
  " Game score doubled ","doesn't pick up the Skat.","knocks !",
  "FH passed:","MH passed:","RH passed:",
  "Say Kontra","with Kontra","if 18","Kontra"," Kontra ! ","Re","by ",
  "Replay"," Replay game ","with the cards from","me",
  "Forehand changes:",
  " Delay ","Take trick after:","seconds","mouse click",
  "Bockrounds","+Ramsch","resume",
  " Bock events ","Bock event","%d Bock events",
  "Bock games:"," Grand Hand ? ",
  "Lost with 60 points","Successful Grand Hand",
  "Successful Kontra","Kontra & Re game",
  "NNN points in game list","N00 points in game list",
  "Game value is >= +72","Game value is >= +96",
  " Menu button ","Any",
  "Spitze","counts 2","Lost Spitze !","Spitze !",
  "Spitze not allowed !",
  "Lowest trump not in your hand.",
  "Grand with 4 jacks.",
  " Null is safe ! "," Rest is mine ! ","Show cards ?",
  "Revolution","Revolution: exchange cards !",
  "Forehand","Middlehand","Rearhand",
  "Klopfen"," Klopfen ","Double game value ?",
  "Schenken"," Schenken ","Give up the game ?","Accept the gift ?",
  "Partner disagrees.","Opponents give up.","Accept",
  "Deal","Fast","Slow",
  IRC_VERS,"Version mismatch",
  "Not on this channel !",
  "\n Start playing by typing /join #xskatXYZ to switch to a free channel.",
  " When all players are on this channel, one player has to enter /go,",
  " or /go2, if there are only 2 players.",
  " Lines that don't begin with / are sent as messages.\n",
  "Official rules",
  "Position","Alternative","Tournament","Dealer",
  "Unknown command (try /help)",
  "/join channel - switch to another channel",
  "/go, /go2 - start a game with 3 or 2 players",
  "/nick name, /nick - change or display your nickname",
  "/who - is on the current channel ?",
  "/list, /list string - show channels (matching xskat/string)",
  "/pos n - set position to 1, 2 or 3",
  "/bell - toggle the alarm bell",
  "/sync - synchronize now",
  "/quote command args - for specialists",
  "/default - set the official rules",
  "/ramsch n, /sramsch b, /kontra n, /bock n, /resumebock n,",
  " /spitze n, /revolution b, /klopfen b, /schenken b,",
  " /bockevents n, /alist b, /tlist b, /start n, /s1 n",
  " - change the rules of the game (n is a number, b is true/false)",
  "/rules - show the current rules to everyone on your channel"
};

static struct {
  char **arr;
  char *name;
} textdesc[] = {
  {ger_text,"german"},
  {eng_text,"english"}
};

VOID init_text()
{
  textarr=textdesc[lang].arr;
}

int langidx(s,def)
char *s;
int def;
{
  char h[80];
  int i;

  for (i=0;i<79 && s && *s;i++,s++) {
    h[i]=tolower(*s);
  }
  h[i]=0;
  for (i=0;i<sizeof(textdesc)/sizeof(textdesc[0]);i++) {
    if (!strcmp(textdesc[i].name,h)) return i;
  }
  if (s) {
    fprintf(stderr,"Unknown language '%s'.  Try one of:",h);
    for (i=0;i<sizeof(textdesc)/sizeof(textdesc[0]);i++) {
      if (i) fputc(',',stderr);
      fprintf(stderr," %s",textdesc[i].name);
    }
    fputs(".\n",stderr);
  }
  if (!def) {
    return -1;
  }
  for (i=0;i<sizeof(textdesc)/sizeof(textdesc[0]);i++) {
    if (!strcmp(textdesc[i].name,DEFAULT_LANGUAGE)) return i;
  }
  return 0;
}
