/*header*/
#include <unistd.h> /* fork, close */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <stdlib.h> /* exit */
#include <algorithm>
#include <string.h> /* strlen */
#include <stdio.h>
#include <vector>
#include <stdio.h> /* perror, fdopen, fgets */
#include <sys/socket.h>
#include <sys/wait.h> /* waitpid */
#include <netdb.h> /* getaddrinfo */
#include <map>
#include <thread>

using namespace std;

#define MAXLEN 1024
#define PORT 34567
#define ACCEPT "ACK_REGISTER "
#define die(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)/*exit*/

void setserver(int &sockSrv, struct sockaddr_in6 &server);

void recive(int sockSrv, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> &usertable, int &n);

void readline(int sockSrv, char *buf, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> &usertable, int &n);

void Register(int sockSrv, char *buf, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> &usertable);

void CallCheck(int sockSrv, char *buf, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> usertable);

void confrimwithuser(int sockSrv, string username1, string username2, 
	struct sockaddr_in6 peer2);

void startcall(int sockSrv, char *buf, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> &usertable, int &n);

void callfail(int sockSrv, char *buf, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> &usertable);

void create_call(int port, string username1, string username2, 
	struct sockaddr_in6 peer1, struct sockaddr_in6 peer2 );

void recsend(int sockpeer, struct sockaddr_in6 media, struct sockaddr_in6 peer,
	string username1, string username2, struct sockaddr_in6 peer1, struct sockaddr_in6 peer2);

int main(){
	cout<<"Messsage Recived: "<<endl;
	int sockSrv;
	struct sockaddr_in6 server;  
    struct sockaddr_in6 client;	
    map<string, struct sockaddr_in6> usertable;
    vector<string> InCall;/*peer that is already in call*/
    int n = 0;

	setserver(sockSrv, server);
	recive(sockSrv, client, usertable, n);
}

/*creat sock, bind are in here*/
void setserver(int &sockSrv, struct sockaddr_in6 &server){
	/* Create a socket and get its file descriptor -- socket(2) */
	sockSrv = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sockSrv == -1) 
	{
		die("Couldn't create a socket"); 
	}

    bzero(&server,sizeof(server));
    server.sin6_family=AF_INET6;  
    server.sin6_port=htons(PORT);  
    server.sin6_addr= in6addr_any;
   	/* Assign address to this socket's Srv */
    if(::bind(sockSrv, (struct sockaddr *)&server,sizeof(server)) < 0)
    {
    	die("Couldn't bind");
    }     
}

/*while recive message*/
void recive(int sockSrv, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> &usertable, int &n){
	socklen_t addrlen=sizeof(client);
	while(1)
	{
		char buf[MAXLEN] = "\0";
		int rec =recvfrom(sockSrv,buf,MAXLEN,0,(struct sockaddr*)&client,&addrlen);                                     
  
      	if (rec < 0)
      	{  
      		die("recvfrom error"); 
      	}
		readline(sockSrv, buf, client, usertable, n);
  	}
}

void readline(int sockSrv, char *buf, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> &usertable, int &n)
{
	char straddr[INET6_ADDRSTRLEN]; 
    inet_ntop(AF_INET6, &client.sin6_addr, straddr,sizeof(straddr));
	if(strstr(buf, "REGISTER ")!=NULL)/*REG message*/
	{
		printf("Message from %s (%d): %s",straddr , htons(client.sin6_port), buf);
    	printf("\n");
		Register(sockSrv, buf, client, usertable);
	}
	else if(strstr(buf,"ACK_CALL")!= NULL)/*call accept from peer2*/
	{
		printf("Message from %s (%d): %s",straddr , htons(client.sin6_port), buf);
    	printf("\n");
		startcall(sockSrv, buf, client, usertable, n);
	}
	else if(strstr(buf, "CALL_FAILED")!= NULL)/*call fail*/
	{
		printf("Message from %s (%d): %s",straddr , htons(client.sin6_port), buf);
    	printf("\n");
		callfail(sockSrv, buf, client, usertable);
	}
	else if(strstr(buf,"CALL FROM")!= NULL)/*call set up*/
	{
		printf("Message from %s (%d): %s",straddr , htons(client.sin6_port), buf);
    	printf("\n");
		CallCheck(sockSrv, buf, client, usertable);
	}
}

/*register goes here*/
void Register(int sockSrv, char *buf, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> &usertable)
{ 
    //print out recive message
    string orgfind =  string(buf);
    string username = orgfind.substr(9);
   	/*check weather username is already in use*/
    if(usertable.find(username)!=usertable.end()){
    	string userexist = "FAIL: user name exist";
    	if (sendto(sockSrv,  userexist.c_str(), strlen(userexist.c_str()), 
    		0, (struct sockaddr*)&client,sizeof(client))==-1)
		{
			close(sockSrv);
			die("Send error message fail");	
		}
    }
    else
    {	/*reg username*/
    	usertable[username] = client;
    	string regaccp = ACCEPT+ username;
    	if (sendto(sockSrv, regaccp.c_str(), strlen(regaccp.c_str()), 
    		0, (struct sockaddr*)&client,sizeof(client))==-1)
		{
			close(sockSrv);
			die("Send ACK_REGISTER messgae fail");	
		}
    }
}

