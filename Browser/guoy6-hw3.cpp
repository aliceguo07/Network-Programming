/*Network program HW3
Creadt by Alice Guo on Mar 19 2015*/
/*header*/
#include <string>
#include <cstddef> //size_t
#include <iostream> 
#include <stdio.h>
#include <stdint.h>// unit_8 = a bytes
#include <ctype.h>// isdigit
//ip header
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>

//define
#define USER_AGENT "guoy6_hw3/1.0"

//namespace
using namespace std;

/*-------------------------------------------------FIRST PART----------------------------------------------------*/
/*                      Step 1: URL Parsing
You need to parse the URL into a host, a port, and a path. For example, 
for http://www.asquaredlabs.com:80/csci4220/, 
the host is www.asquaredlabs.com, the port is 80, and the path is /csci4220/. 
If no port is given, use port 80 as this is the standard, well­known port for HTTP.*/

//struct URL for host port and path
struct URL{
	string front; // ex: http://
	string host; 
	string port; // defult 80 if not given
	string path; // not menditory
};

/*take the the url string to find the first of the string  and check it if not out put error massage*/
size_t findthefirst(string urlin, string needfind){
	size_t position = urlin.find_first_of(needfind);
	if (position == string::npos){// if not found invaild
		cerr<<"The input url is invaild"<<endl;
		exit(1);
	}
	return position;
}

URL URLParsing(char* argv);
URL URLParsing(char* argv){
	string url = string(argv);
	URL input; // the input argv for struct URL
	size_t posfront = findthefirst(url, ":");
	input.front = url.substr(0, posfront);//this would possiably be the front part

	if(posfront+1 != findthefirst(url,"/")||url[findthefirst(url,"/")+1]!='/'){
		cerr<<"The input url is invaild"<<endl;
		exit(1);
	}
	url = url.substr(posfront+3);//remain url
	//finding if have port number or not by using find_first_of()
	size_t hostend = url.find_first_of(":");
	if( hostend == string::npos){// does not given port number, will using 80 as defult
		input.port = "80";
		//port not given so the host is from the begining of the new url to the first of "/"
		input.host = url.substr(0, findthefirst(url,"/"));
	}else{
		//if the port is given the host is from begining to the ":" and port is from ":" to the first of "/"
		input.host = url.substr(0, hostend);
		input.port = url.substr(hostend+1, findthefirst(url,"/")-hostend-1);
	}
	//port and host done, next is to decide if it is have port or not. set a new url string
	url = url.substr(findthefirst(url, "/"));
	input.path = url;

	return input;
}
//URLParsing work proproly
/*FIRST PART END*/

/*--------------------------------------------------SECOND PART---------------------------------------------------*/
/*                                        Step 2: TCP Socket Connection
Use getaddrinfo() to convert your host and port strings to socket addresses. 
You may use a v6­only socket with v6 and v6­mapped v4 addresses,
 or you may create the appropriate type of socket for each address returned from getaddrinfo(). 
If you choose the second option, make sure to close any extra sockets that you don’t use. 
You should attempt to connect() to each address returned until you find one which succeeds.*/

inline void setSocket(int &sockfd) {
	sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("creat sock fail");
		exit(1);
	}
}
// initalize the adressinfo ipv6
struct addrinfo *setAddrinfo(URL url){
	struct addrinfo *serv, hint;
	memset(&hint, 0, sizeof(hint));// initial = =
	hint.ai_family = AF_INET6;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
	int R = getaddrinfo(url.host.c_str(), url.port.c_str(), &hint, &serv);
	if (R!=0) {
        fprintf(stderr, "could not obtain address of %s\n", url.host.c_str());
        exit(1);
    }

    return serv;
}
//loop connecting till success
void connecting(int &sockfd, struct addrinfo *serv){
	bool connected = false;
	int R;
	struct addrinfo * t;
	for( t = serv; t != NULL; t = t->ai_next) {
		R = connect(sockfd, t->ai_addr, t->ai_addrlen);
		if (R == 0) {
			connected = true;
			break;
		}
	}

	freeaddrinfo(serv);// free address

	if(!connected) {
		perror("connect fail");
		exit(1);
	}
}
/*END PART TWO*/

/*--------------------------------------------------THIRD PART---------------------------------------------------*/
/*                              Step 3: Send Request and Headers
An HTTP request consists of a request line followed by headers ­ each header item on one line.
 The request line is of the form “method path version” ­ for example “GET /csci4220/ HTTP/1.1”. 
 The headers can contain additional information about the request or the client. Every HTTP/1.1 request must include a Host header. 
 This header is used to host multiple sites on the same IP address by requiring the client to communicate the hostname part of the URL to the server. It must be in the form “Host: hostname:port” e.g. “Host: www.asquaredlabs.com:80”​. 
 The port can be omitted if it is 80 e.g. “Host: w​ww.asquaredlabs.com”​. While HTTP does not require it, 
 you must also include a User­Agent header to identify your program to servers e.g. “User­Agent: armena2­netprog­hw3/1.0”. 
 Your program must not assume that the entire request headers can be written in a single write() or send().*/
