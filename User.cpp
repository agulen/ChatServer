#include "User.h"
#include <string>

using namespace std;

//CONSTRUCTORS********************************************************************
User::User() { }
User::User(string mname, string mpassword) { name = mname; password = mpassword; }
User::User(string mname, string mpassword, bool mloggedIn, int mfd) 
{
	name = mname;
	password = mpassword;
	loggedIn = mloggedIn;
	fd = mfd; 
}

//SETTERS*************************************************************************
void User::setName(string new_name) { name = new_name; }
void User::setPassword(string new_password) { password = new_password; }
void User::setLoggedIn(bool new_loggedIn) { loggedIn = new_loggedIn; }
void User::setfd(int newfd) { fd = newfd; }

//GETTERS*************************************************************************
string User::getName() { return name; }
string User::getPassword() { return password; }
int User::getfd() { return fd; }
bool User::isLoggedIn() { return loggedIn; }


//METHODS*************************************************************************
bool User::legalName() {
	if(name.find(" ") == name.npos)
		return true;
	else
		return false;
}
