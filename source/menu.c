/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2010-2014 QuakeSpasm developers

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "quakedef.h"
#include "bgmusic.h"
#include <dirent.h>

#ifdef VITA
#include <psp2/io/fcntl.h> 
#endif

#ifdef VITA
#include <psp2/touch.h>
#endif // VITA

// anti-enum propaganda
#define OPT_CSETTING_LSENS 		420
#define OPT_CSETTING_LACC 		421
#define OPT_GSETTING_MAXFPS 	422
#define OPT_GSETTING_FOV 		423
#define OPT_GSETTING_GAMMA 		424
#define OPT_CSETTING_GSEX 		425
#define OPT_CSETTING_GSEY 		426
// end propagating

#ifdef VITA
#define Draw_BgMenu() Draw_StretchPic(0, 0, menu_bk, vid.width, vid.height)
#else
#define Draw_BgMenu() Draw_StretchPic(0, vid.height * 0.5, menu_bk, vid.width/2, vid.height/2)
#endif

extern cvar_t	waypoint_mode;
extern cvar_t	in_aimassist;
extern cvar_t 	joy_invert;
extern cvar_t 	crosshair;
extern cvar_t 	scr_showfps;
extern cvar_t	scr_dynamic_fov;
extern cvar_t 	host_maxfps;
extern cvar_t 	r_fullbright;
extern cvar_t 	gl_texturemode;
extern cvar_t 	scr_fov;
extern cvar_t 	motioncam;
extern cvar_t 	gyromode;
extern cvar_t 	gyrosensx;
extern cvar_t 	gyrosensy;

cvar_t cl_enablereartouchpad = {"cl_enablereartouchpad", "0", CVAR_ARCHIVE};

extern int loadingScreen;
extern int ShowBlslogo;

extern char* loadname2;
extern char* loadnamespec;
extern qboolean loadscreeninit;

extern float cl_forwardspeed;

char* game_build_date;

qpic_t *menu_bk;
qpic_t *start_bk;
qpic_t *pause_bk;
qpic_t *social_badges;

void (*vid_menucmdfn)(void); //johnfitz
void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);

qboolean paused_hack;

typedef struct
{
	int 		occupied;
	int 	 	map_allow_game_settings;
	int 	 	map_use_thumbnail;
	char* 		map_name;
	char* 		map_name_pretty;
	char* 		map_desc_1;
	char* 		map_desc_2;
	char* 		map_desc_3;
	char* 		map_desc_4;
	char* 		map_desc_5;
	char* 		map_desc_6;
	char* 		map_desc_7;
	char* 		map_desc_8;
	char* 		map_author;
	char* 		map_thumbnail_path;
} usermap_t;

usermap_t custom_maps[50];

#define BASE_MAP_COUNT   4
char* base_maps [] = 
{
	"ndu",
	"nzp_warehouse",
	"nzp_warehouse2",
	"christmas_special"
};

enum m_state_e m_state;

int 	old_m_state;

void M_Start_Menu_f (void);

void M_Menu_Main_f (void);
	void M_Paused_Menu_f(void);
	void M_Menu_SinglePlayer_f (void);
		void M_Menu_Maps_f (void);
		void M_Menu_Restart_f (void);
		void M_Menu_Exit_f (void);
	void M_Menu_MultiPlayer_f (void);
		void M_Menu_Setup_f (void);
		void M_Menu_Net_f (void);
		void M_Menu_LanConfig_f (void);
		void M_Menu_GameOptions_f (void);
		void M_Menu_Search_f (void);
		void M_Menu_ServerList_f (void);
	void M_Menu_Achievement_f (void);
	void M_Menu_Options_f (void);
		void M_Menu_Keys_f (void);
		void M_Control_Settings_f (void);
		void M_Graphics_Settings_f (void);
		void M_Menu_Video_f (void);
	void M_Menu_Credits_f (void);
	void M_Menu_Quit_f (void);

void M_Main_Draw (void);
	void M_SinglePlayer_Draw (void);
		void M_Menu_Maps_Draw (void);
	void M_Achievement_Draw (void);
	void M_MultiPlayer_Draw (void);
		void M_Setup_Draw (void);
		void M_Net_Draw (void);
		void M_LanConfig_Draw (void);
		void M_GameOptions_Draw (void);
		void M_Search_Draw (void);
		void M_ServerList_Draw (void);
	void M_Options_Draw (void);
		void M_OSK_Draw (void);
		void M_Keys_Draw (void);
		void M_Control_Settings_Draw (void);
		void M_Graphics_Settings_Draw (void);
		void M_Video_Draw (void);
	void M_Credits_Draw (void);
	void M_Quit_Draw (void);

void M_Main_Key (int key);
	void M_SinglePlayer_Key (int key);
		void M_Load_Key (int key);
		void M_Save_Key (int key);
	void M_Achievement_Key (int key);
	void M_MultiPlayer_Key (int key);
		void M_Setup_Key (int key);
		void M_Net_Key (int key);
		void M_LanConfig_Key (int key);
		void M_GameOptions_Key (int key);
		void M_Search_Key (int key);
		void M_ServerList_Key (int key);
	void M_Options_Key (int key);
		void M_Keys_Key (int key);
		void M_Control_Settings_Key (int key);
		void M_Graphics_Settings_Key (int key);
		void M_Video_Key (int key);
	void M_Credits_Key (int key);
	void M_Quit_Key (int key);

qboolean	m_entersound;		// play after drawing a frame, so caching
								// won't disrupt the sound
qboolean	m_recursiveDraw;

enum m_state_e	m_return_state;
qboolean	m_return_onerror;
char		m_return_reason [32];

#define StartingGame	(m_multiplayer_cursor == 1)
#define JoiningGame		(m_multiplayer_cursor == 0)
#define	IPXConfig		(m_net_cursor == 0)
#define	TCPIPConfig		(m_net_cursor == 1)

#define LINE_HEIGHT 		2
#define LINE_COLOR 			14

void M_ConfigureNetSubsystem(void);

//
// Macros to make menu design for NX & VITA easier
//
int menu_offset_y;

#ifdef VITA

#define OFFSET_SPACING						19

#define MENU_INITVARS()						int y = 0; menu_offset_y = y + 70;
#define DRAW_HEADER(title) 					Draw_ColoredStringScale(10, y + 10, title, 1, 1, 1, 1, 4.0f);
#define DRAW_VERSIONSTRING()				Draw_ColoredStringScale(vid.width - getTextWidth(game_build_date, 2), y + 5, game_build_date, 1, 1, 1, 1, 2.0f);
#define DRAW_MENUOPTION(id, txt, cursor, divider) { \
	menu_offset_y += OFFSET_SPACING; \
	if (cursor == id) \
		Draw_ColoredStringScale(10, menu_offset_y, txt, 1, 0, 0, 1, 2.0f); \
	else \
		Draw_ColoredStringScale(10, menu_offset_y, txt, 1, 1, 1, 1, 2.0f); \
	if (divider == true)  { \
		menu_offset_y += OFFSET_SPACING + 4; \
		Draw_FillByColor(10, menu_offset_y, 325, 4, 220, 220, 220, 255); \
		menu_offset_y -= OFFSET_SPACING/3; \
	} \
}
#define DRAW_BLANKOPTION(txt, divider) 	{ \
	menu_offset_y += OFFSET_SPACING; \
	Draw_ColoredStringScale(10, menu_offset_y, txt, 0.5, 0.5, 0.5, 1, 2.0f); \
	if (divider == true)  { \
		menu_offset_y += OFFSET_SPACING + 4; \
		Draw_FillByColor(10, menu_offset_y, 325, 4, 220, 220, 220, 255); \
		menu_offset_y -= OFFSET_SPACING/3; \
	} \
}
#define DRAW_DESCRIPTION(txt) Draw_ColoredStringScale(10, y + 475, txt, 1, 1, 1, 1, 2.0f);
#define DRAW_BACKBUTTON(id, cursor) { \
	if (cursor == id) \
		Draw_ColoredStringScale(10, 500, "Back", 1, 0, 0, 1, 2.0f); \
	else \
		Draw_ColoredStringScale(10, 500, "Back", 1, 1, 1, 1, 2.0f); \
}
#define DRAW_MAPTHUMB(img) Draw_StretchPic(x_map_info_disp + 245, y + 68, img, 450, 255);
#define DRAW_MAPDESC(id, txt) Draw_ColoredStringScale(x_map_info_disp + 245, y + 329 + (18 * id), txt, 1, 1, 1, 1, 2.0f);
#define DRAW_MAPAUTHOR(id, txt) Draw_ColoredStringScale(x_map_info_disp + 245, y + 329 + (18 * id), txt, 1, 1, 0, 1, 2.0f);
#define DRAW_CREDITLINE(id, txt) Draw_ColoredStringScale(10, menu_offset_y + (OFFSET_SPACING * id), txt, 1, 1, 1, 1, 2.0f);
#define DRAW_SETTINGSVALUE(id, txt) Draw_ColoredStringScale(400, y + 70 + (OFFSET_SPACING * (id + 1)), txt, 1, 1, 1, 1, 2.0f);
#define DRAW_SLIDER(id, r) M_DrawSlider(408, y + 70 + (OFFSET_SPACING * (id + 1)), r, 2.0f);

#else

#define OFFSET_SPACING						15

#define MENU_INITVARS()						int y = vid.height * 0.5; menu_offset_y = y + 55;
#define DRAW_HEADER(title) 					Draw_ColoredStringScale(10, y + 10, title, 1, 1, 1, 1, 3.0f);
#define DRAW_VERSIONSTRING()				Draw_ColoredStringScale(635 - getTextWidth(game_build_date, 1), y + 10, game_build_date, 1, 1, 1, 1, 1.0f);
#define DRAW_MENUOPTION(id, txt, cursor, divider) { \
	menu_offset_y += OFFSET_SPACING; \
	if (cursor == id) \
		Draw_ColoredStringScale(10, menu_offset_y, txt, 1, 0, 0, 1, 1.5f); \
	else \
		Draw_ColoredStringScale(10, menu_offset_y, txt, 1, 1, 1, 1, 1.5f); \
	if (divider == true)  { \
		menu_offset_y += OFFSET_SPACING + 4; \
		Draw_FillByColor(10, menu_offset_y, 240, 3, 220, 220, 220, 255); \
		menu_offset_y -= OFFSET_SPACING/3; \
	} \
}
#define DRAW_BLANKOPTION(txt, divider) 	{ \
	menu_offset_y += OFFSET_SPACING; \
	Draw_ColoredStringScale(10, menu_offset_y, txt, 0.5, 0.5, 0.5, 1, 1.5f); \
	if (divider == true)  { \
		menu_offset_y += OFFSET_SPACING + 4; \
		Draw_FillByColor(10, menu_offset_y, 240, 3, 220, 220, 220, 255); \
		menu_offset_y -= OFFSET_SPACING/3; \
	} \
}
#define DRAW_DESCRIPTION(txt) Draw_ColoredStringScale(10, y + 305, txt, 1, 1, 1, 1, 1.5f);
#define DRAW_BACKBUTTON(id, cursor) { \
	if (cursor == id) \
		Draw_ColoredStringScale(10, y + 335, "Back", 1, 0, 0, 1, 1.5f); \
	else \
		Draw_ColoredStringScale(10, y + 335, "Back", 1, 1, 1, 1, 1.5f); \
}
#define DRAW_MAPTHUMB(img) Draw_StretchPic(x_map_info_disp + 280, y + 45, img, 280, 160);
#define DRAW_MAPDESC(id, txt) Draw_ColoredStringScale(x_map_info_disp + 280, y + 218 + (15 * id), txt, 1, 1, 1, 1, 1.45f);
#define DRAW_MAPAUTHOR(id, txt) Draw_ColoredStringScale(x_map_info_disp + 280, y + 218 + (15 * id), txt, 1, 1, 0, 1, 1.25f);
#define DRAW_CREDITLINE(id, txt) Draw_ColoredStringScale(10, menu_offset_y + ((OFFSET_SPACING - 2) * id), txt, 1, 1, 1, 1, 1.25f);
#define DRAW_SETTINGSVALUE(id, txt) Draw_ColoredStringScale(300, y + 55 + (OFFSET_SPACING * (id + 1)), txt, 1, 1, 1, 1, 1.5f);
#define DRAW_SLIDER(id, r) M_DrawSlider(308, y + 55 + (OFFSET_SPACING * (id + 1)), r, 1.0f);

#endif // VITA

/*
================
M_DrawCharacter

Draws one solid graphics character
================
*/
void M_DrawCharacter (int cx, int line, int num)
{
	Draw_Character (cx, line, num);
}

void M_Print (int cx, int cy, const char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, (*str)+128);
		str++;
		cx += 8;
	}
}

void M_PrintWhite (int cx, int cy, const char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, *str);
		str++;
		cx += 8;
	}
}

void M_DrawTransPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x, y, pic); //johnfitz -- simplified becuase centering is handled elsewhere
}

void M_DrawPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x, y, pic); //johnfitz -- simplified becuase centering is handled elsewhere
}

void M_DrawTransPicTranslate (int x, int y, qpic_t *pic, int top, int bottom) //johnfitz -- more parameters
{
	Draw_TransPicTranslate (x, y, pic, top, bottom); //johnfitz -- simplified becuase centering is handled elsewhere
}

void M_DrawTextBox (int x, int y, int width, int lines)
{
	qpic_t	*p;
	int		cx, cy;
	int		n;

	// draw left side
	cx = x;
	cy = y;
	p = Draw_CachePic ("gfx/box_tl.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_ml.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_bl.lmp");
	M_DrawTransPic (cx, cy+8, p);

	// draw middle
	cx += 8;
	while (width > 0)
	{
		cy = y;
		p = Draw_CachePic ("gfx/box_tm.lmp");
		M_DrawTransPic (cx, cy, p);
		p = Draw_CachePic ("gfx/box_mm.lmp");
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_CachePic ("gfx/box_mm2.lmp");
			M_DrawTransPic (cx, cy, p);
		}
		p = Draw_CachePic ("gfx/box_bm.lmp");
		M_DrawTransPic (cx, cy+8, p);
		width -= 2;
		cx += 16;
	}

	// draw right side
	cy = y;
	p = Draw_CachePic ("gfx/box_tr.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_mr.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_br.lmp");
	M_DrawTransPic (cx, cy+8, p);
}

//=============================================================================

#define MENU_MAIN                          0
#define MENU_SINGLEPLAYER                  1
#define MENU_DEFAULT                       2
#define MENU_PAUSE                         3
#define MENU_START                         4

// naievil: TODO -- fading cyclical background?

void Menu_Background_Draw (int type) {
	qpic_t *bg;
	int i;

	switch(type) {
		case MENU_MAIN:
			bg = Draw_CachePic("gfx/menu/menu_background.tga");
			break;
		default: 
			bg = Draw_CachePic("gfx/menu/menu_background.tga");
			break;
	}

	if (key_dest != key_menu_pause && old_m_state != m_paused_menu)
		Draw_StretchPic(0, vid.height * 0.5, bg, vid.width/2, vid.height/2);
}

//=============================================================================

int m_save_demonum;

/*
================
M_ToggleMenu_f
================
*/
void M_ToggleMenu_f (void)
{
	m_entersound = true;

	if (key_dest == key_menu)
	{
		if (m_state != m_main)
		{
			M_Menu_Main_f ();
			return;
		}

		//IN_Activate();
		key_dest = key_game;
		m_state = m_none;
		return;
	}
	if (key_dest == key_console)
	{
		Con_ToggleConsole_f ();
	}
	else if (sv.active && (svs.maxclients > 1 || key_dest == key_game))
	{
		M_Paused_Menu_f();
	}
	else
	{
		M_Menu_Main_f ();
	}
}

void M_Paused_Menu_f (void) {
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_paused_menu;
	m_entersound = true;
	
	loadingScreen = 0;
	loadscreeninit = false;
}

#define Max_Paused_Items 		4
int M_Paused_Cusor;

void M_Paused_Menu_Draw (void) {
	paused_hack = true;
	MENU_INITVARS();

	// Fill black to make everything easier to see
	Draw_FillByColor(0, 0, 1280, 720, 0, 0, 0, 0.4);

	// Header
	DRAW_HEADER("PAUSED");

	DRAW_MENUOPTION(0, "Resume Carnage", M_Paused_Cusor, false);
	DRAW_MENUOPTION(1, "Restart", M_Paused_Cusor, false);
	DRAW_MENUOPTION(2, "Settings", M_Paused_Cusor, false);
	DRAW_MENUOPTION(3, "End Game", M_Paused_Cusor, false);
}

void M_Paused_Menu_Key (int key)
{
	switch (key)
	{
	case K_BBUTTON:
	case K_ESCAPE:
		paused_hack = false;
		key_dest = key_game;
		m_state = m_none;
		break;

	case K_DOWNARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
        if (++M_Paused_Cusor >= Max_Paused_Items)
            M_Paused_Cusor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
        if (--M_Paused_Cusor < 0)
            M_Paused_Cusor = Max_Paused_Items - 1;
		break;

	case K_ENTER:
	case K_ABUTTON:
		m_entersound = true;

		switch (M_Paused_Cusor)
		{
		case 0:
			// Resume
			paused_hack = false;
			key_dest = key_game;
			m_state = m_none;
			break;
		case 1:
			// Restart
			M_Menu_Restart_f();
            break;
		case 2:
			// 
			M_Menu_Options_f();
			break;
		case 3:
			paused_hack = false;
			// This is supposed to be to exit the map
			M_Menu_Exit_f();
			break;
		}
	}
}


