// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_snapshot.c -- things that happen on snapshot transition,
// not necessarily every single rendered frame

#include "cg_local.h"



/*
==================
CG_ResetEntity
==================
*/
static void CG_ResetEntity( centity_t *cent ) {
	// if an event is set, assume it is new enough to use
	// if the event had timed out, it would have been cleared
	cent->previousEvent = 0;

	cent->trailTime = cg.snap->serverTime;

	VectorCopy (cent->currentState.origin, cent->lerpOrigin);
	VectorCopy (cent->currentState.angles, cent->lerpAngles);
	if ( cent->currentState.eType == ET_PLAYER ) {
		CG_ResetPlayerEntity( cent );
	}
}

/*
===============
CG_TransitionEntity

cent->nextState is moved to cent->currentState and events are fired
===============
*/
static void CG_TransitionEntity( centity_t *cent ) {
	cent->currentState = cent->nextState;
	cent->currentValid = qtrue;

	// reset if the entity wasn't in the last frame or was teleported
	if ( !cent->interpolate ) {
		CG_ResetEntity( cent );
	}

	// clear the next state.  if will be set by the next CG_SetNextSnap
	cent->interpolate = qfalse;

	// check for events
	CG_CheckEvents( cent );
}


/*
==================
CG_SetInitialSnapshot

This will only happen on the very first snapshot, or
on tourney restarts.  All other times will use 
CG_TransitionSnapshot instead.

FIXME: Also called by map_restart?
==================
*/
void CG_SetInitialSnapshot( snapshot_t *snap ) {
	int				i;
	centity_t		*cent;
	entityState_t	*state;
	clientInfo_t	*ci, *yourCi;
	char			yourName[MAX_QPATH];

	cg.snap = snap;

	BG_PlayerStateToEntityState( &snap->ps, &cg_entities[ snap->ps.clientNum ].currentState, qfalse );

	// sort out solid entities
	CG_BuildSolidList();

	CG_ExecuteNewServerCommands( snap->serverCommandSequence );

	// set our local weapon selection pointer to
	// what the server has indicated the current weapon is
	CG_Respawn();

	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		state = &cg.snap->entities[ i ];
		cent = &cg_entities[ state->number ];

		cent->currentState = *state;
		cent->interpolate = qfalse;
		cent->currentValid = qtrue;

		CG_ResetEntity( cent );

		// check for events
		CG_CheckEvents( cent );
	}

	cg.clientNum = snap->ps.clientNum;

	// there is an bug which seems to happen when switching teams out of spec
	// or being in spec a lot and then joining.  Symptoms are the cg.clietnNum
	// appears to get set incorrectly and this code can be the only culprit.

	// The statment above doesnt make any sense though since CG_SetIntialSnapshot is
	// only called on first snapshot or tourney restarts, correct? Consider moving to
	// CG_TransitionSnapShot instead?
	
	// simple check and name based comparison fix
	yourCi = &cgs.clientinfo[ cg.clientNum ];
	trap_Cvar_VariableStringBuffer( "name", yourName, sizeof( yourName ) );
	
	if ( !yourCi->infoValid || Q_stricmp( yourName, yourCi->name ) != 0 ) {
		// scan clientlist and do a name match
		for ( i = 0, ci = cgs.clientinfo; i < cgs.maxclients; i++, ci++  ) {
			if ( !ci->infoValid ) {
				continue;
			}
			
			if ( Q_stricmp( yourName, ci->name ) == 0 ) {
				cg.clientNum = i;
				yourCi = &cgs.clientinfo[ cg.clientNum ];
				break;
			}
		}
	}

	if (cg_debugNemesis.integer & NMS_DEBUG_SNAP) {
		CG_Printf("%sinitial snapshot, client num detected as %i\n", NMS_DEBUG_PREFIX, cg.clientNum);
	}

	if (cg_debugNemesis.integer & NMS_DEBUG_MODEL) {
		CG_Printf("%supdate client info request from ^3%s\n", NMS_DEBUG_PREFIX, "CG_SetInitialSnapshot");
	}
	CG_UpdateNewClientInfo();  // enemy model 
}


