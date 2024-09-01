/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2007-2008 Kristian Duske
Copyright (C) 2010-2011 O. Sezer <sezero@users.sourceforge.net>
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

// snd_dma.c -- main control for any streaming sound output device

#include "quakedef.h"
#include "snd_codec.h"
#include "bgmusic.h"

static void S_Play (void);
static void S_PlayVol (void);
static void S_SoundList (void);
static void S_Update_ (void);
void S_StopAllSounds (qboolean clear);
static void S_StopAllSoundsC (void);

// =======================================================================
// Internal sound data & structures
// =======================================================================

channel_t	snd_channels[MAX_CHANNELS];
int		total_channels;

static int	snd_blocked = 0;
static qboolean	snd_initialized = false;

static dma_t	sn;
volatile dma_t	*shm = NULL;

vec3_t		listener_origin;
vec3_t		listener_forward;
vec3_t		listener_right;
vec3_t		listener_up;

#define	sound_nominal_clip_dist	1000.0

int		soundtime;	// sample PAIRS
int		paintedtime;	// sample PAIRS

int		s_rawend;
portable_samplepair_t	s_rawsamples[MAX_RAW_SAMPLES];


#define	MAX_SFX		1024
static sfx_t	*known_sfx = NULL;	// hunk allocated [MAX_SFX]
static int	num_sfx;

static sfx_t	*ambient_sfx[NUM_AMBIENTS];

static qboolean	sound_started = false;

cvar_t		bgmvolume = {"bgmvolume", "1", CVAR_ARCHIVE};
cvar_t		sfxvolume = {"volume", "0.7", CVAR_ARCHIVE};

cvar_t		precache = {"precache", "1", CVAR_NONE};
cvar_t		loadas8bit = {"loadas8bit", "0", CVAR_NONE};

cvar_t		sndspeed = {"sndspeed", "11025", CVAR_NONE};
cvar_t		snd_mixspeed = {"snd_mixspeed", "44100", CVAR_NONE};

#if defined(_WIN32)
#define SND_FILTERQUALITY_DEFAULT "5"
#else
#define SND_FILTERQUALITY_DEFAULT "1"
#endif

cvar_t		snd_filterquality = {"snd_filterquality", SND_FILTERQUALITY_DEFAULT,
								 CVAR_NONE};

static	cvar_t	nosound = {"nosound", "0", CVAR_NONE};
static	cvar_t	ambient_level = {"ambient_level", "0.3", CVAR_NONE};
static	cvar_t	ambient_fade = {"ambient_fade", "100", CVAR_NONE};
static	cvar_t	snd_noextraupdate = {"snd_noextraupdate", "0", CVAR_NONE};
static	cvar_t	snd_show = {"snd_show", "0", CVAR_NONE};
static	cvar_t	_snd_mixahead = {"_snd_mixahead", "0.1", CVAR_ARCHIVE};


static void S_SoundInfo_f (void)
{
	if (!sound_started || !shm)
	{
		Con_Printf ("sound system not started\n");
		return;
	}

	Con_Printf("%d bit, %s, %d Hz\n", shm->samplebits,
			(shm->channels == 2) ? "stereo" : "mono", shm->speed);
	Con_Printf("%5d samples\n", shm->samples);
	Con_Printf("%5d samplepos\n", shm->samplepos);
	Con_Printf("%5d submission_chunk\n", shm->submission_chunk);
	Con_Printf("%5d total_channels\n", total_channels);
	Con_Printf("%p dma buffer\n", shm->buffer);
}


static void SND_Callback_sfxvolume (cvar_t *var)
{
	SND_InitScaletable ();
}

static void SND_Callback_snd_filterquality (cvar_t *var)
{
	if (snd_filterquality.value < 1 || snd_filterquality.value > 5)
	{
		Con_Printf ("snd_filterquality must be between 1 and 5\n");
		Cvar_SetQuick (&snd_filterquality, SND_FILTERQUALITY_DEFAULT);
	}
}

/*
================
S_Startup
================
*/
void S_Startup (void)
{
	if (!snd_initialized)
		return;

	sound_started = SNDDMA_Init(&sn);

	if (!sound_started)
	{
		Con_Printf("Failed initializing sound\n");
	}
	else
	{
		Con_Printf("Audio: %d bit, %s, %d Hz\n",
				shm->samplebits,
				(shm->channels == 2) ? "stereo" : "mono",
				shm->speed);
	}
}


