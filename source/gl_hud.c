/*
Copyright (C) 1996-1997 Id Software, Inc.

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
// gl_hud.c -- status bar code

#include "quakedef.h"

extern cvar_t	waypoint_mode;

qpic_t      *b_up;
qpic_t      *b_down;
qpic_t      *b_left;
qpic_t      *b_right;
qpic_t      *b_lthumb;
qpic_t      *b_rthumb;
qpic_t      *b_lshoulder;
qpic_t      *b_rshoulder;
qpic_t      *b_abutton;
qpic_t      *b_bbutton;
qpic_t      *b_ybutton;
qpic_t      *b_xbutton;
qpic_t      *b_lt;
qpic_t      *b_rt;
qpic_t 		*b_touch;

qpic_t		*sb_round[5];
qpic_t		*sb_round_num[10];
qpic_t		*sb_moneyback;
qpic_t 		*sb_moneyback_condensed;
qpic_t		*instapic;
qpic_t		*x2pic;
qpic_t 		*revivepic;
qpic_t		*jugpic;
qpic_t		*floppic;
qpic_t		*staminpic;
qpic_t		*doublepic;
qpic_t 		*doublepic2;
qpic_t		*speedpic;
qpic_t		*deadpic;
qpic_t 		*mulepic;
qpic_t		*fragpic;
qpic_t		*bettypic;

qpic_t      *fx_blood_lu;

int old_points[4]; // cypress -- fix point display in coop.
int current_points[4]; // cypress -- fix point display in coop.
int point_change_interval;
int point_change_interval_neg;
int alphabling = 0;
float round_center_x;
float round_center_y;

int screenflash_color;
double screenflash_duration;
int screenflash_type;
double screenflash_worktime;
double screenflash_starttime;

extern qboolean paused_hack;
qboolean domaxammo;
qboolean has_chaptertitle;
qboolean 	doubletap_has_damage_buff;

double HUD_Change_time;//hide hud when not chagned
double bettyprompt_time;
double nameprint_time;

char player_name[16];

typedef struct
{
	int points;
	int negative;
	float x;
	float y;
	float move_x;
	float move_y;
	double alive_time;
} point_change_t;

int  x_value, y_value;

point_change_t point_change[10];

void HUD_Init (void) {
	int		i;

	has_chaptertitle = false;

	for (i=0 ; i<5 ; i++)
	{
		sb_round[i] = Draw_CachePic (va("gfx/hud/r%i.tga",i + 1));
	}

	for (i=0 ; i<10 ; i++)
	{
		sb_round_num[i] = Draw_CachePic (va("gfx/hud/r_num%i.tga",i));
	}

	sb_moneyback = Draw_CachePic ("gfx/hud/moneyback.tga");
	sb_moneyback_condensed = Draw_CachePic ("gfx/hud/moneyback_condensed.tga");
	instapic = Draw_CachePic ("gfx/hud/in_kill.tga");
	x2pic = Draw_CachePic ("gfx/hud/2x.tga");

	revivepic = Draw_CachePic ("gfx/hud/revive.tga");
	jugpic = Draw_CachePic ("gfx/hud/jug.tga");
	floppic = Draw_CachePic ("gfx/hud/flopper.tga");
	staminpic = Draw_CachePic ("gfx/hud/stamin.tga");
	doublepic = Draw_CachePic ("gfx/hud/double.tga");
	doublepic2 = Draw_CachePic ("gfx/hud/double2.tga");
	speedpic = Draw_CachePic ("gfx/hud/speed.tga");
	deadpic = Draw_CachePic ("gfx/hud/dead.tga");
	mulepic = Draw_CachePic ("gfx/hud/mule.tga");
	fragpic = Draw_CachePic ("gfx/hud/frag.tga");
	bettypic = Draw_CachePic ("gfx/hud/betty.tga");

#ifdef VITA
	if (sceKernelGetModel() == 0x20000) { // PSTV
		b_lt = Draw_CachePic ("gfx/butticons/backl1_pstv.tga");
		b_rt = Draw_CachePic ("gfx/butticons/backr1_pstv.tga");
		b_touch = Draw_CachePic ("gfx/butticons/functouch_pstv.tga");
		b_lthumb = Draw_CachePic ("gfx/butticons/backl1_pstv.tga"); // Not existent
		b_rthumb = Draw_CachePic ("gfx/butticons/backr1_pstv.tga"); // Not existent
		b_lshoulder = Draw_CachePic ("gfx/butticons/backl1_pstv.tga"); // Not existent
		b_rshoulder = Draw_CachePic ("gfx/butticons/backr1_pstv.tga"); // Not existent
	} else {
		b_lt = Draw_CachePic ("gfx/butticons/backl1.tga");
		b_rt = Draw_CachePic ("gfx/butticons/backr1.tga");
		b_touch = Draw_CachePic ("gfx/butticons/functouch.tga");
		b_lthumb = Draw_CachePic ("gfx/butticons/backl1.tga"); // Not existent
		b_rthumb = Draw_CachePic ("gfx/butticons/backr1.tga"); // Not existent
		b_lshoulder = Draw_CachePic ("gfx/butticons/backl1.tga"); // Not existent
		b_rshoulder = Draw_CachePic ("gfx/butticons/backr1.tga"); // Not existent
	}
	
	b_left = Draw_CachePic ("gfx/butticons/dpadleft.tga");
	b_right = Draw_CachePic ("gfx/butticons/dpadright.tga");
	b_up = Draw_CachePic ("gfx/butticons/dpadup.tga");
	b_down = Draw_CachePic ("gfx/butticons/dpaddown.tga");
	b_abutton = Draw_CachePic ("gfx/butticons/fbtncross.tga");
	b_bbutton = Draw_CachePic ("gfx/butticons/fbtncircle.tga");
	b_ybutton = Draw_CachePic ("gfx/butticons/fbtntriangle.tga");
	b_xbutton = Draw_CachePic ("gfx/butticons/fbtnsquare.tga");
#else
	b_lt = Draw_CachePic ("gfx/butticons/lt.tga");
	b_rt = Draw_CachePic ("gfx/butticons/rt.tga");
	b_lthumb = Draw_CachePic ("gfx/butticons/lthumb.tga");
	b_rthumb = Draw_CachePic ("gfx/butticons/rthumb.tga");
	b_lshoulder = Draw_CachePic ("gfx/butticons/lshoulder.tga");
	b_rshoulder = Draw_CachePic ("gfx/butticons/rshoulder.tga");
	
	b_left = Draw_CachePic ("gfx/butticons/left.tga");
	b_right = Draw_CachePic ("gfx/butticons/right.tga");
	b_up = Draw_CachePic ("gfx/butticons/up.tga");
	b_down = Draw_CachePic ("gfx/butticons/down.tga");
	b_abutton = Draw_CachePic ("gfx/butticons/abutton.tga");
	b_bbutton = Draw_CachePic ("gfx/butticons/bbutton.tga");
	b_ybutton = Draw_CachePic ("gfx/butticons/ybutton.tga");
	b_xbutton = Draw_CachePic ("gfx/butticons/xbutton.tga");
#endif
	fx_blood_lu = Draw_CachePic ("gfx/hud/blood.tga");
#ifdef VITA
	Achievement_Init();
#endif
}

/*
===============
HUD_NewMap
===============
*/
void HUD_NewMap (void)
{
	int i;
	alphabling = 0;

	for (i=0 ; i<10 ; i++)
	{
		point_change[i].points = 0;
		point_change[i].negative = 0;
		point_change[i].x = 0.0;
		point_change[i].y = 0.0;
		point_change[i].move_x = 0.0;
		point_change[i].move_y = 0.0;
		point_change[i].alive_time = 0.0;
	}

	for(i = 0; i < 4; i++) {
		old_points[i] = current_points[i] = 500;
	}

	point_change_interval = 0;
	point_change_interval_neg = 0;

#ifdef VITA
	round_center_x = vid.width/2 - sb_round[0]->width + 3;
	round_center_y = vid.height/2 - sb_round[0]->height/2 - 27;
#else
	round_center_x = (vid.width/2 - sb_round[0]->width) /2;
	round_center_y = vid.height*3/4 - sb_round[0]->height/2;
#endif
}


/*
=============
HUD_itoa
=============
*/
int HUD_itoa (int num, char *buf)
{
	char	*str;
	int		pow10;
	int		dig;

	str = buf;

	if (num < 0)
	{
		*str++ = '-';
		num = -num;
	}

	for (pow10 = 10 ; num >= pow10 ; pow10 *= 10)
	;

	do
	{
		pow10 /= 10;
		dig = num/pow10;
		*str++ = '0'+dig;
		num -= dig*pow10;
	} while (pow10 != 1);

	*str = 0;

	return str-buf;
}


//=============================================================================

int		pointsort[MAX_SCOREBOARD];

extern char	scoreboardtext[MAX_SCOREBOARD][20];
extern int		scoreboardtop[MAX_SCOREBOARD];
extern int		scoreboardbottom[MAX_SCOREBOARD];
extern int		scoreboardcount[MAX_SCOREBOARD];
extern int		scoreboardlines;

