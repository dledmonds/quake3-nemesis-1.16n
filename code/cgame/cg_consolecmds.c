// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"

// Global var to store last killer since cg.killerName is reset upon respawn
char killedby[MAX_QPATH];

void CG_TargetCommand_f( void ) {
	int		targetNum;
	char	test[4];

	targetNum = CG_CrosshairPlayer();
	if (!targetNum ) {
		return;
	}

	trap_Argv( 1, test, 4 );
	trap_SendConsoleCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}



/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer-10)));
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
	CG_Printf ("(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
		(int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2], 
		(int)cg.refdefViewAngles[YAW]);
}

static void CG_ScoresDown_f( void ) {
	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
}

static void CG_ScoresUp_f( void ) {
	cg.showScores = qfalse;
	cg.scoreFadeTime = cg.time;
}

// Statistics window 
static void CG_drawstatsWindow( void ) {
	if (cg.statsWindow == NULL && 
		!cg.intermissionStarted && 
		cgs.clientinfo[ cg.clientNum ].team != TEAM_SPECTATOR) {

		CG_statsWindow( qfalse );
	}
}

static void CG_removestatsWindow( void ) {
	if( cg.statsWindow != NULL && !cg.intermissionStarted ) {
		cg.statsWindow->effects = WFX_SCROLLLEFT;
		//cg.statsWindow->state = WSTATE_SHUTDOWN;
		CG_windowFree( cg.statsWindow );
		cg.statsWindow = NULL;
	}
}

static void CG_TellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}


/*
=============
CG_NextClientId

Gets the next client id (using wrapping if needed).
=============
*/
static int CG_NextClientId( void ) {
	clientInfo_t *yourCi;
	clientInfo_t *ci;
	int i;
	int followId;  // id of client you are currently following (maybe yourself)
	int wrapAroundId;  // id of first matching client so we can wrap around

	followId = cg.snap->ps.clientNum;
	wrapAroundId = -1;
	yourCi = &cgs.clientinfo[ cg.clientNum ];

	if ( yourCi->team == TEAM_FREE || yourCi->team == TEAM_SPECTATOR || yourCi->health > 0 ) {
		return wrapAroundId;
	}

	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		ci = &cgs.clientinfo[i];
		if ( !ci->infoValid || ci->health <= 0 ) {
			continue;
		}

		if ( yourCi->team == ci->team ) {
			// get first matching client
			if ( wrapAroundId == -1 && i != followId ) {
				wrapAroundId = i;
			}

			// higher client id than currently following
			if ( i > followId ) {
				return i;
			}
		}
	}

	return wrapAroundId;
}


/*
=============
CG_PreviousClientId

Gets the previous client id (using wrapping if needed).
=============
*/
static int CG_PreviousClientId( void ) {
	clientInfo_t *yourCi;
	clientInfo_t *ci;
	int i;
	int followId;  // id of client you are currently following (maybe yourself)
	int wrapAroundId;  // id of first matching client so we can wrap around

	followId = cg.snap->ps.clientNum;
	wrapAroundId = -1;
	yourCi = &cgs.clientinfo[ cg.clientNum ];

	if ( yourCi->team == TEAM_FREE || yourCi->team == TEAM_SPECTATOR || yourCi->health > 0 ) {
		return wrapAroundId;
	}

	for ( i = MAX_CLIENTS - 1; i >= 0 ; i-- ) {
		ci = &cgs.clientinfo[i];
		if ( !ci->infoValid || ci->health <= 0 ) {
			continue;
		}

		if ( yourCi->team == ci->team ) {
			// get first matching client
			if ( wrapAroundId == -1 && i != followId ) {
				wrapAroundId = i;
			}

			// lower client id than currently following
			if ( i < followId ) {
				return i;
			}
		}
	}

	return wrapAroundId;
}


/*
=============
CG_FollowTeamNext

Same as follow command, but always chooses a teammate.
Will choose NEXT teammate if one exists.
=============
*/
static void CG_FollowTeamNext( void ) {
	int clientId;

	clientId = CG_NextClientId();

	if ( clientId != -1 ) {
		trap_SendConsoleCommand( va( "follow %i", clientId ) );
	}
}