/*
================
S_Init
================
*/
void S_Init (void)
{
	int i;

	if (snd_initialized)
	{
		Con_Printf("Sound is already initialized\n");
		return;
	}

	Cvar_RegisterVariable(&nosound);
	Cvar_RegisterVariable(&sfxvolume);
	Cvar_RegisterVariable(&precache);
	Cvar_RegisterVariable(&loadas8bit);
	Cvar_RegisterVariable(&bgmvolume);
	Cvar_RegisterVariable(&ambient_level);
	Cvar_RegisterVariable(&ambient_fade);
	Cvar_RegisterVariable(&snd_noextraupdate);
	Cvar_RegisterVariable(&snd_show);
	Cvar_RegisterVariable(&_snd_mixahead);
	Cvar_RegisterVariable(&sndspeed);
	Cvar_RegisterVariable(&snd_mixspeed);
	Cvar_RegisterVariable(&snd_filterquality);
	
	if (safemode || COM_CheckParm("-nosound"))
		return;

	Con_Printf("\nSound Initialization\n");

	Cmd_AddCommand("play", S_Play);
	Cmd_AddCommand("playvol", S_PlayVol);
	Cmd_AddCommand("stopsound", S_StopAllSoundsC);
	Cmd_AddCommand("soundlist", S_SoundList);
	Cmd_AddCommand("soundinfo", S_SoundInfo_f);

	i = COM_CheckParm("-sndspeed");
	if (i && i < com_argc-1)
	{
		Cvar_SetQuick (&sndspeed, com_argv[i+1]);
	}
	
	i = COM_CheckParm("-mixspeed");
	if (i && i < com_argc-1)
	{
		Cvar_SetQuick (&snd_mixspeed, com_argv[i+1]);
	}

	if (host_parms->memsize < 0x800000)
	{
		Cvar_SetQuick (&loadas8bit, "1");
		Con_Printf ("loading all sounds as 8bit\n");
	}

	Cvar_SetCallback(&sfxvolume, SND_Callback_sfxvolume);
	Cvar_SetCallback(&snd_filterquality, &SND_Callback_snd_filterquality);

	SND_InitScaletable ();

	known_sfx = (sfx_t *) Hunk_AllocName (MAX_SFX*sizeof(sfx_t), "sfx_t");
	num_sfx = 0;

	snd_initialized = true;

	S_Startup ();
	if (sound_started == 0)
		return;

// provides a tick sound until washed clean
//	if (shm->buffer)
//		shm->buffer[4] = shm->buffer[5] = 0x7f;	// force a pop for debugging

	S_CodecInit ();

	S_StopAllSounds (true);
}


// =======================================================================
// Shutdown sound engine
// =======================================================================
void S_Shutdown (void)
{
	if (!sound_started)
		return;

	sound_started = 0;
	snd_blocked = 0;

	S_CodecShutdown();

	SNDDMA_Shutdown();
	shm = NULL;
}


// =======================================================================
// Load a sound
// =======================================================================

/*
==================
S_FindName

==================
*/
static sfx_t *S_FindName (const char *name)
{
	int		i;
	sfx_t	*sfx;

	if (!name)
		Sys_Error ("S_FindName: NULL");

	if (Q_strlen(name) >= MAX_QPATH)
		Sys_Error ("Sound name too long: %s", name);

// see if already loaded
	for (i = 0; i < num_sfx; i++)
	{
		if (!strcmp(known_sfx[i].name, name))
		{
			return &known_sfx[i];
		}
	}

	if (num_sfx == MAX_SFX)
		Sys_Error ("S_FindName: out of sfx_t");

	sfx = &known_sfx[i];
	q_strlcpy (sfx->name, name, sizeof(sfx->name));

	num_sfx++;

	return sfx;
}


/*
==================
S_TouchSound

==================
*/
void S_TouchSound (const char *name)
{
	sfx_t	*sfx;

	if (!sound_started)
		return;

	sfx = S_FindName (name);
	Cache_Check (&sfx->cache);
}

/*
==================
S_PrecacheSound

==================
*/
sfx_t *S_PrecacheSound (const char *name)
{
	sfx_t	*sfx;

	if (!sound_started || nosound.value)
		return NULL;

	sfx = S_FindName (name);

// cache it in
	if (precache.value)
		S_LoadSound (sfx);

	return sfx;
}


//=============================================================================

