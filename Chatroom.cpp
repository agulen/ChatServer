#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include "Chatroom.h"
#include "User.h"

using namespace std;

//CONSTRUCTORS********************************************************************
Chatroom::Chatroom() { } 
Chatroom::Chatroom(string mname) { name = mname; }

//SETTERS*************************************************************************
void Chatroom::setName(string new_name) { name = new_name; }

//GETTERS*************************************************************************
string Chatroom::getName() { return name; }
vector<User> Chatroom::getUsers() { return usersInRoom; }

//METHODS*************************************************************************
void Chatroom::addUser(User u) { 
	usersInRoom.push_back(u); 
}

bool Chatroom::removeUser(User u) {
	vector<User>::iterator itr; 
	for(itr = usersInRoom.begin(); itr != usersInRoom.end(); itr++)
	{
		if(itr->getName() == u.getName())
		{
			usersInRoom.erase(itr);
			return true; 
		}
	}

	return false; 
}

//OPERATORS*********************************************************************
bool Chatroom::operator==(Chatroom r)
{
	if(name == r.getName())
		return true;
	else
		return false; 
}

bool Chatroom::operator!=(Chatroom r)
{
	return !(*this == r);
}
