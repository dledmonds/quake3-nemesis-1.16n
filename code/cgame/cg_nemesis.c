
/* cg_nemesis.c - © Wonkey & Cyrus - www.q3nemesis.com - 2005 */

#include "cg_local.h"

typedef struct {
	char *colorname;
	char *colorValue;
	vec4_t *color;
} colorTable_t;

// OSP Color table
colorTable_t OSP_Colortable[] = {
	{ "red",		"1",		&colorRed },
	{ "green",		"2",		&colorGreen },
	{ "yellow",		"3",		&colorYellow },
	{ "blue",		"4",		&colorBlue },
	{ "cyan",		"5",		&colorCyan },
	{ "magenta",	"6",		&colorMagenta },
	{ "white",		"7",		&colorWhite },	
	{ "orange",		"8",		&colorOrange },	
	{ "black",		"9",		&colorBlack },	
	{ "ltgrey",		"10",		&colorLtGrey },
	{ "mdgrey",		"11",		&colorMdGrey },
	{ "dkgrey",		"12",		&colorDkGrey },	
	{ NULL,			NULL,		NULL }
};

/*
======================
CG_SetColor

Populate float array 'col' with parsed color values from 'colString'
======================
*/
void CG_SetColor(char *colString, float *col, float alpha ) {
	char *s = colString;

	col[0] = 1.0f;
	col[1] = 1.0f;
	col[2] = 1.0f;
	col[3] = (alpha > 1.0f) ? 1.0f : (alpha < 0.0f) ? 0.0f : alpha;

	if(*s == '0' && (*(s+1) == 'x' || *(s+1) == 'X')) {
		s +=2;
		//parse rrggbb
		if(Q_IsHexColorString(s)) {
			col[0] = ((float)(gethex(*(s)) * 16 + gethex(*(s+1)))) / 255.00;
			col[1] = ((float)(gethex(*(s+2)) * 16 + gethex(*(s+3)))) / 255.00;
			col[2] = ((float)(gethex(*(s+4)) * 16 + gethex(*(s+5)))) / 255.00;
			return;
		}
	} else {
		int i = 0;
		while(OSP_Colortable[i].colorname != NULL || 
			OSP_Colortable[i].colorValue != NULL) {
			if(Q_stricmp(s, OSP_Colortable[i].colorname) == 0 || 
				Q_stricmp(s, OSP_Colortable[i].colorValue) == 0) {
				col[0] = (*OSP_Colortable[i].color)[0];
				col[1] = (*OSP_Colortable[i].color)[1];
				col[2] = (*OSP_Colortable[i].color)[2];
				return;
			}
			i++;
		}
	}
}

/*
======================
CG_PlasmaColor

Determines color of plasma ball cells &
wall marks based on team colors and enemy
color.
======================
*/
void CG_PlasmaColor( entityState_t *es, refEntity_t *ent )
{
	clientInfo_t *client;
	clientInfo_t *ourSelf;

	client = &cgs.clientinfo[es->otherEntityNum];
	ourSelf = &cgs.clientinfo[cg.clientNum];

	// Same team(s) check && other teams check with no enemy color defined.
	if( ( client->team == TEAM_RED && ourSelf->team == TEAM_RED ) ||
		( client->team == TEAM_RED && ourSelf->team == TEAM_SPECTATOR ) ||
		( client->team == TEAM_RED && ourSelf->team == TEAM_BLUE && !cg_enemyColor.string[0] ) ) {
			ent->shaderRGBA[0] = 255;
			ent->shaderRGBA[1] = 0;
			ent->shaderRGBA[2] = 0;
			ent->shaderRGBA[3] = 255;
		}
	if( ( client->team == TEAM_BLUE && ourSelf->team == TEAM_BLUE ) ||
		( client->team == TEAM_BLUE && ourSelf->team == TEAM_SPECTATOR ) ||
		( client->team == TEAM_BLUE && ourSelf->team == TEAM_RED && !cg_enemyColor.string[0] ) ) {
			ent->shaderRGBA[0] = 0;
			ent->shaderRGBA[1] = 0;
			ent->shaderRGBA[2] = 255;
			ent->shaderRGBA[3] = 255;
		}
	// Enemy color check
	if( ourSelf->team != TEAM_SPECTATOR && cg_enemyColor.string[0] && client->team != ourSelf->team ) {
		ent->shaderRGBA[0] = 255 * cg.enemyPlasmaColor[0];
		ent->shaderRGBA[1] = 255 * cg.enemyPlasmaColor[1];
		ent->shaderRGBA[2] = 255 * cg.enemyPlasmaColor[2];
		ent->shaderRGBA[3] = 255 * cg.enemyPlasmaColor[3];
	}
}

