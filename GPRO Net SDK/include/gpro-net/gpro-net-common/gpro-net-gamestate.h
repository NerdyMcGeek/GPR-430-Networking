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

	gpro-net-gamestate.h
	Header for common networking mini-game states.
*/

#ifndef _GPRO_NET_GAMESTATE_H_
#define _GPRO_NET_GAMESTATE_H_


#define gpro_flag_raise(value, flag)	(value | flag)	// value with flag raised
#define gpro_flag_toggle(value, flag)	(value ^ flag)	// value with flag toggled
#define gpro_flag_check(value, flag)	(value & flag)	// value contains part of flag
#define gpro_flag_lower(value, flag)	(value & ~flag)	// value with flag lowered
#define gpro_flag_equals(value, flag)	(gpro_flag_check(value, flag) == flag)	// value contains entire flag


#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus


typedef unsigned char
	gpro_battleship[10][10],	// battleship board (one player)
	gpro_checkers[8][4],		// checkerboard (shared)
	gpro_mancala[2][8];			// mancala board (shared)

/*
	Checkers:
		[8]
			 _______________________________________
		 0	| 0  |    | 1  |    | 2  |    | 3  |    |	[4]
			|____|____|____|____|____|____|____|____|
		 1	|    | 0  |    | 1  |    | 2  |    | 3  |
			|____|____|____|____|____|____|____|____|
		 2	| 0  |    | 1  |    | 2  |    | 3  |    |
			|____|____|____|____|____|____|____|____|
		 3	|    | 0  |    | 1  |    | 2  |    | 3  |
			|____|____|____|____|____|____|____|____|
		 4	| 0  |    | 1  |    | 2  |    | 3  |    |
			|____|____|____|____|____|____|____|____|
		 5	|    | 0  |    | 1  |    | 2  |    | 3  |
			|____|____|____|____|____|____|____|____|
		 6	| 0  |    | 1  |    | 2  |    | 3  |    |
			|____|____|____|____|____|____|____|____|
		 7	|    | 0  |    | 1  |    | 2  |    | 3  |
			|____|____|____|____|____|____|____|____|


	Mancala:
		[2]        ___________________________
				  /				7			  \
			 _______________________________________
		 0	| 0  | 1  | 2  | 3  | 4  | 5  | 6  |    |	[8]
			|    |____|____|____|____|____|____|    |
		 1	|    | 6  | 5  | 4  | 3  | 2  | 1  | 0  |
			|____|____|____|____|____|____|____|____|

				  \___________________________/
								7
*/

typedef enum gpro_battleship_flag
{
	gpro_battleship_open,			// we have not attacked this space
	gpro_battleship_miss = 0x01,	// we have attacked this space and missed
	gpro_battleship_hit = 0x02,		// we have attacked this space and hit
	gpro_battleship_damage = 0x04,	// our ship is damaged; their attack hit
	gpro_battleship_ship_p2 = 0x08,	// space contains patrol boat (2 spaces)
	gpro_battleship_ship_s3 = 0x10,	// space contains submarine (3 spaces)
	gpro_battleship_ship_d3 = 0x20,	// space contains destroyer (3 spaces)
	gpro_battleship_ship_b4 = 0x40,	// space contains battleship (4 spaces)
	gpro_battleship_ship_c5 = 0x80,	// space contains carrier (5 spaces)
	gpro_battleship_ship = gpro_battleship_ship_p2 | gpro_battleship_ship_s3 |
		gpro_battleship_ship_d3 | gpro_battleship_ship_b4 | gpro_battleship_ship_c5, // our ships
	gpro_battleship_attack_rec = gpro_battleship_miss | gpro_battleship_hit,	// our records of attack
	gpro_battleship_defend_rec = gpro_battleship_damage | gpro_battleship_ship,	// our records of defense
} gpro_battleship_flag;

typedef enum gpro_checkers_flag
{
	gpro_checkers_open,				// empty space
	gpro_checkers_player1 = 0x01,	// player 1 piece
	gpro_checkers_player2 = 0x02,	// player 2 piece
	gpro_checkers_stack = 0x04,		// stacked pieces
	gpro_checkers_player1_stack = gpro_checkers_player1 | gpro_checkers_stack,	// player 1 stacked pieces
	gpro_checkers_player2_stack = gpro_checkers_player2 | gpro_checkers_stack,	// player 2 stacked pieces
} gpro_checkers_flag;

typedef enum gpro_mancala_index
{
	gpro_mancala_score,
	gpro_mancala_cup1,
	gpro_mancala_cup2,
	gpro_mancala_cup3,
	gpro_mancala_cup4,
	gpro_mancala_cup5,
	gpro_mancala_cup6,
	gpro_mancala_onside,
} gpro_mancala_index;


inline void gpro_battleship_reset(gpro_battleship gs)
{
	int* itr = (int*)gs, * const end = itr + sizeof(gpro_battleship) / sizeof(int);
	while (itr < end)
		*(itr++) = 0;
}

inline void gpro_checkers_reset(gpro_checkers gs)
{
	int* itr = (int*)gs;
	itr[0] = itr[1] = itr[2] = 0x01010101; // player 1 rows
	itr[3] = itr[4] = 0x00000000; // empty rows
	itr[5] = itr[6] = itr[7] = 0x02020202; // player 2 rows
}

inline void gpro_mancala_reset(gpro_mancala gs)
{
	long long* itr = (long long*)gs;
	itr[0] = itr[1] = 0x0004040404040400; // 6 cups, 4 stones each
	gs[0][7] = gs[1][7] = 24; // on-side total
}


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !_GPRO_NET_GAMESTATE_H_