/*
=============
CG_FollowTeamPrevious

Same as follow command, but always chooses a teammate.
Will choose previous teammate if one exists.
=============
*/
static void CG_FollowTeamPrevious( void ) {
	int clientId;

	clientId = CG_PreviousClientId();

	if ( clientId != -1 ) {
		trap_SendConsoleCommand( va( "follow %i", clientId ) );
	}
}

/*
=============
CG_SubstituteName

Substitutes any occurrences of #n with your client name
=============
*/
static char *CG_SubstituteName( char* message ) {

	if ( Q_stricstr( message, "#n" ) ) {
		clientInfo_t *ourCi;

		ourCi = &cgs.clientinfo[ cg.clientNum ];
	
		return Q_replace( message, "#n", ourCi->name );

	} else {
		return message;
	}
}


/*
=============
CG_SubstituteHealth

Substitutes any occurrences of #h with your client health
=============
*/
static char *CG_SubstituteHealth( char* message ) {

	if ( Q_stricstr( message, "#h" ) ) {
		clientInfo_t *ourCi;
		char string[MAX_STRING_TOKENS];
	
		ourCi = &cgs.clientinfo[ cg.clientNum ];
		strcpy( string, va("%i", ourCi->health ) );

		return Q_replace( message, "#h", string );

	} else {
		return message;
	}
}


/*
=============
CG_SubstituteArmor

Substitutes any occurrences of #a with your client armor
=============
*/
static char *CG_SubstituteArmor( char* message ) {

	if ( Q_stricstr( message, "#a" ) ) {
		clientInfo_t *ourCi;
		char string[MAX_STRING_TOKENS];
	
		ourCi = &cgs.clientinfo[ cg.clientNum ];
		strcpy( string, va("%i", ourCi->armor ) );

		return Q_replace( message, "#a", string );

	} else {
		return message;
	}
}


/*
=============
CG_SubstituteScore

Substitutes any occurrences of #s with your client score
=============
*/
static char *CG_SubstituteScore( char* message ) {

	if ( Q_stricstr( message, "#s" ) ) {
		clientInfo_t *ourCi;
		char string[MAX_STRING_TOKENS];
	
		ourCi = &cgs.clientinfo[ cg.clientNum ];
		strcpy( string, va("%i", ourCi->score ) );
	
		return Q_replace( message, "#s", string );

	} else {
		return message;
	}
}


/*
=============
CG_SubstituteTeam

Substitutes any occurrences of #t with your team
=============
*/
static char *CG_SubstituteTeam( char* message ) {

	if ( Q_stricstr( message, "#t" ) ) {
		clientInfo_t *ourCi;
		char *teamString;
	
		ourCi = &cgs.clientinfo[ cg.clientNum ];
	
		if ( ourCi->team == TEAM_RED ) {
			teamString = va( "Red Team" );
		} else if ( ourCi->team == TEAM_BLUE ) {
			teamString = va( "Blue Team" );
		} else if ( ourCi->team == TEAM_FREE ) {
			teamString = va( "Free Team" );
		} else {
			teamString = "No Team";
		}
	
		return Q_replace( message, "#t", teamString );

	} else {
		return message;
	}
}


