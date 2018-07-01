
#include "ui_local.h"


#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"	
#define ART_FIGHT0			"menu/art/load_0"
#define ART_FIGHT1			"menu/art/load_1"
#define ART_FRAMEL			"menu/art/frame2_l"
#define ART_FRAMER			"menu/art/frame1_r"

#define NEMESIS_VERTICAL_SPACING	15

#define NEMESIS_ENEMY_COLOR		10
#define NEMESIS_ACCURACY		11
#define NEMESIS_OSP				12
#define NEMESIS_RAIL_TRAIL		13
#define NEMESIS_XHAIR_COLOR		14
#define	NEMESIS_SPEED			15
#define	NEMESIS_SPEED_BAR		16
#define NEMESIS_ICE				17
#define NEMESIS_XHAIR_STATS		18
#define	NEMESIS_TEAM_ICONS		19
#define NEMESIS_SCREEN_SHOT		20
#define NEMESIS_SCORE_PLUMS		21
#define NEMESIS_BACK			22

typedef struct {
	menuframework_s	menu;

	menutext_s			banner;
	menubitmap_s		framel;
	menubitmap_s		framer;
	menubitmap_s		back;

	menulist_s			enemycolor;
	menuradiobutton_s	accuracy;
	menuradiobutton_s	ospboard;
	menulist_s			railtrail;
	menulist_s			xhaircolor;
	menuradiobutton_s	speed;
	menuradiobutton_s	speedbar;
	menuradiobutton_s	ice;
	menulist_s			xhairstats;
	menuradiobutton_s	teamicons;
	menuradiobutton_s	screenshot;
	menuradiobutton_s	scoreplums;

} nemesis_t;

static nemesis_t	nemesis;

static void Nemesis_SetMenuItems( void ) {
	nemesis.enemycolor.curvalue		= (int)Com_Clamp( 0, 10, trap_Cvar_VariableValue( "cg_enemyColor" ) );
	nemesis.accuracy.curvalue		= (int)trap_Cvar_VariableValue( "cg_drawAcc" ) != 0;
	nemesis.ospboard.curvalue		= (int)trap_Cvar_VariableValue( "cg_scoreboardType" ) != 0;
	nemesis.railtrail.curvalue		= (int)Com_Clamp( 0, 6, trap_Cvar_VariableValue( "cg_railTrail" ) );
	nemesis.xhaircolor.curvalue		= (int)Com_Clamp( 0, 10, trap_Cvar_VariableValue( "cg_crosshairColor" ) );
	nemesis.speed.curvalue			= (int)trap_Cvar_VariableValue( "cg_drawSpeed" ) != 0;
	nemesis.speedbar.curvalue		= (int)trap_Cvar_VariableValue( "cg_drawSpeedBar" ) != 0;
	nemesis.ice.curvalue			= (int)trap_Cvar_VariableValue( "cg_iceShells" ) != 0;
	nemesis.xhairstats.curvalue		= (int)Com_Clamp( 0, 4, trap_Cvar_VariableValue( "cg_teamInfoType" ) );
	nemesis.teamicons.curvalue		= (int)trap_Cvar_VariableValue( "cg_useTeamIcons" ) != 0;
	nemesis.screenshot.curvalue		= (int)trap_Cvar_VariableValue( "cg_autoScreenShot" ) != 0;
	nemesis.scoreplums.curvalue		= (int)trap_Cvar_VariableValue( "cg_scorePlum" ) != 0;
}

static const char *enemycolor_names[] =
{
	"off",
	"red",
	"green",
	"yellow",
	"blue",
	"cyan",
	"magenta",
	"white",
	"orange",
	0
};

static const char *rail_trails[] =
{
	"",
	"Default",
	"AF Style",
	"Q2 Style",
	"QFu Style",
	0
};

static const char *xhair_stats[] =
{
	"off",
	"Text",
	"Gauges",
	0
};