/*
=================
SND_PickChannel

picks a channel based on priorities, empty slots, number of channels
=================
*/
channel_t *SND_PickChannel (int entnum, int entchannel)
{
	int	ch_idx;
	int	first_to_die;
	int	life_left;

// Check for replacement sound, or find the best one to replace
	first_to_die = -1;
	life_left = 0x7fffffff;
	for (ch_idx = NUM_AMBIENTS; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS; ch_idx++)
	{
		if (entchannel != 0		// channel 0 never overrides
			&& snd_channels[ch_idx].entnum == entnum
			&& (snd_channels[ch_idx].entchannel == entchannel || entchannel == -1) )
		{	// always override sound from same entity
			first_to_die = ch_idx;
			break;
		}

		// don't let monster sounds override player sounds
		if (snd_channels[ch_idx].entnum == cl.viewentity && entnum != cl.viewentity && snd_channels[ch_idx].sfx)
			continue;

		if (snd_channels[ch_idx].end - paintedtime < life_left)
		{
			life_left = snd_channels[ch_idx].end - paintedtime;
			first_to_die = ch_idx;
		}
	}

	if (first_to_die == -1)
		return NULL;

	if (snd_channels[first_to_die].sfx)
		snd_channels[first_to_die].sfx = NULL;

	return &snd_channels[first_to_die];
}

/*
=================
SND_Spatialize

spatializes a channel
=================
*/

//
// cypress -- sound dot product square root LUTs
//
float SND_InverseSpatializationRScaleLUT[126] = { 
	0.0f, 0.12247448713915896f, 0.1732050807568878f, 0.21213203435596434f, 
	0.2449489742783179f, 0.2738612787525832f, 0.30000000000000016f, 0.32403703492039315f, 
	0.3464101615137756f, 0.3674234614174769f, 0.38729833462074187f, 0.4062019202317982f, 
	0.4242640687119287f, 0.44158804331639256f, 0.4582575694955842f, 0.4743416490252571f, 
	0.4898979485566358f, 0.5049752469181041f, 0.5196152422706635f, 0.5338539126015658f, 
	0.5477225575051664f, 0.5612486080160914f, 0.5744562646538032f, 0.5873670062235368f, 
	0.6000000000000003f, 0.6123724356957948f, 0.62449979983984f, 0.6363961030678931f, 
	0.6480740698407863f, 0.6595452979136462f, 0.6708203932499373f, 0.6819090848492931f, 
	0.6928203230275513f, 0.7035623639735148f, 0.7141428428542853f, 0.7245688373094723f, 
	0.7348469228349538f, 0.7449832212875673f, 0.7549834435270754f, 0.764852927038918f, 
	0.7745966692414837f, 0.7842193570679065f, 0.7937253933193775f, 0.8031189202104508f, 
	0.8124038404635964f, 0.8215838362577496f, 0.8306623862918079f, 0.8396427811873336f, 
	0.8485281374238574f, 0.8573214099741128f, 0.866025403784439f, 0.8746427842267954f, 
	0.8831760866327851f, 0.8916277250063508f, 0.9000000000000004f, 0.9082951062292479f, 
	0.9165151389911684f, 0.9246621004453469f, 0.9327379053088819f, 0.9407443861113394f, 
	0.9486832980505142f, 0.9565563234854499f, 0.9643650760992959f, 0.9721111047611795f, 
	0.9797958971132716f, 0.9874208829065754f, 0.9949874371066203f, 1.0024968827881715f, 
	1.009950493836208f, 1.0173494974687907f, 1.0246950765959602f, 1.0319883720275151f, 
	1.0392304845413267f, 1.0464224768228179f, 1.0535653752852743f, 1.0606601717798216f, 
	1.0677078252031316f, 1.0747092630102342f, 1.081665382639197f, 1.0885770528538625f, 
	1.0954451150103326f, 1.1022703842524304f, 1.109053650640942f, 1.1157956802210702f, 
	1.1224972160321829f, 1.1291589790636218f, 1.135781669160055f, 1.1423659658795866f, 
	1.1489125293076061f, 1.1554220008291347f, 1.1618950038622256f, 1.1683321445547927f, 
	1.1747340124470735f, 1.181101181101772f, 1.1874342087037921f, 1.1937336386313326f, 
	1.2000000000000004f, 1.2062338081814823f, 1.2124355652982146f, 1.2186057606953946f, 
	1.2247448713915896f, 1.2308533625091176f, 1.2369316876852987f, 1.242980289465606f, 
	1.24899959967968f, 1.2549900398011138f, 1.2609520212918497f, 1.2668859459319932f, 
	1.272792206135786f, 1.2786711852544426f, 1.2845232578665133f, 1.2903487900563946f, 
	1.2961481396815726f, 1.301921656629154f, 1.3076696830622025f, 1.3133925536563702f, 
	1.3190905958272925f, 1.3247641299491775f, 1.3304134695650076f, 1.3360389215887394f, 
	1.3416407864998743f, 1.3472193585307484f, 1.3527749258468689f, 1.358307770720613f, 
	1.363818169698586f, 1.3693063937629157f, 1.3747727084867525f, 1.3802173741842267f, 
	1.3856406460551023f, 1.391042774324356f, 1.3964240043768947f, 1.4017845768876191f, 
	1.4071247279470294f, 1.412444689182554f
};