/*
=============
CG_SubstituteItem

Substitutes any occurrences of #i with last item picked up
=============
*/
static char *CG_SubstituteItem( char* message ) {

	if ( Q_stricstr( message, "#i" ) ) {
		char *itemName;
		
		if ( cg.itemPickup == 0 ) {
			itemName = "";

		} else if ( !strcmp( bg_itemlist[ cg.itemPickup ].classname, "item_armor_combat" ) ) {
			itemName = va( "Yellow Armor" );

		} else if ( !strcmp( bg_itemlist[ cg.itemPickup ].classname, "item_armor_body" ) ) {
			itemName = va( "Red Armor" );

		} else if ( !strcmp( bg_itemlist[ cg.itemPickup ].classname, "item_health_mega" ) ) {
			itemName = va( "Mega Health" );

		} else if ( !strcmp( bg_itemlist[ cg.itemPickup ].classname, "holdable_teleporter" ) ) {
			itemName = va( "Teleporter" );
			
		} else if ( !strcmp( bg_itemlist[ cg.itemPickup ].classname, "holdable_medkit" ) ) {
			itemName = va( "Medikit" );

		} else if ( !strcmp( bg_itemlist[ cg.itemPickup ].classname, "item_quad" ) ) {
			itemName = va( "Quad" );
			
		} else if ( !strcmp( bg_itemlist[ cg.itemPickup ].classname, "item_enviro" ) ) {
			itemName = va( "Environment Suit" );

		} else if ( !strcmp( bg_itemlist[ cg.itemPickup ].classname, "item_haste" ) ) {
			itemName = va( "Haste" );

		} else if ( !strcmp( bg_itemlist[ cg.itemPickup ].classname, "item_invis" ) ) {
			itemName = va( "Invisibility" );

		} else if ( !strcmp( bg_itemlist[ cg.itemPickup ].classname, "item_regen" ) ) {
			itemName = va( "Regeneration" );

		} else if ( !strcmp( bg_itemlist[ cg.itemPickup ].classname, "item_flight" ) ) {
			itemName = va( "Flight" );

		} else {
			itemName = "";
		}

		return Q_replace( message, "#i", itemName );

	} else {
		return message;
	}
}


/*
=============
CG_SubstituteAttacker

Substitutes any occurrences of #l with last attacker name
=============
*/
static char *CG_SubstituteAttacker( char* message ) {

	if ( Q_stricstr( message, "#l" ) ) {
		clientInfo_t *attackerCi;

		attackerCi = &cgs.clientinfo[ CG_LastAttacker() ];

		if ( attackerCi->infoValid ) {
			return Q_replace( message, "#l", attackerCi->name );
		} else {
			return Q_replace( message, "#l", "" );
		}

	} else {
		return message;
	}
}


/*
=============
CG_SubstituteCHClient

Substitutes any occurrences of #x with crosshair client name
=============
*/
static char *CG_SubstituteCHClient( char* message ) {

	if ( Q_stricstr( message, "#x" ) ) {
		clientInfo_t *ci;
	
		ci = &cgs.clientinfo[ cg.crosshairClientNum ];

		if( cg.crosshairClientNum == cg.clientNum ) {
			return Q_replace( message, "#x", va( "%s", "" ) );
		}
	
		return Q_replace( message, "#x", ci->name );

	} else {
		return message;
	}
}


/*
=============
CG_SubstituteKiller

Substitutes any occurrences of #k killer name
=============
*/
static char *CG_SubstituteKiller( char* message ) {

	if ( Q_stricstr( message, "#k" ) ) {
		return Q_replace( message, "#k", killedby );

	} else {
		return message;
	}
}


/*
=============
CG_SayTeam

Runs any substituions on the say_team text before sending it
to the server.
=============
*/
static void CG_SayTeam( void ) {
	char command[MAX_STRING_TOKENS];
	char message[MAX_STRING_TOKENS];
	char *newMessage;

	trap_Args( message, MAX_STRING_TOKENS );
	newMessage = message;

	CG_SubstituteName( newMessage );
	CG_SubstituteHealth( newMessage );
	CG_SubstituteArmor( newMessage );
	CG_SubstituteScore( newMessage );
	CG_SubstituteTeam( newMessage );
	CG_SubstituteItem( newMessage );
	CG_SubstituteAttacker( newMessage );
	CG_SubstituteCHClient( newMessage );
	CG_SubstituteKiller( newMessage );

	Com_sprintf( command, MAX_STRING_TOKENS, "say_team %s", newMessage );
	trap_SendClientCommand( command );
}

/*
=============
CG_SayPublic

Sames as CG_SayTeam except these are public messages.
=============
*/
static void CG_SayPublic( void ) {
	char command[MAX_STRING_TOKENS];
	char message[MAX_STRING_TOKENS];
	char *newMessage;

	trap_Args( message, MAX_STRING_TOKENS );
	newMessage = message;

	CG_SubstituteName( newMessage );
	CG_SubstituteHealth( newMessage );
	CG_SubstituteArmor( newMessage );
	CG_SubstituteScore( newMessage );
	CG_SubstituteTeam( newMessage );
	CG_SubstituteItem( newMessage );
	CG_SubstituteAttacker( newMessage );
	CG_SubstituteCHClient( newMessage );
	CG_SubstituteKiller( newMessage );

	Com_sprintf( command, MAX_STRING_TOKENS, "say %s", newMessage );
	trap_SendClientCommand( command );
}