/*
======================
CG_GetWeaponColor

Selects and returns the correct weapon color based on what team the
firer is on.
======================
*/
float *CG_GetWeaponColor( int clientNum, float *teamColor, float *enemyColor ) {
	clientInfo_t *ci;
	clientInfo_t *ourCi;

	ci = &cgs.clientinfo[ clientNum ];
	ourCi = &cgs.clientinfo[ cg.clientNum ];

	// weapon coloring scheme is simple - if a different team (or team free)
	// they are the enemy.  When in spectator mode or no cg_teamColor is set the
	// team coloring variables in cg default to the team color.
	if ( clientNum != cg.clientNum &&
		 ( ourCi->team != ci->team || ourCi->team == TEAM_FREE ) ) {
		return enemyColor;
	} else {
		return teamColor;
	}
}

/*
======================
CG_SetEnemyColors

Sets the following color variables based on the value passed in.
- enemyHeadColor
- enemyTorsoColor
- enemyLegsColor
- enemyRailColor
- enemyRocketColor
- enemyPlasmaColor
- enemyGrenadeColor

This function parses the following format
"X X X X X X X"
where X can be a number, string or hex value
any X's not provided are assumed to take the value of the first X
======================
*/
void CG_SetEnemyColors( char *colString ) {
	char buffer[MAX_QPATH];
	char headColor[MAX_QPATH];
	char *s1;
	int tokenLength;

	// head
	s1 = buffer;
	tokenLength = Q_nextToken( colString, MOD_DELIMITER, s1 );
	if ( !strlen( s1 ) ) {
		s1 = "2";  // sensible default of green, but should never happen
	}
	strcpy( headColor, s1 );
	CG_SetColor( s1, cg.enemyHeadColor, 1.0f );

	// torso
	colString += tokenLength + 1;
	s1 = buffer;
	tokenLength = Q_nextToken( colString, MOD_DELIMITER, s1 );
	if ( !strlen( s1 ) ) {
		s1 = headColor;
	}
	CG_SetColor( s1, cg.enemyTorsoColor, 1.0f );

	// legs
	colString += tokenLength + 1;
	s1 = buffer;
	tokenLength = Q_nextToken( colString, MOD_DELIMITER, s1 );
	if ( !strlen( s1 ) ) {
		s1 = headColor;
	}
	CG_SetColor( s1, cg.enemyLegsColor, 1.0f );

	// rail
	colString += tokenLength + 1;
	s1 = buffer;
	tokenLength = Q_nextToken( colString, MOD_DELIMITER, s1 );
	if ( !strlen( s1 ) ) {
		s1 = headColor;
	}
	CG_SetColor( s1, cg.enemyRailColor, 1.0f );

	// rocket
	colString += tokenLength + 1;
	s1 = buffer;
	tokenLength = Q_nextToken( colString, MOD_DELIMITER, s1 );
	if ( !strlen( s1 ) ) {
		s1 = headColor;
	}
	CG_SetColor( s1, cg.enemyRocketColor, 1.0f );

	// plasma
	colString += tokenLength + 1;
	s1 = buffer;
	tokenLength = Q_nextToken( colString, MOD_DELIMITER, s1 );
	if ( !strlen( s1 ) ) {
		s1 = headColor;
	}
	CG_SetColor( s1, cg.enemyPlasmaColor, 1.0f );

	// grenade
	colString += tokenLength + 1;
	s1 = buffer;
	tokenLength = Q_nextToken( colString, MOD_DELIMITER, s1 );
	if ( !strlen( s1 ) ) {
		s1 = headColor;
	}
	CG_SetColor( s1, cg.enemyGrenadeColor, 1.0f );
}

