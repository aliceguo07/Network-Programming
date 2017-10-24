server.cpp:
compile: g++ -std=c++11  server.cpp -Ipthread -o server
run: ./server
Server will record the message recive.

Messsage Recived: 
Message from ::1 (61712): REGISTER Alice
Message from ::1 (61713): REGISTER Alphy
Message from ::1 (61712): CALL FROM: Alice TO: Alphy
Message from ::1 (61713): ACK_CALL FROM: Alphy TO: Alice
Message from ::1 (61712): NAME: Alice
Message from ::1 (61713): NAME: Alphy
Message from ::1 (61713): Hi
Message from ::1 (61712): Hello
Message from ::1 (61712): End


client.cpp: g++ -std=c++11  client.cpp -Ipthread -o client
run: ./client localhost

please enter username: Alice
ACK_REGISTER Alice
Make Call or Not(enter Y/N): Y
Enter the user id to call: Alphy
Server: ACK_CALL FROM: Alphy TO: Alice
Server: MEDIA_PORT FROM: Alice To: Alphy 5000

User: Hi
Hello
End
Program End


pplease enter username: Alphy
ACK_REGISTER Alphy
Make Call or Not(enter Y/N): N
Server: CALL_RECIVE from: Alice
Accept Call? (Y/N): Y
Server: MEDIA_PORT FROM: Alphy To: Alice 5000
Hi

User: Hello

User: End
User end the program
End
Program End

