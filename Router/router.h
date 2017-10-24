#include <string>
#include <iostream>
#include <vector>

using namespace std;

#ifndef ROUTER_H
#define ROUTER_H

class router{
	string ip, next, port;
	int value, number;
public:
	router(string, string, string, int);
	string BinaryIpReader();
	string getip();
	string getnext();
	string getport();
	int getvalue();
	int getnumber();
};

#endif

router::router(string inip, string innext, string inport, int no){
	ip = inip;
	next = innext;
	port = inport;
	number = no;
}

string router::getip(){return ip;}

string router::getnext(){return next;}

string router::getport(){return port;}

int router::getvalue(){
	string L= this->BinaryIpReader();
	return L.size();}

int router::getnumber(){return number;}

string router::BinaryIpReader(){
	string binary;
	for(int i=0; i<ip.size()-1; i++){
		for(int j=i; j<ip.size(); j++){
			if(ip[j]=='.'||ip[j]=='/'){
				string str1 = ip.substr(i, j-i);
				//cout<<str1<<endl;
				int num = stoi(str1);
				binary += bitset<8>(num).to_string();
				i = j;
				break;
			}
		}
		if(ip[i]=='/'){
			string str = ip.substr(i+1, ip.size()-i-1);
			value = stoi(str);
			break;
		}
	}
	return binary = binary.substr(0, value);
}