void M_Start_Menu_f ()
{
	key_dest = key_menu;
	m_state = m_start;
	m_entersound = true;
	loadingScreen = 0;
}


static void M_Start_Menu_Draw ()
{
	qpic_t	*start_bk;
	
	start_bk = Draw_CachePic("gfx/lscreen/lscreen.tga");
	Draw_StretchPic(0, 0, start_bk, vid.width, vid.height);
    // Use the useprint canvas because it's easier to draw things scaled well not in console 
	char *s = "Press start";
	GL_SetCanvas(CANVAS_USEPRINT);
 	Draw_String ((vid.width/2 - (strlen(s)*8))/2, vid.height * 0.85, s);
}
void M_Start_Key (int key)
{
	switch (key) {
		default:
		case K_ESCAPE:
			S_LocalSound ("sounds/menu/enter.wav");
			Cbuf_AddText("togglemenu\n");
			M_Menu_Main_f ();
	}
}

//=============================================================================
/* MAIN MENU */

int	m_main_cursor;

#ifdef VITA
#define	MAIN_ITEMS	5
#else
#define	MAIN_ITEMS	3
#endif


void M_Menu_Main_f (void)
{
	if (key_dest != key_menu)
	{
		m_save_demonum = cls.demonum;
		cls.demonum = -1;
	}
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_main;
	m_entersound = true;
	
#ifdef VITA
	void Load_Achivements(void);
	Load_Achivements();
#endif
}

void M_Main_Draw (void)
{
	MENU_INITVARS();

	// Social Badges
	social_badges = Draw_CachePic("gfx/menu/social.tga");

	// Menu Background
	menu_bk = Draw_CachePic("gfx/menu/menu_background.tga");
	Draw_BgMenu();

	// Fill black to make everything easier to see
	Draw_FillByColor(0, 0, 1280, 720, 0, 0, 0, 1);

	// Version String
	DRAW_VERSIONSTRING();

	// Header
	DRAW_HEADER("MAIN MENU");

	DRAW_MENUOPTION(0, "Solo", m_main_cursor, false);
	DRAW_BLANKOPTION("Co-Op (Coming Soon!)", true);
	DRAW_MENUOPTION(1, "Settings", m_main_cursor, false);

#ifdef VITA

	DRAW_MENUOPTION(2, "Achievements", m_main_cursor, true);
	DRAW_MENUOPTION(3, "Credits", m_main_cursor, true);
	DRAW_MENUOPTION(4, "Exit", m_main_cursor, false);

#else

	DRAW_BLANKOPTION("Achievements", true);
	DRAW_MENUOPTION(2, "Credits", m_main_cursor, true);
	//DRAW_MENUOPTION(3, "Exit", m_main_cursor, false);

#endif // VITA

	switch(m_main_cursor) {
		case 0: DRAW_DESCRIPTION("Take on the Hordes by yourself."); break;
		case 1: DRAW_DESCRIPTION("Adjust Control or Graphic Settings."); break;

#ifdef VITA

		case 2: DRAW_DESCRIPTION("View Locked/Unlocked Achievements."); break;
		case 3: DRAW_DESCRIPTION("View Credits for NZ:P."); break;
		case 4: DRAW_DESCRIPTION("Return to LiveArea."); break;

#else

		case 2: DRAW_DESCRIPTION("View Credits for NZ:P."); break;
		//case 3: DRAW_DESCRIPTION("Return to Horizon."); break;

#endif // VITA

		default: break;
	}

#ifdef VITA

	Draw_SubPic(915, 510, 26, 26, 32, 0, 64, 32, social_badges); // YouTube
	Draw_ColoredStringScale(848, 510 + 6, "@nzpteam", 1, 1, 0, 1, 1.0f);

	Draw_SubPic(915, 510 - 26 - 5, 26, 26, 0, 32, 32, 64, social_badges); // Twitter
	Draw_ColoredStringScale(840, 510 - 25, "/NZPTeam", 1, 1, 0, 1, 1.0f);

	Draw_SubPic(915, 510 - 52 - 10, 26, 26, 32, 32, 64, 64, social_badges); // Patreon
	Draw_ColoredStringScale(818, 510 - 52 - 3, "/cypressimplex", 1, 1, 0, 1, 1.0f);

#else

	Draw_SubPic(610, y + 330, 22, 22, 32, 0, 64, 32, social_badges); // YouTube
	Draw_ColoredStringScale(548, y + 337, "@nzpteam", 1, 1, 0, 1, 1.0f);

	Draw_SubPic(610, y + 302, 22, 22, 0, 32, 32, 64, social_badges); // Twitter
	Draw_ColoredStringScale(542, y + 309, "/NZPTeam", 1, 1, 0, 1, 1.0f);

	Draw_SubPic(610, y + 274, 22, 22, 32, 32, 64, 64, social_badges); // Patreon
	Draw_ColoredStringScale(520, y + 280, "/cypressimplex", 1, 1, 0, 1, 1.0f);

#endif // VITA

}


void M_Main_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		IN_Activate();
		key_dest = key_game;
		m_state = m_none;
		cls.demonum = m_save_demonum;
		if (!fitzmode)	/* QuakeSpasm customization: */
			break;
		if (cls.demonum != -1 && !cls.demoplayback && cls.state != ca_connected)
			CL_NextDemo ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		if (++m_main_cursor >= MAIN_ITEMS)
			m_main_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		if (--m_main_cursor < 0)
			m_main_cursor = MAIN_ITEMS - 1;
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		m_entersound = true;

		switch (m_main_cursor)
		{
		case 0:
			M_Menu_SinglePlayer_f ();
			break;

		case 1:
			M_Menu_Options_f ();
			break;
	#ifdef VITA
		case 2:
			M_Menu_Achievement_f ();
			break;
	#endif

	#ifdef VITA	
		case 3:
	#else
		case 2:
	#endif
			M_Menu_Credits_f ();
			break;
	#ifdef VITA
		case 4:
			M_Menu_Quit_f ();
			break;
	#endif
		}
	}
}

qboolean	wasInMenus;

#ifdef VITA

char *restartMessage [] =
{

  " Are you sure you want",
  "  to restart this game? ",  //msg:0
  "                               ",
  "   X :Yes    O : No       "
};

#else

char *restartMessage [] =
{

  " Are you sure you want",
  "  to restart this game? ",  //msg:0
  "                               ",
  "   A :Yes    B : No       "
};

#endif // VITA


void M_Menu_Restart_f (void)
{
	wasInMenus = (key_dest == key_menu_pause);
	key_dest = key_menu;
	m_state = m_restart;
	m_entersound = true;
}

extern int textstate;
void M_Restart_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		m_state = m_paused_menu;
		m_entersound = true;
		break;

	case K_ENTER:
	case K_ABUTTON:
		key_dest = key_game;
		m_state = m_none;
		paused_hack = false;
		//Cbuf_AddText ("restart\n");
		textstate = 0;
		PR_ExecuteProgram (pr_global_struct->Soft_Restart);
		break;
	default:
		break;
	}

}


void M_Restart_Draw (void)
{
	m_state = m_paused_menu;
	m_recursiveDraw = true;
	M_Draw ();
	m_state = m_restart;

	GL_SetCanvas(CANVAS_MENU);

	M_DrawTextBox (56, 76, 24, 4);
	M_Print (64, 84,  restartMessage[0]);
	M_Print (64, 92,  restartMessage[1]);
	M_Print (64, 100, restartMessage[2]);
	M_Print (64, 108, restartMessage[3]);
}




//=============================================================================
/* EXIT MENU */


char *exitMessage [] =
{

  "Are you sure you want to",
  " quit to the Main Menu? ",  //msg:0
  "                                  ",
  "   A :Yes    B : No       "
};


void M_Menu_Exit_f (void)
{
	wasInMenus = (key_dest == key_menu_pause);
	key_dest = key_menu;
	paused_hack = false;
	m_state = m_exit;
	m_entersound = true;
}


void M_Exit_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		m_state = m_paused_menu;
		m_entersound = true;
		break;

	case K_ENTER:
	case K_ABUTTON:
		Cbuf_AddText("disconnect\n");
		paused_hack = false;
		Cbuf_AddText("music tensioned_by_the_damned\n");
		Cbuf_AddText("music_loop 1\n");
		M_Menu_Main_f();
		break;

	default:
		break;
	}

}


void M_Exit_Draw (void)
{
	m_state = m_paused_menu;
	m_recursiveDraw = true;
	M_Draw ();
	m_state = m_exit;

	GL_SetCanvas(CANVAS_MENU);

	M_DrawTextBox (56, 76, 24, 4);
	M_Print (64, 84,  exitMessage[0]);
	M_Print (64, 92,  exitMessage[1]);
	M_Print (64, 100, exitMessage[2]);
	M_Print (64, 108, exitMessage[3]);
}

//=============================================================================
/* SINGLE PLAYER MENU */
#ifdef VITA
int x_map_info_disp = 200;
#else
int x_map_info_disp = 0;
#endif

int	m_singleplayer_cursor;
#define	SINGLEPLAYER_ITEMS	6


void M_Menu_SinglePlayer_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_singleplayer;
	m_entersound = true;
}

void M_SinglePlayer_Draw (void)
{
	qpic_t *menu_ndu 	= Draw_CachePic("gfx/menu/nacht_der_untoten.tga");
	//qpic_t *menu_kn 	= Draw_CachePic("gfx/menu/kino_der_toten.tga");
	qpic_t *menu_wh 	= Draw_CachePic("gfx/menu/nzp_warehouse.tga");
	qpic_t *menu_wh2 	= Draw_CachePic("gfx/menu/nzp_warehouse2.tga");
	//qpic_t* menu_wn 	= Draw_CachePic("gfx/menu/wahnsinn.tga");
	qpic_t* menu_ch 	= Draw_CachePic("gfx/menu/christmas_special.tga");
	qpic_t* menu_custom = Draw_CachePic("gfx/menu/custom.tga");

	MENU_INITVARS();
	paused_hack = false;

	// Menu Background
	Draw_BgMenu();

	// Fill black to make everything easier to see
	Draw_FillByColor(0, 0, 1280, 720, 0, 0, 0, 0.4);

	// Header
	DRAW_HEADER("SOLO");

	DRAW_MENUOPTION(0, "Nacht der Untoten", m_singleplayer_cursor, true);
	DRAW_MENUOPTION(1, "Warehouse", m_singleplayer_cursor, false);
	DRAW_MENUOPTION(2, "Warehouse (Classic)", m_singleplayer_cursor, false);
	DRAW_MENUOPTION(3, "Christmas Special", m_singleplayer_cursor, true);
	DRAW_MENUOPTION(4, "Custom Maps", m_singleplayer_cursor, false);
	DRAW_BACKBUTTON(5, m_singleplayer_cursor);

	// Map description & pic
	switch (m_singleplayer_cursor) {
		case 0:
			DRAW_MAPTHUMB(menu_ndu);
			DRAW_MAPDESC(0, "Desolate bunker located on a Ge-");
			DRAW_MAPDESC(1, "rman airfield, stranded after a");
			DRAW_MAPDESC(2, "brutal plane crash surrounded by");
			DRAW_MAPDESC(3, "hordes of undead. Exploit myste-");
			DRAW_MAPDESC(4, "rious forces at play and hold o-");
			DRAW_MAPDESC(5, "ut against relentless waves. Der");
			DRAW_MAPDESC(6, "Anstieg ist jetzt. Will you fall");
			DRAW_MAPDESC(7, "to the overwhelming onslaught?");
			break;
		case 1: 
			DRAW_MAPTHUMB(menu_wh2);
			DRAW_MAPDESC(0, "Four nameless marines find them-");
			DRAW_MAPDESC(1, "selves at a forsaken warehouse,");
			DRAW_MAPDESC(2, "or is it something more? Fight");
			DRAW_MAPDESC(3, "your way to uncovering its sec-");
			DRAW_MAPDESC(4, "rets, though you may not like");
			DRAW_MAPDESC(5, "what you find..");
			break;
		case 2: 
			DRAW_MAPTHUMB(menu_wh);
			DRAW_MAPDESC(0, "Old Warehouse full of Zombies!");
			DRAW_MAPDESC(1, "Fight your way to the Power");
			DRAW_MAPDESC(2, "Switch through the Hordes!");
			break;
		case 3: 
			DRAW_MAPTHUMB(menu_ch);
			DRAW_MAPDESC(0, "No Santa this year. Though we're");
			DRAW_MAPDESC(1, "sure you will get presents from");
			DRAW_MAPDESC(2, "the undead! Will you accept them?");
			break;
		case 4: 
			DRAW_MAPTHUMB(menu_custom);
			DRAW_MAPDESC(0, "Custom Maps made by Community");
			DRAW_MAPDESC(1, "Members on GitHub and the NZ:P");
			DRAW_MAPDESC(2, "Forum!");
			break;
		default: break;
	}
}


void M_SinglePlayer_Key (int key)
{
#ifdef VITA // For some reasons, clicking on "Solo" on Vita causes double inputs propagation
	static int fix_double_input = 0;
#endif
	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_Main_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		if (++m_singleplayer_cursor >= SINGLEPLAYER_ITEMS)
			m_singleplayer_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		if (--m_singleplayer_cursor < 0)
			m_singleplayer_cursor = SINGLEPLAYER_ITEMS - 1;
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		m_entersound = true;

		switch (m_singleplayer_cursor)
		{
		case 0:
			// Nacht Der Untoten
#ifdef VITA
			if (!fix_double_input) {
				fix_double_input++;
				break;
			}
#endif
			IN_Activate();
			key_dest = key_game;
			if (sv.active)
				Cbuf_AddText ("disconnect\n");

			Cbuf_AddText ("maxplayers 1\n");
			Cbuf_AddText ("deathmatch 0\n");
			Cbuf_AddText ("coop 0\n"); 
			Cbuf_AddText ("music_loop 0\n");
			Cbuf_AddText ("music_stop\n");
			Cbuf_AddText ("map ndu\n");
			loadingScreen = 1;
			loadname2 = "ndu";
			loadnamespec = "Nacht der Untoten";
			break;

		case 1:
			// Warehouse
			IN_Activate();
			key_dest = key_game;
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			
			Cbuf_AddText ("maxplayers 1\n");
			Cbuf_AddText ("deathmatch 0\n");
			Cbuf_AddText ("coop 0\n"); 
			Cbuf_AddText ("map nzp_warehouse2\n");
			Cbuf_AddText ("music_loop 0\n");
			Cbuf_AddText ("music_stop\n");
			loadingScreen = 1;
			loadname2 = "nzp_warehouse2";
			loadnamespec = "Warehouse";
			break;

		case 2:
			// Warehouse (Classic)
			IN_Activate();
			key_dest = key_game;
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			
			Cbuf_AddText ("maxplayers 1\n");
			Cbuf_AddText ("deathmatch 0\n");
			Cbuf_AddText ("coop 0\n"); 
			Cbuf_AddText ("map nzp_warehouse\n");
			Cbuf_AddText ("music_loop 0\n");
			Cbuf_AddText ("music_stop\n");
			loadingScreen = 1;
			loadname2 = "nzp_warehouse";
			loadnamespec = "Warehouse (Classic)";
			break;

		case 3:
			// Christmas Special
			IN_Activate();
			key_dest = key_game;
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			
			Cbuf_AddText ("maxplayers 1\n");
			Cbuf_AddText ("deathmatch 0\n");
			Cbuf_AddText ("coop 0\n"); 
			Cbuf_AddText ("map christmas_special\n");
			Cbuf_AddText ("music_loop 0\n");
			Cbuf_AddText ("music_stop\n");
			loadingScreen = 1;
			loadname2 = "christmas_special";
			loadnamespec = "Christmas Special";
			break;

		case 4:
			// Custom Maps
			M_Menu_Maps_f ();
			break;

		case 5:
			// Back
			M_Menu_Main_f ();
			break;
		}
	}
}

//-------------------------------------------------------
//----------------------ACHIEVMENTS----------------------
//------------------------------------------------------- 

//=============================================================================
/* ACHIEVEMENT MENU */
#ifdef VITA

int m_achievement_cursor;
int m_achievement_selected;
int m_achievement_scroll[2];
int total_unlocked_achievements;
int total_locked_achievements;


achievement_list_t achievement_list[MAX_ACHIEVEMENTS];
qpic_t *achievement_locked;