float SND_InverseSpatializationLScaleLUT[126] = { 
	1.4142135623730951f, 1.408900280360537f, 1.40356688476182f, 1.3982131454109563f, 
	1.3928388277184118f, 1.3874436925511606f, 1.3820274961085253f, 1.3765899897936205f, 
	1.3711309200802089f, 1.3656500283747661f, 1.3601470508735443f, 1.3546217184144067f, 
	1.349073756323204f, 1.34350288425444f, 1.3379088160259651f, 1.3322912594474228f, 
	1.3266499161421599f, 1.3209844813622904f, 1.3152946437965904f, 1.3095800853708794f, 
	1.3038404810405297f, 1.2980754985747167f, 1.2922847983320085f, 1.2864680330268607f, 
	1.2806248474865696f, 1.2747548783981961f, 1.268857754044952f, 1.2629330940315087f, 
	1.2569805089976533f, 1.2509996003196802f, 1.2449899597988732f, 1.2389511693363866f, 
	1.232882800593795f, 1.2267844146385294f, 1.22065556157337f, 1.2144957801491119f, 
	1.208304597359457f, 1.2020815280171306f, 1.1958260743101397f, 1.1895377253370318f, 
	1.183215956619923f, 1.1768602295939816f, 1.1704699910719623f, 1.1640446726822813f, 
	1.1575836902790222f, 1.1510864433221335f, 1.1445523142259595f, 1.137980667674104f, 
	1.1313708498984758f, 1.124722187920199f, 1.1180339887498945f, 1.111305538544643f, 
	1.1045361017187258f, 1.097724920005007f, 1.090871211463571f, 1.0839741694339398f, 
	1.0770329614269005f, 1.0700467279516344f, 1.0630145812734646f, 1.0559356040971435f, 
	1.0488088481701512f, 1.0416333327999825f, 1.0344080432788596f, 1.0271319292087064f, 
	1.0198039027185566f, 1.012422836565829f, 1.0049875621120887f, 0.9974968671629998f, 
	0.9899494936611661f, 0.9823441352194247f, 0.974679434480896f, 0.9669539802906855f, 
	0.9591663046625435f, 0.951314879522022f, 0.94339811320566f, 0.9354143466934849f, 
	0.9273618495495699f, 0.9192388155425113f, 0.9110433579144295f, 0.902773504263389f, 
	0.8944271909999154f, 0.886002257333467f, 0.8774964387392117f, 0.8689073598491378f, 
	0.8602325267042622f, 0.8514693182963196f, 0.8426149773176353f, 0.8336666000266528f, 
	0.8246211251235316f, 0.8154753215150039f, 0.8062257748298544f, 0.7968688725254608f, 
	0.7874007874011805f, 0.7778174593052016f, 0.7681145747868602f, 0.7582875444051543f, 
	0.7483314773547876f, 0.7382411530116693f, 0.728010988928051f, 0.7176350047203655f, 
	0.7071067811865467f, 0.6964194138592051f, 0.6855654600401035f, 0.6745368781616012f, 
	0.663324958071079f, 0.651920240520264f, 0.6403124237432839f, 0.6284902544988258f, 
	0.6164414002968966f, 0.6041522986797276f, 0.5916079783099606f, 0.5787918451395102f, 
	0.5656854249492369f, 0.5522680508593619f, 0.5385164807134492f, 0.5244044240850745f, 
	0.5099019513592772f, 0.4949747468305819f, 0.4795831523312705f, 0.46368092477478373f, 
	0.4472135954999564f, 0.4301162633521297f, 0.41231056256176435f, 0.39370039370058874f, 
	0.3741657386773922f, 0.35355339059327173f, 0.3316624790355378f, 0.30822070014844644f, 
	0.2828427124746164f, 0.2549509756796363f, 0.2236067977499756f, 0.187082869338693f, 
	0.14142135623730406f, 0.07071067811864379f
};

