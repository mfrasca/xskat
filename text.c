
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

#define TEXT_C

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "text.h"

#define VERSION   " X S K A T   4.0 "
#define COPYRIGHT "Copyright 2004 © Gunter Gerhardt"
#define URL       "http://www.xskat.de"

static char *ger_text[]={
  "Null","Karo","Herz","Pik","Kreuz","Grand","Ramsch",
  "As","10","König","Dame","Bube","9","8","7",
  " A"," 10"," K"," D"," B"," 9"," 8"," 7",
  "Schellen","Rot","Grün","Eicheln",
  "As","10","König","Ober","Unter","9","8","7",
  " A"," 10"," K"," O"," U"," 9"," 8"," 7",
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
  "Spiele gewonnen/verloren",
  "Protokoll","Weiter"," Protokoll ",
  "Im Skat ist:",
  "Im Skat war:",
  "Ursprünglicher Skat",
  "Gereizt bis:",
  "Gewonnen mit:",
  "Verloren mit:",
  "Spielliste"," Spielliste ",VERSION,
  COPYRIGHT,URL,
  "Andere Kartenbilder zum Download",
  "sind unter obiger URL zu finden.",
  "Verwendung der Kartenbilder",
  "mit Genehmigung der",
  "Spielkartenfabrik Altenburg GmbH,",
  "Marke ASS Altenburger",
  "Sortieren","Aufwärts","Abwärts","Alternierend","Sequentiell",
  "Normal",
  "Gereizt bis:","Gespielt wird:","Letzter Stich:",
  "Du bist dran !","Vordefiniertes Spiel","Speichern",
  "Dieses Programm ist freie Software;",
  "es kann frei verbreitet werden.",
  "Benutzung auf eigenes Risiko;",
  "es gibt KEINE GARANTIE.",
  "Optionen"," Computer ","Spielstärke","--","-","0","+","++",
  "Name",
  "Vorschläge"," Varianten ",
  "Ramsch spielen","Immer",
  " Ramsch-Varianten ",
  "Skat geht an","letzten Stich","Verlierer",
  "Schieberamsch"," Skat aufnehmen ? ","Fertig",
  "Buben dürfen nicht geschoben werden.",
  "Unter dürfen nicht geschoben werden.",
  " Spielwert verdoppelt ","nimmt den Skat nicht auf.","klopft !",
  "VH schob:","MH schob:","HH schob:",
  "Kontra sagen","mit Kontra","ab 18","Kontra"," Kontra ! ","Re","von ",
  "Wiederholen"," Spiel wiederholen ","mit den Karten von","mir",
  "Vorhand wechselt:","Start","Zurück",
  " Geschwindigkeit ","Nimm Stich nach:","Sekunden","Maus-Klick",
  "Abkürzung","Fragen","Nie",
  "Bock-Runden","+Ramsch","Bock-Runden fortsetzen",
  " Bock-Ereignisse ","Bock-Ereignis","%d Bock-Ereignisse",
  "Bock-Spiele:"," Grand Hand ? ","   Bock-Spiel   ",
  "Verloren mit 60 Augen","Grand Hand gewonnen",
  "Erfolgreicher Kontra","Kontra & Re angesagt",
  "NNN Punkte in Spielliste","N00 Punkte in Spielliste",
  "Spielwert ist >= +72","Spielwert ist >= +96",
  " Benutzeroberfläche ","Tastatur",
  "Menü-Taste","Jede",
  "Wenig Hinweise",
  "Stich-Karten von links nach rechts",
  "Spitze","zählt 2","Spitze verloren !","Spitze !",
  "Spitze nicht erlaubt !",
  "Niedrigster Trumpf nicht auf der Hand.",
  "Grand mit 4.",
  " Null ist dicht ! "," Rest bei mir ! ","Karten aufdecken ?",
  "Revolution","Revolution: Karten austauschen !",
  "Vorhand","Mittelhand","Hinterhand",
  "Klopfen"," Klopfen ","Spielwert verdoppeln ?",
  "Schenken"," Schenken ","Spiel verloren geben ?","Geschenk annehmen ?",
  "Mitspieler lehnt ab.","Gegner geben auf.","Annehmen",
  "Geben","Schnell","Langsam",
  " Grafik & Text ",
  "Blatt","Französisch","Französisch (4 Farben)",
  "Deutsch","Deutsch (4 Farben)",
  "Sprache","Deutsch","English",
  "Alte Regeln",
  "Ein Maus-Klick oder ESC / F1","bringt dieses Menü zur Anzeige",
  " Mehrspieler-Modus ",
  " Lokales Spiel über LAN ",
  "Eigenen Spieltisch eröffnen",
  "An anderem Tisch Platz nehmen",
  "Spieler2 einladen:",
  "Spieler3 einladen:",
  "Rechnername oder IP-Adresse",
  "Spieltisch eröffnen",
  "Spieltisch:",
  "Zugriff auf X-Server wird erlaubt!",
  "Ggf. nach dem Spiel zurücknehmen.",
  "Siehe: man xhost",
  "Am Tisch Platz nehmen",
  "(Schließt dieses Fenster)",
  " Internet-Spiel über IRC ",
  "IRC-Verbindung herstellen mit:",
  "Erweiterte IRC-Konfiguration siehe:",
  "man xskat",
  "Verschiedene Versionen",
  "(Probleme? README.IRC-de oder 'man xskat' ANMERKUNGEN lesen!)",
  "Nicht auf diesem Kanal !",
  "Nur ein Spieler sollte /go sagen !  Nochmal.",
  "Sollte das /go%s sein ?",
  "Verbunden mit Spieler%d.",
  "Warten auf Spieler%d.",
  "\n Zum Spielen mit /join #xskatXYZ auf einen freien Kanal wechseln.",
  " Wenn alle Mitspieler auf diesem Kanal sind, muß einer /go eingeben,",
  " bzw. /go2, wenn nur 2 Spieler teilnehmen.",
  " Zeilen, die nicht mit / beginnen, werden als Nachricht verschickt.\n",
  " ### XSkat-Zeiten: jeden Fr 13:30 und So 18:00 MEZ/MESZ ###\n",
  "Offizielle Regeln",
  "Position","Alternativ","Turnier","Geber",
  "Unbekanntes Kommando (versuche /help)",
  "/join kanal - wechsle auf einen anderen Kanal",
  "/go, /go2 - starte Spiel mit 3 bzw. 2 Spielern",
  "/quit - beende XSkat",
  "/nick name, /nick - ändere Namen oder zeige ihn an",
  "/who - wer ist auf dem aktuellen Kanal ?",
  "/list, /list text - zeige Kanäle (die xskat/text enthalten)",
  "/pos n - setze Position auf 1, 2 oder 3",
  "/bell - schalte Alarmton ein/aus",
  "/sync - synchronisiere sofort",
  "/quote command args - für Spezialisten",
  "/default - aktiviere offizielle Regeln",
  "/ramsch n, /sramsch b, /skattoloser b, /kontra n, /bock n, /resumebock n,",
  " /spitze n, /revolution b, /klopfen b, /schenken b, /oldrules b,",
  " /bockevents n, /alist b, /tlist b, /start n, /s1 n",
  " - ändere Regeln (n ist eine Zahl, b ist true/false)",
  "/rules - zeige allen Mitspielern aktuelle Regeln an"
};

