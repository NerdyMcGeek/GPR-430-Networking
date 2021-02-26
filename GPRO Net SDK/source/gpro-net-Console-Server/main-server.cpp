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

	main-server.c/.cpp
	Main source for console server application.
*/

/*
	GPRO Net Project 2
	By Alexander Wood and Avery Follett
*/

#include "gpro-net/gpro-net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"

#include "gpro-net/gpro-net-common/gpro-net-console.h"
#include "gpro-net/gpro-net-common/gpro-net-gamestate.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 60000
#define MAX_LOBBIES 4

//These are the custom IDs for different tasks that we made
enum GameMessages
{
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1,
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

struct ServerUser;

struct Lobby
{
	std::vector<ServerUser> users;
	int lobbyNumber;
	bool started;
	gpro_mancala gameboard;

	Lobby() : lobbyNumber(-1), started(false) 
	{
		gpro_mancala_reset(gameboard);
	}
	Lobby(int lobbyNumber) : lobbyNumber(lobbyNumber), started(false) 
	{
		gpro_mancala_reset(gameboard);
	}
};

struct ServerUser
{
	int id;
	RakNet::RakString username;
	RakNet::SystemAddress address;
	Lobby* currentLobby;
	bool isSpectator;
	bool turn = false;

	//when we construct a ServerUser, we must provide it a system address at least
	ServerUser(RakNet::SystemAddress address) : address(address), id(-1), currentLobby(NULL), isSpectator(false) {}

	bool operator () (const ServerUser* user) const
	{
		return user->address == address;
	}

	bool isLobbyNum(int lobbySearchNumber)
	{
		return lobbySearchNumber == currentLobby->lobbyNumber;
	}
};

int main(int const argc, char const* const argv[])
{
	//set up RakNet vars
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	RakNet::Packet* packet;

	//start the server
	peer->Startup(MAX_CLIENTS, &sd, 1);
	peer->SetOccasionalPing(true);
	peer->SetMaximumIncomingConnections(MAX_CLIENTS);
	printf("Starting the server.\n");

	//user/lobby management
	std::vector<ServerUser*> users;
	std::vector<Lobby*> lobbies;

	for (int i = 1; i <= MAX_LOBBIES; i++)
	{
		lobbies.push_back(new Lobby(i));
	}

	//open log file to start logging server output
	FILE* logFile = fopen("messageLog.txt", "w");
	if (logFile == NULL)
	{
		printf("Could not open file");
	}
	// ^ this saves in a weird place. users/[user]/remote/[reponame]/bin

	bool running = true;
	//Always running loop to receive packets
	while (running)
	{
		//For each packet the server receives
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			//set up bufPtr to help us handle packets with multiple IDs
			unsigned int bufIndex = 0;
			unsigned char bufPtr = packet->data[bufIndex];
			RakNet::BitStream bsIn(packet->data, packet->length, false);

			//while we are still reading the same packet
			while (bufPtr != NULL)
			{
				//For each ID within packet
				switch (bufPtr)
				{
				case ID_REMOTE_DISCONNECTION_NOTIFICATION:
					printf("Another client has disconnected.\n");
					bufPtr = NULL;
					break;
				case ID_REMOTE_CONNECTION_LOST:
					printf("Another client has lost the connection.\n");
					bufPtr = NULL;
					break;
				case ID_REMOTE_NEW_INCOMING_CONNECTION:
					printf("Another client has connected.\n");
					bufPtr = NULL;
					break;
				case ID_CONNECTION_REQUEST_ACCEPTED:
					printf("Our connection request has been accepted.\n");
					bufPtr = NULL;
					break;
				case ID_NEW_INCOMING_CONNECTION:
				{
					printf("A connection is incoming.\n");
					ServerUser* user = new ServerUser(packet->systemAddress);
					user->id = RakNet::RakNetGUID::ToUint32(packet->guid);
					users.push_back(user);
					bufPtr = NULL;
				}
					break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					printf("The server is full.\n");
					bufPtr = NULL;
					break;
				case ID_DISCONNECTION_NOTIFICATION:
					printf("A client has disconnected.\n");
					fclose(logFile); //stop logging
					bufPtr = NULL;
					break;
				case ID_CONNECTION_LOST:
					printf("A client lost the connection.\n");
					fclose(logFile); //stop logging
					bufPtr = NULL;
					break;
				//This handles when a client connects
				case ID_GAME_MESSAGE_1:
				{
					RakNet::RakString rs;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);

					printf("%s\n", rs.C_String());
					//send message to log
					fprintf(logFile, "%s\n", rs.C_String());

					//let client know you got their welcome message by sending one back
					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
					bsOut.Write("Welcome client!");
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);

					//advance the bufptr
					bufPtr = packet->data[bufIndex + sizeof(RakNet::MessageID) + 2 + rs.GetLength()];
					bufIndex += sizeof(RakNet::MessageID) + 2 + static_cast<int>(rs.GetLength());
				}
				break;
				//This handles messages recieved from clients
				case ID_CHAT_MESSAGE:
				{
					RakNet::RakString rs;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);

					std::vector<ServerUser*>::iterator it = std::find_if(users.begin(), users.end(), ServerUser(packet->systemAddress));
					if (!it[0]->isSpectator)
					{
						//print chat message
						printf("%s", rs.C_String());
						fprintf(logFile, "%s", rs.C_String());

						//distribute chat message to all clients
						RakNet::BitStream bsOut;
						bsOut.Write((RakNet::MessageID)ID_CHAT_MESSAGE);
						bsOut.Write(rs);

						std::vector<ServerUser*>::iterator it = std::find_if(users.begin(), users.end(), ServerUser(packet->systemAddress));

						for (size_t i = 0; i < users.size(); i++)
						{
							if (users[i]->isLobbyNum(it[0]->currentLobby->lobbyNumber))
							{
								peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, users[i]->address, false);
							}
						}
					}

					//advance the bufptr
					bufPtr = packet->data[bufIndex + sizeof(RakNet::MessageID) + 2 + rs.GetLength()];
					bufIndex += sizeof(RakNet::MessageID) + 2 + static_cast<int>(rs.GetLength());
				}
				break;
				//Handles message timestamping
				case ID_TIMESTAMP:
				{
					std::vector<ServerUser*>::iterator it = std::find_if(users.begin(), users.end(), ServerUser(packet->systemAddress));
					if (!it[0]->isSpectator)
					{
						RakNet::Time ts;

						bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
						bsIn.Read(ts);

						//special printf to handle timestamp
						printf("%" PRINTF_64_BIT_MODIFIER "u ", ts);
						//send timestamp to log
						fprintf(logFile, "%" PRINTF_64_BIT_MODIFIER "u ", ts);
					}

						//advance the bufptr
						bufPtr = packet->data[bufIndex + sizeof((RakNet::MessageID)ID_TIMESTAMP) + sizeof(RakNet::Time)];
						bufIndex += sizeof((RakNet::MessageID)ID_TIMESTAMP) + sizeof(RakNet::Time);
				}
				break;
				//Handles client usernames
				case ID_USERNAME:
				{
					RakNet::RakString rs;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);