/*
===============
HUD_Sorpoints
===============
*/
void HUD_Sortpoints (void)
{
	int		i, j, k;

// sort by points
	scoreboardlines = 0;
	for (i=0 ; i<cl.maxclients ; i++)
	{
		if (cl.scores[i].name[0])
		{
			pointsort[scoreboardlines] = i;
			scoreboardlines++;
		}
	}

	for (i=0 ; i<scoreboardlines ; i++)
		for (j=0 ; j<scoreboardlines-1-i ; j++)
			if (cl.scores[pointsort[j]].points < cl.scores[pointsort[j+1]].points)
			{
				k = pointsort[j];
				pointsort[j] = pointsort[j+1];
				pointsort[j+1] = k;
			}
}

/*
===============
HUD_EndScreen
===============
*/
void HUD_EndScreen (void)
{
	scoreboard_t	*s;
	char			str[80];
	int				i, k, l;
	int				y, x, d;

	//sB hack in to stop showing HUD during boot
	if (cl.stats[STAT_ROUNDS] <= 0)
		return;

	HUD_Sortpoints ();
	l = scoreboardlines;

	// "You Survived" text
	if (cl.stats[STAT_ROUNDS] == 1)
		sprintf(str, "You Survived %d Round", cl.stats[STAT_ROUNDS]);
	else
		sprintf(str, "You Survived %d Rounds", cl.stats[STAT_ROUNDS]);

#ifdef VITA

	Draw_ColoredStringScale(vid.width/2 - strlen("GAME OVER")*16, 110, "GAME OVER", 1, 1, 1, 1, 4.0f);
	Draw_ColoredStringScale(vid.width/2 - strlen(str)*8, 150, str, 1, 1, 1, 1, 2.0f);

	Draw_ColoredStringScale(vid.width/2 + 40, 185, "Points", 1, 1, 1, 1, 2.0f);
	Draw_ColoredStringScale(vid.width/2 + 240, 185, "Kills", 1, 1, 1, 1, 2.0f);

	// Individual Player Scores
	y = 0;
	for (i = 0; i < l; i++) {
		k = pointsort[i];
		s = &cl.scores[k];

		if (!s->name[0])
			continue;

		// Draw a bunch of score background graphics to form a nice line.
		for (int j = 0; j < 7; j++) {
			Draw_StretchPic(90 + (108 * j), 205 + y, sb_moneyback, 128, 32);
		}

		Draw_ColoredStringScale(100, 212 + y, s->name, 1, 1, 1, 1, 2.0f);

		int xplus = HUD_itoa (s->points, str)*16;
		Draw_ColoredStringScale (((1145 - xplus)/2)-5, 212 + y, va("%i", s->points), 1, 1, 1, 1, 2.0f);
		xplus = HUD_itoa (s->kills, str)*16;
		Draw_ColoredStringScale (((1535 - xplus)/2)-5, 212 + y, va("%i", s->kills), 1, 1, 1, 1, 2.0f);

		y += 45;
	}

#else

	Draw_ColoredStringScale(vid.width/4 - strlen("GAME OVER")*8, vid.height*3/4 - 98, "GAME OVER", 1, 1, 1, 1, 2.0f);
	Draw_ColoredStringScale(vid.width/4 - strlen(str)*6, vid.height*3/4 - 70, str, 1, 1, 1, 1, 1.5f);

	Draw_ColoredStringScale(vid.width/4 + 35, vid.height*3/4 - 40, "Points", 1, 1, 1, 1, 1.25f);
	Draw_ColoredStringScale(vid.width/4 + 130, vid.height*3/4 - 40, "Kills", 1, 1, 1, 1, 1.25f);

	// Individual Player Scores
	y = 0;
	for (i = 0; i < l; i++) {
		k = pointsort[i];
		s = &cl.scores[k];

		if (!s->name[0])
			continue;

		// Draw a bunch of score background graphics to form a nice line.
		for (int j = 0; j < 6; j++) {
			Draw_StretchPic(85 + (74 * j), vid.height*3/4 - 20 + y, sb_moneyback, 86, 21);
		}

		Draw_ColoredStringScale(100, vid.height*3/4 - 16 + y, s->name, 1, 1, 1, 1, 1.25f);

		int xplus = HUD_itoa (s->points, str)*10;
		Draw_ColoredStringScale (((775 - xplus)/2)-5, vid.height*3/4 - 16 + y, va("%i", s->points), 1, 1, 1, 1, 1.25f);
		xplus = HUD_itoa (s->kills, str)*10;
		Draw_ColoredStringScale (((975 - xplus)/2)-5, vid.height*3/4 - 16 + y, va("%i", s->kills), 1, 1, 1, 1, 1.25f);

		y += 30;
	}

#endif // VITA

}


//=============================================================================


/*
==================
HUD_Points

==================
*/


void HUD_Parse_Point_Change (int points, int negative, int x_start, int y_start)
{
	int i, f;
	char str[10];
	i=9;
	while (i>0)
	{
		point_change[i] = point_change[i - 1];
		i--;
	}

	point_change[i].points = points;
	point_change[i].negative = negative;

	f = HUD_itoa (points, str);
#ifdef VITA
	point_change[i].x = x_start + 105.0 + 4.0*f;
#else
	point_change[i].x = x_start - 10.0 - 8.0*f;
#endif
	point_change[i].y = y_start;
	point_change[i].move_x = 1.0;
	point_change[i].move_y = ((rand ()&0x7fff) / ((float)0x7fff)) - 0.5;

	point_change[i].alive_time = Sys_DoubleTime() + 0.4;
}

void HUD_Points (void)
{
	int				i, k, l;
	int				x, y, f, xplus;
	float 			r, g, b;
	scoreboard_t	*s;
	char str[12];

// scores
	HUD_Sortpoints ();

// draw the text
	l = scoreboardlines;


    x = vid.width/2 - sb_moneyback->width;
    y = vid.height - 16 - fragpic->height - 4 - 16 - sb_moneyback->height;
	for (i=0 ; i<l ; i++)
	{
		k = pointsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

		switch(i) {
			case 0: r = g = b = 1; break;
			case 1: r = 0; g = 0.46; b = 0.7; break;
			case 2: r = 0.92; g = 0.74; b = 0; break;
			case 3: r = 0; g = 0.90; b = 0.13; break;
			default: r = g = b = 1; break;
		}

	// draw background

	// draw number
		f = s->points;
		if (f > current_points[i])
		{
			point_change_interval_neg = 0;
			if (!point_change_interval)
			{
				point_change_interval = (int)(f - old_points[i])/55;;
			}
			current_points[i] = old_points[i] + point_change_interval;
			if (f < current_points[i])
			{
				current_points[i] = f;
				point_change_interval = 0;
			}
		}
		else if (f < current_points[i])
		{
			point_change_interval = 0;
			if (!point_change_interval_neg)
			{
				point_change_interval_neg = (int)(old_points[i] - f)/55;
			}
			current_points[i] = old_points[i] - point_change_interval_neg;
			if (f > current_points[i])
			{
				current_points[i] = f;
				point_change_interval_neg = 0;
			}
		}

		// Draw a "condensed" version of the score background
		// if it's for a player other than us, for easier
		// differential, and screen space saving.
		qpic_t* moneyback;
		int x_position;
		int point_x_offset;

		if (i == cl.viewentity - 1) {
			moneyback = sb_moneyback;

#ifdef VITA
			x_position = 12;
			point_x_offset = 5;
#else
			x_position = 5;
			point_x_offset = 4;
#endif // VITA
			
		} else {
			moneyback = sb_moneyback_condensed;

#ifdef VITA
			x_position = 3;
			point_x_offset = 14;
#else
			x_position = -4;
			point_x_offset = 15;
#endif // VITA			
		}
			

#ifdef VITA
		Draw_StretchPic(x_position, 407 - (35 * i), moneyback, 128, 32);
#else
		Draw_StretchPic(x_position, 629 - (24 * i), moneyback, 86, 21);
#endif // VITA
		
#ifdef VITA
		xplus = HUD_itoa (f, str)*16;
		Draw_ColoredStringScale (((160 - xplus)/2)-point_x_offset, 413 - (35 * i), va("%i", current_points[i]), r, g, b, 1, 2.0f); //2x Scale/White
#else
		xplus = HUD_itoa (f, str)*12;
		Draw_ColoredStringScale (((111 - xplus)/2)-point_x_offset, 633 - (24 * i), va("%i", current_points[i]), r, g, b, 1, 1.5f);
#endif // VITA

		if (old_points[i] != f)
		{
			if (f > old_points[i])
			{
			#ifdef VITA
				HUD_Parse_Point_Change(f - old_points[i], 0, 80 - (xplus), 415 - (35 * i));
			#else 
				HUD_Parse_Point_Change(f - old_points[i], 0, 140 - (xplus), y - (28 * i));	
			#endif // VITA
			}
			else
			{
			#ifdef VITA
				HUD_Parse_Point_Change(old_points[i] - f, 1, 80 - (xplus), 415 - (35 * i));
			#else
				HUD_Parse_Point_Change(old_points[i] - f, 1, 140 - (xplus), y - (28 * i));
			#endif // VITA
			}
			old_points[i] = f;
		}
		
		y += 10;
	}
}


