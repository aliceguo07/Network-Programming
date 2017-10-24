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

void create_call(string username1, string username2, 
	struct sockaddr_in6 peer1, struct sockaddr_in6 peer2 );

void recsend(int sockpeer, struct sockaddr_in6 media,
	struct sockaddr_in6 peer1, struct sockaddr_in6 peer2);

int main(){
	cout<<"Messsage Recived "<<endl;
	int sockSrv;
	struct sockaddr_in6 server;  
    struct sockaddr_in6 client;	
    map<string, struct sockaddr_in6> usertable;
    vector<string> InCall;/*peer that is already in call*/
    int n = 0;

	setserver(sockSrv, server);
	recive(sockSrv, client, usertable, n);
  close(sockSrv);
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
     if(memcmp((usertable.find(username)->second).sin6_addr.s6_addr, client.sin6_addr.s6_addr, 16)!=0){
    	 string userexist = "FAIL: user name exist";
    	 if (sendto(sockSrv,  userexist.c_str(), strlen(userexist.c_str()), 
    		0, (struct sockaddr*)&client,sizeof(client))==-1)
		    {
			 close(sockSrv);
			 die("Send error message fail");	
		    }
      }
      else{
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
    string Call = "CALL FROM:";
    string To = " TO:";
    foundCall = orgfind.find(Call);
    foundTo = orgfind.find(To);
    /*find user name1, 2*/
    string username1, username2;
    username1 = orgfind.substr(foundCall+Call.size(), foundTo-(foundCall+Call.size()));
    username2 = orgfind.substr(foundTo+To.size());
    /*check it username exist, and find address*/
    //map<string,struct sockaddr_in6>::iterator ip1;
    map<string,struct sockaddr_in6>::iterator ip2;
    //ip1 = usertable.find(username1);
    ip2 = usertable.find(username2);
    if(/*ip1 == usertable.end()||*/ip2 == usertable.end())
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
    	//struct sockaddr_in6 peer1 = ip1->second;
    	struct sockaddr_in6 peer2 = ip2->second;
      string callrecive = "CALL FROM:"+ username1+" TO:"+username2;
      if (sendto(sockSrv, callrecive.c_str(), strlen(callrecive.c_str()), 
        0, (struct sockaddr*)&peer2,sizeof(peer2))==-1)
      {
        close(sockSrv);
        die("Send CALL_RECIVE messgae fail"); 
      }
    	//confrimwithuser(sockSrv, username1, username2, peer2);
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
    string Call = "ACK_CALL FROM:";
    string To = " TO:";
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
	int port = rand() % 1000 + 5000;
	create_call(username1, username2, peer1, peer2);
}

void callfail(int sockSrv, char *buf, struct sockaddr_in6 client, 
	map<string, struct sockaddr_in6> &usertable)
{/*call fail send message to peer1*/
	string orgfind =  string(buf);
		size_t foundCall, foundTo;
    string Call = "CALL_FAILED FROM:";
    string To = " TO:";
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

void create_call(string username1, string username2, 
	struct sockaddr_in6 peer1, struct sockaddr_in6 peer2 )
{/*part 3*/
  int sockpeer = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sockpeer == -1) 
  {
    die("Couldn't create a socket"); 
  }
  /*create new port*/
  struct sockaddr_in6 media; 
  bzero(&media,sizeof(media));
    media.sin6_family=AF_INET6;    
    media.sin6_addr= in6addr_any;
  do
  {
    media.sin6_port = htons(rand() % 1000 + 5000);
  }
  while(/*bind*/::bind(sockpeer, (struct sockaddr *)&media,sizeof(media)) < 0);
  /*message for sending*/
  char send1[1024];
  strcpy(send1, "MEDIA_PORT FROM:");
  strcat(send1, username2.c_str());
  strcat(send1, " TO:");
  strcat(send1, username1.c_str());
          
  char s1[100];
  sprintf(s1, " %d", ntohs(peer2.sin6_port));
  strcat(send1, s1);

  char send2[1024];
  strcpy(send2, "MEDIA_PORT FROM:");
  strcat(send2, username1.c_str());
  strcat(send2, " TO:");
  strcat(send2, username2.c_str());
          
  char s2[100];
  sprintf(s2, " %d", ntohs(peer1.sin6_port));
  strcat(send2, s2);
    /*send message to both user*/
  if (sendto(sockpeer, send1, strlen(send1), 
    0, (struct sockaddr*)&peer1,sizeof(peer1))==-1)
	{
		die("Send port messgae fail");	
	}
	if (sendto(sockpeer, send2, strlen(send2), 
    		0, (struct sockaddr*)&peer2,sizeof(peer2))==-1)
	{
		die("Send port messgae fail");	
	}
  pid_t child;
  if((child = fork()) == -1)
  {
    perror("Error: Fork failed!\n");
    exit(1);
  }
  else if(child == 0) //in child process
  {
    while(1)
    {
	    recsend(sockpeer, media, peer1, peer2);
    }
  }
  close(sockpeer);
}

void recsend(int sockpeer, struct sockaddr_in6 media,
	struct sockaddr_in6 peer1, struct sockaddr_in6 peer2)
{
	char buf[MAXLEN] = "\0";
  in_port_t  p1 = 0, p2 = 0;
	/*recive new adress*/
  socklen_t addrlen=sizeof(media);
  int rec =recvfrom(sockpeer,buf,MAXLEN,0,(struct sockaddr*)&media, &addrlen);                                     
  if (rec < 0)
    {  
      close(sockpeer);
    die("recvfrom error"); 
  }
  /*wait until a packet is received from each side before forwarding can begin*/
  if(p1 == 0 || p2 == 0)
  {
    if(media.sin6_addr.s6_addr == peer1.sin6_addr.s6_addr&&media.sin6_port==peer1.sin6_port)
     {
        p1 = media.sin6_port;
        peer1 = media;
      }
    else if(media.sin6_addr.s6_addr == peer2.sin6_addr.s6_addr&&media.sin6_port==peer2.sin6_port)
    {
      p2 = media.sin6_port;
      peer2 = media;
    }
  }
  if(p1 != 0 && p2 != 0)
  {
    /*looking at the source address*/
    if(media.sin6_addr.s6_addr == peer1.sin6_addr.s6_addr&&media.sin6_port==peer1.sin6_port)
    {
      if(sendto(sockpeer, buf, strlen(buf) + 1, 0, (struct sockaddr *)&peer2, addrlen) < 0)
        {
          close(sockpeer);
          die("send fail");
        }
    }
    else if(media.sin6_addr.s6_addr == peer2.sin6_addr.s6_addr&&media.sin6_port==peer2.sin6_port)
    {
      if(sendto(sockpeer, buf, strlen(buf) + 1, 0, (struct sockaddr *)&peer1, addrlen) < 0)
      {
        close(sockpeer);
        die("send fail");                  
      }
    }
  }
}
