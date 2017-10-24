#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <sstream>
#include "trie.h"

using namespace std;

#ifndef NAT_H
#define NAT_H

class Nat{
	string natport, natip;
	int n;
public:
	Nat(string,string);
	string getnatp();
	string getnatip();
	void natprint(router r, pdu p,  int n);
	//void natsave();
};
#endif

	Nat::Nat(string s1, string s2){
		natport = s1;
		natip = s2;
	}

	string Nat::getnatp(){
		return natport;
	}

	string Nat::getnatip(){
		return natip;
	}


	void Nat::natprint(router r, pdu p, int n){
			cout<<natip<<":"<<n<<"->"<<p.gatename()<<":"<<p.getn3()<<" ";
	}

bool natcheck(Nat n, router r){
	return n.getnatp() == r.getport();
}




// bool Nat::nat(string){
// 	if (r.getport()==natport){
// 		return true;
// 	}
// }