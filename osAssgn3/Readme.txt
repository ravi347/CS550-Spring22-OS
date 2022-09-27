Chatroom using a character device
Problem: Enable two or more processes to chat with each other using a character device (say /dev/chatroom).
Details

Implement a kernel module and a user-level program.

Your kernel module should create a character device called /dev/chatroom when inserted into the kernel and implement the open(), close(), read() and write() functions.

-> A user-level process can join the chatroom using the open() system call on /dev/chatroom as soon as the process is started.
-> To send a chat message to other processes in the chatroom, a process uses the write() system call on /dev/chatroom. 
-> The message is sent to all the other processes which have joined the chatroom.
-> To receive a message, a process uses the read() system call on /dev/chatroom.
-> A process can exit the chatroom by typing “Bye!”, which invokes the close() system call on /dev/chatroom. 