void Achievement_Init (void)
{
	achievement_list[0].img = Draw_CachePic("gfx/achievement/ready.tga");
	achievement_list[0].unlocked = 0;
	achievement_list[0].progress = 0;
	strcpy(achievement_list[0].name, "Ready..");
	strcpy(achievement_list[0].description, "Reach round 5");

	achievement_list[1].img = Draw_CachePic("gfx/achievement/steady.tga");
	achievement_list[1].unlocked = 0;
	achievement_list[1].progress = 0;
	strcpy(achievement_list[1].name, "Steady..");
	strcpy(achievement_list[1].description, "Reach round 10");

	achievement_list[2].img = Draw_CachePic("gfx/achievement/go_hell_no.tga");
	achievement_list[2].unlocked = 0;
	achievement_list[2].progress = 0;
	strcpy(achievement_list[2].name, "Go? Hell No...");
	strcpy(achievement_list[2].description, "Reach round 15");

	achievement_list[3].img = Draw_CachePic("gfx/achievement/where_legs_go.tga");
	achievement_list[3].unlocked = 0;
	achievement_list[3].progress = 0;
	strcpy(achievement_list[3].name, "Where Did Legs Go?");
	strcpy(achievement_list[3].description, "Turn a zombie into a crawler");

	achievement_list[4].img = Draw_CachePic("gfx/achievement/the_f_bomb.tga");
	achievement_list[4].unlocked = 0;
	achievement_list[4].progress = 0;
	strcpy(achievement_list[4].name, "The F Bomb");
	strcpy(achievement_list[4].description, "Use the Nuke power-up to kill a single Zombie");

	/*achievement_list[3].img = Draw_CachePic("gfx/achievement/beast");
	achievement_list[3].unlocked = 0;
	achievement_list[3].progress = 0;
	strcpy(achievement_list[3].name, "Beast");
	strcpy(achievement_list[3].description, "Survive round after round 5 with knife and grenades only");

	achievement_list[4].img = Draw_CachePic("gfx/achievement/survival");
	achievement_list[4].unlocked = 0;
	achievement_list[4].progress = 0;
	strcpy(achievement_list[4].name, "Survival Expert");
	strcpy(achievement_list[4].description, "Use pistol and knife only to reach round 10");

	achievement_list[5].img = Draw_CachePic("gfx/achievement/boomstick");
	achievement_list[5].unlocked = 0;
	achievement_list[5].progress = 0;
	strcpy(achievement_list[5].name, "Boomstick");
	strcpy(achievement_list[5].description, "3 for 1 with shotgun blast");

	achievement_list[6].img = Draw_CachePic("gfx/achievement/boom_headshots");
	achievement_list[6].unlocked = 0;
	achievement_list[6].progress = 0;
	strcpy(achievement_list[6].name, "Boom Headshots");
	strcpy(achievement_list[6].description, "Get 10 headshots");

	achievement_list[7].img = Draw_CachePic("gfx/achievement/where_did");
	achievement_list[7].unlocked = 0;
	achievement_list[7].progress = 0;
	strcpy(achievement_list[7].name, "Where Did Legs Go?");
	strcpy(achievement_list[7].description, "Make a crawler zombie");

	achievement_list[8].img = Draw_CachePic("gfx/achievement/keep_the_change");
	achievement_list[8].unlocked = 0;
	achievement_list[8].progress = 0;
	strcpy(achievement_list[8].name, "Keep The Change");
	strcpy(achievement_list[8].description, "Purchase everything");

	achievement_list[9].img = Draw_CachePic("gfx/achievement/big_thanks");
	achievement_list[9].unlocked = 0;
	achievement_list[9].progress = 0;
	strcpy(achievement_list[9].name, "Big Thanks To Explosion");
	strcpy(achievement_list[9].description, "Get 10 kills with one grenade");

	achievement_list[10].img = Draw_CachePic("gfx/achievement/warmed-up");
	achievement_list[10].unlocked = 0;
	achievement_list[10].progress = 0;
	strcpy(achievement_list[10].name, "Getting Warmed-Up");
	strcpy(achievement_list[10].description, "Achieve 10 achievements");

	achievement_list[11].img = Draw_CachePic("gfx/achievement/mysterybox_maniac");
	achievement_list[11].unlocked = 0;
	achievement_list[11].progress = 0;
	strcpy(achievement_list[11].name, "Mysterybox Maniac");
	strcpy(achievement_list[11].description, "use mysterybox 20 times");

	achievement_list[12].img = Draw_CachePic("gfx/achievement/instant_help");
	achievement_list[12].unlocked = 0;
	achievement_list[12].progress = 0;
	strcpy(achievement_list[12].name, "Instant Help");
	strcpy(achievement_list[12].description, "Kill 100 zombies with insta-kill");

	achievement_list[13].img = Draw_CachePic("gfx/achievement/blow_the_bank");
	achievement_list[13].unlocked = 0;
	achievement_list[13].progress = 0;
	strcpy(achievement_list[13].name, "Blow The Bank");
	strcpy(achievement_list[13].description, "earn 1,000,000");

	achievement_list[14].img = Draw_CachePic("gfx/achievement/ammo_cost");
	achievement_list[14].unlocked = 0;
	achievement_list[14].progress = 0;
	strcpy(achievement_list[14].name, "Ammo Cost Too Much");
	strcpy(achievement_list[14].description, "After round 5, don't fire a bullet for whole round");

	achievement_list[15].img = Draw_CachePic("gfx/achievement/the_f_bomb");
	achievement_list[15].unlocked = 0;
	achievement_list[15].progress = 0;
	strcpy(achievement_list[15].name, "The F Bomb");
	strcpy(achievement_list[15].description, "Only nuke one zombie");

	achievement_list[16].img = Draw_CachePic("gfx/achievement/why_are");
	achievement_list[16].unlocked = 0;
	achievement_list[16].progress = 0;
	strcpy(achievement_list[16].name, "Why Are We Waiting?");
	strcpy(achievement_list[16].description, "Stand still for 2 minutes");

	achievement_list[17].img = Draw_CachePic("gfx/achievement/never_missed");
	achievement_list[17].unlocked = 0;
	achievement_list[17].progress = 0;
	strcpy(achievement_list[17].name, "Never Missed A Shot");
	strcpy(achievement_list[17].description, "Make it to round 5 without missing a shot");

	achievement_list[18].img = Draw_CachePic("gfx/achievement/300_bastards_less");
	achievement_list[18].unlocked = 0;
	achievement_list[18].progress = 0;
	strcpy(achievement_list[18].name, "300 Bastards less");
	strcpy(achievement_list[18].description, "Kill 300 zombies");

	achievement_list[19].img = Draw_CachePic("gfx/achievement/music_fan");
	achievement_list[19].unlocked = 0;
	achievement_list[19].progress = 0;
	strcpy(achievement_list[19].name, "Music Fan");
	strcpy(achievement_list[19].description, "Turn on radio 20 times");

	achievement_list[20].img = Draw_CachePic("gfx/achievement/one_clip");
	achievement_list[20].unlocked = 0;
	achievement_list[20].progress = 0;
	strcpy(achievement_list[20].name, "One Clip");
	strcpy(achievement_list[20].description, "Complete a round with mg42 without reloading");

	achievement_list[21].img = Draw_CachePic("gfx/achievement/one_20_20");
	achievement_list[21].unlocked = 0;
	achievement_list[21].progress = 0;
	strcpy(achievement_list[21].name, "One Clip, 20 Bullets, 20 Headshots");
	strcpy(achievement_list[21].description, "Score 20 headshots, with 20 bullets, and don't reload");

	achievement_list[22].img = Draw_CachePic("gfx/achievement/and_stay_out");
	achievement_list[22].unlocked = 0;
	achievement_list[22].progress = 0;
	strcpy(achievement_list[22].name, "And Stay out");
	strcpy(achievement_list[22].description, "Don't let zombies in for 2 rounds");*/

	achievement_locked = Draw_CachePic("gfx/achievement/achievement_locked.tga");

	m_achievement_scroll[0] = 0;
	m_achievement_scroll[1] = 0;
}


void Load_Achivements (void)
{
	int achievement_file;
	achievement_file = sceIoOpen(va("%s/data/ach.dat",com_gamedir), SCE_O_RDONLY, 0);

	if (achievement_file >= 0) {
		char* buffer = (char*)calloc(2, sizeof(char));
		for (int i = 0; i < MAX_ACHIEVEMENTS; i++) {
			sceIoRead(achievement_file, buffer, sizeof(char)*2);
			achievement_list[i].unlocked = atoi(buffer);
			sceIoRead(achievement_file, buffer, sizeof(char)*2);
			achievement_list[i].progress = atoi(buffer);
		}
	} else {
		achievement_file = sceIoOpen(va("%s/data/ach.dat",com_gamedir), SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0);
		
		for (int i = 0; i < MAX_ACHIEVEMENTS; i++) {
			sceIoWrite(achievement_file, "0\n", sizeof(char)*2);
			sceIoWrite(achievement_file, "0\n", sizeof(char)*2);
		}
	}
	sceIoClose(achievement_file);
}
void Save_Achivements (void)
{
	int achievement_file;
	achievement_file = sceIoOpen(va("%s/data/ach.dat",com_gamedir), SCE_O_WRONLY, 0);
		
	//if(!achievement_file)
		//Sys_Error("Achievement file not preset.");

	if (achievement_file >= 0) {
		for (int i = 0; i < MAX_ACHIEVEMENTS; i++) {
			char* buffer = va("%i\n", achievement_list[i].unlocked);
			char* buffer2 = va("%i\n", achievement_list[i].progress);
			sceIoWrite(achievement_file, va("%i\n", achievement_list[i].unlocked), strlen(buffer));
			sceIoWrite(achievement_file, va("%i\n", achievement_list[i].progress), strlen(buffer2));
		}
	} else {
		Load_Achivements();
	}
}


void M_Menu_Achievement_f (void)
{
	key_dest = key_menu;
	m_state = m_achievement;
	m_entersound = true;
	Load_Achivements();
}

void M_Achievement_Draw (void)
{
    int i, b, y;
    int unlocked_achievement[MAX_ACHIEVEMENTS];
    int locked_achievement[MAX_ACHIEVEMENTS];
	int maxLenght = floor((vid.width - 155)/16);
	int	stringLenght;
	char *description;
	char *string_line = (char*) malloc(maxLenght);
	int lines;

    y = 5;
    total_unlocked_achievements = 0;
    total_locked_achievements = 0;

    for (i = 0; i < MAX_ACHIEVEMENTS; i++)
    {
        unlocked_achievement[i] = -1;
        locked_achievement[i] = -1;
        if (achievement_list[i].unlocked)
        {
            unlocked_achievement[total_unlocked_achievements] = i;
            total_unlocked_achievements += 1;
        }
        else
        {
            locked_achievement[total_locked_achievements] = i;
            total_locked_achievements += 1;
        }
    }

	// Background
	if (paused_hack == false)
		Draw_BgMenu();

	// Fill black to make everything easier to see
	Draw_FillByColor(0, 0, 960, 544, 0, 0, 0, 1);

    if (!m_achievement_selected)
    {
        Draw_FillByColor(15, 8, 225, 12, 204, 0, 0, 150);
        Draw_FillByColor(240, 8, 225, 12, 0, 0, 0, 150);

        if (total_unlocked_achievements <= 0)
        {
            Draw_FillByColor(15, 25, vid.width - 30, 60, 0, 0, 0, 150);
            Draw_StretchPic (20, 30 + y, achievement_locked, 150, 50);
            Draw_ColoredStringScale (180, 50 + y, "No achievements unlocked :(", 255, 255, 255, 1, 1.0f);
        }
        else
        {
            for (i = 0; i < 3; i++)
            {
                if (unlocked_achievement[i + m_achievement_scroll[0]] >= 0)
                {
                    Draw_FillByColor(15, 25 + y, vid.width - 30, 60, 0, 0, 0, 150);

                    Draw_StretchPic (20, 30 + y, achievement_list[unlocked_achievement[i + m_achievement_scroll[0]]].img, 150, 50);
                    Draw_ColoredStringScale (180, 30 + y, achievement_list[unlocked_achievement[i + m_achievement_scroll[0]]].name, 255, 255, 255, 1, 1.0f);
					description = achievement_list[unlocked_achievement[i + m_achievement_scroll[0]]].description;
					stringLenght = strlen(description);
					lines = stringLenght/maxLenght;
					if ((maxLenght % stringLenght) != 0)
						lines++;
					
					for (b = 0; b < lines; b++) {
						strncpy(string_line, description+maxLenght*b, (maxLenght-1));
						Draw_ColoredStringScale (180, 60 + y + b*8, string_line, 255, 255, 255, 1, 1.0f);
					}
					
					//if (stringLenght <= maxLenght)
					//	Draw_String (125, 40 + y, description);

                    y += 75;
                }
            }
        }
    }
    else
    {
        if (total_locked_achievements <= 0)
        {
            Draw_FillByColor(15, 25, vid.width - 30, 60, 0, 0, 0, 150);
            Draw_StretchPic (20, 30 + y, achievement_locked, 150, 50);
            Draw_ColoredStringScale (180, 50 + y, "All achievements unlocked :)", 255, 255, 255, 1, 1.0f);
        }

        Draw_FillByColor(15, 8, 225, 12, 0, 0, 0, 150);
        Draw_FillByColor(vid.width - 167, 8, 225, 12, 204, 0, 0, 150);

        for (i = 0; i < 3; i++)
        {
            if (locked_achievement[i + m_achievement_scroll[1]] >= 0)
            {
                Draw_FillByColor(15, 25 + y, vid.width - 30, 60, 0, 0, 0, 150);

                Draw_StretchPic (20, 30 + y, achievement_locked, 150, 50);
                Draw_ColoredStringScale (180, 30 + y, achievement_list[locked_achievement[i + m_achievement_scroll[1]]].name, 255, 255, 255, 1, 1.0f);
				description = achievement_list[locked_achievement[i + m_achievement_scroll[1]]].description;
				stringLenght = strlen(description);
				lines = stringLenght/maxLenght;
				if ((maxLenght % stringLenght) != 0)
					lines++;
				
				for (b = 0; b < lines; b++) {
					strncpy(string_line, description+maxLenght*b, (maxLenght-1));
					Draw_ColoredStringScale (180, 60 + y + b*8, string_line, 255, 255, 255, 1, 1.0f);
				}
					
					
				//if (stringLenght <= maxLenght)
				//	Draw_String (125, 40 + y, description);

                y += 70;
            }
        }
    }

	free(string_line);
    Draw_ColoredStringScale (15, 10, "Unlocked Achievements", 255, 255, 255, 1, 1.0f);
    Draw_ColoredStringScale (vid.width - 167, 10, "Locked Achievements", 255, 255, 255, 1, 1.0f);
}

void M_Achievement_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_Main_f ();
		break;

    case K_LEFTARROW:
        m_achievement_selected = 0;
        break;

    case K_RIGHTARROW:
        m_achievement_selected = 1;
        break;

    case K_UPARROW:
        m_achievement_scroll[m_achievement_selected]--;
        if (m_achievement_scroll[m_achievement_selected] < 0)
            m_achievement_scroll[m_achievement_selected] = 0;
        break;

    case K_DOWNARROW:
        m_achievement_scroll[m_achievement_selected]++;

        if (m_achievement_selected)
        {
            if (m_achievement_scroll[1] > total_locked_achievements - 3)
                m_achievement_scroll[1] = total_locked_achievements - 3;
        }
        else
        {
            if (m_achievement_scroll[0] > total_unlocked_achievements - 3)
                m_achievement_scroll[0] = total_unlocked_achievements - 3;
        }
        if (m_achievement_scroll[m_achievement_selected] < 0)
            m_achievement_scroll[m_achievement_selected] = 0;
        break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		m_entersound = true;
		switch (m_achievement_cursor)
		{
		}
	}
}
#endif

int m_maps_cursor;
int	MAP_ITEMS;
int user_maps_num = 0;
int current_custom_map_page;
int custom_map_pages;
int multiplier;
char  user_levels[256][MAX_QPATH];

void M_Menu_Maps_f (void) 
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_maps;
	m_entersound = true;
	MAP_ITEMS = 13;
	current_custom_map_page = 1;
}
extern vrect_t scr_vrect;
void M_Menu_Maps_Draw (void)
{
	qpic_t* menu_cuthum;

	MENU_INITVARS();

	// Menu Background
	Draw_BgMenu();

	// Fill black to make everything easier to see
	Draw_FillByColor(0, 0, 1280, 720, 0, 0, 0, 0.4);

	// Header
	DRAW_HEADER("CUSTOM MAPS");

	int line_increment;
	line_increment = 0;

	if (current_custom_map_page > 1)
		multiplier = (current_custom_map_page - 1) * 15;
	else
		multiplier = 0;

	for (int i = 0; i < 15; i++) {
		if (custom_maps[i + multiplier].occupied == false)
			continue;

		if (custom_maps[i + multiplier].map_name_pretty != 0) {
			DRAW_MENUOPTION(i, custom_maps[i + multiplier].map_name_pretty, m_maps_cursor, false);
		} else {
			DRAW_MENUOPTION(i, custom_maps[i + multiplier].map_name, m_maps_cursor, false);
		}

		if (m_maps_cursor == i) {
			if (custom_maps[i + multiplier].map_use_thumbnail == 1) {
				menu_cuthum = Draw_CachePic(custom_maps[i + multiplier].map_thumbnail_path);

				if (menu_cuthum != NULL)
					DRAW_MAPTHUMB(menu_cuthum);
			}

			if (custom_maps[i + multiplier].map_desc_1 != 0) {
				if (strcmp(custom_maps[i + multiplier].map_desc_1, " ") != 0) {
					DRAW_MAPDESC(0, custom_maps[i + multiplier].map_desc_1);
				}
			}
			if (custom_maps[i + multiplier].map_desc_2 != 0) {
				if (strcmp(custom_maps[i + multiplier].map_desc_2, " ") != 0) {
					line_increment++;
					DRAW_MAPDESC(1, custom_maps[i + multiplier].map_desc_2);
				}
			}
			if (custom_maps[i + multiplier].map_desc_3 != 0) {
				if (strcmp(custom_maps[i + multiplier].map_desc_3, " ") != 0) {
					line_increment++;
					DRAW_MAPDESC(2, custom_maps[i + multiplier].map_desc_3);
				}
			}
			if (custom_maps[i + multiplier].map_desc_4 != 0) {
				if (strcmp(custom_maps[i + multiplier].map_desc_4, " ") != 0) {
					line_increment++;
					DRAW_MAPDESC(3, custom_maps[i + multiplier].map_desc_4);
				}
			}
			if (custom_maps[i + multiplier].map_desc_5 != 0) {
				if (strcmp(custom_maps[i + multiplier].map_desc_5, " ") != 0) {
					line_increment++;
					DRAW_MAPDESC(4, custom_maps[i + multiplier].map_desc_5);
				}
			}
			if (custom_maps[i + multiplier].map_desc_6 != 0) {
				if (strcmp(custom_maps[i + multiplier].map_desc_6, " ") != 0) {
					line_increment++;
					DRAW_MAPDESC(5, custom_maps[i + multiplier].map_desc_6);
				}
			}
			if (custom_maps[i + multiplier].map_desc_7 != 0) {
				if (strcmp(custom_maps[i + multiplier].map_desc_7, " ") != 0) {
					line_increment++;
					DRAW_MAPDESC(6, custom_maps[i + multiplier].map_desc_7);
				}
			}
			if (custom_maps[i + multiplier].map_desc_8 != 0) {
				if (strcmp(custom_maps[i + multiplier].map_desc_8, " ") != 0) {
					line_increment++;
					DRAW_MAPDESC(7, custom_maps[i + multiplier].map_desc_8);
				}
			}
			if (custom_maps[i + multiplier].map_author != 0) {
				if (strcmp(custom_maps[i + multiplier].map_author, " ") != 0) {
					line_increment++;
					DRAW_MAPAUTHOR(line_increment, custom_maps[i + multiplier].map_author);
				}
			}
		}
	}

#ifdef VITA

	menu_offset_y = 441;

#else

	menu_offset_y = y + 285;

#endif // VITA

	if (current_custom_map_page != custom_map_pages) {
		DRAW_MENUOPTION(15, "Next Page", m_maps_cursor, false);
	} else {
		DRAW_BLANKOPTION("Next Page", false);
	}

	if (current_custom_map_page != 1) {
		DRAW_MENUOPTION(16, "Previous Page", m_maps_cursor, false);
	} else {
		DRAW_BLANKOPTION("Previous Page", false);
	}

	DRAW_BACKBUTTON(17, m_maps_cursor);
}


