
#include "cg_local.h"

vec4_t	yellow1	= {1, 1, 0, 0.25f};
vec4_t	yellow2	= {1, 1, 0, 0.20f};


// Windowing system setup
void CG_windowInit(void)
{
	int i;

	cg.winHandler.numActiveWindows = 0;
	for(i=0; i<MAX_WINDOW_COUNT; i++) {
		cg.winHandler.window[i].inuse = qfalse;
	}
	CG_initStrings();
}

// String buffer handling
void CG_initStrings(void)
{
	int i;

	for(i=0; i<MAX_STRINGS; i++) {
		cg.aStringPool[i].fActive = qfalse;
		cg.aStringPool[i].str[0] = 0;
	}
}

qboolean CG_addString(cg_window_t *w, char *buf)
{
	int i;

	// Check if we're reusing the current buf
	if(w->lineText[w->lineCount] != NULL) {
		for(i=0; i<MAX_STRINGS; i++) {
			if(!cg.aStringPool[i].fActive) continue;

			if(w->lineText[w->lineCount] == (char *)&cg.aStringPool[i].str) {
				w->lineCount++;
				cg.aStringPool[i].fActive = qtrue;
				strcpy(cg.aStringPool[i].str, buf);

				return(qtrue);
			}
		}
	}

	for(i=0; i<MAX_STRINGS; i++) {
		if(!cg.aStringPool[i].fActive) {
			cg.aStringPool[i].fActive = qtrue;
			strcpy(cg.aStringPool[i].str, buf);
			w->lineText[w->lineCount++] = (char *)&cg.aStringPool[i].str;

			return(qtrue);
		}
	}

	return(qfalse);
}

void CG_printWindow(char *str)
{
	int pos = 0, pos2 = 0;
	char buf[MAX_STRING_CHARS];
	cg_window_t *w = cg.windowCurrent;

	if(w == NULL) return;

	// Silly logic for a strict format
	Q_strncpyz(buf, str, MAX_STRING_CHARS);
	while(buf[pos] > 0 && w->lineCount < MAX_WINDOW_LINES) {
		if(buf[pos] == '\n') {
			if(pos2 == pos) {
				if(!CG_addString(w, " ")) {
					return;
				}
			} else {
				buf[pos] = 0;
				if(!CG_addString(w, buf + pos2)) {
					return;
				}
			}
			pos2 = ++pos;
			continue;
		}
		pos++;
	}

	if(pos2 < pos) {
		CG_addString(w, buf + pos2);
	}
}

// Window stuct "constructor" with some common defaults
void CG_windowReset(cg_window_t *w, int fx, int startupLength)
{
	vec4_t colorGeneralBorder = {1, 1, 0, 0.5};
	vec4_t colorGeneralFill   = {1, 1, 0, 0.5};

	w->effects = fx;
	w->fontScaleX = 0.25;
	w->fontScaleY = 0.25;
	w->flashPeriod = 1000;
	w->flashMidpoint = w->flashPeriod / 2;
	w->id = WID_NONE;
	w->inuse = qtrue;
	w->lineCount = 0;
	w->state = (fx >= WFX_FADEIN) ? WSTATE_START : WSTATE_COMPLETE;
	w->targetTime = (startupLength > 0) ? startupLength : 0;
	w->time = trap_Milliseconds();
	w->x = 0;
	w->y = 0;

	memcpy(&w->colorBorder, &colorGeneralBorder, sizeof(vec4_t));
	memcpy(&w->colorBackground, &colorGeneralFill, sizeof(vec4_t));
}

// Reserve a window
cg_window_t *CG_windowAlloc(int fx, int startupLength)
{
	int i;
	cg_window_t *w;
	cg_windowHandler_t *wh = &cg.winHandler;

	if(wh->numActiveWindows == MAX_WINDOW_COUNT) return(NULL);

	for(i=0; i<MAX_WINDOW_COUNT; i++) {
		w = &wh->window[i];
		if(w->inuse == qfalse) {
			CG_windowReset(w, fx, startupLength);
			wh->activeWindows[wh->numActiveWindows++] = i;
			return(w);
		}
	}

	// Fail if we're a full airplane
	return(NULL);
}

