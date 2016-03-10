#pragma once
/* The unknown key */
#define TOBI_KEY_UNKNOWN            -1

/* Printable keys */
#define TOBI_KEY_SPACE              32
#define TOBI_KEY_APOSTROPHE         39  /* ' */
#define TOBI_KEY_COMMA              44  /* , */
#define TOBI_KEY_MINUS              45  /* - */
#define TOBI_KEY_PERIOD             46  /* . */
#define TOBI_KEY_SLASH              47  /* / */
#define TOBI_KEY_0                  48
#define TOBI_KEY_1                  49
#define TOBI_KEY_2                  50
#define TOBI_KEY_3                  51
#define TOBI_KEY_4                  52
#define TOBI_KEY_5                  53
#define TOBI_KEY_6                  54
#define TOBI_KEY_7                  55
#define TOBI_KEY_8                  56
#define TOBI_KEY_9                  57
#define TOBI_KEY_SEMICOLON          59  /* ; */
#define TOBI_KEY_EQUAL              61  /* = */
#define TOBI_KEY_A                  65
#define TOBI_KEY_B                  66
#define TOBI_KEY_C                  67
#define TOBI_KEY_D                  68
#define TOBI_KEY_E                  69
#define TOBI_KEY_F                  70
#define TOBI_KEY_G                  71
#define TOBI_KEY_H                  72
#define TOBI_KEY_I                  73
#define TOBI_KEY_J                  74
#define TOBI_KEY_K                  75
#define TOBI_KEY_L                  76
#define TOBI_KEY_M                  77
#define TOBI_KEY_N                  78
#define TOBI_KEY_O                  79
#define TOBI_KEY_P                  80
#define TOBI_KEY_Q                  81
#define TOBI_KEY_R                  82
#define TOBI_KEY_S                  83
#define TOBI_KEY_T                  84
#define TOBI_KEY_U                  85
#define TOBI_KEY_V                  86
#define TOBI_KEY_W                  87
#define TOBI_KEY_X                  88
#define TOBI_KEY_Y                  89
#define TOBI_KEY_Z                  90
#define TOBI_KEY_LEFT_BRACKET       91  /* [ */
#define TOBI_KEY_BACKSLASH          92  /* \ */
#define TOBI_KEY_RIGHT_BRACKET      93  /* ] */
#define TOBI_KEY_GRAVE_ACCENT       96  /* ` */
#define TOBI_KEY_WORLD_1            161 /* non-US #1 */
#define TOBI_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define TOBI_KEY_ESCAPE             256
#define TOBI_KEY_ENTER              257
#define TOBI_KEY_TAB                258
#define TOBI_KEY_BACKSPACE          259
#define TOBI_KEY_INSERT             260
#define TOBI_KEY_DELETE             261
#define TOBI_KEY_RIGHT              262
#define TOBI_KEY_LEFT               263
#define TOBI_KEY_DOWN               264
#define TOBI_KEY_UP                 265
#define TOBI_KEY_PAGE_UP            266
#define TOBI_KEY_PAGE_DOWN          267
#define TOBI_KEY_HOME               268
#define TOBI_KEY_END                269
#define TOBI_KEY_CAPS_LOCK          280
#define TOBI_KEY_SCROLL_LOCK        281
#define TOBI_KEY_NUM_LOCK           282
#define TOBI_KEY_PRINT_SCREEN       283
#define TOBI_KEY_PAUSE              284
#define TOBI_KEY_F1                 290
#define TOBI_KEY_F2                 291
#define TOBI_KEY_F3                 292
#define TOBI_KEY_F4                 293
#define TOBI_KEY_F5                 294
#define TOBI_KEY_F6                 295
#define TOBI_KEY_F7                 296
#define TOBI_KEY_F8                 297
#define TOBI_KEY_F9                 298
#define TOBI_KEY_F10                299
#define TOBI_KEY_F11                300
#define TOBI_KEY_F12                301
#define TOBI_KEY_F13                302
#define TOBI_KEY_F14                303
#define TOBI_KEY_F15                304
#define TOBI_KEY_F16                305
#define TOBI_KEY_F17                306
#define TOBI_KEY_F18                307
#define TOBI_KEY_F19                308
#define TOBI_KEY_F20                309
#define TOBI_KEY_F21                310
#define TOBI_KEY_F22                311
#define TOBI_KEY_F23                312
#define TOBI_KEY_F24                313
#define TOBI_KEY_F25                314
#define TOBI_KEY_KP_0               320
#define TOBI_KEY_KP_1               321
#define TOBI_KEY_KP_2               322
#define TOBI_KEY_KP_3               323
#define TOBI_KEY_KP_4               324
#define TOBI_KEY_KP_5               325
#define TOBI_KEY_KP_6               326
#define TOBI_KEY_KP_7               327
#define TOBI_KEY_KP_8               328
#define TOBI_KEY_KP_9               329
#define TOBI_KEY_KP_DECIMAL         330
#define TOBI_KEY_KP_DIVIDE          331
#define TOBI_KEY_KP_MULTIPLY        332
#define TOBI_KEY_KP_SUBTRACT        333
#define TOBI_KEY_KP_ADD             334
#define TOBI_KEY_KP_ENTER           335
#define TOBI_KEY_KP_EQUAL           336
#define TOBI_KEY_LEFT_SHIFT         340
#define TOBI_KEY_LEFT_CONTROL       341
#define TOBI_KEY_LEFT_ALT           342
#define TOBI_KEY_LEFT_SUPER         343
#define TOBI_KEY_RIGHT_SHIFT        344
#define TOBI_KEY_RIGHT_CONTROL      345
#define TOBI_KEY_RIGHT_ALT          346
#define TOBI_KEY_RIGHT_SUPER        347
#define TOBI_KEY_MENU               348
#define TOBI_KEY_LAST               TOBI_KEY_MENU

class InputInterFace
{
public:
	static InputInterFace* getInstance();
	~InputInterFace();

	bool getKeyState(int key) {	return keyStates[key]; }
	
	void setKeyState(int key, bool state) { keyStates[key] = state; }

	void getCursorMove(double &cursorMoveX, double &cursorMoveY);

	void setCursorPos(double cursorPosX, double cursorPosY);

private:
	bool keyStates[350];
	InputInterFace();
	static InputInterFace* instance;

	double cursorPosX;
	double cursorPosY;

	double prevCursorPosX;
	double prevCursorPosY;
};