void M_Menu_Maps_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_SinglePlayer_f ();
		break;
	case K_DOWNARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		m_maps_cursor++;

		if (m_maps_cursor < 14 && custom_maps[m_maps_cursor + multiplier].occupied == false) {
			m_maps_cursor = 15;
		}

		if (m_maps_cursor == 15 && current_custom_map_page == custom_map_pages)
			m_maps_cursor = 16;
		
		if (m_maps_cursor == 16 && current_custom_map_page == 1)
			m_maps_cursor = 17;

		if (m_maps_cursor >= 18)
			m_maps_cursor = 0;
		break;
	case K_UPARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		m_maps_cursor--;

		if (m_maps_cursor < 0)
			m_maps_cursor = 17;

		if (m_maps_cursor == 16 && current_custom_map_page == 1)
			m_maps_cursor = 15;

		if (m_maps_cursor == 15 && current_custom_map_page == custom_map_pages)
			m_maps_cursor = 14;

		if (m_maps_cursor <= 14 && custom_maps[m_maps_cursor + multiplier].occupied == false) {
			for (int i = 14; i > -1; i--) {
				if (custom_maps[i + multiplier].occupied == true) {
					m_maps_cursor = i;
					break;
				}
			}
		}
		break;
	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		m_entersound = true;

		if (m_maps_cursor == 17) {
			M_Menu_SinglePlayer_f ();
		} else if (m_maps_cursor == 16) {
			current_custom_map_page--;
			m_maps_cursor = 0;
		} else if (m_maps_cursor == 15) {
			current_custom_map_page++;
			m_maps_cursor = 0;
		} else {
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			
			Cbuf_AddText ("maxplayers 1\n");
			Cbuf_AddText ("deathmatch 0\n");
			Cbuf_AddText ("coop 0\n"); 
			Cbuf_AddText ("music_loop 0\n");
			Cbuf_AddText ("music_stop\n");

			char map_selection[MAX_QPATH];
			strcpy(map_selection, custom_maps[m_maps_cursor + multiplier].map_name);
			Cbuf_AddText (va("map %s\n", custom_maps[m_maps_cursor + multiplier].map_name));
			
			loadingScreen = 1;
			loadname2 = custom_maps[m_maps_cursor + multiplier].map_name;
			if (custom_maps[m_maps_cursor + multiplier].map_name_pretty != 0)
				loadnamespec = custom_maps[m_maps_cursor + multiplier].map_name_pretty;
			else
				loadnamespec = custom_maps[m_maps_cursor + multiplier].map_name;
		}
		break;
	}
}

//==================== Map Find System By Crow_bar =============================
// Modified by Naievil for Nintendo Switch release 

extern char	cwd[MAX_OSPATH];

void Map_Finder(void)
{
	DIR* dir;
    struct dirent* ent;
	char map_dir[MAX_OSPATH];
	qboolean breakaway;

	for (int i = 0; i < 50; i++) {
		custom_maps[i].occupied = false;
	}

	strcpy(map_dir, strcat(cwd, "/nzp/maps"));
	dir = opendir(map_dir);
    if(dir==NULL)
    {
        Host_Error("Map_Finder: Failed to open dir %s\n", map_dir);
    }
    else
    {
        while ((ent = readdir(dir)))
        {
	        if(!strcmp(COM_FileGetExtension(ent->d_name),"bsp") || !strcmp(COM_FileGetExtension(ent->d_name),"BSP")) 
			{
				// Attempt to fix operating system files (macOS, bleh)
				// from appearing in maps menu.
				if (ent->d_name[0] == '.' || ent->d_name[0] == '_')
					continue;

				breakaway = false;
				char ntype[32];
				COM_StripExtension(ent->d_name, ntype, sizeof(ntype));

				for (int j = 0; j < BASE_MAP_COUNT; j++)
					{
						if (breakaway == true)
							break;

						if(!strcmp(ntype, base_maps[j]))
						{
							//Con_Printf("ntype: %s\n base_map: %s\n", ntype, base_maps[j]);
							breakaway = true;
						}
					}

				if (breakaway == true)
				{
					//Con_Printf("Breaking away at ntype: %s\n", ntype);
					continue;
				}
				else
				{
					//Con_Printf("Success at ntype: %s\n", ntype);
					custom_maps[user_maps_num].occupied = true;
					custom_maps[user_maps_num].map_name = malloc(sizeof(char)*32);
					sprintf(custom_maps[user_maps_num].map_name, "%s", ntype);

					char* 	setting_path;
					FILE* 	setting_file;

					setting_path 									= malloc(sizeof(char)*64);
					custom_maps[user_maps_num].map_thumbnail_path 	= malloc(sizeof(char)*64);

					strcpy(setting_path, map_dir);
					strcat(setting_path, "/");

					strcpy(custom_maps[user_maps_num].map_thumbnail_path, 	"gfx/menu/custom/");
					strcat(setting_path, 									custom_maps[user_maps_num].map_name);
					strcat(custom_maps[user_maps_num].map_thumbnail_path, 	custom_maps[user_maps_num].map_name);
					strcat(custom_maps[user_maps_num].map_thumbnail_path, 	".tga");
					strcat(setting_path, 									".txt");

					setting_file = fopen(setting_path, "r");

					if (setting_file != NULL) {
						int state;
						state = 0;
						int value;

						custom_maps[user_maps_num].map_name_pretty = malloc(sizeof(char)*32);
						custom_maps[user_maps_num].map_desc_1 = malloc(sizeof(char)*40);
						custom_maps[user_maps_num].map_desc_2 = malloc(sizeof(char)*40);
						custom_maps[user_maps_num].map_desc_3 = malloc(sizeof(char)*40);
						custom_maps[user_maps_num].map_desc_4 = malloc(sizeof(char)*40);
						custom_maps[user_maps_num].map_desc_5 = malloc(sizeof(char)*40);
						custom_maps[user_maps_num].map_desc_6 = malloc(sizeof(char)*40);
						custom_maps[user_maps_num].map_desc_7 = malloc(sizeof(char)*40);
						custom_maps[user_maps_num].map_desc_8 = malloc(sizeof(char)*40);
						custom_maps[user_maps_num].map_author = malloc(sizeof(char)*40);

						char buffer[64];
						int bufferlen = sizeof(buffer);

						while(fgets(buffer, bufferlen, setting_file)) {
							// strip newlines
							buffer[strcspn(buffer, "\r")] = 0;
							buffer[strcspn(buffer, "\n")] = 0;

							switch(state) {
								case 0: strcpy(custom_maps[user_maps_num].map_name_pretty, buffer); break;
								case 1: strcpy(custom_maps[user_maps_num].map_desc_1, buffer); break;
								case 2: strcpy(custom_maps[user_maps_num].map_desc_2, buffer); break;
								case 3: strcpy(custom_maps[user_maps_num].map_desc_3, buffer); break;
								case 4: strcpy(custom_maps[user_maps_num].map_desc_4, buffer); break;
								case 5: strcpy(custom_maps[user_maps_num].map_desc_5, buffer); break;
								case 6: strcpy(custom_maps[user_maps_num].map_desc_6, buffer); break;
								case 7: strcpy(custom_maps[user_maps_num].map_desc_7, buffer); break;
								case 8: strcpy(custom_maps[user_maps_num].map_desc_8, buffer); break;
								case 9: strcpy(custom_maps[user_maps_num].map_author, buffer); break;
								case 10: value = 0; sscanf(buffer, "%d", &value); custom_maps[user_maps_num].map_use_thumbnail = value; break;
								case 11: value = 0; sscanf(buffer, "%d", &value); custom_maps[user_maps_num].map_allow_game_settings = value; break;
								default: break;
							}
							state++;
						}
					fclose(setting_file);
				}
				user_maps_num = user_maps_num + 1;
				}
			}
        }
        closedir(dir);
    }
	custom_map_pages = (int)ceil((double)(user_maps_num + 1)/15);
}

//=============================================================================
/* MULTIPLAYER MENU */

int	m_multiplayer_cursor;
#define	MULTIPLAYER_ITEMS	3


void M_Menu_MultiPlayer_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_multiplayer;
	m_entersound = true;
}


void M_MultiPlayer_Draw (void)
{
	int		f;
	qpic_t	*p;

	// M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	// p = Draw_CachePic ("gfx/p_multi.lmp");
	// M_DrawPic ( (320-p->width)/2, 4, p);
	// M_DrawTransPic (72, 32, Draw_CachePic ("gfx/mp_menu.lmp") );

	f = (int)(realtime * 10)%6;

	// M_DrawTransPic (54, 32 + m_multiplayer_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );

	if (ipxAvailable || tcpipAvailable)
		return;
	M_PrintWhite ((320/2) - ((27*8)/2), 148, "No Communications Available");
}


void M_MultiPlayer_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_Main_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		if (++m_multiplayer_cursor >= MULTIPLAYER_ITEMS)
			m_multiplayer_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		if (--m_multiplayer_cursor < 0)
			m_multiplayer_cursor = MULTIPLAYER_ITEMS - 1;
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		m_entersound = true;
		switch (m_multiplayer_cursor)
		{
		case 0:
			if (ipxAvailable || tcpipAvailable)
				M_Menu_Net_f ();
			break;

		case 1:
			if (ipxAvailable || tcpipAvailable)
				M_Menu_Net_f ();
			break;

		case 2:
			M_Menu_Setup_f ();
			break;
		}
	}
}

//=============================================================================
/* SETUP MENU */

int		setup_cursor = 4;
int		setup_cursor_table[] = {40, 56, 80, 104, 140};

char	setup_hostname[16];
char	setup_myname[16];
int		setup_oldtop;
int		setup_oldbottom;
int		setup_top;
int		setup_bottom;

#define	NUM_SETUP_CMDS	5

void M_Menu_Setup_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_setup;
	m_entersound = true;
	Q_strcpy(setup_myname, cl_name.string);
	Q_strcpy(setup_hostname, hostname.string);
	setup_top = setup_oldtop = ((int)cl_color.value) >> 4;
	setup_bottom = setup_oldbottom = ((int)cl_color.value) & 15;
}


void M_Setup_Draw (void)
{
	qpic_t	*p;

	//M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	//p = Draw_CachePic ("gfx/p_multi.lmp");
	//M_DrawPic ( (320-p->width)/2, 4, p);

	M_Print (64, 40, "Hostname");
	M_DrawTextBox (160, 32, 16, 1);
	M_Print (168, 40, setup_hostname);

	M_Print (64, 56, "Your name");
	M_DrawTextBox (160, 48, 16, 1);
	M_Print (168, 56, setup_myname);

	M_Print (64, 80, "Shirt color");
	M_Print (64, 104, "Pants color");

	M_DrawTextBox (64, 140-8, 14, 1);
	M_Print (72, 140, "Accept Changes");

	//p = Draw_CachePic ("gfx/bigbox.lmp");
	//M_DrawTransPic (160, 64, p);
	//p = Draw_CachePic ("gfx/menuplyr.lmp");
	//M_DrawTransPicTranslate (172, 72, p, setup_top, setup_bottom);

	M_DrawCharacter (56, setup_cursor_table [setup_cursor], 12+((int)(realtime*4)&1));

	if (setup_cursor == 0)
		M_DrawCharacter (168 + 8*strlen(setup_hostname), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));

	if (setup_cursor == 1)
		M_DrawCharacter (168 + 8*strlen(setup_myname), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));
}


void M_Setup_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_MultiPlayer_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		setup_cursor--;
		if (setup_cursor < 0)
			setup_cursor = NUM_SETUP_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		setup_cursor++;
		if (setup_cursor >= NUM_SETUP_CMDS)
			setup_cursor = 0;
		break;

	case K_LEFTARROW:
		if (setup_cursor < 2)
			return;
		S_LocalSound ("sounds/menu/navigate.wav");
		if (setup_cursor == 2)
			setup_top = setup_top - 1;
		if (setup_cursor == 3)
			setup_bottom = setup_bottom - 1;
		break;
	case K_RIGHTARROW:
		if (setup_cursor < 2)
			return;
forward:
		S_LocalSound ("sounds/menu/navigate.wav");
		if (setup_cursor == 2)
			setup_top = setup_top + 1;
		if (setup_cursor == 3)
			setup_bottom = setup_bottom + 1;
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		if (setup_cursor == 0 || setup_cursor == 1)
			return;

		if (setup_cursor == 2 || setup_cursor == 3)
			goto forward;

		// setup_cursor == 4 (OK)
		if (strcmp(cl_name.string, setup_myname) != 0)
			Cbuf_AddText ( va ("name \"%s\"\n", setup_myname) );
		if (strcmp(hostname.string, setup_hostname) != 0)
			Cvar_Set("hostname", setup_hostname);
		if (setup_top != setup_oldtop || setup_bottom != setup_oldbottom)
			Cbuf_AddText( va ("color %i %i\n", setup_top, setup_bottom) );
		m_entersound = true;
		M_Menu_MultiPlayer_f ();
		break;

	case K_BACKSPACE:
		if (setup_cursor == 0)
		{
			if (strlen(setup_hostname))
				setup_hostname[strlen(setup_hostname)-1] = 0;
		}

		if (setup_cursor == 1)
		{
			if (strlen(setup_myname))
				setup_myname[strlen(setup_myname)-1] = 0;
		}
		break;
	}

	if (setup_top > 13)
		setup_top = 0;
	if (setup_top < 0)
		setup_top = 13;
	if (setup_bottom > 13)
		setup_bottom = 0;
	if (setup_bottom < 0)
		setup_bottom = 13;
}


void M_Setup_Char (int k)
{
	int l;

	switch (setup_cursor)
	{
	case 0:
		l = strlen(setup_hostname);
		if (l < 15)
		{
			setup_hostname[l+1] = 0;
			setup_hostname[l] = k;
		}
		break;
	case 1:
		l = strlen(setup_myname);
		if (l < 15)
		{
			setup_myname[l+1] = 0;
			setup_myname[l] = k;
		}
		break;
	}
}


qboolean M_Setup_TextEntry (void)
{
	return (setup_cursor == 0 || setup_cursor == 1);
}

//=============================================================================
/* NET MENU */

int	m_net_cursor;
int m_net_items;

const char *net_helpMessage [] =
{
/* .........1.........2.... */
  " Novell network LANs    ",
  " or Windows 95 DOS-box. ",
  "                        ",
  "(LAN=Local Area Network)",

  " Commonly used to play  ",
  " over the Internet, but ",
  " also used on a Local   ",
  " Area Network.          "
};

void M_Menu_Net_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_net;
	m_entersound = true;
	m_net_items = 2;

	if (m_net_cursor >= m_net_items)
		m_net_cursor = 0;
	m_net_cursor--;
	M_Net_Key (K_DOWNARROW);
}


void M_Net_Draw (void)
{
	int		f;
	qpic_t	*p;

	// M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	// p = Draw_CachePic ("gfx/p_multi.lmp");
	// M_DrawPic ( (320-p->width)/2, 4, p);

	f = 32;

	// if (ipxAvailable)
	// 	p = Draw_CachePic ("gfx/netmen3.lmp");
	// else
	// 	p = Draw_CachePic ("gfx/dim_ipx.lmp");
	// M_DrawTransPic (72, f, p);

	f += 19;
	// if (tcpipAvailable)
	// 	p = Draw_CachePic ("gfx/netmen4.lmp");
	// else
	// 	p = Draw_CachePic ("gfx/dim_tcp.lmp");
	// M_DrawTransPic (72, f, p);

	f = (320-26*8)/2;
	M_DrawTextBox (f, 96, 24, 4);
	f += 8;
	M_Print (f, 104, net_helpMessage[m_net_cursor*4+0]);
	M_Print (f, 112, net_helpMessage[m_net_cursor*4+1]);
	M_Print (f, 120, net_helpMessage[m_net_cursor*4+2]);
	M_Print (f, 128, net_helpMessage[m_net_cursor*4+3]);

	f = (int)(realtime * 10)%6;
	//M_DrawTransPic (54, 32 + m_net_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );
}


