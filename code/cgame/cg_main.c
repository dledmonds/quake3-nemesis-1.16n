// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"
#include "..\game\bg_promode.h"

// cvar modification counts
int forceModelModificationCount;
int enemyModelModificationCount;
int crosshairColorModificationCount;
int crosshairAlphaModificationCount;
int enemyColorModificationCount;
int teamColorModificationCount;
int uinfoModificationCount;
int nameModificationCount;
int hudTypeModificationCount;
int hudFileModificationCount;

void CG_Init( int serverMessageNum, int serverCommandSequence );
void CG_Shutdown( void );

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6 ) {
	switch ( command ) {
	case CG_INIT:
		CG_Init( arg0, arg1 );
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
		return 0;
	case CG_CONSOLE_COMMAND:
		return CG_ConsoleCommand();
	case CG_DRAW_ACTIVE_FRAME:
		CG_DrawActiveFrame( arg0, arg1, arg2 );
		return 0;
	case CG_CROSSHAIR_PLAYER:
		return CG_CrosshairPlayer();
	case CG_LAST_ATTACKER:
		return CG_LastAttacker();
	default:
		CG_Error( "vmMain: unknown command %i", command );
		break;
	}
	return -1;
}


cg_t				cg;
cgs_t				cgs;
clientStats_t		stats;
centity_t			cg_entities[MAX_GENTITIES];
weaponInfo_t		cg_weapons[MAX_WEAPONS];
itemInfo_t			cg_items[MAX_ITEMS];

vmCvar_t	cg_railTrailTime;
vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_gibs;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
vmCvar_t	cg_drawAmmoWarning;
vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_drawRewards;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_crosshairHealth;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_drawStatus;
vmCvar_t	cg_animSpeed;
vmCvar_t	cg_debugAnim;
vmCvar_t	cg_debugPosition;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
vmCvar_t	cg_noPlayerAnims;
vmCvar_t	cg_showmiss;
vmCvar_t	cg_footsteps;
vmCvar_t	cg_addMarks;
vmCvar_t	cg_brassTime;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
vmCvar_t	cg_gun_frame;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_tracerChance;
vmCvar_t	cg_tracerWidth;
vmCvar_t	cg_tracerLength;
vmCvar_t	cg_autoswitch;
vmCvar_t	cg_ignore;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_zoomFov;
vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_stereoSeparation;
vmCvar_t	cg_lagometer;
vmCvar_t	cg_drawAttacker;
vmCvar_t	cg_syncronousClients;
vmCvar_t 	cg_stats;
vmCvar_t 	cg_buildScript;
vmCvar_t 	cg_forceModel;
vmCvar_t	cg_paused;
vmCvar_t	cg_blood;
vmCvar_t	cg_predictItems;
vmCvar_t	cg_deferPlayers;
vmCvar_t	cg_drawTeamOverlay;
vmCvar_t	cg_teamOverlayUserinfo;

// Nemesis
vmCvar_t	cg_cgame;
vmCvar_t	cg_uinfo;
vmCvar_t	player_name;  // detect name changes for auth hash regeneration
vmCvar_t	int_cl_maxpackets;
vmCvar_t	int_cl_timenudge;
vmCvar_t	cg_railTrail;
vmCvar_t	cg_trueLightning;
vmCvar_t	cg_enemyColor;
vmCvar_t	cg_teamColor;
vmCvar_t	cg_enemyModel;
vmCvar_t	cg_specEnemyModel;
vmCvar_t	cg_drawAcc;
vmCvar_t	cg_crosshairAlpha;
vmCvar_t	cg_crosshairColor;
vmCvar_t	cg_useTeamIcons;
vmCvar_t	cg_drawSpeed;
vmCvar_t	cg_drawSpeedBar;
vmCvar_t	cg_enableBreath;
vmCvar_t	cg_scoreboardType;
vmCvar_t	cg_hudType;
vmCvar_t	cg_hudFile;
vmCvar_t	cg_hudDebug;
vmCvar_t	cg_autoScreenshot;
vmCvar_t	cg_autoRecord;
vmCvar_t	cg_thawSoundType;
vmCvar_t	cg_iceShells;
vmCvar_t	cg_teamInfoType;
vmCvar_t	cg_noChatBeep;
vmCvar_t	cg_autoAnnounce;
vmCvar_t	cg_noFriendShader;
vmCvar_t	cg_noConnectionShader;
vmCvar_t	cg_noProjectileTrail;
vmCvar_t	cg_scorePlum;
vmCvar_t	cg_debugNemesis;
vmCvar_t	cg_teamColorWeapons;
vmCvar_t	cg_specialEffects;
vmCvar_t	cg_ammoCheck;
vmCvar_t 	cg_chatTime;
vmCvar_t 	cg_chatHeight;
vmCvar_t	cg_chatFilter;
vmCvar_t	cg_consoleFilter;


typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