					std::vector<ServerUser*>::iterator it = std::find_if(users.begin(), users.end(), ServerUser(packet->systemAddress));
					if (!it[0]->isSpectator)
					{
						//print the username
						printf("%s ", rs.C_String());
						fprintf(logFile, "%s ", rs.C_String());

						//distribute username to all clients
						RakNet::BitStream bsOut;
						bsOut.Write((RakNet::MessageID)ID_USERNAME);
						bsOut.Write(rs);

						std::vector<ServerUser*>::iterator it = std::find_if(users.begin(), users.end(), ServerUser(packet->systemAddress));

						for (size_t i = 0; i < users.size(); i++)
						{
							if (users[i]->isLobbyNum(it[0]->currentLobby->lobbyNumber))
							{
								peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, users[i]->address, false);
							}
						}
					}

					//advance the bufptr
					bufPtr = packet->data[bufIndex + sizeof(RakNet::MessageID) + 2 + rs.GetLength()];
					bufIndex += sizeof(RakNet::MessageID) + 2 + static_cast<int>(rs.GetLength());
				}
				break;
				//Proper shutdown procedure (called when client sends the message quitServer)
				case ID_SHUTDOWN:
				{
					running = false;
					bufPtr = NULL;
				}
				break;
				//Handles usernames and adds to a vector when connected
				case ID_JOIN_USERNAME:
				{
					RakNet::RakString rs;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);

					//add username to user based on system IP address
					std::vector<ServerUser*>::iterator it = std::find_if(users.begin(), users.end(), ServerUser(packet->systemAddress));
					it[0]->username = rs;

					printf("%s ", it[0]->username.C_String());

					//advance the bufptr
					bufPtr = packet->data[bufIndex + sizeof(RakNet::MessageID) + 2 + rs.GetLength()];
					bufIndex += sizeof(RakNet::MessageID) + 2 + static_cast<int>(rs.GetLength());
				}
				break;
				//Prints out a vector of connected users when requested by client (client sends message clientList)
				case ID_PRINT_CONNECTED_USERS:
				{
					//loop through all connected users and add them all to a string
					RakNet::RakString usersString = "Users: \n";
					for (int i = 0; i < users.size(); i++)
					{
						usersString += users.at(i)->username + " ";
						printf("%s ", usersString.C_String());
					}

					usersString += "\n";

					//send the string of connected users to the client who requested
					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_PRINT_CONNECTED_USERS);
					bsOut.Write(users);

					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);

					bufPtr = NULL;
				}
				break;
				case ID_LOBBY_SELECT:
				{
					char ri[2];
					char lobby1text[2] = "1";
					char lobby2text[2] = "2";
					char lobby3text[2] = "3";
					char lobby4text[2] = "4";

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(ri);

					std::vector<ServerUser*>::iterator it = std::find_if(users.begin(), users.end(), ServerUser(packet->systemAddress));

					//Assign a lobby number to the user
					if (strcmp(ri, lobby1text) == 0)
					{
						it[0]->currentLobby = lobbies[0];
						it[0]->currentLobby->users.push_back(*it[0]);
						printf(it[0]->address.ToString());
						printf(" connected to lobby ");
						printf("%d ", it[0]->currentLobby->lobbyNumber);
						if (it[0]->currentLobby->users.size() > 2)
						{
							it[0]->isSpectator = true;
							printf("as spectator\n");
						}
						else
						{
							printf("as player\n");
						}
					}
					else if (strcmp(ri, lobby2text) == 0)
					{
						it[0]->currentLobby = lobbies[1];
						it[0]->currentLobby->users.push_back(*it[0]);
						printf(it[0]->address.ToString());
						printf(" connected to lobby ");
						printf("%d ", it[0]->currentLobby->lobbyNumber);
						if (it[0]->currentLobby->users.size() > 2)
						{
							it[0]->isSpectator = true;
							printf("as spectator\n");
						}
						else
						{
							printf("as player\n");
						}
					}
					else if (strcmp(ri, lobby3text) == 0)
					{
						it[0]->currentLobby = lobbies[2];
						it[0]->currentLobby->users.push_back(*it[0]);
						printf(it[0]->address.ToString());
						printf(" connected to lobby ");
						printf("%d ", it[0]->currentLobby->lobbyNumber);
						if (it[0]->currentLobby->users.size() > 2)
						{
							it[0]->isSpectator = true;
							printf("as spectator\n");
						}
						else
						{
							printf("as player\n");
						}
					}
					else if (strcmp(ri, lobby4text) == 0)
					{
						it[0]->currentLobby = lobbies[3];
						it[0]->currentLobby->users.push_back(*it[0]);
						printf(it[0]->address.ToString());
						printf(" connected to lobby ");
						printf("%d ", it[0]->currentLobby->lobbyNumber);
						if (it[0]->currentLobby->users.size() > 2)
						{
							it[0]->isSpectator = true;
							printf("as spectator\n");
						}
						else
						{
							printf("as player\n");
						}
					}
					else
					{
						printf("Invalid lobby number");
					}

					//Starts the lobby
					if (it[0]->currentLobby->users.size() >= 2)
					{
						it[0]->currentLobby->started = true;
						it[0]->currentLobby->users[0].turn = true;
						printf("Game started for lobby ");
						printf("%d\n", it[0]->currentLobby->lobbyNumber);

						RakNet::BitStream bsOut;

						for (size_t j = 0; j < it[0]->currentLobby->users.size(); j++)
						{
							bsOut.Write((RakNet::MessageID)ID_START_GAME);
							bsOut.Write(it[0]->currentLobby->gameboard);
							bsOut.Write(it[0]->currentLobby->users[j].turn);
							bsOut.Write(it[0]->currentLobby->users[j].isSpectator);
							peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, it[0]->currentLobby->users[j].address, false);
							bsOut.Reset();
						}
					}

					bufPtr = NULL;
				}
				break;
				case ID_MOVE:
				{
					gpro_mancala_index index;

					//read in move from client
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(index);

					//run game logic to handle move

				}
				break;
				default:
					bufPtr = NULL;
					break;
				}
			}
		}
		
		for (size_t i = 0; i < lobbies.size(); i++)
		{
			//Game Logic
			if (lobbies[i]->started)
			{

			}
		}
	}

	//shut down server
	RakNet::RakPeerInterface::DestroyInstance(peer);

	//close log file
	fclose(logFile);

	printf("\n\n");
	system("pause");
}
