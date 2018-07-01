// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"

//==========================================================================

/* Auto announce constants */
#define ITEM_YELLOW_ARMOR	1
#define ITEM_RED_ARMOR		2
#define ITEM_MEGA_HEALTH	4
#define ITEM_TELEPORTER		8
#define ITEM_MEDIKIT		16
#define ITEM_QUAD			32
#define ITEM_HASTE			64
#define ITEM_INVISIBILITY	128
#define ITEM_REGENERATION	256
#define ITEM_FLIGHT			512
#define ITEM_ENVIRO_SUIT	1024


/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = "Tied for ";
	} else {
		t = "";
	}

	if ( rank == 1 ) {
		s = S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
	} else if ( rank == 2 ) {
		s = S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
	} else if ( rank == 3 ) {
		s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
	} else if ( rank == 11 ) {
		s = "11th";
	} else if ( rank == 12 ) {
		s = "12th";
	} else if ( rank == 13 ) {
		s = "13th";
	} else if ( rank % 10 == 1 ) {
		s = va("%ist", rank);
	} else if ( rank % 10 == 2 ) {
		s = va("%ind", rank);
	} else if ( rank % 10 == 3 ) {
		s = va("%ird", rank);
	} else {
		s = va("%ith", rank);
	}

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}

// Nemesis - Function from Darrell's freeze
void CG_BodyObituary( entityState_t *ent, char *targetName ) {
	int	target, attacker;
	char	*message;
	char	*message2;
	const char	*attackerInfo;
	char	attackerName[ 32 ];
	gender_t	gender;
	char	*s;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;

	attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
	if ( !attackerInfo ) return;
	Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof ( attackerName ) - 2 );
	strcat( attackerName, S_COLOR_WHITE );

	if ( rand() & 1 ) {
		message = "thawed";
		message2 = "";
	} else {
		gender = cgs.clientinfo[ target ].gender;
		message = "evicted";
		if ( gender == GENDER_FEMALE ) {
			message2 = "from her igloo";
		} else if ( gender == GENDER_NEUTER ) {
			message2 = "from its igloo";
		} else {
			message2 = "from his igloo";
		}
	}

	if ( attacker == cg.snap->ps.clientNum ) {
		s = va( "You thawed %s", targetName );
		CG_CenterPrint( s );
	}
	if ( target == cg.snap->ps.clientNum ) {
		s = va( "%s unfroze you", attackerName );
		CG_CenterPrint( s );
	}
	CG_Printf( "%s %s %s %s\n", attackerName, message, targetName, message2 );
}