static char *eng_text[]={
  "Null","Diamond","Heart","Spade","Club","Grand","Ramsch",
  "Ace","10","King","Queen","Jack","9","8","7",
  " A"," 10"," K"," Q"," J"," 9"," 8"," 7",
  "Bells","Hearts","Leaves","Acorns",
  "Ace","10","King","Ober","Unter","9","8","7",
  " A"," 10"," K"," O"," U"," 9"," 8"," 7",
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
  "Games won/lost",
  "Log","Continue"," Log ",
  "Skat is:",
  "Skat was:",
  "Original Skat",
  "Last bid:",
  "Won with:",
  "Lost with:",
  "Game list"," Game list ",VERSION,
  COPYRIGHT,URL,
  "Download other card images",
  "from the URL given above.",
  "Card images used",
  "with permission by",
  "Spielkartenfabrik Altenburg GmbH,",
  "Marke ASS Altenburger",
  "Sort","Up","Down","Alternating","Sequential",
  "Normal",
  "Bidding:","Playing:","Last trick:",
  "It's your turn !","      predefined game      ","Save",
  "This program is free software;",
  "you can redistribute it freely.",
  "Use xskat at your own risk;",
  "there is NO WARRANTY.",
  "Options"," Computer ","Playing strength","--","-","0","+","++",
  "Name",
  "Hints"," Variations ",
  "Play Ramsch","Always",
  " Ramsch variations ",
  "Skat goes to","last trick","loser",
  "Schieberamsch"," Pick up Skat ? ","Done",
  "It's not allowed to pass on jacks.",
  "It's not allowed to pass on unters.",
  " Game score doubled ","doesn't pick up the Skat.","knocks !",
  "FH passed:","MH passed:","RH passed:",
  "Say Kontra","with Kontra","if 18","Kontra"," Kontra ! ","Re","by ",
  "Replay"," Replay game ","with the cards from","me",
  "Forehand changes:","Start","Back",
  " Speed ","Take trick after:","seconds","mouse click",
  "Shortcut","Ask","Never",
  "Bockrounds","+Ramsch","Resume Bockrounds",
  " Bock events ","Bock event","%d Bock events",
  "Bock games:"," Grand Hand ? ","   Bock game   ",
  "Lost with 60 points","Successful Grand Hand",
  "Successful Kontra","Kontra & Re game",
  "NNN points in game list","N00 points in game list",
  "Game value is >= +72","Game value is >= +96",
  " User interface ","Keyboard",
  "Menu button","Any",
  "Few messages",
  "Trick cards from left to right",
  "Spitze","counts 2","Lost Spitze !","Spitze !",
  "Spitze not allowed !",
  "Lowest trump not in your hand.",
  "Grand with 4.",
  " Null is safe ! "," Rest is mine ! ","Show cards ?",
  "Revolution","Revolution: exchange cards !",
  "Forehand","Middlehand","Rearhand",
  "Klopfen"," Klopfen ","Double game value ?",
  "Schenken"," Schenken ","Give up the game ?","Accept the gift ?",
  "Partner disagrees.","Opponents give up.","Accept",
  "Deal","Fast","Slow",
  " Graphic & Text ",
  "Cards","French","French (4 colors)",
  "German","German (4 colors)",
  "Language","Deutsch","English",
  "Old rules",
  "A mouse click or ESC / F1","will bring up this menu",
  " Multiplayer mode ",
  " Local game via LAN ",
  "Open own game table",
  "Join other table",
  "Invite Player2:",
  "Invite Player3:",
  "Computer name or IP address",
  "Open game table",
  "Game table:",
  "Access to X server will be granted!",
  "Remove after the game, if need be.",
  "See: man xhost",
  "Join game table",
  "(Will close this window)",
  " Internet game via IRC ",
  "Establish IRC connection with:",
  "Extended IRC configuration see:",
  "man xskat",
  "Version mismatch",
  "(Problems? See README.IRC or 'man xskat' NOTES!)",
  "Not on this channel !",
  "Only one player should say /go !  Try again.",
  "Did you mean /go%s ?",
  "Connected to Player%d.",
  "Waiting for Player%d.",
  "\n Start playing by typing /join #xskatXYZ to switch to a free channel.",
  " When all players are on this channel, one player has to enter /go,",
  " or /go2, if there are only 2 players.",
  " Lines that don't begin with / are sent as messages.\n",
  " ### XSkat times: every Fri 13:30 and Sun 18:00 MET/MEST ###\n",
  "Official rules",
  "Position","Alternative","Tournament","Dealer",
  "Unknown command (try /help)",
  "/join channel - switch to another channel",
  "/go, /go2 - start a game with 3 or 2 players",
  "/quit - terminate XSkat",
  "/nick name, /nick - change or display your nickname",
  "/who - is on the current channel ?",
  "/list, /list string - show channels (matching xskat/string)",
  "/pos n - set position to 1, 2 or 3",
  "/bell - toggle the alarm bell",
  "/sync - synchronize now",
  "/quote command args - for specialists",
  "/default - set the official rules",
  "/ramsch n, /sramsch b, /skattoloser b, /kontra n, /bock n, /resumebock n,",
  " /spitze n, /revolution b, /klopfen b, /schenken b, /oldrules b,",
  " /bockevents n, /alist b, /tlist b, /start n, /s1 n",
  " - change the rules of the game (n is a number, b is true/false)",
  "/rules - show the current rules to everyone on your channel"
};