void SND_Spatialize (channel_t *ch)
{
	vec_t	dot;
	vec_t	dist;
	vec_t	lscale, rscale, scale;
	vec3_t	source_vec;

	// anything coming from the view entity will always be full volume
	// cypress -- added full volume for no attenuation.
	if (ch->entnum == cl.viewentity || ch->dist_mult == 0)
	{
		ch->leftvol = ch->master_vol;
		ch->rightvol = ch->master_vol;
		return;
	}

// calculate stereo seperation and distance attenuation
	VectorSubtract(ch->origin, listener_origin, source_vec);
	dist = VectorNormalize(source_vec) * ch->dist_mult;
	dot = DotProduct(listener_right, source_vec);

	int dot_index = (dot + 1) * 63;
	rscale = SND_InverseSpatializationRScaleLUT[dot_index];
	lscale = SND_InverseSpatializationLScaleLUT[dot_index];

// add in distance effect
	scale = (1.0 - dist) * rscale;
	ch->rightvol = (int) (ch->master_vol * scale);
	if (ch->rightvol < 0)
		ch->rightvol = 0;

	scale = (1.0 - dist) * lscale;
	ch->leftvol = (int) (ch->master_vol * scale);
	if (ch->leftvol < 0)
		ch->leftvol = 0;
}


// =======================================================================
// Start a sound effect
// =======================================================================

void S_StartSound (int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation)
{
	channel_t	*target_chan, *check;
	sfxcache_t	*sc;
	int		ch_idx;
	int		skip;

	if (!sound_started)
		return;

	if (!sfx)
		return;

	if (nosound.value)
		return;

// pick a channel to play on
	target_chan = SND_PickChannel(entnum, entchannel);
	if (!target_chan)
		return;

// spatialize
	memset (target_chan, 0, sizeof(*target_chan));
	VectorCopy(origin, target_chan->origin);
	target_chan->dist_mult = attenuation / sound_nominal_clip_dist;
	target_chan->master_vol = (int) (fvol * 255);
	target_chan->entnum = entnum;
	target_chan->entchannel = entchannel;
	SND_Spatialize(target_chan);

	if (!target_chan->leftvol && !target_chan->rightvol)
		return;		// not audible at all

// new channel
	sc = S_LoadSound (sfx);
	if (!sc)
	{
		target_chan->sfx = NULL;
		return;		// couldn't load the sound's data
	}

	target_chan->sfx = sfx;
	target_chan->pos = 0.0;
	target_chan->end = paintedtime + sc->length;

// if an identical sound has also been started this frame, offset the pos
// a bit to keep it from just making the first one louder
	check = &snd_channels[NUM_AMBIENTS];
	for (ch_idx = NUM_AMBIENTS; ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS; ch_idx++, check++)
	{
		if (check == target_chan)
			continue;
		if (check->sfx == sfx && !check->pos)
		{
			/*
			skip = rand () % (int)(0.1 * shm->speed);
			if (skip >= target_chan->end)
				skip = target_chan->end - 1;
			*/
			/* LordHavoc: fixed skip calculations */
			skip = 0.1 * shm->speed; /* 0.1 * sc->speed */
			if (skip > sc->length)
				skip = sc->length;
			if (skip > 0)
				skip = rand() % skip;
			target_chan->pos += skip;
			target_chan->end -= skip;
			break;
		}
	}
}

void S_StopSound (int entnum, int entchannel)
{
	int	i;

	for (i = 0; i < MAX_DYNAMIC_CHANNELS; i++)
	{
		if (snd_channels[i].entnum == entnum
			&& snd_channels[i].entchannel == entchannel)
		{
			snd_channels[i].end = 0;
			snd_channels[i].sfx = NULL;
			return;
		}
	}
}

void S_StopAllSounds (qboolean clear)
{
	int		i;

	if (!sound_started)
		return;

	total_channels = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;	// no statics

	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (snd_channels[i].sfx)
			snd_channels[i].sfx = NULL;
	}

	memset(snd_channels, 0, MAX_CHANNELS * sizeof(channel_t));

	if (clear)
		S_ClearBuffer ();
}