// End

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent ) {
	int			mod;
	int			target, attacker;
	char		*message;
	char		*message2;
	const char	*targetInfo;
	const char	*attackerInfo;
	char		targetName[32];
	char		attackerName[32];
	gender_t	gender;
	clientInfo_t	*ci;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;
	mod = ent->eventParm;

	if ( target < 0 || target >= MAX_CLIENTS ) {
		CG_Error( "CG_Obituary: target out of range" );
	}
	ci = &cgs.clientinfo[target];

	if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
	} else {
		attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
	}

	targetInfo = CG_ConfigString( CS_PLAYERS + target );
	if ( !targetInfo ) {
		return;
	}
	Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	strcat( targetName, S_COLOR_WHITE );

	message2 = "";

	// check for single client messages

	switch( mod ) {
	case MOD_SUICIDE:
		message = "suicides";
		break;
	case MOD_FALLING:
		message = "cratered";
		break;
	case MOD_CRUSH:
		message = "was squished";
		break;
	case MOD_WATER:
		message = "sank like a rock";
		break;
	case MOD_SLIME:
		message = "melted";
		break;
	case MOD_LAVA:
		message = "does a back flip into the lava";
		break;
	case MOD_TARGET_LASER:
		message = "saw the light";
		break;
	case MOD_TRIGGER_HURT:
		message = "was in the wrong place";
		break;
	// Nemesis - Freeze
	case MOD_UNKNOWN:
		CG_BodyObituary( ent, targetName );
		return;
	// End
	default:
		message = NULL;
		break;
	}

	// why NG uses MOD_SLIME when you are thawed I have no idea!
	if (target == cg.clientNum && CG_isNoGhost() && CG_isFreezeTag() && mod == MOD_SLIME) {
		// there's a chance you died in the slime, but more likely you just got thawed
		if (cg_debugNemesis.integer & NMS_DEBUG_STATS) {
			CG_Printf("%sNoGhost thaw detected\n", NMS_DEBUG_PREFIX);
		}
			
	} else if (mod != MOD_UNKNOWN && message != NULL && target == cg.clientNum) {
		// world killed you
		stats.suicides++;

		if (cg_debugNemesis.integer & NMS_DEBUG_STATS) {
			char *worldType;
			switch (mod) {
				case MOD_SUICIDE:
					worldType = "suicide";
					break;
				case MOD_FALLING:
					worldType = "falling";
					break;
				case MOD_CRUSH:
					worldType = "crushed";
					break;
				case MOD_WATER:
					worldType = "drowned";
					break;
				case MOD_SLIME:
					worldType = "slime";
					break;
				case MOD_LAVA:
					worldType = "lava";
					break;
				case MOD_TARGET_LASER:
					worldType = "laser";
					break;
				case MOD_TRIGGER_HURT:
					worldType = "wrong place";
					break;
				case MOD_UNKNOWN:
					worldType = "unknown";
					break;
				default:
					worldType = "default";
					break;
			}
			CG_Printf("%skilled by world (%i - %s)\n", NMS_DEBUG_PREFIX, mod, worldType);
		}
	}

	if (attacker == target) {

		// killed youself
		if (target == cg.clientNum) {
			stats.suicides++;

			if (cg_debugNemesis.integer & NMS_DEBUG_STATS) {
				CG_Printf("%skilled yourself\n", NMS_DEBUG_PREFIX);
			}
		}

		gender = ci->gender;
		switch (mod) {
		case MOD_GRENADE_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "tripped on her own grenade";
			else if ( gender == GENDER_NEUTER )
				message = "tripped on its own grenade";
			else
				message = "tripped on his own grenade";
			break;
		case MOD_ROCKET_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "blew herself up";
			else if ( gender == GENDER_NEUTER )
				message = "blew itself up";
			else
				message = "blew himself up";
			break;
		case MOD_PLASMA_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "melted herself";
			else if ( gender == GENDER_NEUTER )
				message = "melted itself";
			else
				message = "melted himself";
			break;
		case MOD_BFG_SPLASH:
			message = "should have used a smaller gun";
			break;
		default:
			if ( gender == GENDER_FEMALE )
				message = "killed herself";
			else if ( gender == GENDER_NEUTER )
				message = "killed itself";
			else
				message = "killed himself";
			break;
		}
	}

	if (message) {
		CG_Printf( "%s %s.\n", targetName, message);
		return;
	}

	// check for kill messages from the current clientNum
	if ( attacker == cg.snap->ps.clientNum ) {
		char	*s;

		if( cgs.gametype == GT_TOURNAMENT ) {
			return;
		}

		if ( cgs.gametype < GT_TEAM ) {
			s = va("You fragged %s\n%s place with %i", targetName, 
				CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
				cg.snap->ps.persistant[PERS_SCORE] );
		} else {
			s = va("You fragged %s", targetName );
		}
		// Nemesis - Freeze
		if ( CG_isFreezeTag() ) {
			s = va("You froze %s", targetName );
		}
		// End
		CG_CenterPrint( s );

		// print the text message as well
	}

	// check for double client messages
	if ( !attackerInfo ) {
		attacker = ENTITYNUM_WORLD;
		strcpy( attackerName, "noname" );
	} else {
		Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
		strcat( attackerName, S_COLOR_WHITE );
		// check for kill messages about the current clientNum
		if ( target == cg.snap->ps.clientNum ) {
			Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
			strcpy( killedby, cg.killerName );
		}
	}

	if ( attacker != ENTITYNUM_WORLD ) {
		switch (mod) {
		case MOD_GRAPPLE:
			message = "was caught by";
			break;
		case MOD_GAUNTLET:
			message = "was pummeled by";
			if (target == cg.clientNum) {
				stats.weaponDeaths[WP_GAUNTLET]++;
			} else if (attacker == cg.clientNum) {
				stats.weaponKills[WP_GAUNTLET]++;
			}
			break;
		case MOD_MACHINEGUN:
			message = "was machinegunned by";
			if (target == cg.clientNum) {
				stats.weaponDeaths[WP_MACHINEGUN]++;
			} else if (attacker == cg.clientNum) {
				stats.weaponKills[WP_MACHINEGUN]++;
			}
			break;
		case MOD_SHOTGUN:
			message = "was gunned down by";
			if (target == cg.clientNum) {
				stats.weaponDeaths[WP_SHOTGUN]++;
			} else if (attacker == cg.clientNum) {
				stats.weaponKills[WP_SHOTGUN]++;
			}
			break;
		case MOD_GRENADE:
			message = "ate";
			message2 = "'s grenade";
			if (target == cg.clientNum) {
				stats.weaponDeaths[WP_GRENADE_LAUNCHER]++;
			} else if (attacker == cg.clientNum) {
				stats.weaponKills[WP_GRENADE_LAUNCHER]++;
			}
			break;
		case MOD_GRENADE_SPLASH:
			message = "was shredded by";
			message2 = "'s shrapnel";
			if (target == cg.clientNum) {
				stats.weaponDeaths[WP_GRENADE_LAUNCHER]++;
			} else if (attacker == cg.clientNum) {
				stats.weaponKills[WP_GRENADE_LAUNCHER]++;
			}
			break;
		case MOD_ROCKET:
			message = "ate";
			message2 = "'s rocket";
			if (target == cg.clientNum) {
				stats.weaponDeaths[WP_ROCKET_LAUNCHER]++;
			} else if (attacker == cg.clientNum) {
				stats.weaponKills[WP_ROCKET_LAUNCHER]++;
			}
			break;
		case MOD_ROCKET_SPLASH:
			message = "almost dodged";
			message2 = "'s rocket";
			if (target == cg.clientNum) {
				stats.weaponDeaths[WP_ROCKET_LAUNCHER]++;
			} else if (attacker == cg.clientNum) {
				stats.weaponKills[WP_ROCKET_LAUNCHER]++;
			}
			break;
		case MOD_PLASMA:
			message = "was melted by";
			message2 = "'s plasmagun";
			if (target == cg.clientNum) {
				stats.weaponDeaths[WP_PLASMAGUN]++;
			} else if (attacker == cg.clientNum) {
				stats.weaponKills[WP_PLASMAGUN]++;
			}
			break;
		case MOD_PLASMA_SPLASH:
			message = "was melted by";
			message2 = "'s plasmagun";
			if (target == cg.clientNum) {
				stats.weaponDeaths[WP_PLASMAGUN]++;
			} else if (attacker == cg.clientNum) {
				stats.weaponKills[WP_PLASMAGUN]++;
			}
			break;
		case MOD_RAILGUN:
			message = "was railed by";
			if (target == cg.clientNum) {
				stats.weaponDeaths[WP_RAILGUN]++;
			} else if (attacker == cg.clientNum) {
				stats.weaponKills[WP_RAILGUN]++;
			}
			break;
		case MOD_LIGHTNING:
			message = "was electrocuted by";
			if (target == cg.clientNum) {
				stats.weaponDeaths[WP_LIGHTNING]++;
			} else if (attacker == cg.clientNum) {
				stats.weaponKills[WP_LIGHTNING]++;
			}
			break;
		case MOD_BFG:
		case MOD_BFG_SPLASH:
			message = "was blasted by";
			message2 = "'s BFG";
			break;
		case MOD_TELEFRAG:
			message = "tried to invade";
			message2 = "'s personal space";
			break;
		default:
			// Nemesis - Freeze
			if ( CG_isFreezeTag() ) {
				message = "was frozen by";
			} else {
				message = "was killed by";
			}
			// End
			break;
		}

		if (message) {
			CG_Printf( "%s %s %s%s\n", 
				targetName, message, attackerName, message2);
			return;
		}
	}

	// we don't know what it was
	CG_Printf( "%s died.\n", targetName );
}