/*
======================
CG_SetTeamColors

Sets the following color variables based on the value passed in.
- teamRailColor
- teamRocketColor
- teamPlasmaColor
- teamGrenadeColor

This function parses the following format
"X X X X"
where X can be a number, string or hex value
any X's not provided are assumed to take the value of the first X
======================
*/
void CG_SetTeamColors( char *colString ) {
	char buffer[MAX_QPATH];
	char railColor[MAX_QPATH];
	char *s1;
	int tokenLength;

	// rail
	s1 = buffer;
	tokenLength = Q_nextToken( colString, MOD_DELIMITER, s1 );
	if ( !strlen( s1 ) ) {
		s1 = "2";  // sensible default of green, but should never happen
	}
	strcpy( railColor, s1 );
	CG_SetColor( s1, cg.teamRailColor, 1.0f );

	// rocket
	colString += tokenLength + 1;
	s1 = buffer;
	tokenLength = Q_nextToken( colString, MOD_DELIMITER, s1 );
	if ( !strlen( s1 ) ) {
		s1 = railColor;
	}
	CG_SetColor( s1, cg.teamRocketColor, 1.0f );

	// plasma
	colString += tokenLength + 1;
	s1 = buffer;
	tokenLength = Q_nextToken( colString, MOD_DELIMITER, s1 );
	if ( !strlen( s1 ) ) {
		s1 = railColor;
	}
	CG_SetColor( s1, cg.teamPlasmaColor, 1.0f );

	// grenade
	colString += tokenLength + 1;
	s1 = buffer;
	tokenLength = Q_nextToken( colString, MOD_DELIMITER, s1 );
	if ( !strlen( s1 ) ) {
		s1 = railColor;
	}
	CG_SetColor( s1, cg.teamGrenadeColor, 1.0f );
}

/*
=======================
CG_EnemyColor

Colorizes the entity accordingly
=======================
*/
void CG_EnemyColor( clientInfo_t *ci, refEntity_t *ref, vec4_t colorVec ) {
	clientInfo_t	*ourCi;

	ourCi = &cgs.clientinfo[ cg.clientNum ];

	if ( cg_enemyModel.string[0] == 0 ) {
		return;
	}

	/* Spectators dont get to use enemy colors by default except in non-team games. */
	switch( ci->team ) {
		case TEAM_RED:
			if( ourCi->team == TEAM_SPECTATOR ) {
				ref->shaderRGBA[0] = 255;
				ref->shaderRGBA[1] = 0;
				ref->shaderRGBA[2] = 0;
				ref->shaderRGBA[3] = 255;
			}
			if( ourCi->team == TEAM_BLUE ) {
				ref->shaderRGBA[0] = 255 * colorVec[0];
				ref->shaderRGBA[1] = 255 * colorVec[1];
				ref->shaderRGBA[2] = 255 * colorVec[2];
				ref->shaderRGBA[3] = 255 * colorVec[3];
			}
			break;
		case TEAM_BLUE:
			if( ourCi->team == TEAM_SPECTATOR ) {
				ref->shaderRGBA[0] = 0;
				ref->shaderRGBA[1] = 0;
				ref->shaderRGBA[2] = 255;
				ref->shaderRGBA[3] = 255;
			}
			if( ourCi->team == TEAM_RED ) {
				ref->shaderRGBA[0] = 255 * colorVec[0];
				ref->shaderRGBA[1] = 255 * colorVec[1];
				ref->shaderRGBA[2] = 255 * colorVec[2];
				ref->shaderRGBA[3] = 255 * colorVec[3];
			}
			break;
		case TEAM_FREE:
			ref->shaderRGBA[0] = 255 * colorVec[0];
			ref->shaderRGBA[1] = 255 * colorVec[1];
			ref->shaderRGBA[2] = 255 * colorVec[2];
			ref->shaderRGBA[3] = 255 * colorVec[3];
			break;
	}
}