/*
===================
CG_TransitionSnapshot

The transition point from snap to nextSnap has passed
===================
*/
static void CG_TransitionSnapshot( void ) {
	centity_t			*cent;
	snapshot_t			*oldFrame;
	int					i;
	qboolean			redoModels;  // Nemesis - enemyModel
	qboolean			redoStats;
	clientInfo_t		ci;

	if ( !cg.snap ) {
		CG_Error( "CG_TransitionSnapshot: NULL cg.snap" );
	}
	if ( !cg.nextSnap ) {
		CG_Error( "CG_TransitionSnapshot: NULL cg.nextSnap" );
	}

	// execute any server string commands before transitioning entities
	CG_ExecuteNewServerCommands( cg.nextSnap->serverCommandSequence );

	// if we had a map_restart, set everthing with initial

	// clear the currentValid flag for all entities in the existing snapshot
	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		cent = &cg_entities[ cg.snap->entities[ i ].number ];
		cent->currentValid = qfalse;
	}

	// Nemesis - enemyModel
	redoModels = qfalse;
	redoStats = qfalse;

	ci = cgs.clientinfo[cg.clientNum];
	if (ci.team != stats.team) {
		redoModels = qtrue;
		redoStats = qtrue;

		// superhud - reparse because of team and enemy color options
		CG_Hud_Parse();

		if (cg_debugNemesis.integer & NMS_DEBUG_STATS  ||
			cg_debugNemesis.integer & NMS_DEBUG_SNAP) {
			CG_Printf("%steam change from ^3%i ^7to ^3%i\n", NMS_DEBUG_PREFIX, stats.team, ci.team);
		}
	}

	// if this is us, store away our damage for stats
	if (redoStats == qfalse && 
		cg.snap->ps.clientNum == cg.clientNum && 
		cg.nextSnap->ps.clientNum == cg.clientNum) {

		int given = cg.nextSnap->ps.persistant[PERS_HITS];

		int takenHPre = cg.snap->ps.stats[STAT_HEALTH];
		int takenHPost = cg.nextSnap->ps.stats[STAT_HEALTH];
		int takenAPre = cg.snap->ps.stats[STAT_ARMOR];
		int takenAPost = cg.nextSnap->ps.stats[STAT_ARMOR];

		if (cg_debugNemesis.integer & NMS_DEBUG_STATS && stats.damageGiven != given) {
			CG_Printf("%sgiven ^3%i ^7damage\n", NMS_DEBUG_PREFIX, given - stats.damageGiven);
		}

		stats.damageGiven = given;

		// cap at 100, otherwise the ticking down to 100 looks like damage
		if (takenHPre > 100) takenHPre = 100;
		if (takenHPost > 100) takenHPost = 100;
		if (takenAPre > 100) takenAPre = 100;
		if (takenAPost > 100) takenAPost = 100;

		// cap at 0 (as we die then) or this is false damamge
		if (takenHPre < 0) takenHPre = 0;
		if (takenHPost < 0) takenHPost = 0;
		if (takenAPre < 0) takenAPre = 0;
		if (takenAPost < 0) takenAPost = 0;

		if (takenHPre > takenHPost) {
			stats.damageReceived += takenHPre - takenHPost;

			if (cg_debugNemesis.integer & NMS_DEBUG_STATS) {
				CG_Printf("%sreceived ^3%i ^7health damage\n", NMS_DEBUG_PREFIX, takenHPre - takenHPost);
			}
		}

		if (takenAPre > takenAPost) {
			stats.damageReceived += takenAPre - takenAPost;

			if (cg_debugNemesis.integer & NMS_DEBUG_STATS) {
				CG_Printf("%sreceived ^3%i ^7armor damage\n", NMS_DEBUG_PREFIX, takenAPre - takenAPost);
			}
		}
	}

#ifdef NMS_MULTIVIEW
	// OSP - multiview handling
	if (cg.snap->ps.powerups[PW_MVCLIENTLIST] != cg.mvClientList) {
		CG_mvProcessClientList();
	}