static void S_StopAllSoundsC (void)
{
	S_StopAllSounds (true);
}

void S_ClearBuffer (void)
{
	int		clear;

	if (!sound_started || !shm)
		return;

	SNDDMA_LockBuffer ();
	if (! shm->buffer)
		return;

	s_rawend = 0;

	if (shm->samplebits == 8 && !shm->signed8)
		clear = 0x80;
	else
		clear = 0;

	memset(shm->buffer, clear, shm->samples * shm->samplebits / 8);

	SNDDMA_Submit ();
}


/*
=================
S_StaticSound
=================
*/
void S_StaticSound (sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
	channel_t	*ss;
	sfxcache_t		*sc;

	if (!sfx)
		return;

	if (total_channels == MAX_CHANNELS)
	{
		Con_Printf ("total_channels == MAX_CHANNELS\n");
		return;
	}

	ss = &snd_channels[total_channels];
	total_channels++;

	sc = S_LoadSound (sfx);
	if (!sc)
		return;

	if (sc->loopstart == -1)
	{
		Con_Printf ("Sound %s not looped\n", sfx->name);
		return;
	}

	ss->sfx = sfx;
	VectorCopy (origin, ss->origin);
	ss->master_vol = (int)vol;
	ss->dist_mult = (attenuation / 64) / sound_nominal_clip_dist;
	ss->end = paintedtime + sc->length;

	SND_Spatialize (ss);
}


//=============================================================================

/*
===================
S_UpdateAmbientSounds
===================
*/
static void S_UpdateAmbientSounds (void)
{
	mleaf_t		*l;
	int		vol, ambient_channel;
	channel_t	*chan;

// no ambients when disconnected
	if (cls.state != ca_connected)
		return;
// calc ambient sound levels
	if (!cl.worldmodel)
		return;

	l = Mod_PointInLeaf (listener_origin, cl.worldmodel);
	if (!l || !ambient_level.value)
	{
		for (ambient_channel = 0; ambient_channel < NUM_AMBIENTS; ambient_channel++)
			snd_channels[ambient_channel].sfx = NULL;
		return;
	}

	for (ambient_channel = 0; ambient_channel < NUM_AMBIENTS; ambient_channel++)
	{
		chan = &snd_channels[ambient_channel];
		chan->sfx = ambient_sfx[ambient_channel];

		vol = (int) (ambient_level.value * l->ambient_sound_level[ambient_channel]);
		if (vol < 8)
			vol = 0;

	// don't adjust volume too fast
		if (chan->master_vol < vol)
		{
			chan->master_vol += (int) (host_frametime * ambient_fade.value);
			if (chan->master_vol > vol)
				chan->master_vol = vol;
		}
		else if (chan->master_vol > vol)
		{
			chan->master_vol -= (int) (host_frametime * ambient_fade.value);
			if (chan->master_vol < vol)
				chan->master_vol = vol;
		}

		chan->leftvol = chan->rightvol = chan->master_vol;
	}
}


