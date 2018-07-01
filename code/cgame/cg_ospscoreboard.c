// cg_ospscoreboard -- draw osp scoreboard on top of the game screen
#include "cg_local.h"

// screen is 640 x 480
#define OSP_SB_TOP				50			// vertical distance to start of scoreboard
#define OSP_SB_HEADER			100			// vertical distance to start of player lines (header end)

#define OSP_SB_RED_XMIN			10			// LHS limit for red team area
#define OSP_SB_RED_XMAX			310			// RHS limit for red team area
#define OSP_SB_BLUE_XMIN		330			// LHS limit for blue team area
#define OSP_SB_BLUE_XMAX		630			// RHS limit for blue team area
#define OSP_SB_FREE_XMIN		170			// LHS limit for free team area
#define OSP_SB_FREE_XMAX		470			// RHS limit for free team area
#define OSP_SB_SPEC_XMIN		10			// LHS limit for spectator area
#define OSP_SB_SPEC_XMAX		630			// RHS limit for spectator area

#define OSP_SB_TSCORE_OFFSET	0			// distance into team section to draw team score
#define OSP_SB_TSTAT_OFFSET		175			// distance into team section to draw info
#define OSP_SB_TSTAT_INSET		10			// distance from OSP_SB_TOP to draw info

#define OSP_SB_TCHAR_WIDTH		8			// width of characters used for team info 
#define OSP_SB_TCHAR_HEIGHT		12			// height of characters used for team info 

#define OSP_SB_FLAG_OFFSET		5			// distance into player line to draw flag
#define OSP_SB_MODEL_OFFSET		10			// distance into player line to draw head model
#define OSP_SB_SCORE_OFFSET		45			// distance into player line to draw score
#define OSP_SB_PING_OFFSET		80			// distance into player line to draw ping
#define OSP_SB_TIME_OFFSET		115			// distance into player line to draw time
#define OSP_SB_NAME_OFFSET		150			// distance into player line to draw name

#define OSP_SB_CHAR_WIDTH		10			// width of characters used for player 
#define OSP_SB_CHAR_HEIGHT		12			// height of characters used for player 
#define OSP_SB_NAME_MAX_CHARS	15			// maximum characters to display for player name
#define	OSP_SB_FFA				35

#define OSP_SB_LINE_HEIGHT		20  // height of player lines
#define OSP_SB_LINE_INSET		(int)((OSP_SB_LINE_HEIGHT - OSP_SB_CHAR_HEIGHT) / 2)  // ensures centered names


/*
=============
CG_DrawLabels

Draws labels for score, ping, time and name
=============
*/
void CG_DrawLabels( int x, float *color ) {
	CG_DrawStringExt( x + 35, OSP_SB_HEADER - 10, "Score", color, qtrue, qtrue, 8, 10, 0 );
	CG_DrawStringExt( x + 80, OSP_SB_HEADER - 10, "Ping", color, qtrue, qtrue, 8, 10, 0 );
	CG_DrawStringExt( x + 120, OSP_SB_HEADER - 10, "Min", color, qtrue, qtrue, 8, 10, 0 );
	CG_DrawStringExt( x + 150, OSP_SB_HEADER - 10, "Name", color, qtrue, qtrue, 8, 10, 0 );
}


