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

	main-server.cpp
	Main source for console server application.
*/

#include "gpro-net/gpro-net-server/gpro-net-RakNet-Server.hpp"


//set up messages that the server will recieve

//keep track of "sub-servers" for games
list <server> subServers;



//this will be similar to the setup for project 2 with the server and the lobbies I think
int main(int const argc, char const* const argv[])
{
	gproNet::cRakNetServer server;
	//start up master server

	//set up any default "sub-servers" (those server ip should somehow be stored here so that the client doesn't need to know it)
	//add to list of "sub-servers"
	subServers = new list<server>;

	//while server is running
	while (1)
	{
		//get packet bitstream
		//read messages and then select correct response
		server.MessageLoop();
		//Message::Client connects
			//server sends welcome message
			//client gives username
			//server sends list of "sub-servers"
			//client can choose a sub server to connect to
			//connect that client to the server they have selected (once they are connected to that, the indiviual sub-server should run it's own game logic, that is not handled here)

		//Message::Client reconnects (is sent back after a game ends)
			//client username is (hopefully) saved
			//client recieves welcome back message
			//same as before, server sends current list of sub-servers
			//client can choose same as before
			//client connects same as before

		//Message::Sub-Server set up
			//Add a new sub-server to list

		//Message::Sub-Server shut down
			//remove the sub-server from the list
			

	}

	printf("\n\n");
	system("pause");
}
