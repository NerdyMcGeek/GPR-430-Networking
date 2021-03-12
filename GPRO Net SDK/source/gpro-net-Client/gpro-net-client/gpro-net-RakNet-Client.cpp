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

	gpro-net-RakNet-Client.cpp
	Source for RakNet client management.
*/

#include "gpro-net/gpro-net-client/gpro-net-RakNet-Client.hpp"
#include <iostream>

namespace gproNet
{


	cRakNetClient::cRakNetClient()
	{
		RakNet::SocketDescriptor sd;
		char SERVER_IP[16] = "172.16.2.56";

		peer->Startup(1, &sd, 1);
		peer->SetMaximumIncomingConnections(0);
		peer->Connect(SERVER_IP, SET_GPRO_SERVER_PORT, 0, 0);
	}

	cRakNetClient::~cRakNetClient()
	{
		peer->Shutdown(0);
	}

	bool cRakNetClient::ProcessMessage(RakNet::BitStream& bitstream, RakNet::SystemAddress const sender, RakNet::Time const dtSendToReceive, RakNet::MessageID const msgID)
	{
		if (cRakNetManager::ProcessMessage(bitstream, sender, dtSendToReceive, msgID))
			return true;

		// client-specific messages
		switch (msgID)
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			//printf("Another client has disconnected.\n");
			break;
		case ID_REMOTE_CONNECTION_LOST:
			//printf("Another client has lost the connection.\n");
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			//printf("Another client has connected.\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			//printf("The server is full.\n");
			return true;
		case ID_DISCONNECTION_NOTIFICATION:
			//printf("We have been disconnected.\n");
			return true;
		case ID_CONNECTION_LOST:
			//printf("Connection lost.\n");
			return true;

		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
			// client connects to server, send greeting
			RakNet::BitStream bitstream_w;
			WriteTest(bitstream_w, "Hello server from client");
			peer->Send(&bitstream_w, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);
		}	return true;

			// test message
		case ID_GPRO_MESSAGE_COMMON_BEGIN:
		{
			// client receives greeting, just print it
			ReadTest(bitstream);
		}	return true;

		case ID_GPRO_MESSAGE_SERVER_LIST:
		{
			//read server number request from server
			ReadAndPrint(bitstream);

			//get server number the client wants to connect to
			int serverNum;
			std::cin >> serverNum;

			//send server number to server to get corresponding address
			RakNet::BitStream bitstream_w;
			Write(ID_GPRO_MESSAGE_SERVER_ADDRESS_REQUEST, bitstream_w, serverNum);
			peer->Send(&bitstream_w, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);

			
		}	return true;
		}
		return false;
	}
}