void M_Net_Key (int k)
{
again:
	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_MultiPlayer_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		if (++m_net_cursor >= m_net_items)
			m_net_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		if (--m_net_cursor < 0)
			m_net_cursor = m_net_items - 1;
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		m_entersound = true;
		M_Menu_LanConfig_f ();
		break;
	}

	if (m_net_cursor == 0 && !ipxAvailable)
		goto again;
	if (m_net_cursor == 1 && !tcpipAvailable)
		goto again;
}

//=============================================================================
/* OPTIONS MENU */

enum
{
	OPT_CUSTOMIZE = 0,
	OPT_CONSOLE,	// 1
	OPT_DEFAULTS,	// 2
	OPT_SCALE,
	OPT_SCRSIZE,
	OPT_GAMMA,
	OPT_CONTRAST,
	OPT_MOUSESPEED,
	OPT_SBALPHA,
	OPT_SNDVOL,
	OPT_MUSICVOL,
	OPT_MUSICEXT,
	OPT_ALWAYRUN,
	OPT_INVMOUSE,
	OPT_ALWAYSMLOOK,
	OPT_LOOKSPRING,
	OPT_LOOKSTRAFE,
	OPT_AIMASSIST,
//#ifdef _WIN32
//	OPT_USEMOUSE,
//#endif
	OPT_VIDEO,	// This is the last before OPTIONS_ITEMS
	OPTIONS_ITEMS
};

enum
{
	ALWAYSRUN_OFF = 0,
	ALWAYSRUN_VANILLA,
	ALWAYSRUN_QUAKESPASM,
	ALWAYSRUN_ITEMS
};

#define	SLIDER_RANGE	10

int		options_cursor;
void M_Menu_Options_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	old_m_state = m_state;
	m_state = m_options;
	m_entersound = true;
}

// stupid fucking hack because i am lazy
void M_AdjustSlidersAdvanced (int dir, int option)
{
	S_LocalSound ("sounds/menu/navigate.wav");

	switch(option) {
		case OPT_GSETTING_GAMMA:	// gamma
			vid_gamma.value -= dir * 0.05;
			if (vid_gamma.value < 0.5)
				vid_gamma.value = 0.5;
			if (vid_gamma.value > 1)
				vid_gamma.value = 1;
			Cvar_SetValue ("gamma", vid_gamma.value);
			break;
		case OPT_GSETTING_MAXFPS:
			host_maxfps.value = host_maxfps.value + dir * 5;
			if (host_maxfps.value < 30)
				host_maxfps.value = 30;
			if (host_maxfps.value > 65)
				host_maxfps.value = 65;
			Cvar_SetValue ("host_maxfps", host_maxfps.value);
			break;
		case OPT_GSETTING_FOV:
			scr_fov.value += dir * 5;
			if (scr_fov.value < 50)
				scr_fov.value = 50;
			if (scr_fov.value > 120 && dir > 0)
				scr_fov.value = 120;
			Cvar_SetValue ("fov", scr_fov.value);
			break;
		case OPT_CSETTING_LSENS:
			sensitivity.value += dir * 0.5;
			if (sensitivity.value < 1)
				sensitivity.value = 1;
			if (sensitivity.value > 11)
				sensitivity.value = 11;
			Cvar_SetValue ("sensitivity", sensitivity.value);
			break;
		case OPT_CSETTING_GSEX:
			gyrosensx.value += dir * 0.05;
			if (gyrosensx.value < 0.5)
				gyrosensx.value = 0.5;
			if (gyrosensx.value > 1)
				gyrosensx.value = 1;
			Cvar_SetValue ("gyrosensx", gyrosensx.value);
			break;
		case OPT_CSETTING_GSEY:
			gyrosensy.value += dir * 0.05;
			if (gyrosensy.value < 0.5)
				gyrosensy.value = 0.5;
			if (gyrosensy.value > 1)
				gyrosensy.value = 1;
			Cvar_SetValue ("gyrosensy", gyrosensy.value);
			break;
		/*case OPT_CSETTING_LACC:
			in_acceleration.value -= dir * 0.25;
			if (in_acceleration.value < 0.5)
				in_acceleration.value = 0.5;
			if (in_acceleration.value > 2)
				in_acceleration.value = 2;
			Cvar_SetValue ("acceleration", in_acceleration.value);
			break;*/
		default: break;
	}
}

void M_AdjustSliders (int dir)
{
	float	f, l;

	S_LocalSound ("sounds/menu/navigate.wav");

	switch (options_cursor)
	{
	case OPT_SCALE:	// console and menu scale
		l = ((vid.width + 31) / 32) / 10.0;
		f = scr_conscale.value + dir * .1;
		if (f < 1)	f = 1;
		else if(f > l)	f = l;
		Cvar_SetValue ("scr_conscale", f);
		Cvar_SetValue ("scr_menuscale", f);
		Cvar_SetValue ("scr_sbarscale", f);
		break;
	case OPT_SCRSIZE:	// screen size
		f = scr_viewsize.value + dir * 10;
		if (f > 120)	f = 120;
		else if(f < 30)	f = 30;
		Cvar_SetValue ("viewsize", f);
		break;
	case OPT_GAMMA:	// gamma
		f = vid_gamma.value - dir * 0.05;
		if (f < 0.5)	f = 0.5;
		else if (f > 1)	f = 1;
		Cvar_SetValue ("gamma", f);
		break;
	case OPT_CONTRAST:	// contrast
		f = vid_contrast.value + dir * 0.1;
		if (f < 1)	f = 1;
		else if (f > 2)	f = 2;
		Cvar_SetValue ("contrast", f);
		break;
	case OPT_MOUSESPEED:	// mouse speed
		f = sensitivity.value + dir * 0.5;
		if (f > 11)	f = 11;
		else if (f < 1)	f = 1;
		Cvar_SetValue ("sensitivity", f);
		break;
	case OPT_SBALPHA:	// statusbar alpha
		f = scr_sbaralpha.value - dir * 0.05;
		if (f < 0)	f = 0;
		else if (f > 1)	f = 1;
		Cvar_SetValue ("scr_sbaralpha", f);
		break;
	case OPT_MUSICVOL:	// music volume
		f = bgmvolume.value + dir * 0.1;
		if (f < 0)	f = 0;
		else if (f > 1)	f = 1;
		Cvar_SetValue ("bgmvolume", f);
		break;
	case OPT_MUSICEXT:	// enable external music vs cdaudio
		Cvar_Set ("bgm_extmusic", bgm_extmusic.value ? "0" : "1");
		break;
	case OPT_SNDVOL:	// sfx volume
		f = sfxvolume.value + dir * 0.1;
		if (f < 0)	f = 0;
		else if (f > 1)	f = 1;
		Cvar_SetValue ("volume", f);
		break;

	case OPT_ALWAYRUN:	// always run						
		Cvar_SetValue ("cl_alwaysrun", 0);
		Cvar_SetValue ("cl_forwardspeed", 165);
		Cvar_SetValue ("cl_backspeed", 140);
		break;

	case OPT_INVMOUSE:	// invert mouse
		Cvar_SetValue ("m_pitch", -m_pitch.value);
		break;

	case OPT_ALWAYSMLOOK:
		if (in_mlook.state & 1)
			Cbuf_AddText("-mlook");
		else
			Cbuf_AddText("+mlook");
		break;

	case OPT_LOOKSPRING:	// lookspring
		Cvar_Set ("lookspring", lookspring.value ? "0" : "1");
		break;

	case OPT_LOOKSTRAFE:	// lookstrafe
		Cvar_Set ("lookstrafe", lookstrafe.value ? "0" : "1");
		break;

	case OPT_AIMASSIST:
		Cvar_Set ("in_aimassist", in_aimassist.value ? "0" : "1");
		break;
	}
}

void M_DrawSlider (int x, int y, float range, float scale)
{
	int	i;

	if (range < 0)
		range = 0;
	if (range > 1)
		range = 1;
	Draw_CharacterScale(x - 8*scale, y, 128, scale);
	for (i = 0; i < SLIDER_RANGE; i++)
		Draw_CharacterScale(x + i*8*scale, y, 129, scale);
	Draw_CharacterScale(x + i * 8 * scale, y, 130, scale);
	Draw_CharacterScale(x + (SLIDER_RANGE-1) * 8 * range * scale, y, 131, scale);
}

void M_DrawCheckbox (int x, int y, int on)
{
#if 0
	if (on)
		M_DrawCharacter (x, y, 131);
	else
		M_DrawCharacter (x, y, 129);
#endif
	if (on)
		M_Print (x, y, "on");
	else
		M_Print (x, y, "off");
}

#define OPTION_ITEMS 5

void M_Options_Draw (void)
{
	MENU_INITVARS();

	// Menu Background
	if (paused_hack == false)
		Draw_BgMenu();

	// Fill black to make everything easier to see
	Draw_FillByColor(0, 0, 1280, 720, 0, 0, 0, 0.4);

	// Header
	DRAW_HEADER("SETTINGS");

	DRAW_MENUOPTION(0, "Graphics Settings", options_cursor, false);
	DRAW_MENUOPTION(1, "Controls", options_cursor, false);
	DRAW_MENUOPTION(2, "Control Settings", options_cursor, true);
	DRAW_MENUOPTION(3, "Console", options_cursor, false);

	DRAW_BACKBUTTON(4, options_cursor);

	// Descriptions
	switch(options_cursor) {
		case 0: DRAW_DESCRIPTION("Adjust settings relating to Graphical Fidelity."); break;
		case 1: DRAW_DESCRIPTION("Customize your Control Scheme."); break;
		case 2: DRAW_DESCRIPTION("Adjust settings in relation to how NZ:P Controls."); break;
		case 3: DRAW_DESCRIPTION("Option the Console to input Commands."); break;
		default: break;
	}
}

extern qboolean console_enabled;
void M_Options_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:

		if (paused_hack == true) {
			M_Paused_Menu_f ();
		} else {
			M_Menu_Main_f ();	
		}

		break;
	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		m_entersound = true;

		switch (options_cursor) {
			case 0:
				M_Graphics_Settings_f();
				break;
			case 1:
				M_Menu_Keys_f();
				break;
			case 2:
				M_Control_Settings_f();
				break;
			case 3:
				m_state = m_none;
				paused_hack = false;
				console_enabled = true;
				Con_ToggleConsole_f();
				break;
			case 4:

				if (paused_hack == true) {
					M_Paused_Menu_f ();
				} else {
					M_Menu_Main_f ();	
				}

				break;
			default: break;
		}

		break;
	case K_UPARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		options_cursor--;
		if (options_cursor < 0) {
			options_cursor = OPTION_ITEMS-1;
		}
		break;
	case K_DOWNARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		options_cursor++;
        if (options_cursor >= OPTION_ITEMS)
            options_cursor = 0;
		break;
	}
}

//
// NZ:P Graphics Settings
//

void M_Graphics_Settings_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_net; // lazy
	m_entersound = true;
}

static int gsettings_cursor;

#define GSETTINGS_ITEMS 		6

void M_Graphics_Settings_Draw (void)
{
	MENU_INITVARS();
	float r;

	// Menu Background
	if (paused_hack == false)
		Draw_BgMenu();

	// Fill black to make everything easier to see
	Draw_FillByColor(0, 0, 1280, 720, 0, 0, 0, 0.4);

	// Header
	DRAW_HEADER("GRAPHICS SETTINGS");

	DRAW_MENUOPTION(0, "Show FPS", gsettings_cursor, false);
	DRAW_MENUOPTION(1, "Max FPS", gsettings_cursor, false);
	DRAW_MENUOPTION(2, "Field of View", gsettings_cursor, false);
	DRAW_MENUOPTION(3, "Brightness", gsettings_cursor, false);
	DRAW_MENUOPTION(4, "Fullbright", gsettings_cursor, false);
	DRAW_MENUOPTION(5, "Retro", gsettings_cursor, false);

	DRAW_BACKBUTTON(6, gsettings_cursor);

	// Show FPS
	if (scr_showfps.value == 0) {
		DRAW_SETTINGSVALUE(0, "Disabled");
	} else {
		DRAW_SETTINGSVALUE(0, "Enabled");
	}

	// Max FPS
	r = (host_maxfps.value - 30.0)*(1.0/35.0);
	DRAW_SLIDER(1, r);
	
	// Field of View
	r = (scr_fov.value - 50.0)*(1.0/70.0);
	DRAW_SLIDER(2, r);

	// Brightness
	r = (1.0 - vid_gamma.value) / 0.5;
	DRAW_SLIDER(3, r);

	// Fullbright
	if (r_fullbright.value == 0) {
		DRAW_SETTINGSVALUE(4, "Disabled");
	} else {
		DRAW_SETTINGSVALUE(4, "Enabled");
	}

	// Retro
	if (!strcmp(gl_texturemode.string, "GL_LINEAR")) {
		DRAW_SETTINGSVALUE(5, "Disabled");
	} else {
		DRAW_SETTINGSVALUE(5, "Enabled");
	}

	// Descriptions
	switch(gsettings_cursor) {
		case 0: DRAW_DESCRIPTION("Toggle Framerate Overlay."); break;
		case 1: DRAW_DESCRIPTION("Increase or Decrease Max Frames per Second."); break;
		case 2: DRAW_DESCRIPTION("Adjust Game Field of View."); break;
		case 3: DRAW_DESCRIPTION("Increase or Decrease Game Brightness."); break;
		case 4: DRAW_DESCRIPTION("Toggle all non-realtime lights."); break;
		case 5: DRAW_DESCRIPTION("Toggle gl_nearest texture filtering."); break;
		default: break;
	}
}

void M_Graphics_Settings_Key (int key)
{
	switch(key) {
		case K_ABUTTON:
			S_LocalSound ("sounds/menu/enter.wav");
			switch(gsettings_cursor) {
				case 0: Cvar_SetValue("scr_showfps", scr_showfps.value ? 0 : 1); break;
				case 4: Cvar_SetValue("r_fullbright", r_fullbright.value ? 0 : 1); break;
				case 5: Cvar_Set("gl_texturemode", strcmp(gl_texturemode.string, "GL_LINEAR") ? "GL_LINEAR" : "GL_NEAREST_MIPMAP_LINEAR" ); break;
				case 6: M_Menu_Options_f (); break;
				default: break;
			}
			break;
		case K_BBUTTON:
			M_Menu_Options_f ();
			break;
		case K_LEFTARROW:
			switch(gsettings_cursor) {
				case 1: M_AdjustSlidersAdvanced(-1, OPT_GSETTING_MAXFPS); break;
				case 2: M_AdjustSlidersAdvanced(-1, OPT_GSETTING_FOV); break;
				case 3: M_AdjustSlidersAdvanced(-1, OPT_GSETTING_GAMMA); break;
				default: break;
			}
			break;
		case K_RIGHTARROW:
			switch(gsettings_cursor) {
				case 1: M_AdjustSlidersAdvanced(1, OPT_GSETTING_MAXFPS); break;
				case 2: M_AdjustSlidersAdvanced(1, OPT_GSETTING_FOV); break;
				case 3: M_AdjustSlidersAdvanced(1, OPT_GSETTING_GAMMA); break;
				default: break;
			}
			break;
		case K_UPARROW:
			S_LocalSound ("sounds/menu/navigate.wav");
			gsettings_cursor--;

			if (gsettings_cursor < 0)
				gsettings_cursor = GSETTINGS_ITEMS;
			break;
		case K_DOWNARROW:
			S_LocalSound ("sounds/menu/navigate.wav");
			gsettings_cursor++;

			if (gsettings_cursor > GSETTINGS_ITEMS)
				gsettings_cursor = 0;
			break;
		default: break;
	}
}

//
// NZ:P Control Settings
//

void M_Control_Settings_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_video; // lazy
	m_entersound = true;
}

static int csettings_cursor;

#ifdef VITA
void Vita_ToggleRearTouchPad (void)
{
	if (cl_enablereartouchpad.value == 0) {
		sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 0);
		sceTouchDisableTouchForce(SCE_TOUCH_PORT_BACK);
	} else {
		sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 1);
		sceTouchEnableTouchForce(SCE_TOUCH_PORT_BACK);
	}
}
#endif

//
// PSVita requires an extra menu option for disabling of the rear touch pad.
//
#ifdef VITA
#define CSETTINGS_ITEMS 		10
#else
#define CSETTINGS_ITEMS 		9
#endif