/*
==================
HUD_Point_Change

==================
*/
void HUD_Point_Change (void)
{
	int	i;

	for (i=0 ; i<10 ; i++)
	{
		if (point_change[i].points)
		{
			if (point_change[i].negative)
			{
			#ifdef VITA
				Draw_ColoredStringScale (point_change[i].x, point_change[i].y, va ("-%i", point_change[i].points), 1, 0, 0, 1, 2);
			#else
				Draw_ColoredStringScale (point_change[i].x, point_change[i].y, va ("-%i", point_change[i].points), 1, 0, 0, 1, 1.25f);
			#endif
			}
			else
			{
			#ifdef VITA
				Draw_ColoredStringScale (point_change[i].x, point_change[i].y, va ("+%i", point_change[i].points), 1, 1, 0, 1, 2);
			#else
				Draw_ColoredStringScale (point_change[i].x, point_change[i].y, va ("+%i", point_change[i].points), 1, 1, 0, 1, 1.25f);
			#endif
			}
			point_change[i].y = point_change[i].y + point_change[i].move_y;
			point_change[i].x = point_change[i].x + point_change[i].move_x;
			if (point_change[i].alive_time && point_change[i].alive_time < Sys_DoubleTime())
			{
				point_change[i].points = 0;
				point_change[i].negative = 0;
				point_change[i].x = 0.0;
				point_change[i].y = 0.0;
				point_change[i].move_x = 0.0;
				point_change[i].move_y = 0.0;
				point_change[i].alive_time = 0.0;
			}
		}
	}
}

/*
==================
HUD_Blood

==================
*/
void HUD_Blood (void)
{
    float alpha;
	//blubswillrule:
	//this function scales linearly from health = 0 to health = 100
	//alpha = (100.0 - (float)cl.stats[STAT_HEALTH])/100*255;
	//but we want the picture to be fully visible at health = 20, so use this function instead
	alpha = (100.0 - ((1.25 * (float) cl.stats[STAT_HEALTH]) - 25))/100;

    if (alpha <= 0.0)
        return;
    
    float modifier = (sin(cl.time * 10) * 20) - 20;//always negative
    if(modifier < -35.0)
	modifier = -35.0;
    
    alpha += (modifier/255);

    if (alpha > 1) {
    	alpha = 1;
    }
    
    if(alpha < 0.0)
	    return;
    
	// Set the canvas to default so we can stretch by normal bounds
	GL_SetCanvas(CANVAS_DEFAULT);
    Draw_AlphaStretchPic (0, 0, vid.width, vid.height, alpha, fx_blood_lu);
	GL_SetCanvas(CANVAS_USEPRINT);
}

/*
===============
HUD_MaxAmmo
===============
*/
int maxammoy;
float maxammoopac;

void HUD_MaxAmmo(void)
{
	maxammoy -= cl.time * 0.003;
	maxammoopac -= cl.time * 0.05;

#ifdef VITA

	Draw_ColoredStringScale(vid.width/2 - strlen("MAX AMMO!")*8, maxammoy, "MAX AMMO!", 255, 255, 255, maxammoopac/255, 2.0f);

#else

	Draw_ColoredStringScale(vid.width/4 - strlen("MAX AMMO!")*8, maxammoy, "MAX AMMO!", 255, 255, 255, maxammoopac/255, 2.0f);

#endif // VITA

	if (maxammoopac <= 0) {
		domaxammo = false;
	}
}

/*
===============
HUD_WorldText
===============
*/

// modified from scatter's worldspawn parser
void HUD_WorldText(float alpha)
{
	// for parser
	char key[128], value[4096];
	char *data;

	// first, parse worldspawn
	data = COM_Parse(cl.worldmodel->entities);

	if (!data)
		return; // err
	if (com_token[0] != '{')
		return; // err

	while(1) {
		data = COM_Parse(data);

		if (!data)
			return; // err
		if (com_token[0] == '}')
			break; // end of worldspawn
		
		if (com_token[0] == '_')
			strcpy(key, com_token + 1);
		else
			strcpy(key, com_token);
		
		while (key[strlen(key)-1] == ' ') // remove trailing spaces
			key[strlen(key)-1] = 0;
		
		data = COM_Parse(data);
		if (!data)
			return; // err

		strcpy(value, com_token);

#ifdef VITA

		if (!strcmp("chaptertitle", key)) // search for chaptertitle key
		{
			has_chaptertitle = true;
			Draw_ColoredStringScale(12, 407 - (19 * 5), value, 1, 1, 1, alpha/255, 2.0f);
		}
		if (!strcmp("location", key)) // search for location key
		{
			Draw_ColoredStringScale(12, 407 - (19 * 4), value, 1, 1, 1, alpha/255, 2.0f);
		}
		if (!strcmp("date", key)) // search for date key
		{
			Draw_ColoredStringScale(12, 407 - (19 * 3), value, 1, 1, 1, alpha/255, 2.0f);
		}
		if (!strcmp("person", key)) // search for person key
		{
			Draw_ColoredStringScale(12, 407 - (19 * 2), value, 1, 1, 1, alpha/255, 2.0f);
		}

#else

		if (!strcmp("chaptertitle", key)) // search for chaptertitle key
		{
			has_chaptertitle = true;
			Draw_ColoredStringScale(12, 629 - (15 * 5), value, 1, 1, 1, alpha/255, 1.5f);
		}
		if (!strcmp("location", key)) // search for location key
		{
			Draw_ColoredStringScale(12, 629 - (15 * 4), value, 1, 1, 1, alpha/255, 1.5f);
		}
		if (!strcmp("date", key)) // search for date key
		{
			Draw_ColoredStringScale(12, 629 - (15 * 3), value, 1, 1, 1, alpha/255, 1.5f);
		}
		if (!strcmp("person", key)) // search for person key
		{
			Draw_ColoredStringScale(12, 629 - (15 * 2), value, 1, 1, 1, alpha/255, 1.5f);
		}

#endif // VITA

	}
}

/*
===============
HUD_Rounds
===============
*/

float 	color_shift[3];
float 	color_shift_end[3];
float 	color_shift_steps[3];
int		color_shift_init;
float 	blinking;
float 	endroundchange;
int 	textstate;
float 	value, value2;

