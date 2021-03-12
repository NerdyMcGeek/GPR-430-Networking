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

	gpro-net-RakNet-Server.cpp
	Source for RakNet server management.
*/

#include "gpro-net/gpro-net-server/gpro-net-RakNet-Server.hpp"
#include <vector>


namespace gproNet
{
	std::vector<RakNet::SystemAddress*> gameServerList;

	cRakNetServer::cRakNetServer()
	{
		RakNet::SocketDescriptor sd(SET_GPRO_SERVER_PORT, 0);
		unsigned short MAX_CLIENTS = 10;

		peer->Startup(MAX_CLIENTS, &sd, 1);
		peer->SetMaximumIncomingConnections(MAX_CLIENTS);

		gameServerList.push_back(new RakNet::SystemAddress());
		gameServerList.push_back(new RakNet::SystemAddress());
		gameServerList.push_back(new RakNet::SystemAddress());
		gameServerList.push_back(new RakNet::SystemAddress());
		gameServerList.push_back(new RakNet::SystemAddress());
	}

	cRakNetServer::~cRakNetServer()
	{
		peer->Shutdown(0);
	}

	bool cRakNetServer::ProcessMessage(RakNet::BitStream& bitstream, RakNet::SystemAddress const sender, RakNet::Time const dtSendToReceive, RakNet::MessageID const msgID)
	{
		if (cRakNetManager::ProcessMessage(bitstream, sender, dtSendToReceive, msgID))
			return true;

		// server-specific messages
		switch (msgID)
		{
		case ID_NEW_INCOMING_CONNECTION:
			//printf("A connection is incoming.\n");
			return true;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			//printf("The server is full.\n");
			return true;
		case ID_DISCONNECTION_NOTIFICATION:
			//printf("A client has disconnected.\n");
			return true;
		case ID_CONNECTION_LOST:
			//printf("A client lost the connection.\n");
			return true;

			// test message
		case ID_GPRO_MESSAGE_COMMON_BEGIN:
		{
			// server receives greeting, print it and send one back
			RakNet::BitStream bitstream_w;
			RakNet::BitStream serverList;
			ReadTest(bitstream);
			WriteTest(bitstream_w, "Hello client from server");
			peer->Send(&bitstream_w, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);
			Write((RakNet::MessageID)ID_GPRO_MESSAGE_SERVER_LIST, serverList, "Enter server number to join:");
			peer->Send(&serverList, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);
		}	return true;

		case ID_GPRO_MESSAGE_SERVER_ADDRESS_REQUEST:
		{
			//read server number from bitstream
			int serverNum = GetInt(bitstream);
			
			//locate corresonding server address
			RakNet::SystemAddress gameServerAddress = *gameServerList[serverNum - 1];

			//send requested server address to client
			RakNet::BitStream bitstream_w; //NOTE: This is where I stopped when I ran out of time

		}	return true;

		}
		return false;
	}
}