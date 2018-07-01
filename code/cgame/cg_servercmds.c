// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

#include "cg_local.h"
#include "..\game\bg_promode.h"

/*
=================
CG_ParseScores

=================
*/
static void CG_ParseScores( void ) {
	int		i, powerups;

	cg.numScores = atoi( CG_Argv( 1 ) );
	if ( cg.numScores > MAX_CLIENTS ) {
		cg.numScores = MAX_CLIENTS;
	}

	cg.teamScores[0] = atoi( CG_Argv( 2 ) );
	cg.teamScores[1] = atoi( CG_Argv( 3 ) );

	memset( cg.scores, 0, sizeof( cg.scores ) );
	for ( i = 0 ; i < cg.numScores ; i++ ) {
		cg.scores[i].client = atoi( CG_Argv( i * 6 + 4 ) );
		cg.scores[i].score = atoi( CG_Argv( i * 6 + 5 ) );
		cg.scores[i].ping = atoi( CG_Argv( i * 6 + 6 ) );
		cg.scores[i].time = atoi( CG_Argv( i * 6 + 7 ) );
		cg.scores[i].scoreFlags = atoi( CG_Argv( i * 6 + 8 ) );
		powerups = atoi( CG_Argv( i * 6 + 9 ) );

		if ( cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS ) {
			cg.scores[i].client = 0;
		}
		cgs.clientinfo[ cg.scores[i].client ].score = cg.scores[i].score;
		cgs.clientinfo[ cg.scores[i].client ].powerups = powerups;
	}

}

/*
=================
CG_ParseTeamInfo

=================
*/
static void CG_ParseTeamInfo( void ) {
	int		i;
	int		client;

	numSortedTeamPlayers = atoi( CG_Argv( 1 ) );

	for ( i = 0 ; i < numSortedTeamPlayers ; i++ ) {
		client = atoi( CG_Argv( i * 6 + 2 ) );

		sortedTeamPlayers[i] = client;

		cgs.clientinfo[ client ].location = atoi( CG_Argv( i * 6 + 3 ) );
		cgs.clientinfo[ client ].health = atoi( CG_Argv( i * 6 + 4 ) );
		cgs.clientinfo[ client ].armor = atoi( CG_Argv( i * 6 + 5 ) );
		cgs.clientinfo[ client ].curWeapon = atoi( CG_Argv( i * 6 + 6 ) );
		cgs.clientinfo[ client ].powerups = atoi( CG_Argv( i * 6 + 7 ) );
	}
}


/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo( void ) {
	const char	*info;
	char	*mapname;
	char	*tmp;

	info = CG_ConfigString( CS_SERVERINFO );
	cgs.gametype = atoi( Info_ValueForKey( info, "g_gametype" ) );
	cgs.dmflags = atoi( Info_ValueForKey( info, "dmflags" ) );
	cgs.teamflags = atoi( Info_ValueForKey( info, "teamflags" ) );
	cgs.fraglimit = atoi( Info_ValueForKey( info, "fraglimit" ) );
	cgs.capturelimit = atoi( Info_ValueForKey( info, "capturelimit" ) );
	cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
	cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	cgs.fastRail = atoi( Info_ValueForKey( info, "g_fastRail" ) );

	// Nemesis detection
	tmp = Info_ValueForKey( info, "nms_proMode" );
	if (strlen(tmp) != 0) {
		cgs.isNemesis = qtrue;
	} else {
		cgs.isNemesis = qfalse;

		// make sure CPM is disabled as other servers make use of the same
		// CS slot we do for detecting promode (OSP for instance).
		CPM_UpdateSettings(0);
	}

	// NG detection
	tmp = Info_ValueForKey( info, "g_gameMod" );
	if (strlen(tmp) != 0) {
		cgs.isNoGhost = qtrue;
	} else {
		cgs.isNoGhost = qfalse;
	}
	cgs.g_gameMod = atoi( tmp );

	cgs.freezelimit = atoi( Info_ValueForKey( info, "freezelimit" ) );	// Nemesis - DA Freeze
	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );
	Q_strncpyz( cgs.modName, Info_ValueForKey( info, "gamename" ), sizeof( cgs.modName ) );
}