static	cvarTable_t		cvarTable[] = {
	{ &cg_ignore, "cg_ignore", "0", 0 },	// used for debugging
	{ &cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE },
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
	{ &cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "90", CVAR_ARCHIVE },
	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE  },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
	{ &cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE  },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
	{ &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
	{ &cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE  },
	{ &cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE  },
	{ &cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
	{ &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &cg_crosshairHealth, "cg_crosshairHealth", "0", CVAR_ARCHIVE },	// 11-08-05 - Disabled by default for xhair color.
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
	{ &cg_brassTime, "cg_brassTime", "1250", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },
	{ &cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE },
	{ &cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE  },
	{ &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },
	{ &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },
	{ &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
	{ &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
	{ &cg_bobup , "cg_bobup", "0.005", CVAR_ARCHIVE },
	{ &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
	{ &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },
	{ &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
	{ &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
	{ &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
	{ &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
	{ &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },
	{ &cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT },
	{ &cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT },
	{ &cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT },
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "40", 0 },
	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
	{ &cg_thirdPerson, "cg_thirdPerson", "0", 0 },
	{ &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE },
	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
	{ &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
	{ &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
	{ &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO },
	{ &cg_stats, "cg_stats", "0", 0 },

	// Nemesis
	{ &cg_cgame, "cgame", "", CVAR_ROM | CVAR_USERINFO },
	{ &cg_uinfo, "cg_uinfo", "0", CVAR_ROM | CVAR_USERINFO },
	{ &player_name, "name", "", CVAR_ARCHIVE },
	{ &int_cl_maxpackets, "cl_maxpackets", "30", CVAR_ARCHIVE },
	{ &int_cl_timenudge, "cl_timenudge", "0", CVAR_ARCHIVE },
	{ &cg_railTrail, "cg_railTrail", "1", CVAR_ARCHIVE },
	{ &cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE},
	{ &cg_enemyColor, "cg_enemyColor", "", CVAR_ARCHIVE },
	{ &cg_teamColor, "cg_teamColor", "", CVAR_ARCHIVE },
	{ &cg_enemyModel, "cg_enemyModel", "2", CVAR_ARCHIVE },
	{ &cg_specEnemyModel, "cg_specEnemyModel", "1", CVAR_ARCHIVE },
	{ &cg_drawAcc, "cg_drawAcc", "0", CVAR_ARCHIVE },
	{ &cg_crosshairAlpha, "cg_crosshairAlpha", "1.0", CVAR_ARCHIVE },
	{ &cg_crosshairColor, "cg_crosshairColor", "White", CVAR_ARCHIVE },
	{ &cg_useTeamIcons, "cg_useTeamIcons", "0", CVAR_ARCHIVE },
	{ &cg_drawSpeed, "cg_drawSpeed", "0", CVAR_ARCHIVE },
	{ &cg_drawSpeedBar, "cg_drawSpeedBar", "0", CVAR_ARCHIVE },
	{ &cg_enableBreath, "cg_enableBreath", "1", CVAR_ARCHIVE },
	{ &cg_scoreboardType, "cg_scoreboardType", "1", CVAR_ARCHIVE },
	{ &cg_hudType, "cg_hudType", "0", CVAR_ARCHIVE },
	{ &cg_hudFile, "cg_hudFile", "hud1.cfg", CVAR_ARCHIVE },
	{ &cg_hudDebug, "cg_hudDebug", "0", CVAR_ARCHIVE },
	{ &cg_autoScreenshot, "cg_autoScreenshot", "0", CVAR_ARCHIVE },
	{ &cg_autoRecord, "cg_autoRecord", "0", CVAR_ARCHIVE },
	{ &cg_thawSoundType, "cg_thawSoundType", "0", CVAR_ARCHIVE },
	{ &cg_iceShells, "cg_iceShells", "1", CVAR_ARCHIVE },
	{ &cg_teamInfoType, "cg_teamInfoType", "2", CVAR_ARCHIVE },
	{ &cg_noChatBeep, "cg_noChatBeep", "0", CVAR_ARCHIVE },
	{ &cg_autoAnnounce, "cg_autoAnnounce", "0", CVAR_ARCHIVE },
	{ &cg_noFriendShader, "cg_noFriendShader", "0", CVAR_ARCHIVE },
	{ &cg_noConnectionShader, "cg_noConnectionShader", "0", CVAR_ARCHIVE },
	{ &cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE },
	{ &cg_scorePlum, "cg_scorePlum", "1", CVAR_ARCHIVE },
	{ &cg_debugNemesis, "cg_debugNemesis", "0", CVAR_CHEAT },
	{ &cg_teamColorWeapons, "cg_teamColorWeapons", "0", CVAR_ARCHIVE },
	{ &cg_specialEffects, "cg_specialEffects", "3", CVAR_ARCHIVE },
	{ &cg_ammoCheck, "cg_ammoCheck", "0", CVAR_ARCHIVE },
	{ &cg_chatTime, "cg_chatTime", "4", CVAR_ARCHIVE  },
	{ &cg_chatHeight, "cg_chatHeight", "3", CVAR_ARCHIVE },
	{ &cg_chatFilter, "cg_chatFilter", "5", CVAR_ARCHIVE },
	{ &cg_consoleFilter, "cg_consoleFilter", "0", CVAR_ARCHIVE },

	// the following variables are created in other parts of the system,
	// but we also reference them here

	{ &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures
	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
	{ &cg_blood, "com_blood", "1", CVAR_ARCHIVE },
	{ &cg_syncronousClients, "g_syncronousClients", "0", 0 },	// communicated by systeminfo
};

static	int		cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );

// OSP function - For setting clients timenudge and packets in /players command
static void CG_setClientFlags(void) {

	if(cg.demoPlayback)
		return;

	trap_Cvar_Set("cg_uinfo", va("%d %d %d", int_cl_timenudge.integer, int_cl_maxpackets.integer));
}

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	char		var[MAX_TOKEN_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}

	// see if we are also running the server on this machine
	trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );

	if( CG_isNemesis() ) {
		CG_setClientFlags();
	}
	
	forceModelModificationCount = cg_forceModel.modificationCount;
	enemyModelModificationCount = cg_enemyModel.modificationCount;
	crosshairColorModificationCount = cg_crosshairColor.modificationCount;
	crosshairAlphaModificationCount = cg_crosshairAlpha.modificationCount;
	enemyColorModificationCount = cg_enemyColor.modificationCount;
	teamColorModificationCount = cg_teamColor.modificationCount;
	uinfoModificationCount = int_cl_maxpackets.modificationCount + int_cl_timenudge.modificationCount;
	nameModificationCount = player_name.modificationCount;
	hudTypeModificationCount = cg_hudType.modificationCount;
	hudFileModificationCount = cg_hudFile.modificationCount;
}

/*
======================
CG_UpdateNewClientInfo
======================
*/
void CG_UpdateNewClientInfo( void ) {
	int		i;

	for (i = 0; i < MAX_CLIENTS; i++) {
		const char		*clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS + i );
		if ( !clientInfo[0] ) {
			continue;
		}
		CG_NewClientInfo( i );
	}
}

/*
=================
CG_SetClientHash

Send the client auth hash to the server
=================
*/
static void CG_SetClientHash() {
	if (CG_isNemesis()) { 
		char authInput[256];
		char calculatedHash[33];
		int i;
		char clientName[MAX_QPATH];

		// clean out input
		for (i = 0; i < 256; i++) {
			authInput[i] = '\0';
		}

		trap_Cvar_VariableStringBuffer("name", clientName, sizeof(clientName));

		/* this code has an equivalent in g_client.c ClientUserInfoChanged(..)
		 * secret word is a word that changes per release
		 */
		Q_strncpyz(authInput, "github-release", 256);  // secret word
		Q_strcat(authInput, 256, clientName);
		SanitizeString(authInput, authInput, qfalse);
		/*CG_Printf("Cinput '%s' (%i)\n", authInput, strlen(authInput));
		for(i=0; i<(int)strlen(authInput)+1; i++) {
			CG_Printf(" %i", authInput[i]);
		}
		CG_Printf("\n");*/
		GenerateHash(authInput, calculatedHash);
		//CG_Printf("Chash '%s'\n", calculatedHash);

		trap_SendClientCommand(va("setauth 1 %s", calculatedHash));
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		// try not to put extra actions in here ...
		trap_Cvar_Update( cv->vmCvar );
	}

	// check for modications here

	// If team overlay is on, ask for updates from the server.  If its off,
	// let the server know so we don't receive it
	if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount ) {
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

		if ( cg_drawTeamOverlay.integer > 0 ) {
			trap_Cvar_Set( "teamoverlay", "1" );
		} else {
			trap_Cvar_Set( "teamoverlay", "0" );
		}
		// FIXME E3 HACK
		trap_Cvar_Set( "teamoverlay", "1" );
	}

	// update cg_uinfo to send any relevent updates to the server
	if (CG_isNemesis()) {
		if (uinfoModificationCount != (int_cl_maxpackets.modificationCount + int_cl_timenudge.modificationCount)) {
			uinfoModificationCount = int_cl_maxpackets.modificationCount + int_cl_timenudge.modificationCount;

			CG_setClientFlags();
		}
	}

	if ( forceModelModificationCount != cg_forceModel.modificationCount ) {
		forceModelModificationCount = cg_forceModel.modificationCount;

		if (cg_debugNemesis.integer & NMS_DEBUG_MODEL) {
			CG_Printf("%supdate client info request from ^3%s (%s)\n", NMS_DEBUG_PREFIX, "CG_UpdateCvars", "cg_forceModel");
		}

		CG_UpdateNewClientInfo();
	}

	if ( enemyModelModificationCount != cg_enemyModel.modificationCount ) {
		enemyModelModificationCount = cg_enemyModel.modificationCount;

		if (cg_debugNemesis.integer & NMS_DEBUG_MODEL) {
			CG_Printf("%supdate client info request from ^3%s (%s)\n", NMS_DEBUG_PREFIX, "CG_UpdateCvars", "cg_enemyModel");
		}

		CG_UpdateNewClientInfo();
	}

	if ( crosshairColorModificationCount != cg_crosshairColor.modificationCount || 
		 crosshairAlphaModificationCount != cg_crosshairAlpha.modificationCount ) {
		crosshairColorModificationCount = cg_crosshairColor.modificationCount;
		crosshairAlphaModificationCount = cg_crosshairAlpha.modificationCount;

		CG_SetColor( cg_crosshairColor.string, cg.xhairColor, cg_crosshairAlpha.value );
	}

	if ( enemyColorModificationCount != cg_enemyColor.modificationCount ) {
		enemyColorModificationCount = cg_enemyColor.modificationCount;

		if (cg_debugNemesis.integer & NMS_DEBUG_MODEL) {
			CG_Printf("%supdate client info request from ^3%s (%s)\n", NMS_DEBUG_PREFIX, "CG_UpdateCvars", "cg_enemyColor");
		}

		CG_SetEnemyColors( cg_enemyColor.string );
		CG_UpdateNewClientInfo();  // redo clients as models are cached when switching from spec.
	}

	if ( teamColorModificationCount != cg_teamColor.modificationCount ) {
		teamColorModificationCount = cg_teamColor.modificationCount;

		if (cg_debugNemesis.integer & NMS_DEBUG_MODEL) {
			CG_Printf("%supdate client info request from ^3%s (%s)\n", NMS_DEBUG_PREFIX, "CG_UpdateCvars", "cg_teamColor");
		}

		CG_SetTeamColors( cg_teamColor.string );
		CG_UpdateNewClientInfo();  // redo clients as models are cached when switching from spec.
	}

	if ( nameModificationCount != player_name.modificationCount) {
		nameModificationCount = player_name.modificationCount;

		CG_SetClientHash();
	}

	if (hudTypeModificationCount != cg_hudType.modificationCount) {
		hudTypeModificationCount = cg_hudType.modificationCount;

		if (cg_hudType.integer < HUD_TYPE_DEFAULT || cg_hudType.integer > HUD_TYPE_SUPERHUD) {
			trap_Cvar_Set("cg_hudType", "0");
		}

		if (cg_hudType.integer == HUD_TYPE_SUPERHUD) {
			CG_Hud_Parse();
		}
	}

	if (hudFileModificationCount != cg_hudFile.modificationCount) {
		hudFileModificationCount = cg_hudFile.modificationCount;

		// no point parsing if we're not using super HUD
		if (cg_hudType.integer == HUD_TYPE_SUPERHUD) {
			CG_Hud_Parse();
		}
	}

}


int CG_CrosshairPlayer( void ) {
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) {
		return -1;
	}
	return cg.crosshairClientNum;
}


int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}


void QDECL CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Error( text );
}

#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	CG_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	CG_Printf ("%s", text);
}

#endif



/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds( int itemNum ) {
	gitem_t			*item;
	char			data[MAX_QPATH];
	char			*s, *start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	trap_S_RegisterSound( item->pickup_sound );

	// parse the space seperated precache string for other media
	s = item->sounds;
	if (!s || !s[0])
		return;

	while (*s) {
		start = s;
		while (*s && *s != ' ') {
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) {
			CG_Error( "PrecacheItem: %s has bad precache string", 
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp(data+len-3, "wav" )) {
			trap_S_RegisterSound( data );
		}
	}
}


/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
	int		i;
	char	items[MAX_ITEMS+1];
	char	name[MAX_QPATH];
	const char	*soundName;

	if ( cgs.timelimit || cg_buildScript.integer ) {	// should we always load this?
		cgs.media.oneMinuteSound = trap_S_RegisterSound( "sound/feedback/1_minute.wav" );
		cgs.media.fiveMinuteSound = trap_S_RegisterSound( "sound/feedback/5_minute.wav" );
		cgs.media.suddenDeathSound = trap_S_RegisterSound( "sound/feedback/sudden_death.wav" );
	}

	if ( cgs.fraglimit || cg_buildScript.integer ) {
		cgs.media.oneFragSound = trap_S_RegisterSound( "sound/feedback/1_frag.wav" );
		cgs.media.twoFragSound = trap_S_RegisterSound( "sound/feedback/2_frags.wav" );
		cgs.media.threeFragSound = trap_S_RegisterSound( "sound/feedback/3_frags.wav" );
	}

//	if ( cgs.gametype == GT_TOURNAMENT || cg_buildScript.integer ) {
//  We always need this since a warmup can be enabled in any game mode
		cgs.media.count3Sound = trap_S_RegisterSound( "sound/feedback/three.wav" );
		cgs.media.count2Sound = trap_S_RegisterSound( "sound/feedback/two.wav" );
		cgs.media.count1Sound = trap_S_RegisterSound( "sound/feedback/one.wav" );
		cgs.media.countFightSound = trap_S_RegisterSound( "sound/feedback/fight.wav" );
		cgs.media.countPrepareSound = trap_S_RegisterSound( "sound/feedback/prepare.wav" );
//	}

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {
		cgs.media.redLeadsSound = trap_S_RegisterSound( "sound/feedback/redleads.wav" );
		cgs.media.blueLeadsSound = trap_S_RegisterSound( "sound/feedback/blueleads.wav" );
		cgs.media.teamsTiedSound = trap_S_RegisterSound( "sound/feedback/teamstied.wav" );
		cgs.media.hitTeamSound = trap_S_RegisterSound( "sound/feedback/hit_teammate.wav" );

		// 1.32 CTF Sounds
		cgs.media.redScoredSound = trap_S_RegisterSound( "sound/teamplay/voc_red_scores.wav" );
		cgs.media.blueScoredSound = trap_S_RegisterSound( "sound/teamplay/voc_blue_scores.wav" );
		cgs.media.captureYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav" );
		cgs.media.captureOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_opponent.wav" );
		cgs.media.returnYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_yourteam.wav" );
		cgs.media.returnOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav" );
		cgs.media.redFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/voc_red_returned.wav" );
		cgs.media.blueFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/voc_blue_returned.wav" );
		cgs.media.enemyTookYourFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_enemy_flag.wav" );
		cgs.media.yourTeamTookEnemyFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_team_flag.wav" );
		cgs.media.youHaveFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_you_flag.wav" );
	}

	cgs.media.tracerSound = trap_S_RegisterSound( "sound/weapons/machinegun/buletby1.wav" );
	cgs.media.selectSound = trap_S_RegisterSound( "sound/weapons/change.wav" );
	cgs.media.wearOffSound = trap_S_RegisterSound( "sound/items/wearoff.wav" );
	cgs.media.useNothingSound = trap_S_RegisterSound( "sound/items/use_nothing.wav" );
	cgs.media.gibSound = trap_S_RegisterSound( "sound/player/gibsplt1.wav" );
	cgs.media.gibBounce1Sound = trap_S_RegisterSound( "sound/player/gibimp1.wav" );
	cgs.media.gibBounce2Sound = trap_S_RegisterSound( "sound/player/gibimp2.wav" );
	cgs.media.gibBounce3Sound = trap_S_RegisterSound( "sound/player/gibimp3.wav" );

	cgs.media.iceShatter = trap_S_RegisterSound( "sound/icebreak.wav" );

	cgs.media.teleInSound = trap_S_RegisterSound( "sound/world/telein.wav" );
	cgs.media.teleOutSound = trap_S_RegisterSound( "sound/world/teleout.wav" );
	cgs.media.respawnSound = trap_S_RegisterSound( "sound/items/respawn1.wav" );

	cgs.media.noAmmoSound = trap_S_RegisterSound( "sound/weapons/noammo.wav" );

	cgs.media.talkSound = trap_S_RegisterSound( "sound/player/talk.wav" );
	cgs.media.landSound = trap_S_RegisterSound( "sound/player/land1.wav");

	// CPM: Hit tones
	cgs.media.hitSound[0] = trap_S_RegisterSound( "sound/feedback/hitlowest.wav" );
	cgs.media.hitSound[1] = trap_S_RegisterSound( "sound/feedback/hitlow.wav" );
	cgs.media.hitSound[2] = trap_S_RegisterSound( "sound/feedback/hit.wav" );
	cgs.media.hitSound[3] = trap_S_RegisterSound( "sound/feedback/hithigh.wav" );
	// end

	cgs.media.impressiveSound = trap_S_RegisterSound( "sound/feedback/impressive.wav" );
	cgs.media.excellentSound = trap_S_RegisterSound( "sound/feedback/excellent.wav" );
	cgs.media.deniedSound = trap_S_RegisterSound( "sound/feedback/denied.wav" );
	cgs.media.humiliationSound = trap_S_RegisterSound( "sound/feedback/humiliation.wav" );

	cgs.media.takenLeadSound = trap_S_RegisterSound( "sound/feedback/takenlead.wav");
	cgs.media.tiedLeadSound = trap_S_RegisterSound( "sound/feedback/tiedlead.wav");
	cgs.media.lostLeadSound = trap_S_RegisterSound( "sound/feedback/lostlead.wav");

	cgs.media.watrInSound = trap_S_RegisterSound( "sound/player/watr_in.wav");
	cgs.media.watrOutSound = trap_S_RegisterSound( "sound/player/watr_out.wav");
	cgs.media.watrUnSound = trap_S_RegisterSound( "sound/player/watr_un.wav");

	cgs.media.jumpPadSound = trap_S_RegisterSound ("sound/world/jumppad.wav" );
	cgs.media.win = trap_S_RegisterSound( "sound/misc/win.wav" );
	cgs.media.lose = trap_S_RegisterSound( "sound/misc/lose.wav" );

	for (i=0 ; i<4 ; i++) {
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_NORMAL][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/boot%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_BOOT][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/flesh%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_FLESH][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/mech%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_MECH][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/energy%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_ENERGY][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/splash%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SPLASH][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/clank%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_METAL][i] = trap_S_RegisterSound (name);
	}

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS ) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_RegisterItemSounds( i );
		}
	}

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
		soundName = CG_ConfigString( CS_SOUNDS+i );
		if ( !soundName[0] ) {
			break;
		}
		if ( soundName[0] == '*' ) {
			continue;	// custom sound
		}
		cgs.gameSounds[i] = trap_S_RegisterSound( soundName );
	}

	// FIXME: only needed with item
	cgs.media.flightSound = trap_S_RegisterSound( "sound/items/flight.wav" );
	cgs.media.medkitSound = trap_S_RegisterSound ("sound/items/use_medkit.wav");
	cgs.media.quadSound = trap_S_RegisterSound("sound/items/damage3.wav");
	cgs.media.sfx_ric1 = trap_S_RegisterSound ("sound/weapons/machinegun/ric1.wav");
	cgs.media.sfx_ric2 = trap_S_RegisterSound ("sound/weapons/machinegun/ric2.wav");
	cgs.media.sfx_ric3 = trap_S_RegisterSound ("sound/weapons/machinegun/ric3.wav");
	cgs.media.sfx_railg = trap_S_RegisterSound ("sound/weapons/railgun/railgf1a.wav");
	cgs.media.sfx_rockexp = trap_S_RegisterSound ("sound/weapons/rocket/rocklx1a.wav");
	cgs.media.sfx_plasmaexp = trap_S_RegisterSound ("sound/weapons/plasma/plasmx1a.wav");
}


