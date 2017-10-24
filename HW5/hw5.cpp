#include <string>
#include <cstddef> 
#include <iostream> 
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <stdio.h>
#include <string>

#define USER_AGENT "guoy6_hw3/1.0"

#define BUF_SIZE 1024

using namespace std;

struct URL{
	string front; // ex: http://
	string host; 
	string port; // defult 80 if not given
	string path; // not menditory
};

URL URLParsing(char* argv);/*Parsing the URL*/
string messagerequest(URL url);/*parsing the request*/
string messageheadrequest(URL url);/*head request*/
void setSocket(int &sockfd);/*set the stock*/
struct addrinfo *setAddrinfo(URL url);
void connecting(int &sockfd, struct addrinfo *serv);/*connecting*/
void https(URL url);/*openssl https connection*/
size_t findthefirst(string urlin, string needfind);
int parse_h(std::string h);
void SHOWC(SSL * ssl);
void sending(int &sockfd, string message, int flags);
void Receive(int sockfd, char *pLine, unsigned int header_size);
void http(URL url);/*http*/

int main(int argc, char *argv[]){
	if (argc != 2){
		perror("enter only one argument");
		exit(1);
	}

	/*set the variable*/
	URL url = URLParsing(argv[1]);
	if(url.front == "http"){
		http(url);
	}
	else if(url.front == "https"){
		https(url);
	}
	else{
		cout<<"wrong header"<<endl;
		exit(1);
	}
	return 0;
}
/*https function, doing ssl connection*/
void https(URL url)
{
    int sockfd, len;
    struct addrinfo *serv = NULL;
    char buffer[BUF_SIZE + 1];
    SSL_CTX *CTX;
    SSL *ssl;

    //Init SSL lib
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    CTX = SSL_CTX_new(SSLv23_client_method());
    if (CTX == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    
    setSocket(sockfd);
    
    /*Connect to server*/
    if (connect(sockfd, serv->ai_addr, serv->ai_addrlen) != 0) {
        perror("Connect ");
        exit(errno);
    }
    printf("server connected\n");
    
    /*Create SSL*/
    ssl = SSL_new(CTX);
    SSL_set_fd(ssl, sockfd);
    /*ssl connection*/
    if (SSL_connect(ssl) == -1)
        ERR_print_errors_fp(stderr);
    else {
        //printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        SHOWC(ssl);
    }
    
    /*get header*/
    string t= messageheadrequest(url);
    bzero(buffer, BUF_SIZE + 1);
    strcpy(buffer, t.c_str());
    //Send msg using SSL
    len = SSL_write(ssl, buffer, strlen(buffer));
    if (len < 0)
        printf
        ("Msg'%s'send failed！Error code %d，Error msg '%s'\n",
         buffer, errno, strerror(errno));
    
    /*recv header*/
    bzero(buffer, BUF_SIZE + 1);
    len = SSL_read(ssl, buffer, BUF_SIZE);
    if (len < 0)
    {
        printf
        ("Fail to recv！Error code %d，Error msg '%s'\n",
         errno, strerror(errno));
    }
    
    int s=parse_h(buffer)+len;
    /*Get request*/
    t = messagerequest(url);
    bzero(buffer, BUF_SIZE + 1);
    strcpy(buffer, t.c_str());
    /*send*/
    len = SSL_write(ssl, buffer, strlen(buffer));
    if (len < 0)
        printf
        ("Msg'%s'send failed！Error code %d，Error msg '%s'\n",
         buffer, errno, strerror(errno));
    
    /*recv*/
    ssize_t rr, recvd = 0;
    char * buf = new char[s];
    std::string r="";
    while (s > 0)
    {
        rr = SSL_read(ssl,buf,s);
        r=r+buf;
        s -= rr;
        buf += rr;
        recvd += rr;
    }
    if(r.size()>recvd) r.erase(r.begin()+recvd,r.end());
    if (recvd > 0)
        printf("Recv successful:'%s'，%d bytes recvd\n",
               r.c_str(), r.size());
    else {
        printf
        ("Fail to recv！Error code %d，Error msg '%s'\n",
         errno, strerror(errno));
        goto finish;
    }
    
    
finish:
    //close connection
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(CTX);
    return;
}

/*http from hw3*/
void http(URL url){
	int sockfd;
	struct addrinfo *serv = NULL;// addrinfo
	char buf[BUF_SIZE];

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
	Receive(sockfd, buf, BUF_SIZE);

	//close
	close(sockfd);
	printf("Socket Closed\r\n");
}


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
		if(input.front == "http"){
			input.port = "80";
		}
		else if(input.front == "https"){
			input.port = "443";
		}
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

size_t findthefirst(string urlin, string needfind){
	size_t position = urlin.find_first_of(needfind);
	if (position == string::npos){// if not found invaild
		cerr<<"The input url is invaild"<<endl;
		exit(1);
	}
	return position;
}

string messagerequest(URL url){
 	string msgrequest;
 	msgrequest= "GET "+url.path+" HTTP/1.1\r\n"+ "Host: " + url.host;
 	msgrequest += ":"+ url.port;
 	msgrequest += "\r\nUser­Agent: " + string(USER_AGENT) + "\r\n\r\n";
 	return msgrequest;
}

string messageheadrequest(URL url){
 	string msgrequest;
 	msgrequest= "HEAD "+url.path+" HTTP/1.1\r\n"+ "Host: " + url.host;
 	msgrequest += ":"+ url.port;
 	msgrequest += "\r\nUser­Agent: " + string(USER_AGENT) + "\r\n\r\n";
 	return msgrequest;
}

void setSocket(int &sockfd) {
	sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("creat sock fail");
		exit(1);
	}
}

struct addrinfo *setAddrinfo(URL url){
	struct addrinfo *serv, hint;
	memset(&hint, 0, sizeof(hint));// initial
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
//parse
int parse_h(std::string h)
{
    std::string t="Content-Length: ";
    int f=h.find(t);
    if (f==-1) return -1;
    int b=0,e=0;
    for(int i=f;i<h.size();++i)
    {
        if(h[i]==' ') b=i;
        if(h[i]=='\n') {e=i;break;}
    }
    std::string temp;
    temp.append(h.begin()+b+1,h.begin()+e);
    return atoi(temp.c_str());
}

void SHOWC(SSL * ssl)
{
    X509 *cert;
    char *line;
    
    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        printf("Cert info:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Cert: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    } else
        printf("No Cert！\n");
}

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
	int totalbyte=header.size();
	do{
		char content[size];
		recvbyte= recv(sockfd, content, size, 0);
		if ( recvbyte > 0 ){
			totalbyte += recvbyte;
		 	fwrite(content, recvbyte, 1, stdout);
		 	//check if reach the size end
		 	if(totalbyte>=size){
		 		break;
		 	}
		}else /*if ( recvbyte == 0 ){
		 	if(strlen(content)>0){}
		 	else{
		 		printf("receive failed");
                break;
		 	}
                    
        }else*/{
        	printf("recv failed: ");
            break;
        }
 
    } while(totalbyte<size);

    cout<<endl;
}

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