/*
===================
S_RawSamples		(from QuakeII)

Streaming music support. Byte swapping
of data must be handled by the codec.
Expects data in signed 16 bit, or unsigned
8 bit format.
===================
*/
void S_RawSamples (int samples, int rate, int width, int channels, byte *data, float volume)
{
	int i;
	int src, dst;
	float scale;
	int intVolume;

	if (s_rawend < paintedtime)
		s_rawend = paintedtime;

	scale = (float) rate / shm->speed;
	intVolume = (int) (256 * volume);

	if (channels == 2 && width == 2)
	{
		for (i = 0; ; i++)
		{
			src = i * scale;
			if (src >= samples)
				break;
			dst = s_rawend & (MAX_RAW_SAMPLES - 1);
			s_rawend++;
			s_rawsamples [dst].left = ((short *) data)[src * 2] * intVolume;
			s_rawsamples [dst].right = ((short *) data)[src * 2 + 1] * intVolume;
		}
	}
	else if (channels == 1 && width == 2)
	{
		for (i = 0; ; i++)
		{
			src = i * scale;
			if (src >= samples)
				break;
			dst = s_rawend & (MAX_RAW_SAMPLES - 1);
			s_rawend++;
			s_rawsamples [dst].left = ((short *) data)[src] * intVolume;
			s_rawsamples [dst].right = ((short *) data)[src] * intVolume;
		}
	}
	else if (channels == 2 && width == 1)
	{
		intVolume *= 256;

		for (i = 0; ; i++)
		{
			src = i * scale;
			if (src >= samples)
				break;
			dst = s_rawend & (MAX_RAW_SAMPLES - 1);
			s_rawend++;
		//	s_rawsamples [dst].left = ((signed char *) data)[src * 2] * intVolume;
		//	s_rawsamples [dst].right = ((signed char *) data)[src * 2 + 1] * intVolume;
			s_rawsamples [dst].left = (((byte *) data)[src * 2] - 128) * intVolume;
			s_rawsamples [dst].right = (((byte *) data)[src * 2 + 1] - 128) * intVolume;
		}
	}
	else if (channels == 1 && width == 1)
	{
		intVolume *= 256;

		for (i = 0; ; i++)
		{
			src = i * scale;
			if (src >= samples)
				break;
			dst = s_rawend & (MAX_RAW_SAMPLES - 1);
			s_rawend++;
		//	s_rawsamples [dst].left = ((signed char *) data)[src] * intVolume;
		//	s_rawsamples [dst].right = ((signed char *) data)[src] * intVolume;
			s_rawsamples [dst].left = (((byte *) data)[src] - 128) * intVolume;
			s_rawsamples [dst].right = (((byte *) data)[src] - 128) * intVolume;
		}
	}
}

