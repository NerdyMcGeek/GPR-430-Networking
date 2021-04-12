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

	gpro-net-client.c
	Main source for client framework.
*/

// Description of spatial pose.
struct sSpatialPose
{
    float scale[3];     // non-uniform scale
    float rotate[3];    // orientation as Euler angles
    float translate[3]; // translation

    // read from stream
    RakNet::BitStream& Read(RakNet::BitStream& bitstream)
    {
        //data will be coming in here already compressed from another client
        //means we need to decompress rotation data here
            //undo what we did in compressing the data below
            //decompress the 2 numbers we've recieved
            //recreate the 3rd number that we don't have
        //psuedocode:
        //read in the recieved data
        //a = a'/511*sqrt(2)
        float newRotate1 = bitstream.Read(rotate[0]) / (511 * sqrt(2));
        float newRotate2 = bitstream.Read(rotate[1]) / (511 * sqrt(2));
        float newRotate3 = //I cant remember the math, but you want to calculate the third value using the 2 you've already recieved


        bitstream.Read(scale[0]);
        bitstream.Read(scale[1]);
        bitstream.Read(scale[2]);

        bitstream.Read(translate[0]);
        bitstream.Read(translate[1]);
        bitstream.Read(translate[2]);
        return bitstream;
    }

    // write to stream
    RakNet::BitStream& Write(RakNet::BitStream& bitstream) const
    {
        //writing out data to send to server/another client will go
        //data will need to be compressed here
            //we can at least compress roatation data since we know that it will be a unit vector (? possibly wrong term, but means we will know that it has the value of 1)
            //to compress rotation data we will remove/ignore the largest piece of the data (this is because we can reconstruct it on the other side given the other rotation values)
            //the remaining two pieces of data can be reduced to a smaller size. this will mean the function will not send as much data across the network and will free up bandwith
        //psuedocode:
        //check to see which has the largest value
        if (rotate[0] > rotate[1] && rotate[0] > rotate[2])
        {
            //compress the first rotation float
            //a' = a*sqrt(2) * 511   <- I think this was the equation for compressing the data we had in class (might be incorrect but similar idea)
            newRotate1 = rotate[1] * sqrt(2) * 511;
            //compress the second rotation float
            newRotate2 = rotate[2] * sqrt(2) * 511;

            //bitshift the data into a smaller structure, something like the size of an int instead

        }
        else if //continue checking with the other values in rotate and use adjust accordingly to wich one is the larges and getting ignored

        bitstream.Write(scale[0]);
        bitstream.Write(scale[1]);
        bitstream.Write(scale[2]);
        bitstream.Write(newRotate1); //replace this roatation data with the new value after compression
        bitstream.Write(newRotate2);
        
        bitstream.Write(translate[0]);
        bitstream.Write(translate[1]);
        bitstream.Write(translate[2]);
        return bitstream;
    }
};

