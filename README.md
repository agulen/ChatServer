ChatServer
==========

A chat server written using sockets in C++. The server runs on port 9999
The server accepts the following commands: 

CHAT - server connect
USERS - prints list of users
USER <name> <password> - login and username creation
ROOMS - prints list of rooms
JOIN <room> - join a room or create a room if given room name doesn't exist
PART <room> - leave a joined room
LIST <room> - list users in a room
SAY <target> <length> <message> - send a message to a user or entire room
QUIT - disconnect and leave all joined rooms

All room names must begin with the '@' symbol to differentiate them from usernames