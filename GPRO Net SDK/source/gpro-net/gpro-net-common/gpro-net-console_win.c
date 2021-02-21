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

	gpro-net-console_win.c
	Console management source for Windows.
*/

#include "gpro-net/gpro-net-common/gpro-net-console.h"
#ifdef _WIN32

#include <io.h>
#include <stdio.h>
#include <Windows.h>


//-----------------------------------------------------------------------------

int gpro_consoleGetCursor(short* const x_out, short* const y_out)
{
	if (x_out && y_out)
	{
		CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo[1];
		HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE), console = GetConsoleWindow();
		if (stdHandle && console &&
			GetConsoleScreenBufferInfo(stdHandle, screenBufferInfo))
		{
			*x_out = screenBufferInfo->dwCursorPosition.X;
			*y_out = screenBufferInfo->dwCursorPosition.Y;
			return 0;
		}
		return -2;
	}
	return -1;
}


int gpro_consoleSetCursor(short const x, short const y)
{
	COORD const pos = { x, y };
	HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE), console = GetConsoleWindow();
	if (stdHandle && console &&
		SetConsoleCursorPosition(stdHandle, pos))
	{
		return 0;
	}
	return -2;
}


int gpro_consoleToggleCursor(int const visible)
{
	CONSOLE_CURSOR_INFO cursorInfo[1];
	HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE), console = GetConsoleWindow();
	if (stdHandle && console &&
		GetConsoleCursorInfo(stdHandle, cursorInfo))
	{
		cursorInfo->bVisible = visible;
		if (SetConsoleCursorInfo(stdHandle, cursorInfo))
		{
			return 0;
		}
	}
	return -2;
}


int gpro_consoleGetColor(gpro_consoleColor* const fg_out, gpro_consoleColor* const bg_out)
{
	if (fg_out && bg_out)
	{
		CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo[1];
		HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE), console = GetConsoleWindow();
		if (stdHandle && console &&
			GetConsoleScreenBufferInfo(stdHandle, screenBufferInfo))
		{
			*fg_out = (gpro_consoleColor)(screenBufferInfo->wAttributes & 0xf);
			*bg_out = (gpro_consoleColor)(screenBufferInfo->wAttributes >> 4 & 0xf);
			return 0;
		}
		return -2;
	}
	return -1;
}


int gpro_consoleSetColor(gpro_consoleColor const fg, gpro_consoleColor const bg)
{
	HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE), console = GetConsoleWindow();
	if (stdHandle && console &&
		SetConsoleTextAttribute(stdHandle, (short)(fg | bg << 4)))
	{
		return 0;
	}
	return -2;
}


int gpro_consoleResetColor()
{
	return gpro_consoleSetColor(gpro_consoleColor_white, gpro_consoleColor_black);
}


int gpro_consoleGetCursorColor(short* const x_out, short* const y_out, gpro_consoleColor* const fg_out, gpro_consoleColor* const bg_out)
{
	if (x_out && y_out && fg_out && bg_out)
	{
		CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo[1];
		HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE), console = GetConsoleWindow();
		if (stdHandle && console &&
			GetConsoleScreenBufferInfo(stdHandle, screenBufferInfo))
		{
			*x_out = screenBufferInfo->dwCursorPosition.X;
			*y_out = screenBufferInfo->dwCursorPosition.Y;
			*fg_out = (gpro_consoleColor)(screenBufferInfo->wAttributes & 0xf);
			*bg_out = (gpro_consoleColor)(screenBufferInfo->wAttributes >> 4 & 0xf);
			return 0;
		}
		return -2;
	}
	return -1;
}


int gpro_consoleSetCursorColor(short const x, short const y, gpro_consoleColor const fg, gpro_consoleColor const bg)
{
	COORD const pos = { x, y };
	HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE), console = GetConsoleWindow();
	if (stdHandle && console &&
		SetConsoleCursorPosition(stdHandle, pos) &&
		SetConsoleTextAttribute(stdHandle, (short)(fg | bg << 4)))
	{
		return 0;
	}
	return -2;
}


int gpro_consoleDrawTestPatch()
{
	HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE), console = GetConsoleWindow();
	if (stdHandle && console)
	{
		// test all colors and shifts
		short x, y;
		gpro_consoleColor fg, bg;
		for (y = 0; y < 16; ++y)
		{
			for (x = 0; x < 16; ++x)
			{
				fg = (gpro_consoleColor)y;
				bg = (gpro_consoleColor)x;
				gpro_consoleSetColor(fg, bg);
				gpro_consoleSetCursor(x * 2, y);
				printf("%x", (int)x);
				gpro_consoleSetCursorColor(x * 2 + 1, y, fg, bg);
				printf("%x", (int)y);
			}
		}
		gpro_consoleGetCursor(&x, &y);
		gpro_consoleGetColor(&fg, &bg);
		gpro_consoleGetCursorColor(&x, &y, &fg, &bg);
		gpro_consoleResetColor();
		printf("[]=(%d, %d) \n", (int)x, (int)y);

		// done
		return 0;
	}
	return -2;
}


int gpro_consoleClear()
{
	// help to avoid using system("cls"): https://docs.microsoft.com/en-us/windows/console/clearing-the-screen 
	HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE), console = GetConsoleWindow();
	if (stdHandle && console)
	{
		// simple clear
		//system("cls");

		CONSOLE_SCREEN_BUFFER_INFO buffer[1];
		if (GetConsoleScreenBufferInfo(stdHandle, buffer))
		{
			COORD const coord = { 0, 0 };
			DWORD const sz = (buffer->dwSize.X * buffer->dwSize.Y);
			DWORD write[1] = { 0 };
			if (FillConsoleOutputCharacterA(stdHandle, ' ', sz, coord, write) &&
				GetConsoleScreenBufferInfo(stdHandle, buffer) &&
				FillConsoleOutputAttribute(stdHandle, buffer->wAttributes, sz, coord, write) &&
				SetConsoleCursorPosition(stdHandle, coord))
			{
				// done
				return 0;
			}
		}
	}
	return -2;
}


//-----------------------------------------------------------------------------

int gpro_consolePrintDebug(char const* const format, ...)
{
	if (format)
	{
		unsigned char str[256] = { 0 };
		va_list args = 0;
		int result = 0;

		// fill buffer with formatted arguments
		va_start(args, format);
		result = _vsnprintf(str, sizeof(str), format, args);
		va_end(args);

		// internal print
		OutputDebugStringA(str);

		// return length
		return result;
	}
	return -1;
}


//-----------------------------------------------------------------------------


#endif	// _WIN32