/*
==================
CG_ParseWarmup
==================
*/
static void CG_ParseWarmup( void ) {
	const char	*info;
	int			warmup;

	info = CG_ConfigString( CS_WARMUP );

	warmup = atoi( info );
	cg.warmupCount = -1;

	if ( warmup == 0 && cg.warmup ) {

	} else if ( warmup > 0 && cg.warmup <= 0 ) {
		trap_S_StartLocalSound( cgs.media.countPrepareSound, CHAN_ANNOUNCER );
	}

	cg.warmup = warmup;
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues( void ) {
	const char *s;

	cgs.scores1 = atoi( CG_ConfigString( CS_SCORES1 ) );
	cgs.scores2 = atoi( CG_ConfigString( CS_SCORES2 ) );
	cgs.levelStartTime = atoi( CG_ConfigString( CS_LEVEL_START_TIME ) );
	s = CG_ConfigString( CS_FLAGSTATUS );
	cgs.redflag = s[0] - '0';
	cgs.blueflag = s[1] - '0';
	cg.warmup = atoi( CG_ConfigString( CS_WARMUP ) );
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified( void ) {
	const char	*str;
	int		num;

	num = atoi( CG_Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	trap_GetGameState( &cgs.gameState );

	// look up the individual string that was modified
	str = CG_ConfigString( num );

	// do something with it if necessary
	if ( num == CS_MUSIC ) {
		CG_StartMusic();
	} else if ( num == CS_SERVERINFO ) {
		CG_ParseServerinfo();
	} else if ( num == CS_WARMUP ) {
		CG_ParseWarmup();
	} else if ( num == CS_SCORES1 ) {
		cgs.scores1 = atoi( str );
	} else if ( num == CS_SCORES2 ) {
		cgs.scores2 = atoi( str );
	} else if ( num == CS_WARMUP ) {
		CG_ParseWarmup();
	} else if ( num == CS_LEVEL_START_TIME ) {
		cgs.levelStartTime = atoi( str );
	} else if ( num == CS_VOTE_TIME ) {
		cgs.voteTime = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_YES ) {
		cgs.voteYes = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_NO ) {
		cgs.voteNo = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_STRING ) {
		Q_strncpyz( cgs.voteString, str, sizeof( cgs.voteString ) );
	} else if ( num == CS_INTERMISSION ) {
		cg.intermissionStarted = atoi( str );
	} else if ( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS ) {
		cgs.gameModels[ num-CS_MODELS ] = trap_R_RegisterModel( str );
	} else if ( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_MODELS ) {
		if ( str[0] != '*' ) {	// player specific sounds don't register here
			cgs.gameSounds[ num-CS_SOUNDS] = trap_S_RegisterSound( str );
		}
		// This is a bit hackish but its the only way to do it
		// without modifying communication on both client/server.
		if( Q_stricmp( str, "sound/teamplay/flagret_red.wav" ) == 0 ) {
			cg.flagSoundIndex[0] = cgs.gameSounds[ num-CS_SOUNDS]; 
		}
		if( Q_stricmp( str, "sound/teamplay/flagret_blu.wav" ) == 0 ) {
			cg.flagSoundIndex[1] = cgs.gameSounds[ num-CS_SOUNDS]; 
		}
		if( Q_stricmp( str, "sound/teamplay/flagcap_red.wav" ) == 0 ) {
			cg.flagSoundIndex[2] = cgs.gameSounds[ num-CS_SOUNDS]; 
		}
		if( Q_stricmp( str, "sound/teamplay/flagcap_blu.wav" ) == 0 ) {
			cg.flagSoundIndex[3] = cgs.gameSounds[ num-CS_SOUNDS]; 
		}
	} else if ( num >= CS_PLAYERS && num < CS_PLAYERS+MAX_CLIENTS ) {
		CG_NewClientInfo( num - CS_PLAYERS );
	} else if ( num == CS_FLAGSTATUS ) {
		// format is rb where its red/blue, 0 is at base, 1 is taken, 2 is dropped
		cgs.redflag = str[0] - '0';
		cgs.blueflag = str[1] - '0';
	}
	// CPM
		else if ( num == CS_PRO_MODE )
		{
			CPM_UpdateSettings( (atoi(str)) );
		}
	// !CPM
		
}


/*
=======================
CG_AddToChat

Add a rotating list of chat messages
=======================
*/
static void CG_AddToChat( const char *str ) {
	int len;
	char *p, *ls;
	int lastcolor;
	int chatHeight;

	cg.chatTime = cg.time;

	if (cg_hudType.integer == HUD_TYPE_SUPERHUD) {
		chatHeight = 8;  // max HUD_CHAT_X elements
	} else {
		if (cg_chatHeight.integer < TEAMCHAT_HEIGHT) {
			chatHeight = cg_chatHeight.integer;
		} else {
			chatHeight = TEAMCHAT_HEIGHT;
		}
	}

	if (chatHeight <= 0 || (cg_chatTime.integer * 1000) <= 0) {
		// team chat disabled, dump into normal chat
		cgs.chatPos = cgs.chatLastPos = 0;
		return;
	}

	len = 0;

	p = cgs.chatMsgs[cgs.chatPos % chatHeight];
	*p = 0;

	lastcolor = '7';

	ls = NULL;
	while (*str) {
		if (len > TEAMCHAT_WIDTH - 1) {
			if (ls) {
				str -= (p - ls);
				str++;
				p -= (p - ls);
			}
			*p = 0;

			cgs.chatMsgTimes[cgs.chatPos % chatHeight] = cg.time;

			cgs.chatPos++;
			p = cgs.chatMsgs[cgs.chatPos % chatHeight];
			*p = 0;
			*p++ = Q_COLOR_ESCAPE;
			*p++ = lastcolor;
			len = 0;
			ls = NULL;
		}

		if ( Q_IsColorString( str ) ) {
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}
		if (*str == ' ') {
			ls = p;
		}
		*p++ = *str++;
		len++;
	}
	*p = 0;

	cgs.chatMsgTimes[cgs.chatPos % chatHeight] = cg.time;
	cgs.chatPos++;

	if (cgs.chatPos - cgs.chatLastPos > chatHeight)
		cgs.chatLastPos = cgs.chatPos - chatHeight;
}



/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.

A tournement restart will clear everything, but doesn't
require a reload of all the media
===============
*/
static void CG_MapRestart( void ) {
	if ( cg_showmiss.integer ) {
		CG_Printf( "CG_MapRestart\n" );
	}

	CG_InitLocalEntities();
	CG_InitMarkPolys();

	// make sure the "3 frags left" warnings play again
	cg.fraglimitWarnings = 0;

	cg.timelimitWarnings = 0;

	cg.intermissionStarted = qfalse;

	cgs.voteTime = 0;

	CG_StartMusic();

	// we really should clear more parts of cg here and stop sounds

	// reset the sound buffer(dont want sounds carried over from previous match)
	cg.soundBuffer[cg.soundBufferOut] = 0;

	// play the "fight" sound if this is a restart without warmup
	if ( cg.warmup == 0 /* && cgs.gametype == GT_TOURNAMENT */) {
		trap_S_StartLocalSound( cgs.media.countFightSound, CHAN_ANNOUNCER );
		CG_CenterPrint( "FIGHT!" );
	}

	cg.takenScreenshot = qfalse;  // auto screenshot
	cg.screenshotTime = 0;  // auto screenshot

	// Tourney intermission sounds
	cg.musicStart = qfalse;
	cg.musicStartTime = 0;

	// auto demo
	if ( cg_autoRecord.integer && !cg.playerRecord ) {
		CG_NamedRecord(NULL);
		cg.playerRecord = qtrue;
	}

	// Reset intermission stats window
	if( cg.statsWindow != NULL ) {
		//cg.statsWindow->time = trap_Milliseconds();
		cg.statsWindow->effects = WFX_SCROLLLEFT;
		CG_windowFree( cg.statsWindow );
		cg.statsWindow = NULL;
	}
	if( cg.intermWindow != NULL ) {
		//cg.intermWindow->time = trap_Milliseconds();
		cg.intermWindow->effects = WFX_SCROLLRIGHT;
		CG_windowFree( cg.intermWindow );
		cg.intermWindow = NULL;
	}
	// reset client stats
	memset( &stats, 0, sizeof(stats) );
}


/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand( void ) {
	const char	*cmd;

	cmd = CG_Argv(0);

	if ( !cmd[0] ) {
		// server claimed the command
		return;
	}

	if ( !strcmp( cmd, "cp" ) ) {
		CG_CenterPrint( CG_Argv(1) );
		return;
	}

	if ( !strcmp( cmd, "cs" ) ) {
		CG_ConfigStringModified();
		return;
	}

	if ( !strcmp( cmd, "print" ) ) {
		if (!(cg_consoleFilter.integer & CHAT_NO_SERVER_CHAT)) {
			CG_Printf("%s", CG_Argv(1));
		}

		if (!(cg_chatFilter.integer & CHAT_NO_SERVER_CHAT)) {
			CG_AddToChat(CG_Argv(1));
		}

		return;
	}

	if ( !strcmp( cmd, "chat" ) ) {
		if (!(cg_consoleFilter.integer & CHAT_NO_PUBLIC_CHAT)) {
			if (!cg_noChatBeep.integer) {
				trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
			}
			CG_Printf("%s\n", CG_Argv(1));
		}

		if (!(cg_chatFilter.integer & CHAT_NO_PUBLIC_CHAT)) {
			CG_AddToChat(CG_Argv(1));
		}

		return;
	}

	if ( !strcmp( cmd, "tchat" ) ) {
		if (!(cg_consoleFilter.integer & CHAT_NO_TEAM_CHAT)) {
			if (!cg_noChatBeep.integer) {
				trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
			}
			CG_Printf("%s\n", CG_Argv(1));
		}

		if (!(cg_chatFilter.integer & CHAT_NO_TEAM_CHAT)) {
			CG_AddToChat(CG_Argv(1));
		}

		return;
	}

	if ( !strcmp( cmd, "scores" ) ) {
		CG_ParseScores();
		return;
	}

	if ( !strcmp( cmd, "tinfo" ) ) {
		CG_ParseTeamInfo();
		return;
	}

	if ( !strcmp( cmd, "map_restart" ) ) {
		CG_MapRestart();
		return;
	}

	// loaddeferred can be both a servercmd and a consolecmd
	if ( !strcmp( cmd, "loaddefered" ) ) {	// FIXME: spelled wrong, but not changing for demo
		CG_LoadDeferredPlayers();
		return;
	}

	// clientLevelShot is sent before taking a special screenshot for
	// the menu system during development
	if ( !strcmp( cmd, "clientLevelShot" ) ) {
		cg.levelShot = qtrue;
		return;
	}

	CG_Printf( "Unknown client game command: %s\n", cmd );
}


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
	while ( cgs.serverCommandSequence < latestSequence ) {
		if ( trap_GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