void CG_IntermissionWindow( void ) {
	int	 w;
	vec4_t	none	= {0, 0, 0, 0};
	char	*s0, *s1, *s2, *s3;
	char	*str, *map, *host, *mod;
	const char *info;
	cg_window_t *sw = CG_windowAlloc(WFX_TEXTSIZING|WFX_FLASH|WFX_SCROLLLEFT, 250);

	cg.intermWindow = sw;
	if(sw == NULL) return;

	memset( &str, 0, sizeof(str) );

	// Yes I know this is a expensive call but at least it only happens at intermission.
	info = CG_ConfigString( CS_SERVERINFO );
	map = Info_ValueForKey( info, "mapname" );
	host = Info_ValueForKey( info, "sv_hostname" );

	if ( CG_isFreezePlus() ) {
		mod = "FreezePlus";
	} else if ( CG_isNoGhost() ) {
		mod = "NoGhost";
	} else if ( CG_isNemesis() ) {
		mod = "Nemesis";
	} else if ( CG_isOSP() ) {
		mod = "OSP";
	} else {
		// show the best we can
		mod = cgs.modName;
	}

	// Window specific
	sw->state = WSTATE_START;
	sw->id = WID_INTERM;
	sw->fontScaleX = 0.9f;
	sw->fontScaleY = 0.9f;
	sw->x = 640;
	sw->y = 430;

	s0 = va("Mapname: %s\n", map);
	s1 = va("Server: %s\n", host);
	s2 = va("Server Mod: %s\n", mod);
	s3 = va("Nemesis Ver: %s", MOD_VERSION);
	
	// Assume mod version is longest string then compare against host name
	// Added * 10 to adjust for character width and + 10 to account for white space.
	w = (CG_DrawStrlen( s3 ) * 10) + 10;
	if( ((CG_DrawStrlen( s1 ) * 10) + 10) > w ) {
		w = (CG_DrawStrlen( s1 ) * 10) + 10;
	}

	// Just in case the host string is obscenely long, truncate
	// to prevent overlapping stats window.(640-290=350, sw->x=350 is minimum we can use)
	if( w > 290 ) {
		w = 290;
	}

	sw->x -= w;

	//sw->flashPeriod = 700;
	//sw->flashMidpoint = sw->flashPeriod * 0.5f;
	// Use no colors period
	memcpy(&sw->colorBorder, none, sizeof(vec4_t));
	memcpy(&sw->colorBackground, none, sizeof(vec4_t));
	memcpy(&sw->colorBackground2, none, sizeof(vec4_t));

	cg.windowCurrent = sw;
	if( sw != NULL ) {
		CG_printWindow( va("%s%s%s%s", s0, s1, s2, s3) );
	}
}

