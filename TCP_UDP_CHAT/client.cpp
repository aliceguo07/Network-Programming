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
#include <time.h>

using namespace std;

#define MAXLEN 1024
#define PORT "34567"

void setclient(int &sockfd);

bool Regsend(int sockfd, struct addrinfo *server, string &username);

void setupcall(int sockfd, struct addrinfo *server, string &username);

void MakeCall(int sockfd, struct addrinfo *server, string &username);

inline void makecallyn(int sockfd, struct addrinfo *server, string &username);

void CallMessage(int sockfd, char *buf, struct addrinfo *server, string &username);

void RejectMessage(int sockfd, char *buf, struct addrinfo *server, string &username);

void call(int sockfd,char *buf, string &username, struct addrinfo *server);

void recivemess(int sockfd, struct addrinfo *media);

int main(int argc, char *argv[]) {
	string ip; 
	int sockfd;
	bool regstute = false;

	struct addrinfo *server, hint;
	memset(&hint,0,sizeof(hint));
    hint.ai_family = AF_INET6;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;

	string username;
	/*get host information*/
	if(argc!=2){
		cout<<"Error: Please enter server address\n";
		exit(1);
	}
	if (getaddrinfo(argv[1], PORT , &hint, &server)!=0)  
    {  
    	printf("getaddrinfo()error\n");  
    	exit(1);  
    }
    setclient(sockfd);
    regstute = Regsend(sockfd, server, username);
    while(!regstute){
    	regstute = Regsend(sockfd, server, username);
    }
    /*register success
    starting calling process or parpare to recive call*/
    setupcall(sockfd, server, username);
}

void setclient(int &sockfd)
{/*ser sock*/
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM,0))==-1)  
    {  
     	printf("socket() error\n");  
     	exit(1);  
    }  
}

bool Regsend(int sockfd, struct addrinfo *server, string &username)
{
	char buf[MAXLEN] = "\0";
	/*register*/
	bool regsucess = false;
	cout<<"please enter username: ";
	cin>> username;
	/*check the username*/
	for(int i = 0; i<username.size(); i++)
	{
		if(!isalnum(username[i]))
		{
			cout<<"Invaild username";
			return regsucess;
		}
	}
	string rgems = "REGISTER "+ username;
	/*send username*/
	if (sendto(sockfd, rgems.c_str(), strlen(rgems.c_str()), 0, server->ai_addr, server->ai_addrlen)==-1)
	{
		perror("register send fail");	
		exit(1);
	}
	socklen_t addrlen=sizeof(server);
	/*recv from server*/
	if(recvfrom(sockfd,buf,MAXLEN,0,server->ai_addr, &server->ai_addrlen)<0){
		perror("Recv error ");	
		exit(1);
	}
	/*fail username already exist*/
	if(strstr(buf, "FAIL")!=NULL){
		cout<<"username exist.\nPlease enter a new name"<<endl;
		return regsucess;
	}
	/*success*/
	if(strstr(buf, "ACK_REGISTER")!=NULL){
		string outmessage = string(buf);
		cout<<outmessage<<endl;
		regsucess = true;
	}
	return regsucess;
}

void setupcall(int sockfd, struct addrinfo *server, string &username){
	/*ask if what call or not*/
	makecallyn(sockfd, server, username);
	while(1){
		char buf[MAXLEN] = "\0";
		/*waiting for incoming call or other message*/
		if(recvfrom(sockfd,buf,MAXLEN,0,server->ai_addr, &server->ai_addrlen)<0)
		{
			perror("Recv error ");
		}
		if(strstr(buf, "CALL_FAILED")!=NULL)
		{/*call failed, ask if want to make call again*/
			printf("Server: %s",buf);
    		printf("\n");
    		makecallyn(sockfd, server, username);
		}
		if(strstr(buf, "ACK_CALL")!=NULL)
		{/*call accept make call*/
			printf("Server: %s",buf);
    		printf("\n");
		}
		if(strstr(buf, "CALL_RECIVE")!=NULL)
		{/*recive call or not*/
			printf("Server: %s",buf);
    		printf("\n");
    		cout<<"Accept Call? (Y/N): ";
    		string YN;
    		cin>>YN;
    		if(YN == "Y")
			{
				CallMessage(sockfd, buf, server, username);
			}
			else
			{
				RejectMessage(sockfd, buf, server, username);
			}
		}
		if(strstr(buf, "MEDIA_PORT")!=NULL)
		{/* make call*/
			printf("Server: %s",buf);
    		printf("\n");
    		call(sockfd, buf, username, server);
		}
	}
}