static struct {
  char **arr;
  char *name[NUM_LANG];
  char *langpref[5];
} textdesc[NUM_LANG] = {
  {ger_text,{"deutsch","german"},{"de","german",0}},
  {eng_text,{"englisch","english"},{"en",0}}
};

VOID init_text()
{
  int i,j;

  for (i=0;i<NUM_LANG;i++) {
    for (j=0;j<TX_NUM_TX;j++) {
      textarr[j].t[i]=textdesc[i].arr[j];
    }
  }
}

char *idxlang(i)
int i;
{
  return textdesc[i].name[0];
}

int langidx(s)
char *s;
{
  char h[80];
  int i,j;

  for (i=0;i<79 && s && *s;i++,s++) {
    h[i]=tolower(*s);
  }
  h[i]=0;
  for (i=0;i<NUM_LANG;i++) {
    for (j=0;j<NUM_LANG;j++) {
      if (!strcmp(textdesc[i].name[j],h)) return i;
    }
  }
  if (s) {
    fprintf(stderr,"Unknown language '%s'.  Try one of:",h);
    for (i=0;i<NUM_LANG;i++) {
      if (i) fputc(',',stderr);
      fprintf(stderr," %s",textdesc[i].name[1]);
    }
    fputs(".\n",stderr);
  }
  s=getenv("LANG");
  if (!s) s=getenv("LANGUAGE");
  if (s) {
    for (i=0;i<79 && s && *s;i++,s++) {
      h[i]=tolower(*s);
    }
    h[i]=0;
    for (i=0;i<NUM_LANG;i++) {
      for (j=0;j<5 && textdesc[i].langpref[j];j++) {
	if (!strncmp(h,textdesc[i].langpref[j],
		     strlen(textdesc[i].langpref[j]))) return i;
      }
    }
  }
  for (i=0;i<NUM_LANG;i++) {
    for (j=0;j<NUM_LANG;j++) {
      if (!strcmp(textdesc[i].name[j],DEFAULT_LANGUAGE)) return i;
    }
  }
  return 0;
}