void CG_statsWindow( qboolean interMission )
{
	int weaponCount;
	int kills, deaths, score;
	float efficiency;
	char *weaponName, *damageGivenStr;
	const char		*line = "^7.............................................";

	int i;
	cg_window_t *sw = CG_windowAlloc(WFX_TEXTSIZING|WFX_FLASH|WFX_SCROLLRIGHT, 350);

	cg.statsWindow = sw;
	if(sw == NULL) return;

	// work out how many weapons to display for positioning
	weaponCount = 0;
	for (i=WP_GAUNTLET; i <= WP_BFG; i++) {
		if (stats.weaponShots[i] != 0 || stats.weaponKills[i] != 0 || stats.weaponDeaths[i] != 0) {
			weaponCount++;
		}
	}

	// Window specific
	sw->state = WSTATE_START;
	sw->id = WID_ACC;
	sw->fontScaleX = 0.9f;
	sw->fontScaleY = 0.9f;
	sw->x = 10;
	if( interMission ) {
		sw->y = 390 - (weaponCount * (WINDOW_FONTHEIGHT+3) * sw->fontScaleY);
	} else {
		sw->y = 350 - (weaponCount * (WINDOW_FONTHEIGHT+3) * sw->fontScaleY);
	}

	sw->flashPeriod = 700;
	sw->flashMidpoint = sw->flashPeriod * 0.5f;
	memcpy(&sw->colorBackground, yellow2, sizeof(vec4_t));
	memcpy(&sw->colorBackground2, &yellow1, sizeof(vec4_t));

	// gather stats
	kills = 0;
	deaths = 0;
	for (i=WP_GAUNTLET; i <= WP_BFG; i++) {
		kills += stats.weaponKills[i];
		deaths += stats.weaponDeaths[i];
	}

	score = 0;
	for (i = 0 ; i < cg.numScores ; i++) {
		if (cg.scores[i].client == cg.clientNum) {
			score = cg.scores[i].score;  //kills - stats.suicides;
			break;
		}
	}

	if (kills == 0) {
		efficiency = 0.0f;
	} else {
		efficiency = kills * 100.0f / (kills + deaths + stats.suicides);
	}

	cg.windowCurrent = sw;
	CG_printWindow("   ^3Score   ^2Kills  ^1Deaths    ^1Suic    ^3Effcny");
	CG_printWindow(va("^7%8i%8i%8i%8i %5i.%i%i%%\n\n", score, kills, deaths, stats.suicides, 
		(int)efficiency,
		(int)(efficiency * 10) % 10,
		(int)(efficiency * 100) % 10));
	CG_printWindow(va("^5Weapon          ^3Accrcy  ^7Hits/Atts  ^2Klls  ^1Dths\n%s", line));

	for (i=WP_GAUNTLET; i <= WP_BFG; i++) {
		if (stats.weaponShots[i] == 0  && stats.weaponKills[i] == 0 && stats.weaponDeaths[i] == 0) {
			continue;
		}

		switch (i) {
			case WP_GAUNTLET:
				weaponName = "Gauntlet";
				break;
			case WP_MACHINEGUN:
				weaponName = "MachineGun";
				break;

			case WP_SHOTGUN:
				weaponName = "ShotGun";
				break;

			case WP_GRENADE_LAUNCHER:
				weaponName = "G.Launcher";
				break;

			case WP_ROCKET_LAUNCHER:
				weaponName = "R.Launcher";
				break;

			case WP_LIGHTNING:
				weaponName = "LightningGun";
				break;

			case WP_RAILGUN:
				weaponName = "RailGun";
				break;

			case WP_PLASMAGUN:
				weaponName = "PlasmaGun";
				break;

			case WP_BFG:
				weaponName = "BFG";
				break;

			default:
				weaponName = "Unknown";
				break;
		}

		if (stats.weaponHits[i] == 0) {
			efficiency = 0.0f;
		} else {
			efficiency = stats.weaponHits[i] * 100.0f / stats.weaponShots[i];
		}

		CG_printWindow(va("^7%-12s : ^3%3i.%i%i%%  ^7%4i/%-4i  ^2%4i  ^1%4i\n", 
			weaponName, 
			(int)efficiency,
			(int)(efficiency * 10) % 10,
			(int)(efficiency * 100) % 10, 
			stats.weaponHits[i], 
			stats.weaponShots[i], 
			stats.weaponKills[i], 
			stats.weaponDeaths[i]));
	}

	// damage given is reliable for everything but freezetag
	// FIXME: checkout F_PersistantSpectator for problem code
	if (CG_isFreezeTag()) {
		damageGivenStr = "  N/A";
	} else {
		damageGivenStr = va("%5i", stats.damageGiven);
	}

	CG_printWindow(va("\n^3Damage Given :^7%5s  ^2Armor :^7%5i ^2(^7%i ^1RA ^7%i ^3YA^2)", 
		damageGivenStr,
		stats.armorTotal, 
		stats.armorRA, 
		stats.armorYA));

	CG_printWindow(va("^3Damage Recvd :^7%5i  ^2Health:^7%5i ^2(^7%i ^5MH^2)", 
		stats.damageReceived, 
		stats.healthTotal, 
		stats.healthMH));
}

