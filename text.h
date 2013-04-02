
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

#ifndef TEXT_H
#define TEXT_H

#undef EXTERN
#ifdef TEXT_C
#define EXTERN
#else
#define EXTERN extern
#endif

#define NUM_LANG 2

typedef struct {
  char *t[NUM_LANG];
} tx_typ;

enum {
  TX_NULL,TX_KARO,TX_HERZ,TX_PIK,TX_KREUZ,TX_GRAND,TX_RAMSCH,
  TX_AS,TX_ZEHN,TX_KOENIG,TX_DAME,TX_BUBE,TX_NEUN,TX_ACHT,TX_SIEBEN,
  TX_A,TX_10,TX_K,TX_D,TX_B,TX_9,TX_8,TX_7,
  TX_SCHELLEN,TX_ROT,TX_GRUEN,TX_EICHEL,
  TX_ASD,TX_ZEHND,TX_KOENIGD,TX_OBERD,TX_UNTERD,TX_NEUND,TX_ACHTD,TX_SIEBEND,
  TX_AD,TX_10D,TX_KD,TX_OD,TX_UD,TX_9D,TX_8D,TX_7D,
  TX_HAND_GESP,TX_SCHN_ANGE,TX_SCHW_ANGE,
  TX_OUVE_GESP,TX_OUVE_HAND_GESP,
  TX_PASSE,TX_JA,TX_NEIN,TX_SPIELER_N,TX_COMPUTER,TX_LINKS,TX_RECHTS,
  TX_DRUECKEN,TX_SPIELT,TX_OUVE_HAND,TX_OUVE,TX_HAND,TX_OUVERT,
  TX_UEBERLEGT,TX_GEREIZT_BIS_N,TX_LOESCHEN,TX_SPIELTE,
  TX_GEWONNEN,TX_GEWANN,TX_VERLOREN,TX_VERLOR,TX_N_AUGEN,
  TX_UEBERREIZT,TX_SCHNEIDERFREI,TX_NICHT_SCHWARZ,
  TX_JUNGFRAU,TX_DURCHMARSCH,
  TX_NIEMAND,TX_GEWINNT,TX_VERLIERT,TX_DEN_RAMSCH,
  TX_DAS_NULLSPIEL,TX_DAS_SPIEL_SCHWARZ,TX_MIT_N_ZU_M_AUGEN,
  TX_HAND_F,TX_SPIEL_BEENDEN,TX_SPIELENDE,TX_BEENDET_DAS_SPIEL,TX_AHA,
  TX_LOESCHEN_F,TX_SPIEL,TX_SCHNEIDER,TX_SCHWARZ,TX_ANSAGEN,TX_HINWEIS,
  TX_F1,
  TX_F2,
  TX_F3,
  TX_F4,
  TX_U1,
  TX_U2,
  TX_U3,
  TX_U4,
  TX_U5,
  TX_ANGESAGT_IST,TX_RESULTAT,TX_DER_SPIELWERT_IST,TX_ENDE,
  TX_SPIELE_GEWONNEN_VERLOREN,
  TX_PROTOKOLL,TX_WEITER,TX_PROTOKOLL_T,
  TX_IM_SKAT_IST,
  TX_IM_SKAT_WAR,
  TX_URSPRUENG_SKAT,
  TX_GEREIZT_BIS,
  TX_GEWO_MIT,
  TX_VERL_MIT,
  TX_SPIELLISTE,TX_SPIELLISTE_T,TX_XSKAT,
  TX_COPYRIGHT,TX_URL,
  TX_DOWNLOAD1,TX_DOWNLOAD2,
  TX_ALT1,TX_ALT2,TX_ALT3,TX_ALT4,
  TX_SORTIEREN,TX_AUFWAERTS,TX_ABWAERTS,TX_ALTERNIEREND,TX_SEQUENTIELL,
  TX_NORMAL,
  TX_GEREIZT_BIS_L,TX_GESPIELT_WIRD,TX_LETZTER_STICH,
  TX_DU_BIST_DRAN,TX_VORDEFINIERTES_SPIEL,TX_SPEICHERN,
  TX_FR1,
  TX_FR2,
  TX_FR3,
  TX_FR4,
  TX_OPTIONEN,TX_COMPUTER_T,TX_SPIELSTAERKE,TX_MM,TX_M,TX_0,TX_P,TX_PP,
  TX_NAME,
  TX_VORSCHLAEGE,TX_VARIANTEN,
  TX_RAMSCH_SPIELEN,TX_IMMER,
  TX_RAMSCH_VARIANTEN_T,
  TX_SKAT_GEHT_AN,TX_LETZTEN_STICH,TX_VERLIERER,
  TX_SCHIEBERAMSCH,TX_SKAT_AUFNEHMEN,TX_FERTIG,
  TX_BUBEN_NICHT,
  TX_UNTER_NICHT,
  TX_VERDOPPELT,TX_SCHIEBT,TX_KLOPFT,
  TX_VH_SCHOB,TX_MH_SCHOB,TX_HH_SCHOB,
  TX_KONTRA_SAGEN,TX_MIT_KONTRA,TX_AB18,TX_KONTRA,TX_KONTRA_T,TX_RE,TX_VON,
  TX_WIEDER,TX_SPIEL_WIEDER,TX_MIT_DEN_KARTEN,TX_MIR,
  TX_VORHAND_WECHSELT,TX_START,TX_ZURUECK,
  TX_GESCHWINDIGKEIT,TX_NIMM_STICH_NACH,TX_SEKUNDEN,TX_MAUS_KLICK,
  TX_ABKUERZUNG,TX_FRAGEN,TX_NIE,
  TX_BOCK_RUNDEN,TX_UND_RAMSCH,TX_FORTSETZEN,
  TX_BOCK_EREIGNISSE_T,TX_BOCK_EREIGNIS,TX_N_BOCK_EREIGNISSE,
  TX_BOCK_SPIELE,TX_GRAND_HAND,TX_BOCK_SPIEL,
  TX_VERLOREN_MIT_60,TX_GRAND_HAND_GEWONNEN,
  TX_ERFOLGREICHER_KONTRA,TX_KONTRA_RE_ANGESAGT,
  TX_NNN_IN_SPIELLISTE,TX_N00_IN_SPIELLISTE,
  TX_SPIELWERT_72,TX_SPIELWERT_96,
  TX_BENUTZEROBERFLAECHE,TX_TASTATUR,
  TX_MENU_BUTTON,TX_JEDER,
  TX_WENIG_HINWEISE,
  TX_STICH_KARTEN_VON_LINKS_NACH_RECHTS,
  TX_SPITZE,TX_PLUS2,TX_SPITZE_VERLOREN,TX_SPITZE_PROT,
  TX_SPITZE_F1,
  TX_SPITZE_F2,
  TX_SPITZE_F3,
  TX_NULL_DICHT,TX_REST_BEI_MIR,TX_KARTEN_AUFDECKEN,
  TX_REVOLUTION,TX_KARTEN_AUSTAUSCHEN,
  TX_VORHAND,TX_MITTELHAND,TX_HINTERHAND,
  TX_KLOPFEN,TX_KLOPFEN_T,TX_SPIELWERT_VERDOPPELN,
  TX_SCHENKEN,TX_SCHENKEN_T,TX_SPIEL_VERLOREN_GEBEN,TX_GESCHENK_ANNEHMEN,
  TX_MITSPIELER_LEHNT_AB,TX_GEGNER_GEBEN_AUF,TX_AKZEPTIERT,
  TX_GEBEN,TX_SCHNELL,TX_LANGSAM,
  TX_GRAFIK_UND_TEXT_T,
  TX_BLATT,TX_FRANZOESISCH,TX_FRANZOESISCH_4_FARBEN,
  TX_DEUTSCH,TX_DEUTSCH_4_FARBEN,
  TX_SPRACHE,TX_GERMAN,TX_ENGLISH,
  TX_ALTE_REGELN,
  TX_INTRO1,TX_INTRO2,
  TX_MEHRSPIELER,
  TX_LAN_T,
  TX_EIGENER_TISCH,
  TX_ANDERER_TISCH,
  TX_SPIELER_2,
  TX_SPIELER_3,
  TX_RECHNER_IP,
  TX_TISCH_OEFFNEN,
  TX_TISCH,
  TX_ZUGRIFF_1,
  TX_ZUGRIFF_2,
  TX_MAN_XHOST,
  TX_PLATZ_NEHMEN,
  TX_SCHLIESST_FENSTER,
  TX_IRC_T,
  TX_IRC_VERBINDUNG_HERSTELLEN,
  TX_IRC_KONFIGURATION_SIEHE,
  TX_MAN_XSKAT,
  TX_VERSCHIEDENE_VERSIONEN,
  TX_README_IRC,
  TX_NICHT_HIER,
  TX_NUR_EINER_SOLLTE_GO_SAGEN,
  TX_SOLLTE_DAS_GO_X_SEIN,
  TX_VERBUNDEN_MIT_SPIELER_N,
  TX_WARTEN_AUF_SPIELER_N,
  TX_IRC_INTRO1,
  TX_IRC_INTRO2,
  TX_IRC_INTRO3,
  TX_IRC_INTRO4,
  TX_IRC_INTROX,
  TX_OFFIZIELLE_REGELN,
  TX_POSITION,TX_ALTERNATIV,TX_TURNIER,TX_GEBER,
  TX_UNBEKANNTES_KOMMANDO,
  TX_IRC_HELP01,
  TX_IRC_HELP02,
  TX_IRC_HELP03,
  TX_IRC_HELP04,
  TX_IRC_HELP05,
  TX_IRC_HELP06,
  TX_IRC_HELP07,
  TX_IRC_HELP08,
  TX_IRC_HELP09,
  TX_IRC_HELP10,
  TX_IRC_HELP11,
  TX_IRC_HELP12,
  TX_IRC_HELP13,
  TX_IRC_HELP14,
  TX_IRC_HELP15,
  TX_IRC_HELPXX,
  TX_NUM_TX
};

EXTERN tx_typ textarr[TX_NUM_TX];
EXTERN char spnames[4][2][NUM_LANG][10];
EXTERN char conames[2][2][40];
EXTERN char usrname[2][40];

#endif /* TEXT_H */