//==========================================================================

/*
===============
CG_UseItem
===============
*/
static void CG_UseItem( centity_t *cent ) {
	int			itemNum;
	gitem_t		*item;
	entityState_t *es;

	es = &cent->currentState;
	
	itemNum = (es->event & ~EV_EVENT_BITS) - EV_USE_ITEM0;
	if ( itemNum < 0 || itemNum > HI_NUM_HOLDABLE ) {
		itemNum = 0;
	}

	// print a message if the local player
	if ( es->number == cg.snap->ps.clientNum ) {
		if ( !itemNum ) {
			CG_CenterPrint( "No item to use" );
		} else {
			item = BG_FindItemForHoldable( itemNum );
			CG_CenterPrint( va("Use %s", item->pickup_name) );
		}
	}

	switch ( itemNum ) {
	default:
	case HI_NONE:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useNothingSound );
		break;

	case HI_TELEPORTER:
		break;

	case HI_MEDKIT:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.medkitSound );
		break;
	}

}

/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
static void CG_ItemPickup( int itemNum ) {
	cg.itemPickup = itemNum;
	cg.itemPickupTime = cg.time;
	cg.itemPickupBlendTime = cg.time;
	// see if it should be the grabbed weapon
	if ( bg_itemlist[itemNum].giType == IT_WEAPON ) {
		// select it immediately
		if ( cg_autoswitch.integer && bg_itemlist[itemNum].giTag != WP_MACHINEGUN ) {
			cg.weaponSelectTime = cg.time;
			cg.weaponSelect = bg_itemlist[itemNum].giTag;
		}
	}

}


