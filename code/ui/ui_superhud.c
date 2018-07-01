
#include "ui_local.h"


#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"	
#define ART_FIGHT0			"menu/art/load_0"
#define ART_FIGHT1			"menu/art/load_1"
#define ART_FRAMEL			"menu/art/frame2_l"
#define ART_FRAMER			"menu/art/frame1_r"

#define MAX_CONFIGS			128
#define NAMEBUFSIZE			( MAX_CONFIGS * 16 )

#define SUPERHUD_CPMA		"hud/hud_shot/cpma"
#define SUPERHUD_NOPIC		"hud/hud_shot/nopic"
#define SUPERHUD_Q3A		"hud/hud_shot/q3a_default"

#define SUPERHUD_TYPE		10
#define SUPERHUD_CONFIG		11
#define SUPERHUD_BACK		12
#define SUPERHUD_PICTURE	13

typedef struct {
	menuframework_s	menu;

	menutext_s			banner;
	menubitmap_s		framel;
	menubitmap_s		framer;
	menubitmap_s		back;

	menulist_s			hudtype;
	menulist_s			hudconfig;
	menubitmap_s		hudPic;

	char			names[NAMEBUFSIZE];
	char*			configlist[MAX_CONFIGS];

} superHud_t;

static superHud_t	superHud;
static char hudFile[MAX_QPATH];

static void SuperHud_SetMenuItems( void ) {
	int i;
	superHud.hudtype.curvalue		= (int)Com_Clamp( 0, 3, trap_Cvar_VariableValue( "cg_hudType" ) );

	strcpy( hudFile, UI_Cvar_VariableString( "cg_hudFile" ));

	// Get current config in use
	for( i = 0; i < superHud.hudconfig.numitems; i++ ) {
		if(Q_stricmp( superHud.hudconfig.itemnames[i], hudFile) == 0) {
			superHud.hudconfig.curvalue = i;
		}
	}
	superHud.hudconfig.itemnames[superHud.hudconfig.curvalue] = hudFile;
}

static const char *hudtype_list[] =
{
	"Q3A Default",
	"CPMA Style",
	"Super HUD",
	0
};


static void SuperHud_Event( void* ptr, int notification ) {
	if( notification != QM_ACTIVATED ) {
		return;
	}


	switch( ((menucommon_s*)ptr)->id ) {
	case SUPERHUD_TYPE:
		trap_Cvar_SetValue( "cg_hudType", superHud.hudtype.curvalue );
		if( superHud.hudtype.curvalue == 0 ) {
			trap_Cvar_Set( "cg_hudType", "0" );
		}
		// Disable hud cfg selection if not set to superhud type
		if( superHud.hudtype.curvalue != 2 ) {
			superHud.hudconfig.generic.flags	|= QMF_GRAYED;
		} else {
			superHud.hudconfig.generic.flags	&= ~QMF_GRAYED;
		}
		break;

	case SUPERHUD_CONFIG:
		trap_Cvar_Set( "cg_hudFile", superHud.hudconfig.itemnames[superHud.hudconfig.curvalue] );
		break;

	case SUPERHUD_BACK:
		UI_PopMenu();
		break;
	}
}

static void UI_SuperHud_DrawHudPic( void *self ) {
	menubitmap_s	*b;
	char			hudName[64];
	int				x;
	int				y;
	int				w;
	int				h;
	int				i = 0;

	b = (menubitmap_s *)self;

	strcpy(hudName, superHud.hudconfig.itemnames[superHud.hudconfig.curvalue]);

	while( hudName[i] != '.' ) {
		i++;
	}
	hudName[i] = '\0';

	if( superHud.hudtype.curvalue == 0 ) {
		b->shader = trap_R_RegisterShaderNoMip( SUPERHUD_Q3A );
	} else if( superHud.hudtype.curvalue == 1 ) {
		b->shader = trap_R_RegisterShaderNoMip( SUPERHUD_CPMA );
	} else {
		b->shader = trap_R_RegisterShaderNoMip( va("hud/hud_shot/%s", hudName));
	}

	if(!b->shader && b->errorpic) {
		b->shader = trap_R_RegisterShaderNoMip( b->errorpic );
	}
	
	x = b->generic.x - 2;
	y = b->generic.y - 2;
	UI_FillRect( x, y, b->width + 4, 164, colorRed );

	x = b->generic.x;
	y = b->generic.y;
	w = b->width;
	h =	b->height;
	if( b->shader ) {
		UI_DrawHandlePic( x, y, w, h, b->shader );
	}
}

