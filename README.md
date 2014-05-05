ChatServer
==========

A chat server written using sockets in C++. The server runs on port 9999.
The server accepts the following commands: 

1. CHAT - server connect
2. USERS - prints list of users
3. USER <name> <password> - login and username creation
4. ROOMS - prints list of rooms
5. JOIN <room> - join a room or create a room if given room name doesn't exist
6. PART <room> - leave a joined room
7. LIST <room> - list users in a room
8. SAY <target> <length> <message> - send a message to a user or entire room
9. QUIT - disconnect and leave all joined rooms

All room names must begin with the '@' symbol to differentiate them from usernames