/*
=============
CG_SayLeet

Sames as CG_SayPublic except these messges are leeted as well.
=============
*/
static void CG_SayLeet( void ) {
	char command[MAX_STRING_TOKENS];
	char message[MAX_STRING_TOKENS];
	char *leetMessage;
	char *newMessage;
	int ran, lastRan;

	int charToNumPercentage = 60;
	int charSwitchCase = 20;

	trap_Args( message, MAX_STRING_TOKENS );
	newMessage = message;
	leetMessage = message;

	Q_replace( leetMessage, "hacker", "haX0r" );
	Q_replace( leetMessage, "own", "pWn" );
	Q_replace( leetMessage, "you", "joo" );
	Q_replace( leetMessage, "fear", "pheer" );

	lastRan = cg.time;
	while ( *leetMessage ) {
		ran = abs( Q_rand( &lastRan ) % 100 );  // random number from 0 - 99
		lastRan = ran;

		if ( ( *leetMessage == 'o' || *leetMessage == 'O' ) && ran <= charToNumPercentage ) {
			*leetMessage++ = '0';
			continue;
		}
		if ( ( *leetMessage == 'e' || *leetMessage == 'E' ) && ran <= charToNumPercentage ) {
			*leetMessage++ = '3';
			continue;
		}
		if ( ( *leetMessage == 'a' || *leetMessage == 'A' ) && ran <= charToNumPercentage ) {
			*leetMessage++ = '4';
			continue;
		}
		if ( ( *leetMessage == 's' || *leetMessage == 'S' ) && ran <= charToNumPercentage ) {
			*leetMessage++ = '5';
			continue;
		}
		if ( ( *leetMessage == 'l' || *leetMessage == 'L' ) && ran <= charToNumPercentage ) {
			*leetMessage++ = '1';
			continue;
		}

		// upper to lower
		if ( (int)*leetMessage >= 65 && (int)*leetMessage <= 90 && ran <= charSwitchCase ) {
			*leetMessage = (char)(*leetMessage + 32);
			leetMessage++;
			continue;
		}

		// lower to upper
		if ( (int)*leetMessage >= 97 && (int)*leetMessage <= 122 && ran <= charSwitchCase ) {
			*leetMessage = (char)(*leetMessage - 32);
			leetMessage++;
			continue;
		}

		leetMessage++;
	}

	CG_SubstituteName( newMessage );
	CG_SubstituteHealth( newMessage );
	CG_SubstituteArmor( newMessage );
	CG_SubstituteScore( newMessage );
	CG_SubstituteTeam( newMessage );
	CG_SubstituteItem( newMessage );
	CG_SubstituteAttacker( newMessage );
	CG_SubstituteCHClient( newMessage );
	CG_SubstituteKiller( newMessage );

	Com_sprintf( command, MAX_STRING_TOKENS, "say %s", newMessage );
	trap_SendClientCommand( command );
}

/*
=============
CG_PlayerList

Dumps list of player names with their id
=============
*/
static void CG_PlayerList( void ) {
	int i;
	clientInfo_t *ci;
	const char *teamString;

	// if this is Nemesis, pass it to the server
	if (CG_isNemesis()) {
		trap_SendClientCommand("players");
		return;
	}

	CG_Printf( "^3----------------------------------------\n" );
	CG_Printf( "^3TEAM  ID  NAME\n" );
	CG_Printf( "^3----------------------------------------\n" );
	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		ci = &cgs.clientinfo[i];
		if ( !ci->infoValid ) {
			continue;
		}

		if ( ci->team == TEAM_RED ) {
			teamString = "^1Red  ";
		} else if ( ci->team == TEAM_BLUE ) {
			teamString = "^5Blue ";
		} else if ( ci->team == TEAM_SPECTATOR ) {
			teamString = "^7Spec ";
		} else if ( ci->team == TEAM_FREE ) {
			teamString = "^3Free ";
		} else {
			teamString = "^7???? ";
		}

		CG_Printf( "%-5s ^3%-3i ^7%s\n", teamString, i, ci->name );
	}
}