string messagerequest(URL url){
 	string msgrequest;
 	msgrequest= "GET "+url.path+" HTTP/1.1\r\n"+ "Host: " + url.host;
 	if (url.port!= "80"){
 		msgrequest += ":"+ url.port;
 	}
 	msgrequest += "\r\nUser­Agent: " + string(USER_AGENT) + "\r\n\r\n";
 	return msgrequest;
}

//send() calls return the number of bytes sent.
//1 char is one byte= 8bits => using uint8_t is one byte to spreate sending.
void sending(int &sockfd, string message, int flags){//flags = 0 for TCP?
	int lenthmsg = message.size();
	int sendbyte;
	const uint8_t* one = reinterpret_cast<const uint8_t*>(message.c_str());
	//Converts between types by reinterpreting the underlying bit pattern
	while (lenthmsg > 0){
		sendbyte = send(sockfd, one, message.size(), flags);
		if(sendbyte < 0) {
			perror("send request error");
			exit(1);
		} else {
			lenthmsg -= sendbyte;
			one += sendbyte;
		}
	}
}
/*END THIRD PART*/
/*--------------------------------------------------FOURTH PART---------------------------------------------------*/
/*                         Step 4: Read Response
Separate the response into headers and content ­remember, these are separated by a blank line.
Write the headers to standard error, and the response to standard output. 
You will need to use multiple calls to read() or recv() to accomplish this. 
Since we may receive a binary file from the server, you should use write(), fwrite(), 
or cout.write() to write the data bytes to standard output. 
The server will typically provide a “Content­Length: x” header where x is the number of bytes that will be contained in the response body. 
You will need to find and parse this header to determine the number of bytes to read. 
You may handle the case where the server does not include this header by exiting with an error.
Be sure to close the socket after the response has been read.*/
void Receive(int sockfd, char *pLine, unsigned int header_size) {

	while (string(pLine).find("\r\n\r\n")==string::npos && strlen(pLine) < header_size) {
		if (!recv(sockfd, pLine, header_size, 0)){
			perror("receive error");
			exit(1);
		}
	}
	//this would be the header
	printf("Headers\r\n%s\r\n", pLine);

	string header = string(pLine);
	//find content-length(15) and ype
	int size;// length
	size_t findlenth = header.find("Content-Length");

	//if lenth not found, could still choose contiuned or not....
	if(findlenth == string::npos){
		cout<<"Error: cannot find the content length\n If still want to contiuned recive content Enter y, else end the program";
		string y;
		cin>> y;
		if(y=="y"){
			cout<<"Enter size want recive: ";
			int temp;
			cin>>temp;
			size = temp;
		}else{
			cout<<"End";
			exit(0);
			//end not find the lenth
		}
	}else{
		string ssize;
		for(int i = findlenth+16; i<header.size(); i++){
			if (!isdigit(header[i])){
				size = stoi(ssize);
				break;
			}
			ssize += header[i];
		}
	}
	//above set the size that wll be recive
	//message recive here
	int recvbyte=0;
	int totalbyte=0;
	do{
		char content[size];
		recvbyte= recv(sockfd, content, size, 0);
		if ( recvbyte > 0 ){
		 	printf("%s", content);
		 	totalbyte += recvbyte;
		}else if ( recvbyte == 0 ){
		 	if(strlen(content)>0){}
		 	else{
		 		printf("receive failed");
                break;
		 	}
                    
        }else{
        	printf("recv failed: ");
            break;
        }
 
    } while(totalbyte<size);

    cout<<endl;
}

//finally here is the mean
int main(int argc, char* argv[]){
	//check if the argv is vaild
	if (argc != 2){
		perror("enter only one argument");
		exit(1);
	}

	/*set the variable*/
	URL url = URLParsing(argv[1]);//url
	int sockfd;
	struct addrinfo *serv = NULL;// addrinfo
	char buf[1024];

	/*PART2 RUN*/
	setSocket(sockfd);
	serv = setAddrinfo(url);
	//connect
	connecting(sockfd, serv);
	//test the URLParsing function
	//cout<<url.front<<"\n"<<url.host<<"\n"<<url.port<<"\n"<<url.path<<endl;
	string msg = messagerequest(url);
	// cout<< msg<<endl;
	sending(sockfd, msg, 0);
	//recive
	Receive(sockfd, buf, 1024);

	//close
	close(sockfd);
	printf("Socket Closed\r\n");

	return 0;

}