/*
============
S_Update

Called once each time through the main loop
============
*/
void S_Update (vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
{
	int			i, j;
	int			total;
	channel_t	*ch;
	channel_t	*combine;

	if (!sound_started || (snd_blocked > 0))
		return;

	VectorCopy(origin, listener_origin);
	VectorCopy(forward, listener_forward);
	VectorCopy(right, listener_right);
	VectorCopy(up, listener_up);

// update general area ambient sound sources
	S_UpdateAmbientSounds ();

	combine = NULL;

// update spatialization for static and dynamic sounds
	ch = snd_channels + NUM_AMBIENTS;
	for (i = NUM_AMBIENTS; i < total_channels; i++, ch++)
	{
		if (!ch->sfx)
			continue;
		SND_Spatialize(ch);	// respatialize channel
		if (!ch->leftvol && !ch->rightvol)
			continue;

	// try to combine static sounds with a previous channel of the same
	// sound effect so we don't mix five torches every frame

		if (i >= MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS)
		{
		// see if it can just use the last one
			if (combine && combine->sfx == ch->sfx)
			{
				combine->leftvol += ch->leftvol;
				combine->rightvol += ch->rightvol;
				ch->leftvol = ch->rightvol = 0;
				continue;
			}
		// search for one
			combine = snd_channels + MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;
			for (j = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS; j < i; j++, combine++)
			{
				if (combine->sfx == ch->sfx)
					break;
			}

			if (j == total_channels)
			{
				combine = NULL;
			}
			else
			{
				if (combine != ch)
				{
					combine->leftvol += ch->leftvol;
					combine->rightvol += ch->rightvol;
					ch->leftvol = ch->rightvol = 0;
				}
				continue;
			}
		}
	}

//
// debugging output
//
	if (snd_show.value)
	{
		total = 0;
		ch = snd_channels;
		for (i = 0; i < total_channels; i++, ch++)
		{
			if (ch->sfx && (ch->leftvol || ch->rightvol) )
			{
			//	Con_Printf ("%3i %3i %s\n", ch->leftvol, ch->rightvol, ch->sfx->name);
				total++;
			}
		}

		Con_Printf ("----(%i)----\n", total);
	}

// add raw data from streamed samples
//	BGM_Update();	// moved to the main loop just before S_Update ()

// mix some sound
	S_Update_();
}

static void GetSoundtime (void)
{
	int		samplepos;
	static	int		buffers;
	static	int		oldsamplepos;
	int		fullsamples;

	fullsamples = shm->samples / shm->channels;

// it is possible to miscount buffers if it has wrapped twice between
// calls to S_Update.  Oh well.
	samplepos = SNDDMA_GetDMAPos();

	if (samplepos < oldsamplepos)
	{
		buffers++;	// buffer wrapped

		if (paintedtime > 0x40000000)
		{	// time to chop things off to avoid 32 bit limits
			buffers = 0;
			paintedtime = fullsamples;
			S_StopAllSounds (true);
		}
	}
	oldsamplepos = samplepos;

	soundtime = buffers*fullsamples + samplepos/shm->channels;
}

void S_ExtraUpdate (void)
{
	if (snd_noextraupdate.value)
		return;		// don't pollute timings
	S_Update_();
}

static void S_Update_ (void)
{
	unsigned int	endtime;
	int		samps;

	if (!sound_started || (snd_blocked > 0))
		return;

	SNDDMA_LockBuffer ();
	if (! shm->buffer)
		return;

// Updates DMA time
	GetSoundtime();

// check to make sure that we haven't overshot
	if (paintedtime < soundtime)
	{
	//	Con_Printf ("S_Update_ : overflow\n");
		paintedtime = soundtime;
	}

// mix ahead of current position
	endtime = soundtime + (unsigned int)(_snd_mixahead.value * shm->speed);
	samps = shm->samples >> (shm->channels - 1);
	endtime = q_min(endtime, (unsigned int)(soundtime + samps));

	S_PaintChannels (endtime);

	SNDDMA_Submit ();
}

void S_BlockSound (void)
{
/* FIXME: do we really need the blocking at the
 * driver level?
 */
	if (sound_started && snd_blocked == 0)	/* ++snd_blocked == 1 */
	{
		snd_blocked  = 1;
		S_ClearBuffer ();
		if (shm)
			SNDDMA_BlockSound();
	}
}

void S_UnblockSound (void)
{
	if (!sound_started || !snd_blocked)
		return;
	if (snd_blocked == 1)			/* --snd_blocked == 0 */
	{
		snd_blocked  = 0;
		SNDDMA_UnblockSound();
		S_ClearBuffer ();
	}
}

/*
===============================================================================

console functions

===============================================================================
*/

static void S_Play (void)
{
	static int hash = 345;
	int		i;
	char	name[256];
	sfx_t	*sfx;

	i = 1;
	while (i < Cmd_Argc())
	{
		q_strlcpy(name, Cmd_Argv(i), sizeof(name));
		if (!Q_strrchr(Cmd_Argv(i), '.'))
		{
			q_strlcat(name, ".wav", sizeof(name));
		}
		sfx = S_PrecacheSound(name);
		S_StartSound(hash++, 0, sfx, listener_origin, 1.0, 1.0);
		i++;
	}
}

static void S_PlayVol (void)
{
	static int hash = 543;
	int		i;
	float	vol;
	char	name[256];
	sfx_t	*sfx;

	i = 1;
	while (i < Cmd_Argc())
	{
		q_strlcpy(name, Cmd_Argv(i), sizeof(name));
		if (!Q_strrchr(Cmd_Argv(i), '.'))
		{
			q_strlcat(name, ".wav", sizeof(name));
		}
		sfx = S_PrecacheSound(name);
		vol = Q_atof(Cmd_Argv(i + 1));
		S_StartSound(hash++, 0, sfx, listener_origin, vol, 1.0);
		i += 2;
	}
}

static void S_SoundList (void)
{
	int		i;
	sfx_t	*sfx;
	sfxcache_t	*sc;
	int		size, total;

	total = 0;
	for (sfx = known_sfx, i = 0; i < num_sfx; i++, sfx++)
	{
		sc = (sfxcache_t *) Cache_Check (&sfx->cache);
		if (!sc)
			continue;
		size = sc->length*sc->width*(sc->stereo + 1);
		total += size;
		if (sc->loopstart >= 0)
			Con_SafePrintf ("L"); //johnfitz -- was Con_Printf
		else
			Con_SafePrintf (" "); //johnfitz -- was Con_Printf
		Con_SafePrintf("(%2db) %6i : %s\n", sc->width*8, size, sfx->name); //johnfitz -- was Con_Printf
	}
	Con_Printf ("%i sounds, %i bytes\n", num_sfx, total); //johnfitz -- added count
}


void S_LocalSound (const char *name)
{
	sfx_t	*sfx;

	if (nosound.value)
		return;
	if (!sound_started)
		return;

	sfx = S_PrecacheSound (name);
	if (!sfx)
	{
		Con_Printf ("S_LocalSound: can't cache %s\n", name);
		return;
	}
	S_StartSound (cl.viewentity, -1, sfx, vec3_origin, 1, 1);
}


void S_ClearPrecache (void)
{
}


void S_BeginPrecaching (void)
{
}


void S_EndPrecaching (void)
{
}