//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
	int			i;
	char		items[MAX_ITEMS+1];
	static char		*sb_nums[11] = {
		"gfx/2d/numbers/zero_32b",
		"gfx/2d/numbers/one_32b",
		"gfx/2d/numbers/two_32b",
		"gfx/2d/numbers/three_32b",
		"gfx/2d/numbers/four_32b",
		"gfx/2d/numbers/five_32b",
		"gfx/2d/numbers/six_32b",
		"gfx/2d/numbers/seven_32b",
		"gfx/2d/numbers/eight_32b",
		"gfx/2d/numbers/nine_32b",
		"gfx/2d/numbers/minus_32b",
	};

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	trap_R_ClearScene();

	CG_LoadingString( cgs.mapname );

	trap_R_LoadWorldMap( cgs.mapname );

	// precache status bar pics
	CG_LoadingString( "game media" );

	for ( i=0 ; i<11 ; i++) {
		cgs.media.numberShaders[i] = trap_R_RegisterShader( sb_nums[i] );
	}

	cgs.media.botSkillShaders[0] = trap_R_RegisterShader( "menu/art/skill1.tga" );
	cgs.media.botSkillShaders[1] = trap_R_RegisterShader( "menu/art/skill2.tga" );
	cgs.media.botSkillShaders[2] = trap_R_RegisterShader( "menu/art/skill3.tga" );
	cgs.media.botSkillShaders[3] = trap_R_RegisterShader( "menu/art/skill4.tga" );
	cgs.media.botSkillShaders[4] = trap_R_RegisterShader( "menu/art/skill5.tga" );

	cgs.media.viewBloodShader = trap_R_RegisterShader( "viewBloodBlend" );

	cgs.media.deferShader = trap_R_RegisterShaderNoMip( "gfx/2d/defer.tga" );

	cgs.media.scoreboardName = trap_R_RegisterShaderNoMip( "menu/tab/name.tga" );
	cgs.media.scoreboardPing = trap_R_RegisterShaderNoMip( "menu/tab/ping.tga" );
	cgs.media.scoreboardScore = trap_R_RegisterShaderNoMip( "menu/tab/score.tga" );
	cgs.media.scoreboardTime = trap_R_RegisterShaderNoMip( "menu/tab/time.tga" );

	cgs.media.smokePuffShader = trap_R_RegisterShader( "smokePuff" );
	cgs.media.smokePuffRageProShader = trap_R_RegisterShader( "smokePuffRagePro" );
	cgs.media.shotgunSmokePuffShader = trap_R_RegisterShader( "shotgunSmokePuff" );
	cgs.media.plasmaBallShader = trap_R_RegisterShader( "sprites/plasma1" );
	cgs.media.bloodTrailShader = trap_R_RegisterShader( "bloodTrail" );
	cgs.media.lagometerShader = trap_R_RegisterShader("lagometer" );
	cgs.media.connectionShader = trap_R_RegisterShader( "disconnected" );

	cgs.media.waterBubbleShader = trap_R_RegisterShader( "waterBubble" );

	cgs.media.tracerShader = trap_R_RegisterShader( "gfx/misc/tracer" );
	cgs.media.selectShader = trap_R_RegisterShader( "gfx/2d/select" );

	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
		cgs.media.crosshairShader[i] = trap_R_RegisterShader( va("gfx/2d/crosshair%c2", 'a'+i) );
	}

	cgs.media.backTileShader = trap_R_RegisterShader( "gfx/2d/backtile" );
	cgs.media.noammoShader = trap_R_RegisterShader( "icons/noammo" );

	// powerup shaders
	cgs.media.quadShader = trap_R_RegisterShader("powerups/quad" );
	cgs.media.quadWeaponShader = trap_R_RegisterShader("powerups/quadWeapon" );
	cgs.media.battleSuitShader = trap_R_RegisterShader("powerups/battleSuit" );
	cgs.media.battleWeaponShader = trap_R_RegisterShader("powerups/battleWeapon" );
	cgs.media.invisShader = trap_R_RegisterShader("powerups/invisibility" );
	cgs.media.regenShader = trap_R_RegisterShader("powerups/regen" );
	cgs.media.hastePuffShader = trap_R_RegisterShader("hasteSmokePuff" );

	if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
		cgs.media.redFlagModel = trap_R_RegisterModel( "models/flags/r_flag.md3" );
		cgs.media.blueFlagModel = trap_R_RegisterModel( "models/flags/b_flag.md3" );
		cgs.media.redFlagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_red1" );
		cgs.media.redFlagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_red2" );
		cgs.media.redFlagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_red3" );
		cgs.media.blueFlagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_blu1" );
		cgs.media.blueFlagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_blu2" );
		cgs.media.blueFlagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_blu3" );
	}

	cgs.media.friendShader = trap_R_RegisterShader( "sprites/foe" );
	cgs.media.teamStatusBar = trap_R_RegisterShader( "gfx/2d/colorbar.tga" );
	cgs.media.armorModel = trap_R_RegisterModel( "models/powerups/armor/armor_yel.md3" );
	cgs.media.armorIcon  = trap_R_RegisterShaderNoMip( "icons/iconr_yellow" );

	// Nemesis - New media
	cgs.media.freezeShader = trap_R_RegisterShader( "freezeShader" );
	cgs.media.freezeMarkShader = trap_R_RegisterShader( "freezeMarkShader" );
	cgs.media.plasmaBallShader2 = trap_R_RegisterShader( "sprites/plasma_color" );
	cgs.media.teamSparks = trap_R_RegisterShader( "gfx/misc/spark_team" );
	cgs.media.quadColor = trap_R_RegisterShader("powerups/quadColor" );
	cgs.media.healthModel = trap_R_RegisterModel( "models/powerups/health/medium_cross.md3" );
	cgs.media.healthSphere = trap_R_RegisterModel( "models/powerups/health/medium_sphere.md3" );
	cgs.media.medalAccuracy = trap_R_RegisterShaderNoMip( "menu/medals/medal_accuracy.tga" );
	cgs.media.redQuadShader = trap_R_RegisterShader("powerups/quadred" );
	cgs.media.teamIconRed = trap_R_RegisterShader( "team_red_icon" );
	cgs.media.teamIconBlue = trap_R_RegisterShader( "team_blue_icon" );
	cgs.media.scoreBarRed = trap_R_RegisterShader( "scoreboard_bar_red" );
	cgs.media.scoreBarBlue = trap_R_RegisterShader( "scoreboard_bar_blue" );
	// End
	
	cgs.media.machinegunBrassModel = trap_R_RegisterModel( "models/weapons2/shells/m_shell.md3" );
	cgs.media.shotgunBrassModel = trap_R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

	cgs.media.gibAbdomen = trap_R_RegisterModel( "models/gibs/abdomen.md3" );
	cgs.media.gibArm = trap_R_RegisterModel( "models/gibs/arm.md3" );
	cgs.media.gibChest = trap_R_RegisterModel( "models/gibs/chest.md3" );
	cgs.media.gibFist = trap_R_RegisterModel( "models/gibs/fist.md3" );
	cgs.media.gibFoot = trap_R_RegisterModel( "models/gibs/foot.md3" );
	cgs.media.gibForearm = trap_R_RegisterModel( "models/gibs/forearm.md3" );
	cgs.media.gibIntestine = trap_R_RegisterModel( "models/gibs/intestine.md3" );
	cgs.media.gibLeg = trap_R_RegisterModel( "models/gibs/leg.md3" );
	cgs.media.gibSkull = trap_R_RegisterModel( "models/gibs/skull.md3" );
	cgs.media.gibBrain = trap_R_RegisterModel( "models/gibs/brain.md3" );
	
	cgs.media.balloonShader = trap_R_RegisterShader( "sprites/balloon3" );

	cgs.media.bloodExplosionShader = trap_R_RegisterShader( "bloodExplosion" );

	cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/bullet.md3");
	cgs.media.ringFlashModel = trap_R_RegisterModel("models/weaphits/ring02.md3");
	cgs.media.dishFlashModel = trap_R_RegisterModel("models/weaphits/boom01.md3");
	cgs.media.teleportEffectModel = trap_R_RegisterModel( "models/misc/telep.md3" );
	cgs.media.teleportEffectShader = trap_R_RegisterShader( "teleportEffect" );

	cgs.media.medalImpressive = trap_R_RegisterShaderNoMip( "medal_impressive" );
	cgs.media.medalExcellent = trap_R_RegisterShaderNoMip( "medal_excellent" );
	cgs.media.medalGauntlet = trap_R_RegisterShaderNoMip( "medal_gauntlet" );


	memset( cg_items, 0, sizeof( cg_items ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_LoadingItem( i );
			CG_RegisterItemVisuals( i );
		}
	}

	// wall marks
	cgs.media.bulletMarkShader = trap_R_RegisterShader( "gfx/damage/bullet_mrk" );
	cgs.media.burnMarkShader = trap_R_RegisterShader( "gfx/damage/burn_med_mrk" );
	cgs.media.holeMarkShader = trap_R_RegisterShader( "gfx/damage/hole_lg_mrk" );
	cgs.media.energyMarkShader = trap_R_RegisterShader( "gfx/damage/plasma_mrk" );
	cgs.media.shadowMarkShader = trap_R_RegisterShader( "markShadow" );
	cgs.media.wakeMarkShader = trap_R_RegisterShader( "wake" );
	cgs.media.bloodMarkShader = trap_R_RegisterShader( "bloodMark" );

	// register the inline models
	cgs.numInlineModels = trap_CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char	name[10];
		vec3_t			mins, maxs;
		int				j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = trap_R_RegisterModel( name );
		trap_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
	}

	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) {
		const char		*modelName;

		modelName = CG_ConfigString( CS_MODELS+i );
		if ( !modelName[0] ) {
			break;
		}
		cgs.gameModels[i] = trap_R_RegisterModel( modelName );
	}
}