/*
=============
CG_PersistentFollow

Same as follow command, but continues to follow after
death/frozen through periodic checks.
=============
*/
static void CG_PersistentFollow( int playerId ) {
	clientInfo_t *ci;
	clientInfo_t *ourCi;

	ourCi = &cgs.clientinfo[ cg.clientNum ];
	if ( ourCi->team != TEAM_SPECTATOR ) {
		CG_Printf( "Must be a spectator to use persistent follow mode\n" );
		return;
	}

	if ( playerId == cg.clientNum ) {
		return;
	}

	ci = &cgs.clientinfo[ playerId ];
	if ( !ci->infoValid ) {
		CG_Printf( "Invalid client at slot %i\n", playerId );
		return;
	}

	trap_SendClientCommand( va( "follow %i\n", playerId ) );

	cg.persistentFollow = qtrue;		// enabled
	cg.persistentFollowId = playerId;	// who we follow
	cg.persistentFollowTime = cg.time;	// used to time retries when player is frozen

	CG_Printf( "Persistent follow enabled for %s\n", ci->name );
}


/*
=============
CG_RecordPlayer

Records the player with given id throughout current map
and keeps following them even when dead/frozen through 
periodic checks.
=============
*/
static void CG_RecordPlayer( void ) {
	const char *sPlayerId;
	int playerId;
	clientInfo_t *ci;
	clientInfo_t *ourCi;
	//char playerName[MAX_QPATH];

	ourCi = &cgs.clientinfo[ cg.clientNum ];
	if ( ourCi->team != TEAM_SPECTATOR ) {
		CG_Printf( "Must be a spectator to use player record mode\n" );
		return;
	}

	sPlayerId = CG_Argv( 1 );
	if ( Q_stricmp( sPlayerId, "" ) == 0 ) {
		CG_Printf( "\recordplayer <ID> <DEMONAME>\n" );
		return;
	}

	playerId = atoi( sPlayerId );

	if ( playerId == cg.clientNum ) {
		return;
	}

	ci = &cgs.clientinfo[ playerId ];

	if ( !ci->infoValid ) {
		CG_Printf( "Invalid client at slot %i\n", playerId );
		return;
	}

	CG_PersistentFollow( playerId );

	CG_NamedRecord(CG_Argv(2));

	cg.playerRecord = qtrue;
	CG_Printf( "Player recoding enabled for %s\n", ci->name );
}


/*
=============
CG_StartPersistentFollow

Same as follow command, but continues to follow after
death/frozen through periodic checks.
=============
*/
static void CG_StartPersistentFollow( void ) {
	const char *sPlayerId;
	int playerId;

	sPlayerId = CG_Argv( 1 );
	if ( Q_stricmp( sPlayerId, "" ) == 0 ) {
		CG_Printf( "startPersistentFollow <ID>\n" );
		return;
	}

	playerId = atoi( sPlayerId );
	CG_PersistentFollow( playerId );
}


/*
=============
CG_StopPersistentFollow

Turns off the persistent following of a player.
=============
*/
static void CG_StopPersistentFollow( void ) {
	if ( !cg.persistentFollow ) {
		CG_Printf( "Persistent follow not enabled\n" );
	}

	cg.persistentFollow = qfalse;
	CG_Printf( "Persistent follow disabled\n" );
}


/*
=============
CG_Record

Issues record command but wraps it in g_syncronousclients calls
=============
*/
static void CG_Record( void ) {
	CG_NamedRecord(CG_Argv(1));
}


/*
=============
CG_LookupIP_f

Performs a county lookup from IP address
=============
*/
static void CG_LookupIP_f(void) {
	const char *ipStr;
	int tld;

	ipStr = CG_Argv(1);
	if (Q_stricmp(ipStr, "") == 0) {
		CG_Printf( "iplookup <IP>\n" );
		return;
	}

	tld = CG_LookupIP(ipStr);
	if (tld != -1) {
		const char *code, *name;

		code = CG_LookupCountryCode(tld);
		name = CG_LookupCountryName(tld);

		CG_Printf("%s detected as %s (%s)\n", ipStr, name, code);
	}
}