inline void makecallyn(int sockfd, struct addrinfo *server, string &username){
	cout<<"Make Call or Not(enter Y/N): ";
	string YN;
	cin>>YN;
	if(YN == "Y")
	{/*making call*/
		MakeCall(sockfd, server, username);
	}
	else if(YN == "N"){}
	else{
		cout<<"Enter Invaild, would not make call"<<endl;
	}
}

void MakeCall(int sockfd, struct addrinfo *server, string &username)
{
	string peerid,message;
	cout<<"Enter the user id to call: ";
	cin>> peerid;/*enter user id*/
	message = "CALL FROM: "+ username + " TO: " +peerid;
	if (sendto(sockfd, message.c_str(), strlen(message.c_str()), 0, server->ai_addr, server->ai_addrlen)==-1)
	{
		perror("Call send fail");
		exit(1);
	}
}

void CallMessage(int sockfd, char *buf, struct addrinfo *server, string &username)
{/*when recive call*/
	string orgfind =  string(buf);
	string Call = "CALL_RECIVE from: ";
	string username1 = orgfind.substr(orgfind.find(Call)+Call.size());
	string message = "ACK_CALL FROM: "+ username+ " TO: " +username1;
	if (sendto(sockfd, message.c_str(), strlen(message.c_str()), 0, server->ai_addr, server->ai_addrlen)==-1)
	{
		perror("ACK_CALL message send fail");
		exit(1);
	}
}

void RejectMessage(int sockfd, char *buf, struct addrinfo *server, string &username)
{/*Reject call recive*/
	string orgfind =  string(buf);
	string Call = "CALL_RECIVE FROM: ";
	string username1 = orgfind.substr(orgfind.find(Call)+Call.size());
	string message = "CALL_FAILED FROM: "+ username+ " TO: " +username1 + " Reason: user reject";
	if (sendto(sockfd, message.c_str(), strlen(message.c_str()), 0, server->ai_addr, server->ai_addrlen)==-1)
	{
		perror("Reject message send fail");
		exit(1);
	}
}

void call(int sockfd,char *buf, string &username, struct addrinfo *server)
{/*sending*/
	struct addrinfo *media = server;
	string nameninfo = "NAME: " + username;
	if (sendto(sockfd, nameninfo.c_str(), strlen(nameninfo.c_str()), 0, media->ai_addr, media->ai_addrlen)==-1)
	{
		perror("Address confrim fail");
		exit(1);
	}
	std::thread recvmess(recivemess, sockfd, media);
	while(1){
		memset(buf, '\0', MAXLEN);
    	string message;
    	cin>>message;
    	if (sendto(sockfd, message.c_str(), strlen(message.c_str()), 0, media->ai_addr, media->ai_addrlen)==-1)
		{
			perror("Message send fail");
			exit(1);
		}
		if(message == "End"){
			cout<<"Program End"<<endl;
			exit(0);
		}
	}
	recvmess.join();
	cout<<"Program End"<<endl;
	exit(0);
}

void recivemess(int sockfd, struct addrinfo *media)
{/*recive*/
	char buf[MAXLEN] = "\0";
	while(1){
		memset(buf, '\0', MAXLEN);	
		if(recvfrom(sockfd,buf,MAXLEN,0,media->ai_addr, &media->ai_addrlen)<0)
		{
			perror("Recv error ");
		}
		printf("\nUser: %s",buf);
    	printf("\n");
    	string buff = string(buf);
    	if (buff == "End"){
    		cout<<"User end the program"<<endl;
    		break;
    	}
	}
}