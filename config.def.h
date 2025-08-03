#include <X11/XF86keysym.h>

/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "JetBrainsMono Nerd Font:size=10", "monospace:size=10" };
static const char dmenufont[]       = "JetBrainsMono Nerd Font:size=10";

//===============================================================================
// COLOR SCHEME - Modern Dark Theme with High Contrast
//===============================================================================

// Tokyo Night theme scheme colours
static const char background[]      = "#1a1b26";  // Dark purple-gray background
static const char text_primary[]    = "#15161e";  // Light blue-white text
static const char text_secondary[]  = "#c0caf5";  // Muted text
static const char accent_blue[]     = "#7aa2f7";  // Bright blue accent
static const char accent_purple[]   = "#bb9af7";  // Purple accent
static const char accent_red[]      = "#f38ba8";  // Red accent
static const char border_inactive[] = "#6c7086";  // Muted border
static const char urgent[]          = "#f7768e";  // Red for urgent/warnings

// Color assignments for dwm elements
static const char *colors[][3]      = {
	/*                    fg              bg           border        */
	[SchemeNorm]     = { text_secondary,  background,  border_inactive }, // Unfocused windows
	[SchemeSel]      = { text_primary,    accent_red, accent_red}, // Focused window
};

//===============================================================================
// TAGS AND RULES
//===============================================================================

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class         instance    title       tags mask     isfloating   monitor */
	{ "Gimp",        NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",     NULL,       NULL,       1 << 8,       0,           -1 },
	{ "Brave",       NULL,       NULL,       1 << 1,       0,           -1 },
	{ "ghostty",     NULL,       NULL,       0,            0,           -1 },
};

//===============================================================================
// LAYOUTS
//===============================================================================

/* layout(s) */
static const float mfact     = 0.5; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

//===============================================================================
// KEY DEFINITIONS AND COMMANDS
//===============================================================================

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */

// dmenu with matching colors
static const char *dmenucmd[] = { 
    "dmenu_run", 
    "-m", dmenumon, 
    "-fn", dmenufont, 
    "-nb", background,      // Normal background
    "-nf", text_secondary,  // Normal foreground
    "-sb", accent_red,     // Selected background
    "-sf", background,      // Selected foreground
    "-p", "Run:",          // Prompt text
    NULL 
};

static const char *termcmd[]     = { "ghostty", NULL };
static const char *browsercmd[]  = { "brave", NULL };

// Volume control commands with notifications (PipeWire)
static const char *volumeup[] = { "/bin/sh", "-c", 
    "wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%+ && "
    "notify-send -t 1500 -h string:x-canonical-private-synchronous:volume "
    "'Volume' \"$(wpctl get-volume @DEFAULT_AUDIO_SINK@ | awk '{printf \"%.0f%%\", $2*100}')\"", 
    NULL };

static const char *volumedown[] = { "/bin/sh", "-c", 
    "wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%- && "
    "notify-send -t 1500 -h string:x-canonical-private-synchronous:volume "
    "'Volume' \"$(wpctl get-volume @DEFAULT_AUDIO_SINK@ | awk '{printf \"%.0f%%\", $2*100}')\"", 
    NULL };

static const char *volumemute[] = { "/bin/sh", "-c", 
    "wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle && "
    "if wpctl get-volume @DEFAULT_AUDIO_SINK@ | grep -q 'MUTED'; then "
    "notify-send -t 1500 -h string:x-canonical-private-synchronous:volume 'Volume' 'Muted 🔇'; "
    "else notify-send -t 1500 -h string:x-canonical-private-synchronous:volume 'Volume' "
    "\"$(wpctl get-volume @DEFAULT_AUDIO_SINK@ | awk '{printf \"%.0f%%\", $2*100}') 🔊\"; fi", 
    NULL };

// Brightness control commands with notifications
static const char *brightnessup[] = { "/bin/sh", "-c", 
    "brightnessctl set 5%+ && "
    "notify-send -t 1500 -h string:x-canonical-private-synchronous:brightness "
    "'Brightness' \"$(brightnessctl get)%\"", 
    NULL };

static const char *brightnessdown[] = { "/bin/sh", "-c", 
    "brightnessctl set 5%- && "
    "notify-send -t 1500 -h string:x-canonical-private-synchronous:brightness "
    "'Brightness' \"$(brightnessctl get)%\"", 
    NULL };

// Microphone control commands with notifications (PipeWire)
static const char *micmute[] = { "/bin/sh", "-c", 
    "wpctl set-mute @DEFAULT_AUDIO_SOURCE@ toggle && "
    "if wpctl get-volume @DEFAULT_AUDIO_SOURCE@ | grep -q 'MUTED'; then "
    "notify-send -t 1500 -h string:x-canonical-private-synchronous:microphone 'Microphone' 'Muted 🎤❌'; "
    "else notify-send -t 1500 -h string:x-canonical-private-synchronous:microphone 'Microphone' 'Unmuted 🎤✅'; fi", 
    NULL };
//===============================================================================
// KEYBINDINGS
//===============================================================================

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_space,  spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_w,      spawn,          {.v = browsercmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_p,      setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },

	/* Volume control */
	{ 0,                            XF86XK_AudioRaiseVolume, spawn, {.v = volumeup } },
	{ 0,                            XF86XK_AudioLowerVolume, spawn, {.v = volumedown } },
	{ 0,                            XF86XK_AudioMute,        spawn, {.v = volumemute } },
	
	/* Brightness control */
	{ 0,                            XF86XK_MonBrightnessUp,   spawn, {.v = brightnessup } },
	{ 0,                            XF86XK_MonBrightnessDown, spawn, {.v = brightnessdown } },
  	/* Microphone control */
	{ 0,                            XF86XK_AudioMicMute,      spawn, {.v = micmute } },

	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

//===============================================================================
// MOUSE BINDINGS
//===============================================================================

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