/*
=================================
CG_QuadColor

8-12-06: Revised due to 
how ugly and messy it was before,
I must have been drunk before!
=================================
*/
void CG_QuadColor( refEntity_t *ent, int team )
{
	clientInfo_t	*ci;

	ci = &cgs.clientinfo[ cg.clientNum ];	// Ourself, team is other(s)

	// Blue Quad( Blue Team/Default )
	if( ( team == TEAM_FREE && !cg_enemyColor.string[0] ) || ( team == TEAM_BLUE && !cg_enemyColor.string[0] ) ||
		( ci->team == TEAM_SPECTATOR && team == TEAM_BLUE ) || ( ci->team == TEAM_SPECTATOR && team == TEAM_FREE ) ||
		( ci->team == TEAM_BLUE && team == TEAM_BLUE ) )
	{
		ent->customShader = cgs.media.quadShader;
		return;
	}
	// Red Quad( Red Team )
	if( team == TEAM_RED && !cg_enemyColor.string[0] || ( ci->team == TEAM_SPECTATOR && team == TEAM_RED ) ||
		( ci->team == TEAM_RED && team == TEAM_RED ) )
	{
		ent->customShader = cgs.media.redQuadShader;
		return;
	}
	// Enemy Quad Color
	if( cg_enemyColor.string[0] )
	{
		if( ( ci->team == TEAM_RED && team == TEAM_BLUE ) || ( ci->team == TEAM_BLUE && team == TEAM_RED ) ||
			( ci->team == TEAM_FREE && team == TEAM_FREE ) )
		{
			ent->shaderRGBA[0] = 255 * cg.enemyLegsColor[0];
			ent->shaderRGBA[1] = 255 * cg.enemyLegsColor[1];
			ent->shaderRGBA[2] = 255 * cg.enemyLegsColor[2];
			ent->shaderRGBA[3] = 255 * cg.enemyLegsColor[3];
			ent->customShader = cgs.media.quadColor;
		}
	}
}

/*
======================
CG_DrawAccuracy

Players accuracy
======================
*/
float CG_DrawAccuracy( float y )
{
	char	*s;
	int		shots, hits;
	int		i, j, w, x;
	
	x = 620;
	
	/* Ugh! - following another player results in accuracy override in freezetag,
	   but how to fix if accuracy soley depends on ps index's, I dont think we can.

	   UPDATE: Yep we cant, unless the mod authors add in these 2 ps index's into
	   their PersistantSpectator function in freezetag. Temp fix for following, just
	   return.  PS data is really starting to piss me off in freezetag since its not
	   persistant.
	*/
	if( (cg.snap->ps.pm_flags & PMF_FOLLOW) ) {
		return y;
	}

	CG_DrawPic( x, y, 16, 16, cgs.media.medalAccuracy );

	shots = cg.snap->ps.persistant[PERS_ACCURACY_SHOTS];
	hits = cg.snap->ps.persistant[PERS_ACCURACY_HITS];
	
	if( shots == 0 ) {
		s = "0.0";
	} else {
		i = ( hits * 100 ) / shots;
		j = ( ( hits * 1000 ) / shots ) - i * 10;
		s = va( "%i.%i", i, j );
	}

	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawStringExt( x - w, y, s, colorWhite, qtrue, qtrue, 16, 16, 0);
	return y;
}


/*
=========================
CG_WeaponAccCheck
==========================
*/
void CG_WeaponAccCheck( void )
{
	if( stats.lastWeaponHit != cg.snap->ps.persistant[PERS_ACCURACY_HITS] )
	{
		stats.weaponHits[cg.snap->ps.weapon] += cg.snap->ps.persistant[PERS_ACCURACY_HITS] - stats.lastWeaponHit;
	}
	
	stats.lastWeaponHit = cg.snap->ps.persistant[PERS_ACCURACY_HITS];
	
	if( stats.lastWeaponShot != cg.snap->ps.persistant[PERS_ACCURACY_SHOTS] )
	{
		stats.weaponShots[cg.snap->ps.weapon] += cg.snap->ps.persistant[PERS_ACCURACY_SHOTS] - stats.lastWeaponShot;
	}
	
	stats.lastWeaponShot = cg.snap->ps.persistant[PERS_ACCURACY_SHOTS];
}