// Try not to use a command over 18 letters(for print reasons) unless you change
// -18s in CG_Help.

// Help toggled using command "chelp", some mods use "help" so we must use chelp
// to prevent conflicts between the 2 commands.

// Help console
static const helpTable_t		Table[] = {
	{ "cg_railTrail",
	  "cg_railTrail <1|2|3|4>\n\nSet the style of rail trails to\n1 - Q3 Default\n2 - Alternate Fire Trail\n3 - Q3 Old Style Rail\n4 - QF Style Trail" },

	{ "cg_trueLightning",
	  "cg_trueLightning <0.0 to 1.0>\n\nCPMA true lightning\n1.0 shows beam hitting target directly (straight line)" },

	{ "cg_enemyColor",
      "cg_enemyColor <STRING|NUMBER|HEX>\n\nForces enemy color (when using nms skins on cg_enemyModel only)\nSTRING is one of red, green, yellow, blue, cyan, magenta, white, orange, black, ltgrey, mdgrey or dkgrey\nNUMBER is 1 to 12\nHEX is a hexidecimal number as in 0xffffff" },

	{ "cg_enemyModel",
	  "cg_enemyModel <MODEL[/SKIN]>\n\nSet enemy model with optional skin\nModel can be any standard or custom model\nSkin must be nms (standard models only) for coloring to work" },

	{ "cg_specEnemyModel",
	  "cg_specEnemyModel <0|1>\n\nWhether to use enemy model when in spectator mode\n0 - use client models\n1 - use enemy models for all" },

	{ "cg_ammoCheck",
	  "cg_ammoCheck <0|1>\n\nOSP sytle ammo check\n0 - Standard Q3A ammo check\n1 - Dont allow changing to weapons with 0 ammo" },

	{ "cg_drawAcc",
	  "cg_drawAcc <0|1>\n\nClient accuracy counter in top right\n0 - off\n1 - on" },

	{ "cg_useTeamIcons",
	  "cg_useTeamIcons <0|1>\n\nWhether or not to display team icons on the CPMA HUD\n0 - no icons\n1 - show icons" },

	{ "cg_drawSpeed",
	  "cg_drawSpeed <0|1>\n\nClient speed in top right, displayed in units per second\n0 - off\n1 - on" },

	{ "cg_drawSpeedBar",
	  "cg_drawSpeedBar <0|1>\n\nClient speed using colored bar in top right\n0 - off\n1 - on" },

	{ "cg_enableBreath",
	  "cg_enableBreath <0|1>\n\nShow player breath puffs in freeze games\n0 - off\n1 - on" },

	{ "cg_scoreBoardType",
	  "cg_scoreBoardType <0|1>\n\nSelect a scoreboard style\n0 - standard\n1 - OSP style" },

	{ "cg_hudType",
	  "cg_hudType <0|1|2>\n\nSelect a HUD type\n0 - Standard HUD\n1 - CPMA HUD\n2 - Super HUD" },

	{ "cg_hudFile",
	  "cg_hudFile <STRING>\n\nSet HUD file for super HUD (from hud directory)" },

	{ "cg_hudDebug",
	  "cg_hudDebug <0|1>\n\nEnable debug mode for super HUD\n0 - enable debug\n1 - disable debug" },

	{ "cg_autoScreenshot",
	  "cg_autoScreenshot <0|1>\n\nAutomatic taking of screenshot at end of a game\n0 - off\n1 - on" },

	{ "cg_autoRecord",
	  "cg_autoRecord <0|1>\n\nAutomatic demo recording\n0 - off\n1 - on" },

	{ "cg_thawSoundType",
	  "cg_thawSoundType <0|1>\n\nChoose thaw sound for freeze games\n0 - default gib sound\n1 - glass break" },

	{ "cg_teamInfoType",
	  "cg_teamInfoType <0|1|2>\n\nChoose team display when in crosshairs\n0 - name, 1 - name|health|armor, 2 - name|health, armor gauges" },

	{ "cg_iceShells",
	  "cg_iceShells <0|1>\n\nWhether to display ice shells in freeze games\n0 - off, 1 - on" },

	{ "cg_noChatBeep",
	  "cg_noChatBeep <0|1>\n\nWhether to beep when a player sends a message\n0 - off, 1 - on" },

	{ "cg_noFriendShader",
	  "cg_noFriendShader <0|1>\n\nWhether to draw the small triangle over team mates heads\n0 - off, 1 - on" },

	{ "cg_noConnectionShader",
	  "cg_noConnectionShader <0|1>\n\nWhether to draw the connection interrupted icon over lagging players\n0 - off, 1 - on" },

	{ "cg_autoAnnounce",
	  "cg_autoAnnounce <0 - 2047>\n\nAutomatically announce to team mates what powerup you picked up\n0 - off, above 0 - on\n\nAdd up the following numbers to get the correct value -\n1    - Yellow armor\n2    - Red armor\n4    - Mega Health\n8    - Personal Teleporter\n16   - Medikit\n32   - Quad\n64   - Haste\n128  - Invisibility\n256  - Regeneration\n512  - Flight\n1024 - Environmental Suit" },

	{ "cg_crosshairColor",
	  "cg_crosshairColor <STRING|NUMBER|HEX>\n\nChange color of crosshair\nSTRING is one of red, green, yellow, blue, cyan, magenta, white, orange, black, ltgrey, mdgrey or dkgrey\nNUMBER is 1 to 12\nHEX is a hexidecimal number as in 0xffffff" },

	{ "cg_crosshairAlpha",
	  "cg_crossHairAlpha <0.0 - 1.0>\n\nChanges the opacity of the crosshair\n0 - fully opaque\n1 - fully solid" },

	{ "cg_consoleFilter",
	  "cg_consoleFilter <0 - 7>\n\nSet visible console chat\n0 - see all, above 0 starts filtering\n\nAdd up the following numbers to get the correct value -\n1    - no public chat\n2    - no team chat\n4    - no server messages" },

	{ "cg_chatFilter",
	  "cg_chatFilter <0 - 7>\n\nSet visible chat\n0 - see all, above 0 starts filtering\n\nAdd up the following numbers to get the correct value -\n1    - no public chat\n2    - no team chat\n4    - no server messages" },

	{ "cg_noProjectileTrail",
	  "cg_noProjectileTrail <0|1>\n\n0 - display weapon trails\n1 - do not display weapon trails" },

	{ "cg_scorePlum",
	  "cg_scorePlum <0|1>\n\n0 - score plums off\n1 - score plums on" },

	{ "cg_specialEffects",
	  "cg_specialEffects <NUMBER>\n\n0 - no effects\n1 - sparks\n2 - plasma trail\n3 - sparks and plasma trail" },

	{ "players",
	  "players\n\nPlayer listings showing team, player slot and player name" },

	{ "startrecord",
	  "startRecord\n\nStarts recording a demo (switching g_syncronousclients automatically)" },

	{ "startpersistentfollow",
	  "startPersistentFollow <X>\n\nFollows player with slot X whenever they are alive (not frozen)" },

	{ "stoppersistentfollow",
	  "stopPersostentFollow\n\nDisables persistent follow mode" },

	{ "recordplayer",
	  "recordPlayer <X> [DEMONAME]\n\nStarts recording a demo of player at slot X (uses persistent follow mode)\nOptional parameter of DEMONAME" },

	{ "followTeamNext",
	  "followTeamNext\n\nFollow the next (alive) player on your team" },

	{ "followTeamPrev",
	  "followTeamPrev\n\nFollow the previous (alive) player on your team" },

	{ "say",
	  "say <MESSAGE>\n\nBroadcast the message to other players substituing the following special tokens\n\n#a - armor\n#h - health\n#i - last powerup collected\n#k - last player to kill you\n#l - last player to hit you\n#s - score\n#t - team\n#x - client in crosshair" },

	{ "say_team",
	  "say_team <MESSAGE>\n\nBroadcast the message to other players substituing the following special tokens\n\n#a - armor\n#h - health\n#i - last powerup collected\n#k - last player to kill you\n#l - last player to hit you\n#s - score\n#t - team\n#x - client in crosshair" },

	{ "say_leet",
	  "say_leet <MESSAGE>\n\nConverts message to 'leet speak' then broadcasts to other players.\nYou can also use the chat token - see say or say_team help for more information" }
};