void M_Control_Settings_Draw (void)
{
	MENU_INITVARS();
	float r;

	// Menu Background
	if (paused_hack == false)
		Draw_BgMenu();

	// Fill black to make everything easier to see
	Draw_FillByColor(0, 0, 1280, 720, 0, 0, 0, 0.4);

	// Header
	DRAW_HEADER("CONTROL SETTINGS");

	DRAW_MENUOPTION(0, "Draw Crosshair", csettings_cursor, false);
	DRAW_MENUOPTION(1, "Aim Assist", csettings_cursor, false);
	DRAW_MENUOPTION(2, "Look Sensitivity", csettings_cursor, false);
	DRAW_BLANKOPTION("Look Acceleration", false);
	DRAW_MENUOPTION(4, "Look Inversion", csettings_cursor, false);
	DRAW_MENUOPTION(5, "Gyroscopic Aim", csettings_cursor, false);
	DRAW_MENUOPTION(6, "Gyro Mode", csettings_cursor, false);
	DRAW_MENUOPTION(7, "Gyro Sensitivity X", csettings_cursor, false);
	DRAW_MENUOPTION(8, "Gyro Sensitivity Y", csettings_cursor, false);

#ifdef VITA

	DRAW_MENUOPTION(9, "Rear TouchPad", csettings_cursor, false);
	DRAW_BACKBUTTON(10, csettings_cursor);

#else

	DRAW_BACKBUTTON(9, csettings_cursor);

#endif // VITA

	// Draw Crosshair
	if (crosshair.value == 0) {
		DRAW_SETTINGSVALUE(0, "Disabled");
	} else {
		DRAW_SETTINGSVALUE(0, "Enabled");
	}

	// Aim Assist
	if (in_aimassist.value == 0) {
		DRAW_SETTINGSVALUE(1, "Disabled");
	} else {
		DRAW_SETTINGSVALUE(1, "Enabled");
	}

	// Look Sensitivity
	r = (sensitivity.value - 1)/10;
	DRAW_SLIDER(2, r);

	// Look Inversion
	if (joy_invert.value == 0) {
		DRAW_SETTINGSVALUE(4, "Disabled");
	} else {
		DRAW_SETTINGSVALUE(4, "Enabled");
	}

	// Gyroscopic Aim
	if (motioncam.value == 0) {
		DRAW_SETTINGSVALUE(5, "Disabled");
	} else {
		DRAW_SETTINGSVALUE(5, "Enabled");
	}

	// Gyro Mode
	if (gyromode.value == 0) {
		DRAW_SETTINGSVALUE(6, "Always On");
	} else {
		DRAW_SETTINGSVALUE(6, "ADS Only");
	}

	// Gyro Sensitivity X
	r = (gyrosensx.value - 0.5)*(1.0/0.5);
	DRAW_SLIDER(7, r);

	// Gyro Sensitivity Y
	r = (gyrosensy.value - 0.5)*(1.0/0.5);
	DRAW_SLIDER(8, r);

#ifdef VITA

	// Rear TouchPad
	if (cl_enablereartouchpad.value == 0) {
		DRAW_SETTINGSVALUE(9, "Disabled");
	} else {
		DRAW_SETTINGSVALUE(9, "Enabled");
	}

#endif // VITA

	// Descriptions
	switch(csettings_cursor) {
		case 0: DRAW_DESCRIPTION("Toggle Crosshair in-game."); break;
		case 1: DRAW_DESCRIPTION("Toggle Assisted Aim to improve Targeting."); break;
		case 2: DRAW_DESCRIPTION("Adjust Look Sensitivity."); break;
		case 3: DRAW_DESCRIPTION("Adjust Look Acceleration."); break;
		case 4: DRAW_DESCRIPTION("Toggle inverted Camera control."); break;
		case 5: DRAW_DESCRIPTION("Toggle Gyroscopic Aiming."); break;
		case 6: DRAW_DESCRIPTION("Set to use Gyro Always or only when ADS."); break;
		case 7: DRAW_DESCRIPTION("Adjust Gyro Sensitivty on the X Axis."); break;
		case 8: DRAW_DESCRIPTION("Adjust Gyro Sensitivty on the Y Axis."); break;

#ifdef VITA

		case 9: DRAW_DESCRIPTION("Toggle support for the PSVita Rear TouchPad."); break;

#endif // VITA

	}	
}

void M_Control_Settings_Key (int key)
{
	switch(key) {
		case K_ABUTTON:
			S_LocalSound ("sounds/menu/enter.wav");
			switch(csettings_cursor) {
				case 0: Cvar_SetValue("crosshair", crosshair.value ? 0 : 1); break;
				case 1: Cvar_SetValue("in_aimassist", in_aimassist.value ? 0 : 1); break;
				//case 2: break;
				//case 3: break;
				case 4: Cvar_SetValue("joy_invert", joy_invert.value ? 0 : 1); break;
				case 5: Cvar_SetValue("motioncam", motioncam.value ? 0 : 1); break;
				case 6: Cvar_SetValue("gyromode", gyromode.value ? 0 : 1); break;
#ifdef VITA
				case 9: Cvar_SetValue("cl_enablereartouchpad", cl_enablereartouchpad.value ? 0 : 1); Vita_ToggleRearTouchPad(); break;
				case 10: M_Menu_Options_f (); break;
#else
				case 9: M_Menu_Options_f (); break;
#endif
				default: break;
			}
			break;
		case K_BBUTTON:
			M_Menu_Options_f ();
			break;
		case K_LEFTARROW:
			switch(csettings_cursor) {
				case 2: M_AdjustSlidersAdvanced(-1, OPT_CSETTING_LSENS); break;
				case 3: M_AdjustSlidersAdvanced(-1, OPT_CSETTING_LACC); break;
				case 7: M_AdjustSlidersAdvanced(-1, OPT_CSETTING_GSEX); break;
				case 8: M_AdjustSlidersAdvanced(-1, OPT_CSETTING_GSEY); break;
				default: break;
			}
			break;
		case K_RIGHTARROW:
			switch(csettings_cursor) {
				case 2: M_AdjustSlidersAdvanced(1, OPT_CSETTING_LSENS); break;
				case 3: M_AdjustSlidersAdvanced(1, OPT_CSETTING_LACC); break;
				case 7: M_AdjustSlidersAdvanced(1, OPT_CSETTING_GSEX); break;
				case 8: M_AdjustSlidersAdvanced(1, OPT_CSETTING_GSEY); break;
				default: break;
			}
			break;
		case K_UPARROW:
			S_LocalSound ("sounds/menu/navigate.wav");
			csettings_cursor--;

			if (csettings_cursor == 3)
				csettings_cursor = 2;

			if (csettings_cursor < 0)
				csettings_cursor = CSETTINGS_ITEMS;
			break;
		case K_DOWNARROW:
			S_LocalSound ("sounds/menu/navigate.wav");
			csettings_cursor++;

			if (csettings_cursor == 3)
				csettings_cursor = 4;

			if (csettings_cursor > CSETTINGS_ITEMS)
				csettings_cursor = 0;
			break;
		default: break;
	}
}

//=============================================================================
/* KEYS MENU */

char *bindnames[][2] =
{
#if 0 // Current consoles using this engine do have those buttons mapped to analogs
	{"+forward", 		"Walk Forward"},
	{"+back", 			"Walk Backward"},
	{"+moveleft", 		"Move Left"},
	{"+moveright", 		"Move Right"},
	{"+lookup", 		"Look Up"},
	{"+lookdown", 		"Look Down"},
	{"+left", 			"Look Left"},
	{"+right", 			"Look Right"},
#endif
	{"+jump",			"Jump"},
	{"+attack",			"Fire"},
	{"+aim", 			"Aim Down Sight"},
	{"+switch", 		"Switch Weapon"},
	{"+use", 			"Interact"},
	{"+reload", 		"Reload"},
	{"+knife", 			"Melee"},
	{"+grenade", 		"Grenade"},
	{"impulse 23", 		"Sprint"},
	{"impulse 30", 		"Crouch"}
};

#define	NUMCOMMANDS	(sizeof(bindnames)/sizeof(bindnames[0]))

static int	keys_cursor;
static qboolean	bind_grab;

void M_Menu_Keys_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_keys;
	m_entersound = true;
}


void M_FindKeysForCommand (const char *command, int *threekeys)
{
	int		count;
	int		j;
	int		l;
	char	*b;

	threekeys[0] = threekeys[1] = threekeys[2] = -1;
	l = strlen(command);
	count = 0;

	for (j = 0; j < MAX_KEYS; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
		{
			threekeys[count] = j;
			count++;
			if (count == 3)
				break;
		}
	}
}

void M_UnbindCommand (const char *command)
{
	int		j;
	int		l;
	char	*b;

	l = strlen(command);

	for (j = 0; j < MAX_KEYS; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
			Key_SetBinding (j, NULL);
	}
}

extern qpic_t	*pic_up, *pic_down;

extern qpic_t      *b_up;
extern qpic_t      *b_down;
extern qpic_t      *b_left;
extern qpic_t      *b_right;
extern qpic_t      *b_lthumb;
extern qpic_t      *b_rthumb;
extern qpic_t      *b_lshoulder;
extern qpic_t      *b_rshoulder;
extern qpic_t      *b_abutton;
extern qpic_t      *b_bbutton;
extern qpic_t      *b_ybutton;
extern qpic_t      *b_xbutton;
extern qpic_t      *b_lt;
extern qpic_t      *b_rt;

void M_Keys_Draw (void)
{
#ifdef VITA
	int y = 0;
#else
	int y = vid.height * 0.5;
#endif
	char* b;

	// Menu Background
	if (paused_hack == false)
		Draw_BgMenu();

	// Fill black to make everything easier to see
	Draw_FillByColor(0, 0, 1280, 720, 0, 0, 0, 0.4);

	// Header
	DRAW_HEADER("CONTROLS");

	if (bind_grab) {
#ifndef VITA
		Draw_ColoredStringScale(86, y + 305, "Press a key or button for this action", 1, 1, 1, 1, 1.5f);
#else
		Draw_ColoredStringScale((vid.width/2 - strlen("Press a key or button for this action")*8), y + 475, "Press a key or button for this action", 1, 1, 1, 1, 2.0f);
#endif // VITA
	} else {
#ifndef VITA
		Draw_ColoredStringScale(150, y + 305, "Press   to change,   to clear", 1, 1, 1, 1, 1.5f);
		Draw_StretchPic(220, y + 305, b_abutton, 16, 16);
		Draw_StretchPic(375, y + 305, b_ybutton, 16, 16);
#else
		Draw_ColoredStringScale((vid.width/2 - strlen("Press   to change,   to clear")*8), y + 475, "Press   to change,   to clear", 1, 1, 1, 1, 2.0f);
		Draw_StretchPic(340, y + 468, b_abutton, 28, 28);
		Draw_StretchPic(545, y + 468, b_ybutton, 28, 28);
#endif // VITA
	}

	for(int i = 0; i < (int)NUMCOMMANDS; i++) {
#ifndef VITA
		int y_offset = y + (55 + 15 * (i + 1));
		if (i == keys_cursor) {
			Draw_ColoredStringScale(10, y_offset, bindnames[i][1], 1, 0, 0, 1, 1.5f);
		} else {
			Draw_ColoredStringScale(10, y_offset, bindnames[i][1], 1, 1, 1, 1, 1.5f);
		}
#else
		int y_offset = y + (70 + 30 * (i + 1));
		if (i == keys_cursor) {
			Draw_ColoredStringScale(10, y_offset, bindnames[i][1], 1, 0, 0, 1, 2.0f);
		} else {
			Draw_ColoredStringScale(10, y_offset, bindnames[i][1], 1, 1, 1, 1, 2.0f);
		}
#endif // VITA

		for (int j = 0; j < 256; j++) {
			b = keybindings[j];

			if (!b) {
				continue;
			}	
				
			if (!strcmp (b, bindnames[i][0]))
			{
				qpic_t *btn_to_draw = GetButtonIcon(bindnames[i][0]);
#ifndef VITA
				if (!strcmp("LSHOULDER", bindnames[i][0]) || !strcmp("RSHOULDER", bindnames[i][0]))
					Draw_StretchPic(300, y_offset + 4, btn_to_draw, 16, 8);
				else
					Draw_StretchPic(300, y_offset, btn_to_draw, 16, 16);
#else
				if (!strcmp("LSHOULDER", bindnames[i][0]) || !strcmp("RSHOULDER", bindnames[i][0]))
					Draw_StretchPic(400, y_offset + 4, btn_to_draw, 28, 14);
				else
					Draw_StretchPic(400, y_offset, btn_to_draw, 28, 28);
#endif // VITA
                break;
			}
		}
	}

	DRAW_BACKBUTTON(NUMCOMMANDS, keys_cursor);

	// Cursor Flashing
	if (keys_cursor != NUMCOMMANDS) {
#ifndef VITA
		M_DrawCharacter (282, y + 58 + (keys_cursor + 1)*15, 12+((int)(realtime*4)&1));
#else
		Draw_CharacterScale(375, y + 74 + (keys_cursor + 1)*30, 12+((int)(realtime*4)&1), 2.0f);
#endif // VITA
	}
}


void M_Keys_Key (int k)
{
	char	cmd[80];
	int		keys[3];

	if (bind_grab)
	{	// defining a key
		S_LocalSound ("sounds/menu/enter.wav");
		if ((k != K_ESCAPE) && (k != '`'))
		{
			sprintf (cmd, "bind \"%s\" \"%s\"\n", Key_KeynumToString (k), bindnames[keys_cursor][0]);
			Cbuf_InsertText (cmd);
		}

		bind_grab = false;
		IN_Deactivate(modestate == MS_WINDOWED); // deactivate because we're returning to the menu
		return;
	}

	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:	
		M_Menu_Options_f ();
		break;

	case K_LEFTARROW:
	case K_UPARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		keys_cursor--;
		if (keys_cursor < 0)
			keys_cursor = NUMCOMMANDS;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		keys_cursor++;
		if (keys_cursor >= (int)NUMCOMMANDS + 1)
			keys_cursor = 0;
		break;

	case K_ENTER:		// go into bind mode
	case K_KP_ENTER:
	case K_ABUTTON:
		if (keys_cursor == NUMCOMMANDS)
			M_Menu_Options_f();
		else {
			M_FindKeysForCommand (bindnames[keys_cursor][0], keys);
			S_LocalSound ("sounds/menu/enter.wav");
			if (keys[2] != -1)
				M_UnbindCommand (bindnames[keys_cursor][0]);
			bind_grab = true;
			IN_Activate(); // activate to allow mouse key binding
		}
		break;

	case K_BACKSPACE:	// delete bindings
	case K_DEL:
	case K_YBUTTON:
		S_LocalSound ("sounds/menu/enter.wav");
		M_UnbindCommand (bindnames[keys_cursor][0]);
		break;
	}
}

//=============================================================================
/* CONSOLE OSK */

#define CHAR_SIZE 8
#define MAX_Y 8
#define MAX_X 12

#define MAX_CHAR_LINE 36
#define MAX_CHAR      72

int  osk_pos_x = 0;
int  osk_pos_y = 0;
int  max_len   = 0;
int  m_old_state = 0;

char* osk_out_buff = NULL;
char  osk_buffer[128];

char *osk_help [] =
{
	"CONFIRM: ",
	"    Y    ",
	"CANCEL:  ",
	"    B    ",
	"DELETE:  ",
	"    X    ",
	"ADD CHAR:",
	"    A    ",
	""
};

char *osk_text [] =
{
	" 1 2 3 4 5 6 7 8 9 0 - = ` ",
	" q w e r t y u i o p [ ]   ",
	"   a s d f g h j k l ; ' \\ ",
	"     z x c v b n m   , . / ",
	"                           ",
	" ! @ # $ % ^ & * ( ) _ + ~ ",
	" Q W E R T Y U I O P { }   ",
	"   A S D F G H J K L : \" | ",
	"     Z X C V B N M   < > ? "
};

void M_OSK_Draw (void) {

	int x,y;
	int i;

	char *selected_line = osk_text[osk_pos_y];
	char selected_char[2];

	GL_SetCanvas(CANVAS_MENU);

	selected_char[0] = selected_line[1+(2*osk_pos_x)];
	selected_char[1] = '\0';
	if (selected_char[0] == ' ' || selected_char[0] == '\t')
		selected_char[0] = 'X';

	y = 20;
	x = 16;

	M_DrawTextBox (10, 10, 		     26, 10);
	M_DrawTextBox (10+(26*CHAR_SIZE),    10,  10, 10);
	M_DrawTextBox (10, 10+(10*CHAR_SIZE),36,  3);

	for(i=0;i<=MAX_Y;i++)
	{
		M_PrintWhite (x, y+(CHAR_SIZE*i), osk_text[i]);
		if (i % 2 == 0)
			M_Print      (x+(27*CHAR_SIZE), y+(CHAR_SIZE*i), osk_help[i]);
		else
			M_PrintWhite (x+(27*CHAR_SIZE), y+(CHAR_SIZE*i), osk_help[i]);
	}

	int text_len = strlen(osk_buffer);
	if (text_len > MAX_CHAR_LINE) {

		char oneline[MAX_CHAR_LINE+1];
		strncpy(oneline,osk_buffer,MAX_CHAR_LINE);
		oneline[MAX_CHAR_LINE] = '\0';

		M_Print (x+4, y+4+(CHAR_SIZE*(MAX_Y+2)), oneline );

		strncpy(oneline,osk_buffer+MAX_CHAR_LINE, text_len - MAX_CHAR_LINE);
		oneline[text_len - MAX_CHAR_LINE] = '\0';

		M_Print (x+4, y+4+(CHAR_SIZE*(MAX_Y+3)), oneline );
		M_PrintWhite (x+4+(CHAR_SIZE*(text_len - MAX_CHAR_LINE)), y+4+(CHAR_SIZE*(MAX_Y+3)),"_");
	}
	else {
		M_Print (x+4, y+4+(CHAR_SIZE*(MAX_Y+2)), osk_buffer );
		M_PrintWhite (x+4+(CHAR_SIZE*(text_len)), y+4+(CHAR_SIZE*(MAX_Y+2)),"_");
	}
	M_Print      (x+((((osk_pos_x)*2)+1)*CHAR_SIZE), y+(osk_pos_y*CHAR_SIZE), selected_char);
}

