#include "cg_local.h"

#define HUD_BUFFER_SIZE 16000

typedef enum {
	HUD_NO_ITEM,

	HUD_AMMO_MESSAGE,
	HUD_ARMOR,
	HUD_ARMOR_BAR,
	HUD_ATTACKER_NAME,
	HUD_CENTERPRINT,
	HUD_CHAT_1,
	HUD_CHAT_2,
	HUD_CHAT_3,
	HUD_CHAT_4,
	HUD_CHAT_5,
	HUD_CHAT_6,
	HUD_CHAT_7,
	HUD_CHAT_8,
	HUD_CONNECTION_INTERRUPTED,
	HUD_COUNTDOWN,
	HUD_CROSSHAIR_NAME,
	HUD_ENEMY_COUNT,
	HUD_FOLLOW_MESSAGE_1,
	HUD_FOLLOW_MESSAGE_2,
	HUD_FPS,
	HUD_GAMETYPE,
	HUD_HEALTH,
	HUD_HEALTH_BAR,
	HUD_HOLDABLE_ICON,
	HUD_HUD_NAME,
	HUD_LAGOMETER,
	HUD_PICKUP_ICON,
	HUD_PICKUP_NAME,
	HUD_POWERUP_ICON_1,
	HUD_POWERUP_ICON_2,
	HUD_POWERUP_TIMER_1,
	HUD_POWERUP_TIMER_2,
	HUD_SCORE_LIMIT,
	HUD_SCORE_TEAM,
	HUD_SCORE_ENEMY,
	HUD_SPEED,
	HUD_SPEED_BAR,
	HUD_TEAMOVERLAY_1,
	HUD_TEAMOVERLAY_2,
	HUD_TEAMOVERLAY_3,
	HUD_TEAMOVERLAY_4,
	HUD_TEAMOVERLAY_5,
	HUD_TEAMOVERLAY_6,
	HUD_TEAMOVERLAY_7,
	HUD_TEAMOVERLAY_8,
	HUD_TEAM_COUNT,
	HUD_TIMER,
	HUD_VERSUS_MESSAGE,
	HUD_VOTE_MESSAGE,
	HUD_WAITING_FOR_PLAYERS,
	HUD_WEAPON_AMMO,
	HUD_WEAPON_AMMO_ICON,
	HUD_WEAPON_ICON,
	HUD_WEAPON_LIST,

	HUD_MAX_ITEMS
} hudItemType_t;

typedef enum {
	HUD_TEXT_LEFT,
	HUD_TEXT_CENTER,
	HUD_TEXT_RIGHT,

	HUD_TEXT_TOP,
	HUD_TEXT_MIDDLE,
	HUD_TEXT_BOTTOM
} hudTextAlign_t;

typedef enum {
	HUD_FONT_ID,
	HUD_FONT_3WAVE,
	HUD_FONT_CPMA,
	HUD_FONT_IDBLOCK
} hudFont_t;

#define HUD_MULTILINE_SPACING_FACTOR	0.5  // means spaces between multiple lines of 1/2 line height

#define HUD_WEAPONLIST_HORIZONTAL		0x0001
#define HUD_WEAPONLIST_VERTICAL			0x0002

#define HUD_SPEEDBAR_HORIZONTAL 		0x0001
#define HUD_SPEEDBAR_VERTICAL			0x0002

#define HUD_HEALTH_BLINK				0x0001
#define HUD_HEALTH_HIGHLIGHT			0x0002

#define HUD_AMMO_BLINK					0x0001
#define HUD_AMMO_HIGHLIGHT				0x0002

typedef struct {
	int x, y, width, height;
	float color[4];
	float backgroundColor[4];
	int borderSize[4];  // top, right, bottom, left (like CSS)
	float borderColor[4];
	hudFont_t font;
	int fontHeight;
	int fontWidth;
	hudTextAlign_t hPosition;
	hudTextAlign_t vPosition;
	qboolean forceColor;
	qboolean shadow;
	int maxChars;
	qhandle_t image;
	int extra;  // extra bitmask field
} hudItem_t;

static hudItem_t hudItems[HUD_MAX_ITEMS];

/*
=================
CG_Hud_Reset

Reset HUD Items
=================
*/
static void CG_Hud_Reset() {
	int i;

	memset(&hudItems, 0, sizeof(hudItems));

	// set defaults
	for (i = 0; i < HUD_MAX_ITEMS; i++) {
		hudItems[i].maxChars = 999;
	}
}


