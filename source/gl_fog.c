/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2007-2008 Kristian Duske
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
//gl_fog.c -- global and volumetric fog

//sB EDITED THIS AND GOT RID OF VOLUMETRIC FOG IN ORDER TO ADD CONSISTENCY TO NZP BUILDS. SWITCHED TO A START/END VALUE. 

#include "quakedef.h"

//==============================================================================
//
//  GLOBAL FOG
//
//==============================================================================

#define DEFAULT_DENSITY 1.0
#define DEFAULT_GRAY 0.3

//extern refdef_t r_refdef;

float density = 1.0;
float fog_density_gl;

float 		fog_start;
float 		fog_end;
float 		fog_red;
float 		fog_green;
float 		fog_blue;

float old_density;
float old_start;
float old_end;
float old_red;
float old_green;
float old_blue;

float fade_time; //duration of fade
float fade_done; //time when fade will be done

/*
=============
Fog_Update

update internal variables
=============
*/
void Fog_Update (float start, float end, float red, float green, float blue, float time)
{
	if (start <= 0.01f || end <= 0.01f) {
		start = 0.0f;
		end = 0.0f;
	}

	//save previous settings for fade
	if (time > 0)
	{
		//check for a fade in progress
		if (fade_done > cl.time)
		{
			float f;

			f = (fade_done - cl.time) / fade_time;
			old_start = f * old_start + (1.0 - f) * fog_start;
			old_end = f * old_end + (1.0 - f) * fog_end;
			old_red = f * old_red + (1.0 - f) * fog_red;
			old_green = f * old_green + (1.0 - f) * fog_green;
			old_blue = f * old_blue + (1.0 - f) * fog_blue;
			old_density = f * old_density + (1.0 - f) * fog_density_gl;
		}
		else
		{
			old_start = fog_start;
			old_end = fog_end;
			old_red = fog_red;
			old_green = fog_green;
			old_blue = fog_blue;
			old_density = fog_density_gl;
		}
	}

	fog_start = start;
	fog_end = end;
	fog_red = red;
	fog_green = green;
	fog_blue = blue;
	fade_time = time;
	fade_done = cl.time + time;

	if (fog_start )
	fog_density_gl = ((fog_start / fog_end))/3.5;
}

/*
=============
Fog_ParseServerMessage

handle an SVC_FOG message from server
=============
*/
void Fog_ParseServerMessage (void)
{
	float start, end, red, green, blue, time;

	start = MSG_ReadByte() / 255.0;
	end = MSG_ReadByte() / 255.0;
	red = MSG_ReadByte() / 255.0;
	green = MSG_ReadByte() / 255.0;
	blue = MSG_ReadByte() / 255.0;
	time = q_max(0.0, MSG_ReadShort() / 100.0);

	if (start < 0.01f || end < 0.01f) {
		start = 0.0f;
		end = 0.0f;
	}

	Fog_Update (start, end, red, green, blue, time);
}

/*
=============
Fog_FogCommand_f

handle the 'fog' console command
=============
*/
void Fog_FogCommand_f (void)
{
	switch (Cmd_Argc())
	{
	default:
	case 1:
		Con_Printf("usage:\n");
		Con_Printf("   fog <fade>\n");
		Con_Printf("   fog <start> <end>\n");
		Con_Printf("   fog <red> <green> <blue>\n");
		Con_Printf("   fog <fade> <red> <green> <blue>\n");
		Con_Printf("   fog <start> <end> <red> <green> <blue>\n");
		Con_Printf("   fog <start> <end> <red> <green> <blue> <fade>\n");
		Con_Printf("current values:\n");
		Con_Printf("   \"start\" is \"%f\"\n", fog_start);
		Con_Printf("   \"end\" is \"%f\"\n", fog_end);
		Con_Printf("   \"red\" is \"%f\"\n", fog_red);
		Con_Printf("   \"green\" is \"%f\"\n", fog_green);
		Con_Printf("   \"blue\" is \"%f\"\n", fog_blue);
		Con_Printf("   \"fade\" is \"%f\"\n", fade_time);
		break;
	case 2: //TEST
		Fog_Update(fog_start,
				   fog_end,
				   fog_red,
				   fog_green,
				   fog_blue,
				   q_max(0.0, atof(Cmd_Argv(1))));
		break;
	case 3:
		Fog_Update(q_max(0.0, atof(Cmd_Argv(1))),
				   q_max(0.0, atof(Cmd_Argv(2))),
				   fog_red,
				   fog_green,
				   fog_blue,
				   0.0);
		break;
	case 4:
		Fog_Update(fog_start,
				   fog_end,
				   CLAMP(0.0, atof(Cmd_Argv(1)), 255.0),
				   CLAMP(0.0, atof(Cmd_Argv(2)), 255.0),
				   CLAMP(0.0, atof(Cmd_Argv(3)), 255.0),
				   0.0);
		break;
	case 5: //TEST
		Fog_Update(fog_start,
				   fog_end,
				   CLAMP(0.0, atof(Cmd_Argv(1)), 255.0),
				   CLAMP(0.0, atof(Cmd_Argv(2)), 255.0),
				   CLAMP(0.0, atof(Cmd_Argv(3)), 255.0),
				   q_max(0.0, atof(Cmd_Argv(4))));
		break;
	case 6:
		Fog_Update(q_max(0.0, atof(Cmd_Argv(1))),
				   q_max(0.0, atof(Cmd_Argv(2))),
				   CLAMP(0.0, atof(Cmd_Argv(3)), 255.0),
				   CLAMP(0.0, atof(Cmd_Argv(4)), 255.0),
				   CLAMP(0.0, atof(Cmd_Argv(5)), 255.0),
				   0.0);
		break;
	case 7:
		Fog_Update(q_max(0.0, atof(Cmd_Argv(1))),
				   q_max(0.0, atof(Cmd_Argv(2))),
				   CLAMP(0.0, atof(Cmd_Argv(3)), 255.0),
				   CLAMP(0.0, atof(Cmd_Argv(4)), 255.0),
				   CLAMP(0.0, atof(Cmd_Argv(5)), 255.0),
				   q_max(0.0, atof(Cmd_Argv(6))));
		break;
	}
}

