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

qpic_t		*sb_round[5];
qpic_t		*sb_round_num[10];
qpic_t		*sb_moneyback;
qpic_t		*instapic;
qpic_t		*x2pic;
qpic_t 		*revivepic;
qpic_t		*jugpic;
qpic_t		*floppic;
qpic_t		*staminpic;
qpic_t		*doublepic;
qpic_t		*speedpic;
qpic_t		*deadpic;
qpic_t 		*mulepic;
qpic_t		*fragpic;
qpic_t		*bettypic;

qpic_t      *fx_blood_lu;

int old_points;
int current_points;
int point_change_interval;
int point_change_interval_neg;
int alphabling = 0;
float round_center_x;
float round_center_y;

extern qboolean paused_hack;

double HUD_Change_time;//hide hud when not chagned

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

	for (i=0 ; i<5 ; i++)
	{
		sb_round[i] = Draw_CachePic (va("gfx/hud/r%i.tga",i + 1));
	}

	for (i=0 ; i<10 ; i++)
	{
		sb_round_num[i] = Draw_CachePic (va("gfx/hud/r_num%i.tga",i));
	}

	sb_moneyback = Draw_CachePic ("gfx/hud/moneyback.tga");
	instapic = Draw_CachePic ("gfx/hud/in_kill.tga");
	x2pic = Draw_CachePic ("gfx/hud/2x.tga");

	revivepic = Draw_CachePic ("gfx/hud/revive.tga");
	jugpic = Draw_CachePic ("gfx/hud/jug.tga");
	floppic = Draw_CachePic ("gfx/hud/flopper.tga");
	staminpic = Draw_CachePic ("gfx/hud/stamin.tga");
	doublepic = Draw_CachePic ("gfx/hud/double.tga");
	speedpic = Draw_CachePic ("gfx/hud/speed.tga");
	deadpic = Draw_CachePic ("gfx/hud/dead.tga");
	mulepic = Draw_CachePic ("gfx/hud/mule.tga");
	fragpic = Draw_CachePic ("gfx/hud/frag.tga");
	bettypic = Draw_CachePic ("gfx/hud/betty.tga");