/*
=================
CG_DrawOSPClientScore

Draws player score/ping/name starting at given coordinates
=================
*/
void CG_DrawOSPClientScore( int x, int y, clientInfo_t *ci, score_t *score ) {
	char string[1024];
	float color[4];

	color[0] = color[1] = color[2] = 1.0f;
	color[3] = 1.0f;

	// just in case?
	if ( score->client < 0 || score->client >= cgs.maxclients ) {
		Com_Printf( "Bad score->client: %i\n", score->client );
		return;
	}

	// flag or bot skill
	if ( ci->powerups & ( 1 << PW_REDFLAG ) && ci->team == TEAM_RED ) {
		CG_DrawFlagModel( x + OSP_SB_FLAG_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_RED );

	} else if ( ci->powerups & ( 1 << PW_BLUEFLAG ) && ci->team == TEAM_BLUE ) {
		CG_DrawFlagModel( x + OSP_SB_FLAG_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_BLUE );

/*	} else if ( ci->botSkill > 0 && ci->botSkill <= 5 && cg_drawIcons.integer ) {
		CG_DrawPic( x + OSP_SB_FLAG_OFFSET, y, 
					OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, cgs.media.botSkillShaders[ ci->botSkill - 1 ] );*/
	}

	// 2nd flag (rtf) or headmodel
	if ( ci->powerups & ( 1 << PW_REDFLAG ) && ci->team == TEAM_BLUE ) {
		CG_DrawFlagModel( x + OSP_SB_FLAG_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_RED );

	} else if ( ci->powerups & ( 1 << PW_BLUEFLAG ) && ci->team == TEAM_RED ) {
		CG_DrawFlagModel( x + OSP_SB_FLAG_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_BLUE );

	} else {
		vec3_t headAngles;
		VectorClear( headAngles );
		headAngles[YAW] = 180;
		CG_DrawHead( x + OSP_SB_MODEL_OFFSET, y,
			OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, score->client, headAngles );
	}

	// score/ping/time
	if ( score->ping == -1 ) {
		Com_sprintf( string, sizeof(string), "connecting" );
		CG_DrawStringExt( x + OSP_SB_SCORE_OFFSET, y, string, color, 
					      qfalse, qtrue,  
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

	} else {
		Com_sprintf( string, sizeof(string), "%i", score->score );
		CG_DrawStringExt( x + OSP_SB_SCORE_OFFSET - 10, y, string, colorYellow, 
					      qfalse, qtrue,
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

		Com_sprintf( string, sizeof(string), "%i", score->ping );
		CG_DrawStringExt( x + OSP_SB_PING_OFFSET, y, string, color, 
					      qfalse, qtrue,
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

		Com_sprintf( string, sizeof(string), "%i", score->time );
		CG_DrawStringExt( x + OSP_SB_TIME_OFFSET + 5, y, string, color, 
					      qfalse, qtrue,
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );
	}

	// name
	Com_sprintf( string, sizeof(string), "%s", ci->name );
	// In ffa & 1v1 allow max name (36)
	if ( cgs.gametype < GT_TEAM ) {
		CG_DrawStringExt( x + OSP_SB_NAME_OFFSET, y, string, color, 
				      qfalse, qtrue,
					  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, OSP_SB_NAME_MAX_CHARS + 21 );
	} else {
		CG_DrawStringExt( x + OSP_SB_NAME_OFFSET, y, string, color, 
				      qfalse, qtrue,
					  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, OSP_SB_NAME_MAX_CHARS );
	}


	// highlight players in the ready state
	if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << score->client ) ) {
		// Lets change "READY" to "FROZEN" for freeze mode
		if ( CG_isFreezeTag() && cg.intermissionStarted == qfalse && cg.warmup == qfalse ) {
			CG_DrawStringExt( x, y + 2, "FROZE", colorYellow, 
						  qfalse, qfalse, 
						  OSP_SB_CHAR_WIDTH - 4, OSP_SB_CHAR_HEIGHT - 4, 0 );
		} else {
			CG_DrawStringExt( x + OSP_SB_FLAG_OFFSET, y + 2, "READY", colorYellow, 
						  qfalse, qfalse, 
						  OSP_SB_CHAR_WIDTH - 4, OSP_SB_CHAR_HEIGHT - 4, 0 );
		}
	}
}


/*
=================
CG_DrawOSPTeamScoreboard

Draw the osp team scoreboard
=================
*/
void CG_DrawOSPTeamScoreboard( void ) {
	int i, tmp;
	int redTotal, blueTotal, specTotal, highTotal;
	int redPingTotal, bluePingTotal;
	int x, y;
	int redCount, blueCount, specCount;
	char *string;
	score_t	*score;
	clientInfo_t *ci;

	redTotal = blueTotal = specTotal = 0;
	redPingTotal = bluePingTotal = 0;

	for ( i = 0 ; i < cg.numScores ; i++ ) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[ score->client ];

		if ( ci->team == TEAM_RED ) {
			redTotal++;
			redPingTotal += score->ping;

		} else if ( ci->team == TEAM_BLUE ) {
			blueTotal++;
			bluePingTotal += score->ping;

		} else {
			specTotal++;
		}
	}

	if ( redTotal >= blueTotal ) {
		highTotal = redTotal;
	} else {
		highTotal = blueTotal;
	}

	// red header
	CG_DrawPic( 86, OSP_SB_TOP - 14, 225, 15, cgs.media.scoreBarRed );

	CG_DrawTeamBackground( OSP_SB_RED_XMIN, OSP_SB_TOP, 
						   OSP_SB_RED_XMAX - OSP_SB_RED_XMIN,
						   OSP_SB_HEADER - OSP_SB_TOP,
						   0.25f,
						   TEAM_RED );

	if( cg_useTeamIcons.integer ) {
		CG_DrawPic( OSP_SB_RED_XMAX - 50, OSP_SB_TOP, 50, 50, cgs.media.teamIconRed);
	}

	// red players
	CG_DrawTeamBackground( OSP_SB_RED_XMIN, OSP_SB_HEADER, 
						   OSP_SB_RED_XMAX - OSP_SB_RED_XMIN,
						   redTotal * OSP_SB_LINE_HEIGHT,
						   0.15f,
						   TEAM_RED );

	CG_DrawLabels( OSP_SB_RED_XMIN, colorRed );


	// blue header
	CG_DrawPic( 406, OSP_SB_TOP - 14, 225, 15, cgs.media.scoreBarBlue );

	CG_DrawTeamBackground( OSP_SB_BLUE_XMIN, OSP_SB_TOP, 
						   OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
						   OSP_SB_HEADER - OSP_SB_TOP,
						   0.25f,
						   TEAM_BLUE );

	if( cg_useTeamIcons.integer ) {
		CG_DrawPic( OSP_SB_BLUE_XMAX - 50, OSP_SB_TOP, 50, 50, cgs.media.teamIconBlue);
	}
	CG_DrawLabels( OSP_SB_BLUE_XMIN, colorBlue );

	// blue players
	CG_DrawTeamBackground( OSP_SB_BLUE_XMIN, OSP_SB_HEADER, 
						   OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
						   blueTotal * OSP_SB_LINE_HEIGHT,
						   0.15f,
						   TEAM_BLUE );
	

	// spectators
	if ( specTotal > 0 ) {
		specTotal++;
		// added spectator string since its in OSP
		CG_DrawStringExt( OSP_SB_SPEC_XMIN + 256, OSP_SB_HEADER + ( highTotal * OSP_SB_LINE_HEIGHT ) + 8,
			"Spectator", colorYellow, qtrue, qtrue, 12, 12, 0 );

		CG_DrawTeamBackground( OSP_SB_SPEC_XMIN, OSP_SB_HEADER + ( highTotal * OSP_SB_LINE_HEIGHT ) + 20, 
							   OSP_SB_SPEC_XMAX - OSP_SB_SPEC_XMIN,
							   (int)(specTotal / 2) * OSP_SB_LINE_HEIGHT,
							   0.20f,
							   TEAM_SPECTATOR );
	}


	// team score
	CG_DrawField( OSP_SB_RED_XMIN + 40, OSP_SB_TOP + 2, 3, 30, 36, cg.teamScores[ 0 ], qfalse );
	CG_DrawField( OSP_SB_BLUE_XMIN + 40, OSP_SB_TOP + 2, 3, 30, 36, cg.teamScores[ 1 ], qfalse );

	// work out appropiate size of labels
	tmp = CG_DrawStrlen( "Av. Ping" ) * OSP_SB_TCHAR_WIDTH + 10;  // "Av. Ping" is longest string

	// player count
	CG_DrawStringExt( OSP_SB_RED_XMIN + OSP_SB_TSTAT_OFFSET - tmp, 
					  OSP_SB_TOP - 10,// + OSP_SB_TSTAT_INSET,
					  "PLAYERS", colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	string = va( "%i", redTotal );
	CG_DrawStringExt( OSP_SB_TSTAT_OFFSET, 
					  OSP_SB_TOP - 10,// + OSP_SB_TSTAT_INSET,
					  string, colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );

	CG_DrawStringExt( OSP_SB_BLUE_XMIN + OSP_SB_TSTAT_OFFSET - tmp, 
					  OSP_SB_TOP - 10, // + OSP_SB_TSTAT_INSET,
					  "PLAYERS", colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	string = va( "%i", blueTotal );
	CG_DrawStringExt( OSP_SB_BLUE_XMIN + OSP_SB_TSTAT_OFFSET - 10, OSP_SB_TOP - 10,// + OSP_SB_TSTAT_INSET,
					  string, colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );

	// average pings
	CG_DrawStringExt( OSP_SB_RED_XMIN + OSP_SB_TSTAT_OFFSET + 100 - tmp, 
					  OSP_SB_TOP - 10, 
					  "AV.PING", colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	if ( redTotal > 0 ) {
		string = va( "%i", (int)( redPingTotal / redTotal) );
		CG_DrawStringExt( OSP_SB_TSTAT_OFFSET + 100, 
						  OSP_SB_TOP - 10,
						  string, colorWhite, 
						  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	}

	CG_DrawStringExt( OSP_SB_BLUE_XMIN + OSP_SB_TSTAT_OFFSET + 100 - tmp, 
					  OSP_SB_TOP - 10,
					  "AV.PING", colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	if ( blueTotal > 0 ) {
		string = va( "%i", (int)(bluePingTotal / blueTotal ) );
		CG_DrawStringExt( OSP_SB_BLUE_XMIN + OSP_SB_TSTAT_OFFSET + 90, 
						  OSP_SB_TOP - 10,
						  string, colorWhite, 
						  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	}

	// player scores
	redCount = 0;
	blueCount = 0;
	specCount = 0;
	for ( i = 0 ; i < cg.numScores ; i++ ) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[ score->client ];

		if ( ci->team == TEAM_RED ) {
			x = OSP_SB_RED_XMIN;
			y = OSP_SB_HEADER + ( redCount++ * OSP_SB_LINE_HEIGHT ) + OSP_SB_LINE_INSET;

		} else if ( ci->team == TEAM_BLUE ) {
			x = OSP_SB_BLUE_XMIN;
			y = OSP_SB_HEADER + ( blueCount++ * OSP_SB_LINE_HEIGHT ) + OSP_SB_LINE_INSET;

		} else {
			tmp = (int)( ( specCount++ ) / 2 );

			if ( (specCount % 2) == 0 ) {
				x = OSP_SB_BLUE_XMIN;
				y = OSP_SB_HEADER + ( (highTotal + tmp ) * OSP_SB_LINE_HEIGHT + 20 ) + OSP_SB_LINE_INSET;

			} else {
				x = OSP_SB_RED_XMIN;
				y = OSP_SB_HEADER + ( (highTotal + tmp ) * OSP_SB_LINE_HEIGHT + 20 ) + OSP_SB_LINE_INSET;
			}
		}

		CG_DrawOSPClientScore( x, y, ci, score );
	}
}


/*
=================
CG_DrawOSPFFAScoreboard

Draw the osp free for all scoreboard
=================
*/
void CG_DrawOSPFFAScoreboard( void ) {
	int i, tmp;
	int x, y;
	int playerTotal, specTotal;
	int playerCount, specCount;
	score_t	*score;
	clientInfo_t *ci;

	playerTotal = specTotal = 0;

	for ( i = 0 ; i < cg.numScores ; i++ ) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[ score->client ];

		if ( ci->team == TEAM_FREE ) {
			playerTotal++;
		} else {
			specTotal++;
		}
	}

	// This looks really bad.
/*	CG_DrawTeamBackground( OSP_SB_FREE_XMIN, OSP_SB_TOP, 
						   OSP_SB_FREE_XMAX - OSP_SB_FREE_XMIN,
						   OSP_SB_HEADER - OSP_SB_TOP,
						   0.33f,
						   TEAM_FREE );

	CG_DrawTeamBackground( OSP_SB_FREE_XMIN, OSP_SB_HEADER, 
						   OSP_SB_FREE_XMAX - OSP_SB_BLUE_XMIN,
						   playerTotal * OSP_SB_LINE_HEIGHT,
						   0.20f,
						   TEAM_FREE );

	if ( specTotal > 0 ) {
		specTotal++;
		CG_DrawTeamBackground( OSP_SB_SPEC_XMIN, OSP_SB_HEADER + ( playerTotal * OSP_SB_LINE_HEIGHT ) + 10, 
							   OSP_SB_SPEC_XMAX - OSP_SB_SPEC_XMIN,
							   (int)(specTotal / 2) * OSP_SB_LINE_HEIGHT,
							   0.33f,
							   TEAM_SPECTATOR );
	}
*/
	CG_DrawLabels( OSP_SB_FREE_XMIN + OSP_SB_FFA, colorWhite );

	// player scores
	playerCount = specCount = 0;
	for ( i = 0 ; i < cg.numScores ; i++ ) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[ score->client ];

		if ( ci->team == TEAM_FREE ) {
			x = OSP_SB_FREE_XMIN;
			y = OSP_SB_HEADER + ( playerCount++ * OSP_SB_LINE_HEIGHT ) + OSP_SB_LINE_INSET;

		} else {
			tmp = (int)( ( specCount++ ) / 2 );

			if ( (specCount % 2) == 0 ) {
				x = OSP_SB_BLUE_XMIN;
				y = OSP_SB_HEADER + ( (playerTotal + tmp ) * OSP_SB_LINE_HEIGHT + 10 ) + OSP_SB_LINE_INSET;

			} else {
				x = OSP_SB_RED_XMIN;
				y = OSP_SB_HEADER + ( (playerTotal + tmp ) * OSP_SB_LINE_HEIGHT + 10 ) + OSP_SB_LINE_INSET;
			}
		}

		CG_DrawOSPClientScore( x + OSP_SB_FFA, y, ci, score );
	}
}


/*
=================
CG_DrawOSPScoreboard

Draw the osp in-game scoreboard
=================
*/
qboolean CG_DrawOSPScoreboard( void ) {
	int		x, y, w;
	float	fade;
	float	*fadeColor;
	char	*s;

	// don't draw anything if the menu or console is up
	if ( cg_paused.integer ) {
		cg.deferredPlayerLoading = 0;
		return qfalse;
	}

	if ( cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		cg.deferredPlayerLoading = 0;
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	if ( cg.warmup && !cg.showScores ) {
		return qfalse;
	}

	if ( cg.showScores || 
		 cg.predictedPlayerState.pm_type == PM_DEAD ||
		 cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		fade = 1.0;
		fadeColor = colorWhite;
	} else {
		fadeColor = CG_FadeColor( cg.scoreFadeTime, FADE_TIME );
		
		if ( !fadeColor ) {
			// next time scoreboard comes up, don't print killer
			cg.deferredPlayerLoading = 0;
			cg.killerName[0] = 0;
			return qfalse;
		}
		fade = *fadeColor;
	}

	// print killer line if not 1v1
	if (cg.killerName[0] && cgs.gametype != GT_TOURNAMENT) {
		s = va("Fragged by %s", cg.killerName );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		x = ( SCREEN_WIDTH - w ) / 2;
		y = 40;
		CG_DrawBigString( x, 40, s, fade );
	}

	// draw gfx icons
	// CG_DrawPic( SB_SCORE_X + (SB_RATING_WIDTH / 2), y, 64, 32, cgs.media.scoreboardScore );

	if ( cgs.gametype >= GT_TEAM ) {
		CG_DrawOSPTeamScoreboard();
	} else if ( cgs.gametype == GT_TOURNAMENT ) {
		CG_DrawOSPTourneyScoreboard();
	} else {
		CG_DrawNormalScoreboard();
	}

	// load any models that have been deferred
	if ( ++cg.deferredPlayerLoading > 10 ) {
		CG_LoadDeferredPlayers();
	}

	return qtrue;
}


/*
=================
CG_DrawOSPTourneyPlayerScore

Draws player name, head, score, ping, time, wins/losses
=================
*/
void CG_DrawOSPTourneyPlayerScore(clientInfo_t *ci, score_t *score, qboolean isLeft, score_t *oppScore) {
	int x, y;
	int tmp, headSize;
	int split;  // what to divide cg.time by to give time intervals
	int loopSpeed;  // animation (head turn) loop speed in seconds
	int frames;  // total number of frames in the sequence
	int midPoint;  // frame at which animation switch from left->right to right->left
	vec3_t headAngles;
	char nameString[MAX_STRING_TOKENS];

	// head - size and movement speed is based on difference in scores
	if (oppScore == NULL) {
		headSize = 32;
		loopSpeed = 16;
	} else {
		int scoreDiff = score->score - oppScore->score;
		if (scoreDiff >= 4) {
			headSize = 48;
			loopSpeed = 8;
		} else if (scoreDiff >= 2) {
			headSize = 40;
			loopSpeed = 12;
		} else if (scoreDiff >= -1) {
			headSize = 32;
			loopSpeed = 16;
		} else if (scoreDiff >= -3) {
			headSize = 24;
			loopSpeed = 20;
		} else {
			headSize = 16;
			loopSpeed = 24;
		}
	}

	if (isLeft) {
		x = OSP_SB_RED_XMIN + 26 - (headSize/2);
		y = OSP_SB_TOP + (48 - headSize)/2;
	} else {
		x = OSP_SB_BLUE_XMAX - 26 - (headSize/2);
		y = OSP_SB_TOP + (48 - headSize)/2;
	}
	VectorClear( headAngles );

	split = 10;
	frames = (loopSpeed * 1000) / split;
	midPoint = (int)(frames / 2);

	tmp = (int)(cg.time / split);  // how many time intervals
	tmp = (tmp % frames) + 1;  // which frame we are in (1 to frames)

	if (tmp > midPoint) {
		// moving right to left
		headAngles[YAW] = 270 - (int)(180.0f/midPoint * (tmp-midPoint));
	} else {
		// moving left to right
		headAngles[YAW] = 90 + (int)(180.0f/midPoint * tmp);
	}

	CG_DrawHead(x, y, headSize, headSize, score->client, headAngles);

	if (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << score->client)) {
		CG_DrawStringExt(x, y + (int)(headSize/2) - 4, "READY", colorYellow, qfalse, qfalse, 8, 8, 0);
	}

	// name
	if (isLeft) {
		x = OSP_SB_RED_XMIN + 50 + 2;
		y = OSP_SB_TOP + 2;
		Q_strncpyz(nameString, ci->name, sizeof(nameString));
	} else {
		// FIXME: we should right align this name, but it is a pain working out the printable
		// characters as ^ mess up %17s alignment.

		x = OSP_SB_BLUE_XMIN + 2 + 60 + 2;
		y = OSP_SB_TOP + 2;
		Q_strncpyz(nameString, ci->name, sizeof(nameString));
	}
	CG_DrawStringExt(x, y, nameString, colorWhite, qfalse, qtrue, 10, 12, 18);

	// wins/loses and ping
	if (isLeft) {
		x = OSP_SB_RED_XMIN + 50 + 2;
		y = OSP_SB_TOP + 20;
	} else {
		x = OSP_SB_BLUE_XMIN + 2 + 60 + 2;
		y = OSP_SB_TOP + 20;
	}
	CG_DrawStringExt(x, y, "^7Win   Loss  Ping  Time", colorWhite, qfalse, qtrue, 8, 8, 50);
	CG_DrawStringExt(x, y + 10 + 2,
		va("^2%3i    ^1%3i   ^7%3i   %3i", ci->wins, ci->losses, score->ping, score->time),
		colorWhite, qfalse, qtrue, 8, 8, 50 );

	// score
	if (isLeft) {
		// if double figures or negative, adjust left
		if ((int)(score->score / 10) || score->score < 0) {
			CG_DrawField(OSP_SB_RED_XMAX - 2 - 30, OSP_SB_TOP + 2 + 5, 3, 30, 36, score->score, qfalse);
		} else {
			CG_DrawField(OSP_SB_RED_XMAX - 2, OSP_SB_TOP + 2 + 5, 3, 30, 36, score->score, qfalse);
		}
	} else {
		CG_DrawField(OSP_SB_BLUE_XMIN + 2 + 30 + 2, OSP_SB_TOP + 2 + 5, 3, 30, 36, score->score, qfalse);
	}
}


/*
=================
CG_DrawOSPTourneySpectatorScore

Draws player name, ping, wins/losses
=================
*/
void CG_DrawOSPTourneySpectatorScore(int x, int y, clientInfo_t *ci, score_t *score) {
	char nameString[MAX_STRING_TOKENS];

	if (score->ping == -1) {
		CG_DrawStringExt(x, y, "connecting", colorYellow, qfalse, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 12);
	} else {
		CG_DrawStringExt(x, y, va("%2i/%-2i", ci->wins, ci->losses), colorYellow, qfalse, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 5);
		CG_DrawStringExt(x + 55, y, va("%-3i", score->ping), colorWhite, qfalse, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 5);
	}

	Q_strncpyz(nameString, ci->name, sizeof(nameString));
	CG_DrawStringExt(x + 90, y, nameString, colorWhite, qfalse, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 18);
}


/*
=================
CG_DrawOSPTourneyScoreboard

Draw the custom 1v1 scoreboard
=================
*/
void CG_DrawOSPTourneyScoreboard( void ) {
	int i, tmp;
	int x, y;
	int specTotal, specCount;
	score_t *score, *s1, *s2;
	clientInfo_t *ci;
	vec4_t hcolor;

	s1 = NULL;
	s2 = NULL;
	specTotal = 0;
	for (i = 0 ; i < cg.numScores ; i++) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[score->client];

		if (ci->team == TEAM_FREE) {
			if (!s1) {
				s1 = &cg.scores[i];
			} else if (!s2) {
				s2 = &cg.scores[i];
			}

		} else if (ci->team == TEAM_SPECTATOR) {
			specTotal++;
		}
	}

	// yellow backing
	hcolor[0] = 0.75f;
	hcolor[1] = 0.75f;
	hcolor[2] = 0.75f;
	hcolor[3] = 0.2f;

	if (s1) {
		// player one layout
		trap_R_SetColor( hcolor );
		CG_FillRect(
			OSP_SB_RED_XMIN, 
			OSP_SB_TOP, 
			OSP_SB_RED_XMAX - OSP_SB_RED_XMIN, 
			OSP_SB_HEADER - OSP_SB_TOP, 
			hcolor);
		hcolor[3] = 0.4f;
		CG_DrawBorder(
			OSP_SB_RED_XMIN, 
			OSP_SB_TOP, 
			OSP_SB_RED_XMAX - OSP_SB_RED_XMIN, 
			OSP_SB_HEADER - OSP_SB_TOP,
			1,
			hcolor);

		ci = &cgs.clientinfo[s1->client];
		CG_DrawOSPTourneyPlayerScore(ci, s1, qtrue, s2);
	}

	if (s2) {
		// player two layout
		hcolor[3] = 0.2f;
		CG_FillRect(
			OSP_SB_BLUE_XMIN, 
			OSP_SB_TOP, 
			OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
			OSP_SB_HEADER - OSP_SB_TOP,
			hcolor);
		hcolor[3] = 0.4f;
		CG_DrawBorder(
			OSP_SB_BLUE_XMIN, 
			OSP_SB_TOP, 
			OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
			OSP_SB_HEADER - OSP_SB_TOP,
			1,
			hcolor);

		ci = &cgs.clientinfo[s2->client];
		CG_DrawOSPTourneyPlayerScore(ci, s2, qfalse, s1);
	}

	// spectator layout
	if ( specTotal > 0 ) {
		specTotal++;

		CG_DrawStringExt( OSP_SB_SPEC_XMIN + 256, OSP_SB_HEADER + OSP_SB_LINE_HEIGHT + 8,
			"Spectator", colorYellow, qtrue, qtrue, 12, 12, 0 );

		CG_DrawTeamBackground( OSP_SB_SPEC_XMIN, OSP_SB_HEADER + OSP_SB_LINE_HEIGHT + 20, 
							   OSP_SB_SPEC_XMAX - OSP_SB_SPEC_XMIN,
							   (int)(specTotal / 2) * OSP_SB_LINE_HEIGHT,
							   0.20f,
							   TEAM_SPECTATOR );
	}

	specCount = 0;
	for ( i = 0 ; i < cg.numScores ; i++ ) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[ score->client ];

		if ( ci->team != TEAM_FREE ) {
			tmp = (int)( ( specCount++ ) / 2 );

			if ( (specCount % 2) == 0 ) {
				x = OSP_SB_BLUE_XMIN;
				y = OSP_SB_HEADER + ( (tmp+1) * OSP_SB_LINE_HEIGHT + 20 ) + OSP_SB_LINE_INSET;

			} else {
				x = OSP_SB_RED_XMIN;
				y = OSP_SB_HEADER + ( (tmp+1) * OSP_SB_LINE_HEIGHT + 20 ) + OSP_SB_LINE_INSET;
			}

			CG_DrawOSPTourneySpectatorScore(x, y, ci, score);
		}
	}
}