/*call check*/
void CallCheck(int sockSrv, char *buf, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> usertable)
{   
    /*readin line and find userid*/
    string orgfind =  string(buf);
    size_t foundCall, foundTo;
    string Call = "CALL FROM: ";
    string To = " TO: ";
    foundCall = orgfind.find(Call);
    foundTo = orgfind.find(To);
    /*find user name1, 2*/
    string username1, username2;
    username1 = orgfind.substr(foundCall+Call.size(), foundTo-(foundCall+Call.size()));
    username2 = orgfind.substr(foundTo+To.size());
    /*check it username exist, and find address*/
    map<string,struct sockaddr_in6>::iterator ip1;
    map<string,struct sockaddr_in6>::iterator ip2;
    ip1 = usertable.find(username1);
    ip2 = usertable.find(username2);
    if(ip1 == usertable.end()||ip2 == usertable.end())
    {/*username not find*/
    	string Unknowen = "CALL_FAILED unknown peer";
    	if (sendto(sockSrv, Unknowen.c_str(), strlen(Unknowen.c_str()), 
    		0, (struct sockaddr*)&client,sizeof(client))==-1)
		{
			close(sockSrv);
			die("Send CALL_FAILED messgae fail");	
		}
    }
    else{
    	/*get two peer address*/
    	struct sockaddr_in6 peer1 = ip1->second;
    	struct sockaddr_in6 peer2 = ip2->second;
    	confrimwithuser(sockSrv, username1, username2, peer2);
    }
}

void confrimwithuser(int sockSrv, string username1, string username2,
	struct sockaddr_in6 peer2)
{/*send a pack to peer2 if accept call*/
	string callrecive = "CALL_RECIVE from: "+ username1;
	if (sendto(sockSrv, callrecive.c_str(), strlen(callrecive.c_str()), 
    		0, (struct sockaddr*)&peer2,sizeof(peer2))==-1)
	{
		close(sockSrv);
		die("Send CALL_RECIVE messgae fail");	
	}
}


void startcall(int sockSrv, char *buf, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> &usertable, int &n)
{/*call start*/
	string orgfind =  string(buf);
	size_t foundCall, foundTo;
    string Call = "ACK_CALL FROM: ";
    string To = " TO: ";
    foundCall = orgfind.find(Call);
    foundTo = orgfind.find(To);
    /*find user name1, 2*/
    string username1, username2;
    username2 = orgfind.substr(foundCall+Call.size(), foundTo-(foundCall+Call.size()));
    username1 = orgfind.substr(foundTo+To.size());
    /*check it username exist, and find address*/
    map<string,struct sockaddr_in6>::iterator ip1;
    map<string,struct sockaddr_in6>::iterator ip2;
    ip1 = usertable.find(username1);
    ip2 = usertable.find(username2);
	struct sockaddr_in6 peer1 = ip1->second;
    struct sockaddr_in6 peer2 = ip2->second;
    if (sendto(sockSrv, orgfind.c_str(), strlen(orgfind.c_str()), 
    		0, (struct sockaddr*)&peer1,sizeof(peer1))==-1)
	{
		close(sockSrv);
		die("Send ACK_CALL messgae fail");	
	}
	int port = 5000+n;
	n++;
	create_call(port, username1, username2, peer1, peer2);
}

void callfail(int sockSrv, char *buf, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> &usertable)
{/*call fail send message to peer1*/
	string orgfind =  string(buf);
		size_t foundCall, foundTo;
    string Call = "CALL_FAILED FROM: ";
    string To = " TO: ";
    string Reason = " Reason: ";
    foundCall = orgfind.find(Call);
    foundTo = orgfind.find(To);
    /*find user name1, 2*/
    string username1, username2;
    username2 = orgfind.substr(foundCall+Call.size(), (foundTo-(Call.size()))-1);
    username1 = orgfind.substr(foundTo+(To.size()+1), (orgfind.find(Reason)-(foundTo+To.size()))-1);
    /*check it username exist, and find address*/
    map<string,struct sockaddr_in6>::iterator ip1;
    map<string,struct sockaddr_in6>::iterator ip2;
    ip1 = usertable.find(username1);
    ip2 = usertable.find(username2);
	struct sockaddr_in6 peer1 = ip1->second;
    struct sockaddr_in6 peer2 = ip2->second;
    if (sendto(sockSrv, orgfind.c_str(), strlen(orgfind.c_str()), 
    		0, (struct sockaddr*)&peer1,sizeof(peer1))==-1)
	{
		close(sockSrv);
		die("Send CALL_FAILED messgae fail");	
	}
}

