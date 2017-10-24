Yiqing Guo(661051755)
rewrite for project2
I miss understand the assignment, the first time submit it works on the client I write by myself.
I change the code a bit, so it works on the p2_sample_client.js I hope you can take a look at it and re-grade it
thank you


compile: g++ -std=c++11 new_server.cpp -Ipthread -o nserver

run: ./nserver
Messsage Recived 
Message from ::1 (64886): REGISTER ua
Message from ::1 (64887): REGISTER ub
Message from ::1 (64887): CALL FROM:ub TO:ua
Message from ::1 (64886): ACK_CALL FROM:ua TO:ub
^C

yiqings-MacBook-Air:Project2 yiqingguo$ node p2_sample_client.js localhost ua
Received ACK_REGISTER
Unknown packet received!
Hi there, this is ub
Unknown packet received!
Hi there, this is ub
Unknown packet received!
Hi there, this is ub
Unknown packet received!
Hi there, this is ub
Unknown packet received!
Hi there, this is ub
^C

yiqings-MacBook-Air:Project2 yiqingguo$ node p2_sample_client.js localhost ub ua
Received ACK_REGISTER
Call accepted!
Unknown packet received!
Hi there, this is ua
Unknown packet received!
Hi there, this is ua
Unknown packet received!
Hi there, this is ua
Unknown packet received!
Hi there, this is ua
Unknown packet received!
Hi there, this is ua
Unknown packet received!
Hi there, this is ua
^C
