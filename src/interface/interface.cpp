/*
 * This file is part of the VSS-Vision project.
 *
 * This Source Code Form is subject to the terms of the GNU GENERAL PUBLIC LICENSE,
 * v. 3.0. If a copy of the GPL was not distributed with this
 * file, You can obtain one at http://www.gnu.org/licenses/gpl-3.0/.
 */

#include "interface.h"

Interface::Interface(){

}

void Interface::createLoopSendState(vss_state::Global_State *global_state){
	this->global_state = global_state;	// assim não há a necessidade de atualizar "setAlgo" o dado toda hora

	zmq::context_t context (1);
	zmq::socket_t socket (context, ZMQ_PUB);

	std::cout << "Connecting to server…" << std::endl;
	socket.bind("tcp://*:5555");

	while (true) {
		usleep (100000);

		std::string msg_str;
	    global_state->SerializeToString(&msg_str);

	    zmq::message_t request (msg_str.size());
	    memcpy ((void *) request.data (), msg_str.c_str(), msg_str.size());
	    std::cout << "Sending State data ..." << std::endl;
	    socket.send (request);
	}
}

void Interface::createLoopReceiveState(vss_state::Global_State *global_state){
	this->global_state = global_state;	// assim não há a necessidade de atualizar "setAlgo" o dado toda hora
	
	zmq::context_t context(1);
	const char * protocol =
	"tcp://localhost:5555";
	//  Socket to talk to server
	std::cout << "Connecting to server…" << std::endl;
	zmq::socket_t socket (context, ZMQ_SUB);
	//  sock.bind("epgm://eth0;239.192.1.1:5556");
	socket.connect(protocol);
	socket.setsockopt (ZMQ_SUBSCRIBE, "", 0);

	int request_nbr;
	while(true){
		zmq::message_t request;
        socket.recv (&request, 0); //  Wait for next request from client
        std::cout << "Received" << std::endl;
        std::string msg_str(static_cast<char*>(request.data()), request.size());
        global_state->ParseFromString(msg_str);
        printState();
	}
	socket.close();
}

void Interface::createLoopSendCommands(vss_command::Global_Commands*){

}

void Interface::createLoopReceiveCommands(vss_command::Global_Commands*){

}

void Interface::printState(){
	std::string text_str;
    google::protobuf::TextFormat::PrintToString(*global_state, &text_str);
    std::cout << text_str << std::endl;
}

void Interface::printCommand(){

}