/*
=============
Fog_ParseWorldspawn

called at map load
=============
*/
void Fog_ParseWorldspawn (void)
{
	char key[128], value[4096];
	const char *data;

	fog_density_gl = DEFAULT_DENSITY;
	//initially no fog
	fog_start = 0;
	old_start = 0;

	fog_end = -1;
	old_end = -1;

	fog_red = 0.0;
	old_red = 0.0;

	fog_green = 0.0;
	old_green = 0.0;

	fog_blue = 0.0;
	old_blue = 0.0;

	fade_time = 0.0;
	fade_done = 0.0;

	data = COM_Parse(cl.worldmodel->entities);
	if (!data)
		return; // error
	if (com_token[0] != '{')
		return; // error
	while (1)
	{
		data = COM_Parse(data);
		if (!data)
			return; // error
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
			return; // error
		strcpy(value, com_token);

		if (!strcmp("fog", key))
		{
			sscanf(value, "%f %f %f %f %f", &fog_start, &fog_end, &fog_red, &fog_green, &fog_blue);
		}

		fog_density_gl = ((fog_start / fog_end))/3.5;
	}

	if (fog_start <= 0.01f || fog_end <= 0.01f) {
		fog_start = 0.0f;
		fog_end = 0.0f;
	}
}

/*
=============
Fog_GetColor

calculates fog color for this frame, taking into account fade times
=============
*/
float *Fog_GetColor (void)
{
	static float c[4]; // = {0.1f, 0.1f, 0.1f, 1.0f}

	float f;
	int i;

	if (fade_done > cl.time)
	{
		f = (fade_done - cl.time) / fade_time;
		c[0] = f * old_red + (1.0 - f) * fog_red;
		c[1] = f * old_green + (1.0 - f) * fog_green;
		c[2] = f * old_blue + (1.0 - f) * fog_blue;
		c[3] = 1.0;
	}
	else
	{
		c[0] = fog_red;
		c[1] = fog_green;
		c[2] = fog_blue;
		c[3] = 1.0;
	}

	//find closest 24-bit RGB value, so solid-colored sky can match the fog perfectly
	for (i=0;i<3;i++)
		c[i] = (float)(Q_rint(c[i] * 255)) / 255.0f;

	for (i = 0; i < 3; i++)
		c[i] /= 64.0;

	return c;
}

/*
=============
Fog_GetDensity

returns current density of fog

=============
*/
float Fog_GetDensity (void)
{
	float f;

	if (fade_done > cl.time)
	{
		f = (fade_done - cl.time) / fade_time;
		return f * old_density + (1.0 - f) * fog_density_gl;
	}
	else
		return fog_density_gl;
}