//=============================================================================
/* VIDEO MENU */

void M_Menu_Video_f (void)
{
	(*vid_menucmdfn) (); //johnfitz
}


void M_Video_Draw (void)
{
	(*vid_menudrawfn) ();
}


void M_Video_Key (int key)
{
	(*vid_menukeyfn) (key);
}

//=============================================================================
/* HELP MENU */

int		help_page;
#define	NUM_CREDITS_PAGES	1


void M_Menu_Credits_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	old_m_state = m_state;
	m_state = m_credits;
	m_entersound = true;
	help_page = 0;
}



void M_Credits_Draw (void)
{
	MENU_INITVARS();

	// Menu Background
	Draw_BgMenu();

	// Fill black to make everything easier to see
	Draw_FillByColor(0, 0, 1280, 720, 0, 0, 0, 0.4);

	// Version String
	DRAW_VERSIONSTRING();

	// Header
	DRAW_HEADER("CREDITS");

	DRAW_CREDITLINE(0, "Programming:");
	DRAW_CREDITLINE(1, "Blubswillrule, Jukki, DR_Mabuse1981, Naievil, Cypress");
	DRAW_CREDITLINE(2, "ScatterBox");
	DRAW_CREDITLINE(3, "");
	DRAW_CREDITLINE(4, "Models:");
	DRAW_CREDITLINE(5, "Blubswillrule, Ju[s]tice, Derped_Crusader");
	DRAW_CREDITLINE(6, "");
	DRAW_CREDITLINE(7, "GFX:");
	DRAW_CREDITLINE(8, "Blubswillrule, Ju[s]tice, Cypress, Derped_Crusader");
	DRAW_CREDITLINE(9, "");
	DRAW_CREDITLINE(10, "Sounds/Music:");
	DRAW_CREDITLINE(11, "Blubswillrule, Biodude, Cypress, Marty P.");
	DRAW_CREDITLINE(12, "");
	DRAW_CREDITLINE(13, "Special Thanks:");
	DRAW_CREDITLINE(14, "- Spike, Eukara: [FTEQW]");
	DRAW_CREDITLINE(15, "- Shpuld: [CleanQC4FTE]");
	DRAW_CREDITLINE(16, "- Crow_Bar, st1x51: [dQuake(plus)]");
	DRAW_CREDITLINE(17, "- fgsfdsfgs: [Quakespasm-NX]");
	DRAW_CREDITLINE(18, "- Rinnegatamante: [Initial VITA Port, VITA Auto-Updater]");
	DRAW_CREDITLINE(19, "- Azenn: [GFX Help]");
	DRAW_CREDITLINE(20, "- BCDeshiG: [Extensive Testing]");
	
	DRAW_BACKBUTTON(0, 0);
}


void M_Credits_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case K_ENTER:
	case K_BBUTTON:
	case K_KP_ENTER:
	case K_ABUTTON:
		M_Menu_Main_f ();
		break;

	/*case K_UPARROW:
	case K_RIGHTARROW:
		m_entersound = true;
		if (++help_page >= NUM_CREDITS_PAGES)
			help_page = 0;
		break;

	case K_DOWNARROW:
	case K_LEFTARROW:
		m_entersound = true;
		if (--help_page < 0)
			help_page = NUM_CREDITS_PAGES-1;
		break;*/
	default: break;
	}

}

//=============================================================================
/* QUIT MENU */

int		msgNumber;
enum m_state_e	m_quit_prevstate;

void M_Menu_Quit_f (void)
{
	if (m_state == m_quit)
		return;
	wasInMenus = (key_dest == key_menu);
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_quit_prevstate = m_state;
	old_m_state = m_state;
	m_state = m_quit;
	m_entersound = true;
	msgNumber = rand()&7;
}


void M_Quit_Key (int key)
{
	if (key == K_ESCAPE)
	{
		if (wasInMenus)
		{
			m_state = m_quit_prevstate;
			m_entersound = true;
		}
		else
		{
			IN_Activate();
			key_dest = key_game;
			m_state = m_none;
		}
	}
}


void M_Quit_Char (int key)
{
	switch (key)
	{
	case 'n':
	case 'N':
		if (wasInMenus)
		{
			m_state = m_quit_prevstate;
			m_entersound = true;
		}
		else
		{
			IN_Activate();
			key_dest = key_game;
			m_state = m_none;
		}
		break;

	case 'y':
	case 'Y':
		IN_Deactivate(modestate == MS_WINDOWED);
		key_dest = key_console;
		Host_Quit_f ();
		break;

	default:
		break;
	}

}


qboolean M_Quit_TextEntry (void)
{
	return true;
}


void M_Quit_Draw (void) //johnfitz -- modified for new quit message
{
	char	msg1[] = "Nazi Zombies Portable";
	char	msg2[] = "by NZP Dev Team"; /* msg2/msg3 are mostly [40] */
	char	msg3[] = "Press a to quit";
	int		boxlen;

	if (wasInMenus)
	{
		m_state = m_quit_prevstate;
		m_recursiveDraw = true;
		M_Draw ();
		m_state = m_quit;
	}

	//okay, this is kind of fucked up.  M_DrawTextBox will always act as if
	//width is even. Also, the width and lines values are for the interior of the box,
	//but the x and y values include the border.
	boxlen = q_max(strlen(msg1), q_max((sizeof(msg2)-1),(sizeof(msg3)-1))) + 1;
	if (boxlen & 1) boxlen++;
	M_DrawTextBox	(160-4*(boxlen+2), 76, boxlen, 4);

	//now do the text
	M_Print			(160-4*strlen(msg1), 88, msg1);
	M_Print			(160-4*(sizeof(msg2)-1), 96, msg2);
	M_PrintWhite		(160-4*(sizeof(msg3)-1), 104, msg3);
}

//=============================================================================
/* LAN CONFIG MENU */

int		lanConfig_cursor = -1;
int		lanConfig_cursor_table [] = {72, 92, 124};
#define NUM_LANCONFIG_CMDS	3

int 	lanConfig_port;
char	lanConfig_portname[6];
char	lanConfig_joinname[22];

void M_Menu_LanConfig_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_lanconfig;
	m_entersound = true;
	if (lanConfig_cursor == -1)
	{
		if (JoiningGame && TCPIPConfig)
			lanConfig_cursor = 2;
		else
			lanConfig_cursor = 1;
	}
	if (StartingGame && lanConfig_cursor == 2)
		lanConfig_cursor = 1;
	lanConfig_port = DEFAULTnet_hostport;
	sprintf(lanConfig_portname, "%u", lanConfig_port);

	m_return_onerror = false;
	m_return_reason[0] = 0;
}


void M_LanConfig_Draw (void)
{
	qpic_t	*p;
	int		basex;
	const char	*startJoin;
	const char	*protocol;

	// M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	// p = Draw_CachePic ("gfx/p_multi.lmp");
	// basex = (320-p->width)/2;
	// M_DrawPic (basex, 4, p);

	if (StartingGame)
		startJoin = "New Game";
	else
		startJoin = "Join Game";
	if (IPXConfig)
		protocol = "IPX";
	else
		protocol = "TCP/IP";
	M_Print (basex, 32, va ("%s - %s", startJoin, protocol));
	basex += 8;

	M_Print (basex, 52, "Address:");
	if (IPXConfig)
		M_Print (basex+9*8, 52, my_ipx_address);
	else
		M_Print (basex+9*8, 52, my_tcpip_address);

	M_Print (basex, lanConfig_cursor_table[0], "Port");
	M_DrawTextBox (basex+8*8, lanConfig_cursor_table[0]-8, 6, 1);
	M_Print (basex+9*8, lanConfig_cursor_table[0], lanConfig_portname);

	if (JoiningGame)
	{
		M_Print (basex, lanConfig_cursor_table[1], "Search for local games...");
		M_Print (basex, 108, "Join game at:");
		M_DrawTextBox (basex+8, lanConfig_cursor_table[2]-8, 22, 1);
		M_Print (basex+16, lanConfig_cursor_table[2], lanConfig_joinname);
	}
	else
	{
		M_DrawTextBox (basex, lanConfig_cursor_table[1]-8, 2, 1);
		M_Print (basex+8, lanConfig_cursor_table[1], "OK");
	}

	M_DrawCharacter (basex-8, lanConfig_cursor_table [lanConfig_cursor], 12+((int)(realtime*4)&1));

	if (lanConfig_cursor == 0)
		M_DrawCharacter (basex+9*8 + 8*strlen(lanConfig_portname), lanConfig_cursor_table [0], 10+((int)(realtime*4)&1));

	if (lanConfig_cursor == 2)
		M_DrawCharacter (basex+16 + 8*strlen(lanConfig_joinname), lanConfig_cursor_table [2], 10+((int)(realtime*4)&1));

	if (*m_return_reason)
		M_PrintWhite (basex, 148, m_return_reason);
}


void M_LanConfig_Key (int key)
{
	int		l;

	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_Net_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		lanConfig_cursor--;
		if (lanConfig_cursor < 0)
			lanConfig_cursor = NUM_LANCONFIG_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		lanConfig_cursor++;
		if (lanConfig_cursor >= NUM_LANCONFIG_CMDS)
			lanConfig_cursor = 0;
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		if (lanConfig_cursor == 0)
			break;

		m_entersound = true;

		M_ConfigureNetSubsystem ();

		if (lanConfig_cursor == 1)
		{
			if (StartingGame)
			{
				M_Menu_GameOptions_f ();
				break;
			}
			M_Menu_Search_f();
			break;
		}

		if (lanConfig_cursor == 2)
		{
			m_return_state = m_state;
			m_return_onerror = true;
			IN_Activate();
			key_dest = key_game;
			m_state = m_none;
			Cbuf_AddText ( va ("connect \"%s\"\n", lanConfig_joinname) );
			break;
		}

		break;

	case K_BACKSPACE:
		if (lanConfig_cursor == 0)
		{
			if (strlen(lanConfig_portname))
				lanConfig_portname[strlen(lanConfig_portname)-1] = 0;
		}

		if (lanConfig_cursor == 2)
		{
			if (strlen(lanConfig_joinname))
				lanConfig_joinname[strlen(lanConfig_joinname)-1] = 0;
		}
		break;
	}

	if (StartingGame && lanConfig_cursor == 2)
	{
		if (key == K_UPARROW)
			lanConfig_cursor = 1;
		else
			lanConfig_cursor = 0;
	}

	l =  Q_atoi(lanConfig_portname);
	if (l > 65535)
		l = lanConfig_port;
	else
		lanConfig_port = l;
	sprintf(lanConfig_portname, "%u", lanConfig_port);
}


void M_LanConfig_Char (int key)
{
	int l;

	switch (lanConfig_cursor)
	{
	case 0:
		if (key < '0' || key > '9')
			return;
		l = strlen(lanConfig_portname);
		if (l < 5)
		{
			lanConfig_portname[l+1] = 0;
			lanConfig_portname[l] = key;
		}
		break;
	case 2:
		l = strlen(lanConfig_joinname);
		if (l < 21)
		{
			lanConfig_joinname[l+1] = 0;
			lanConfig_joinname[l] = key;
		}
		break;
	}
}


qboolean M_LanConfig_TextEntry (void)
{
	return (lanConfig_cursor == 0 || lanConfig_cursor == 2);
}

//=============================================================================
/* GAME OPTIONS MENU */

typedef struct
{
	const char	*name;
	const char	*description;
} level_t;

level_t		levels[] =
{
	{"start", "Entrance"},	// 0

	{"e1m1", "Slipgate Complex"},				// 1
	{"e1m2", "Castle of the Damned"},
	{"e1m3", "The Necropolis"},
	{"e1m4", "The Grisly Grotto"},
	{"e1m5", "Gloom Keep"},
	{"e1m6", "The Door To Chthon"},
	{"e1m7", "The House of Chthon"},
	{"e1m8", "Ziggurat Vertigo"},

	{"e2m1", "The Installation"},				// 9
	{"e2m2", "Ogre Citadel"},
	{"e2m3", "Crypt of Decay"},
	{"e2m4", "The Ebon Fortress"},
	{"e2m5", "The Wizard's Manse"},
	{"e2m6", "The Dismal Oubliette"},
	{"e2m7", "Underearth"},

	{"e3m1", "Termination Central"},			// 16
	{"e3m2", "The Vaults of Zin"},
	{"e3m3", "The Tomb of Terror"},
	{"e3m4", "Satan's Dark Delight"},
	{"e3m5", "Wind Tunnels"},
	{"e3m6", "Chambers of Torment"},
	{"e3m7", "The Haunted Halls"},

	{"e4m1", "The Sewage System"},				// 23
	{"e4m2", "The Tower of Despair"},
	{"e4m3", "The Elder God Shrine"},
	{"e4m4", "The Palace of Hate"},
	{"e4m5", "Hell's Atrium"},
	{"e4m6", "The Pain Maze"},
	{"e4m7", "Azure Agony"},
	{"e4m8", "The Nameless City"},

	{"end", "Shub-Niggurath's Pit"},			// 31

	{"dm1", "Place of Two Deaths"},				// 32
	{"dm2", "Claustrophobopolis"},
	{"dm3", "The Abandoned Base"},
	{"dm4", "The Bad Place"},
	{"dm5", "The Cistern"},
	{"dm6", "The Dark Zone"}
};

//MED 01/06/97 added hipnotic levels
level_t     hipnoticlevels[] =
{
	{"start", "Command HQ"},	// 0

	{"hip1m1", "The Pumping Station"},			// 1
	{"hip1m2", "Storage Facility"},
	{"hip1m3", "The Lost Mine"},
	{"hip1m4", "Research Facility"},
	{"hip1m5", "Military Complex"},

	{"hip2m1", "Ancient Realms"},				// 6
	{"hip2m2", "The Black Cathedral"},
	{"hip2m3", "The Catacombs"},
	{"hip2m4", "The Crypt"},
	{"hip2m5", "Mortum's Keep"},
	{"hip2m6", "The Gremlin's Domain"},

	{"hip3m1", "Tur Torment"},				// 12
	{"hip3m2", "Pandemonium"},
	{"hip3m3", "Limbo"},
	{"hip3m4", "The Gauntlet"},

	{"hipend", "Armagon's Lair"},				// 16

	{"hipdm1", "The Edge of Oblivion"}			// 17
};

//PGM 01/07/97 added rogue levels
//PGM 03/02/97 added dmatch level
level_t		roguelevels[] =
{
	{"start",	"Split Decision"},
	{"r1m1",	"Deviant's Domain"},
	{"r1m2",	"Dread Portal"},
	{"r1m3",	"Judgement Call"},
	{"r1m4",	"Cave of Death"},
	{"r1m5",	"Towers of Wrath"},
	{"r1m6",	"Temple of Pain"},
	{"r1m7",	"Tomb of the Overlord"},
	{"r2m1",	"Tempus Fugit"},
	{"r2m2",	"Elemental Fury I"},
	{"r2m3",	"Elemental Fury II"},
	{"r2m4",	"Curse of Osiris"},
	{"r2m5",	"Wizard's Keep"},
	{"r2m6",	"Blood Sacrifice"},
	{"r2m7",	"Last Bastion"},
	{"r2m8",	"Source of Evil"},
	{"ctf1",    "Division of Change"}
};

typedef struct
{
	const char	*description;
	int		firstLevel;
	int		levels;
} episode_t;

episode_t	episodes[] =
{
	{"Welcome to Quake", 0, 1},
	{"Doomed Dimension", 1, 8},
	{"Realm of Black Magic", 9, 7},
	{"Netherworld", 16, 7},
	{"The Elder World", 23, 8},
	{"Final Level", 31, 1},
	{"Deathmatch Arena", 32, 6}
};

//MED 01/06/97  added hipnotic episodes
episode_t   hipnoticepisodes[] =
{
	{"Scourge of Armagon", 0, 1},
	{"Fortress of the Dead", 1, 5},
	{"Dominion of Darkness", 6, 6},
	{"The Rift", 12, 4},
	{"Final Level", 16, 1},
	{"Deathmatch Arena", 17, 1}
};

//PGM 01/07/97 added rogue episodes
//PGM 03/02/97 added dmatch episode
episode_t	rogueepisodes[] =
{
	{"Introduction", 0, 1},
	{"Hell's Fortress", 1, 7},
	{"Corridors of Time", 8, 8},
	{"Deathmatch Arena", 16, 1}
};

int	startepisode;
int	startlevel;
int maxplayers;
qboolean m_serverInfoMessage = false;
double m_serverInfoMessageTime;

void M_Menu_GameOptions_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_gameoptions;
	m_entersound = true;
	if (maxplayers == 0)
		maxplayers = svs.maxclients;
	if (maxplayers < 2)
		maxplayers = svs.maxclientslimit;
}


int gameoptions_cursor_table[] = {40, 56, 64, 72, 80, 88, 96, 112, 120};
#define	NUM_GAMEOPTIONS	9
int		gameoptions_cursor;