/*
===============
UI_SuperHud_MenuInit
===============
*/
static void UI_SuperHud_MenuInit( void ) {
	int		i, len, x, y;
	char	*configname;

	UI_ModsMenu_Cache();

	memset( &superHud, 0 ,sizeof(superHud_t) );
	superHud.menu.wrapAround = qtrue;
	superHud.menu.fullscreen = qtrue;

	x = 175;
	y = 150;

	superHud.banner.generic.type		= MTYPE_BTEXT;
	superHud.banner.generic.x			= 320;
	superHud.banner.generic.y			= 16;
	superHud.banner.string				= "NEMESIS HUD SETUP";
	superHud.banner.color				= color_white;
	superHud.banner.style				= UI_CENTER;

	superHud.framel.generic.type		= MTYPE_BITMAP;
	superHud.framel.generic.name		= ART_FRAMEL;
	superHud.framel.generic.flags		= QMF_INACTIVE;
	superHud.framel.generic.x			= 0;  
	superHud.framel.generic.y			= 78;
	superHud.framel.width				= 256;
	superHud.framel.height				= 329;

	superHud.framer.generic.type		= MTYPE_BITMAP;
	superHud.framer.generic.name		= ART_FRAMER;
	superHud.framer.generic.flags		= QMF_INACTIVE;
	superHud.framer.generic.x			= 376;
	superHud.framer.generic.y			= 76;
	superHud.framer.width				= 256;
	superHud.framer.height				= 334;

	superHud.back.generic.type			= MTYPE_BITMAP;
	superHud.back.generic.name			= ART_BACK0;
	superHud.back.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	superHud.back.generic.id			= SUPERHUD_BACK;
	superHud.back.generic.callback		= SuperHud_Event;
	superHud.back.generic.x				= 0;
	superHud.back.generic.y				= 480-64;
	superHud.back.width					= 128;
	superHud.back.height				= 64;
	superHud.back.focuspic				= ART_BACK1;

	superHud.hudtype.generic.type		= MTYPE_SPINCONTROL;
	superHud.hudtype.generic.name		= "HUD Type:";
	superHud.hudtype.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	superHud.hudtype.generic.callback	= SuperHud_Event;
	superHud.hudtype.generic.id			= SUPERHUD_TYPE;
	superHud.hudtype.generic.x			= x;
	superHud.hudtype.generic.y			= y + 30;
	superHud.hudtype.itemnames			= hudtype_list;

	superHud.hudconfig.generic.type		= MTYPE_SPINCONTROL;
	superHud.hudconfig.generic.name		= "HUD Config:";
	superHud.hudconfig.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	superHud.hudconfig.generic.callback	= SuperHud_Event;
	superHud.hudconfig.generic.id		= SUPERHUD_CONFIG;
	superHud.hudconfig.generic.x		= x;
	superHud.hudconfig.generic.y		= y + 45;
	superHud.hudconfig.numitems			= trap_FS_GetFileList( "hud", "cfg", superHud.names, NAMEBUFSIZE );
	superHud.hudconfig.itemnames		= (const char **)superHud.configlist;

	superHud.hudPic.generic.type		= MTYPE_BITMAP;
	superHud.hudPic.generic.flags		= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	superHud.hudPic.generic.x			= x + 150;
	superHud.hudPic.generic.y			= y;
	superHud.hudPic.generic.id			= SUPERHUD_PICTURE;
	superHud.hudPic.width				= 200;
	superHud.hudPic.height				= 160;
	superHud.hudPic.errorpic			= SUPERHUD_NOPIC;
	superHud.hudPic.generic.ownerdraw	= UI_SuperHud_DrawHudPic;


	if (!superHud.hudconfig.numitems) {
		strcpy(superHud.names,"No HUD Files Found.");
		superHud.hudconfig.numitems = 1;
	} else if (superHud.hudconfig.numitems > MAX_CONFIGS)
		superHud.hudconfig.numitems = MAX_CONFIGS;
	
	configname = superHud.names;

	for ( i = 0; i < superHud.hudconfig.numitems; i++ ) {
		superHud.hudconfig.itemnames[i] = configname;
		len = strlen( configname );
		configname += len + 1;
	}


	Menu_AddItem( &superHud.menu, &superHud.banner );
	Menu_AddItem( &superHud.menu, &superHud.framel );
	Menu_AddItem( &superHud.menu, &superHud.framer );
	Menu_AddItem( &superHud.menu, &superHud.back );
	Menu_AddItem( &superHud.menu, &superHud.hudtype );
	Menu_AddItem( &superHud.menu, &superHud.hudconfig );
	Menu_AddItem( &superHud.menu, &superHud.hudPic );

	SuperHud_SetMenuItems();
}

/*
===============
UI_NemesisMenu
===============
*/
void UI_SuperHudMenu( void ) {
	UI_SuperHud_MenuInit();
	// Disable hud selection if not set to superhud type
	if( superHud.hudtype.curvalue != 2 ) {
		superHud.hudconfig.generic.flags	|= QMF_GRAYED;
	} else {
		superHud.hudconfig.generic.flags	&= ~QMF_GRAYED;
	}
	UI_PushMenu( &superHud.menu );
}