// Set the window width and height based on the windows text/font parameters
void CG_windowNormalizeOnText(cg_window_t *w)
{
	int i, tmp;

	if(w == NULL) return;

	w->w = 0;
	w->h = 0;

	w->fontWidth = w->fontScaleX * WINDOW_FONTWIDTH;
	w->fontHeight = w->fontScaleY * WINDOW_FONTHEIGHT;

	for(i=0; i<w->lineCount; i++) {
		tmp = CG_DrawStrlen((char*)w->lineText[i]) * w->fontWidth;

		if(tmp > w->w) {
			w->w = tmp;
		}
	}

	for(i=0; i<w->lineCount; i++) {
		w->lineHeight[i] = w->fontHeight;
		w->h += w->lineHeight[i] + 3;
	}

	// Border + margins
	w->w += 10;
	w->h += 3;

	// Set up bottom alignment
	if(w->x < 0) w->x += 640 - w->w;
	if(w->y < 0) w->y += 480 - w->h;
}



void CG_removeStrings(cg_window_t *w)
{
	int i, j;

	for(i=0; i<w->lineCount; i++) {
		char *ref = w->lineText[i];

		for(j=0; j<MAX_STRINGS; j++) {
			if(!cg.aStringPool[j].fActive) continue;

			if(ref == (char *)&cg.aStringPool[j].str) {
				w->lineText[i] = NULL;
				cg.aStringPool[j].fActive = qfalse;
				cg.aStringPool[j].str[0] = 0;

				break;
			}
		}
	}
}

// Free up a window reservation
void CG_windowFree(cg_window_t *w)
{
	int i, j;
	cg_windowHandler_t *wh = &cg.winHandler;

	if(w == NULL) return;

	if(w->effects >= WFX_FADEIN && w->state != WSTATE_OFF && w->inuse == qtrue) {
		w->state = WSTATE_SHUTDOWN;
		w->time = trap_Milliseconds();
		return;
	}

	for(i=0; i<wh->numActiveWindows; i++) {
		if(w == &wh->window[wh->activeWindows[i]]) {
			for(j=i; j<wh->numActiveWindows; j++) {
				if(j+1 < wh->numActiveWindows) {
					wh->activeWindows[j] = wh->activeWindows[j+1];
				}
			}

			w->id = WID_NONE;
			w->inuse = qfalse;
			w->state = WSTATE_OFF;

			CG_removeStrings(w);

			wh->numActiveWindows--;

			break;
		}
	}
}

void CG_windowCleanup(void)
{
	int i;
	cg_window_t *w;
	cg_windowHandler_t *wh = &cg.winHandler;

	for(i=0; i<wh->numActiveWindows; i++) {
		w = &wh->window[wh->activeWindows[i]];
		if(!w->inuse || w->state == WSTATE_OFF) {
			CG_windowFree(w);
			i--;
		}
	}
}

