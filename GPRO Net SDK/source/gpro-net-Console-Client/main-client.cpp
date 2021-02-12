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
	GPRO Net Project 1
	By Alexander Wood and Avery Follett
*/

#include "gpro-net/gpro-net.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/GetTime.h"

#define SERVER_PORT 60000

const int MAX_MESSAGE_SZ = 256;

enum GameMessages
{
	ID_GAME_MESSAGE_1=ID_USER_PACKET_ENUM+1,
	ID_CHAT_MESSAGE,
	ID_USERNAME,
	ID_JOIN_USERNAME,
	ID_PRINT_CONNECTED_USERS,
	ID_SHUTDOWN
};

int main(int const argc, char const* const argv[])
{
	char str[512];
	char un[512];
	char shutdown[MAX_MESSAGE_SZ] = "quitServer\n";
	char clientList[MAX_MESSAGE_SZ] = "clientList\n";

	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	RakNet::SystemAddress sysAddress;

	RakNet::SocketDescriptor sd;
	peer->Startup(1, &sd, 1);
	peer->SetOccasionalPing(true);

	printf("Enter server IP or hit enter for 127.0.0.1\n");
	gets_s(str);
	if (str[0] == 0) {
		strcpy(str, "172.16.2.186");
	}

	printf("Enter a username. No spaces\n");
	gets_s(un);

	printf("Connecting...\n");
	peer->Connect(str, SERVER_PORT, 0, 0);

	bool running = true;

	while (running)
	{
		if (GetKeyState(VK_SPACE) & 0x8000)
		{
			printf("Enter message:\n");
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
				//running = false;
			}
			else if (strcmp(message, clientList) == 0)
			{
				bsOut.Write((RakNet::MessageID)ID_PRINT_CONNECTED_USERS);
			}
			else
			{
				bsOut.Write((RakNet::MessageID)ID_TIMESTAMP);
				bsOut.Write(timestamp);

				bsOut.Write((RakNet::MessageID)ID_USERNAME);
				bsOut.Write(username);

				bsOut.Write((RakNet::MessageID)ID_CHAT_MESSAGE);
				bsOut.Write(chatMessage);
			}

			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, sysAddress, false);
		}

		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			unsigned int bufIndex = 0;
			unsigned char bufPtr = packet->data[bufIndex];
			RakNet::BitStream bsIn(packet->data, packet->length, false);

			while (bufPtr != NULL)
			{
				switch (packet->data[0])
				{
				// Handles initial connection to server
				case ID_CONNECTION_REQUEST_ACCEPTED:
				{
					printf("Our connection request has been accepted.\n");

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

					printf("\n%" PRINTF_64_BIT_MODIFIER "u ", ts);

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

					bufPtr = NULL;
				}
				break;
				default:
					printf("Message with identifier %i has arrived.\n", packet->data[0]);
					break;
				}
			}
		}
	}

	RakNet::RakPeerInterface::DestroyInstance(peer);

	printf("\n\n");
	system("pause");
}
