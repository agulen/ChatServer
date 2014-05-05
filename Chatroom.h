#include <vector>
#include <string> 
#include "User.h"

#ifndef CHATROOM_H
#define CHATROOM_H

using namespace std;

class Chatroom
{
	public:
		//CONSTRUCTORS********************************************************************
		Chatroom();
		Chatroom(string mname);
		
		//SETTERS*************************************************************************
		void setName(string new_name);

		//GETTERS*************************************************************************
		string getName();
		vector<User> getUsers();

		//METHODS*************************************************************************
		void addUser(User u);
		bool removeUser(User u);

		//OPERATORS***********************************************************************
		bool operator==(Chatroom r);
		bool operator!=(Chatroom r);
	private:
		//VARIABLES***********************************************************************
		string name;
		vector<User> usersInRoom; 
};

#endif