/*
=================
CG_Hud_ParseTagValues

Parses attributes for tag
=================
*/
static qboolean CG_Hud_ParseTagValues(int tagType, char *p) {
	char *token;

	token = COM_ParseExt(&p, qtrue);

	if (token[0] != '{') {
		CG_Printf("ERROR: opening brace '{' not found\n");
		return qfalse;
	}

	while (*p) {
		token = COM_ParseExt(&p, qtrue);
		if (!token[0]) {
			CG_Printf( "ERROR: closing brace '}' not found\n" );
			return qfalse;
		}

		// end of tag
		if (token[0] == '}') {
			break;
		} else {
			if (cg_hudDebug.integer) {
				CG_Printf("%s parsing attribute ^3%s\n", NMS_DEBUG_PREFIX, token);
			}

			// Rectangle
			if (!Q_stricmp(token, "backgroundColor")) {
				token = COM_ParseExt(&p, qfalse);
				if (token[0] == 'T') {
					// using team colors
					clientInfo_t* ci;
					ci = &cgs.clientinfo[cg.clientNum];
					if (ci->team == TEAM_BLUE) {
						hudItems[tagType].backgroundColor[0] = 0.0f;
						hudItems[tagType].backgroundColor[1] = 0.0f;
						hudItems[tagType].backgroundColor[2] = 1.0f;
					} else if (ci->team == TEAM_RED || ci->team == TEAM_SPECTATOR) {
						hudItems[tagType].backgroundColor[0] = 1.0f;
						hudItems[tagType].backgroundColor[1] = 0.0f;
						hudItems[tagType].backgroundColor[2] = 0.0f;
					} else {
						hudItems[tagType].backgroundColor[0] = 0.5f;
						hudItems[tagType].backgroundColor[1] = 0.5f;
						hudItems[tagType].backgroundColor[2] = 0.5f;
					}

				} else if (token[0] == 'E') {
					// using enemy colors
					clientInfo_t* ci;
					ci = &cgs.clientinfo[cg.clientNum];
					if (ci->team == TEAM_SPECTATOR) {
						hudItems[tagType].backgroundColor[0] = 0.0f;
						hudItems[tagType].backgroundColor[1] = 0.0f;
						hudItems[tagType].backgroundColor[2] = 1.0f;
					} else {
						hudItems[tagType].backgroundColor[0] = cg.enemyTorsoColor[0];
						hudItems[tagType].backgroundColor[1] = cg.enemyTorsoColor[1];
						hudItems[tagType].backgroundColor[2] = cg.enemyTorsoColor[2];
					}

				} else {
					// normal RGB
					hudItems[tagType].backgroundColor[0] = atof(token);
					token = COM_ParseExt(&p, qfalse);
					hudItems[tagType].backgroundColor[1] = atof(token);
					token = COM_ParseExt(&p, qfalse);
					hudItems[tagType].backgroundColor[2] = atof(token);
				}

				// alpha parsing for RGB, team or enemy color usage
				token = COM_ParseExt(&p, qfalse);
				hudItems[tagType].backgroundColor[3] = atof(token);

			} else if (!Q_stricmp(token, "border")) {
				token = COM_ParseExt(&p, qfalse);
				hudItems[tagType].borderSize[0] = atoi(token);
				token = COM_ParseExt(&p, qfalse);
				hudItems[tagType].borderSize[1] = atoi(token);
				token = COM_ParseExt(&p, qfalse);
				hudItems[tagType].borderSize[2] = atoi(token);
				token = COM_ParseExt(&p, qfalse);
				hudItems[tagType].borderSize[3] = atoi(token);

			} else if (!Q_stricmp(token, "borderColor")) {
				token = COM_ParseExt(&p, qfalse);
				if (token[0] == 'T') {
					// using team colors
					clientInfo_t* ci;
					ci = &cgs.clientinfo[cg.clientNum];
					if (ci->team == TEAM_BLUE) {
						hudItems[tagType].borderColor[0] = 0.0f;
						hudItems[tagType].borderColor[1] = 0.0f;
						hudItems[tagType].borderColor[2] = 1.0f;
					} else if (ci->team == TEAM_RED || ci->team == TEAM_SPECTATOR) {
						hudItems[tagType].borderColor[0] = 1.0f;
						hudItems[tagType].borderColor[1] = 0.0f;
						hudItems[tagType].borderColor[2] = 0.0f;
					} else {
						hudItems[tagType].borderColor[0] = 0.5f;
						hudItems[tagType].borderColor[1] = 0.5f;
						hudItems[tagType].borderColor[2] = 0.5f;
					}

				} else if (token[0] == 'E') {
					// using enemy colors
					clientInfo_t* ci;
					ci = &cgs.clientinfo[cg.clientNum];
					if (ci->team == TEAM_SPECTATOR) {
						hudItems[tagType].backgroundColor[0] = 0.0f;
						hudItems[tagType].backgroundColor[1] = 0.0f;
						hudItems[tagType].backgroundColor[2] = 1.0f;
					} else {
						hudItems[tagType].backgroundColor[0] = cg.enemyTorsoColor[0];
						hudItems[tagType].backgroundColor[1] = cg.enemyTorsoColor[1];
						hudItems[tagType].backgroundColor[2] = cg.enemyTorsoColor[2];
					}

				} else {
					// normal RGB
					hudItems[tagType].borderColor[0] = atof(token);
					token = COM_ParseExt(&p, qfalse);
					hudItems[tagType].borderColor[1] = atof(token);
					token = COM_ParseExt(&p, qfalse);
					hudItems[tagType].borderColor[2] = atof(token);
				}

				// alpha parsing for RGB, team or enemy color usage
				token = COM_ParseExt(&p, qfalse);
				hudItems[tagType].borderColor[3] = atof(token);

			} else if (!Q_stricmp(token, "color")) {
				token = COM_ParseExt(&p, qfalse);
				if (token[0] == 'T') {
					// using team colors
					clientInfo_t* ci;
					ci = &cgs.clientinfo[cg.clientNum];
					if (ci->team == TEAM_BLUE) {
						hudItems[tagType].color[0] = 0.0f;
						hudItems[tagType].color[1] = 0.0f;
						hudItems[tagType].color[2] = 1.0f;
					} else if (ci->team == TEAM_RED || ci->team == TEAM_SPECTATOR) {
						hudItems[tagType].color[0] = 1.0f;
						hudItems[tagType].color[1] = 0.0f;
						hudItems[tagType].color[2] = 0.0f;
					} else {
						hudItems[tagType].color[0] = 0.5f;
						hudItems[tagType].color[1] = 0.5f;
						hudItems[tagType].color[2] = 0.5f;
					}

				} else if (token[0] == 'E') {
					// using enemy colors
					clientInfo_t* ci;
					ci = &cgs.clientinfo[cg.clientNum];
					if (ci->team == TEAM_SPECTATOR) {
						hudItems[tagType].backgroundColor[0] = 0.0f;
						hudItems[tagType].backgroundColor[1] = 0.0f;
						hudItems[tagType].backgroundColor[2] = 1.0f;
					} else {
						hudItems[tagType].backgroundColor[0] = cg.enemyTorsoColor[0];
						hudItems[tagType].backgroundColor[1] = cg.enemyTorsoColor[1];
						hudItems[tagType].backgroundColor[2] = cg.enemyTorsoColor[2];
					}

				} else {
					// normal RGB
					hudItems[tagType].color[0] = atof(token);
					token = COM_ParseExt(&p, qfalse);
					hudItems[tagType].color[1] = atof(token);
					token = COM_ParseExt(&p, qfalse);
					hudItems[tagType].color[2] = atof(token);
				}

				// alpha parsing for RGB, team or enemy color usage
				token = COM_ParseExt(&p, qfalse);
				hudItems[tagType].color[3] = atof(token);

			} else if (!Q_stricmp(token, "extra")) {
				token = COM_ParseExt(&p, qfalse);
				hudItems[tagType].extra = atoi(token);

			} else if (!Q_stricmp(token, "forceColor")) {
				token = COM_ParseExt( &p, qfalse );
				hudItems[tagType].forceColor = atoi( token );

			} else if (!Q_stricmp(token, "font")) {
				token = COM_ParseExt(&p, qfalse);
				if (!Q_stricmp(token, "IDBLOCK")) {
					hudItems[tagType].font = HUD_FONT_IDBLOCK;
				} else if (!Q_stricmp(token, "THREEWAVE")) {
					hudItems[tagType].font = HUD_FONT_3WAVE;
				} else if (!Q_stricmp(token, "CPMA")) {
					hudItems[tagType].font = HUD_FONT_CPMA;
				} else {
					hudItems[tagType].font = HUD_FONT_ID;
				}

			} else if (!Q_stricmp(token, "fontHorizontal")) {
				token = COM_ParseExt( &p, qfalse );
				if ( !Q_stricmp( token, "CENTER" ) ) {
					hudItems[tagType].hPosition = HUD_TEXT_CENTER;
				} else if ( !Q_stricmp( token, "LEFT" ) ) {
					hudItems[tagType].hPosition = HUD_TEXT_LEFT;
				} else if ( !Q_stricmp( token, "RIGHT" ) ) {
					hudItems[tagType].hPosition = HUD_TEXT_RIGHT;
				}

			} else if (!Q_stricmp(token, "fontShadow")) {
				token = COM_ParseExt( &p, qfalse );
				hudItems[tagType].shadow = atoi( token );

			} else if (!Q_stricmp(token, "fontSize")) {
				token = COM_ParseExt( &p, qfalse );
				hudItems[tagType].fontWidth= atoi( token );
				token = COM_ParseExt( &p, qfalse );
				hudItems[tagType].fontHeight = atoi( token );

			} else if (!Q_stricmp(token, "fontVertical")) {
				token = COM_ParseExt( &p, qfalse );
				if ( !Q_stricmp( token, "MIDDLE" ) ) {
					hudItems[tagType].vPosition = HUD_TEXT_MIDDLE;
				} else if ( !Q_stricmp( token, "TOP" ) ) {
					hudItems[tagType].vPosition = HUD_TEXT_TOP;
				} else if ( !Q_stricmp( token, "BOTTOM" ) ) {
					hudItems[tagType].vPosition = HUD_TEXT_BOTTOM;
				}

			} else if (!Q_stricmp(token, "image")) {
				token = COM_ParseExt( &p, qfalse );
				hudItems[tagType].image = trap_R_RegisterShader(token);

			} else if (!Q_stricmp(token, "rect")) {
				token = COM_ParseExt( &p, qfalse );
				hudItems[tagType].x = atof( token );
				token = COM_ParseExt( &p, qfalse );
				hudItems[tagType].y = atof( token );
				token = COM_ParseExt( &p, qfalse );
				hudItems[tagType].width = atof( token );
				token = COM_ParseExt( &p, qfalse );
				hudItems[tagType].height = atof( token );
			}
		}
    }

	return qtrue;
}

/*
=================
CG_ParseTags

Parses the tags
=================
*/
static void CG_ParseTags(char *p) {
	char *token;

	token = COM_ParseExt(&p, qtrue);

	while (*p) {
		token = COM_ParseExt(&p, qtrue);

		if (!token[0]) {
			return;
		} else {
			// embed debug messages so we don't see all the cruff passing through

			if (!Q_stricmp(token, "AMMO_MESSAGE")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_AMMO_MESSAGE, p);
			} else if (!Q_stricmp(token, "ARMOR")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_ARMOR, p);
			} else if (!Q_stricmp(token, "ARMOR_BAR")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_ARMOR_BAR, p);
			} else if (!Q_stricmp(token, "ATTACKER_NAME")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_ATTACKER_NAME, p);
			} else if (!Q_stricmp(token, "CENTER_PRINT")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_CENTERPRINT, p);
			} else if (!Q_stricmp(token, "CHAT_1")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_CHAT_1, p);
			} else if (!Q_stricmp(token, "CHAT_2")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_CHAT_2, p);
			} else if (!Q_stricmp(token, "CHAT_3")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_CHAT_3, p);
			} else if (!Q_stricmp(token, "CHAT_4")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_CHAT_4, p);
			} else if (!Q_stricmp(token, "CHAT_5")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_CHAT_5, p);
			} else if (!Q_stricmp(token, "CHAT_6")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_CHAT_6, p);
			} else if (!Q_stricmp(token, "CHAT_7")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_CHAT_7, p);
			} else if (!Q_stricmp(token, "CHAT_8")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_CHAT_8, p);
			} else if (!Q_stricmp(token, "CONNECTION_INTERRUPT")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_CONNECTION_INTERRUPTED, p);
			} else if (!Q_stricmp(token, "COUNTDOWN")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_COUNTDOWN, p);
			} else if (!Q_stricmp(token, "CROSSHAIR_NAME")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_CROSSHAIR_NAME, p);
			} else if (!Q_stricmp(token, "ENEMY_COUNT")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_ENEMY_COUNT, p);
			} else if (!Q_stricmp(token, "FOLLOW_1")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_FOLLOW_MESSAGE_1, p);
			} else if (!Q_stricmp(token, "FOLLOW_2")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_FOLLOW_MESSAGE_2, p);
			} else if (!Q_stricmp(token, "FPS")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_FPS, p);
			} else if (!Q_stricmp(token, "GAMETYPE")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_GAMETYPE, p);
			} else if (!Q_stricmp(token, "HEALTH")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_HEALTH, p);
			} else if (!Q_stricmp(token, "HEALTH_BAR")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_HEALTH_BAR, p);
			} else if (!Q_stricmp(token, "HOLDABLE_ICON")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_HOLDABLE_ICON, p);
			} else if (!Q_stricmp(token, "HUD_NAME")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_HUD_NAME, p);
			} else if (!Q_stricmp(token, "LAGOMETER")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_LAGOMETER, p);
			} else if (!Q_stricmp(token, "PICKUP_ICON")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_PICKUP_ICON, p);
			} else if (!Q_stricmp(token, "PICKUP_NAME")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_PICKUP_NAME, p);
			} else if (!Q_stricmp(token, "POWERUP_ICON_1")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_POWERUP_ICON_1, p);
			} else if (!Q_stricmp(token, "POWERUP_ICON_2")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_POWERUP_ICON_2, p);
			} else if (!Q_stricmp(token, "POWERUP_TIMER_1")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_POWERUP_TIMER_1, p);
			} else if (!Q_stricmp(token, "POWERUP_TIMER_2")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_POWERUP_TIMER_2, p);
			} else if (!Q_stricmp(token, "SCORE_LIMIT")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_SCORE_LIMIT, p);
			} else if (!Q_stricmp(token, "SCORE_TEAM")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_SCORE_TEAM, p);
			} else if (!Q_stricmp(token, "SCORE_ENEMY")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_SCORE_ENEMY, p);
			} else if (!Q_stricmp(token, "SPEED")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_SPEED, p);
			} else if (!Q_stricmp(token, "SPEED_BAR")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_SPEED_BAR, p);
			} else if (!Q_stricmp(token, "TEAMOVERLAY_1")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_TEAMOVERLAY_1, p);
			} else if (!Q_stricmp(token, "TEAMOVERLAY_2")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_TEAMOVERLAY_2, p);
			} else if (!Q_stricmp(token, "TEAMOVERLAY_3")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_TEAMOVERLAY_3, p);
			} else if (!Q_stricmp(token, "TEAMOVERLAY_4")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_TEAMOVERLAY_4, p);
			} else if (!Q_stricmp(token, "TEAMOVERLAY_5")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_TEAMOVERLAY_5, p);
			} else if (!Q_stricmp(token, "TEAMOVERLAY_6")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_TEAMOVERLAY_6, p);
			} else if (!Q_stricmp(token, "TEAMOVERLAY_7")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_TEAMOVERLAY_7, p);
			} else if (!Q_stricmp(token, "TEAMOVERLAY_8")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_TEAMOVERLAY_8, p);
			} else if (!Q_stricmp(token, "TEAM_COUNT")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_TEAM_COUNT, p);
			} else if (!Q_stricmp(token, "TIMER")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_TIMER, p);
			} else if (!Q_stricmp(token, "VERSUS")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_VERSUS_MESSAGE, p);
			} else if (!Q_stricmp(token, "VOTE")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_VOTE_MESSAGE, p);
			} else if (!Q_stricmp(token, "WAITING_FOR_PLAYERS")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_WAITING_FOR_PLAYERS, p);
			} else if (!Q_stricmp(token, "WEAPON_AMMO")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_WEAPON_AMMO, p);
			} else if (!Q_stricmp(token, "WEAPON_AMMO_ICON")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_WEAPON_AMMO_ICON, p);
			} else if (!Q_stricmp(token, "WEAPON_ICON")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_WEAPON_ICON, p);
			} else if (!Q_stricmp(token, "WEAPON_LIST")) {
				if (cg_hudDebug.integer) CG_Printf("%s parsing item ^3%s\n", NMS_DEBUG_PREFIX, token);
				CG_Hud_ParseTagValues(HUD_WEAPON_LIST, p);
			}
	    }
	}
}