/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) {
	char	*snd;

	// don't do more than two pain sounds a second
	if ( cg.time - cent->pe.painTime < 500 ) {
		return;
	}

	if ( health < 25 ) {
		snd = "*pain25_1.wav";
	} else if ( health < 50 ) {
		snd = "*pain50_1.wav";
	} else if ( health < 75 ) {
		snd = "*pain75_1.wav";
	} else {
		snd = "*pain100_1.wav";
	}
	trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE, 
		CG_CustomSound( cent->currentState.number, snd ) );

	// save pain time for programitic twitch animation
	cent->pe.painTime = cg.time;
	cent->pe.painDirection ^= 1;
}

/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
#define	DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(x"\n");}
void CG_EntityEvent( centity_t *cent, vec3_t position ) {
	entityState_t	*es;
	int				event;
	vec3_t			dir;
	const char		*s;
	int				clientNum;
	clientInfo_t	*ci;

	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;

	if ( cg_debugEvents.integer ) {
		CG_Printf( "ent:%3i  event:%3i ", es->number, event );
	}

	if ( !event ) {
		DEBUGNAME("ZEROEVENT");
		return;
	}

	clientNum = es->clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	switch ( event ) {
	//
	// movement generated events
	//
	case EV_FOOTSTEP:
		DEBUGNAME("EV_FOOTSTEP");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ ci->footsteps ][rand()&3] );
		}
		break;
	case EV_FOOTSTEP_METAL:
		DEBUGNAME("EV_FOOTSTEP_METAL");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_METAL ][rand()&3] );
		}
		break;
	case EV_FOOTSPLASH:
		DEBUGNAME("EV_FOOTSPLASH");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_FOOTWADE:
		DEBUGNAME("EV_FOOTWADE");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_SWIM:
		DEBUGNAME("EV_SWIM");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;


	case EV_FALL_SHORT:
		DEBUGNAME("EV_FALL_SHORT");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.landSound );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -8;
			cg.landTime = cg.time;
		}
		break;
	case EV_FALL_MEDIUM:
		DEBUGNAME("EV_FALL_MEDIUM");
		// use normal pain sound
                trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*pain100_1.wav" ) );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -16;
			cg.landTime = cg.time;
		}
		break;
	case EV_FALL_FAR:
		DEBUGNAME("EV_FALL_FAR");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*fall1.wav" ) );
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -24;
			cg.landTime = cg.time;
		}
		break;

	case EV_STEP_4:
	case EV_STEP_8:
	case EV_STEP_12:
	case EV_STEP_16:		// smooth out step up transitions
		DEBUGNAME("EV_STEP");
	{
		float	oldStep;
		int		delta;
		int		step;

		if ( clientNum != cg.predictedPlayerState.clientNum ) {
			break;
		}
		// if we are interpolating, we don't need to smooth steps
		if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
			cg_nopredict.integer || cg_syncronousClients.integer ) {
			break;
		}
		// check for stepping up before a previous step is completed
		delta = cg.time - cg.stepTime;
		if (delta < STEP_TIME) {
			oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
		} else {
			oldStep = 0;
		}

		// add this amount
		step = 4 * (event - EV_STEP_4 + 1 );
		cg.stepChange = oldStep + step;
		if ( cg.stepChange > MAX_STEP_CHANGE ) {
			cg.stepChange = MAX_STEP_CHANGE;
		}
		cg.stepTime = cg.time;
		break;
	}

	case EV_JUMP_PAD:
		DEBUGNAME("EV_JUMP_PAD");
		// boing sound at origin, jump sound on player
		trap_S_StartSound ( cent->lerpOrigin, -1, CHAN_VOICE, cgs.media.jumpPadSound );
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		break;

	case EV_JUMP:
		DEBUGNAME("EV_JUMP");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		break;
	case EV_TAUNT:
		DEBUGNAME("EV_TAUNT");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt.wav" ) );
		break;
	case EV_WATER_TOUCH:
		DEBUGNAME("EV_WATER_TOUCH");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
		break;
	case EV_WATER_LEAVE:
		DEBUGNAME("EV_WATER_LEAVE");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
		break;
	case EV_WATER_UNDER:
		DEBUGNAME("EV_WATER_UNDER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
		break;
	case EV_WATER_CLEAR:
		DEBUGNAME("EV_WATER_CLEAR");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*gasp.wav" ) );
		break;

	case EV_ITEM_PICKUP:
		DEBUGNAME("EV_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];

			if (es->clientNum == cg.clientNum) {
				// ok, we just picked something up

				if (!strcmp(item->classname, "item_armor_combat")) {
					stats.armorYA++;
					stats.armorTotal += 50;

					if (cgs.gametype >= GT_TEAM && (cg_autoAnnounce.integer & ITEM_YELLOW_ARMOR)) {
					    trap_SendClientCommand( va( "say_team Taken %s\n", "Yellow Armor" ) );
					}

				} else if (!strcmp(item->classname, "item_armor_body")) {
					stats.armorRA++;
					stats.armorTotal += 100;

					if (cgs.gametype >= GT_TEAM && (cg_autoAnnounce.integer & ITEM_RED_ARMOR)) {
					    trap_SendClientCommand( va( "say_team Taken %s\n", "Red Armor" ) );
					}

				} else if (!strcmp(item->classname, "item_health_mega")) {
					stats.healthMH++;
					stats.healthTotal += 100;

					if (cgs.gametype >= GT_TEAM && (cg_autoAnnounce.integer & ITEM_MEGA_HEALTH)) {
						trap_SendClientCommand( va( "say_team Taken %s\n", "Mega Health" ) );
					}

				} else if (!strcmp(item->classname, "item_health_large")) {
					stats.healthTotal += 50;

				} else if (!strcmp(item->classname, "item_health")) {
					stats.healthTotal += 25;

				} else if (!strcmp(item->classname, "item_health_small")) {
					stats.healthTotal += 5;

				} else if (!strcmp(item->classname, "item_armor_shard")) {
					stats.armorTotal += 5;

				} else if ( cgs.gametype >= GT_TEAM && 
							(cg_autoAnnounce.integer & ITEM_TELEPORTER) &&
							!strcmp( item->classname, "holdable_teleporter" ) ) {
					trap_SendClientCommand( va( "say_team Taken %s\n", "Holdable Teleporter" ) );

				} else if ( cgs.gametype >= GT_TEAM && 
							(cg_autoAnnounce.integer & ITEM_MEDIKIT) &&
					        !strcmp( item->classname, "holdable_medkit" ) ) {
					trap_SendClientCommand( va( "say_team Taken %s\n", "Medikit" ) );

				} else if ( cgs.gametype >= GT_TEAM && 
							(cg_autoAnnounce.integer & ITEM_QUAD) &&
					        !strcmp( item->classname, "item_quad" ) ) {
					trap_SendClientCommand( va( "say_team Taken %s\n", "Quad" ) );

				} else if ( cgs.gametype >= GT_TEAM && 
							(cg_autoAnnounce.integer & ITEM_ENVIRO_SUIT) &&
					        !strcmp( item->classname, "item_enviro" ) ) {
					trap_SendClientCommand( va( "say_team Taken %s\n", "Enviromental Suit" ) );

				} else if ( cgs.gametype >= GT_TEAM && 
							(cg_autoAnnounce.integer & ITEM_HASTE) &&
					        !strcmp( item->classname, "item_haste" ) ) {
					trap_SendClientCommand( va( "say_team Taken %s\n", "Haste" ) );

				} else if ( cgs.gametype >= GT_TEAM && 
							(cg_autoAnnounce.integer & ITEM_INVISIBILITY) &&
					        !strcmp( item->classname, "item_invis" ) ) {
					trap_SendClientCommand( va( "say_team Taken %s\n", "Invisibility" ) );

				} else if ( cgs.gametype >= GT_TEAM && 
							(cg_autoAnnounce.integer & ITEM_REGENERATION) &&
					        !strcmp( item->classname, "item_regen" ) ) {
					trap_SendClientCommand( va( "say_team Taken %s\n", "Regeneration" ) );

				} else if ( cgs.gametype >= GT_TEAM && 
							(cg_autoAnnounce.integer & ITEM_FLIGHT) &&
					        !strcmp( item->classname, "item_flight" ) ) {
					trap_SendClientCommand( va( "say_team Taken %s\n", "Flight" ) );
				}
			}

			// Hoping client side prediction doesnt screw this up
			//if( Q_stricmp( item->pickup_sound, "sound/teamplay/flagtk_red.wav" ) == 0 ) {
			//	break;
			//}
			//if( Q_stricmp( item->pickup_sound, "sound/teamplay/flagtk_blu.wav" ) == 0 ) {
			//	break;
			//}

			// powerups and team items will have a separate global sound, this one
			// will be played at prediction time
			if ( item->giType == IT_POWERUP || item->giType == IT_TEAM) {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( "sound/items/n_health.wav" ) );
			} else {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound ) );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	case EV_GLOBAL_ITEM_PICKUP:
		DEBUGNAME("EV_GLOBAL_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];

			// 1.32 Flag taken sounds
			if( Q_stricmp( item->pickup_sound, "sound/teamplay/flagtk_red.wav" ) == 0 ) {
				if (cg.snap->ps.powerups[PW_REDFLAG]) {
				} else {
					if (cgs.clientinfo[cg.clientNum].team == TEAM_RED) {
						CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
					} else if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE) {
						CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
					}
				}
				break;
			}
			if( Q_stricmp( item->pickup_sound, "sound/teamplay/flagtk_blu.wav" ) == 0 ) {
				if (cg.snap->ps.powerups[PW_BLUEFLAG]) {
				} else {
					if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE) {
						CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
					} else if (cgs.clientinfo[cg.clientNum].team == TEAM_RED) {
						CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
					}
				}
				break;
			}
			// powerup pickups are global
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound( item->pickup_sound ) );

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	//
	// weapon events
	//
	case EV_NOAMMO:
		DEBUGNAME("EV_NOAMMO");
