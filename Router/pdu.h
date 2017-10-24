#include <string>
#include <iostream>
#include <vector>

using namespace std;

#ifndef PDU_H
#define PDU_H

class pdu{
	string port, ip, gate; 
	int n1, ttl, n2, n3;
public:
	pdu(string, string, string, int, int, int, int);
	string Binarypdus();
	string portname();
	string ipname();
	string gatename();
	int getn1();
	int getttl();
	int getn2();
	int getn3();
	void ttlchange();

};
#endif
//initial
pdu::pdu(string iport, string iip, string igate, int in1, int ittl, int in2, int in3){
	port = iport;
	ip = iip;
	gate = igate;
	n1 = in1;
	ttl = ittl;
	n2 = in2;
	n3 = in3;
}

string pdu::portname(){return port;}

string pdu::ipname(){return ip;}

string pdu::gatename(){return gate;}

int pdu::getn1(){return n1;}

int pdu::getttl(){return ttl;}

int pdu::getn2(){return n2;}

int pdu::getn3(){return n3;}

void pdu::ttlchange(){ttl -= 1;}
// to binary 
string pdu::Binarypdus(){
	string binary;
	for(int i=0; i<gate.size(); i++){
		for(int j=i; j<gate.size(); j++){
			if(gate[j]=='.'){
				string str1 = gate.substr(i, j-i);
				//cout<<str1<<endl;
				int num = stoi(str1);
				binary += bitset<8>(num).to_string();
				i = j;
				break;
			}
			if(j==gate.size()-1){
				string str1 = gate.substr(i, gate.size()-i);
				int num = stoi(str1);
				binary += bitset<8>(num).to_string();
			}
		}
	}
	return binary;
}