/*
===================
CG_RegisterClients

===================
*/
static void CG_RegisterClients( void ) {
	int		i;

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0] ) {
			continue;
		}
		CG_LoadingClient( i );
		CG_NewClientInfo( i );
	}
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void ) {
	char	*s;
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = (char *)CG_ConfigString( CS_MUSIC );
	Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

	trap_S_StartBackgroundTrack( parm1, parm2 );
}


/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence ) {
	const char	*s;

	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( &stats, 0, sizeof(stats) );
	memset( cg_entities, 0, sizeof(cg_entities) );
	memset( cg_weapons, 0, sizeof(cg_weapons) );
	memset( cg_items, 0, sizeof(cg_items) );

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader = trap_R_RegisterShader( "gfx/2d/bigchars" );
	cgs.media.threewaveCharsetShader = trap_R_RegisterShader( "gfx/2d/bigchars-threewave" );
	cgs.media.cpmaCharsetShader = trap_R_RegisterShader( "gfx/2d/bigchars-cpma" );
	cgs.media.whiteShader = trap_R_RegisterShader( "white" );
	cgs.media.charsetProp		= trap_R_RegisterShaderNoMip( "menu/art/font1_prop.tga" );
	cgs.media.charsetPropGlow	= trap_R_RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
	cgs.media.charsetPropB	= trap_R_RegisterShaderNoMip( "menu/art/font2_prop.tga" );

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	cg.weaponSelect = WP_MACHINEGUN;

	cgs.redflag = cgs.blueflag = -1; // For compatibily, default to unset for
	// old servers

	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

	// get the gamestate from the client system
	trap_GetGameState( &cgs.gameState );

	// Had to move this here, cgs.authorize didnt get set yet, yikes!
	// Also since CG_isNemesis value gets set now, so we can check for promode
	// on Nemesis servers only.
	CG_ParseServerinfo(); 

	// CPM: Setup according to the pro mode settings
	if( CG_isNemesis() ) {
		s = CG_ConfigString( CS_PRO_MODE );
		CPM_UpdateSettings( (atoi(s)) );
	}

	s = CG_ConfigString( CS_GAME_VERSION );

	if ( strcmp( s, GAME_VERSION ) ) {
		CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	// force initial setting of cg_uinfo CVAR
	CG_setClientFlags();

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	// load the new map
	CG_LoadingString( "collision map" );

	trap_CM_LoadMap( cgs.mapname );

	cg.loading = qtrue;		// force players to load instead of defer

	CG_LoadingString( "sounds" );

	CG_RegisterSounds();

	CG_RegisterGraphics();

	CG_RegisterClients();		// if low on memory, some clients will be deferred

	cg.loading = qfalse;	// future players will be deferred

	CG_InitLocalEntities();

	CG_InitMarkPolys();

	// remove the last loading update
	cg.infoScreenText[0] = 0;

	// Make sure we have update values (scores)
	CG_SetConfigValues();

	CG_StartMusic();

	CG_LoadingString( "" );

	// cgame version
	trap_Cvar_Set( "cgame", MOD_FULLNAME );

	CG_SetClientHash();

	// Precache current colors
	CG_SetColor( cg_crosshairColor.string, cg.xhairColor, cg_crosshairAlpha.value );
	CG_SetEnemyColors( cg_enemyColor.string );
	CG_SetTeamColors( cg_teamColor.string );

	// OSP Window setup
	CG_windowInit();

	// superhud
	CG_Hud_Parse();

	CG_Printf( "\n^5Loaded %s ^3v%s ^5released on ^3%s\n", MOD_NAME, MOD_VERSION, __DATE__ );
	CG_Printf( "^5(c) Cyrus & Wonkey - ^3%s\n\n", MOD_WEBSITE );
	CG_Printf( "^5Detected ^3%s ^5as the server mod\n\n", CG_detectedModName() );
	CG_Printf( "^5Type ^3\\chelp ^5for list of available %s commands\n\n", MOD_NAME );
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
}