//		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
		if ( es->number == cg.snap->ps.clientNum ) {
			CG_OutOfAmmoChange();
		}
		break;
	case EV_CHANGE_WEAPON:
		DEBUGNAME("EV_CHANGE_WEAPON");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
		break;
	case EV_FIRE_WEAPON:
		DEBUGNAME("EV_FIRE_WEAPON");
		CG_FireWeapon( cent );
		break;

	case EV_USE_ITEM0:
		DEBUGNAME("EV_USE_ITEM0");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM1:
		DEBUGNAME("EV_USE_ITEM1");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM2:
		DEBUGNAME("EV_USE_ITEM2");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM3:
		DEBUGNAME("EV_USE_ITEM3");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM4:
		DEBUGNAME("EV_USE_ITEM4");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM5:
		DEBUGNAME("EV_USE_ITEM5");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM6:
		DEBUGNAME("EV_USE_ITEM6");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM7:
		DEBUGNAME("EV_USE_ITEM7");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM8:
		DEBUGNAME("EV_USE_ITEM8");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM9:
		DEBUGNAME("EV_USE_ITEM9");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM10:
		DEBUGNAME("EV_USE_ITEM10");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM11:
		DEBUGNAME("EV_USE_ITEM11");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM12:
		DEBUGNAME("EV_USE_ITEM12");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM13:
		DEBUGNAME("EV_USE_ITEM13");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM14:
		DEBUGNAME("EV_USE_ITEM14");
		CG_UseItem( cent );
		break;

	//=================================================================

	//
	// other events
	//
	case EV_PLAYER_TELEPORT_IN:
		DEBUGNAME("EV_PLAYER_TELEPORT_IN");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleInSound );
		CG_SpawnEffect( position);
		break;

	case EV_PLAYER_TELEPORT_OUT:
		DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound );
		CG_SpawnEffect(  position);
		break;

	case EV_ITEM_POP:
		DEBUGNAME("EV_ITEM_POP");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;
	case EV_ITEM_RESPAWN:
		DEBUGNAME("EV_ITEM_RESPAWN");
		cent->miscTime = cg.time;	// scale up from this
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;

	case EV_GRENADE_BOUNCE:
		DEBUGNAME("EV_GRENADE_BOUNCE");
		if ( rand() & 1 ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, trap_S_RegisterSound("sound/weapons/grenade/hgrenb1a.wav") );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, trap_S_RegisterSound("sound/weapons/grenade/hgrenb2a.wav") );
		}
		break;

	//
	// missile impacts
	//
	case EV_MISSILE_HIT:
		DEBUGNAME("EV_MISSILE_HIT");
		ByteToDir( es->eventParm, dir );
		CG_MissileHitPlayer( es->weapon, position, dir, es->otherEntityNum );
		break;

	case EV_MISSILE_MISS:
		DEBUGNAME("EV_MISSILE_MISS");
		ByteToDir( es->eventParm, dir );
		CG_MissileHitWall( es, es->weapon, 0, position, dir );
		break;

	case EV_RAILTRAIL:
		DEBUGNAME("EV_RAILTRAIL");
		cent->currentState.weapon = WP_RAILGUN;
		// if the end was on a nomark surface, don't make an explosion

		// Nemesis - Custom rail trails
		switch ( cg_railTrail.integer ) {
			case 1:
				CG_RailTrail( ci, es->origin2, es->pos.trBase );
				break;
			case 2:
				CG_RailTrail2( ci, es->origin2, es->pos.trBase );
				break;
			case 3:
				CG_RailTrail( ci, es->origin2, es->pos.trBase );
				break;
			case 4:
				CG_RailTrail3( ci, es->origin2, es->pos.trBase );
				break;
			default:
				CG_RailTrail( ci, es->origin2, es->pos.trBase );
				break;
		}
		// End

		if ( es->eventParm != 255 ) {
			ByteToDir( es->eventParm, dir );
			CG_MissileHitWall( es, es->weapon, es->clientNum, position, dir );
		}
		break;

	case EV_BULLET_HIT_WALL:
		DEBUGNAME("EV_BULLET_HIT_WALL");
		ByteToDir( es->eventParm, dir );
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD );
		break;

	case EV_BULLET_HIT_FLESH:
		DEBUGNAME("EV_BULLET_HIT_FLESH");
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm );
		break;

	case EV_SHOTGUN:
		DEBUGNAME("EV_SHOTGUN");
		CG_ShotgunFire( es );
		break;

	case EV_GENERAL_SOUND:
		DEBUGNAME("EV_GENERAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_GLOBAL_SOUND:	// play from the player's head so it never diminishes
		DEBUGNAME("EV_GLOBAL_SOUND");

		// This is more professional for freezetag.
		if( CG_isFreezeTag() ) {
			int		num;
			num = atoi( CG_Argv( 1 ) );

			if ( num == CS_SCORES1 ) {
				CG_AddBufferedSound(cgs.media.redScoredSound);
			} else if( num == CS_SCORES2 ) {
				CG_AddBufferedSound(cgs.media.blueScoredSound);
			}
			break;
		}

		 /*Cyrus - 1.32 CTF Sounds - A cool hack, we simply retrieve the sound path string from the server
		 and store its index in one of the flagSoundIndex indexs, do a comparison of the original event, if same
		 use our own media to replace it :P */

		// RED Flag retunred
		if( cgs.gameSounds[ es->eventParm ] == cg.flagSoundIndex[0]) {
			if ( ci->team == TEAM_RED ) {
				CG_AddBufferedSound( cgs.media.returnYourTeamSound );
			} else {
				CG_AddBufferedSound( cgs.media.returnOpponentSound );
			}
			CG_AddBufferedSound( cgs.media.redFlagReturnedSound );
			break;
		}
		// BLUE Flag retunred
		if( cgs.gameSounds[ es->eventParm ] == cg.flagSoundIndex[1] ) {
			if ( ci->team == TEAM_BLUE ) {
				CG_AddBufferedSound( cgs.media.returnYourTeamSound );
			} else {
				CG_AddBufferedSound( cgs.media.returnOpponentSound );
			}
			CG_AddBufferedSound( cgs.media.blueFlagReturnedSound );
			break;
		}
		// RED flag capture
		if( cgs.gameSounds[ es->eventParm ] == cg.flagSoundIndex[2]) {
			if ( ci->team == TEAM_RED ) {
				CG_AddBufferedSound( cgs.media.captureYourTeamSound );
			} else if( ci->team == TEAM_BLUE ) {
				CG_AddBufferedSound( cgs.media.captureOpponentSound );
			}
			CG_AddBufferedSound(cgs.media.redScoredSound);
			break;
		}
		// BLUE flag capture
		if( cgs.gameSounds[ es->eventParm ] == cg.flagSoundIndex[3]) {
			if ( ci->team == TEAM_BLUE ) {
				CG_AddBufferedSound( cgs.media.captureYourTeamSound );
			} else if( ci->team == TEAM_RED ) {
				CG_AddBufferedSound( cgs.media.captureOpponentSound );
			}
			CG_AddBufferedSound(cgs.media.blueScoredSound);
			break;
		}

		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound( es->number, s ) );
		}

		break;

	case EV_PAIN:
		// local player sounds are triggered in CG_CheckLocalSounds,
		// so ignore events on the player
		DEBUGNAME("EV_PAIN");
		if ( cent->currentState.number != cg.snap->ps.clientNum ) {
			CG_PainEvent( cent, es->eventParm );
		}
		// single bubble underwater
		if (CG_PointContents(position, es->number) & CONTENTS_WATER) {
			cg.drowning[es->number] = qtrue;
		}
		break;

	case EV_DEATH1:
	case EV_DEATH2:
	case EV_DEATH3:
		DEBUGNAME("EV_DEATHx");
		// lots of bubbles underwater
		if (CG_PointContents(position, es->number) & CONTENTS_WATER) {
			cg.drowningTime[es->number] = cg.time + 500;
		}
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, 
				CG_CustomSound( es->number, va("*death%i.wav", event - EV_DEATH1 + 1) ) );
		break;


	case EV_OBITUARY:
		DEBUGNAME("EV_OBITUARY");
		CG_Obituary( es );
		if (cg.clientNum == es->otherEntityNum2) {
			int plumValue = 1;

			// killed yourself
			if (es->otherEntityNum == cg.clientNum) {
				plumValue = -1;
			}

			CG_ScorePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, plumValue );
		}
		break;

	//
	// powerup events
	//
	case EV_POWERUP_QUAD:
		DEBUGNAME("EV_POWERUP_QUAD");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_QUAD;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.quadSound );
		break;
	case EV_POWERUP_BATTLESUIT:
		DEBUGNAME("EV_POWERUP_BATTLESUIT");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_BATTLESUIT;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, trap_S_RegisterSound("sound/items/protect3.wav") );
		break;
	case EV_POWERUP_REGEN:
		DEBUGNAME("EV_POWERUP_REGEN");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_REGEN;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, trap_S_RegisterSound("sound/items/regen.wav") );
		break;

	case EV_GIB_PLAYER:
		DEBUGNAME("EV_GIB_PLAYER");
		// ice shatter sound effect
		if ( cg_thawSoundType.integer == 1 && CG_isFreezeTag() ) {
				trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.iceShatter );
			} else {
				trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			}
		CG_GibPlayer( cent->lerpOrigin );
		break;

	case EV_DEBUG_LINE:
		DEBUGNAME("EV_DEBUG_LINE");
		CG_Beam( cent );
		break;

	default:
		DEBUGNAME("UNKNOWN");
		CG_Error( "Unknown event: %i", event );
		break;
	}

}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {
	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) {
		if ( cent->previousEvent ) {
			return;	// already fired
		}
		cent->previousEvent = 1;

		cent->currentState.event = cent->currentState.eType - ET_EVENTS;
	} else {
		// check for events riding with another entity
		if ( cent->currentState.event == cent->previousEvent ) {
			return;
		}
		cent->previousEvent = cent->currentState.event;
		if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) {
			return;
		}
	}

	// calculate the position at exactly the frame time
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
	CG_SetEntitySoundPosition( cent );

	CG_EntityEvent( cent, cent->lerpOrigin );
}