void HUD_Rounds (void)
{
	int i, x_offset, icon_num, savex;
	int num[3];
	x_offset = 0;
	savex = 0;

	// Round and Title text - cypress
	// extra creds to scatterbox for some x/y vals
	// ------------------
	// First, fade from white to red, ~3s duration
	if (!textstate) {
		if (!value)
			value = 255;

#ifdef VITA

		Draw_ColoredStringScale(vid.width/2 - strlen("Round")*16, 160, "Round", 1, value/255, value/255, 1, 4.0f);

#else

		Draw_ColoredStringScale(vid.width/4 - strlen("Round")*8, vid.height*3/4 - sb_round[0]->height - 10, "Round", 1, value/255, value/255, 1, 2.0f);

#endif // VITA
		
		value -= cl.time * 0.4;

		// prep values for next stage
		if (value <= 0) {
			value = 255;
			value2 = 0;
			textstate = 1;
		}
	} 
	// Now, fade out, and start fading worldtext in
	// ~3s for fade out, 
	else if (textstate == 1) {

#ifdef VITA

		Draw_ColoredStringScale(vid.width/2 - strlen("Round")*16, 160, "Round", 1, 0, 0, value/255, 4.0f);

#else

		Draw_ColoredStringScale(vid.width/4 - strlen("Round")*8, vid.height*3/4 - sb_round[0]->height - 10, "Round", 1, 0, 0, value/255, 2.0f);

#endif // VITA		

		HUD_WorldText(value2);

		if (has_chaptertitle == false) {
#ifdef VITA

			Draw_ColoredStringScale(12, 407 - 19, "'Nazi Zombies'", 1, 1, 1, value2/255, 2.0f);

#else

			Draw_ColoredStringScale(6, 629 - 15, "'Nazi Zombies'", 1, 1, 1, value2/255, 1.5f);

#endif // VITA
		}
		
		value -= cl.time * 0.4;
		value2 += cl.time * 0.4;

		// prep values for next stage
		if (value <= 0) {
			value2 = 0;
			textstate = 2;
		}
	}
	// Hold world text for a few seconds
	else if (textstate == 2) {
		HUD_WorldText(255);
		
		if (has_chaptertitle == false) {
#ifdef VITA

			Draw_ColoredStringScale(12, 407 - 19, "'Nazi Zombies'", 1, 1, 1, 1, 2.0f);

#else

			Draw_ColoredStringScale(6, 629 - 15, "'Nazi Zombies'", 1, 1, 1, 1, 1.5f);

#endif // VITA
		}

		value2 += cl.time * 0.4;

		if (value2 >= 255) {
			value2 = 255;
			textstate = 3;
		}
	}
	// Fade worldtext out, finally.
	else if (textstate == 3) {
		HUD_WorldText(value2);
		
		if (has_chaptertitle == false) {
#ifdef VITA

			Draw_ColoredStringScale(12, 407 - 19, "'Nazi Zombies'", 1, 1, 1, value2/255, 2.0f);

#else

			Draw_ColoredStringScale(6, 629 - 15, "'Nazi Zombies'", 1, 1, 1, value2/255, 1.5f);

#endif // VITA
		}

		value2 -= cl.time * 0.4;

		// prep values for next stage
		if (value2 <= 0) {
			textstate = -1;
		}
	}
	// ------------------
	// End Round and Title text - cypress

	int x_offset2 = 2; //Extra offset for all round images to keep things uniform (may not be neccesary?) -- SPECIFIC TO WHOLE ROUNDS
	int y_offset = 50; //y_offset -- SPECIFIC TO WHOLE ROUNDS
	int x_mark_offset = 10; //Needed x offset for stretched marks 
	int y_mark_offset = 50; //y offset specific to marks
	int stretch_x = 22; //Stretch round mark image x
	int stretch_y = 96; //Stretch round mark image y

	if (cl.stats[STAT_ROUNDCHANGE] == 1)//this is the rounds icon at the middle of the screen
	{
	#ifdef VITA
		Draw_ColorStretchPic ((vid.width/2 - sb_round[0]->width) - x_offset2 + 5, vid.height/2 - sb_round[0]->height/2 - 27, stretch_x, stretch_y, sb_round[0], 0.4196, 0.004, 0, alphabling/255);
	#else
		Draw_ColorPic ((vid.width/2 - sb_round[0]->width) /2, vid.height*3/4 - sb_round[0]->height/2, sb_round[0], 0.4196, 0.004, 0, alphabling/255);
	#endif

		alphabling = alphabling + 15;

		if (alphabling < 0)
			alphabling = 0;
		else if (alphabling > 255)
			alphabling = 255;
	}
	else if (cl.stats[STAT_ROUNDCHANGE] == 2)//this is the rounds icon moving from middle
	{
	#ifdef VITA
		Draw_ColorStretchPic (round_center_x, round_center_y, stretch_x, stretch_y, sb_round[0], 0.4196, 0.004, 0, 1);
		
		round_center_x -= 2.03;
		round_center_y = round_center_y + ((544*1.015)/544);
		
		if (round_center_x <= 5*(vid.width/2)/272 + x_mark_offset)
			round_center_x = 5*(vid.width/2)/272 + x_mark_offset;
		if (round_center_y >= vid.height - sb_round[4]->height - y_mark_offset)
			round_center_y = vid.height - sb_round[4]->height - y_mark_offset;
	#else
		Draw_ColorPic (round_center_x, round_center_y, sb_round[0], 0.4196, 0.004, 0, 1);
	
		round_center_x = round_center_x - ((229/108)*2 - 0.2)*(vid.width/2)/480;
		round_center_y = round_center_y + ((vid.height*1.015)/272);
		if (round_center_x <= 5)
			round_center_x = 5;
		if (round_center_y >= 250*vid.height/272)
			round_center_y = 250*vid.height/272;
	#endif
	}
	else if (cl.stats[STAT_ROUNDCHANGE] == 3)//shift to white
	{
		if (!color_shift_init)
		{
			color_shift[0] = 107;
			color_shift[1] = 1;
			color_shift[2] = 0;
			for (i = 0; i < 3; i++)
			{
				color_shift_end[i] = 255;
				color_shift_steps[i] = (color_shift_end[i] - color_shift[i])/60;
			}
			color_shift_init = 1;
		}
		for (i = 0; i < 3; i++)
		{
			if (color_shift[i] < color_shift_end[i])
				color_shift[i] = color_shift[i] + color_shift_steps[i];

			if (color_shift[i] >= color_shift_end[i])
				color_shift[i] = color_shift_end[i];
		}
		if (cl.stats[STAT_ROUNDS] > 0 && cl.stats[STAT_ROUNDS] < 11)
		{

			for (i = 0; i < cl.stats[STAT_ROUNDS]; i++)
			{
				if (i == 4)
				{
				#ifdef VITA
					Draw_ColorStretchPic (5*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				#else
					Draw_ColorPic (5*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				#endif
					savex = x_offset + 10;
					x_offset = x_offset + 10;
					continue;
				}
				if (i == 9)
				{
				#ifdef VITA
					Draw_ColorStretchPic ((5 + savex/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				#else
					Draw_ColorPic ((5 + savex/2)*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				#endif
					continue;
				}
				if (i > 4)
					icon_num = i - 5;
				else
					icon_num = i;
			#ifdef VITA
				Draw_ColorStretchPic ((5 + x_offset/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[icon_num]->height - y_mark_offset, stretch_x, stretch_y, sb_round[icon_num], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round[icon_num]->width + 23;
			#else
				Draw_ColorPic ((5 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round[icon_num]->height - 4, sb_round[icon_num], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round[icon_num]->width + 3;
			#endif

				
			}
		}
		else
		{
			if (cl.stats[STAT_ROUNDS] >= 100)
			{
				num[2] = (int)(cl.stats[STAT_ROUNDS]/100);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[2]]->height - y_offset, 64, 96, sb_round_num[num[2]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round_num[num[2]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[2]]->height - 4, sb_round_num[num[2]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round_num[num[2]]->width - 8;
			#endif	
			}
			else
				num[2] = 0;
			if (cl.stats[STAT_ROUNDS] >= 10)
			{
				num[1] = (int)((cl.stats[STAT_ROUNDS] - num[2]*100)/10);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[1]]->height - y_offset, 64, 96, sb_round_num[num[1]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round_num[num[1]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[1]]->height - 4, sb_round_num[num[1]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round_num[num[1]]->width - 8;
			#endif
			}
			else
				num[1] = 0;

			num[0] = cl.stats[STAT_ROUNDS] - num[2]*100 - num[1]*10;
		#ifdef VITA
			Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[0]]->height - y_offset, 64, 96, sb_round_num[num[0]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
			x_offset = x_offset + sb_round_num[num[0]]->width + 25;
		#else
			Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[0]]->height - 4, sb_round_num[num[0]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
			x_offset = x_offset + sb_round_num[num[0]]->width - 8;
		#endif
		}
	}
	else if (cl.stats[STAT_ROUNDCHANGE] == 4)//blink white
	{
		if (endroundchange > cl.time) {
			blinking = (float)(((int)(realtime*475)&510) - 255);
			blinking = abs(blinking);
		} else {
			if (blinking)
				blinking = blinking - 1;
			else
				blinking = 0;
		}
		
		if (cl.stats[STAT_ROUNDS] > 0 && cl.stats[STAT_ROUNDS] < 11)
		{
			for (i = 0; i < cl.stats[STAT_ROUNDS]; i++)
			{
				if (i == 4)
				{
				#ifdef VITA
					Draw_ColorStretchPic (5*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], 1, 1, 1, blinking/255);
				#else
					Draw_ColorPic (5*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], 1, 1, 1, blinking/255);
				#endif
					savex = x_offset + 10;
					x_offset = x_offset + 10;
					continue;
				}
				if (i == 9)
				{
				#ifdef VITA
					Draw_ColorStretchPic ((5 + savex/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], 1, 1, 1, blinking/255);
				#else
					Draw_ColorPic ((5 + savex/2)*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], 1, 1, 1, blinking/255);
				#endif
					continue;
				}
				if (i > 4)
					icon_num = i - 5;
				else
					icon_num = i;
			#ifdef VITA
				Draw_ColorStretchPic ((5 + x_offset/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[icon_num]->height - y_mark_offset, stretch_x, stretch_y, sb_round[icon_num], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round[icon_num]->width + 23;
			#else
				Draw_ColorPic ((5 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round[icon_num]->height - 4, sb_round[icon_num], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round[icon_num]->width + 3;
			#endif
			}
		}
		else
		{
			if (cl.stats[STAT_ROUNDS] >= 100)
			{
				num[2] = (int)(cl.stats[STAT_ROUNDS]/100);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[2]]->height - y_offset, 64, 96, sb_round_num[num[2]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[2]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[2]]->height - 4, sb_round_num[num[2]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[2]]->width - 8;
			#endif
			}
			else
				num[2] = 0;
			if (cl.stats[STAT_ROUNDS] >= 10)
			{
				num[1] = (int)((cl.stats[STAT_ROUNDS] - num[2]*100)/10);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[1]]->height - y_offset, 64, 96, sb_round_num[num[1]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[1]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[1]]->height - 4, sb_round_num[num[1]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[1]]->width - 8;
			#endif
			}
			else
				num[1] = 0;

			num[0] = cl.stats[STAT_ROUNDS] - num[2]*100 - num[1]*10;
		#ifdef VITA
			Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[0]]->height - y_offset, 64, 96, sb_round_num[num[0]], 1, 1, 1, blinking/255);
			x_offset = x_offset + sb_round_num[num[0]]->width + 25;
		#else
			Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[0]]->height - 4, sb_round_num[num[0]], 1, 1, 1, blinking/255);
			x_offset = x_offset + sb_round_num[num[0]]->width - 8;
		#endif
		}

		if (endroundchange == 0) {
			endroundchange = cl.time + 7.5;
			blinking = 0;
		}
	}
	else if (cl.stats[STAT_ROUNDCHANGE] == 5)//blink white
	{
		if (blinking > 0)
			blinking = blinking - 10;
		if (blinking < 0)
			blinking = 0;
		if (cl.stats[STAT_ROUNDS] > 0 && cl.stats[STAT_ROUNDS] < 11)
		{
			for (i = 0; i < cl.stats[STAT_ROUNDS]; i++)
			{
				if (i == 4)
				{
				#ifdef VITA
					Draw_ColorStretchPic (5*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], 1, 1, 1, blinking/255);
				#else
					Draw_ColorPic (5*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], 1, 1, 1, blinking/255);
				#endif
					savex = x_offset + 10;
					x_offset = x_offset + 10;
					continue;
				}
				if (i == 9)
				{
				#ifdef VITA
					Draw_ColorStretchPic ((5 + savex/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], 1, 1, 1, blinking/255);
				#else 
					Draw_ColorPic ((5 + savex/2)*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], 1, 1, 1, blinking/255);
				#endif
					continue;
				}
				if (i > 4)
					icon_num = i - 5;
				else
					icon_num = i;
			#ifdef VITA
				Draw_ColorStretchPic ((5 + x_offset/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[icon_num]->height - y_mark_offset, stretch_x, stretch_y, sb_round[icon_num], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round[icon_num]->width + 23;
			#else
				Draw_ColorPic ((5 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round[icon_num]->height - 4, sb_round[icon_num], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round[icon_num]->width + 3;
			#endif
			}
		}
		else
		{
			if (cl.stats[STAT_ROUNDS] >= 100)
			{
				num[2] = (int)(cl.stats[STAT_ROUNDS]/100);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[2]]->height - y_offset, 64, 96, sb_round_num[num[2]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[2]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[2]]->height - 4, sb_round_num[num[2]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[2]]->width - 8;
			#endif
			}
			else
				num[2] = 0;
			if (cl.stats[STAT_ROUNDS] >= 10)
			{
				num[1] = (int)((cl.stats[STAT_ROUNDS] - num[2]*100)/10);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[1]]->height - y_offset, 64, 96, sb_round_num[num[1]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[1]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[1]]->height - 4, sb_round_num[num[1]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[1]]->width - 8;
			#endif
			}
			else
				num[1] = 0;

			num[0] = cl.stats[STAT_ROUNDS] - num[2]*100 - num[1]*10;
		#ifdef VITA
			Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[0]]->height - y_offset, 64, 96, sb_round_num[num[0]], 1, 1, 1, blinking/255);
			x_offset = x_offset + sb_round_num[num[0]]->width + 25;
		#else
			Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[0]]->height - 4, sb_round_num[num[0]], 1, 1, 1, blinking/255);
			x_offset = x_offset + sb_round_num[num[0]]->width - 8;
		#endif
		}
	}
	else if (cl.stats[STAT_ROUNDCHANGE] == 6)//blink white while fading back
	{
		if (endroundchange) {
			endroundchange = 0;
			blinking = 0;
		}

		color_shift_init = 0;

		blinking += host_frametime*475;
		if (blinking > 255) blinking = 255;

		if (cl.stats[STAT_ROUNDS] > 0 && cl.stats[STAT_ROUNDS] < 11)
		{
			for (i = 0; i < cl.stats[STAT_ROUNDS]; i++)
			{
				if (i == 4)
				{
				#ifdef VITA
					Draw_ColorStretchPic (5*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], 1, 1, 1, blinking/255);
				#else
					Draw_ColorPic (5*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], 1, 1, 1, blinking/255);
				#endif
					savex = x_offset + 10;
					x_offset = x_offset + 10;
					continue;
				}
				if (i == 9)
				{
				#ifdef VITA
					Draw_ColorStretchPic ((5 + savex/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], 1, 1, 1, blinking/255);
				#else
					Draw_ColorPic ((5 + savex/2)*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], 1, 1, 1, blinking/255);
				#endif
					continue;
				}
				if (i > 4)
					icon_num = i - 5;
				else
					icon_num = i;
			#ifdef VITA
				Draw_ColorStretchPic ((5 + x_offset/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[icon_num]->height - y_mark_offset, stretch_x, stretch_y, sb_round[icon_num], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round[icon_num]->width + 23;
			#else
				Draw_ColorPic ((5 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round[icon_num]->height - 4, sb_round[icon_num], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round[icon_num]->width + 3;
			#endif
			}
		}
		else
		{
			if (cl.stats[STAT_ROUNDS] >= 100)
			{
				num[2] = (int)(cl.stats[STAT_ROUNDS]/100);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[2]]->height - y_offset, 64, 96, sb_round_num[num[2]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[2]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[2]]->height - 4, sb_round_num[num[2]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[2]]->width - 8;
			#endif
			}
			else
				num[2] = 0;
			if (cl.stats[STAT_ROUNDS] >= 10)
			{
				num[1] = (int)((cl.stats[STAT_ROUNDS] - num[2]*100)/10);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[1]]->height - y_offset, 64, 96, sb_round_num[num[1]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[1]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[1]]->height - 4, sb_round_num[num[1]], 1, 1, 1, blinking/255);
				x_offset = x_offset + sb_round_num[num[1]]->width - 8;
			#endif
			}
			else
				num[1] = 0;

			num[0] = cl.stats[STAT_ROUNDS] - num[2]*100 - num[1]*10;
		#ifdef VITA
			Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[0]]->height - y_offset, 64, 96, sb_round_num[num[0]], 1, 1, 1, blinking/255);
			x_offset = x_offset + sb_round_num[num[0]]->width + 25;
		#else
			Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[0]]->height - 4, sb_round_num[num[0]], 1, 1, 1, blinking/255);
			x_offset = x_offset + sb_round_num[num[0]]->width - 8;
		#endif
		}
	}
	else if (cl.stats[STAT_ROUNDCHANGE] == 7)//blink white while fading back
	{
		if (!color_shift_init)
		{
			color_shift_end[0] = 107;
			color_shift_end[1] = 1;
			color_shift_end[2] = 0;
			for (i = 0; i < 3; i++)
			{
				color_shift[i] = 255;
				color_shift_steps[i] = (color_shift[i] - color_shift_end[i])/60;
			}
			color_shift_init = 1;
		}
		for (i = 0; i < 3; i++)
		{
			if (color_shift[i] > color_shift_end[i])
				color_shift[i] = color_shift[i] - color_shift_steps[i];

			if (color_shift[i] < color_shift_end[i])
				color_shift[i] = color_shift_end[i];
		}
		if (cl.stats[STAT_ROUNDS] > 0 && cl.stats[STAT_ROUNDS] < 11)
		{
			for (i = 0; i < cl.stats[STAT_ROUNDS]; i++)
			{
				if (i == 4)
				{
				#ifdef VITA
					Draw_ColorStretchPic (5*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				#else
					Draw_ColorPic (5*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				#endif
					savex = x_offset + 10;
					x_offset = x_offset + 10;
					continue;
				}
				if (i == 9)
				{
				#ifdef VITA
					Draw_ColorStretchPic ((5 + savex/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				#else
					Draw_ColorPic ((5 + savex/2)*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				#endif
					continue;
				}
				if (i > 4)
					icon_num = i - 5;
				else
					icon_num = i;
			#ifdef VITA
				Draw_ColorStretchPic ((5 + x_offset/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[icon_num]->height - y_mark_offset, stretch_x, stretch_y, sb_round[icon_num], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round[icon_num]->width + 23;
			#else
				Draw_ColorPic ((5 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round[icon_num]->height - 4, sb_round[icon_num], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round[icon_num]->width + 3;
			#endif
			}
		}
		else
		{
			if (cl.stats[STAT_ROUNDS] >= 100)
			{
				num[2] = (int)(cl.stats[STAT_ROUNDS]/100);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[2]]->height - y_offset, 64, 96, sb_round_num[num[2]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round_num[num[2]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[2]]->height - 4, sb_round_num[num[2]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round_num[num[2]]->width - 8;
			#endif
			}
			else
				num[2] = 0;
			if (cl.stats[STAT_ROUNDS] >= 10)
			{
				num[1] = (int)((cl.stats[STAT_ROUNDS] - num[2]*100)/10);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[1]]->height - y_offset, 64, 96, sb_round_num[num[1]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round_num[num[1]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[1]]->height - 4, sb_round_num[num[1]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
				x_offset = x_offset + sb_round_num[num[1]]->width - 8;
			#endif
			}
			else
				num[1] = 0;

			num[0] = cl.stats[STAT_ROUNDS] - num[2]*100 - num[1]*10;
		#ifdef VITA
			Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[0]]->height - y_offset, 64, 96, sb_round_num[num[0]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
			x_offset = x_offset + sb_round_num[num[0]]->width + 25;
		#else
			Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[0]]->height - 4, sb_round_num[num[0]], (color_shift[0]/255), (color_shift[1]/255), (color_shift[2]/255), 1);
			x_offset = x_offset + sb_round_num[num[0]]->width - 8;
		#endif
		}
	}
	else
	{
		color_shift[0] = 107;
		color_shift[1] = 1;
		color_shift[2] = 0;
		color_shift_init = 0;
		alphabling = 0;
		if (cl.stats[STAT_ROUNDS] > 0 && cl.stats[STAT_ROUNDS] < 11)
		{
			for (i = 0; i < cl.stats[STAT_ROUNDS]; i++)
			{
				if (i == 4)
				{
				#ifdef VITA
					Draw_ColorStretchPic (5*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], 0.4196, 0.004, 0, 1);
				#else
					Draw_ColorPic (5*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], 0.4196, 0.004, 0, 1);
				#endif
					savex = x_offset + 10;
					x_offset = x_offset + 10;
					continue;
				}
				if (i == 9)
				{
				#ifdef VITA
					Draw_ColorStretchPic ((5 + savex/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[4]->height - y_mark_offset, 120, 96, sb_round[4], 0.4196, 0.004, 0, 1);
				#else
					Draw_ColorPic ((5 + savex/2)*(vid.width/2)/272, vid.height - sb_round[4]->height - 4, sb_round[4], 0.4196, 0.004, 0, 1);
				#endif
					continue;
				}
				if (i > 4)
					icon_num = i - 5;
				else
					icon_num = i;
			#ifdef VITA
				Draw_ColorStretchPic ((5 + x_offset/2)*(vid.width/2)/272 + x_mark_offset, vid.height - sb_round[icon_num]->height - y_mark_offset, stretch_x, stretch_y, sb_round[icon_num], 0.4196, 0.004, 0, 1);
				x_offset = x_offset + sb_round[icon_num]->width + 23;
			#else
				Draw_ColorPic ((5 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round[icon_num]->height - 4, sb_round[icon_num], 0.4196, 0.004, 0, 1);
				x_offset = x_offset + sb_round[icon_num]->width + 3;
			#endif
			}
		}
		else
		{
			if (cl.stats[STAT_ROUNDS] >= 100)
			{
				num[2] = (int)(cl.stats[STAT_ROUNDS]/100);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[2]]->height - y_offset, 64, 96, sb_round_num[num[2]], 0.4196, 0.004, 0, 1);
				x_offset = x_offset + sb_round_num[num[2]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[2]]->height - 4, sb_round_num[num[2]], 0.4196, 0.004, 0, 1);
				x_offset = x_offset + sb_round_num[num[2]]->width - 8;
			#endif
			}
			else
				num[2] = 0;
			if (cl.stats[STAT_ROUNDS] >= 10)
			{
				num[1] = (int)((cl.stats[STAT_ROUNDS] - num[2]*100)/10);
			#ifdef VITA
				Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[1]]->height - y_offset, 64, 96, sb_round_num[num[1]], 0.4196, 0.004, 0, 1);
				x_offset = x_offset + sb_round_num[num[1]]->width + 25;
			#else
				Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[1]]->height - 4, sb_round_num[num[1]], 0.4196, 0.004, 0, 1);
				x_offset = x_offset + sb_round_num[num[1]]->width - 8;
			#endif
			}
			else
				num[1] = 0;

			num[0] = cl.stats[STAT_ROUNDS] - num[2]*100 - num[1]*10;
			
			if(cl.stats[STAT_ROUNDS] == 0)
				return;
		#ifdef VITA
			Draw_ColorStretchPic ((2 + x_offset/2)*(vid.width/2)/272 - x_offset2, vid.height - sb_round_num[num[0]]->height - y_offset, 64, 96, sb_round_num[num[0]], 0.4196, 0.004, 0, 1);
			x_offset = x_offset + sb_round_num[num[0]]->width + 25;
		#else
			Draw_ColorPic ((2 + x_offset/2)*(vid.width/2)/272, vid.height - sb_round_num[num[0]]->height - 4, sb_round_num[num[0]], 0.4196, 0.004, 0, 1);
			x_offset = x_offset + sb_round_num[num[0]]->width - 8;
		#endif
		}
	}
}

//=============================================================================

/*
===============
HUD_Perks
===============
*/
#define 	P_JUG		1
#define 	P_DOUBLE	2
#define 	P_SPEED		4
#define 	P_REVIVE	8
#define 	P_FLOP		16
#define 	P_STAMIN	32
#define 	P_DEAD 		64
#define 	P_MULE 		128

int perk_order[9];
int current_perk_order;

void HUD_Perks (void)
{
	int x, y, scale;

#ifdef VITA
	x = 36;
	y = 6;
	scale = 45;
#else
	x = 26;
	y = 366;
	scale = 26;
#endif // VITA

	// Double-Tap 2.0 specialty icon
	qpic_t* double_tap_icon;
	if (doubletap_has_damage_buff)
		double_tap_icon = doublepic2;
	else
		double_tap_icon = doublepic;

	// Draw second column first -- these need to be
	// overlayed below the first column.
	for (int i = 4; i < 8; i++) {
		if (perk_order[i]) {
			if (perk_order[i] == P_JUG) {Draw_StretchPic(x, y, jugpic, scale, scale);}
			if (perk_order[i] == P_DOUBLE) {Draw_StretchPic(x, y, double_tap_icon, scale, scale);}
			if (perk_order[i] == P_SPEED) {Draw_StretchPic(x, y, speedpic, scale, scale);}
			if (perk_order[i] == P_REVIVE) {Draw_StretchPic(x, y, revivepic, scale, scale);}
			if (perk_order[i] == P_FLOP) {Draw_StretchPic(x, y, floppic, scale, scale);}
			if (perk_order[i] == P_STAMIN) {Draw_StretchPic(x, y, staminpic, scale, scale);}
			if (perk_order[i] == P_DEAD) {Draw_StretchPic(x, y, deadpic, scale, scale);}
			if (perk_order[i] == P_MULE) {Draw_StretchPic(x, y, mulepic, scale, scale);}
		}
		y += scale;
	}

#ifdef VITA
	x = 12;
	y = 6;
#else
	x = 8;
	y = 366;
#endif // VITA

	// Now the first column.
	for (int i = 0; i < 4; i++) {
		if (perk_order[i]) {
			if (perk_order[i] == P_JUG) {Draw_StretchPic(x, y, jugpic, scale, scale);}
			if (perk_order[i] == P_DOUBLE) {Draw_StretchPic(x, y, double_tap_icon, scale, scale);}
			if (perk_order[i] == P_SPEED) {Draw_StretchPic(x, y, speedpic, scale, scale);}
			if (perk_order[i] == P_REVIVE) {Draw_StretchPic(x, y, revivepic, scale, scale);}
			if (perk_order[i] == P_FLOP) {Draw_StretchPic(x, y, floppic, scale, scale);}
			if (perk_order[i] == P_STAMIN) {Draw_StretchPic(x, y, staminpic, scale, scale);}
			if (perk_order[i] == P_DEAD) {Draw_StretchPic(x, y, deadpic, scale, scale);}
			if (perk_order[i] == P_MULE) {Draw_StretchPic(x, y, mulepic, scale, scale);}
		}
		y += scale;
	}
}

//=============================================================================

/*
===============
HUD_Powerups
===============
*/
void HUD_Powerups (void)
{
	int count = 0;

	if (!cl.stats[STAT_X2] && !cl.stats[STAT_INSTA])
		return;

	// horrible way to offset check :)))))))))))))))))) :DDDDDDDD XOXO

	if (cl.stats[STAT_X2])
		count++;

	if (cl.stats[STAT_INSTA])
		count++;

	// both are avail draw fixed order
	if (count == 2) {
#ifdef VITA
		Draw_StretchPic(418, 480, x2pic, 64, 64);
		Draw_StretchPic(484, 480, instapic, 64, 64);
#else
		Draw_StretchPic(277, 678, x2pic, 32, 32);
		Draw_StretchPic(317, 678, instapic, 32, 32);
#endif // VITA
	} else {
		if (cl.stats[STAT_X2])
#ifdef VITA
			Draw_StretchPic(451, 480, x2pic, 64, 64);
#else
			Draw_StretchPic(306, 678, x2pic, 32, 32);
#endif // VITA
		if(cl.stats[STAT_INSTA])
#ifdef VITA
			Draw_StretchPic(451, 480, instapic, 64, 64);
#else
			Draw_StretchPic(306, 678, instapic, 32, 32);
#endif // VITA
	}
}

//=============================================================================


/*
===============
HUD_ProgressBar
===============
*/
void HUD_ProgressBar (void)
{
	float progressbar;

	if (cl.progress_bar)
	{
		progressbar = 100 - ((cl.progress_bar-sv.time)*10);
		if (progressbar >= 100)
			progressbar = 100;
		Draw_FillByColor  ((vid.width)/4 - 51, vid.height/2 + (vid.height/2)*0.75 - 1, 102, 5, 0, 0, 0, 100/255.0);
		Draw_FillByColor ((vid.width)/4 - 50, vid.height/2 + (vid.height/2)*0.75, progressbar, 3, 1, 0, 0, 100/255.0);

		Draw_String ((vid.width/2 - (88))/2, vid.height/2 + (vid.height/2)*0.75 + 10, "Reviving...");
	}
}

//=============================================================================

/*
===============
HUD_Achievement

Achievements based on code by Arkage
===============
*/

#ifdef VITA
int		achievement; // the background image
int		achievement_unlocked;
char		achievement_text[MAX_QPATH];
double		achievement_time;
float smallsec;
int ach_pic;

void HUD_Achievement (void)
{

	if (achievement_unlocked == 1)
	{
		
		smallsec = smallsec + 0.4;
		if (smallsec >= 30)
			smallsec = 30;
		//Background image
		//Sbar_DrawPic (176, 5, achievement);
		// The achievement text
		Draw_AlphaStretchPic (30, smallsec, 200, 100, 255, achievement_list[ach_pic].img);
	}

	// Reset the achievement
	if (Sys_DoubleTime() >= achievement_time)
	{
		achievement_unlocked = 0;
	}

}

void HUD_Parse_Achievement (int ach)
{
	if (achievement_list[ach].unlocked)
		return;

	achievement_unlocked = 1;
	smallsec = 0;
	achievement_time = Sys_DoubleTime() + 10;
	ach_pic = ach;
	achievement_list[ach].unlocked = 1;
	Save_Achivements();
}
#endif
//=============================================================================

/*
===============
HUD_Ammo
===============
*/

int GetLowAmmo(int weapon, int type)
{
	switch (weapon)
	{
		case W_COLT: if (type) return 2; else return 16;
		case W_KAR: if (type) return 1; else return 10;
		case W_KAR_SCOPE: if (type) return 1; else return 10;
		case W_M1A1: if (type) return 4; else return 24;
		case W_SAWNOFF: if (type) return 1; else return 12;
		case W_DB: if (type) return 1; else return 12;
		case W_THOMPSON: if (type) return 6; else return 40;
		case W_BAR: if (type) return 6; else return 28;
		default: return 0;
	}
}

int IsDualWeapon(int weapon)
{
	switch(weapon) {
		case W_BIATCH:
		case W_SNUFF:
			return 1;
		default:
			return 0;
	}

	return 0;
}


void HUD_Ammo (void)
{
	char* magstring;
	int reslen;

#ifdef VITA

	float scale = 2.0f;
	y_value = vid.height - 36;
	x_value = 860;

#else

	float scale = 1.25f;
	y_value = vid.height - 3 - fragpic->height;
	x_value = 575;

#endif // VITA

	reslen = strlen(va("/%i", cl.stats[STAT_AMMO]));

	//
	// Magazine
	//
	magstring = va("%i", cl.stats[STAT_CURRENTMAG]);
	if (GetLowAmmo(cl.stats[STAT_ACTIVEWEAPON], 1) >= cl.stats[STAT_CURRENTMAG]) {
		Draw_ColoredStringScale((x_value - (reslen*8*scale)) - strlen(magstring)*8*scale, y_value, magstring, 1, 0, 0, 1, scale);
	} else {
		Draw_ColoredStringScale((x_value - (reslen*8*scale)) - strlen(magstring)*8*scale, y_value, magstring, 1, 1, 1, 1, scale);
	}

	//
	// Reserve Ammo
	//
	magstring = va("/%i", cl.stats[STAT_AMMO]);
	if (GetLowAmmo(cl.stats[STAT_ACTIVEWEAPON], 0) >= cl.stats[STAT_AMMO]) {
		Draw_ColoredStringScale(x_value - strlen(magstring)*8*scale, y_value, magstring, 1, 0, 0, 1, scale);
	} else {
		Draw_ColoredStringScale(x_value - strlen(magstring)*8*scale, y_value, magstring, 1, 1, 1, 1, scale);
	}

	//
	// Second Magazine
	//
	if (IsDualWeapon(cl.stats[STAT_ACTIVEWEAPON])) {
		magstring = va("%i", cl.stats[STAT_CURRENTMAG2]);
		if (GetLowAmmo(cl.stats[STAT_ACTIVEWEAPON], 0) >= cl.stats[STAT_CURRENTMAG2]) {
			Draw_ColoredStringScale(x_value - 34*scale - strlen(magstring)*8*scale, y_value, magstring, 1, 0, 0, 1, scale);
		} else {
			Draw_ColoredStringScale(x_value - 34*scale - strlen(magstring)*8*scale, y_value, magstring, 1, 1, 1, 1, scale);
		}
	}
}

/*
===============
HUD_AmmoString
===============
*/

void HUD_AmmoString (void)
{
	int len = 0;
	
	if (GetLowAmmo(cl.stats[STAT_ACTIVEWEAPON], 1) >= cl.stats[STAT_CURRENTMAG])
	{
		if (0 < cl.stats[STAT_AMMO] && cl.stats[STAT_CURRENTMAG] >= 0) {
		#ifdef VITA
			Draw_ColoredStringScale ((vid.width)/2 - 43, (vid.height)/2 + 34, "Reload", 1, 1, 1, 1, 2.0f);
		#else
			Draw_ColoredStringScale(vid.width/4 - strlen("Reload")*6, (vid.height)*3/4 + 40, "Reload", 1, 1, 1, 1, 1.5f);
		#endif
		} else if (0 < cl.stats[STAT_CURRENTMAG]) {
		#ifdef VITA
			Draw_ColoredStringScale ((vid.width)/2 - 73, (vid.height)/2 + 34, "LOW AMMO", 1, 1, 0, 1, 2.5f);
		#else
			Draw_ColoredStringScale(vid.width/4 - strlen("LOW AMMO")*6, (vid.height)*3/4 + 40, "LOW AMMO", 1, 1, 0, 1, 1.5f);
		#endif
		} else {
		#ifdef VITA
			Draw_ColoredStringScale ((vid.width)/2 - 66, (vid.height)/2 + 34, "NO AMMO", 1, 0, 0, 1, 2.5f);
		#else
			Draw_ColoredStringScale(vid.width/4 - strlen("NO AMMO")*6, (vid.height)*3/4 + 40, "NO AMMO", 1, 0, 0, 1, 1.5f);
		#endif
		}
	}

}

//=============================================================================

/*
===============
HUD_Grenades
===============
*/
#define 	UI_FRAG		1
#define 	UI_BETTY	2

void HUD_Grenades (void)
{
	if (cl.stats[STAT_GRENADES])
	{	
	#ifdef VITA
		x_value = vid.width - bettypic->width - 70;
		y_value = vid.height - 40 - fragpic->height;
	#else
		x_value = vid.width/2 - 40;
		y_value = vid.height - 16 - fragpic->height;
	#endif
	}
	if (cl.stats[STAT_GRENADES] & UI_FRAG)
	{
	#ifdef VITA
		Draw_StretchPic (x_value, y_value, fragpic, 64, 64);
		if (cl.stats[STAT_PRIGRENADES] <= 0)
			Draw_ColoredStringScale (x_value + 36, y_value + 44, va ("%i",cl.stats[STAT_PRIGRENADES]), 1, 0, 0, 1, 2.0f);
		else
			Draw_ColoredStringScale (x_value + 36, y_value + 44, va ("%i",cl.stats[STAT_PRIGRENADES]), 1, 1, 1, 1, 2.0f);
	#else
		Draw_StretchPic (x_value - 24, y_value, fragpic, 26, 26);
		if (cl.stats[STAT_PRIGRENADES] <= 0)
			Draw_ColoredStringScale (x_value + 15 - 24, y_value + 18, va ("%i",cl.stats[STAT_PRIGRENADES]), 1, 0, 0, 1, 1.25f);
		else
			Draw_ColoredStringScale (x_value + 15 - 24, y_value + 18, va ("%i",cl.stats[STAT_PRIGRENADES]), 1, 1, 1, 1, 1.25f);
	#endif
	}
	if (cl.stats[STAT_GRENADES] & UI_BETTY)
	{
	#ifdef VITA
		Draw_StretchPic (x_value + fragpic->width + 15, y_value, bettypic, 64, 64);
		if (cl.stats[STAT_SECGRENADES] <= 0) {
			Draw_ColoredStringScale (x_value + fragpic->width + 46, y_value + 44, va ("%i",cl.stats[STAT_SECGRENADES]), 1, 0, 0, 1, 2.0f);
		} else {
			Draw_ColoredStringScale (x_value + fragpic->width + 46, y_value + 44, va ("%i",cl.stats[STAT_SECGRENADES]), 1, 1, 1, 1, 2.0f);
		}
	#else
		Draw_StretchPic (x_value, y_value, bettypic, 26, 26);
		if (cl.stats[STAT_SECGRENADES] <= 0) {
			Draw_ColoredStringScale (x_value + 15, y_value + 18, va ("%i",cl.stats[STAT_SECGRENADES]), 1, 0, 0, 1, 1.25f);
		} else {
			Draw_ColoredStringScale (x_value + 15, y_value + 18, va ("%i",cl.stats[STAT_SECGRENADES]), 1, 1, 1, 1, 1.25f);
		}
	#endif
	}
}

//=============================================================================

/*
===============
HUD_Weapon
===============
*/


char *GetWeaponName (int wep)
{
	switch (wep)
	{
		case W_COLT:
			return "Colt M1911";
		case W_BIATCH:
      		return "Mustang & Sally";
		case W_KAR:
			return "Kar 98k";
    	case W_ARMAGEDDON:
			return "Armageddon";
		case W_THOMPSON:
			return "Thompson";
		case W_GIBS:
			return "Gibs-O-Matic";
		case W_357:
			return ".357 Magnum";
		case W_KILLU:
			return ".357 Plus 1 K1L-u";
		case W_BAR:
			return "BAR";
		case W_WIDOW:
			return "The Widow Maker";
		case W_BK:
			return "Ballistic Knife";
		case W_BROWNING:
			return "Browning M1919";
		case W_ACCELERATOR:
			return "B115 Accelerator";
		case W_DB:
			return "Double-Barreled Shotgun";
		case W_BORE:
			return "24 Bore Long Range";
		case W_FG:
			return "FG42";
		case W_IMPELLER:
			return "420 Impeller";
		case W_GEWEHR:
			return "Gewehr 43";
		case W_COMPRESSOR:
			return "G115 Compressor";
		case W_KAR_SCOPE:
			return "Scoped Kar 98k";
		case W_HEADCRACKER:
			return "Headcracker";
		case W_M1:
			return "M1 Garand";
		case W_M1000:
			return "M1000";
		case W_M1A1:
			return "M1A1 Carbine";
		case W_WIDDER:
			return "Widdershins RC-1";
		case W_M2:
			return "M2 Flamethrower";
		case W_FIW:
			return "F1W Nitrogen Cooled";
		case W_MP40:
			return "MP40";
		case W_AFTERBURNER:
			return "The Afterburner";
		case W_MG:
			return "MG42";
		case W_BARRACUDA:
			return "Barracuda FU-A11";
		case W_PANZER:
			return "Panzerschrek";
		case W_LONGINUS:
			return "Longinus";
		case W_PPSH:
			return "PPSh-41";
		case W_REAPER:
			return "The Reaper";
		case W_PTRS:
			return "PTRS-41";
		case W_PENETRATOR:
			return "The Penetrator";
		case W_RAY:
			return "Ray Gun";
		case W_PORTER:
			return "Porter's X2 Ray Gun";
		case W_SAWNOFF:
			return "Sawnoff shotgun";
		case W_SNUFF:
			return "The Snuff Box";
		case W_STG:
			return "STG-44";
		case W_SPATZ:
			return "Spatz-447 +";
		case W_TRENCH:
			return "M1897 Trench Gun";
		case W_GUT:
			return "Gut Shot";
		case W_TYPE:
			return "Type 100";
		case W_SAMURAI:
			return "1001 Samurais";
    	case W_TESLA:
      		return "Wunderwaffe DG-2";
		default:
			return " ";
	}
}

extern char *pr_strings;
void HUD_Weapon (void)
{
	char str[32];
	float l;
#ifdef VITA
	y_value = 480;
#else
	y_value = vid.height - 16 - fragpic->height;
#endif
	strcpy(str, cl.weaponname);
	//strcpy(str, GetWeaponName(cl.stats[STAT_ACTIVEWEAPON]));
	l = strlen(str);
#ifdef VITA
	x_value = vid.width - fragpic->width - 65 - l*16;
	Draw_ColoredStringScale (x_value, y_value, str, 1, 1, 1, 1, 2.0f);
#else
	x_value = vid.width/2 - 63 - l*10;
	Draw_ColoredStringScale (x_value, y_value, str, 1, 1, 1, 1, 1.25f);
#endif
}

/*
===============
HUD_BettyPrompt
===============
*/
void HUD_BettyPrompt (void)
{
	char str[64];
	char str2[32];

#ifdef VITA
	strcpy(str, va("Tap    then press %s to\n", GetGrenadeButtonL()));
	strcpy(str2, "place a Bouncing Betty\n");
#else
	strcpy(str, va("Press %s to place a\n", GetBettyButtonL()));
	strcpy(str2, "Bouncing Betty\n");
#endif // VITA

	int x, x2, y;
#ifdef VITA
	x = (vid.width - strlen(str)*16)/2;
	x2 = (vid.width - strlen(str2)*16)/2;
	y = 165;
#else
	x = (vid.width - strlen(str)*10)/2;
	x2 = (vid.width - strlen(str2)*10)/2;
	y = vid.height*3/4 - sb_round[0]->height - 15;
#endif // VITA

#ifdef VITA

	Draw_ColoredStringScale(x, y, str, 255, 255, 255, 255, 2.0f);
	Draw_ColoredStringScale(x, y + 24, str2, 255, 255, 255, 255, 2.0f);
	Draw_Pic (x + 4*16 - 6, y - 5, b_touch);
	Draw_Pic (x + 18*16, y - 5, GetButtonIcon("+grenade"));

#else

	Draw_ColoredStringScale(x, y, str, 255, 255, 255, 255, 1.25f);
	Draw_ColoredStringScale(x, y + 12, str2, 255, 255, 255, 255, 1.25f);
	Draw_Pic (x + 6*10, y, GetButtonIcon("impulse 33"));

#endif // VITA
}

/*
===============
HUD_PlayerName
===============
*/
void HUD_PlayerName (void)
{
	float alpha = 1.0;
	int x, y;
	float scale;

#ifdef VITA
	scale = 2.0f;
	x = 147;
	y = 413;
#else
	scale = 1.25f;
	x = 100;
	y = 633;
#endif

	if (nameprint_time - sv.time < 1)
		alpha = (nameprint_time - sv.time);

	Draw_ColoredStringScale(x, y, player_name, 255, 255, 255, alpha, scale);
}

/*
===============
HUD_Screenflash
===============
*/

//
// Types of screen-flashes.
//

// Colors
#define SCREENFLASH_COLOR_WHITE			0
#define SCREENFLASH_COLOR_BLACK			1

// Types
#define SCREENFLASH_FADE_INANDOUT		0
#define SCREENFLASH_FADE_IN 			1
#define SCREENFLASH_FADE_OUT 			2

//
// invert float takes in float value between 0 and 1, inverts position
// eg: 0.1 returns 0.9, 0.34 returns 0.66
float invertfloat(float input) {
    if (input < 0)
        return 0; // adjust to lower boundary
    else if (input > 1)
        return 1; // adjust to upper boundary
    else
        return (1 - input);
}

void HUD_Screenflash (void)
{
	int r, g, b, a;
	float flash_alpha;

	double percentage_complete = screenflash_worktime / (screenflash_duration - screenflash_starttime);

	// Fade Out
	if (screenflash_type == SCREENFLASH_FADE_OUT) {
		flash_alpha = invertfloat((float)percentage_complete);
	}
	// Fade In
	else if (screenflash_type == SCREENFLASH_FADE_IN) {
		flash_alpha = (float)percentage_complete;
	}
	// Fade In + Fade Out
	else {
		// Fade In
		if (percentage_complete < 0.5) {
			flash_alpha = (float)percentage_complete*2;
		} 
		// Fade Out
		else {
			flash_alpha = invertfloat((float)percentage_complete)*2;
		}
	}

	// Obtain the flash color
	switch(screenflash_color) {
		case SCREENFLASH_COLOR_BLACK: r = 0; g = 0; b = 0; a = (int)(flash_alpha * 255); break;
		case SCREENFLASH_COLOR_WHITE: r = 255; g = 255; b = 255; a = (int)(flash_alpha * 255); break;
		default: r = 255; g = 0; b = 0; a = 255; break;
	}

	screenflash_worktime += host_frametime;

#ifdef VITA
	Draw_FillByColor(0, 0, vid.width, vid.height, r, g, b, a);
#else
	Draw_FillByColor(0, vid.height * 0.5, vid.width/2, vid.height/2, r, g, b, a);
#endif
}

//=============================================================================


void HUD_Draw (void) {
	if (m_state == m_exit || paused_hack == true)
		return;

	if (key_dest == key_menu_pause) {
		// Make sure we still draw the screen flash.
		if (screenflash_duration > sv.time)
			HUD_Screenflash();
		return;
	}

	GL_SetCanvas(CANVAS_USEPRINT);

	if (waypoint_mode.value) {
#ifndef VITA
		Draw_ColoredStringScale(475, 364, "WAYPOINT MODE", 1, 1, 1, 1, 1.5f);
		Draw_ColoredStringScale(329, 382, "FIRE   to Create Waypoint", 1, 1, 1, 1, 1.5f);
		Draw_ColoredStringScale(329, 395, "USE    to Select Waypoint", 1, 1, 1, 1, 1.5f);
		Draw_ColoredStringScale(329, 408, "AIM    to Link   Waypoint", 1, 1, 1, 1, 1.5f);
		Draw_ColoredStringScale(329, 421, "KNIFE  to Remove Waypoint", 1, 1, 1, 1, 1.5f);
		Draw_ColoredStringScale(329, 434, "SWITCH to Move   Waypoint", 1, 1, 1, 1, 1.5f);
		Draw_ColoredStringScale(329, 447, "RELOAD to Evolve Waypoint", 1, 1, 1, 1, 1.5f);
#else
		Draw_ColoredStringScale(741, 4,   "WAYPOINT MODE", 1, 1, 1, 1, 2.0f);
		Draw_ColoredStringScale(549, 29,  "FIRE   to Create Waypoint", 1, 1, 1, 1, 2.0f);
		Draw_ColoredStringScale(549, 49,  "USE    to Select Waypoint", 1, 1, 1, 1, 2.0f);
		Draw_ColoredStringScale(549, 69,  "AIM    to Link   Waypoint", 1, 1, 1, 1, 2.0f);
		Draw_ColoredStringScale(549, 89,  "KNIFE  to Remove Waypoint", 1, 1, 1, 1, 2.0f);
		Draw_ColoredStringScale(549, 109, "SWITCH to Move   Waypoint", 1, 1, 1, 1, 2.0f);
		Draw_ColoredStringScale(549, 129, "RELOAD to Evolve Waypoint", 1, 1, 1, 1, 2.0f);
#endif
		return;
	}

	if (cl.stats[STAT_HEALTH] <= 0) {
		HUD_EndScreen ();

		// Make sure we still draw the screen flash.
		if (screenflash_duration > sv.time)
			HUD_Screenflash();
		return;
	}

	if (bettyprompt_time > sv.time)
		HUD_BettyPrompt();

	if (nameprint_time > sv.time)
		HUD_PlayerName();

	HUD_Blood(); 
	HUD_Rounds();
	HUD_Perks();
	HUD_Powerups();
	HUD_ProgressBar();
	if ((HUD_Change_time > Sys_DoubleTime() || GetLowAmmo(cl.stats[STAT_ACTIVEWEAPON], 1) >= cl.stats[STAT_CURRENTMAG] || GetLowAmmo(cl.stats[STAT_ACTIVEWEAPON], 0) >= cl.stats[STAT_AMMO]) && cl.stats[STAT_HEALTH] >= 20)
	{ //these elements are only drawn when relevant for few seconds
		HUD_Ammo();
		HUD_Grenades();
		HUD_Weapon();
		HUD_AmmoString();
	}
	HUD_Points();
	HUD_Point_Change();
#ifdef VITA
	HUD_Achievement();
#endif

	if (domaxammo == true) {
		if (maxammoopac <= 0) {
			maxammoopac = 255;
#ifdef VITA
			maxammoy = 150;
#else
			maxammoy = vid.height*3/4 - sb_round[0]->height - 10;
#endif // VITA
		}
		HUD_MaxAmmo();
	}

	// This should always come last!
	if (screenflash_duration > sv.time)
		HUD_Screenflash();

	GL_SetCanvas(CANVAS_DEFAULT);
}
