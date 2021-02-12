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

#include "gpro-net/gpro-net.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"

#define MAX_CLIENTS 10
#define SERVER_PORT 60000

enum GameMessages
{
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1,
	ID_CHAT_MESSAGE,
	ID_USERNAME,
	ID_JOIN_USERNAME,
	ID_PRINT_CONNECTED_USERS,
	ID_SHUTDOWN
};

int main(int const argc, char const* const argv[])
{
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();

	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	peer->Startup(MAX_CLIENTS, &sd, 1);
	peer->SetOccasionalPing(true);
	RakNet::Packet* packet;

	std::vector<RakNet::RakString> connectedUsers;

	printf("Starting the server.\n");
	peer->SetMaximumIncomingConnections(MAX_CLIENTS);

	FILE* logFile = fopen("messageLog.txt", "w");
	if (logFile == NULL)
	{
		printf("Could not open file");
	}
	// ^ this saves in a weird place. users/[user]/remote/reponame/bin

	bool running = true;
	//Always running loop to receive packets
	while (running)
	{
		//For each packet the server receives
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			unsigned int bufIndex = 0;
			unsigned char bufPtr = packet->data[bufIndex];
			RakNet::BitStream bsIn(packet->data, packet->length, false);

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
					printf("A connection is incoming.\n");
					bufPtr = NULL;
					break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					printf("The server is full.\n");
					bufPtr = NULL;
					break;
				case ID_DISCONNECTION_NOTIFICATION:
					printf("A client has disconnected.\n");
					fclose(logFile);
					bufPtr = NULL;
					break;
				case ID_CONNECTION_LOST:
					printf("A client lost the connection.\n");
					fclose(logFile);
					bufPtr = NULL;
					break;
				case ID_GAME_MESSAGE_1:
				{
					RakNet::RakString rs;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);

					printf("%s\n", rs.C_String());
					//send message to log
					fprintf(logFile, "%s\n", rs.C_String());
					//fclose(logFile);

					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
					bsOut.Write("Welcome client!");
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);

					bufPtr = packet->data[bufIndex + sizeof(RakNet::MessageID) + 2 + rs.GetLength()];
					bufIndex += sizeof(RakNet::MessageID) + 2 + static_cast<int>(rs.GetLength());
				}
				break;
				case ID_CHAT_MESSAGE:
				{
					RakNet::RakString rs;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);

					printf("%s ", rs.C_String());
					fprintf(logFile, "%s ", rs.C_String());

					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_CHAT_MESSAGE);
					bsOut.Write(rs);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_RAKNET_GUID, true);

					bufPtr = packet->data[bufIndex + sizeof(RakNet::MessageID) + 2 + rs.GetLength()];
					bufIndex += sizeof(RakNet::MessageID) + 2 + static_cast<int>(rs.GetLength());
				}
				break;
				case ID_TIMESTAMP:
				{
					RakNet::Time ts;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(ts);

					printf("%" PRINTF_64_BIT_MODIFIER "u ", ts);
					//send timestamp to log (?)
					fprintf(logFile, "%" PRINTF_64_BIT_MODIFIER "u ", ts);

					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_TIMESTAMP);
					bsOut.Write(ts);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_RAKNET_GUID, true);

					bufPtr = packet->data[bufIndex + sizeof((RakNet::MessageID)ID_TIMESTAMP) + sizeof(RakNet::Time)];
					bufIndex += sizeof((RakNet::MessageID)ID_TIMESTAMP) + sizeof(RakNet::Time);
				}
				break;
				case ID_USERNAME:
				{
					RakNet::RakString rs;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);

					printf("%s ", rs.C_String());
					fprintf(logFile, "%s ", rs.C_String());

					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_USERNAME);
					bsOut.Write(rs);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_RAKNET_GUID, true);

					bufPtr = packet->data[bufIndex + sizeof(RakNet::MessageID) + 2 + rs.GetLength()];
					bufIndex += sizeof(RakNet::MessageID) + 2 + static_cast<int>(rs.GetLength());
				}
				break;
				case ID_SHUTDOWN:
				{
					running = false;
					bufPtr = NULL;
				}
				break;
				case ID_JOIN_USERNAME:
				{
					RakNet::RakString rs;

					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);

					connectedUsers.push_back(rs + " " + packet->systemAddress.ToString() + "\n");

					printf("%s ", rs.C_String());

					bufPtr = packet->data[bufIndex + sizeof(RakNet::MessageID) + 2 + rs.GetLength()];
					bufIndex += sizeof(RakNet::MessageID) + 2 + static_cast<int>(rs.GetLength());
				}
				break;
				case ID_PRINT_CONNECTED_USERS:
				{
					RakNet::RakString users = "Users: \n";
					for (int i = 0; i < connectedUsers.size(); i++)
					{
						users += connectedUsers.at(i) + " ";
						printf("%s ", users.C_String());
					}

					users += "\n";

					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_PRINT_CONNECTED_USERS);
					bsOut.Write(users);
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);

					bufPtr = NULL;
				}
				break;
				default:
					//printf("\nMessage with identifier %i has arrived.\n", bufPtr);
					bufPtr = NULL;
					break;
				}
			}
			//printf("End of packet.\n");
		}
	}

	RakNet::RakPeerInterface::DestroyInstance(peer);

	fclose(logFile);

	printf("\n\n");
	//system("pause");
}
