#include "stdafx.h"

aeInput Input;
typedef struct {
	char   *name;
	int     keynum;
} keyname_t;

keyname_t keynames[] = {
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"SHIFT", K_SHIFT},

	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
// fuh ->
	{"MOUSE4", K_MOUSE4},
	{"MOUSE5", K_MOUSE5},
	{"MOUSE6", K_MOUSE6},
	{"MOUSE7", K_MOUSE7},
	{"MOUSE8", K_MOUSE8},
// <- fuh

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},
	{"AUX17", K_AUX17},
	{"AUX18", K_AUX18},
	{"AUX19", K_AUX19},
	{"AUX20", K_AUX20},
	{"AUX21", K_AUX21},
	{"AUX22", K_AUX22},
	{"AUX23", K_AUX23},
	{"AUX24", K_AUX24},
	{"AUX25", K_AUX25},
	{"AUX26", K_AUX26},
	{"AUX27", K_AUX27},
	{"AUX28", K_AUX28},
	{"AUX29", K_AUX29},
	{"AUX30", K_AUX30},
	{"AUX31", K_AUX31},
	{"AUX32", K_AUX32},

// BorisU -->
	// Keypad stuff..
	// Tonik's code

	{"NUMLOCK", KP_NUMLOCK},
	{"KP_NUMLCK", KP_NUMLOCK},
	{"KP_NUMLOCK", KP_NUMLOCK},
	{"KP_SLASH", KP_SLASH},
	{"KP_DIVIDE", KP_SLASH},
	{"KP_STAR", KP_STAR},
	{"KP_MULTIPLY", KP_STAR},

	{"KP_MINUS", KP_MINUS},

	{"KP_HOME", KP_HOME},
	{"KP_7", KP_HOME},
	{"KP_UPARROW", KP_UPARROW},
	{"KP_8", KP_UPARROW},
	{"KP_PGUP", KP_PGUP},
	{"KP_9", KP_PGUP},

	{"KP_PLUS", KP_PLUS},

	{"KP_LEFTARROW", KP_LEFTARROW},
	{"KP_4", KP_LEFTARROW},
	{"KP_5", KP_5},
	{"KP_RIGHTARROW", KP_RIGHTARROW},
	{"KP_6", KP_RIGHTARROW},

	{"KP_END", KP_END},
	{"KP_1", KP_END},
	{"KP_DOWNARROW", KP_DOWNARROW},
	{"KP_2", KP_DOWNARROW},
	{"KP_PGDN", KP_PGDN},
	{"KP_3", KP_PGDN},

	{"KP_INS", KP_INS},
	{"KP_0", KP_INS},
	{"KP_DEL", KP_DEL},
	{"KP_DOT", KP_DEL},
	{"KP_ENTER", KP_ENTER},

	{"CAPSLOCK", K_CAPSLOCK},
	{"PRINTSCR", K_PRINTSCR},
	{"SCRLCK", K_SCRLCK},
	{"SCROLLCK", K_SCRLCK},
// <-- BorisU

	{"PAUSE", K_PAUSE},

	{"MWHEELUP", K_MWHEELUP},
	{"MWHEELDOWN", K_MWHEELDOWN},

	{"SEMICOLON", ';'},	// because a raw semicolon seperates commands

	{NULL, 0}
};

/*================================
  aeInput Class metods
================================*/
/*===================
aeInput::aeInput constructor
===================*/
aeInput::aeInput(  )
{
	memset( keydown, 0, 256 * sizeof( bool ) );
	autorepeatstatus = false;
	messagestart = messageend = 0;
	active = true;
	mouse_buttons = 8;
	mouse_oldbuttonstate = 0;
}

/*===================
aeInput::~aeInput destructor
===================*/
aeInput::~aeInput(  )
{
	MouseActivate( false );
}

/*===================
aeInput::Init
===================*/

void aeInput::Init(  )
{
	MouseActivate( TRUE );
}

/*===================
aeInput::Shutdown
===================*/

void aeInput::Shutdown(  )
{
	MouseActivate( FALSE );
}

/*===================
aeInput::Activate
===================*/
void aeInput::Activate( bool state )
{
	if( active  == state ) 
		return;
	active = state;
	MouseActivate( state );
	if ( state )
		return;
	memset( keydown, 0, 256 * sizeof( bool ) );
	messagestart = messageend = 0;
	mousemove.x = mousemove.y = 0;
}

/*===================
aeInput::KeyEvent

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================*/
void aeInput::Key_Event( int key, bool down )
{
	if ( !active )
		return;
	keydown[key] = down;
	AddMessage( key | ( down ? 0 : keyDownMask ) );
}

/*===================
aeInput::AddMessage
===================*/
void aeInput::AddMessage( int key )
{
	int newend = messageend + 1;

	if ( newend >= MAX_KEY_MESSAGES )
		newend = 0;
	if ( newend == messagestart )
	{
		Log.Print( "Error: Переполнение очереди сообщений" ) ;
		return;
	}
	keymessages[newend] = key;
	messageend = newend;
}

/*===================
aeInput::GetKeyEvent
возвращает коды клавиш и символы с учетом shift,capslock
===================*/
int aeInput::GetKeyMessage(  )
{
	if ( messageend == messagestart )
		return 0;
	int key = keymessages[messagestart];

	if ( ++messagestart >= MAX_KEY_MESSAGES )
		messagestart = 0;
	return key;
}

/*===================
aeInput::GetMouseMove
возвращает перемещение мыши 
?(обработанное с учетом sens/m_pitch/m_yaw)
и обнуляет его
===================*/
point_t aeInput::GetMouseMove(  )
{
	point_t ret = mousemove;

	mousemove.x = mousemove.y = 0;
	return ret;
}

/*===================
aeInput::SetAutoRepeatMode
устанавливает режим автоповтора клавиш
===================*/
void aeInput::SetAutoRepeatMode( bool mode )
{
	autorepeatstatus = mode;
}

/*===================
aeInput::ProcessInput
===================*/
extern void Sys_SendKeyEvents( void );
void aeInput::ProcessInput(  )
{
	Sys_SendKeyEvents(  );
	IN_KeyInput(  );
	IN_MouseInput(  );
}

/*===================
aeInput::IN_KeyInput
Called by the system between frames
Should NOT be called during an interrupt!
===================*/
void aeInput::IN_KeyInput(  )
{
	if ( !autorepeatstatus )
		return;
}