// Main window-drawing handler
void CG_windowDraw(void)
{
	int h, x, y, i, j, milli, t_offset, tmp;
	cg_window_t *w;
	qboolean fCleanup = qfalse;
	vec4_t *bg;
	vec4_t textColor, borderColor, bgColor;

	if(cg.winHandler.numActiveWindows == 0) {
		return;
	}

	milli = trap_Milliseconds();
	memcpy(textColor, colorWhite, sizeof(vec4_t));

	for(i=0; i<cg.winHandler.numActiveWindows; i++) {
		w = &cg.winHandler.window[cg.winHandler.activeWindows[i]];

		if(!w->inuse || w->state == WSTATE_OFF) {
			fCleanup = qtrue;
			continue;
		}

		if(w->effects & WFX_TEXTSIZING) {
			CG_windowNormalizeOnText(w);
			w->effects &= ~WFX_TEXTSIZING;
		}

		bg = ((w->effects & WFX_FLASH) && (milli % w->flashPeriod) > w->flashMidpoint) ? &w->colorBackground2 : &w->colorBackground;

		h = w->h;
		x = w->x;
		y = w->y;
		t_offset = milli - w->time;
		textColor[3] = 1.0f;
		memcpy(&borderColor, w->colorBorder, sizeof(vec4_t));
		memcpy(&bgColor, bg, sizeof(vec4_t));

		tmp = w->targetTime - t_offset;

		// Window started - FIXME: Are calculations correct?
		if(w->state == WSTATE_START) {
			// Scroll up
			if(w->effects & WFX_SCROLLUP) {
				if(tmp > 0) {
					y += (480 - y) * tmp / w->targetTime;
				} else {
					w->state = WSTATE_COMPLETE;
				}
				
				w->curY = y;
			}
			// Scroll down
			if(w->effects & WFX_SCROLLDOWN) {
				if(tmp > 0) {
					y -= (0 + y) * tmp / w->targetTime;
				} else {
					w->state = WSTATE_COMPLETE;
				}
				
				w->curY = y;
			}
			// Scroll right
			if(w->effects & WFX_SCROLLRIGHT) {
				if(tmp > 0) {
					x -= (640 - x) * tmp / w->targetTime;
				} else {
					w->state = WSTATE_COMPLETE;
				}
				
				w->curX = x;
			}
			// Scroll left
			if(w->effects & WFX_SCROLLLEFT) {
				if(tmp > 0) {
					x += (640 - x) * tmp / w->targetTime;
				} else {
					w->state = WSTATE_COMPLETE;
				}
				
				w->curX = x;
			}
			// Fade in
			if(w->effects & WFX_FADEIN) {
				if(tmp > 0) {
					textColor[3] = (float)((float)t_offset / (float)w->targetTime);
				} else {
					w->state = WSTATE_COMPLETE;
				}
			}
			// Window Shutdown FIXME: Are calculations correct?
		} else if(w->state == WSTATE_SHUTDOWN) {
			// Fade in
			if(w->effects & WFX_FADEIN) {
				if(tmp > 0) {
					textColor[3] -= (float)((float)t_offset / (float)w->targetTime);
				} else {
					textColor[3] = 0.0f;
					w->state = WSTATE_OFF;
				}
			}
			// Scroll up
			if(w->effects & WFX_SCROLLUP) {
				if(tmp > 0) y = w->curY - (480 + w->y) * t_offset / w->targetTime;
				if(tmp < 0 || y >= 480) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			}
			// Scroll right
			if(w->effects & WFX_SCROLLRIGHT) {
				if(tmp > 0) x = w->curX + (640 - w->x) * t_offset / w->targetTime;
				if(tmp < 0 || x >= 640) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			}
			// Scroll left
			if(w->effects & WFX_SCROLLLEFT) {
				if(tmp > 0) x = w->curX - (640 - w->x) * t_offset / w->targetTime;
				if(tmp < 0 || x >= 640) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			}
		}

		borderColor[3] *= textColor[3];
		bgColor[3] *= textColor[3];

		CG_FillRect(x, y, w->w, h, bgColor);
		CG_DrawRect(x, y, w->w, h, 1, borderColor);

		for(j=w->lineCount-1; j>=0; j--) {
			h -= (w->lineHeight[j] + 3);
			
			CG_DrawStringExt(x+5, y + h, (char*)w->lineText[j], textColor, qfalse, qtrue, w->fontWidth, w->fontHeight, 0);
		}
	}

	if(fCleanup) {
		CG_windowCleanup();
	}
}