static void Nemesis_Event( void* ptr, int notification ) {
	if( notification != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case NEMESIS_ENEMY_COLOR:
		trap_Cvar_SetValue( "cg_enemyColor", nemesis.enemycolor.curvalue );
		if( nemesis.enemycolor.curvalue == 0 ) {
			trap_Cvar_Set( "cg_enemyColor", "" );
		}
		break;

	case NEMESIS_ACCURACY:
		trap_Cvar_SetValue( "cg_drawAcc", nemesis.accuracy.curvalue );
		break;

	case NEMESIS_OSP:
		trap_Cvar_SetValue( "cg_scoreboardType", nemesis.ospboard.curvalue );
		break;

	case NEMESIS_RAIL_TRAIL:
		trap_Cvar_SetValue( "cg_railTrail", nemesis.railtrail.curvalue );
		if( nemesis.railtrail.curvalue == 0 ) {
			trap_Cvar_SetValue( "cg_railTrail", 1 );
		}
		break;

	case NEMESIS_XHAIR_COLOR:
		trap_Cvar_SetValue( "cg_crosshairColor", nemesis.xhaircolor.curvalue );
		if( nemesis.xhaircolor.curvalue != 0 ) {
			trap_Cvar_SetValue( "cg_crosshairHealth", 0 );
		} else {
			trap_Cvar_Set( "cg_crosshairColor", "" );
			trap_Cvar_SetValue( "cg_crosshairHealth", 1 );
		}
		break;

	case NEMESIS_SPEED:
		trap_Cvar_SetValue( "cg_drawSpeed", nemesis.speed.curvalue );
		break;

	case NEMESIS_SPEED_BAR:
		trap_Cvar_SetValue( "cg_drawSpeedBar", nemesis.speedbar.curvalue );
		break;

	case NEMESIS_ICE:
		trap_Cvar_SetValue( "cg_iceShells", nemesis.ice.curvalue );
		break;

	case NEMESIS_XHAIR_STATS:
		trap_Cvar_SetValue( "cg_teamInfoType", nemesis.xhairstats.curvalue );
		break;

	case NEMESIS_TEAM_ICONS:
		trap_Cvar_SetValue( "cg_useTeamIcons", nemesis.teamicons.curvalue );
		break;

	case NEMESIS_SCREEN_SHOT:
		trap_Cvar_SetValue( "cg_autoScreenShot", nemesis.screenshot.curvalue );
		break;
	case NEMESIS_SCORE_PLUMS:
		trap_Cvar_SetValue( "cg_autoScreenShot", nemesis.scoreplums.curvalue );
		break;

	case NEMESIS_BACK:
		UI_PopMenu();
		break;
	}
}

