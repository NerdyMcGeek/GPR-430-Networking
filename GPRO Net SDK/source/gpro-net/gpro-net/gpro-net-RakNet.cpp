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

	gpro-net-RakNet.cpp
	Source for RakNet common management.
*/

#include "gpro-net/gpro-net/gpro-net-RakNet.hpp"


namespace gproNet
{
	cRakNetManager::cRakNetManager()
		: peer(RakNet::RakPeerInterface::GetInstance())
	{
	}

	cRakNetManager::~cRakNetManager()
	{
		RakNet::RakPeerInterface::DestroyInstance(peer);
	}

	bool cRakNetManager::ProcessMessage(RakNet::BitStream& bitstream, RakNet::SystemAddress const sender, RakNet::Time const dtSendToReceive, RakNet::MessageID const msgID)
	{
		// process messages that can be handled the same way for all types of peers
		//switch (msgID)
		//{
		//}
		return false;
	}

	RakNet::BitStream& cRakNetManager::WriteTimestamp(RakNet::BitStream& bitstream)
	{
		bitstream.Write((RakNet::MessageID)ID_TIMESTAMP);
		bitstream.Write((RakNet::Time)RakNet::GetTime());
		return bitstream;
	}

	RakNet::BitStream& cRakNetManager::ReadTimestamp(RakNet::BitStream& bitstream, RakNet::Time& dtSendToReceive_out, RakNet::MessageID& msgID_out)
	{
		RakNet::Time tSend = 0, tReceive = 0;
		if (msgID_out == ID_TIMESTAMP)
		{
			tReceive = RakNet::GetTime();
			bitstream.Read(tSend);
			bitstream.Read(msgID_out);
			dtSendToReceive_out = (tReceive - tSend);
		}
		return bitstream;
	}

	RakNet::BitStream& cRakNetManager::WriteTest(RakNet::BitStream& bitstream, char const message[])
	{
		WriteTimestamp(bitstream);
		bitstream.Write((RakNet::MessageID)ID_GPRO_MESSAGE_COMMON_BEGIN);
		bitstream.Write(message);
		return bitstream;
	}

	RakNet::BitStream& cRakNetManager::ReadTest(RakNet::BitStream& bitstream)
	{
		RakNet::RakString rs;
		bitstream.Read(rs);
		printf("%s\n", rs.C_String());
		return bitstream;
	}

	RakNet::BitStream& cRakNetManager::Write(RakNet::MessageID msgID, RakNet::BitStream& bitstream, char const message[])
	{
		WriteTimestamp(bitstream);
		bitstream.Write(msgID);
		bitstream.Write(message);
		return bitstream;
	}

	RakNet::BitStream& cRakNetManager::Write(RakNet::MessageID msgID, RakNet::BitStream& bitstream, int const message)
	{
		WriteTimestamp(bitstream);
		bitstream.Write(msgID);
		bitstream.Write(message);
		return bitstream;
	}

	RakNet::RakString cRakNetManager::Read(RakNet::BitStream& bitstream)
	{
		RakNet::RakString rs;
		bitstream.Read(rs);
		return rs;
	}

	int cRakNetManager::GetInt(RakNet::BitStream& bitstream)
	{
		int rs;
		bitstream.Read(rs);
		return rs;
	}

	RakNet::BitStream& cRakNetManager::ReadAndPrint(RakNet::BitStream& bitstream)
	{
		RakNet::RakString rs;
		bitstream.Read(rs);
		printf("%s\n", rs.C_String());
		return bitstream;
	}

	int cRakNetManager::MessageLoop()
	{
		int count = 0;
		RakNet::Packet* packet = 0;
		RakNet::MessageID msgID = 0;
		RakNet::Time dtSendToReceive = 0;

		while (packet = peer->Receive())
		{
			RakNet::BitStream bitstream(packet->data, packet->length, false);
			bitstream.Read(msgID);

			// process timestamp
			ReadTimestamp(bitstream, dtSendToReceive, msgID);

			// process content
			if (ProcessMessage(bitstream, packet->systemAddress, dtSendToReceive, msgID))
				++count;

			// done with packet
			peer->DeallocatePacket(packet);
		}

		// done
		return count;
	}
}