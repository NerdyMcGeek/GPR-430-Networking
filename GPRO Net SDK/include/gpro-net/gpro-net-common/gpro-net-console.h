/*
   Copyright 2021 Daniel S. Buckstein

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	GPRO Net SDK: Networking framework.
	By Daniel S. Buckstein

	gpro-net-console.h
	Console management interface.
*/

#ifndef _GPRO_NET_CONSOLE_H_
#define _GPRO_NET_CONSOLE_H_


#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus


//-----------------------------------------------------------------------------

// eprintf
//	Shorthand macro for outputting formatted string to standard error.
#define eprintf(fmt, ...)		fprintf(stderr, fmt, __VA_ARGS__)

// dprintf
//	Shorthand macro for outputting formatted string to debugging interface.
#define dprintf(fmt, ...)		gpro_consolePrintDebug(fmt, __VA_ARGS__)


// gpro_consoleColor
//	List of color codes for changing display style in console.
typedef enum gpro_consoleColor
{
	// Channels.
	///
	gpro_consoleColor_0 = 0x0,	// Color is disabled.
	gpro_consoleColor_b = 0x1,	// Color contains blue channel.
	gpro_consoleColor_g = 0x2,	// Color contains green channel.
	gpro_consoleColor_gb,			// Color contains green and blue channels.
	gpro_consoleColor_r = 0x4,	// Color contains red channel.
	gpro_consoleColor_br,			// Color contains blue and red channels.
	gpro_consoleColor_rg,			// Color contains red and green channels.
	gpro_consoleColor_rgb,		// Color contains red, green and blue channels.
	gpro_consoleColor_a = 0x8,	// Color is fully opaque/intense.

	// Colors.
	///
	gpro_consoleColor_black = gpro_consoleColor_0,	// Black:           (0, 0, 0, 0)
	gpro_consoleColor_blue_d,						// Blue (dark):     (0, 0, 1, 0)
	gpro_consoleColor_green_d,						// Green (dark):    (0, 1, 0, 0)
	gpro_consoleColor_cyan_d,						// Cyan (dark):     (0, 1, 1, 0)
	gpro_consoleColor_red_d,						// Red (dark):      (1, 0, 0, 0)
	gpro_consoleColor_magenta_d,					// Magenta (dark):  (1, 0, 1, 0)
	gpro_consoleColor_yellow_d,						// Yellow (dark):   (1, 1, 0, 0)
	gpro_consoleColor_grey,							// Grey:            (1, 1, 1, 0)
	gpro_consoleColor_grey_d,						// Grey (dark):     (0, 0, 0, 1)
	gpro_consoleColor_blue,							// Blue:            (0, 0, 1, 1)
	gpro_consoleColor_green,						// Green:           (0, 1, 0, 1)
	gpro_consoleColor_cyan,							// Cyan:            (0, 1, 1, 1)
	gpro_consoleColor_red,							// Red:             (1, 0, 0, 1)
	gpro_consoleColor_magenta,						// Magenta:         (1, 0, 1, 1)
	gpro_consoleColor_yellow,						// Yellow:          (1, 1, 0, 1)
	gpro_consoleColor_white,						// White:           (1, 1, 1, 1)
} gpro_consoleColor;


//-----------------------------------------------------------------------------

// gpro_consoleGetCursor
//	Get position of cursor in console.
//		param x_out: pointer to value to store horizontal coordinate
//			valid: non-null
//		param y_out: pointer to value to store vertical coordinate (from top)
//			valid: non-null
//		return SUCCESS: 0 if operation succeeded
//		return FAILURE: -2 if operation failed
//		return FAILURE: -1 if invalid parameters
int gpro_consoleGetCursor(short* const x_out, short* const y_out);

// gpro_consoleSetCursor
//	Set position of cursor in console.
//		param x: value of horizontal coordinate to set
//		param y: value of vertical coordinate to set (from top)
//		return SUCCESS: 0 if operation succeeded
//		return FAILURE: -2 if operation failed
int gpro_consoleSetCursor(short const x, short const y);

// gpro_consoleToggleCursor
//	Toggle blinking underscore at cursor location.
//		param visible: visible flag; interpreted as boolean
//		return SUCCESS: 0 if operation succeeded
//		return FAILURE: -2 if operation failed
int gpro_consoleToggleCursor(int const visible);

// gpro_consoleGetColor
//	Set color of console text.
//		param fg_out: pointer to description of foreground (character) channels
//			valid: non-null
//		param bg_out: pointer to description of background (console) channels
//			valid: non-null
//		return SUCCESS: 0 if operation succeeded
//		return FAILURE: -2 if operation failed
//		return FAILURE: -1 if invalid parameters
int gpro_consoleGetColor(gpro_consoleColor* const fg_out, gpro_consoleColor* const bg_out);

// gpro_consoleSetColor
//	Set color of console text.
//		param fg: description of foreground (character) channels to use
//		param bg: description of background (console) channels to use
//		return SUCCESS: 0 if operation succeeded
//		return FAILURE: -2 if operation failed
int gpro_consoleSetColor(gpro_consoleColor const fg, gpro_consoleColor const bg);

// gpro_consoleResetColor
//	Reset color of console text.
//		return SUCCESS: 0 if operation succeeded
//		return FAILURE: -2 if operation failed
int gpro_consoleResetColor();

// gpro_consoleGetCursorColor
//	Get console cursor position and color.
//		param x_out: pointer to value to store horizontal coordinate
//			valid: non-null
//		param y_out: pointer to value to store vertical coordinate (from top)
//			valid: non-null
//		param fg_out: pointer to description of foreground (character) channels
//			valid: non-null
//		param bg_out: pointer to description of background (console) channels
//			valid: non-null
//		return SUCCESS: 0 if operation succeeded
//		return FAILURE: -2 if operation failed
//		return FAILURE: -1 if invalid parameters
int gpro_consoleGetCursorColor(short* const x_out, short* const y_out, gpro_consoleColor* const fg_out, gpro_consoleColor* const bg_out);

// gpro_consoleSetCursorColor
//	Get console cursor position and color.
//		param x: value of horizontal coordinate to set
//		param y: value of vertical coordinate to set (from top)
//		param fg: description of foreground (character) channels to use
//		param bg: description of background (console) channels to use
//		return SUCCESS: 0 if operation succeeded
//		return FAILURE: -2 if operation failed
//		return FAILURE: -1 if invalid parameters
int gpro_consoleSetCursorColor(short const x, short const y, gpro_consoleColor const fg, gpro_consoleColor const bg);

// gpro_consoleDrawTestPatch
//	Display test patch in console.
//		return SUCCESS: 0 if operation succeeded
//		return FAILURE: -2 if operation failed
int gpro_consoleDrawTestPatch();

// gpro_consoleClear
//	Clear console, erasing text and setting background to set color.
//		return SUCCESS: 0 if operation succeeded
//		return FAILURE: -2 if operation failed
int gpro_consoleClear();


//-----------------------------------------------------------------------------

// gpro_consolePrintDebug
//	Print formatted string to debugging interface.
//		param format: format string, as used with standard 'printf'
//			valid: non-null c-string
//		params ...: parameter list matching specifications in 'format'
//		return SUCCESS: result of internal print operation if succeeded
//		return FAILURE: -1 if invalid parameters
int gpro_consolePrintDebug(char const* const format, ...);


//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !_GPRO_NET_CONSOLE_H_