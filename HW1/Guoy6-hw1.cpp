/*homework 1 for network progarmming by Alice
resubmit mar 19 2015*/
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>


using namespace std;

int main(int argc, char* argv[]){
    /*BUILT STRUCT*/
    int sockfd;
    //create socket, exit when fail
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        exit(1);
    }   
     //constract initialize
    struct addrinfo *serv, hint;
    memset(&hint,0,sizeof(hint));
    hint.ai_family = AF_INET6;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
    //geting the infomation of the sever
    int R;
    R = getaddrinfo(argv[1], "12345", &hint, &serv);
    if (R!=0) {
        fprintf(stderr, "could not obtain address of %s\n", argv[1]);
        exit(1);
    }
    /*REG MESSAGE*/
    //perpare to sending reg information
    string myid = argv[2];
    string REGISTER = "REGISTER "+ myid;
    const char *regmess = REGISTER.c_str();//set the sending msaasge
    int rc;
    //message sent
    rc = sendto(sockfd, regmess, strlen(regmess), 0, serv->ai_addr, serv->ai_addrlen);
    if(rc<0){
        perror("register failed");
        exit(1);
    }
    //geting peer information
    string peerid = argv[3];
	string Get_id = "GET_ADDR "+peerid;
    const char *getid = Get_id.c_str();//set id message
    int getpeer;
    //get peer
    getpeer = sendto(sockfd, getid, strlen(getid), 0, serv->ai_addr, serv->ai_addrlen);
    if(getpeer<0){
        perror("get id failed");
        exit(1);
    }
    //message recive for peer id
	struct sockaddr_in6 from;// the return adress
    char buf[1024];//recive buf
    int recvlen;
    socklen_t addrlen = sizeof(from);
    recvlen = recvfrom(sockfd, buf, 1024, 0, (struct sockaddr *)&from, &addrlen);
    if (recvlen <= 0 || strcmp(buf, "NOT FOUND") == 0){// decied weather is vaild id
        perror("get id failed");
        exit(1);
    }// id recived

    //TO tanslate the adress recive
    char portc[10];
    char ipadd[60];
    int seprater = (int)(strchr(buf, ' ') - buf);// sepeate adress and port
    strncpy(ipadd, buf, seprater);
    strcpy(portc, buf+seprater+1);
    long int port = (long int)portc;// get peer port
    struct sockaddr_in6 peer;// constracted the peer id
    peer.sin6_family=AF_INET6;
    peer.sin6_port = htons(port);
    if (inet_pton(AF_INET6, ipadd, (void *) &peer) != 1) {
        perror("get id failed");
        exit(1);
    }
    else{
        inet_pton(AF_INET6, ipadd, (void *) &peer);
    }

    //prepare to sending message to peer
	char *message = argv[4];
    struct addrinfo *ai_peer;//sending adress
    R = getaddrinfo(ipadd, portc, &hint, &ai_peer);
    if (R!=0) {
        fprintf(stderr, "could not obtain sending address of %s\n", argv[1]);
        exit(1);
    }
    rc = sendto(sockfd, message, strlen(message), 0, ai_peer->ai_addr, ai_peer->ai_addrlen);// sending
    if(rc<0){
    	perror("send failed");
    	exit(1);
    }
    //finish sent
    //start infinity loop for recive message
    for (;;){
    	char node[80];
		char service[80];
		char rec[1024];
    	socklen_t peerlen = sizeof(from);
    	recvlen = recvfrom(sockfd, rec, 1024, 0, (struct sockaddr *)&from, &peerlen);
    	if (recvlen <= 0){
    		perror("receive failed");
    		exit(1);
    	}
        //get the adress from where recive
    	int gs = getnameinfo((struct sockaddr *)&from, sizeof(from),node, sizeof(node), service, sizeof(service), 0| NI_NUMERICHOST | NI_NUMERICSERV);
    	if(gs<0){
    		perror("get name failed");
    		exit(1);
    	}
        //print out recive message
    	printf("received from %s (%s): ", node, service);
    	fwrite(rec, recvlen, 1, stdout);
		printf("\n");
    }

    return 0;
}