#endif

	// move nextSnap to snap and do the transitions
	oldFrame = cg.snap;
	cg.snap = cg.nextSnap;

	BG_PlayerStateToEntityState( &cg.snap->ps, &cg_entities[ cg.snap->ps.clientNum ].currentState, qfalse );
	cg_entities[ cg.snap->ps.clientNum ].interpolate = qfalse;

	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		cent = &cg_entities[ cg.snap->entities[ i ].number ];
		CG_TransitionEntity( cent );
	}

	cg.nextSnap = NULL;

	// check for playerstate transition events
	if ( oldFrame ) {
		playerState_t	*ops, *ps;

		ops = &oldFrame->ps;
		ps = &cg.snap->ps;
		// teleporting checks are irrespective of prediction
		if ( ( ps->eFlags ^ ops->eFlags ) & EF_TELEPORT_BIT ) {
			cg.thisFrameTeleport = qtrue;	// will be cleared by prediction code
		}

		// if we are not doing client side movement prediction for any
		// reason, then the client events and view changes will be issued now
		if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW)
			|| cg_nopredict.integer || cg_syncronousClients.integer ) {
			CG_TransitionPlayerState( ps, ops );
		}
	}

	// now this snapshot events have fired, reset the stats
	if (redoStats == qtrue) {
		memset(&stats, 0, sizeof(stats));
	}
	stats.team = ci.team;

	// Nemesis - enemyModel
	if ( redoModels ) {
		if (cg_debugNemesis.integer & NMS_DEBUG_MODEL) {
			CG_Printf("%supdate client info request from ^3%s\n", NMS_DEBUG_PREFIX, "CG_TransitionSnapshot");
		}

		CG_UpdateNewClientInfo();
	}

	// Spectator scrolling text
	/*if( cg.specWindow != NULL && cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
		cg.specWindow->effects = WFX_SCROLLLEFT; // This is what is causing Nemesis to crash!
		CG_windowFree(cg.specWindow);
		cg.specWindow = NULL;
	}*/
}

/*
===================
CG_SetNextSnap

A new snapshot has just been read in from the client system.
===================
*/
static void CG_SetNextSnap( snapshot_t *snap ) {
	int					num;
	entityState_t		*es;
	centity_t			*cent;

	cg.nextSnap = snap;

	BG_PlayerStateToEntityState( &snap->ps, &cg_entities[ snap->ps.clientNum ].nextState, qfalse );
	cg_entities[ cg.snap->ps.clientNum ].interpolate = qtrue;

	// check for extrapolation errors
	for ( num = 0 ; num < snap->numEntities ; num++ ) {
		es = &snap->entities[num];
		cent = &cg_entities[ es->number ];

		cent->nextState = *es;

		// if this frame is a teleport, or the entity wasn't in the
		// previous frame, don't interpolate
		if ( !cent->currentValid || ( ( cent->currentState.eFlags ^ es->eFlags ) & EF_TELEPORT_BIT )  ) {
			cent->interpolate = qfalse;
		} else {
			cent->interpolate = qtrue;
		}
	}

	// if the next frame is a teleport for the playerstate, we
	// can't interpolate during demos
	if ( cg.snap && ( ( snap->ps.eFlags ^ cg.snap->ps.eFlags ) & EF_TELEPORT_BIT ) ) {
		cg.nextFrameTeleport = qtrue;
	} else {
		cg.nextFrameTeleport = qfalse;
	}

	// if changing follow mode, don't interpolate
	if ( cg.nextSnap->ps.clientNum != cg.snap->ps.clientNum ) {
		cg.nextFrameTeleport = qtrue;
	}

	// if changing server restarts, don't interpolate
	if ( ( cg.nextSnap->snapFlags ^ cg.snap->snapFlags ) & SNAPFLAG_SERVERCOUNT ) {
		cg.nextFrameTeleport = qtrue;
	}

	// sort out solid entities
	CG_BuildSolidList();
}