void create_call(int port, string username1, string username2, 
	struct sockaddr_in6 peer1, struct sockaddr_in6 peer2 )
{/*part 3*/
	string str =to_string(port);/*message for sending*/
	string send1 = "MEDIA_PORT FROM: " + username1+" To: "+username2 + " " +str;
	string send2 = "MEDIA_PORT FROM: " + username2+" To: "+username1 + " " +str;
	int sockpeer = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sockpeer == -1) 
	{
		die("Couldn't create a socket"); 
	}
	/*create new port*/
	struct sockaddr_in6 media; 
	bzero(&media,sizeof(media));
    media.sin6_family=AF_INET6;  
    media.sin6_port=htons(port);  
    media.sin6_addr= in6addr_any;

    struct sockaddr_in6 peer;
    /*bind*/
    if(::bind(sockpeer, (struct sockaddr *)&media,sizeof(media)) < 0)
    {
    	die("Couldn't bind");
    }
    /*send message to both user*/
    if (sendto(sockpeer, send1.c_str(), strlen(send1.c_str()), 
    		0, (struct sockaddr*)&peer1,sizeof(peer1))==-1)
	{
		die("Send port messgae fail");	
	}
	if (sendto(sockpeer, send2.c_str(), strlen(send2.c_str()), 
    		0, (struct sockaddr*)&peer2,sizeof(peer2))==-1)
	{
		die("Send port messgae fail");	
	}
	std::thread peerconn(recsend, sockpeer, media, peer, username1, username2, peer1, peer2);
	peerconn.join();
	//recsend(sockpeer, media, peer, peer1, peer2);
}

void recsend(int sockpeer, struct sockaddr_in6 media, struct sockaddr_in6 peer,
	string username1, string username2, struct sockaddr_in6 peer1, struct sockaddr_in6 peer2)
{
	char buf[MAXLEN] = "\0";
	/*recive new adress*/
	for(int i = 0; i <2; i ++){
		memset(buf, '\0', sizeof(buf));
		socklen_t addrlen=sizeof(peer);
		int rec =recvfrom(sockpeer,buf,MAXLEN,0,(struct sockaddr*)&peer, &addrlen);                                     
    	if (rec < 0)
    	{  
    		die("recvfrom error"); 
    	}
    	char straddr[INET6_ADDRSTRLEN]; 
    	inet_ntop(AF_INET6, &peer.sin6_addr, straddr,sizeof(straddr));
    	printf("Message from %s (%d): %s",straddr , htons(peer.sin6_port), buf);
   		printf("\n");
   		string orgfind =  string(buf);
   		if(username1 == orgfind.substr(orgfind.find("NAME: ")+6))
   		{
   			peer1 = peer;
   		}
   		else
   		{
   			peer2 = peer;
   		}
	}
	/*peer1 information*/
	char straddr1[INET6_ADDRSTRLEN]; 
    inet_ntop(AF_INET6, &peer1.sin6_addr, straddr1,sizeof(straddr1));
    string p1 = string(straddr1);
    int portp1 = htons(peer1.sin6_port);
    /*peer2 information*/
	char straddr2[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &peer2.sin6_addr, straddr2,sizeof(straddr2));
    string p2 = string(straddr2);
    int portp2 = htons(peer2.sin6_port);
	while(1){
		memset(buf, '\0', MAXLEN);
		/*recive message*/
		socklen_t addrlen=sizeof(peer);
		int rec =recvfrom(sockpeer,buf,MAXLEN,0,(struct sockaddr*)&peer, &addrlen);                                     
      	if (rec < 0)
      	{  
      		die("recvfrom error"); 
      	}
      	/*find who send message*/
      	char straddr[INET6_ADDRSTRLEN]; 
    	inet_ntop(AF_INET6, &peer.sin6_addr, straddr,sizeof(straddr));
    	printf("Message from %s (%d): %s",straddr , htons(peer.sin6_port), buf);
    	printf("\n");
    	string p = string(straddr);
      	if(p1 == p && htons(peer.sin6_port) == portp1)
      	{
      		if (sendto(sockpeer, buf, strlen(buf), 
    		0, (struct sockaddr*)&peer2,sizeof(peer2))==-1)
			{
				die("Send messgae fail");	
			}
      	}
      	else
      	{
      		if (sendto(sockpeer, buf, strlen(buf), 
    		0, (struct sockaddr*)&peer1,sizeof(peer1))==-1)
			{
				die("Send messgae fail");	
			}
      	}
      	if(string(buf)=="End"){
      		break;
      	}
      	sleep(5);
	}
}