/*
===============
UI_Nemesis_MenuInit
===============
*/
static void UI_Nemesis_MenuInit( void ) {
	int		x, y;

	UI_ModsMenu_Cache();

	memset( &nemesis, 0 ,sizeof(nemesis_t) );
	nemesis.menu.wrapAround = qtrue;
	nemesis.menu.fullscreen = qtrue;

	x = 340;
	y = 150;

	nemesis.banner.generic.type			= MTYPE_BTEXT;
	nemesis.banner.generic.x			= 320;
	nemesis.banner.generic.y			= 16;
	nemesis.banner.string				= "NEMESIS SETUP";
	nemesis.banner.color				= color_white;
	nemesis.banner.style				= UI_CENTER;

	nemesis.framel.generic.type			= MTYPE_BITMAP;
	nemesis.framel.generic.name			= ART_FRAMEL;
	nemesis.framel.generic.flags		= QMF_INACTIVE;
	nemesis.framel.generic.x			= 0;  
	nemesis.framel.generic.y			= 78;
	nemesis.framel.width				= 256;
	nemesis.framel.height				= 329;

	nemesis.framer.generic.type			= MTYPE_BITMAP;
	nemesis.framer.generic.name			= ART_FRAMER;
	nemesis.framer.generic.flags		= QMF_INACTIVE;
	nemesis.framer.generic.x			= 376;
	nemesis.framer.generic.y			= 76;
	nemesis.framer.width				= 256;
	nemesis.framer.height				= 334;

	nemesis.back.generic.type			= MTYPE_BITMAP;
	nemesis.back.generic.name			= ART_BACK0;
	nemesis.back.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	nemesis.back.generic.id				= NEMESIS_BACK;
	nemesis.back.generic.callback		= Nemesis_Event;
	nemesis.back.generic.x				= 0;
	nemesis.back.generic.y				= 480-64;
	nemesis.back.width					= 128;
	nemesis.back.height					= 64;
	nemesis.back.focuspic				= ART_BACK1;

	nemesis.enemycolor.generic.type		= MTYPE_SPINCONTROL;
	nemesis.enemycolor.generic.name		= "Enemy Color:";
	nemesis.enemycolor.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.enemycolor.generic.callback	= Nemesis_Event;
	nemesis.enemycolor.generic.id		= NEMESIS_ENEMY_COLOR;
	nemesis.enemycolor.generic.x		= x;
	nemesis.enemycolor.generic.y		= y;
	nemesis.enemycolor.itemnames		= enemycolor_names;

	y += NEMESIS_VERTICAL_SPACING;
	nemesis.accuracy.generic.type        = MTYPE_RADIOBUTTON;
	nemesis.accuracy.generic.name		= "Display Accuracy:";
	nemesis.accuracy.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.accuracy.generic.callback	= Nemesis_Event;
	nemesis.accuracy.generic.id			= NEMESIS_ACCURACY;
	nemesis.accuracy.generic.x			= x;
	nemesis.accuracy.generic.y			= y;

	y += NEMESIS_VERTICAL_SPACING;
	nemesis.ospboard.generic.type		= MTYPE_RADIOBUTTON;
	nemesis.ospboard.generic.name		= "OSP ScoreBoard:";
	nemesis.ospboard.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.ospboard.generic.callback	= Nemesis_Event;
	nemesis.ospboard.generic.id			= NEMESIS_OSP;
	nemesis.ospboard.generic.x			= x;
	nemesis.ospboard.generic.y			= y;

	y += NEMESIS_VERTICAL_SPACING;
	nemesis.railtrail.generic.type		= MTYPE_SPINCONTROL;
	nemesis.railtrail.generic.name		= "Rail Trail:";
	nemesis.railtrail.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.railtrail.generic.callback	= Nemesis_Event;
	nemesis.railtrail.generic.id		= NEMESIS_RAIL_TRAIL;
	nemesis.railtrail.generic.x			= x;
	nemesis.railtrail.generic.y			= y;
	nemesis.railtrail.itemnames			= rail_trails;

	y += NEMESIS_VERTICAL_SPACING;
	nemesis.xhaircolor.generic.type		= MTYPE_SPINCONTROL;
	nemesis.xhaircolor.generic.name		= "Cross Hair Color:";
	nemesis.xhaircolor.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.xhaircolor.generic.callback	= Nemesis_Event;
	nemesis.xhaircolor.generic.id		= NEMESIS_XHAIR_COLOR;
	nemesis.xhaircolor.generic.x		= x;
	nemesis.xhaircolor.generic.y		= y;
	nemesis.xhaircolor.itemnames		= enemycolor_names;

	y += NEMESIS_VERTICAL_SPACING;
	nemesis.speed.generic.type			= MTYPE_RADIOBUTTON;
	nemesis.speed.generic.name			= "Draw Speed:";
	nemesis.speed.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.speed.generic.callback		= Nemesis_Event;
	nemesis.speed.generic.id			= NEMESIS_SPEED;
	nemesis.speed.generic.x				= x;
	nemesis.speed.generic.y				= y;

	y += NEMESIS_VERTICAL_SPACING;
	nemesis.speedbar.generic.type		= MTYPE_RADIOBUTTON;
	nemesis.speedbar.generic.name		= "Draw Speed Bar:";
	nemesis.speedbar.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.speedbar.generic.callback	= Nemesis_Event;
	nemesis.speedbar.generic.id			= NEMESIS_SPEED_BAR;
	nemesis.speedbar.generic.x			= x;
	nemesis.speedbar.generic.y			= y;

	y += NEMESIS_VERTICAL_SPACING;
	nemesis.ice.generic.type			= MTYPE_RADIOBUTTON;
	nemesis.ice.generic.name			= "Ice Shells:";
	nemesis.ice.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.ice.generic.callback		= Nemesis_Event;
	nemesis.ice.generic.id				= NEMESIS_ICE;
	nemesis.ice.generic.x				= x;
	nemesis.ice.generic.y				= y;

	y += NEMESIS_VERTICAL_SPACING;
	nemesis.xhairstats.generic.type		= MTYPE_SPINCONTROL;
	nemesis.xhairstats.generic.name		= "Team Crosshair Stats:";
	nemesis.xhairstats.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.xhairstats.generic.callback	= Nemesis_Event;
	nemesis.xhairstats.generic.id		= NEMESIS_XHAIR_STATS;
	nemesis.xhairstats.generic.x		= x;
	nemesis.xhairstats.generic.y		= y;
	nemesis.xhairstats.itemnames		= xhair_stats;

	y += NEMESIS_VERTICAL_SPACING;
	nemesis.teamicons.generic.type		= MTYPE_RADIOBUTTON;
	nemesis.teamicons.generic.name		= "Team Icons:";
	nemesis.teamicons.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.teamicons.generic.callback	= Nemesis_Event;
	nemesis.teamicons.generic.id		= NEMESIS_TEAM_ICONS;
	nemesis.teamicons.generic.x			= x;
	nemesis.teamicons.generic.y			= y;

	y += NEMESIS_VERTICAL_SPACING;
	nemesis.screenshot.generic.type		= MTYPE_RADIOBUTTON;
	nemesis.screenshot.generic.name		= "Auto ScreenShot:";
	nemesis.screenshot.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.screenshot.generic.callback	= Nemesis_Event;
	nemesis.screenshot.generic.id		= NEMESIS_SCREEN_SHOT;
	nemesis.screenshot.generic.x		= x;
	nemesis.screenshot.generic.y		= y;

	y += NEMESIS_VERTICAL_SPACING;
	nemesis.scoreplums.generic.type		= MTYPE_RADIOBUTTON;
	nemesis.scoreplums.generic.name		= "Score Plums:";
	nemesis.scoreplums.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	nemesis.scoreplums.generic.callback	= Nemesis_Event;
	nemesis.scoreplums.generic.id		= NEMESIS_SCORE_PLUMS;
	nemesis.scoreplums.generic.x		= x;
	nemesis.scoreplums.generic.y		= y;

	Menu_AddItem( &nemesis.menu, &nemesis.banner );
	Menu_AddItem( &nemesis.menu, &nemesis.framel );
	Menu_AddItem( &nemesis.menu, &nemesis.framer );
	Menu_AddItem( &nemesis.menu, &nemesis.back );
	Menu_AddItem( &nemesis.menu, &nemesis.enemycolor );
	Menu_AddItem( &nemesis.menu, &nemesis.accuracy );
	Menu_AddItem( &nemesis.menu, &nemesis.ospboard );
	Menu_AddItem( &nemesis.menu, &nemesis.railtrail );
	Menu_AddItem( &nemesis.menu, &nemesis.xhaircolor );
	Menu_AddItem( &nemesis.menu, &nemesis.speed );
	Menu_AddItem( &nemesis.menu, &nemesis.speedbar );
	Menu_AddItem( &nemesis.menu, &nemesis.ice );
	Menu_AddItem( &nemesis.menu, &nemesis.xhairstats );
	Menu_AddItem( &nemesis.menu, &nemesis.teamicons );
	Menu_AddItem( &nemesis.menu, &nemesis.screenshot );
	Menu_AddItem( &nemesis.menu, &nemesis.scoreplums );

	Nemesis_SetMenuItems();
}

/*
===============
UI_NemesisMenu
===============
*/
void UI_NemesisMenu( void ) {
	UI_Nemesis_MenuInit();
	UI_PushMenu( &nemesis.menu );
}
