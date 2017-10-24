#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <sstream>
#include "nat.h"


using namespace std;

//read in the file
void routesreader(string argv, std::vector<router>& R, std::vector<int>& range, Trie* trie, int &n);

void arpreader(string argv, map<string, string>& arpmap);

void natreader(string argv, std::vector<Nat>& nats, string bools);

void pdureader(vector<pdu>& pdus);

void mainpart(Trie *trie, std::vector<router> R, std::vector<int> range, map<string, string> arpmap, std::vector<Nat> nats, vector<pdu> pdus);

/*function used by mainpart*/

void firstprint(bool nat, pdu p, std::vector<int>& natp, std::vector<router> R, int n, std::vector<Nat>& nats, int natint);

void portppp0(string arpfind, Trie* trie, std::vector<router> R, std::vector<int> range, map<string, string> arpmap, pdu p);

// change address to binary
string Binary(string gate);

int main(int argc, char* argv[]){
	//check if the arguments vaild
	if (argc < 4){
		std::cerr<< "invaild arguments"<< endl;
		return 1;
	}

	Trie* trie = new Trie();
	string arpname, natname, routesname;

	std::vector<router> R;
	std::vector<int> range;
	int n = 1;
	//routerfile read
	routesname = string(argv[1]);
	routesreader(routesname, R, range, trie, n);
	//cout<<R.size()<<endl;

	//arpfile read
	map<string, string> arpmap;
	arpname = string(argv[2]);
	arpreader(arpname, arpmap);
	//cout<<arpmap.size()<<endl;


	//natfile read
	std::vector<Nat> nats;
	natname = string(argv[3]);
	natreader(natname, nats, string(argv[4]));

	//pdu file
	vector<pdu> pdus;
	pdureader(pdus);
	/*start*/
	//main print part for result
	mainpart(trie, R, range, arpmap, nats, pdus);
}
/* FUNCTION START*/
/*---------------------------------Binary-----------------------------------*/
string Binary(string gate){
	string binary;
	for(int i=0; i<gate.size(); i++){
		for(int j=i; j<gate.size(); j++){
			if(gate[j]=='.'){
				string str1 = gate.substr(i, j-i);
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

/*--------------------------------------------------------reading in file function-------------------------------------------------*/
void routesreader(string argv, std::vector<router>& R, std::vector<int>& range, Trie* trie, int &n){
	ifstream routesfile;
	routesfile.open(argv);
	// check the file
	if(!routesfile.good()){
		cerr<<"cannot read the router file"<<endl;
		exit(1);
	}
	string r1, r2, r3;

	//read in the file
	while(!routesfile.eof()){
		routesfile>>r1>>r2>>r3;
		router r(r1, r2, r3, n);
		range.push_back(n);
		trie->addIP(r);
		R.push_back(r);
		n++;
	}
	//cout<<R.size()<<endl;
}

void arpreader(string argv, map<string, string>& arpmap){
	ifstream arpfile;
	arpfile.open(argv);
	//check file
	if(!arpfile.good()){
		cerr<<"cannot read the arp file"<<endl;
		exit(1);
	}
	string arpf, arps;
	while(!arpfile.eof()){
		arpfile>>arpf>>arps;
		arpmap[arpf] = arps;
	}

}

void natreader(string argv, std::vector<Nat>& nats, string bools){
	ifstream natfile;
	natfile.open(argv);
	if(!natfile.good()){
		cerr<<"cannot read the nat file"<<endl;
		exit(1);
	}
	//reading the file
	string s1,s2;
	if(bools == "part2"){
		while(!natfile.eof()){
		natfile>>s1>>s2;
		Nat n(s1, s2);
		nats.push_back(n);
		}
	}
}

void pdureader(vector<pdu>& pdus){
	ifstream pdufile;
	string pduname = "pdus.txt";
	pdufile.open(pduname);
	if(!pdufile.good()){
		cerr<<"cannot read the pdu file"<<endl;
		exit(1);
	}

	string port, ip, gate;
	int n1, ttl, n2, n3;
	//read pdu file
	while(!pdufile.eof()){
		pdufile>> port>> ip>> gate>> n1>> ttl>> n2>> n3;
		pdu p(port, ip, gate, n1, ttl, n2, n3);
		pdus.push_back(p);
	}
}

/*=====================================================end of reading file start to process===================================*/
void mainpart(Trie *trie, std::vector<router> R, std::vector<int> range, map<string, string> arpmap, std::vector<Nat> nats, vector<pdu> pdus){
	std::vector<int> natp;
	string arpfind, ports;
	for(std::vector<int>::size_type k =0; k<pdus.size();k++){
		pdu p = pdus[k];
		int n = trie->searchIP(p);
		bool find = false;
		bool nat = false;
		int natint;
		map<string, string>::iterator it;
		for (std::vector<int>::size_type i =0; i<range.size();i++){
			for (std::vector<string>::size_type j =0; j<nats.size();j++){
				if(natcheck(nats[j], R[i])){
					natint = j;
					nat = true;
					break;
				}
			}
			if(n==range[i]){
				find = true;
				p.ttlchange();
				//print the first part
				firstprint(nat, p, natp, R , n, nats, natint);
				if(p.getttl()<=0){
					cout<<"discarded (TTL expired)"<<endl;
					break;
				}else{
					ports = R[i].getport();
					arpfind = R[i].getnext();
					if (ports == "ppp0"){
						portppp0(arpfind, trie, R, range, arpmap, p);
					}else if (arpfind == "0.0.0.0"){
						arpfind = p.gatename();
						it = arpmap.find(arpfind);
						if(it==arpmap.end()){
							cout<<"discarded (destination unreachable)"<<endl;
							break;
						}
						else{
							cout<<"directly connected ("<<ports<<"-"<<it->second<<") ttl "<<p.getttl()<<endl;
							break;
						}
					}
					else{
						it = arpmap.find(arpfind);
						if(it==arpmap.end()){
							cout<<"discarded (destination unreachable)"<<endl;
							break;
						}
						else{
							cout<<"via "<<arpfind<<"("<<ports<<"-"<<it->second<<") ttl "<<p.getttl()<<endl;
							break;
						}
					}
				}
			}
		}

				//for end
		if(!find){
			int n = 0;
			for (std::vector<int>::size_type i =0; i<range.size();i++){
				for (std::vector<string>::size_type j =0; j<nats.size();j++){
					if(natcheck(nats[j], R[i])){
						natint = j;
						nat = true;
						break;
					}
				}
				if(R[i].getvalue()==0){
					n = i;
					p.ttlchange();
					break;
				}
			}
			firstprint(nat, p, natp, R , n, nats, natint);
			if(p.getttl()<=0){
				cout<<"discarded (TTL expired)"<<endl;
				break;
			}
			if(n ==0){
				cout<<"discarded (destination unreachable)"<<endl;
				break;
			}
			ports = R[n].getport();
			arpfind = R[n].getnext();
			if (ports == "ppp0"){
			portppp0(arpfind, trie, R, range, arpmap, p);
	 		}
	 	}
	}
}

//ex 192.168.1.99:9999->10.3.0.1:22 
void firstprint(bool nat, pdu p, std::vector<int>& natp, std::vector<router> R, int n, std::vector<Nat>& nats, int natint){
	if(!nat){
		//print the first part ip and gate way
		cout<<p.ipname()<<":"<<p.getn2()<<"->"<<p.gatename()<<":"<<p.getn3()<<" ";
	}else{
		int natpset;
		if (natp.size()==0){
			natpset = p.getn2();
			natp.push_back(natpset);
		}else{
			int sizev = natp.size()-1;
			if (natp[sizev]<20000){
				natpset = 20000;
				natp.push_back(natpset);
			}else{
				natpset=natp[sizev]+1;
				natp.push_back(natpset);
			}
		}
		nats[natint].natprint(R[n], p, natpset);//nats[natint].natprint(R[i], p, natp);
	}
}

//function for it is PPP0
void portppp0(string arpfind, Trie* trie, std::vector<router> R, std::vector<int> range, map<string, string> arpmap, pdu p){
	string nextgate;
	string nextport;
	map<string, string>::iterator it;
	bool pfind = false;
	string by = Binary(arpfind);
	int w = trie->search(by);
	for (std::vector<int>::size_type j =0; j<range.size();j++){
	 	if(w==range[j]){
	 		pfind = true;
			nextgate = R[j].getnext();
			nextport = R[j].getport();
	 		if (nextport == "ppp0" & nextgate!="0.0.0.0"){
	 			cout<<"via "<< R[j].getnext()<<"(ppp0) ttl "<<p.getttl()<<endl;
	 			break;
	 		}else if(nextport == "ppp0" & nextgate=="0.0.0.0"){
	 			cout<<"via "<< arpfind<<"(ppp0) ttl "<<p.getttl()<<endl;
	 			break;
	 		}else if(nextport != "ppp0" & nextgate=="0.0.0.0"){
	 			it = arpmap.find(arpfind);
	 			if(it==arpmap.end()){
	 				cout<<"discarded (destination unreachable)"<<endl;
	 				break;
	 			}else{
	 				cout<<"directly connected "<<"("<<nextport<<"-"<<it->second<<") ttl"<<p.getttl()<<endl;
	 				break;
	 			}
	 		}else{
	 			it = arpmap.find(arpfind);
	 			if(it==arpmap.end()){
	 				cout<<"discarded (destination unreachable)"<<endl;
	 				break;
	 			}else{
	 				cout<<"via "<<nextgate<<"("<<nextport<<"-"<<it->second<<") ttl"<<p.getttl()<<endl;
	 				break;
	 			}
	 		}
	 	}
	}
}