/*
=====================================
CG_ProModeHUD

Close replication of CPMA HUD, not a ripoff, all imitated :)
=====================================
*/
void CG_ProModeHUD( void )
{
	centity_t		*cent;
	playerState_t	*ps;
	float			*fadeColor;
	char			s[1024];//, *name;
	int				i, x = 5, y = 60;
	int				value, value2, color;
	qhandle_t		icon;
	vec3_t			angles, origin;
	vec4_t			hcolor;
	static float	colors[4][4] = { 
		{ 1.0f, 0.69f, 0.0f, 1.0f },    // normal
		{ 1.0f, 0.2f, 0.2f, 1.0f },     // low health
		{ 0.5f, 0.5f, 0.5f, 1.0f },     // weapon firing
		{ 1.0f, 1.0f, 1.0f, 1.0f }		// health > 100
	};
	static float	bar_colors[3][4] = { 
		{ 1.0f, 0.69f, 0.0f, 0.5f },    // Health 25 - 100
		{ 1.0f, 0.2f, 0.2f, 0.5f },	    // Health < 25
		{ 1.0f, 1.0f, 1.0f, 0.5f }	    // Armor & Health > 100
	}; 

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	// For health model icon
	VectorClear( angles );
	origin[0] = 70;
	origin[1] = 0;
	origin[2] = 0;
	angles[YAW] = 160 * sin( cg.time / 1000.0 );

	// Upper left ammo window
	for( i = WP_GAUNTLET; i < WP_NUM_WEAPONS - 1; i++ ) {
		if( ps->stats[STAT_WEAPONS] & ( 1 << i ) ) {
			value = ps->ammo[i];
			icon = cg_weapons[i].weaponIcon;
			
			if ( cg.showScores ) {
				continue;
			}

			// Prevent printing more than 3 digits aka Noghost instagib
			if( value > 999 ) {
				value = 999; // or maybe something like value = itoa("A shit load of ammo!").... :P
			}

			Com_sprintf( s, sizeof( s ), "%i", value );

			if( i == 1 ) {
				y += 20;
			} else if( ps->weapon == i ) {
					fadeColor = colorWhite;
					// Were running low on ammo!
					if ( value <= 5 ) {
						fadeColor = colorRed;
					}
					CG_DrawStringExt( x + 25, y += 20, s, fadeColor, qfalse, qtrue, 16, 16, 0 );
				} else {
					fadeColor = colorMdGrey;
					CG_DrawStringExt( x + 25, y += 20, s, fadeColor, qfalse, qtrue, 10, 12, 0 );
				}

			CG_FillRect( 0, 73, 75, 1, colorWhite );
			CG_DrawPic( x, y, 16, 16, icon );

			// Weapon is out of ammo, draw a cross out
			if ( value == 0 ) {
				CG_DrawPic( x, y, 16, 16, cgs.media.noammoShader);
			}
		}
	}

	if ( !cg.showScores ) {
		CG_FillRect( 0, y + 23, 75, 1, colorWhite );
		// Team icons
		if ( cg_useTeamIcons.integer ) {
			if ( ps->persistant[PERS_TEAM] == TEAM_BLUE ) {
				CG_DrawPic( x, y + 30, 50, 50, cgs.media.teamIconBlue );
			} else if ( ps->persistant[PERS_TEAM] == TEAM_RED ) {
				CG_DrawPic( x, y + 30, 50, 50, cgs.media.teamIconRed);
			}
		}
	}

	// Health
	value = ps->stats[STAT_HEALTH];
	value2 = ps->stats[STAT_HEALTH];

	if ( value > 100 ) {
		trap_R_SetColor( colors[3] );
	} else if (value > 25) {
		trap_R_SetColor( colors[0] );
	} else if (value > 0) {
		color = (cg.time >> 8) & 1;
		trap_R_SetColor( colors[color] );
	} else {
		trap_R_SetColor( colors[1] );
	}

	// stretch the health up when taking damage
	CG_DrawField( 212, 442, 3, 22, 28, value, qfalse );
	CG_ColorForHealth( hcolor );
	trap_R_SetColor( hcolor );

	// Health Bar Gauges
	do { 
		if ( value > ps->stats[STAT_MAX_HEALTH] ) {
			value = ps->stats[STAT_MAX_HEALTH];
		}
		if ( value <= 25 ) {
			color = (cg.time >> 8) & 1;
			CG_DrawWidthGauge( 60, 440, 125, 13, bar_colors[color], value, qtrue);
		} else  {
			CG_DrawWidthGauge( 60, 440, 125, 13, bar_colors[0], value, qtrue);
		}
	} while ( value > ps->stats[STAT_MAX_HEALTH]);

	if ( value2 > ps->stats[STAT_MAX_HEALTH] ) {
		CG_DrawWidthGauge( 60, 460, 125, 13, bar_colors[2], value2 - ps->stats[STAT_MAX_HEALTH], qtrue);
	}

	CG_FillRect( 60, 456, 125, 1, colorWhite );
	CG_FillRect( 260, 440, 1, 35, colorWhite );
	CG_Draw3DModel( 15, 445, 40, 40, cgs.media.healthModel, 0, origin, angles );
	CG_Draw3DModel( 15, 445, 40, 40, cgs.media.healthSphere, 0, origin, angles );

	// Armor
	origin[0] = 90;
	origin[1] = 0;
	origin[2] = -10;
	angles[YAW] = ( cg.time & 2047 ) * 360 / 2048.0;
	value = ps->stats[STAT_ARMOR];
	value2 = ps->stats[STAT_ARMOR];

	CG_DrawField( 452, 442, 3, 22, 28, value, qfalse );

	// Armor Bar Gauges
	do { 
		if ( value > ps->stats[STAT_MAX_HEALTH] ) {
			value = ps->stats[STAT_MAX_HEALTH];
		}
		CG_DrawWidthGauge( 300, 440, 125, 13, bar_colors[0], value, qtrue);
	} while ( value > ps->stats[STAT_MAX_HEALTH]);

	if ( value2 > ps->stats[STAT_MAX_HEALTH] ) {
		CG_DrawWidthGauge( 300, 460, 125, 13, bar_colors[2], value2 - ps->stats[STAT_MAX_HEALTH], qtrue);
	}

	CG_FillRect( 300, 456, 125, 1, colorWhite );
	CG_FillRect( 500, 440, 1, 35, colorWhite );	
	CG_Draw3DModel( 265, 445, 30, 30, cgs.media.armorModel, 0, origin, angles );
}