#ifdef VITA
	if (sceKernelGetModel() == 0x20000) { // PSTV
		b_lt = Draw_CachePic ("gfx/butticons/backl1_pstv.tga");
		b_rt = Draw_CachePic ("gfx/butticons/backr1_pstv.tga");
		b_lthumb = Draw_CachePic ("gfx/butticons/backl1_pstv.tga"); // Not existent
		b_rthumb = Draw_CachePic ("gfx/butticons/backr1_pstv.tga"); // Not existent
		b_lshoulder = Draw_CachePic ("gfx/butticons/backl1_pstv.tga"); // Not existent
		b_rshoulder = Draw_CachePic ("gfx/butticons/backr1_pstv.tga"); // Not existent
	} else {
		b_lt = Draw_CachePic ("gfx/butticons/backl1.tga");
		b_rt = Draw_CachePic ("gfx/butticons/backr1.tga");
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

	old_points = 500;
	current_points = 500;
	point_change_interval = 0;
	point_change_interval_neg = 0;

	round_center_x = (vid.width/2 - sb_round[0]->width) /2;
	round_center_y = vid.height*3/4 - sb_round[0]->height/2;
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
	if(cl.stats[STAT_ROUNDS] >= 1)
	{
		HUD_Sortpoints ();

		l = scoreboardlines;

		Draw_String ((vid.width/2 - 9*8)/2, vid.height/2 + (vid.height)*40/272, "GAME OVER");

		sprintf (str,"You survived %3i rounds", cl.stats[STAT_ROUNDS]);
		Draw_String ((vid.width/2 - strlen (str)*8)/2, vid.height/2 + (vid.height)*48/272, str);

		sprintf (str,"Name           Kills     Points");
		x = (vid.width/2 - strlen (str)*8)/2;

		Draw_String (x, vid.height/2 + vid.height*68/272, str);
		y = 0;
		for (i=0; i<l ; i++)
		{
			k = pointsort[i];
			s = &cl.scores[k];
			if (!s->name[0])
				continue;

			Draw_String (x, vid.height/2 + vid.height*78/272 + y, s->name);

			d = strlen (va("%i",s->kills));
			Draw_String (x + (20 - d)*8, vid.height/2 + (vid.height)*78/272 + y, va("%i",s->kills));

			d = strlen (va("%i",s->points));
			Draw_String (x + (31 - d)*8, vid.height/2 + (vid.height)*78/272 + y, va("%i",s->points));
			y += 20;
		}
	}
	else
	{
		return;
	}

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
	point_change[i].x = x_start - 10.0 - 4.0*f;
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

	// draw background

	// draw number
		f = s->points;
		if (f > current_points)
		{
			point_change_interval_neg = 0;
			if (!point_change_interval)
			{
				point_change_interval = (int)(f - old_points)/55;;
			}
			current_points = old_points + point_change_interval;
			if (f < current_points)
			{
				current_points = f;
				point_change_interval = 0;
			}
		}
		else if (f < current_points)
		{
			point_change_interval = 0;
			if (!point_change_interval_neg)
			{
				point_change_interval_neg = (int)(old_points - f)/55;
			}
			current_points = old_points - point_change_interval_neg;
			if (f > current_points)
			{
				current_points = f;
				point_change_interval_neg = 0;
			}
		}
#ifdef VITA
		Draw_StretchPic(12, 407, sb_moneyback, 128, 32);
#else
		Draw_StretchPic(8, 629, sb_moneyback, 86, 21);
#endif // VITA
		xplus = HUD_itoa (f, str);
		
#ifdef VITA
		Draw_ColoredStringScale (106 - (xplus*16), 415, va("%i", current_points), 1, 1, 1, 1, 2.0f); //2x Scale/White
#else
		Draw_String (vid.width/2 - (xplus*8) - 16, y + 3, va("%i", current_points));
#endif // VITA

		if (old_points != f)
		{
			if (f > old_points)
			{
			#ifdef VITA
				HUD_Parse_Point_Change(f - old_points, 0, 80 - (xplus*12), 415);
			#else 
				HUD_Parse_Point_Change(f - old_points, 0, vid.width/2 - (xplus*8) - 16, y + 3);	
			#endif // VITA
			}
			else
			{
			#ifdef VITA
				HUD_Parse_Point_Change(old_points - f, 1, 80 - (xplus*12), 415);
			#else
				HUD_Parse_Point_Change(old_points - f, 1, vid.width/2 - (xplus*8) - 16, y + 3);
			#endif // VITA
			}
			old_points = f;
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
				Draw_ColoredStringScale (point_change[i].x, point_change[i].y, va ("-%i", point_change[i].points), 1, 0, 0, 1, 1.5f);
			#else
				Draw_ColoredString (point_change[i].x, point_change[i].y, va ("-%i", point_change[i].points), 1, 0, 0, 1);
			#endif
			}
			else
			{
			#ifdef VITA
				Draw_ColoredStringScale (point_change[i].x, point_change[i].y, va ("+%i", point_change[i].points), 1, 1, 0, 1, 1.5f);
			#else
				Draw_ColoredString (point_change[i].x, point_change[i].y, va ("+%i", point_change[i].points), 1, 1, 0, 1);
			#endif
			}
			point_change[i].y = point_change[i].y + point_change[i].move_y;
			point_change[i].x = point_change[i].x - point_change[i].move_x;
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

//=============================================================================

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
void HUD_Rounds (void)
{
	int i, x_offset, icon_num, savex;
	int num[3];
	x_offset = 0;
	savex = 0;
	
	int x_offset2 = 2; //Extra offset for all round images to keep things uniform (may not be neccesary?) -- SPECIFIC TO WHOLE ROUNDS
	int y_offset = 50; //y_offset -- SPECIFIC TO WHOLE ROUNDS
	int x_mark_offset = 10; //Needed x offset for stretched marks 
	int y_mark_offset = 50; //y offset specific to marks
	int stretch_x = 22; //Stretch round mark image x
	int stretch_y = 96; //Stretch round mark image y

	if (cl.stats[STAT_ROUNDCHANGE] == 1)//this is the rounds icon at the middle of the screen
	{
	#ifdef VITA
		Draw_ColorStretchPic ((vid.width/2 - sb_round[0]->width) /2 - x_offset2, vid.height*3/4 - sb_round[0]->height/2, stretch_x, stretch_y, sb_round[0], 0.4196, 0.004, 0, alphabling/255);
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
		
		round_center_x = round_center_x - ((229/108)*2 - 0.2)*(vid.width/2)/480;
		round_center_y = round_center_y + ((vid.height*1.015)/272);
		if (round_center_x <= 5)
			round_center_x = 5;
		if (round_center_y >= 250*vid.height/272)
			round_center_y = 250*vid.height/272;
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
		blinking = (float)(((int)(realtime*1000)&510) - 255);
		blinking = abs(blinking);
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
		color_shift_init = 0;
		blinking = ((int)(realtime*1000)&510) - 255;
		blinking = abs(blinking);
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
	scale = 30;
#endif // VITA

	// Draw second column first -- these need to be
	// overlayed below the first column.
	for (int i = 4; i < 8; i++) {
		if (perk_order[i]) {
			if (perk_order[i] == P_JUG) {Draw_StretchPic(x, y, jugpic, scale, scale);}
			if (perk_order[i] == P_DOUBLE) {Draw_StretchPic(x, y, doublepic, scale, scale);}
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
	x = 10;
	y = 366;
#endif // VITA

	// Now the first column.
	for (int i = 0; i < 4; i++) {
		if (perk_order[i]) {
			if (perk_order[i] == P_JUG) {Draw_StretchPic(x, y, jugpic, scale, scale);}
			if (perk_order[i] == P_DOUBLE) {Draw_StretchPic(x, y, doublepic, scale, scale);}
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
	int count;

	// horrible way to offset check :)))))))))))))))))) :DDDDDDDD XOXO

	if (cl.stats[STAT_X2])
		count++;

	if (cl.stats[STAT_INSTA])
		count++;

	// both are avail draw fixed order
	if (count == 2) {
#ifdef VITA
		Draw_StretchPic(422, 480, x2pic, 64, 64);
		Draw_StretchPic(480, 480, instapic, 64, 64);
#else
		Draw_StretchPic(275, 672, x2pic, 42, 42);
		Draw_StretchPic(319, 672, instapic, 42, 42);
#endif // VITA
	} else {
		if (cl.stats[STAT_X2])
#ifdef VITA
			Draw_StretchPic(451, 480, x2pic, 64, 64);
#else
			Draw_StretchPic(299, 672, x2pic, 42, 42);
#endif // VITA
		if(cl.stats[STAT_INSTA])
#ifdef VITA
			Draw_StretchPic(451, 480, instapic, 64, 64);
#else
			Draw_StretchPic(299, 672, instapic, 42, 42);
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


int		achievement; // the background image
int		achievement_unlocked;
char		achievement_text[MAX_QPATH];
double		achievement_time;
float smallsec;
int ach_pic;

//sb FIXME!
achievement_list_t achievement_list[MAX_ACHIEVEMENTS];
qpic_t *achievement_locked;

void HUD_Achievement (void)
{

	if (achievement_unlocked == 1)
	{
		smallsec = smallsec + 0.7;
		if (smallsec >= 55)
			smallsec = 55;
		//Background image
		//Sbar_DrawPic (176, 5, achievement);
		// The achievement text
		Draw_AlphaStretchPic (30, smallsec - 50, 200, 100, 0.7f, achievement_list[ach_pic].img);
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
	//Save_Achivements();
}

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
	char str[12];
	char str2[12];
    int xplus, xplus2;
	char *magstring;
	char *mag2string;

#ifdef VITA
	y_value = vid.height - 36;
#else
	y_value = vid.height - 16;
#endif
	if (GetLowAmmo(cl.stats[STAT_ACTIVEWEAPON], 1) >= cl.stats[STAT_CURRENTMAG])
		magstring = va ("%i",cl.stats[STAT_CURRENTMAG]);
	else
		magstring = va ("%i",cl.stats[STAT_CURRENTMAG]);

	xplus = HUD_itoa (cl.stats[STAT_CURRENTMAG], str);
#ifdef VITA
	Draw_ColoredStringScale (790 - (xplus*16), y_value, magstring, 1, 1, 1, 1, 2.0f);
#else
	Draw_ColoredString (vid.width/2 - 42 - (xplus*8), y_value, magstring, 1, 1, 1, 1);
#endif

	mag2string = va("%i", cl.stats[STAT_CURRENTMAG2]);
	xplus2 = HUD_itoa (cl.stats[STAT_CURRENTMAG2], str2);

	if (IsDualWeapon(cl.stats[STAT_ACTIVEWEAPON])) {
	#ifdef VITA
		Draw_ColoredStringScale (790 - (xplus2*16), y_value, mag2string, 1, 1, 1, 1, 2.0f);
	#else
		Draw_ColoredString (vid.width/2 - 56 - (xplus2*8), y_value, mag2string, 1, 1, 1, 1);
	#endif
	}

	if (GetLowAmmo(cl.stats[STAT_ACTIVEWEAPON], 0) >= cl.stats[STAT_AMMO])
	{
	#ifdef VITA
		Draw_ColoredStringScale (795, y_value, "/", 1, 0, 0, 1, 2.0f);
		Draw_ColoredStringScale (810, y_value, va ("%i",cl.stats[STAT_AMMO]), 1, 0, 0, 1, 2.0f);
	#else
		Draw_ColoredString (vid.width/2 - 42, y_value, "/", 1, 0, 0, 1);
		Draw_ColoredString (vid.width/2 - 34, y_value, va ("%i",cl.stats[STAT_AMMO]), 1, 0, 0, 1);
	#endif
	}
	else
	{
	#ifdef VITA
		Draw_ColoredStringScale (795, y_value, "/", 1, 1, 1, 1, 2.0f);
		Draw_ColoredStringScale (810, y_value, va ("%i",cl.stats[STAT_AMMO]), 1, 1, 1, 1, 2.0f);
	#else
		Draw_Character (vid.width/2 - 42, y_value, '/');
		Draw_String (vid.width/2 - 34, y_value, va ("%i",cl.stats[STAT_AMMO]));
	#endif
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
			Draw_String ((vid.width)/4, (vid.height)*3/4 + 40, "Reload");
		#endif
		} else if (0 < cl.stats[STAT_CURRENTMAG]) {
		#ifdef VITA
			Draw_ColoredStringScale ((vid.width)/2 - 73, (vid.height)/2 + 34, "LOW AMMO", 1, 1, 0, 1, 2.5f);
		#else
			Draw_ColoredString ((vid.width/2 - len*8)/2, (vid.height)*3/4 + 40, "LOW AMMO", 1, 1, 0, 1);
		#endif
		} else {
		#ifdef VITA
			Draw_ColoredStringScale ((vid.width)/2 - 66, (vid.height)/2 + 34, "NO AMMO", 1, 0, 0, 1, 2.5f);
		#else
			Draw_ColoredString ((vid.width/2 - len*8)/2, (vid.height)*3/4 + 40, "NO AMMO", 1, 0, 0, 1);
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
		x_value = vid.width/2 - 50;
		y_value = vid.height - 16 - fragpic->height - 4;
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
		Draw_Pic (x_value, y_value, fragpic);
		if (cl.stats[STAT_PRIGRENADES] <= 0)
			Draw_ColoredString (x_value + 24, y_value + 28, va ("%i",cl.stats[STAT_PRIGRENADES]), 1, 0, 0, 1);
		else
			Draw_String (x_value + 24, y_value + 28, va ("%i",cl.stats[STAT_PRIGRENADES]));
	#endif
	}
	if (cl.stats[STAT_GRENADES] & UI_BETTY)
	{
	#ifdef VITA
		Draw_StretchPic (x_value + fragpic->width + 15, y_value, bettypic, 64, 64);
		if (cl.stats[STAT_PRIGRENADES] <= 0) {
			Draw_ColoredStringScale (x_value + 46, y_value + 44, va ("%i",cl.stats[STAT_SECGRENADES]), 1, 0, 0, 1, 2.0f);
		} else {
			Draw_ColoredStringScale (x_value + fragpic->width + 46, y_value + 44, va ("%i",cl.stats[STAT_SECGRENADES]), 1, 1, 1, 1, 2.0f);
		}
	#else
		Draw_Pic (x_value - fragpic->width - 5, y_value, bettypic);
		if (cl.stats[STAT_PRIGRENADES] <= 0) {
			Draw_ColoredString (x_value + 24, y_value + 28, va ("%i",cl.stats[STAT_SECGRENADES]), 1, 0, 0, 1);
		} else {
			Draw_String (x_value - fragpic->width + 20, y_value + 28, va ("%i",cl.stats[STAT_SECGRENADES]));
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
	y_value = vid.height - 16 - fragpic->height - 4 - 16;
#endif
	strcpy(str, pr_strings+sv_player->v.Weapon_Name);
	//strcpy(str, GetWeaponName(cl.stats[STAT_ACTIVEWEAPON]));
	l = strlen(str);
#ifdef VITA
	x_value = vid.width - fragpic->width - 65 - l*16;
	Draw_ColoredStringScale (x_value, y_value, str, 1, 1, 1, 1, 2.0f);
#else
	x_value = vid.width/2 - 8 - l*8;
	Draw_String (x_value, y_value, str);
#endif
}

//=============================================================================


void HUD_Draw (void) {
	if (key_dest == key_menu_pause || paused_hack == true || m_state == m_exit) {
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

	if (cl.stats[STAT_HEALTH] <= 0) 
	{
		HUD_EndScreen ();
		return;
	}

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
/*
	HUD_Achievement();
*/

	GL_SetCanvas(CANVAS_DEFAULT);
}