/*
=============
Fog_SetupFrame

called at the beginning of each frame
=============
*/
void Fog_SetupFrame (void)
{
	//glFogfv(GL_FOG_COLOR, Fog_GetColor());
	//glFogf(GL_FOG_DENSITY, Fog_GetDensity() / 64.0);
	
	/*float c[4];
	float f, s, e;

	if (fade_done > cl.time)
	{
		f = (fade_done - cl.time) / fade_time;
		s = f * old_start + (1.0 - f) * fog_start;
		e = f * old_end + (1.0 - f) * fog_end;
		c[0] = f * old_red + (1.0 - f) * fog_red;
		c[1] = f * old_green + (1.0 - f) * fog_green;
		c[2] = f * old_blue + (1.0 - f) * fog_blue;
		c[3] = 1.0;
        //c[3] = r_skyfog.value;
	}
	else
	{
		s = fog_start;
		e = fog_end;
		c[0] = fog_red;
		c[1] = fog_green;
		c[2] = fog_blue;
		c[3] = 1.0;
        //c[3] = r_skyfog.value;
	}

	if(e == 0)
		e = -1;*/

	glFogfv(GL_FOG_COLOR, Fog_GetColor());
#ifdef VITA
	glFogf(GL_FOG_DENSITY, fog_density_gl);
#else
	glFogf(GL_FOG_DENSITY, fog_density_gl / 64.0);
#endif // VITA
	glFogf(GL_FOG_START, fog_start);
	glFogf(GL_FOG_END, fog_end);
	//glFogf(GL_FOG_COLOR, *c);

	//if(s == 0 || e < 0)
		//glDisable(GL_FOG);
}

/*
=============
Fog_GetStart
returns current start of fog
=============
*/
float Fog_GetStart (void)
{
	float f;

	if (fade_done > cl.time)
	{
		f = (fade_done - cl.time) / fade_time;
		return f * old_start + (1.0 - f) * fog_start;
	}
	else
		return fog_start;
}

/*
=============
Fog_GetEnd
returns current end of fog
=============
*/
float Fog_GetEnd (void)
{
	float f;

	if (fade_done > cl.time)
	{
		f = (fade_done - cl.time) / fade_time;
		return f * old_start + (1.0 - f) * fog_end;
	}
	else
		return fog_end;
}

/*
=============
Fog_EnableGFog

called before drawing stuff that should be fogged
=============
*/
void Fog_EnableGFog (void)
{
	if (!(Fog_GetStart() == 0) || !(Fog_GetEnd() <= 0))
		glEnable(GL_FOG);
}

/*
=============
Fog_DisableGFog

called after drawing stuff that should be fogged
=============
*/
void Fog_DisableGFog (void)
{
	if (!(Fog_GetStart() == 0) || !(Fog_GetEnd() <= 0))
		glDisable(GL_FOG);
}

/*
=============
Fog_StartAdditive

called before drawing stuff that is additive blended -- sets fog color to black
=============
*/
void Fog_StartAdditive (void)
{
	vec3_t color = {0,0,0};

	if (Fog_GetDensity() > 0)
		glFogfv(GL_FOG_COLOR, color);
}

/*
=============
Fog_StopAdditive

called after drawing stuff that is additive blended -- restores fog color
=============
*/
void Fog_StopAdditive (void)
{
	if (Fog_GetDensity() > 0)
		glFogfv(GL_FOG_COLOR, Fog_GetColor());
}

//==============================================================================
//
//  VOLUMETRIC FOG
//
//==============================================================================

cvar_t r_vfog = {"r_vfog", "1", CVAR_NONE};

//void Fog_DrawVFog (void){}
//void Fog_MarkModels (void){}

//==============================================================================
//
//  INIT
//
//==============================================================================

/*
=============
Fog_NewMap

called whenever a map is loaded
=============
*/
void Fog_NewMap (void)
{
	Fog_ParseWorldspawn (); //for global fog
	//Fog_MarkModels (); //for volumetric fog
}

/*
=============
Fog_Init

called when quake initializes
=============
*/
void Fog_Init (void)
{
	Cmd_AddCommand ("fog",Fog_FogCommand_f);

	//Cvar_RegisterVariable (&r_vfog);

	//set up global fog
	/*
	fog_density_gl = DEFAULT_DENSITY;
	fog_red = DEFAULT_GRAY;
	fog_green = DEFAULT_GRAY;
	fog_blue = DEFAULT_GRAY;
	*/
	
	fog_start = 0;
	fog_end = -1;
	fog_red = 0.5;
	fog_green = 0.5;
	fog_blue = 0.5;
	fade_time = 1;
	fog_density_gl = DEFAULT_DENSITY;

	Fog_SetupState ();
}

/*
=============
Fog_SetupState
 
ericw -- moved from Fog_Init, state that needs to be setup when a new context is created
=============
*/
void Fog_SetupState (void)
{
	glFogi(GL_FOG_MODE, GL_EXP2);
}