/*
===================
CG_Hud_Parse

Parse hud identified in cg_hudFile

Currently we key on the type.  This allows fast retrieval, but only one element 
to be painted onscreen per type
===================
*/
void CG_Hud_Parse() {
	fileHandle_t fh;
	int	len;
	static char	buf[HUD_BUFFER_SIZE];
	char hudFile[64];

	CG_Hud_Reset();

	memset(&buf, 0, sizeof(buf));

	memset(&hudFile, 0, sizeof(hudFile));
	Q_strcat((char*)&hudFile, sizeof(hudFile), "hud/");
	Q_strcat((char*)&hudFile, sizeof(hudFile), cg_hudFile.string);

	len = trap_FS_FOpenFile(hudFile, &fh, FS_READ);
	
	if (!fh) {
		CG_Printf("FAILED. Couldn't open hud file: %s\n", hudFile);
		return;
	}
	if (len > sizeof(buf)) {
		CG_Printf("WARNING: hud file too large: %d\n", len);
		trap_FS_FCloseFile(fh);
		return;
	}

	if (len <= 0) {
		trap_FS_FCloseFile(fh);
		return;
	}

	trap_FS_Read(buf, sizeof(buf), fh);
	trap_FS_FCloseFile(fh);

	CG_ParseTags(buf);
}


/*
==================
CG_JustifiedXCoordComplex

Work out justified x position for given alignment type
==================
*/
static int CG_JustifiedXCoordComplex(int x, int width, int dataWidth, hudTextAlign_t alignment) {
	int newX;

	newX = x;
	if (alignment != HUD_TEXT_LEFT) {
		if (alignment == HUD_TEXT_CENTER) {
			int midpt = (int)(x + width / 2);
			newX = midpt - (int)(dataWidth / 2);
		}

		if (alignment == HUD_TEXT_RIGHT) {
			int farpt = x + width;
			newX = farpt - dataWidth;
		}
	}

	return newX;
}


/*
==================
CG_JustifiedXCoordNumeric

Work out justified x position for given integer
==================
*/
static int CG_JustifiedXCoordNumeric(int x, int width, int fontWidth, hudTextAlign_t hPosition, int value) {
	char num[16];
	int length;

	Com_sprintf(num, sizeof(num), "%i", value);
	length = strlen(num);

	return CG_JustifiedXCoordComplex(x, width, length * fontWidth, hPosition);
}


/*
==================
CG_JustifiedXCoordText

Work out justified x position for given text
==================
*/
static int CG_JustifiedXCoordText(int x, int width, int fontWidth, hudTextAlign_t alignment, const char* text) {
	return CG_JustifiedXCoordComplex(x, width, CG_DrawStrlen(text) * fontWidth, alignment);
}


/*
==================
CG_JustifiedYCoordComplex

Work out justified y position for given alignment type and number of lines
==================
*/
static int CG_JustifiedYCoordComplex(int y, int height, int fontHeight, int lineCount, hudTextAlign_t alignment) {
	int newY;

	newY = y;
	if (alignment != HUD_TEXT_TOP) {
		if (alignment == HUD_TEXT_BOTTOM) {
			newY = newY + height
				- (fontHeight * lineCount) // line height
				- (fontHeight * HUD_MULTILINE_SPACING_FACTOR * (lineCount-1));  // line spacing
		}

		if (alignment == HUD_TEXT_MIDDLE) {
			newY = newY + (int)(height / 2)
				+ (fontHeight/2)
				- (fontHeight * lineCount) // line height
				- (fontHeight * HUD_MULTILINE_SPACING_FACTOR * (lineCount-1));  // line spacing
		}
	}

	return newY;
}


/*
==================
CG_JustifiedYCoord 

Work out justified y position by examining rectangle and alignment setting
==================
*/
static int CG_JustifiedYCoord(hudItem_t* hudItem, int lineCount) {
	return CG_JustifiedYCoordComplex(
		hudItem->y,
		hudItem->height,
		hudItem->fontHeight,
		lineCount,
		hudItem->vPosition);
}


/*
=================
CG_Hud_DrawBackground

Draw a background if required
=================
*/
static void CG_Hud_DrawBackground(hudItem_t *hudItem) {
	if (hudItem->backgroundColor[3] > 0) {
		CG_FillRect(
			hudItem->x, 
			hudItem->y, 
			hudItem->width, 
			hudItem->height, 
			hudItem->backgroundColor);
	}
}


/*
=================
CG_Hud_DrawBorderComplex

Draw a border with varying sizes as given
=================
*/
static void CG_Hud_DrawBorderComplex(int x, int y, int width, int height, float borderColor[4],
									 int borderTop, int borderRight, int borderBottom, int borderLeft) {

	if (borderColor[3] > 0) {
		// top
		CG_FillRect(x, y, width, borderTop, borderColor);

		// right
		CG_FillRect(x + width - borderRight, y, borderRight, height, borderColor);

		// bottom
		CG_FillRect(x, y + height - borderBottom, width, borderBottom, borderColor);

		// left
		CG_FillRect(x, y, borderLeft, height, borderColor);
	}
}


/*
=================
CG_Hud_DrawBorder

Draw a border with varying sizes
=================
*/
static void CG_Hud_DrawBorder(hudItem_t *hudItem) {
	if (hudItem->borderColor[3] > 0) {
		CG_Hud_DrawBorderComplex(
			hudItem->x,
			hudItem->y, 
			hudItem->width,
			hudItem->height,
			hudItem->borderColor,
			hudItem->borderSize[0],
			hudItem->borderSize[1],
			hudItem->borderSize[2],
			hudItem->borderSize[3]);
	}
}


/*
=====================
CG_Hud_DrawGauge

Draw horizontal or vertical gauge
=====================
*/
void CG_Hud_DrawGauge(int x, int y, int width, int height, vec4_t color, int max, int value, qboolean horizontal) {
	int shadedArea;

	if (horizontal) {
		shadedArea = (value * 1.0f) / max * width;
	} else {
		shadedArea = (value * 1.0f) / max * height;
	}
	
	trap_R_SetColor(color);	

	if (horizontal) {
		CG_DrawPic(x, y, shadedArea, height, cgs.media.whiteShader);
	} else {
		CG_DrawPic(x, y + height - shadedArea, width, shadedArea, cgs.media.whiteShader);
	}

	trap_R_SetColor(NULL);
}


/*
=================
CG_Hud_GetFontShader

Get correct shader for font
=================
*/
static qhandle_t CG_Hud_GetFontShader(hudFont_t font) {
	qhandle_t shader;

	shader = cgs.media.charsetShader;
	if (font == HUD_FONT_3WAVE) {
		shader = cgs.media.threewaveCharsetShader;
	} else if (font == HUD_FONT_CPMA) {
		shader = cgs.media.cpmaCharsetShader;
	}

	return shader;
}


