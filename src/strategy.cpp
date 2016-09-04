/*
 * This file is part of the VSS-SampleStrategy project.
 *
 * This Source Code Form is subject to the terms of the GNU GENERAL PUBLIC LICENSE,
 * v. 3.0. If a copy of the GPL was not distributed with this
 * file, You can obtain one at http://www.gnu.org/licenses/gpl-3.0/.
 */

#include "strategy.h"

Strategy::Strategy(){
	its_real_transmition = false;
	has_new_state = has_new_command = false;
	robot_radius = 8.0;
	distance_stop = 5.0;
	situation = 0;
	srand(time(NULL));
	goal_glob.x = (rand() % 110) + 20;
	goal_glob.y = (rand() % 100) + 20;
}

void Strategy::init(string main_color){
	this->main_color = main_color;

	thread_comm = new thread(bind(&Strategy::comm_thread, this));
	//thread_send = new thread(bind(&Strategy::send_thread, this));

	thread_comm->join();
	//thread_send->join();
}

void Strategy::comm_thread(){
	interface_receive.createSocketReceiveState(&global_state);

	if(main_color == "yellow"){
		interface_send.createSendCommandsTeam1(&global_commands);
	}else{
		interface_send.createSendCommandsTeam2(&global_commands);
	}

	while(true){
		// Only loop if has a new state
		interface_receive.receiveState();
		state = common::Global_State2State(global_state, main_color);
		situation = global_state.situation();
		has_new_state = true;

		calc_strategy();
		has_new_state = false;
		if(main_color == "yellow"){
			interface_send.sendCommandTeam1();
		}else{
			interface_send.sendCommandTeam2();
		}	
	}
}

void Strategy::calc_strategy(){
	// Padrão, não mudar nada
	bool all_robots_ok = false;
	global_commands = vss_command::Global_Commands();
	global_commands.set_situation(NONE); 
	if(main_color == "yellow"){
		global_commands.set_is_team_yellow(true);	// IF IS YELLOW: TRUE
	}else{
		global_commands.set_is_team_yellow(false);
	}

	// Calcula o comando dos rodas 
	commands[0] = calc_cmd_to(state.robots[0].pose, state.ball, distance_stop);
	//commands[0] = calc_cmd_to(state.robots[0].pose, goal_glob, distance_stop);
	//commands[0] = calc_cmd_to(state.robots[0].pose, goal_glob, distance_stop);

	//Padrão, não mudar
	for(int i = 0 ; i < 3 ; i++){
		vss_command::Robot_Command *robot = global_commands.add_robot_commands();
		robot->set_id(i);
		robot->set_left_vel(commands[i].left);
		robot->set_right_vel(commands[i].right);
	}
}

common::Command Strategy::calc_cmd_to(btVector3 act, btVector3 goal, float distance_to_stop){
	Command cmd;
	float distance_robot_goal;
	float angulation_robot_goal;
	float angulation_robot_robot_goal;

	// Diferença entre angulação do robô e do objetivo
	distance_robot_goal = distancePoint(goal, act);
	angulation_robot_goal = angulation(goal, act);
	angulation_robot_goal -= 180; // 180 if comes from VSS-Simulator
    if(angulation_robot_goal < 0){
    	angulation_robot_goal += 360;
    }
	int angles = (int)fabs(act.z - angulation_robot_goal) % 360;
	int dist = angles > 180 ? 360 - angles : angles*-1;
	angulation_robot_robot_goal = dist;

	// Regras de movimentação
	if(fabs(angulation_robot_robot_goal) <= 135){
		cmd.left = distance_robot_goal + 0.2*(angulation_robot_robot_goal * robot_radius / 2.00);
		cmd.right = distance_robot_goal - 0.2*(angulation_robot_robot_goal * robot_radius / 2.00);
		
		cmd.left *= 0.3;
		cmd.right *= 0.3;
	}else{
		if(angulation_robot_robot_goal >= 0){
			cmd.left = 50;
			cmd.right = -50;
		}else{
			cmd.left = -50;
			cmd.right = 50;
		}
	}

	//cmd.left e cmd.right são PWM (0 a 255 para frente) (256 á 252 para trás)

	if(distance_robot_goal < 15.0){
		cmd.left = 0;
		cmd.right = 0;
	}

	return cmd;
}