/*
========================
CG_ReadNextSnapshot

This is the only place new snapshots are requested
This may increment cgs.processedSnapshotNum multiple
times if the client system fails to return a
valid snapshot.
========================
*/
static snapshot_t *CG_ReadNextSnapshot( void ) {
	qboolean	r;
	snapshot_t	*dest;

	if ( cg.latestSnapshotNum > cgs.processedSnapshotNum + 1000 ) {
		CG_Error( "CG_ReadNextSnapshot: way out of range, %i > %i", 
			cg.latestSnapshotNum, cgs.processedSnapshotNum );
	}

	while ( cgs.processedSnapshotNum < cg.latestSnapshotNum ) {
		// decide which of the two slots to load it into
		if ( cg.snap == &cg.activeSnapshots[0] ) {
			dest = &cg.activeSnapshots[1];
		} else {
			dest = &cg.activeSnapshots[0];
		}

		// try to read the snapshot from the client system
		cgs.processedSnapshotNum++;
		r = trap_GetSnapshot( cgs.processedSnapshotNum, dest );

		// if it succeeded, return
		if ( r ) {
			if (cg_hudType.integer == HUD_TYPE_SUPERHUD) {
				CG_Hud_AddLagometerSnapshotInfo(dest);
			} else {
				CG_AddLagometerSnapshotInfo( dest );
			}
			return dest;
		}

		// a GetSnapshot will return failure if the snapshot
		// never arrived, or  is so old that its entities
		// have been shoved off the end of the circular
		// buffer in the client system.

		// record as a dropped packet
		if (cg_hudType.integer == HUD_TYPE_SUPERHUD) {
			CG_Hud_AddLagometerSnapshotInfo(NULL);
		} else {
			CG_AddLagometerSnapshotInfo( NULL );
		}

		// If there are additional snapshots, continue trying to
		// read them.
	}

	// nothing left to read
	return NULL;
}


/*
============
CG_ProcessSnapshots

We are trying to set up a renderable view, so determine
what the simulated time is, and try to get snapshots
both before and after that time if available.

If we don't have a valid cg.snap after exiting this function,
then a 3D game view cannot be rendered.  This should only happen
right after the initial connection.  After cg.snap has been valid
once, it will never turn invalid.

Even if cg.snap is valid, cg.nextSnap may not be, if the snapshot
hasn't arrived yet (it becomes an extrapolating situation instead
of an interpolating one)

============
*/
void CG_ProcessSnapshots( void ) {
	snapshot_t		*snap;
	int				n;

	// see what the latest snapshot the client system has is
	trap_GetCurrentSnapshotNumber( &n, &cg.latestSnapshotTime );
	if ( n != cg.latestSnapshotNum ) {
		if ( n < cg.latestSnapshotNum ) {
			// this should never happen
			CG_Error( "CG_ProcessSnapshots: n < cg.latestSnapshotNum" );
		}
		cg.latestSnapshotNum = n;
	}

	// If we have yet to receive a snapshot, check for it.
	// Once we have gotten the first snapshot, cg.snap will
	// always have valid data for the rest of the game
	while ( !cg.snap ) {
		snap = CG_ReadNextSnapshot();
		if ( !snap ) {
			// we can't continue until we get a snapshot
			return;
		}

		// set our weapon selection to what
		// the playerstate is currently using
		if ( !( snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
			CG_SetInitialSnapshot( snap );
		}
	}

	// loop until we either have a valid nextSnap with a serverTime
	// greater than cg.time to interpolate towards, or we run
	// out of available snapshots
	do {
		// if we don't have a nextframe, try and read a new one in
		if ( !cg.nextSnap ) {
			snap = CG_ReadNextSnapshot();

			// if we still don't have a nextframe, we will just have to
			// extrapolate
			if ( !snap ) {
				break;
			}

			CG_SetNextSnap( snap );

			// if time went backwards, we have a level restart
			if ( cg.nextSnap->serverTime < cg.snap->serverTime ) {
				CG_Error( "CG_ProcessSnapshots: Server time went backwards" );
			}
		}

		// if our time is < nextFrame's, we have a nice interpolating state
		if ( cg.time >= cg.snap->serverTime && cg.time < cg.nextSnap->serverTime ) {
			break;
		}

		// we have passed the transition from nextFrame to frame
		CG_TransitionSnapshot();
	} while ( 1 );

	// assert our valid conditions upon exiting
	if ( cg.snap == NULL ) {
		CG_Error( "CG_ProcessSnapshots: cg.snap == NULL" );
	}
	if ( cg.time < cg.snap->serverTime ) {
		// this can happen right after a vid_restart
		cg.time = cg.snap->serverTime;
	}
	if ( cg.nextSnap != NULL && cg.nextSnap->serverTime <= cg.time ) {
		CG_Error( "CG_ProcessSnapshots: cg.nextSnap->serverTime <= cg.time" );
	}

}