/*
=================
CG_Hud_DrawMultiLineText

Draw multi line text element for given type
=================
*/
static void CG_Hud_DrawMultiLineText(int x, int y, int width, int height, int fontWidth, int fontHeight, 
									 hudTextAlign_t hPosition, hudTextAlign_t vPosition, qhandle_t shader,
									 float color[4], qboolean forceColor, qboolean shadow, int maxChars,
									 const char* text, int lineCount) {

	char *start;
	int jx, jy;
	int l;

	start = (char*)text;

	jy = CG_JustifiedYCoordComplex(y, height, fontHeight, lineCount, vPosition);

	while (1) {
		char linebuffer[1024];

		for (l = 0; l < 40; l++) {
			if (!start[l] || start[l] == '\n') {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		jx = CG_JustifiedXCoordText(x, width, fontWidth, hPosition, linebuffer);

		CG_DrawStringExtComplex(jx, jy, linebuffer, color, forceColor, shadow, fontWidth, fontHeight, 
			maxChars, shader);

		jy += fontHeight + (fontHeight * HUD_MULTILINE_SPACING_FACTOR);

		while (*start && (*start != '\n')) {
			start++;
		}
		if (!*start) {
			break;
		}
		start++;
	}
}


/*
=================
CG_Hud_DrawTextComplex

Draw text for with given details
=================
*/
static void CG_Hud_DrawTextComplex(int x, int y, int width, int height, int fontWidth, int fontHeight,
								   hudTextAlign_t hPosition, hudTextAlign_t vPosition, hudFont_t font, 
								   float color[4], qboolean forceColor, qboolean shadow, int maxChars,
								   const char* text) {

	int jx, jy;
	int lineCount;
	char *s;
	qhandle_t shader;

	shader = CG_Hud_GetFontShader(font);

	// multi line check
	lineCount = 1;
	s = (char*)text;
	while (*s) {
		if (*s == '\n')
			lineCount++;
		s++;
	}

	if (lineCount > 1) {
		CG_Hud_DrawMultiLineText(x, y, width, height, fontWidth, fontHeight, hPosition, vPosition,
			shader, color, forceColor, shadow, maxChars, text, lineCount);
		return;
	}

	jx = CG_JustifiedXCoordText(x, width, fontWidth, hPosition,  text);
	jy = CG_JustifiedYCoordComplex(y, height, fontHeight, lineCount, vPosition);

	CG_DrawStringExtComplex(jx, jy, text, color, forceColor, shadow, fontWidth, fontHeight, 
		maxChars, shader);
}


/*
=================
CG_Hud_DrawTextElement

Draw text element for given type
=================
*/
static void CG_Hud_DrawTextElement(int type, const char* text) {
	hudItem_t *hudItem;

	hudItem = &hudItems[type];

	if (hudItem == NULL) {
		return;
	}

	CG_Hud_DrawBackground(hudItem);
	CG_Hud_DrawBorder(hudItem);

	if (cg_hudDebug.integer) {
		CG_DrawBorder(hudItem->x, hudItem->y, hudItem->width, hudItem->height, 1, colorYellow);
	}

	CG_Hud_DrawTextComplex(
		hudItem->x,
		hudItem->y,
		hudItem->width,
		hudItem->height,
		hudItem->fontWidth,
		hudItem->fontHeight,
		hudItem->hPosition,
		hudItem->vPosition,
		hudItem->font,
		hudItem->color,
		hudItem->forceColor,
		hudItem->shadow,
		hudItem->maxChars,
		text);
}


/*
=================
CG_Hud_DrawIntComplex

Draw integer with given details
=================
*/
static void CG_Hud_DrawIntComplex(int x, int y, int width, int height, int fontWidth, int fontHeight,
								  hudTextAlign_t hPosition, hudTextAlign_t vPosition, hudFont_t font, 
								  float color[4], qboolean forceColor, qboolean shadow, int maxChars,
								  int value) {

	if (font == HUD_FONT_IDBLOCK) {
		char num[16], *ptr;
		int length, frame;
		int jx, jy;
		vec4_t hcolor;

		hcolor[0] = color[0];
		hcolor[1] = color[1];
		hcolor[2] = color[2];
		hcolor[3] = color[3];

		jx = CG_JustifiedXCoordNumeric(x, width, fontWidth, hPosition, value);
		jy = CG_JustifiedYCoordComplex(y, height, fontHeight, 1, vPosition);

		Com_sprintf(num, sizeof(num), "%i", value);
		length = strlen(num);
	
		trap_R_SetColor(color);

		ptr = num;
		while (*ptr && length) {
			if (*ptr == '-') {
				frame = STAT_MINUS;
			} else {
				frame = *ptr -'0';
			}

			CG_DrawPic(jx, jy, fontWidth, fontHeight, cgs.media.numberShaders[frame]);

			jx += fontWidth;
			ptr++;
			length--;
		}

		trap_R_SetColor(NULL);

	} else {
		CG_Hud_DrawTextComplex(x, y, width, height, fontWidth, fontHeight, hPosition, vPosition, font,
			color, forceColor, shadow, maxChars, va("%i", value));
	}
}


/*
=================
CG_Hud_DrawIntElement

Draw integer element for given type
=================
*/
static void CG_Hud_DrawIntElement(int type, int value) {
	hudItem_t *hudItem;

	hudItem = &hudItems[type];

	if (hudItem == NULL) {
		return;
	}

	CG_Hud_DrawBackground(hudItem);
	CG_Hud_DrawBorder(hudItem);

	if (cg_hudDebug.integer) {
		CG_DrawBorder(hudItem->x, hudItem->y, hudItem->width, hudItem->height, 1, colorYellow);
	}

	CG_Hud_DrawIntComplex(
		hudItem->x,
		hudItem->y,
		hudItem->width,
		hudItem->height,
		hudItem->fontWidth,
		hudItem->fontHeight,
		hudItem->hPosition,
		hudItem->vPosition,
		hudItem->font,
		hudItem->color,
		hudItem->forceColor,
		hudItem->shadow,
		hudItem->maxChars,
		value);
}


/*
================
CG_Hud_DrawBlinkHighlightIntElement

Draws an integer element blinking and/or highlighted red if value below MAX
================
*/
static void CG_Hud_DrawBlinkHighlightIntElement(int type, int value, int max, int blink, int highlight) {
	hudItem_t *hudItem;

	hudItem = &hudItems[type];

	if (hudItem == NULL) {
		return;
	}

	CG_Hud_DrawBackground(hudItem);
	CG_Hud_DrawBorder(hudItem);

	if (cg_hudDebug.integer) {
		CG_DrawBorder(hudItem->x, hudItem->y, hudItem->width, hudItem->height, 1, colorYellow);
	}

	if (value < max && value > 0) {
		if (hudItem->extra & blink) {
			int tmp;
			tmp = ((cg.time / 500) % 2);

			if (tmp) {
				// don't draw this frame
				return;
			}
		}

		if (hudItem->extra & highlight) {
			float color[4];
			color[0] = 1.0f;
			color[1] = 0.0f;
			color[2] = 0.0f;
			color[3] = hudItem->color[3];

			CG_Hud_DrawIntComplex(
				hudItem->x,
				hudItem->y,
				hudItem->width,
				hudItem->height,
				hudItem->fontWidth,
				hudItem->fontHeight,
				hudItem->hPosition,
				hudItem->vPosition,
				hudItem->font,
				color,
				hudItem->forceColor,
				hudItem->shadow,
				hudItem->maxChars,
				value);

			// drawn, so leave
			return;
		}
	}

	CG_Hud_DrawIntComplex(
		hudItem->x,
		hudItem->y,
		hudItem->width,
		hudItem->height,
		hudItem->fontWidth,
		hudItem->fontHeight,
		hudItem->hPosition,
		hudItem->vPosition,
		hudItem->font,
		hudItem->color,
		hudItem->forceColor,
		hudItem->shadow,
		hudItem->maxChars,
		value);
}


/*
=================
CG_Hud_DrawGaugeElement

Draws a double gauge element using color and backgroundColor
=================
*/
#define GAUGE_PADDING 5
static void CG_Hud_DrawGaugeElement(int type, int maxPerGauge, int value) {
	hudItem_t *hudItem;
	int gaugeHeight, firstValue, secondValue;

	hudItem = &hudItems[type];

	if (hudItem == NULL) {
		return;
	}

	if (cg_hudDebug.integer) {
		CG_DrawBorder(hudItem->x, hudItem->y, hudItem->width, hudItem->height, 1, colorYellow);
	}

	gaugeHeight = (hudItem->height - GAUGE_PADDING) / 2;
	if (value > maxPerGauge) {
		firstValue = maxPerGauge;
		secondValue = value - maxPerGauge;
		if (secondValue > maxPerGauge) {
			secondValue = maxPerGauge;
		}
	} else {
		firstValue = value;
		secondValue = 0;
	}

	CG_Hud_DrawGauge(
		hudItem->x, 
		hudItem->y, 
		hudItem->width, 
		gaugeHeight, 
		hudItem->color, 
		maxPerGauge, 
		firstValue, 
		qtrue);

	CG_Hud_DrawBorderComplex(
		hudItem->x,
		hudItem->y,
		hudItem->width,
		gaugeHeight,
		hudItem->borderColor,
		hudItem->borderSize[0],
		hudItem->borderSize[1],
		hudItem->borderSize[2],
		hudItem->borderSize[3]);

	CG_Hud_DrawGauge(
		hudItem->x, 
		hudItem->y + gaugeHeight + GAUGE_PADDING, 
		hudItem->width, 
		gaugeHeight, 
		hudItem->backgroundColor, 
		maxPerGauge, 
		secondValue, 
		qtrue);

	CG_Hud_DrawBorderComplex(
		hudItem->x,
		hudItem->y + gaugeHeight + GAUGE_PADDING,
		hudItem->width,
		gaugeHeight,
		hudItem->borderColor,
		hudItem->borderSize[0],
		hudItem->borderSize[1],
		hudItem->borderSize[2],
		hudItem->borderSize[3]);
}


/*
=================
CG_Hud_DrawIcon

Draw justified icon in given rectangle
=================
*/
static void CG_Hud_DrawIcon(int x, int y, int width, int height, 
							int fontWidth, int fontHeight,
							hudTextAlign_t hPosition, hudTextAlign_t vPosition, 
							qhandle_t icon) {
	int jx, jy;

	jx = CG_JustifiedXCoordComplex(x, width, fontWidth, hPosition);
	jy = CG_JustifiedYCoordComplex(y, height, fontHeight, 1, vPosition);

	CG_DrawPic(jx, jy, fontWidth, fontHeight, icon);
}


/*
=================
CG_Hud_DrawIconElement

Draw justified icon element for given type
=================
*/
static void CG_Hud_DrawIconElement(int type, qhandle_t icon) {
	hudItem_t *hudItem;

	hudItem = &hudItems[type];

	if (hudItem == NULL) {
		return;
	}

	CG_Hud_DrawBackground(hudItem);
	CG_Hud_DrawBorder(hudItem);

	if (cg_hudDebug.integer) {
		CG_DrawBorder(hudItem->x, hudItem->y, hudItem->width, hudItem->height, 1, colorYellow);
	}

	// if a fixed image has been set, use it
	if (hudItem->image) {
		icon = hudItem->image;
	}

	CG_Hud_DrawIcon(
		hudItem->x,
		hudItem->y,
		hudItem->width,
		hudItem->height,
		hudItem->fontWidth,
		hudItem->fontHeight,
		hudItem->hPosition,
		hudItem->vPosition,
		icon);
}


/*
=================
CG_Hud_DrawSpeedBarElement

Draw speed bar element for given type
=================
*/
#define SPEEDBAR_MAX_SPEED 1000
static void CG_Hud_DrawSpeedBarElement(int type) {
	hudItem_t *hudItem;
	char *text;
	int normalSpeed;
	int index, currentSpeed;
	int textFullWidth;  // height and width of bar text
	
	static float bar_colors[3][4] = { 
		{ 0.0f, 1.0f, 0.0f, 0.9f },  // green
		{ 1.0f, 0.9f, 0.0f, 0.9f },  // amber
		{ 1.0f, 0.0f, 0.0f, 0.9f }   // red
	};

	hudItem = &hudItems[type];

	if (hudItem == NULL) {
		return;
	}

	if (cg_hudDebug.integer) {
		CG_DrawBorder(hudItem->x, hudItem->y, hudItem->width, hudItem->height, 1, colorYellow);
	}

	currentSpeed = (int)cg.xyspeed;
	text = va("%i", (int)cg.xyspeed);
	textFullWidth = CG_DrawStrlen(text) * hudItem->fontWidth;

	normalSpeed = cg.snap->ps.speed;
	if (currentSpeed < (int)(normalSpeed * 1.25)) {  // 25%
		index = 0;
	} else if (currentSpeed < (int)(normalSpeed * 1.9)) {  // 90%
		index = 1;
	} else {
		index = 2;
	}

	CG_Hud_DrawGauge(
		hudItem->x,
		hudItem->y,
		hudItem->width,
		hudItem->height,
		bar_colors[index],
		SPEEDBAR_MAX_SPEED,
		currentSpeed,
		!(hudItem->extra & HUD_SPEEDBAR_VERTICAL));

	CG_Hud_DrawTextComplex(
		hudItem->x,
		hudItem->y,
		hudItem->width,
		hudItem->height,
		hudItem->fontWidth,
		hudItem->fontHeight,
		hudItem->hPosition,
		hudItem->vPosition,
		hudItem->font,
		hudItem->color,
		hudItem->forceColor,
		hudItem->shadow,
		hudItem->maxChars,
		text);

	CG_Hud_DrawBorder(hudItem);
}


/*
=================
CG_Hud_DrawTeamOverlayElement

Draw entry for team overlay
=================
*/
#define TEAMOVERLAY_PADDING 5
#define TEAMOVERLAY_MAX_NAME_CHARS 10
#define TEAMOVERLAY_MAX_HEALTH_CHARS 3
#define TEAMOVERLAY_MAX_WEAPON_CHARS 1
#define TEAMOVERLAY_MAX_ARMOR_CHARS 3
#define TEAMOVERLAY_MAX_LOCATION_CHARS 12
static void CG_Hud_DrawTeamOverlayElement(int type, clientInfo_t* ci) {
	hudItem_t *hudItem;
	int totalWidth, jx, value;
	const char *location;
	qhandle_t shader;

	hudItem = &hudItems[type];

	if (hudItem == NULL) {
		return;
	}

	shader = CG_Hud_GetFontShader(hudItem->font);

	totalWidth = 0;
	totalWidth += TEAMOVERLAY_MAX_NAME_CHARS * hudItem->fontWidth;
	totalWidth += TEAMOVERLAY_PADDING;
	totalWidth += TEAMOVERLAY_MAX_HEALTH_CHARS * hudItem->fontWidth;
	totalWidth += TEAMOVERLAY_MAX_WEAPON_CHARS * hudItem->fontWidth;
	totalWidth += TEAMOVERLAY_MAX_ARMOR_CHARS * hudItem->fontWidth;
	totalWidth += TEAMOVERLAY_PADDING;
	totalWidth += TEAMOVERLAY_MAX_LOCATION_CHARS * hudItem->fontWidth;

	jx = CG_JustifiedXCoordComplex(hudItem->x, hudItem->width, totalWidth, hudItem->hPosition);

	CG_Hud_DrawBackground(hudItem);
	CG_Hud_DrawBorder(hudItem);

	if (cg_hudDebug.integer) {
		CG_DrawBorder(hudItem->x, hudItem->y, hudItem->width, hudItem->height, 1, colorYellow);
	}

	CG_Hud_DrawTextComplex(
		jx,
		hudItem->y,
		hudItem->fontWidth * TEAMOVERLAY_MAX_NAME_CHARS,
		hudItem->fontHeight,
		hudItem->fontWidth,
		hudItem->fontHeight,
		HUD_TEXT_LEFT,
		hudItem->vPosition,
		hudItem->font,
		hudItem->color,
		hudItem->forceColor,
		hudItem->shadow,
		TEAMOVERLAY_MAX_NAME_CHARS,
		ci->name);
	jx += TEAMOVERLAY_MAX_NAME_CHARS * hudItem->fontWidth;
	jx += TEAMOVERLAY_PADDING;

	if (ci->health > 0) {
		value = ci->health;
		CG_Hud_DrawIntComplex(
			jx,
			hudItem->y,
			hudItem->fontWidth * TEAMOVERLAY_MAX_HEALTH_CHARS,
			hudItem->fontHeight,
			hudItem->fontWidth,
			hudItem->fontHeight,
			HUD_TEXT_RIGHT,
			hudItem->vPosition,
			hudItem->font,
			hudItem->color,
			hudItem->forceColor,
			hudItem->shadow,
			TEAMOVERLAY_MAX_HEALTH_CHARS,
			value);

		jx += TEAMOVERLAY_MAX_HEALTH_CHARS * hudItem->fontWidth;

		if (cg_weapons[ci->curWeapon].weaponIcon) {
			CG_Hud_DrawIcon(
				jx,
				hudItem->y,
				hudItem->fontWidth * TEAMOVERLAY_MAX_WEAPON_CHARS,
				hudItem->fontHeight,
				hudItem->fontWidth,
				hudItem->fontHeight,
				HUD_TEXT_CENTER,
				hudItem->vPosition,
				cg_weapons[ci->curWeapon].weaponIcon);
		}

		jx += TEAMOVERLAY_MAX_WEAPON_CHARS * hudItem->fontWidth;

		value = ci->armor;
		CG_Hud_DrawIntComplex(
			jx,
			hudItem->y,
			hudItem->fontWidth * TEAMOVERLAY_MAX_ARMOR_CHARS,
			hudItem->fontHeight,
			hudItem->fontWidth,
			hudItem->fontHeight,
			HUD_TEXT_LEFT,
			hudItem->vPosition,
			hudItem->font,
			hudItem->color,
			hudItem->forceColor,
			hudItem->shadow,
			TEAMOVERLAY_MAX_ARMOR_CHARS,
			value);

		jx += TEAMOVERLAY_MAX_ARMOR_CHARS * hudItem->fontWidth;
		jx += TEAMOVERLAY_PADDING;

	} else {
		char* status;
		int statusWidth;

		if (CG_isFreezeTag() && CG_isFrozen(ci - cgs.clientinfo)) {
			status = "FROZEN";
		} else {
			status = "READY";
		}

		statusWidth = 0;
		statusWidth += hudItem->fontWidth * TEAMOVERLAY_MAX_HEALTH_CHARS;
		statusWidth += hudItem->fontWidth * TEAMOVERLAY_MAX_WEAPON_CHARS;
		statusWidth += hudItem->fontWidth * TEAMOVERLAY_MAX_ARMOR_CHARS;

		CG_Hud_DrawTextComplex(
			jx,
			hudItem->y,
			statusWidth,
			hudItem->fontHeight,
			hudItem->fontWidth,
			hudItem->fontHeight,
			HUD_TEXT_CENTER,
			hudItem->vPosition,
			hudItem->font,
			colorYellow,
			hudItem->forceColor,
			hudItem->shadow,
			6,
			status);

		jx += TEAMOVERLAY_MAX_HEALTH_CHARS * hudItem->fontWidth;
		jx += TEAMOVERLAY_MAX_WEAPON_CHARS * hudItem->fontWidth;
		jx += TEAMOVERLAY_MAX_ARMOR_CHARS * hudItem->fontWidth;
		jx += TEAMOVERLAY_PADDING;
	}

	location = CG_ConfigString(CS_LOCATIONS + ci->location);
	if (!location || !*location) {
		location = "unknown";
	}

	CG_Hud_DrawTextComplex(
		jx,
		hudItem->y,
		hudItem->fontWidth * TEAMOVERLAY_MAX_LOCATION_CHARS,
		hudItem->fontHeight,
		hudItem->fontWidth,
		hudItem->fontHeight,
		HUD_TEXT_LEFT,
		hudItem->vPosition,
		hudItem->font,
		hudItem->color,
		hudItem->forceColor,
		hudItem->shadow,
		TEAMOVERLAY_MAX_LOCATION_CHARS,
		location);
}


/*
=================
CG_Hud_DrawWeaponListElement

Draw weapon list
=================
*/
#define WEAPONLIST_PADDING 5  // pixels between icon and ammo count and between each weapon box
static void CG_Hud_DrawWeaponListElement(int type) {
	hudItem_t *hudItem;
	centity_t *cent;
	playerState_t *ps;
	int i, weaponCount;
	int x, y, ix, size, value;
	qhandle_t icon;

	hudItem = &hudItems[type];

	if (hudItem == NULL) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	// work out how many weapons we need to draw
	weaponCount = 0;
	for (i = WP_MACHINEGUN; i < WP_NUM_WEAPONS-1; i++) {
		if (ps->stats[STAT_WEAPONS] & (1 << i)) {
			weaponCount++;
		}
	}

	// we centre around x, y
	// width and height are values for each weapon box, so work out actual width/height needed
	if (hudItem->extra & HUD_WEAPONLIST_HORIZONTAL) {
		size = weaponCount * hudItem->width + (weaponCount-1) * WEAPONLIST_PADDING;
		x = hudItem->x - (size / 2);
		y = hudItem->y - (hudItem->height / 2);
		ix = x + ((hudItem->width - WEAPONLIST_PADDING - (hudItem->fontWidth * 4)) / 2);  // center items within box
	} else {
		// assume vertical
		size = weaponCount * hudItem->height + (weaponCount-1) * WEAPONLIST_PADDING;
		x = hudItem->x - (hudItem->width / 2);
		y = hudItem->y - (size / 2);
		ix = x + ((hudItem->width - WEAPONLIST_PADDING - (hudItem->fontWidth * 4)) / 2);  // center items within box
	}

	for (i = WP_MACHINEGUN; i < WP_NUM_WEAPONS-1; i++) {
		if (ps->stats[STAT_WEAPONS] & (1 << i)) {
			value = ps->ammo[i];
			icon = cg_weapons[i].weaponIcon;

			// draw background
			if (cent->currentState.weapon == i) {
				CG_FillRect(x, y, hudItem->width, hudItem->height, hudItem->color);
			} else if (hudItem->backgroundColor[3] > 0) {
				CG_FillRect(x, y, hudItem->width, hudItem->height, hudItem->backgroundColor);
			}

			CG_Hud_DrawBorderComplex(x, y, hudItem->width, hudItem->height, hudItem->borderColor,
				hudItem->borderSize[0], hudItem->borderSize[1], hudItem->borderSize[2], hudItem->borderSize[3]);

			CG_Hud_DrawIcon(
				ix, 
				y, 
				hudItem->fontWidth,
				hudItem->height,
				hudItem->fontWidth,
				hudItem->fontHeight,
				hudItem->hPosition,
				hudItem->vPosition,
				icon);

			CG_Hud_DrawIntComplex(
				ix + hudItem->fontWidth + WEAPONLIST_PADDING, 
				y, 
				hudItem->fontWidth * 3,
				hudItem->height,
				hudItem->fontWidth,
				hudItem->fontHeight,
				hudItem->hPosition,
				hudItem->vPosition,
				hudItem->font,
				colorWhite,
				qtrue,
				hudItem->shadow,
				3,
				value);

			if (hudItem->extra & HUD_WEAPONLIST_HORIZONTAL) {
				x += hudItem->width + WEAPONLIST_PADDING;
				ix = x + ((hudItem->width - WEAPONLIST_PADDING - (hudItem->fontWidth * 4)) / 2);  // center within box
			} else {
				// assume vertical
				y += hudItem->height + WEAPONLIST_PADDING;
			}
		}
	}
}


//==================================================================================

/*
================
CG_Hud_DrawStatusBar

SuperHUD version of CG_DrawStatusBar
================
*/
static void CG_Hud_DrawStatusBar() {
	centity_t *cent;
	playerState_t *ps;
	int value;

	if (cg_drawStatus.integer == 0) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	// health
	value = ps->stats[STAT_HEALTH];
	CG_Hud_DrawGaugeElement(HUD_HEALTH_BAR, 100, value);
	CG_Hud_DrawBlinkHighlightIntElement(HUD_HEALTH, value, 25, HUD_HEALTH_BLINK, HUD_HEALTH_HIGHLIGHT);

	// armor
	value = ps->stats[STAT_ARMOR];
	CG_Hud_DrawGaugeElement(HUD_ARMOR_BAR, 100, value);
	CG_Hud_DrawIntElement(HUD_ARMOR, value);

	// weapon ammo
	if (cent->currentState.weapon) {
		value = ps->ammo[cent->currentState.weapon];

		if (value >= 0) {
			CG_Hud_DrawBlinkHighlightIntElement(HUD_WEAPON_AMMO, value, 5, HUD_AMMO_BLINK, HUD_AMMO_HIGHLIGHT);
		}
	}

	// weapon icon
	if (cent->currentState.weapon && cg_weapons[cent->currentState.weapon].weaponIcon) {
		CG_Hud_DrawIconElement(HUD_WEAPON_ICON, cg_weapons[cent->currentState.weapon].weaponIcon);
	}

	// weapon ammo icon
	if (cent->currentState.weapon && cg_weapons[cent->currentState.weapon].ammoIcon) {
		CG_Hud_DrawIconElement(HUD_WEAPON_AMMO_ICON, cg_weapons[cent->currentState.weapon].ammoIcon);
	}
}


/*
==================
CG_Hud_DrawFPS

SuperHUD version of CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static void CG_Hud_DrawFPS() {
	char		*s;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va( "%ifps", fps );
		CG_Hud_DrawTextElement(HUD_FPS, s);
	}
}


/*
=================
CG_Hud_DrawTimer

SuperHUD version of CG_DrawTimer
=================
*/
static void CG_Hud_DrawTimer() {
	char		*s;
	int			mins, seconds, tens;
	int			msec, timeLeft, milliMin = 60000;

	timeLeft = ( cgs.timelimit * milliMin ) - ( cg.time - cgs.levelStartTime );
	msec = cg.time - cgs.levelStartTime;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	s = va( "%i:%i%i", mins, tens, seconds );
	CG_Hud_DrawTextElement(HUD_TIMER, s);
}


/*
=================
CG_Hud_DrawSpeed

SuperHUD version of CG_DrawSpeed
=================
*/
static void CG_Hud_DrawSpeed() {
	char *s;

	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	s = va("%i u", (int)cg.xyspeed);
	CG_Hud_DrawTextElement(HUD_SPEED, s);
}


/*
=================
CG_Hud_DrawSpeedBar

SuperHUD version of CG_DrawSpeedBar
=================
*/
static void CG_Hud_DrawSpeedBar() {
	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	CG_Hud_DrawSpeedBarElement(HUD_SPEED_BAR);
}


/*
=================
CG_Hud_DrawScores

SuperHUD version of CG_DrawScores
=================
*/
static void CG_Hud_DrawScores() {
	int			s1, s2;
	int			limit;

	limit = cgs.fraglimit;

	if (cgs.gametype >= GT_TEAM) {
		if (cgs.gametype == GT_CTF) {
			limit = cgs.capturelimit;
		} else if ( cgs.gametype == GT_FREEZE ) {
			limit = cgs.freezelimit;
		}

		s1 = cgs.scores1;
		s2 = cgs.scores2;

	} else {
		s1 = cg.snap->ps.persistant[PERS_SCORE];
		s2 = 0;
	}

	CG_Hud_DrawIntElement(HUD_SCORE_LIMIT, limit);
	CG_Hud_DrawIntElement(HUD_SCORE_TEAM, s1);
	CG_Hud_DrawIntElement(HUD_SCORE_ENEMY, s2);
}


/*
=================
CG_Hud_DrawTeamCount

Draws the count of alive players for own and enemy team
=================
*/
static void CG_Hud_DrawTeamCount() {
	int	i;
	int	teamCount, enemyCount;
	clientInfo_t *yourCi, *ci;

	// only valid if we are actually playing
	if (cg.clientNum != cg.snap->ps.clientNum) {
		return;
	}

	yourCi = &cgs.clientinfo[cg.clientNum];
	if (!(yourCi->team == TEAM_BLUE || yourCi->team == TEAM_RED)) {
		return;
	}

	// don't display when dead
	if (yourCi->health <= 0) {
		return;
	}

	teamCount = 0;
	enemyCount = 0;

	for (i = 0; i < MAX_CLIENTS; i++) {
		ci = &cgs.clientinfo[i];

		if (!ci->infoValid) {
			continue;
		}

		if (yourCi->team == ci->team && !CG_isFrozen(i)) {
			teamCount++;
		} else if ((ci->team == TEAM_RED || ci->team == TEAM_BLUE) && !CG_isFrozen(i)) {
			enemyCount++;
		}
	}

	CG_Hud_DrawIntElement(HUD_TEAM_COUNT, teamCount);
	CG_Hud_DrawIntElement(HUD_ENEMY_COUNT, enemyCount);
}


/*
=================
CG_Hud_DrawTeamOverlay

SuperHUD version of CG_DrawTeamOverlay
=================
*/
static void CG_Hud_DrawTeamOverlay() {
	int i;
	const char *p;
	int plyrs, type;
	clientInfo_t *ci;

	if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE) {
		return; // Not on any team
	}

	plyrs = 0;
	type = 0;
	for (i = 0; i < numSortedTeamPlayers; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if (ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {
			plyrs++;

			switch (plyrs) {
			case 1:
				type = HUD_TEAMOVERLAY_1;
				break;
			case 2:
				type = HUD_TEAMOVERLAY_2;
				break;
			case 3:
				type = HUD_TEAMOVERLAY_3;
				break;
			case 4:
				type = HUD_TEAMOVERLAY_4;
				break;
			case 5:
				type = HUD_TEAMOVERLAY_5;
				break;
			case 6:
				type = HUD_TEAMOVERLAY_6;
				break;
			case 7:
				type = HUD_TEAMOVERLAY_7;
				break;
			case 8:
				type = HUD_TEAMOVERLAY_8;
				break;
			}

			p = CG_ConfigString(CS_LOCATIONS + ci->location);
			if (!p || !*p) p = "unknown";

			CG_Hud_DrawTeamOverlayElement(type,	ci);
		}
	}
}


/*
===================
CG_Hud_DrawPickupItem

SuperHUD version of CG_DrawPickupItem
===================
*/
static void CG_Hud_DrawPickupItem() {
	int value;
	float *fadeColor;  // used to determine if we show or not for now

	value = cg.itemPickup;
	if (value) {
		fadeColor = CG_FadeColor(cg.itemPickupTime, 3000);
		if (fadeColor) {
			CG_RegisterItemVisuals(value);
			CG_Hud_DrawIconElement(HUD_PICKUP_ICON, cg_items[value].icon);
			CG_Hud_DrawTextElement(HUD_PICKUP_NAME, bg_itemlist[value].pickup_name);
		}
	}
}


/*
================
CG_Hud_DrawPowerups

SuperHUD version of CG_DrawPowerups
================
*/
static void CG_Hud_DrawPowerups() {
	int sorted[MAX_POWERUPS];
	int sortedTime[MAX_POWERUPS];
	int i, j, k;
	int active;
	playerState_t *ps;
	int t;
	gitem_t *item;
	int iconType, timerType;

	ps = &cg.snap->ps;

	// sort the list by time remaining
	active = 0;
	for (i = 0 ; i < MAX_POWERUPS ; i++) {
		if (!ps->powerups[i]) {
			continue;
		}
		t = ps->powerups[i] - cg.time;
		// ZOID--don't draw if the power up has unlimited time (999 seconds)
		// This is true of the CTF flags
		if (t < 0 || t > 999000) {
			continue;
		}

		// insert into the list
		for (j = 0 ; j < active ; j++) {
			if (sortedTime[j] >= t) {
				for (k = active - 1 ; k >= j ; k--) {
					sorted[k+1] = sorted[k];
					sortedTime[k+1] = sortedTime[k];
				}
				break;
			}
		}
		sorted[j] = i;
		sortedTime[j] = t;
		active++;
	}

	// draw the icons and timers
	for (i = 0 ; i < active ; i++) {
		if (i == 0) {
			iconType = HUD_POWERUP_ICON_1;
			timerType = HUD_POWERUP_TIMER_1;
		} else if (i == 1) {
			iconType = HUD_POWERUP_ICON_2;
			timerType = HUD_POWERUP_TIMER_2;
		} else {
			continue;  // only handles 2 powerups at the moment
		}

		item = BG_FindItemForPowerup(sorted[i]);

		CG_Hud_DrawIntElement(timerType, sortedTime[i] / 1000);
		CG_Hud_DrawIconElement(iconType, trap_R_RegisterShader(item->icon));
	}
}


/*
=================
CG_Hud_DrawTeamInfo

SuperHUD version of CG_DrawTeamInfo
=================
*/
static void CG_Hud_DrawTeamInfo( void ) {
	int i;
	int	chatHeight, type;

	if (cg.chatTime + (cg_chatTime.integer * 1000) < cg.time) {
		return;
	}

	chatHeight = 8;  // max HUD_CHAT_X elements

	if (cgs.chatLastPos != cgs.chatPos) {
		if (cg.time - cgs.chatMsgTimes[cgs.chatLastPos % chatHeight] > (cg_chatTime.integer * 1000 )) {
			cgs.chatLastPos++;
		}

		type = HUD_CHAT_1;
		for (i = cgs.chatPos - 1; i >= cgs.chatLastPos; i--) {
			CG_Hud_DrawTextElement(type, cgs.chatMsgs[i % chatHeight]);

			if (++type > HUD_CHAT_8) {
				continue;
			}
		}
	}
}


/*
=================
CG_Hud_DrawVote

SuperHUD version of CG_DrawVote
=================
*/
static void CG_Hud_DrawVote(void) {
	char *s;
	int sec;

	if (!cgs.voteTime) {
		return;
	}

	// play a talk beep whenever it is modified
	if (cgs.voteModified) {
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
	}

	sec = (VOTE_TIME - (cg.time - cgs.voteTime)) / 1000;
	if (sec < 0) {
		sec = 0;
	}

	s = va("VOTE(%i):%s yes(F1):%i no(F2):%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
	CG_Hud_DrawTextElement(HUD_VOTE_MESSAGE, s);
}


/*
=================
CG_Hud_DrawFollow

SuperHUD version of CG_DrawFollow
=================
*/
static qboolean CG_Hud_DrawFollow( void ) {
	const char *name;

	if (!(cg.snap->ps.pm_flags & PMF_FOLLOW) || cg.showScores == qtrue) {
		return qfalse;
	}

	name = cgs.clientinfo[cg.snap->ps.clientNum].name;

	CG_Hud_DrawTextElement(HUD_FOLLOW_MESSAGE_1, "following");
	CG_Hud_DrawTextElement(HUD_FOLLOW_MESSAGE_2, name);

	return qtrue;
}


/*
=================
CG_DrawWarmup

SuperHUD version of CG_DrawWarmup
=================
*/
static void CG_Hud_DrawWarmup( void ) {
	int sec;
	int i;
	clientInfo_t *ci1, *ci2;
	const char *s;

	sec = cg.warmup;
	if (!sec || cg.showScores) {
		return;
	}

	if (sec < 0) {
		s = "Waiting for players";
		CG_Hud_DrawTextElement(HUD_WAITING_FOR_PLAYERS, s);
		cg.warmupCount = 0;
		return;
	}

	if (cgs.gametype == GT_TOURNAMENT) {
		// find the two active players
		ci1 = NULL;
		ci2 = NULL;
		for (i = 0 ; i < cgs.maxclients ; i++) {
			if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE) {
				if (!ci1) {
					ci1 = &cgs.clientinfo[i];
				} else {
					ci2 = &cgs.clientinfo[i];
				}
			}
		}

		if (ci1 && ci2) {
			s = va("%s ^7vs %s", ci1->name, ci2->name);
			CG_Hud_DrawTextElement(HUD_VERSUS_MESSAGE, s);
		}
	} else {
		if (cgs.gametype == GT_FFA) {
			s = "Free For All";
		// Nemesis - Freeze
		} else if (cgs.gametype == GT_TEAM && cgs.g_gameMod == 0) {
			s = "Team Deathmatch";
		} else if (CG_isFreezeTag()) {
			s = "Team Freezetag";
		// End
		} else if (cgs.gametype == GT_CTF) {
			s = "Capture the Flag";
		} else {
			s = "";
		}

		CG_Hud_DrawTextElement(HUD_GAMETYPE, s);
	}

	sec = (sec - cg.time) / 1000;
	if (sec < 0) {
		sec = 0;
	}
	s = va("Starts in: %i", sec + 1);
	if (sec != cg.warmupCount) {
		cg.warmupCount = sec;
		switch (sec) {
		case 0:
			trap_S_StartLocalSound(cgs.media.count1Sound, CHAN_ANNOUNCER);
			break;
		case 1:
			trap_S_StartLocalSound(cgs.media.count2Sound, CHAN_ANNOUNCER);
			break;
		case 2:
			trap_S_StartLocalSound(cgs.media.count3Sound, CHAN_ANNOUNCER);
			break;
		default:
			break;
		}
	}

	CG_Hud_DrawTextElement(HUD_COUNTDOWN, s);
}


/*
===================
CG_Hud_DrawCenterString

SuperHUD version of CG_DrawCenterString
===================
*/
static void CG_Hud_DrawCenterString( void ) {
	if (!cg.centerPrintTime) {
		return;
	}

	// only print if not expired
	if (cg.centerPrintTime + (1000 * cg_centertime.value) > cg.time) {
		CG_Hud_DrawTextElement(HUD_CENTERPRINT, cg.centerPrint);
	}
}


/*
=================
CG_Hud_DrawAmmoWarning

SuperHUD version of CG_DrawAmmoWarning
=================
*/
static void CG_Hud_DrawAmmoWarning() {
	const char *s;

	if (cg_drawAmmoWarning.integer == 0) {
		return;
	}

	if (!cg.lowAmmoWarning) {
		return;
	}

	if (cg.lowAmmoWarning == 2) {
		s = "OUT OF AMMO";
	} else {
		s = "LOW AMMO WARNING";
	}

	CG_Hud_DrawTextElement(HUD_AMMO_MESSAGE, s);
}


/*
=================
CG_Hud_DrawCrosshair

SuperHUD version of CG_DrawCrosshair
=================
*/
static void CG_Hud_DrawCrosshair() {
	float w, h;
	qhandle_t hShader;
	float f;
	float x, y;

	if (!cg_drawCrosshair.integer) {
		return;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	if (cg.renderingThirdPerson) {
		return;
	}

	// set color based on health
	if (cg_crosshairHealth.integer) {
		vec4_t hcolor;

		CG_ColorForHealth(hcolor);
		trap_R_SetColor(hcolor);
	} else {
		trap_R_SetColor(cg.xhairColor);	// Nemesis - OSP Crosshair colors
	}

	w = h = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.itemPickupBlendTime;
	if (f > 0 && f < ITEM_BLOB_TIME) {
		f /= ITEM_BLOB_TIME;
		w *= (1 + f);
		h *= (1 + f);
	}

	x = cg_crosshairX.integer;
	y = cg_crosshairY.integer;
	CG_AdjustFrom640(&x, &y, &w, &h);

	hShader = cgs.media.crosshairShader[cg_drawCrosshair.integer % NUM_CROSSHAIRS];

	trap_R_DrawStretchPic(x + cg.refdef.x + 0.5 * (cg.refdef.width - w), 
		y + cg.refdef.y + 0.5 * (cg.refdef.height - h), 
		w, h, 0, 0, 1, 1, hShader);
}


/*
=====================
CG_Hud_DrawCrosshairNames

SuperHUD version of CG_DrawCrosshairNames
=====================
*/
static void CG_Hud_DrawCrosshairNames() {
	float *color;
	char *name;
	clientInfo_t *ci;
	vec4_t drawColor;

	if (!cg_drawCrosshair.integer) {
		return;
	}
	if (!cg_drawCrosshairNames.integer) {
		return;
	}
	if (cg.renderingThirdPerson) {
		return;
	}

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	// draw the name of the player being looked at
	color = CG_FadeColor(cg.crosshairClientTime, 1000);
	if (!color) {
		trap_R_SetColor(NULL);
		return;
	}

	drawColor[0] = colorWhite[0];
	drawColor[1] = colorWhite[1];
	drawColor[2] = colorWhite[2];
	drawColor[3] = color[3] * 0.5f;

	ci = &cgs.clientinfo[ cg.crosshairClientNum ];
	name = ci->name;

	CG_Hud_DrawTextElement(HUD_CROSSHAIR_NAME, name);
}


/*
===================
CG_Hud_DrawHoldableItem

SuperHUD version of CG_DrawHoldableItem
===================
*/
void CG_Hud_DrawHoldableItem() { 
	int value;

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if (value) {
		CG_RegisterItemVisuals(value);
		CG_Hud_DrawIconElement(HUD_HOLDABLE_ICON, cg_items[value].icon);
	}
}


/*
===================
CG_Hud_DrawReward

SuperHUD version of CG_DrawReward

Exact replica of CG_DrawReward for now, and will probably stay like that too ...
===================
*/
void CG_Hud_DrawReward() { 
	float	*color;
	int		i, count;
	float	x, y;
	char	buf[32];

	if ( !cg_drawRewards.integer ) {
		return;
	}

	color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
	if ( !color ) {
		if (cg.rewardStack > 0) {
			for(i = 0; i < cg.rewardStack; i++) {
				cg.rewardSound[i] = cg.rewardSound[i+1];
				cg.rewardShader[i] = cg.rewardShader[i+1];
				cg.rewardCount[i] = cg.rewardCount[i+1];
			}
			cg.rewardTime = cg.time;
			cg.rewardStack--;
			color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
			trap_S_StartLocalSound(cg.rewardSound[0], CHAN_ANNOUNCER);
		} else {
			return;
		}
	}

	trap_R_SetColor( color );

	if ( cg.rewardCount[0] >= 10 ) {
		y = 56;
		x = 320 - ICON_SIZE/2;
		CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );
		Com_sprintf(buf, sizeof(buf), "%d", cg.rewardCount[0]);
		x = ( SCREEN_WIDTH - SMALLCHAR_WIDTH * CG_DrawStrlen( buf ) ) / 2;
		CG_DrawStringExt( x, y+ICON_SIZE, buf, color, qfalse, qtrue,
								SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
	}
	else {

		count = cg.rewardCount[0];

		y = 56;
		x = 320 - count * ICON_SIZE/2;
		for ( i = 0 ; i < count ; i++ ) {
			CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );
			x += ICON_SIZE;
		}
	}
	trap_R_SetColor( NULL );
}


/*
===============================================================================
LAGOMETER
===============================================================================
*/

#define	LAG_SAMPLES		128

typedef struct {
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;
} lagometer_t;

lagometer_t		hudLagometer;

/*
==============
CG_Hud_AddLagometerFrameInfo

SuperHUD version of CG_AddLagometerFrameInfo
==============
*/
void CG_Hud_AddLagometerFrameInfo() {
	int offset;

	offset = cg.time - cg.latestSnapshotTime;
	hudLagometer.frameSamples[hudLagometer.frameCount & ( LAG_SAMPLES - 1)] = offset;
	hudLagometer.frameCount++;
}

/*
==============
CG_Hud_AddLagometerSnapshotInfo

SuperHUD version of CG_AddLagometerSnapshotInfo
==============
*/
void CG_Hud_AddLagometerSnapshotInfo(snapshot_t *snap) {
	// dropped packet
	if (!snap) {
		hudLagometer.snapshotSamples[hudLagometer.snapshotCount & (LAG_SAMPLES - 1)] = -1;
		hudLagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	hudLagometer.snapshotSamples[hudLagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->ping;
	hudLagometer.snapshotFlags[hudLagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->snapFlags;
	hudLagometer.snapshotCount++;
}

/*
==============
CG_Hud_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_Hud_DrawDisconnect() {
	int cmdNum;
	usercmd_t cmd;
	const char *s;
	hudItem_t *hudItem;

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd(cmdNum, &cmd);
	if (cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time) {  // special check for map_restart
		return;
	}

	s = "Connection Interrupted";
	CG_Hud_DrawTextElement(HUD_CONNECTION_INTERRUPTED, s);

	// blink the icon
	if ((cg.time >> 9) & 1) {
		return;
	}

	hudItem = &hudItems[HUD_LAGOMETER];

	if (hudItem == NULL) {
		return;
	}

	CG_Hud_DrawIcon(
		hudItem->x, 
		hudItem->y, 
		hudItem->width, 
		hudItem->height,
		hudItem->width, 
		hudItem->height,
		hudItem->hPosition,
		hudItem->vPosition,
		trap_R_RegisterShader("gfx/2d/net.tga"));
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_Hud_DrawLagometer

SuperHUD version of CG_DrawLagometer
==============
*/
static void CG_Hud_DrawLagometer() {
	int a, x, y, width, height, i;
	float v;
	float ax, ay, aw, ah, mid, range;
	int color;
	float vscale;
	hudItem_t *hudItem;

	if (!cg_lagometer.integer/* || cgs.localServer */) {
		CG_Hud_DrawDisconnect();
		return;
	}

	hudItem = &hudItems[HUD_LAGOMETER];

	if (hudItem == NULL) {
		return;
	}

	x = hudItem->x;
	y = hudItem->y;
	width = hudItem->width;
	height = hudItem->height;

	trap_R_SetColor(NULL);
	CG_DrawPic(x, y, width, height, cgs.media.lagometerShader);

	ax = x;
	ay = y;
	aw = width;
	ah = height;
	CG_AdjustFrom640(&ax, &ay, &aw, &ah);

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for (a = 0 ; a < aw ; a++) {
		i = (hudLagometer.frameCount - 1 - a) & (LAG_SAMPLES - 1);
		v = hudLagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
			}
			if (v > range) {
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		} else if (v < 0) {
			if (color != 2) {
				color = 2;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_BLUE)]);
			}
			v = -v;
			if (v > range) {
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for (a = 0 ; a < aw ; a++) {
		i = (hudLagometer.snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
		v = hudLagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if (hudLagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED) {
				if (color != 5) {
					color = 5;	// YELLOW for rate delay
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_GREEN)]);
				}
			}
			v = v * vscale;
			if (v > range) {
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		} else if (v < 0) {
			if (color != 4) {
				color = 4;		// RED for dropped snapshots
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_RED)]);
			}
			trap_R_DrawStretchPic(ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}

	trap_R_SetColor(NULL);

	if (cg_nopredict.integer || cg_syncronousClients.integer) {
		CG_DrawBigString(ax, ay, "snc", 1.0);
	}

	CG_Hud_DrawDisconnect();
}


/*
=================
CG_Hud_Draw2D

SuperHUD version of CG_Draw2D
=================
*/
void CG_Hud_Draw2D() {
	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		CG_Hud_DrawCrosshair();
		CG_Hud_DrawCrosshairNames();
	} else {
		// don't draw any status if dead
		if (cg.snap->ps.stats[STAT_HEALTH] > 0) {
			CG_Hud_DrawStatusBar();
			CG_Hud_DrawAmmoWarning();
			CG_Hud_DrawWeaponListElement(HUD_WEAPON_LIST);
			CG_Hud_DrawPickupItem();
			CG_Hud_DrawPowerups();
			CG_Hud_DrawCrosshair();
			CG_Hud_DrawCrosshairNames();
			CG_Hud_DrawHoldableItem();
			CG_Hud_DrawReward();
		}

		// chat
		CG_Hud_DrawTeamInfo();
	}

	CG_Hud_DrawVote();

	if (cg.showScores != qtrue) {  // scoreboard not up
		CG_Hud_DrawLagometer();

		if (cg_drawFPS.integer) {
			CG_Hud_DrawFPS();
		}

		if (cg_drawTimer.integer) {
			CG_Hud_DrawTimer();
		}

		if (cg_drawSpeed.integer) {
			CG_Hud_DrawSpeed();
		}

		if (cg_drawSpeedBar.integer) {
			CG_Hud_DrawSpeedBar();
		}

		CG_Hud_DrawScores();

		CG_Hud_DrawTeamCount();

		if (cg_drawTeamOverlay.integer) {
			CG_Hud_DrawTeamOverlay();
		}

		CG_Hud_DrawTextElement(HUD_HUD_NAME, cg_hudFile.string);
	}

	if (!CG_Hud_DrawFollow()) {
		CG_Hud_DrawWarmup();
	}

	// don't draw center string if scoreboard is up
	if (!CG_DrawScoreboard()) {
		CG_Hud_DrawCenterString();
	}

	if (cg_hudDebug.integer) {  // draw pale grid every 10 pixels
		int i;
		vec4_t color = {1.0f, 1.0f, 1.0f, 0.5f};

		for (i = 0; i <= SCREEN_WIDTH; i+=10) {
			CG_FillRect(i, 0, 1, SCREEN_HEIGHT, color);
		}
		for (i = 0; i <= SCREEN_HEIGHT; i+=10) {
			CG_FillRect(0, i, SCREEN_WIDTH, 1, color);
		}
	}
}

//==================================================================================
