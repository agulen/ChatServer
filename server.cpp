//Altan Gulen
//Network Programming
//Code for using network sockets borrowed from beej's guide to network programming: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html

#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <errno.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include "User.h"
#include "Chatroom.h"

#define PORT "9999"
#define MESSAGE_SIZE 80
#define MAXBUF 200
#define MAXCON 500

vector<string> splitString(string str);
char* convertToChar(string str);
Chatroom findRoom(string roomName, vector<Chatroom> &roomList);
User getClientUsername(map<int, User> &userSockets, int user_fd);
void addNewUser(User &newUser, int user_fd, vector<User> &userList, map<int, User> &userSockets);
void *get_in_addr(struct sockaddr *sa);


int main(int argc, char* argv[])
{
	vector<User> userList;
	vector<Chatroom> roomList; 
	map<int, User> userSockets;
	//bool verboseMode = false; 

	fd_set master;
	fd_set clients; 
	int fdmax; 

	int listener;	//socket to listen to
	int newfd;		//new socket from accept
	struct sockaddr_storage remoteaddr; 
    socklen_t addrlen;
    int yes; 

	struct addrinfo hints, *ai, *p;
	char remoteIP[INET6_ADDRSTRLEN];
	int rv;	

    FD_ZERO(&master);    
    FD_ZERO(&clients);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM; 
	hints.ai_flags = AI_PASSIVE; 	//Use my IP

	if(argc == 2 && (strcmp(argv[1], "-v") == 0))	
		//verboseMode = true; 
	

	if((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1; 
	}

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // removes the wait to restart the server
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) // bind to address and port
        { 
            close(listener);
            continue;
        }

        printf("listening on %s\n", 
            inet_ntop(p->ai_family, 
                get_in_addr((struct sockaddr*)&(p->ai_addr)), remoteIP, INET6_ADDRSTRLEN)
            );
        break;
    }

	if(p == NULL)
	{
		fprintf(stderr, "server:failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai);
	
	if(listen(listener, 10) == -1)
	{
		perror("listen");
		exit(3);
	}

	FD_SET(listener, &master); //Adds listener to master fdset
    fdmax = listener; 

    //Server listening loop
	while(1)
	{
		char* buf = convertToChar("DEFAULT VALUE");  
		string bufString = ""; 
		string command; 
		string response; 
		ssize_t numBytes = -2;			

		clients = master; 
		if(select(fdmax + 1, &clients, NULL, NULL, NULL) == -1)
		{
			perror("select:");
			exit(1); 
		}
		for(int i = 0; i <= fdmax; i++)
		{
			if(FD_ISSET(i, &clients))
			{
				if(i == listener)
				{
					//A client wants to talk!
					addrlen = sizeof remoteaddr;
					newfd = accept(listener,	//accept this socket
						(struct sockaddr *)&remoteaddr, //who are you?
						&addrlen);	

					if(newfd == -1)	
					{
						perror("accept");
					}
					else
					{
						FD_SET(newfd, &master); //add new socket to master list
						if(newfd > fdmax)  //update length of fd set
						{
							fdmax = newfd;
						}
					}
				}
				else
				{
					User currentUser;
					//we're talking to a client
					if((numBytes = recv(i, buf, MAXBUF, 0)) <= 0)
					{
						if(numBytes == 0) {} //Socket is closed

						else //Error occurred
						{
							perror("recv");
						}
						close(i); //close socket
						FD_CLR(i, &master); //delete from master list
					}
					else  //Got good data from client
					{
						bufString += buf;

						if(bufString.find("CHAT") != string::npos) //Connecting to server
						{
							response = "TAHC\n";
							if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
							{
								perror("send (CHAT)");
								exit(1);
							}
						}
						else if(bufString.find("USERS") != string::npos) //List all users						
						{
							if(((currentUser = getClientUsername(userSockets, i)).getName()).empty())
							{
								break;	//Client has no username, do not process command
							}
							if(currentUser.isLoggedIn())
							{								
								cout << "I AM " << currentUser.getName();
								response = "List of users on server:\n";
								for(unsigned int j = 0; j < userList.size(); j++)								
									response += "\t" + userList[j].getName() + "\n";
								
							    if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
								{
									perror("send (USERS)");
									exit(1);
								}
							}
						}
						else if(bufString.find("USER") != string::npos) //<nickname> <password> --> Log in. Give 3 tries. if new user, create it.
						{
							if(fdmax > 1)
							{
								vector<string> tokens = splitString(bufString);
								string name = tokens[1];
								string givenPassword = tokens[2]; 
								response = "";

								for(unsigned int j = 0; j < userList.size(); j++)
								{
									if(userList[j].getName() == name)
									{
										response = "That username is taken.\n";										
										if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
										{
											perror("send (USER - username taken)");
											exit(1);
										}
									}
								}
								
								if(userList.size() == 0) //This is our first user!
								{	
									User newUser(tokens[1], tokens[2], true, i);
									addNewUser(newUser, i, userList, userSockets);
									response = "Welcome to the chat server "; 
									response += name;
									response += "!\n";
									if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
									{
										perror("send (USER - welcome to server)");
										exit(1);
									}
									response = "";								
								}
								else
								{
									for(unsigned int j = 0; j < userList.size(); j++) //Loop through all users
									{
										if(userList[j].getName() == name) //Check if user exists
										{											
											if(userList[j].getPassword() != givenPassword) //Check if passwords match or not
											{	
												if(!userList[j].isLoggedIn())
												{
													//User gets 2 more tries to get correct password
													for(int k = 1; k < 3; k++)
													{
														response += "Username recognized, but incorrect password given.\nEnter a password:";
														if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
														{
															perror("send (USER - password attempt)");
															exit(1);
														} 
														while(1) //Wait for password input
														{
															if((numBytes = recv(i, buf, MAXBUF, 0)) <= 0)
															{
																perror("recv (USER - password attempt)");
																exit(1); 
															}
															else //Got new password input!
															{
																givenPassword = buf; 
																break;
															}

														}												 
														
														if(givenPassword == userList[i].getPassword())
														{
															if(userList[i].isLoggedIn()) //Check if user is already logged in
															{
																response = "You are already logged in as ";
																response += (userSockets.find(i)->second).getName();
																response += "\n";

																if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
																{
																	perror("send (USER - already logged in)");
																	exit(1);
																}
															}
															else
																userList[i].setLoggedIn(true);
															break;  //Correct password given, get out of password attempt loop
														}
														else
														{
															if(k == 1) //If the user has failed to login on their third try, then disconnect
															{
																response = "You have reached the maximum amount of attempts to login. Disconnecting.\n";
																if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
																{
																	perror("send (USER - max attempts reached");
																	exit(1);
																} 
																close(i);
																FD_CLR(i, &master); //delete from master list
															}
														}
													}						
												}												
											}
											else
											{
												if(userList[i].isLoggedIn()) //Check if user is already logged in
												{													
													response = "You are already logged in as ";
													response += (userSockets.find(i)->second).getName();
													response += "\n";

													if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
													{
														perror("send (USER - already logged in)");
														exit(1);
													}												
												}	
											}
										}
										else //User doesn't exist, create it
										{											
											User newUser(tokens[1], tokens[2], true, i);
											addNewUser(newUser, i, userList, userSockets);

											response = "Welcome to the chat server "; 
											response += name;
											response += "!\n";
											if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
											{
												perror("send (USER - welcome to server)");
												exit(1);
											}	
											break;
										}
									}
								}	
							}	

							cout << "Num users:" << userList.size() << endl; 						
						}
						else if(bufString.find("ROOMS") != string::npos) //List all rooms
						{
							if(((currentUser = getClientUsername(userSockets, i)).getName()).empty())
							{
								break;	//Client has no username, do not process command
							}
							if(currentUser.isLoggedIn())
							{								
								response = "List of rooms on server:\n";								
								
								for(unsigned int i = 0; i < roomList.size(); i++)								
									response += "\t" + roomList[i].getName() + "\n";
							}
							if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
							{
								perror("send (ROOMS)");
								exit(1);
							}
						}
						else if(bufString.find("JOIN") != string::npos) //<room> --> join room. if doesn't exist, create it.
						{
							if(((currentUser = getClientUsername(userSockets, i)).getName()).empty())
							{
								break;	//Client has no username, do not process command
							}
							if(currentUser.isLoggedIn())
							{
								vector<string> tokens = splitString(bufString);
								string roomName = tokens[1];

								if(roomName[0] != '@')
								{
									response = "All room names must start with the '@' symbol.\n";
									response += "Type another command\n";
									if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
									{
										perror("send (JOIN - illegal roomname)");
										exit(1);
									}
								}
								else
								{
									Chatroom room = findRoom(roomName, roomList); 
									
									if(room.getName() != "")
									{
										room.addUser(currentUser);
										response = currentUser.getName() + " has joined " + room.getName() + ".\n";
										if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
										{
											perror("send (JOIN - joining existing room)");
											exit(1);
										}
									}
									else //Create new room
									{
										Chatroom newRoom(roomName);
										roomList.push_back(newRoom);
										newRoom.addUser(currentUser);
										response = newRoom.getName() + " created. You are the first user.\n";
										if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
										{
											perror("send (JOIN - creating new room)");
											exit(1);
										}
									}	
								}
							}			
						}
						else if(bufString.find("PART") != string::npos) //<room> --> leave room
						{
							if(((currentUser = getClientUsername(userSockets, i)).getName()).empty())
							{
								break;	//Client has no username, do not process command
							}
							if(currentUser.isLoggedIn())
							{
								vector<string> tokens = splitString(bufString);
								string roomName = tokens[1];	
								if(roomName[0] != '@')
								{
									response = "All room names must start with the '@' symbol.\n";
									response += "Type another command\n";
									if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
									{
										perror("send (PART - illegal roomname)");
										exit(1);
									}
								}
								else
								{
									Chatroom room = findRoom(roomName, roomList); 
									
									if(!room.getName().empty())
									{
										room.removeUser(currentUser);
										response = currentUser.getName() + " has left " + room.getName() + ".\n";
										response += "No more messages will be received from this room.\n";
										if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
										{
											perror("send (JOIN - joining existing room)");
											exit(1);
										}
									}	
								}
							}
						}
						else if(bufString.find("LIST") != string::npos) //<room> --> list users in specified room
						{
							if(((currentUser = getClientUsername(userSockets, i)).getName()).empty())
							{
								break;	//Client has no username, do not process command
							}
							if(currentUser.isLoggedIn())
							{
								vector<string> tokens = splitString(bufString);
								string roomName = tokens[1];	
								if(roomName[0] != '@')
								{
									response = "All room names must start with the '@' symbol.\n";
									response += "Type another command\n";
									if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
									{
										perror("send (PART - illegal roomname)");
										exit(1);
									}
								}
								else
								{
									Chatroom room = findRoom(roomName, roomList); 
									
									if(!room.getName().empty())
									{
										if(room.getUsers().size() == 0)
										{
											response = "No users in " + room.getName() + ".\n";
											if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
											{
												perror("send (PART - illegal roomname)");
												exit(1);
											}
										}
										else
										{
											response = "List of users in " + room.getName() + ":\n";		
											for(unsigned int i = 0; i < room.getUsers().size(); i++)
												response += "\t" + room.getUsers()[i].getName() + "\n";											

										}
									}	
								}		
							}					
						}
						else if(bufString.find("SAY") != string::npos) //<target> <length> <msg> --> target can be user or room.
						{
							if(((currentUser = getClientUsername(userSockets, i)).getName()).empty())
							{
								break;	//Client has no username, do not process command
							}
							if(currentUser.isLoggedIn())
							{
								vector<string> tokens = splitString(bufString);
								string target = tokens[1];								
								string message = tokens[3];
								for(unsigned int j = 3; j < tokens.size(); j++)	
								{
									message += tokens[j];
								}							
								message += "\n";
								
								int length = message.size();
								response = "";	

								//Check target exists
								if(target[0] == '@') //Is target a chatroom?
								{
									bool foundRoom = false; 
									for (unsigned int j = 0; j < roomList.size(); ++j) //Loop through rooms
									{
										if(target == roomList[j].getName()) 
										{
											foundRoom = true;

											//Loop through users in current room - send message to all of them
											for(unsigned int k = 0; k < roomList[j].getUsers().size(); ++k) 
											{
												int target_fd = roomList[j].getUsers()[k].getfd();
												if((numBytes = send(target_fd, convertToChar(message), length, 0)) < 0)
												{
													perror("send (SAY - sending to room)");
													exit(1);
												}

											}
										}
									}
									if(!foundRoom)
									{
										response = "That room does not exist.\n";
										if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
										{
											perror("send (SAY - room doesn't exist)");
											exit(1);
										}
									}
								}
								else //Target is a user
								{
									bool foundUser = true;
									for(unsigned int j = 0; j < userList.size(); ++j) //Loop through users
									{
										if(target == userList[j].getName()) //Does user exist?
										{
											foundUser = true; 

											int target_fd = userList[j].getfd();
											if((numBytes = send(target_fd, convertToChar(message), length, 0)) < 0)
											{
												perror("send (SAY - sending to single user)");
												exit(1);
											}
										}
									}
									if(!foundUser)
									{
										response = "That user does not exist.\n";
										if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
										{
											perror("send (SAY - room doesn't exist)");
											exit(1);
										}
									}
								}
							}
						}
						else if(bufString.find("QUIT") != string::npos) //Disconnect user, make him/her leave all rooms
						{
							if(((currentUser = getClientUsername(userSockets, i)).getName()).empty())
							{
								break;	//Client has no username, do not process command
							}
							if(currentUser.isLoggedIn())
							{
								response = "";
								for(unsigned int j = 0; j < roomList.size(); ++j)
								{
									bool leftRoom = roomList[j].removeUser(currentUser);
									if(leftRoom)
									{
										response += "You have left " + roomList[j].getName() + "\n";
									}
								}
								response += "Logging out...\n";
								response += "Good-Bye!\n";
								currentUser.setLoggedIn(false);
								if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
								{
									perror("send (QUIT)");
									exit(1);
								}
								close(i);
								FD_CLR(i, &master); //delete from master list
							}
						}
						else //Illegal command
						{
							response = "Illegal command given.\n";
							response += "List of legal commands:\n";
							response += "\tCHAT - server connect\n";
							response += "\tUSERS - prints list of users\n";
							response += "\tUSER <name> <password> - login and username creation\n";
							response += "\tROOMS - prints list of rooms\n";
							response += "\tJOIN <room> - join a room or create a room if given \n\t\t room name doesn't exist\n";
							response += "\tPART <room> - leave a joined room\n";
							response += "\tLIST <room> - list users in a room\n";
							response += "\tSAY <target> <length> <message> - send a message to a user \n\t\t or entire room\n";
							response += "\tQUIT - disconnect and leave all joined room\n";

							if((numBytes = send(i, convertToChar(response), response.size(), 0)) < 0)
							{
								perror("send - illegal command)");
								exit(1);
							}							
						}						
					}
				}
			}
		}
	}	

	return 0;
}

//Splits strings by whitespace
vector<string> splitString(string str)
{
	string buffer; 
	stringstream ss(str);
	vector<string> tokens;

	while(ss >> buffer)
		tokens.push_back(buffer);
	for(unsigned int i = 0; i < tokens.size(); i++)
	{
		cout << tokens[i] << endl;
	}
	return tokens;
}

//Returns room from Chatroom list
Chatroom findRoom(string roomName, vector<Chatroom> &roomList)
{
	for(unsigned int i = 0; i < roomList.size(); i++)
	{
		if(roomName == roomList[i].getName())
			return roomList[i];
	}

	return Chatroom("");
}

User getClientUsername(map<int, User> &userSockets, int user_fd)
{
	typename map<int,User>::iterator p; 
	User currentUser("", "");
	if((p = userSockets.find(user_fd)) != userSockets.end()) //Does this client have a username?
	{
		currentUser = p->second;
		return currentUser; 
	}
	
	return currentUser;
}

char* convertToChar(string str)
{
	char* converted = new char[str.size() + 1];
	converted[str.size()] = '\0';
	memcpy(converted, str.c_str(), str.size());	

	return converted;
}

void addNewUser(User &newUser, int user_fd, vector<User> &userList, map<int, User> &userSockets)
{
	string response;
	int numBytes = 0; 

	userList.push_back(newUser);
	userSockets.insert( pair<int,User>(user_fd, newUser) );
	response = "New user created with given credentials.\n";

	if((numBytes = send(user_fd, convertToChar(response), response.size(), 0)) < 0)
	{
		perror("send (USER - create new user)");
		exit(1);
	}
	cout << endl << "test1" << endl;
	return;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    //IPv6
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