void M_GameOptions_Draw (void)
{
	qpic_t	*p;
	int		x;

	// M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	// p = Draw_CachePic ("gfx/p_multi.lmp");
	// M_DrawPic ( (320-p->width)/2, 4, p);

	M_DrawTextBox (152, 32, 10, 1);
	M_Print (160, 40, "begin game");

	M_Print (0, 56, "      Max players");
	M_Print (160, 56, va("%i", maxplayers) );

	M_Print (0, 64, "        Game Type");
	if (coop.value)
		M_Print (160, 64, "Cooperative");
	else
		M_Print (160, 64, "Deathmatch");

	M_Print (0, 72, "        Teamplay");
	if (rogue)
	{
		const char *msg;

		switch((int)teamplay.value)
		{
			case 1: msg = "No Friendly Fire"; break;
			case 2: msg = "Friendly Fire"; break;
			case 3: msg = "Tag"; break;
			case 4: msg = "Capture the Flag"; break;
			case 5: msg = "One Flag CTF"; break;
			case 6: msg = "Three Team CTF"; break;
			default: msg = "Off"; break;
		}
		M_Print (160, 72, msg);
	}
	else
	{
		const char *msg;

		switch((int)teamplay.value)
		{
			case 1: msg = "No Friendly Fire"; break;
			case 2: msg = "Friendly Fire"; break;
			default: msg = "Off"; break;
		}
		M_Print (160, 72, msg);
	}

	M_Print (0, 80, "            Skill");
	if (skill.value == 0)
		M_Print (160, 80, "Easy difficulty");
	else if (skill.value == 1)
		M_Print (160, 80, "Normal difficulty");
	else if (skill.value == 2)
		M_Print (160, 80, "Hard difficulty");
	else
		M_Print (160, 80, "Nightmare difficulty");

	M_Print (0, 88, "       Frag Limit");
	if (fraglimit.value == 0)
		M_Print (160, 88, "none");
	else
		M_Print (160, 88, va("%i frags", (int)fraglimit.value));

	M_Print (0, 96, "       Time Limit");
	if (timelimit.value == 0)
		M_Print (160, 96, "none");
	else
		M_Print (160, 96, va("%i minutes", (int)timelimit.value));

	M_Print (0, 112, "         Episode");
	// MED 01/06/97 added hipnotic episodes
	if (hipnotic)
		M_Print (160, 112, hipnoticepisodes[startepisode].description);
	// PGM 01/07/97 added rogue episodes
	else if (rogue)
		M_Print (160, 112, rogueepisodes[startepisode].description);
	else
		M_Print (160, 112, episodes[startepisode].description);

	M_Print (0, 120, "           Level");
	// MED 01/06/97 added hipnotic episodes
	if (hipnotic)
	{
		M_Print (160, 120, hipnoticlevels[hipnoticepisodes[startepisode].firstLevel + startlevel].description);
		M_Print (160, 128, hipnoticlevels[hipnoticepisodes[startepisode].firstLevel + startlevel].name);
	}
	// PGM 01/07/97 added rogue episodes
	else if (rogue)
	{
		M_Print (160, 120, roguelevels[rogueepisodes[startepisode].firstLevel + startlevel].description);
		M_Print (160, 128, roguelevels[rogueepisodes[startepisode].firstLevel + startlevel].name);
	}
	else
	{
		M_Print (160, 120, levels[episodes[startepisode].firstLevel + startlevel].description);
		M_Print (160, 128, levels[episodes[startepisode].firstLevel + startlevel].name);
	}

// line cursor
	M_DrawCharacter (144, gameoptions_cursor_table[gameoptions_cursor], 12+((int)(realtime*4)&1));

	if (m_serverInfoMessage)
	{
		if ((realtime - m_serverInfoMessageTime) < 5.0)
		{
			x = (320-26*8)/2;
			M_DrawTextBox (x, 138, 24, 4);
			x += 8;
			M_Print (x, 146, "  More than 4 players   ");
			M_Print (x, 154, " requires using command ");
			M_Print (x, 162, "line parameters; please ");
			M_Print (x, 170, "   see techinfo.txt.    ");
		}
		else
		{
			m_serverInfoMessage = false;
		}
	}
}


void M_NetStart_Change (int dir)
{
	int count;
	float	f;

	switch (gameoptions_cursor)
	{
	case 1:
		maxplayers += dir;
		if (maxplayers > svs.maxclientslimit)
		{
			maxplayers = svs.maxclientslimit;
			m_serverInfoMessage = true;
			m_serverInfoMessageTime = realtime;
		}
		if (maxplayers < 2)
			maxplayers = 2;
		break;

	case 2:
		Cvar_Set ("coop", coop.value ? "0" : "1");
		break;

	case 3:
		count = (rogue) ? 6 : 2;
		f = teamplay.value + dir;
		if (f > count)	f = 0;
		else if (f < 0)	f = count;
		Cvar_SetValue ("teamplay", f);
		break;

	case 4:
		f = skill.value + dir;
		if (f > 3)	f = 0;
		else if (f < 0)	f = 3;
		Cvar_SetValue ("skill", f);
		break;

	case 5:
		f = fraglimit.value + dir * 10;
		if (f > 100)	f = 0;
		else if (f < 0)	f = 100;
		Cvar_SetValue ("fraglimit", f);
		break;

	case 6:
		f = timelimit.value + dir * 5;
		if (f > 60)	f = 0;
		else if (f < 0)	f = 60;
		Cvar_SetValue ("timelimit", f);
		break;

	case 7:
		startepisode += dir;
	//MED 01/06/97 added hipnotic count
		if (hipnotic)
			count = 6;
	//PGM 01/07/97 added rogue count
	//PGM 03/02/97 added 1 for dmatch episode
		else if (rogue)
			count = 4;
		else if (registered.value)
			count = 7;
		else
			count = 2;

		if (startepisode < 0)
			startepisode = count - 1;

		if (startepisode >= count)
			startepisode = 0;

		startlevel = 0;
		break;

	case 8:
		startlevel += dir;
	//MED 01/06/97 added hipnotic episodes
		if (hipnotic)
			count = hipnoticepisodes[startepisode].levels;
	//PGM 01/06/97 added hipnotic episodes
		else if (rogue)
			count = rogueepisodes[startepisode].levels;
		else
			count = episodes[startepisode].levels;

		if (startlevel < 0)
			startlevel = count - 1;

		if (startlevel >= count)
			startlevel = 0;
		break;
	}
}

void M_GameOptions_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_Net_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		gameoptions_cursor--;
		if (gameoptions_cursor < 0)
			gameoptions_cursor = NUM_GAMEOPTIONS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		gameoptions_cursor++;
		if (gameoptions_cursor >= NUM_GAMEOPTIONS)
			gameoptions_cursor = 0;
		break;

	case K_LEFTARROW:
		if (gameoptions_cursor == 0)
			break;
		S_LocalSound ("sounds/menu/navigate.wav");
		M_NetStart_Change (-1);
		break;

	case K_RIGHTARROW:
		if (gameoptions_cursor == 0)
			break;
		S_LocalSound ("sounds/menu/navigate.wav");
		M_NetStart_Change (1);
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		S_LocalSound ("sounds/menu/enter.wav");
		if (gameoptions_cursor == 0)
		{
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			Cbuf_AddText ("listen 0\n");	// so host_netport will be re-examined
			Cbuf_AddText ( va ("maxplayers %u\n", maxplayers) );
			SCR_BeginLoadingPlaque ();

			if (hipnotic)
				Cbuf_AddText ( va ("map %s\n", hipnoticlevels[hipnoticepisodes[startepisode].firstLevel + startlevel].name) );
			else if (rogue)
				Cbuf_AddText ( va ("map %s\n", roguelevels[rogueepisodes[startepisode].firstLevel + startlevel].name) );
			else
				Cbuf_AddText ( va ("map %s\n", levels[episodes[startepisode].firstLevel + startlevel].name) );

			return;
		}

		M_NetStart_Change (1);
		break;
	}
}

//=============================================================================
/* SEARCH MENU */

qboolean	searchComplete = false;
double		searchCompleteTime;

void M_Menu_Search_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_search;
	m_entersound = false;
	slistSilent = true;
	slistLocal = false;
	searchComplete = false;
	NET_Slist_f();

}


void M_Search_Draw (void)
{
	qpic_t	*p;
	int x;

	// p = Draw_CachePic ("gfx/p_multi.lmp");
	// M_DrawPic ( (320-p->width)/2, 4, p);
	x = (320/2) - ((12*8)/2) + 4;
	M_DrawTextBox (x-8, 32, 12, 1);
	M_Print (x, 40, "Searching...");

	if(slistInProgress)
	{
		NET_Poll();
		return;
	}

	if (! searchComplete)
	{
		searchComplete = true;
		searchCompleteTime = realtime;
	}

	if (hostCacheCount)
	{
		M_Menu_ServerList_f ();
		return;
	}

	M_PrintWhite ((320/2) - ((22*8)/2), 64, "No Quake servers found");
	if ((realtime - searchCompleteTime) < 3.0)
		return;

	M_Menu_LanConfig_f ();
}


void M_Search_Key (int key)
{
}

//=============================================================================
/* SLIST MENU */

int		slist_cursor;
qboolean slist_sorted;

void M_Menu_ServerList_f (void)
{
	IN_Deactivate(modestate == MS_WINDOWED);
	key_dest = key_menu;
	m_state = m_slist;
	m_entersound = true;
	slist_cursor = 0;
	m_return_onerror = false;
	m_return_reason[0] = 0;
	slist_sorted = false;
}


void M_ServerList_Draw (void)
{
	int	n;
	qpic_t	*p;

	if (!slist_sorted)
	{
		slist_sorted = true;
		NET_SlistSort ();
	}

	// p = Draw_CachePic ("gfx/p_multi.lmp");
	// M_DrawPic ( (320-p->width)/2, 4, p);
	for (n = 0; n < hostCacheCount; n++)
		M_Print (16, 32 + 8*n, NET_SlistPrintServer (n));
	M_DrawCharacter (0, 32 + slist_cursor*8, 12+((int)(realtime*4)&1));

	if (*m_return_reason)
		M_PrintWhite (16, 148, m_return_reason);
}


void M_ServerList_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_LanConfig_f ();
		break;

	case K_SPACE:
		M_Menu_Search_f ();
		break;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		slist_cursor--;
		if (slist_cursor < 0)
			slist_cursor = hostCacheCount - 1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("sounds/menu/navigate.wav");
		slist_cursor++;
		if (slist_cursor >= hostCacheCount)
			slist_cursor = 0;
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		S_LocalSound ("sounds/menu/enter.wav");
		m_return_state = m_state;
		m_return_onerror = true;
		slist_sorted = false;
		IN_Activate();
		key_dest = key_game;
		m_state = m_none;
		Cbuf_AddText ( va ("connect \"%s\"\n", NET_SlistPrintServerName(slist_cursor)) );
		break;

	default:
		break;
	}

}

//=============================================================================
/* Menu Subsystem */


void M_Init (void)
{
	Cmd_AddCommand ("togglemenu", M_ToggleMenu_f);
	Cmd_AddCommand ("menu_main", M_Menu_Main_f);
	Cmd_AddCommand ("menu_singleplayer", M_Menu_SinglePlayer_f);
	Cmd_AddCommand ("menu_multiplayer", M_Menu_MultiPlayer_f);
	Cmd_AddCommand ("menu_setup", M_Menu_Setup_f);
	Cmd_AddCommand ("menu_options", M_Menu_Options_f);
	Cmd_AddCommand ("menu_keys", M_Menu_Keys_f);
	Cmd_AddCommand ("menu_video", M_Menu_Video_f);
	Cmd_AddCommand ("help", M_Menu_Credits_f);
	Cmd_AddCommand ("menu_quit", M_Menu_Quit_f);
#ifdef VITA
	Cmd_AddCommand ("savea", Save_Achivements);
	Cmd_AddCommand ("loada", Load_Achivements);
#endif
	Cvar_RegisterVariable(&cl_enablereartouchpad);

	//Sys_FileOpenRead (va("%s/maps/%s.way",com_gamedir, sv.name), &h);
	//Sys_FileOpenRead(va("%s/version.txt", com_gamedir));

	// Snag the game version
	long length;
	FILE* f = fopen(va("%s/version.txt", com_gamedir), "rb");

	if (f)
	{
		fseek (f, 0, SEEK_END);
		length = ftell (f);
		fseek (f, 0, SEEK_SET);
		game_build_date = malloc(length);

		if (game_build_date)
			fread (game_build_date, 1, length, f);

		fclose (f);
	} else {
		game_build_date = "version.txt not found.";
	}

	Map_Finder();
}

qboolean music_init = false;

void M_Draw (void)
{
	if (m_state == m_none || key_dest != key_menu)
		return;

	if (!music_init) {
		Cbuf_AddText("music tensioned_by_the_damned\n");
		Cbuf_AddText("music_loop 1\n");
		music_init = true;
	}

	if (!m_recursiveDraw)
	{
		//scr_copyeverything = 1;
		
		if (scr_con_current)
		{
			Draw_ConsoleBackground ();
			S_ExtraUpdate ();
		}

		Draw_FadeScreen (); //johnfitz -- fade even if console fills screen
	}
	else
	{
		m_recursiveDraw = false;
	}

	GL_SetCanvas(CANVAS_USEPRINT);

	switch (m_state)
	{
	case m_none:
		break;

	case m_start:
		//Menu_Background_Draw (MENU_START);
		M_Start_Menu_Draw ();
		break;
	case m_main:
		//Menu_Background_Draw (MENU_MAIN);
		M_Main_Draw ();
		break;
	case m_paused_menu:
		//Menu_Background_Draw (MENU_PAUSE);
		M_Paused_Menu_Draw ();
		break;
	case m_restart:
		M_Restart_Draw ();
		break;
	case m_exit:
		M_Exit_Draw ();
		break;
	case m_singleplayer:
		//Menu_Background_Draw (MENU_DEFAULT);
		M_SinglePlayer_Draw ();
		break;
	case m_maps:
		//Menu_Background_Draw (MENU_DEFAULT);
		M_Menu_Maps_Draw ();
		break;
	case m_multiplayer:
		M_MultiPlayer_Draw ();
		break;
#ifdef VITA
	case m_achievement:
		M_Achievement_Draw ();
		break;
#endif
	case m_setup:
		GL_SetCanvas (CANVAS_MENU); //johnfitz
		M_Setup_Draw ();
		break;
	case m_net:
		M_Graphics_Settings_Draw ();
		break;
	case m_options:
		M_Options_Draw ();
		break;
	case m_keys:
		M_Keys_Draw ();
		break;
	case m_video:
		M_Control_Settings_Draw ();
		break;
	case m_credits:
		M_Credits_Draw ();
		break;
	case m_quit:
		if (!fitzmode)
		{ /* QuakeSpasm customization: */
			/* Quit now! S.A. */
			key_dest = key_console;
			Host_Quit_f ();
		}
		M_Quit_Draw ();
		break;
	case m_lanconfig:
		M_LanConfig_Draw ();
		break;
	case m_gameoptions:
		//Menu_Background_Draw (MENU_DEFAULT);
		GL_SetCanvas (CANVAS_MENU); //johnfitz
		M_GameOptions_Draw ();
		break;
	case m_search:
		M_Search_Draw ();
		break;
	case m_slist:
		M_ServerList_Draw ();
		break;
	default:
		break;
	}

	if (m_entersound)
	{
		S_LocalSound ("sounds/menu/enter.wav");
		m_entersound = false;
	}

	S_ExtraUpdate ();
}


void M_Keydown (int key)
{
	switch (m_state)
	{
	case m_none:
		return;

	case m_start:
		M_Start_Key (key);
		return;

	case m_main:
		M_Main_Key (key);
		return;

	case m_paused_menu:
		M_Paused_Menu_Key (key);
		return;

	case m_restart:
		M_Restart_Key (key);
		return;

	case m_exit:
		M_Exit_Key (key);
		return;

	case m_singleplayer:
		M_SinglePlayer_Key (key);
		return;

	case m_maps:
		M_Menu_Maps_Key (key);
		return;

	case m_multiplayer:
		M_MultiPlayer_Key (key);
		return;
#ifdef VITA	
	case m_achievement:
		M_Achievement_Key (key);
		return;
#endif

	case m_setup:
		M_Setup_Key (key);
		return;

	case m_net:
		M_Graphics_Settings_Key (key);
		return;

	case m_options:
		M_Options_Key (key);
		return;

	case m_keys:
		M_Keys_Key (key);
		return;

	case m_video:
		M_Control_Settings_Key (key);
		return;

	case m_credits:
		M_Credits_Key (key);
		return;

	case m_quit:
		M_Quit_Key (key);
		return;

	case m_lanconfig:
		M_LanConfig_Key (key);
		return;

	case m_gameoptions:
		M_GameOptions_Key (key);
		return;

	case m_search:
		M_Search_Key (key);
		break;

	case m_slist:
		M_ServerList_Key (key);
		return;

	default:
		return;
	}
}


void M_Charinput (int key)
{
	switch (m_state)
	{
	case m_setup:
		M_Setup_Char (key);
		return;
	case m_quit:
		M_Quit_Char (key);
		return;
	case m_lanconfig:
		M_LanConfig_Char (key);
		return;
	default:
		return;
	}
}


qboolean M_TextEntry (void)
{
	switch (m_state)
	{
	case m_setup:
		return M_Setup_TextEntry ();
	case m_quit:
		return M_Quit_TextEntry ();
	case m_lanconfig:
		return M_LanConfig_TextEntry ();
	default:
		return false;
	}
}


void M_ConfigureNetSubsystem(void)
{
// enable/disable net systems to match desired config
	Cbuf_AddText ("stopdemo\n");

	if (IPXConfig || TCPIPConfig)
		net_hostport = lanConfig_port;
}

