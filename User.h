#ifndef USER_H
#define USER_H

#include <string>
using namespace std; 

class User
{
	public:
		//CONSTRUCTORS********************************************************************
		User();
		User(string mname, string mpassword);
		User(string mname, string mpassword, bool mloggedIn, int mfd);
		
		//SETTERS*************************************************************************
		void setName(string new_name);
		void setPassword(string new_password);
		void setLoggedIn(bool new_loggedIn);
		void setfd(int newfd);

		//GETTERS*************************************************************************
		string getName();
		string getPassword();
		int getfd(); 
		bool isLoggedIn(); 

		//METHODS*************************************************************************
		bool legalName();
	private:
		//VARIABLES***********************************************************************
		string name;
		string password; 
		bool loggedIn; 
		int fd;  
};

#endif
