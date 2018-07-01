
/* cg_utils.c - © Wonkey & Cyrus - www.q3nemesis.com - 2005 */

#include "cg_local.h"


/*
==========================
CG_CleanForPath

Copies the string cleaned of invalid path characters (replaced with _)
==========================
*/
static void CG_CleanForPath(char *s1, char *s2, int size) {

}


/*
==========================
CG_GetCleanMapName

Populates s1 with the map name
==========================
*/
static void CG_GetCleanMapName(char* s1, int size) {
	char* s2;

	s2 = cgs.mapname;
	if (strlen(s2) > 4 && s2[4] == '/' ) {
		s2 += 5;
	}

	Q_strncpyz(s1, s2, size);
}


/*
======================
CG_GetCleanClientName

Populates s1 wit a cleaned (no color or wierd chars) name for the client,
suitable for using in a file path
======================
*/
static char* CG_GetCleanClientName(char* s1, int size) {
	clientInfo_t *ci;

	ci = &cgs.clientinfo[ cg.clientNum ];
	
	return ci->name;
}


/*
================
CG_NamedRecord

Encapsulates all calls for recording so that if no name is specified
a unique on can be generated.

Automatically handles the switching of g_syncronousclients
================
*/
void CG_NamedRecord(const char *name) {
	trap_SendConsoleCommand("g_syncronousclients 1\n");

	if (name == NULL || !*name) {
		name = "";
		//CG_Printf("Issuing ^3record BLANK\n");
		//CG_Printf("Could issue ^3record %s-%s\n", CG_GetCleanMapName(), CG_GetCleanClientName());
	} //else {
		//CG_Printf("Issuing ^3record %s\n", name);
	//}

	trap_SendConsoleCommand(va("record %s\n", name));
	trap_SendConsoleCommand("g_syncronousclients 0\n");
}


/*
=============
CG_isNemesis

Determines whether this is Nemesis Server
=============
*/
qboolean CG_isNemesis( void )
{
	return cgs.isNemesis;
}

/*
=============
CG_isOSP

Determines whether this is OSP
OSP has a variable to declare modname as "baseq3"
This could be a prob but not big enough to worry
about, ignoring for now.
=============
*/
qboolean CG_isOSP( void )
{
	if ( Q_stricmp( cgs.modName, "OSP" ) == 0 ) {
		return qtrue;
	}
	return qfalse;
}

/*
=============
CG_isNoGhost

Determines whether this is NoGhost
=============
*/
qboolean CG_isNoGhost( void )
{
	return cgs.isNoGhost;
}

/*
=============
CG_isFreezePlus

Determines whether this is Freezeplus
=============
*/
qboolean CG_isFreezePlus( void )
{
	if ( Q_stricmp( cgs.modName, "Freezeplus" ) == 0 ) {
		return qtrue;
	}
	return qfalse;
}

/*
=============
CG_isFreezeTag

Determines whether this is freezetag
=============
*/
qboolean CG_isFreezeTag( void )
{
	if ( ( cgs.gametype == GT_TEAM && CG_isNoGhost() && cgs.g_gameMod == 1 ) ||
		 ( cgs.gametype == GT_TEAM && CG_isFreezePlus() ) ||
		 ( cgs.gametype == GT_FREEZE && CG_isNemesis() ) ) {
		return qtrue;
	}

	return qfalse;
}

/*
=============
CG_detectedModName

Detected server mod name
=============
*/
char *CG_detectedModName( void ) {
	if (CG_isNemesis()) {
		return "Nemesis";
	} else if (CG_isNoGhost()) {
		return "NoGhost";
	} else if (CG_isFreezePlus()) {
		return "Freeze Plus";
	} else if (CG_isOSP()) {
		return "OSP";
	}

	return "Unknown";
}

/*
=============
CG_isFrozen

Original Freezetag frozenstate detection
=============
*/
qboolean CG_isFrozen( int clientNum )
{
	int	bits;

	bits = cg.snap->ps.stats[ STAT_CLIENTS_READY ];
	
	if ( bits & ( 1 << clientNum ) ) {
		return qtrue;
	} else {
		return qfalse;
	}
}
