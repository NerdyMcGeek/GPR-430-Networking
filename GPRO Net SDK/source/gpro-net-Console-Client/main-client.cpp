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

	main-client.c/.cpp
	Main source for console client application.
*/

/*
	GPRO Net Project 2
	By Alexander Wood and Avery Follett
*/

#include "gpro-net/gpro-net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Windows.h>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/GetTime.h"

#include "gpro-net/gpro-net-common/gpro-net-console.h"
#include "gpro-net/gpro-net-common/gpro-net-gamestate.h"

#define SERVER_PORT 60000

const int MAX_MESSAGE_SZ = 256;

//These are the custom IDs for different tasks that we made
enum GameMessages
{
	ID_GAME_MESSAGE_1=ID_USER_PACKET_ENUM+1,
	ID_CHAT_MESSAGE,
	ID_USERNAME,
	ID_JOIN_USERNAME,
	ID_PRINT_CONNECTED_USERS,
	ID_SHUTDOWN,
	ID_LOBBY_SELECT,
	ID_START_GAME,
	ID_UPDATE_GAME,
	ID_MOVE
};

void printGameboard(gpro_mancala board)
{
	printf(" _______________________________________ \n");

	printf("| %u  ", board[0][0]);
	printf("| %u  ", board[0][1]);
	printf("| %u  ", board[0][2]);
	printf("| %u  ", board[0][3]);
	printf("| %u  ", board[0][4]);
	printf("| %u  ", board[0][5]);
	printf("| %u  |", board[0][6]);
	printf("    |\n");

	printf("|    |____|____|____|____|____|____|    |\n");

	printf("|    ");
	printf("| %u  ", board[1][1]);
	printf("| %u  ", board[1][2]);
	printf("| %u  ", board[1][3]);
	printf("| %u  ", board[1][4]);
	printf("| %u  ", board[1][5]);
	printf("| %u  ", board[1][6]);
	printf("| %u  |\n", board[1][0]);

	printf("|____|____|____|____|____|____|____|____|\n\n");
}

//	this is going to compare the x and y location of a mouse click to the mancala game 
//	board and return the enum of which position they clicked
//
//	this should only return cups on the player's side of the board,
gpro_mancala_index checkMancalaClickPosition(short xClick, short yClick)
{
	if (xClick >= 32 && xClick <= 35 && yClick >= 3 && yClick <= 4)
	{
		return gpro_mancala_index::gpro_mancala_cup1;
	}
	else if (xClick >= 27 && xClick <= 30 && yClick >= 3 && yClick <= 4)
	{
		return gpro_mancala_index::gpro_mancala_cup2;
	}
	else if (xClick >= 22 && xClick <= 25 && yClick >= 3 && yClick <= 4)
	{
		return gpro_mancala_index::gpro_mancala_cup3;
	}
	else if (xClick >= 17 && xClick <= 20 && yClick >= 3 && yClick <= 4)
	{
		return gpro_mancala_index::gpro_mancala_cup4;
	}
	else if (xClick >= 12 && xClick <= 15 && yClick >= 3 && yClick <= 4)
	{
		return gpro_mancala_index::gpro_mancala_cup5;
	}
	else if (xClick >= 7 && xClick <= 10 && yClick >= 3 && yClick <= 4)
	{
		return gpro_mancala_index::gpro_mancala_cup6;
	}
	else if (xClick >= 37 && xClick <= 40 && yClick >= 1 && yClick <= 4)
	{
		return gpro_mancala_index::gpro_mancala_score;
	}
	else
		return gpro_mancala_index::gpro_mancala_default;
}