static const int TableSize = sizeof( Table ) / sizeof( Table[0] );

/*
=========
CG_Help
=========
*/
void CG_Help( const char *cmd ) {
	int	i;
	qboolean foundCmd;
	const helpTable_t *cv;

	foundCmd = qfalse;
	CG_Printf( "^5------------------------------------------------------------------\n" );
	for ( i = 0; i < TableSize; i++ ) {
		cv = &Table[i];

		if ( Q_stricstr( cv->varName, cmd ) ) {
			CG_Printf( "^3%s\n", cv->help );
			foundCmd = qtrue;
			break;
		}
	}

	if ( !foundCmd ) {
		CG_Printf( "^3Unrecognised command '%s'\n", cmd );
	}

	CG_Printf( "^5------------------------------------------------------------------\n" );
}


/*
=============
CG_Help_f

Runs help command passing it the command help requested on
=============
*/
static void CG_Help_f( void ) {
	const char	*cmd;

	cmd = CG_Argv( 1 );

	if ( cmd[0] ) {
		CG_Help( cmd );

	} else {
		int		i, j, perRow, perCvar, spaces;
		const	helpTable_t *cv;

		CG_Printf( "^3Cvars and Command for %s - type /chelp <COMMAND> for more details\n", MOD_NAME );
		CG_Printf( "^5------------------------------------------------------------------\n" );

		perRow = 3;
		perCvar = (int)( 66 / perRow );

		for ( i = 0; i < TableSize; i++ ) {
			if ( i != 0 && i%perRow == 0 ) {
				CG_Printf( "\n" );
			}

			cv = &Table[i];
			spaces = perCvar - strlen( cv->varName );
			CG_Printf( "^3%s", cv->varName );
			for ( j = 0; j < spaces; j++ ) {
				CG_Printf( " " );
			}
		}

		CG_Printf( "^5------------------------------------------------------------------\n" );
	}
}