/*
=================
CG_DrawProModeScores

Draw the small two score display.
For use with CPMA Hud, this function is 
actually pretty crappy and it was done by ID...
=================
*/
float CG_DrawProModeScores( float y ) {
	const char	*s;
	int			s1, s2, score;
	int			x, w;
	int			v;
	vec4_t		color;
	float		y1;
	gitem_t		*item;

	s1 = cgs.scores1;
	s2 = cgs.scores2;

	y -=  BIGCHAR_HEIGHT + 8;

	y1 = y;

	// draw from the right side to left
	if ( cgs.gametype >= GT_TEAM ) {
		x = 640;

		color[0] = 0;
		color[1] = 0;
		color[2] = 1;
		color[3] = 0.33f;
		s = va( "%2i", s2 );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect( x, y - 20,  w, BIGCHAR_HEIGHT+8, color );
		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
			CG_DrawPic( x, y - 20, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
		}
		CG_DrawBigString( x + 6, y - 16, s, 1.0F);

		if ( cgs.gametype == GT_CTF ) {
			// Display flag status
			item = BG_FindItemForPowerup( PW_BLUEFLAG );

			if (item) {
				y1 = y - BIGCHAR_HEIGHT - 8;
				if( cgs.blueflag >= 0 && cgs.blueflag <= 2 ) {
					CG_DrawPic( x - 34, y1 + 4, w - 6, BIGCHAR_HEIGHT+8, cgs.media.blueFlagShader[cgs.blueflag] );
				}
			}
		}

		color[0] = 1;
		color[1] = 0;
		color[2] = 0;
		color[3] = 0.33f;
		s = va( "%2i", s1 );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect( x + 40, y + 4,  w, BIGCHAR_HEIGHT+8, color );
		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
			CG_DrawPic( x + 40, y + 4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
		}
		CG_DrawBigString( x + 46, y + 8, s, 1.0F);

		if ( cgs.gametype == GT_CTF ) {
			// Display flag status
			item = BG_FindItemForPowerup( PW_REDFLAG );

			if (item) {
				y1 = y - BIGCHAR_HEIGHT - 8;
				if( cgs.redflag >= 0 && cgs.redflag <= 2 ) {
					CG_DrawPic( x + 6, y1 + 28, w - 6, BIGCHAR_HEIGHT+8, cgs.media.redFlagShader[cgs.redflag] );
				}
			}
		}


		// Nemesis - Freeze
		if ( cgs.gametype == GT_CTF ) {
			v = cgs.capturelimit;
		} else if ( cgs.gametype == GT_FREEZE ) {
			v = cgs.freezelimit;
		} else {
			v = cgs.fraglimit;
		}
		// End
		if ( v ) {
			s = va( "%2i", v );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_DrawBigString( x + 86, y - 40, s, 1.0F);
		}

	} else {
		qboolean	spectator;

		x = 640;
		score = cg.snap->ps.persistant[PERS_SCORE];
		spectator = ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR );

		// always show your score in the second box if not in first place
		if ( s1 != score ) {
			s2 = score;
		}
		if ( s2 != SCORE_NOT_PRESENT ) {
			s = va( "%2i", s2 );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			if ( !spectator && score == s2 && score != s1 ) {
				color[0] = 1;
				color[1] = 0;
				color[2] = 0;
				color[3] = 0.33f;
				CG_FillRect( x, y+4,  w, BIGCHAR_HEIGHT+8, color );
				CG_DrawPic( x, y+4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
			} else {
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect( x, y+4,  w, BIGCHAR_HEIGHT+8, color );
			}	
			CG_DrawBigString( x + 8, y+8, s, 1.0F);
		}

		// first place
		if ( s1 != SCORE_NOT_PRESENT ) {
			s = va( "%2i", s1 );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			if ( !spectator && score == s1 ) {
				color[0] = 0;
				color[1] = 0;
				color[2] = 1;
				color[3] = 0.33f;
				CG_FillRect( x + 40, y-20,  w, BIGCHAR_HEIGHT+8, color );
				CG_DrawPic( x + 40, y-20, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
			} else {
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect( x + 40, y-20,  w, BIGCHAR_HEIGHT+8, color );
			}	
			CG_DrawBigString( x + 46, y-16, s, 1.0F);
		}

		if ( cgs.fraglimit ) {
			s = va( "%2i", cgs.fraglimit );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_DrawBigString( x + 86, y-40, s, 1.0F);
		}

	}

	return y1 - 8;
}

char *CG_OpenSSfile( char *filename )
{
	fileHandle_t	f;
	static char	line[256];
	static char ssNum[MAX_QPATH], ssName[MAX_QPATH];
	char	*p, *token, num[6], *map;
	const char *info;
	int		i = 0, cnt = 0, newNum;

	info = CG_ConfigString( CS_SERVERINFO );
	map = Info_ValueForKey( info, "mapname" );

	memset( &line, 0, sizeof(line));

	// Open ss_info.txt
	trap_FS_FOpenFile( filename, &f, FS_READ);

	// If the file doesnt exist, create it
	if( !f ) {
		strcpy( ssName, va("%s/%s/ss_0", cgs.modName, map) );
		trap_FS_FOpenFile( filename, &f, FS_WRITE);
		strcpy( line, "//Generated by Nemesis for screenshot naming, do not modify or delete this file!\n\nss_0\n" );
		trap_FS_Write(line, strlen( line ), f);
		trap_FS_FCloseFile(f);
		return ssName;
	}
	if( f <= 0 ) {
		CG_Printf( "Could not open %s\n", filename );
		trap_FS_FCloseFile(f);
		return "";
	}

	trap_FS_Read( line, sizeof(line), f );
	trap_FS_FCloseFile(f);

	p = line;
	token = COM_ParseExt( &p, qtrue );
	
	if( !token[0] ) {
		return "";
	}

	while( *(token + i ) != '_' ) {
		i++;
	}

	i++;
	while( token[i] >= '0' && token[i] <= '9' ) {
		num[cnt] = token[i];
		i++;
		cnt++;
	}
	newNum = atoi(num);
	newNum++;
	
	trap_FS_FOpenFile( filename, &f, FS_WRITE);
	memset( &line, 0, sizeof(line));
	memset( &ssName, 0, sizeof(ssName));

	strcpy( line, "//Generated by Nemesis for screenshot naming, do not modify or delete this file!\n\n" );	
	strcpy( ssNum, va("ss_%i\n", newNum ) );
	strcat( line, ssNum );
	strcpy( ssName, va("%s/%s/%s", cgs.modName, map, ssNum) );

	trap_FS_Write(line, strlen( line ), f);
	trap_FS_FCloseFile(f);

	return ssName;
}