int main(int const argc, char const* const argv[])
{
	char str[512]; //for the server IP
	char un[512]; //for the client's username
	char lobbyNum[2]; //for the selected lobby number

	//char arrays just used for comparison for user commands
	char shutdown[MAX_MESSAGE_SZ] = "quitServer\n";
	char clientList[MAX_MESSAGE_SZ] = "clientList\n";

	//set up RakNet vars
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	RakNet::SystemAddress sysAddress;
	RakNet::SocketDescriptor sd;

	CONSOLE_SELECTION_INFO selectionInfo;

	//game logic
	bool gameStarted = false;
	bool isTurn = false;
	bool isSpectator = false;

	//start the client process
	peer->Startup(1, &sd, 1);
	peer->SetOccasionalPing(true);

	//ask for user to enter server IP or hit ENTER for default
	printf("Enter server IP or hit enter for default\n");
	gets_s(str);
	if (str[0] == 0) {
		strcpy(str, "172.16.2.51");
	}

	//ask for user to enter their username
	printf("Enter a username. No spaces\n");
	gets_s(un);

	printf("Connecting...\n");
	peer->Connect(str, SERVER_PORT, 0, 0);

	//ask for lobby #
	printf("Enter a lobby number 1-4\n");
	gets_s(lobbyNum);

	//get a reference to THIS window
	//thank you StackOverflow: https://stackoverflow.com/questions/18034988/checking-if-a-window-is-active
	HWND window = GetForegroundWindow();

	bool running = true;

	while (running)
	{
		GetConsoleSelectionInfo(&selectionInfo);
		//if user presses space in the foreground window (doesn't activate other chat windows)
		if ((GetKeyState(VK_SPACE) & 0x8000) && window == GetForegroundWindow() && !gameStarted)
		{
			//asks for chat message
			printf("\nEnter message:");
			char message[MAX_MESSAGE_SZ];
			fgets(message, MAX_MESSAGE_SZ, stdin);

			RakNet::BitStream bsOut;
			RakNet::Time timestamp = RakNet::GetTime();
			RakNet::RakString username = un;
			RakNet::RakString chatMessage = message;

			//check if message is server shutdown command
			if (strcmp(message, shutdown) == 0)
			{
				bsOut.Write((RakNet::MessageID)ID_SHUTDOWN);
			}
			//check if message is print connected users command
			else if (strcmp(message, clientList) == 0)
			{
				bsOut.Write((RakNet::MessageID)ID_PRINT_CONNECTED_USERS);
			}
			//else, user is sending a normal chat message
			else
			{
				bsOut.Write((RakNet::MessageID)ID_TIMESTAMP);
				bsOut.Write(timestamp);

				bsOut.Write((RakNet::MessageID)ID_USERNAME);
				bsOut.Write(username);

				bsOut.Write((RakNet::MessageID)ID_CHAT_MESSAGE);
				bsOut.Write(chatMessage);
			}

			//send the packet to server
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, sysAddress, false);
		}

		//for each packet received from server
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			//set up bufPtr to help us handle packets with multiple IDs
			unsigned int bufIndex = 0;
			unsigned char bufPtr = packet->data[bufIndex];
			RakNet::BitStream bsIn(packet->data, packet->length, false);

			//while we are still reading the same packet
			while (bufPtr != NULL)
			{
				switch (packet->data[0])
				{
				// Handles initial connection to server
				case ID_CONNECTION_REQUEST_ACCEPTED:
				{
					printf("Our connection request has been accepted.\n");

					//save server address so we can send messages to server outside this for loop
					sysAddress = packet->systemAddress;

					RakNet::BitStream bsOut;
					RakNet::Time timestamp = RakNet::GetTime();
					RakNet::RakString username = un;

					bsOut.Write((RakNet::MessageID)ID_TIMESTAMP);
					bsOut.Write(timestamp);

					bsOut.Write((RakNet::MessageID)ID_JOIN_USERNAME);
					bsOut.Write(username);

					bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
					bsOut.Write("Hello world");

					bsOut.Write((RakNet::MessageID)ID_LOBBY_SELECT);
					bsOut.Write(lobbyNum);

					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);

					bufPtr = NULL;
				}
				break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					printf("The server is full.\n");
					bufPtr = NULL;
					break;
				case ID_DISCONNECTION_NOTIFICATION:
					printf("A client has disconnected.\n");
					bufPtr = NULL;
					break;
				case ID_CONNECTION_LOST:
					printf("A client lost the connection.\n");
					bufPtr = NULL;
					break;
				//Recieves welcome message after connecting to server
				case ID_GAME_MESSAGE_1:
				{
					RakNet::RakString rs;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);
					printf("%s\n", rs.C_String());

					//advance bufPtr
					bufPtr = packet->data[bufIndex + sizeof(RakNet::MessageID) + 2 + rs.GetLength()];
					bufIndex += sizeof(RakNet::MessageID) + 2 + static_cast<int>(rs.GetLength());
				}
				break;
				// Reads messages sent from server
				case ID_CHAT_MESSAGE:
				{
					RakNet::RakString rs;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);

					printf("%s ", rs.C_String());

					//advance bufptr
					bufPtr = packet->data[bufIndex + sizeof(RakNet::MessageID) + 2 + rs.GetLength()];
					bufIndex += sizeof(RakNet::MessageID) + 2 + static_cast<int>(rs.GetLength());
				}
				break;
				//Handles timestamps recieved from the server
				case ID_TIMESTAMP:
				{
					RakNet::Time ts;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(ts);

					//special printf to handle timestamp
					printf("\n%" PRINTF_64_BIT_MODIFIER "u ", ts);

					//advance the bufptr
					bufPtr = packet->data[bufIndex + sizeof((RakNet::MessageID)ID_TIMESTAMP) + sizeof(RakNet::Time)];
					bufIndex += sizeof((RakNet::MessageID)ID_TIMESTAMP) + sizeof(RakNet::Time);
				}
				break;
				// Handles usernames recieved from the server
				case ID_USERNAME:
				{
					RakNet::RakString rs;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);

					printf("%s ", rs.C_String());

					//advance the bufptr
					bufPtr = packet->data[bufIndex + sizeof(RakNet::MessageID) + 2 + rs.GetLength()];
					bufIndex += sizeof(RakNet::MessageID) + 2 + static_cast<int>(rs.GetLength());
				}
				break;
				// Reads list of users connected to the server
				case ID_PRINT_CONNECTED_USERS:
				{
					RakNet::RakString users;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(users);

					printf("%s ", users.C_String());

					//nothing further in this packet...
					bufPtr = NULL;
				}
				break;
				case ID_START_GAME:
				{
					gpro_mancala board;
					bool turn;
					bool spectator;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(board);
					bsIn.Read(turn);
					bsIn.Read(spectator);

					isTurn = turn;
					isSpectator = spectator;

					gameStarted = true;
					printf("Game Started\n");

					gpro_consoleClear();

					printGameboard(board);

					if (isTurn)
					{
						printf("It's your turn!\n");
					}

					bufPtr = NULL;
				}
				break;
				case ID_UPDATE_GAME:
				{
					gpro_mancala board;
					bool turn;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(board);
					bsIn.Read(turn);

					isTurn = turn;

					gpro_consoleClear();

					printGameboard(board);

					if (isTurn)
					{
						printf("It's your turn!\n");
					}

					bufPtr = NULL;
				}
				break;
				default:
					//printf("Message with identifier %i has arrived.\n", bufPtr);
					break;
				}
			}
		}

		//should be able to use something like this to get mouse click location
		if (window == GetForegroundWindow() && gameStarted && !isSpectator && isTurn)
		{
			//get coordinate of current selection
			short cursorX = selectionInfo.dwSelectionAnchor.X;
			short cursorY = selectionInfo.dwSelectionAnchor.Y;

			//if it's a valid selection
			if (cursorX != 0 || cursorY != 0)
			{
				//if the index is one of the 6 cups the player can choose from
				gpro_mancala_index clickedMancalaIndex = checkMancalaClickPosition(cursorX, cursorY);

				if (clickedMancalaIndex == gpro_mancala_index::gpro_mancala_cup1 ||
					clickedMancalaIndex == gpro_mancala_index::gpro_mancala_cup2 ||
					clickedMancalaIndex == gpro_mancala_index::gpro_mancala_cup3 ||
					clickedMancalaIndex == gpro_mancala_index::gpro_mancala_cup4 ||
					clickedMancalaIndex == gpro_mancala_index::gpro_mancala_cup5 ||
					clickedMancalaIndex == gpro_mancala_index::gpro_mancala_cup6)
				{
					printf("Clicked on slot\n");

					RakNet::BitStream bsOut;

					//write the cup index that the player chose
					bsOut.Write((RakNet::MessageID)ID_MOVE);
					bsOut.Write(clickedMancalaIndex);

					//send the packet to server
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, sysAddress, false);

					//it's no longer this client's turn
					isTurn = false;
				}
			}
		}
	}

	//shut down client
	RakNet::RakPeerInterface::DestroyInstance(peer);

	printf("\n\n");
	system("pause");
}