typedef struct {
	char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	{ "followteamnext", CG_FollowTeamNext },		// follow next teammate
	{ "followteamprev", CG_FollowTeamPrevious },	// follow previous teammates
	{ "say_team", CG_SayTeam },						// team chat substitutions
	{ "say", CG_SayPublic },						// public chat substitutions
	{ "say_leet", CG_SayLeet },						// leet chat substitutions
	{ "players", CG_PlayerList },					// list of player names and id
	{ "startrecord", CG_Record },					// records a demo
	{ "recordplayer", CG_RecordPlayer },			// records given client id
	{ "startpersistentfollow", CG_StartPersistentFollow },
	{ "stoppersistentfollow", CG_StopPersistentFollow },
	{ "iplookup", CG_LookupIP_f },					// do an IP lookup for country
	{ "chelp", CG_Help_f },							// client help
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
	{ "-scores", CG_ScoresUp_f },

	{ "+cstats", CG_drawstatsWindow },
	{ "-cstats", CG_removestatsWindow },

#ifdef NMS_MULTIVIEW
	{ "mvdel", CG_mvDelete_f },						// multiview delete
	{ "mvnew", CG_mvNew_f },						// multiview add
#endif

	{ "+zoom", CG_ZoomDown_f },
	{ "-zoom", CG_ZoomUp_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "weapon", CG_Weapon_f },
	{ "tell_target", CG_TellTarget_f },
	{ "tell_attacker", CG_TellAttacker_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "loaddefered", CG_LoadDeferredPlayers }	// spelled wrong, but not changing for demo...
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char	*cmd;
	int		i;

	cmd = CG_Argv(0);

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	int		i;

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		trap_AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand ("kill");
	trap_AddCommand ("say");
	trap_AddCommand ("say_team");
	trap_AddCommand ("say_leet");
	trap_AddCommand ("give");
	trap_AddCommand ("god");
	trap_AddCommand ("notarget");
	trap_AddCommand ("noclip");
	trap_AddCommand ("team");
	trap_AddCommand ("follow");
	trap_AddCommand ("levelshot");
	trap_AddCommand ("addbot");
	trap_AddCommand ("setviewpos");
	trap_AddCommand ("vote");
	trap_AddCommand ("callvote");
	trap_AddCommand ("loaddefered");	// spelled wrong, but not changing for demo
}
