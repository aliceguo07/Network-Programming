#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <sstream>
#include "router.h"
#include "pdu.h"

using namespace std;

#ifndef TRIE_H
#define TRIE_H

class Node{
	Node* left;//0
	Node* right;//1
	bool isEnd;
	int c;
	int n;

public:
	Node();
	Node(int c, bool end);
	Node* getLeftPtr();
    Node* getRightPtr();
    void setLeftPtr(Node* node);
    void setRightPtr(Node* node);
    void setEnd();
    bool getEnd();
    void setnum(int num);
    int getnum();


};

class Trie { 
public: 
	Trie(); 
	~Trie(); 
	bool addIP(router routers); 
	int searchIP(pdu p);
	int search(string ip);

private: 
	Node* root; 
};

#endif

Node::Node() {
    isEnd = false;
    left = NULL;
    right = NULL;
}

Node::Node(int c, bool end) {
    isEnd = end;
    left = NULL;
    right = NULL;
}


Node* Node::getLeftPtr() {return left;}

Node* Node::getRightPtr() {return right;}

void Node::setLeftPtr(Node* node) {left = node;}

void Node::setRightPtr(Node* node) {right = node;}

void Node::setEnd(){isEnd=true;}

bool Node::getEnd() {return isEnd;}

void Node::setnum(int num){n = num;}

int Node::getnum(){return n;}

//**---------------------------------------End class NODE----------------------------------------**
//**---------------------------------------class Trie------------------------------------------**

Trie::Trie() { 
	root = new Node(); 
} 
Trie::~Trie() {}


bool Trie::addIP(router routers){
	Node* node = root;
	string IP = routers.BinaryIpReader();
	for(int i = 0; i< IP.length(); i++){
		if(IP[i]=='0'){
			if(node->getLeftPtr()==NULL) {
                node->setLeftPtr(new Node(int(IP[i]), false));
            }
            node = node->getLeftPtr();
        }
        else {
            if(node->getRightPtr()==NULL) {
                node->setRightPtr(new Node(int(IP[i]), false));
            }
            node = node->getRightPtr();
        }
	}
	node->setEnd();	
	node->setnum(routers.getnumber());
	return true;
}

int Trie::searchIP(pdu p){
	string binary = p.Binarypdus();
	Node* node = root;
	int n;
	for(int i=0; i<binary.size(); i++){
		if(binary[i]=='0'){
			if(node->getLeftPtr()==NULL) {
                break;
            }
            node = node->getLeftPtr();
        }
        else {
            if(node->getRightPtr()==NULL) {
                break;
            }
            node = node->getRightPtr();
        }
        if(node->getEnd()){
        	n = node->getnum();
        }
	}
	return n;
}

int Trie::search(string ip){
	Node* node = root;
	int n;
	for(int i=0; i<ip.size(); i++){
		if(ip[i]=='0'){
			if(node->getLeftPtr()==NULL) {
                break;
            }
            node = node->getLeftPtr();
        }
        else {
            if(node->getRightPtr()==NULL) {
                break;
            }
            node = node->getRightPtr();
        }
        if(node->getEnd()){
        	n = node->getnum();
        }
	}
	return n;
}