#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <list>
#include <tuple>
#include <vector>
#include <algorithm>
#include <map>


#include "graphics.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

#define MAXSEGLENGTH 8191
#define MAXATTEMPTS 100
#define DRAWEACHROUTE 0



typedef std::tuple<int, int, int> pin;
typedef std::tuple<char, int, int, int> wire;

enum segment_state{
	UNUSED,
	USED,
	TARGET,
	ROUTING
};

enum checkReturn{
	OK,
	ISTARGET,
	ISUSED
};

class Segment{
private:
	enum segment_state state;
	int length;
	pin sourcepin;
	pin destpin;
	wire index;
public:
	Segment();
	Segment(wire w);
	Segment(char h, int x, int y, int w);
	void clear();
	bool isTarget();
	bool isUnused();
	bool isUsed();
	bool isRouting();
	bool usesSameSource(pin p);
	bool usesSameSource(Segment* s);
	enum checkReturn checkAndSet(Segment * src);

	void setState(enum segment_state s);
	void setLength(int l);
	void setSource(pin s);
	void setDest(pin d);
	void setIndex(wire w);

	pin getSource();
	pin getDest();
	int getLength();
	enum segment_state getState();
	wire getIndex();

};

class Connection {
private:
	int x1;
	int y1;
	int p1;
	int x2;
	int y2;
	int p2;

public:
	Connection();
	Connection(int p[6]);
	Connection(int, int, int, int, int, int);
	~Connection();
	bool operator==(const Connection&);
	bool isEOPL();
	void print();
	pin src();
	pin dest();
};

static const Connection EOPL = { -1, -1, -1, -1, -1, -1 };

Connection::Connection(){
	x1 = 0;
	y1 = 0;
	p1 = 0;
	x2 = 0;
	y2 = 0;
	p2 = 0;
}

Connection::Connection(int p[6]){
	x1 = p[0];
	y1 = p[1];
	p1 = p[2];
	x2 = p[3];
	y2 = p[4];
	p2 = p[5];

}


Connection::Connection(int a, int b, int c, int d, int e, int f){
	x1 = a;
	y1 = b;
	p1 = c;
	x2 = d;
	y2 = e;
	p2 = f;
}
Connection::~Connection(){

}
bool Connection::operator==(const Connection& c){
	return (x1 == c.x1 && y1 == c.y1 && p1 == c.p1 && x2 == c.x2 && y2 == c.y2&& p2 == c.p2);

}
bool Connection::isEOPL(){
	//return (this == &EOPL);
	return (x1 == EOPL.x1 && y1 == EOPL.y1 && p1 == EOPL.p1 && x2 == EOPL.x2 && y2 == EOPL.y2&& p2 == EOPL.p2);
}

void Connection::print(){
	std::cout << "Source: (" << x1 << ", " << y1 << ", " << p1 << ") -> Dest: (" << x2 << ", " << y2 << ", " << p2 << ")" << std::endl;
}

pin Connection::src(){ return std::make_tuple(x1, y1, p1); }
pin Connection::dest(){ return std::make_tuple(x2, y2, p2); }

class Channel{
private:
	Segment *** horiz;
	Segment *** vert;
	int N;
	int W;
	bool tryHard;
//	enum CHANNELMODE mode;
public:
	Channel(int n, int w);
	~Channel();

	void resetRouting();
	void clearAttempt();
	int routingSegmentsUsed();
	int maxW();
	bool findSetAvailableNeighbours(wire t, std::list<wire>* neigh);
	Segment * findSetAvailableNeighbours(Segment * t, std::list<Segment *> * neigh);
	bool route(pin src, pin dest);
	Segment* segmentAt(pin p, int w);
	Segment* segmentAt(bool horiz, int x, int y, int w);
	Segment* segmentAt(wire w);
	Segment* segmentAt(char hv, int x, int y, int w);
	void traceback(Segment * dest); 
	void tryHarder(bool b=true);

};




namespace utilvars{
	int graphn, graphw;
	Channel * routing;
	std::map<pin, enum color_types> colormap;
}

void printConnList(std::list<Connection> connlist){
	std::list<Connection>::iterator it = connlist.begin();

	for (it; it != connlist.end(); it++){
		it->print();
	}
}

struct connections_t {
	int p[6]; // {x1, y1, p1, x2, y2, p2}
};

void popToFront(std::list<struct connections_t>* lst, std::list<struct connections_t>::iterator it){
	struct connections_t temp;

	temp = *(it);
	lst->erase(it);
	lst->push_front(temp);

}

void popToFront(std::list<Connection>* lst, std::list<Connection>::iterator it){
	Connection temp;

	temp = *(it);
	lst->erase(it);
	lst->push_front(temp);

}

Segment::Segment(){
	state = UNUSED;
	sourcepin = std::make_tuple(-1, -1, -1);
	destpin = std::make_tuple(-1, -1, -1);
	length = MAXSEGLENGTH;
}
void Segment::clear(){
	state = UNUSED;
	sourcepin = std::make_tuple(-1, -1, -1);
	destpin = std::make_tuple(-1, -1, -1);
	length = MAXSEGLENGTH;

}
bool Segment::isTarget(){ return (state == TARGET); }
bool Segment::isUnused(){ return (state == UNUSED); }
bool Segment::isUsed(){ return (state == USED); }
bool Segment::isRouting(){ return (state == ROUTING); }
bool Segment::usesSameSource(pin p){ return (sourcepin == p); }
bool Segment::usesSameSource(Segment * s){ return (sourcepin == s->getSource()); }
enum checkReturn Segment::checkAndSet(Segment * src){
	if (state == TARGET){
		length = src->getLength() + 1;
		sourcepin = src->getSource();
		state = USED;	//????
		return ISTARGET;
	}
	if (state == UNUSED){
		length = src->getLength() + 1;
		sourcepin = src->getSource();
		state = ROUTING;
		return OK;
	}
	if (state == USED && usesSameSource(src)){
		length = src->getLength() + 1;
		return OK;
	}

	return ISUSED;

}

void Segment::setState(enum segment_state s){ state = s; }
void Segment::setLength(int l){ length = l; }
void Segment::setSource(pin s){ sourcepin = s; }
void Segment::setDest(pin d) { destpin = d; }
void Segment::setIndex(wire w){ index = w; }

pin Segment::getSource(){ return sourcepin; }
pin Segment::getDest(){ return destpin; }
int Segment::getLength(){ return length; }
enum segment_state Segment::getState(){ return state; }
wire Segment::getIndex(){ return index; }

std::list<Segment *> randomizeList(std::list<Segment *> l){
	std::list<Segment *> newl;
	std::vector<Segment *> v;
	for(std::list<Segment *>::iterator iter = l.begin(); iter!= l.end(); iter++){
		v.push_back(*iter);
	}
	std::random_shuffle(v.begin(), v.end());
	for (int i=0; i<v.size(); i++)
		newl.push_back(v[i]);

	return newl;

}

Channel::Channel(int n, int w){
	N = n;
	W = w;
//	mode = BIDIR;
	tryHard=false;

	horiz = new Segment**[N];
	vert = new Segment  **[N + 1];

	for (int i = 0; i < N; i++){
		horiz[i] = new Segment*[N + 1];
		for (int j = 0; j < N + 1; j++){
			horiz[i][j] = new Segment[W];
			for (int k = 0; k < W; k++)
				horiz[i][j][k].setIndex(std::make_tuple('h', i, j, k));
		}
	}

	for (int i = 0; i < N+1; i++){
		vert[i] = new Segment*[N];
		for (int j = 0; j < N; j++){
			vert[i][j] = new Segment[W];
			for (int k = 0; k < W; k++)
				vert[i][j][k].setIndex(std::make_tuple('v', i, j, k));
		}
	}

}

Channel::~Channel(){
	for (int i = 0; i < N + 1; i++){
		for (int j = 0; j < N; j++){
			delete vert[i][j];
		}
		delete vert[i];
	}
	delete vert;
	for (int i = 0; i < N ; i++){
		for (int j = 0; j < N +1 ; j++){
			delete horiz[i][j];
		}
		delete horiz[i];
	}
	delete horiz;

}

void Channel::resetRouting(){
	for (int i = 0; i < N + 1; i++){
		for (int j = 0; j < N; j++){
			for (int k = 0; k < W; k++){
				vert[i][j][k].clear();
			}
		}
	}

	for (int i = 0; i < N; i++){
		for (int j = 0; j < N + 1; j++){
			for (int k = 0; k < W; k++){
				horiz[i][j][k].clear();
			}
		}
	}

}

void Channel::clearAttempt(){
	Segment * seg;
	for (int i = 0; i < N + 1; i++){
		for (int j = 0; j < N; j++){
			for (int k = 0; k < W; k++){
				seg = &vert[i][j][k];
				seg->setLength(MAXSEGLENGTH);
				if (seg->isRouting()||seg->isTarget()) seg->clear();
			}
		}
	}

	for (int i = 0; i < N; i++){
		for (int j = 0; j < N + 1; j++){
			for (int k = 0; k < W; k++){
				seg = &horiz[i][j][k];
				seg->setLength(MAXSEGLENGTH);
				if (seg->isRouting() || seg->isTarget()) seg->clear();
			}
		}
	}

}

bool Channel::findSetAvailableNeighbours(wire t, std::list<wire> * neigh){
	neigh->clear(); 
	/*int x = std::get<1>(t);
	int y = std::get<2>(t);
	int w = std::get<3>(t);
	pin p = horiz[x][y][w].getSource();
	Segment * seg;
	if (std::get<0>(t) = 'h'){
		seg = &horiz[x][y][w];
		if (x>0 && horiz[x - 1][y][w].isTarget()){
			horiz[x - 1][y][w].setLength(seg->getLength() + 1);
			return true;
		}
	//		|| horiz[x - 1][y][w].isUnused() || horiz[x - 1][y][w].usesSameSource(p))){
			//mark
		//}
	}
	else {
		seg = &vert[x][y][w];

	}*/
	return false;
}

/*  
 Marks neighbouring segments to t as length+1 if available or share the same source
 All marked segments are stored to the list neigh for further processing.
 Returns a pointer to a target segment, if found. Otherwise returns nullptr.
 t - pointer to segment to test
 neigh - list of available neighbours to t
 */
Segment * Channel::findSetAvailableNeighbours(Segment * t, std::list<Segment *> * neigh){
	neigh->clear();
//	pin src = t->getSource();
	//int len = t->getLength();
	char hv = std::get<0>(t->getIndex());
	int x = std::get<1>(t->getIndex());
	int y = std::get<2>(t->getIndex());
	int w = std::get<3>(t->getIndex());
	enum checkReturn csStatus;
	//if (MODE == BIDIR){
		if (hv == 'h'){
			if (x > 0){ // horiz left
				csStatus = segmentAt('h', x - 1, y, w)->checkAndSet(t); 
				if (csStatus == ISTARGET)
					return segmentAt('h', x - 1, y, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('h', x - 1, y, w)); //Segment marked, add to list
			}
			if (x <(N - 1)){ //horiz right
				csStatus = segmentAt('h', x + 1, y, w)->checkAndSet(t);
				if (csStatus == ISTARGET)
					return segmentAt('h', x + 1, y, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('h', x + 1, y, w)); //Segment marked, add to list
			}
			if (y > 0){
				csStatus = segmentAt('v', x , y - 1, w)->checkAndSet(t);
				if (csStatus == ISTARGET)
					return segmentAt('v', x , y - 1, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('v', x, y - 1, w)); //Segment marked, add to list

				csStatus = segmentAt('v', x + 1, y - 1, w)->checkAndSet(t);
				if (csStatus == ISTARGET)
					return segmentAt('v', x + 1, y - 1, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('v', x + 1, y - 1, w)); //Segment marked, add to list
			}
			if (y < N){
				csStatus = segmentAt('v', x, y, w)->checkAndSet(t);
				if (csStatus == ISTARGET)
					return segmentAt('v', x, y, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('v', x, y, w)); //Segment marked, add to list

				csStatus = segmentAt('v', x + 1, y, w)->checkAndSet(t);
				if (csStatus == ISTARGET)
					return segmentAt('v', x + 1, y, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('v', x + 1, y, w)); //Segment marked, add to list

			}

		}
		else{
			if (y > 0){
				csStatus = segmentAt('v', x, y - 1, w)->checkAndSet(t);
				if (csStatus == ISTARGET)
					return segmentAt('v', x, y - 1, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('v', x, y - 1, w)); //Segment marked, add to list
			}
			if (y < (N - 1)){
				csStatus = segmentAt('v', x , y + 1, w)->checkAndSet(t);
				if (csStatus == ISTARGET)
					return segmentAt('v', x , y + 1, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('v', x , y + 1, w)); //Segment marked, add to list
			}
			if (x > 0){
				csStatus = segmentAt('h', x - 1, y, w)->checkAndSet(t);
				if (csStatus == ISTARGET)
					return segmentAt('h', x - 1, y, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('h', x - 1, y, w)); //Segment marked, add to list

				csStatus = segmentAt('h', x - 1, y+1, w)->checkAndSet(t);
				if (csStatus == ISTARGET)
					return segmentAt('h', x - 1, y+1, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('h', x - 1, y+1, w)); //Segment marked, add to list
			}
			if (x<N){
				csStatus = segmentAt('h', x, y, w)->checkAndSet(t);
				if (csStatus == ISTARGET)
					return segmentAt('h', x, y, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('h', x, y, w)); //Segment marked, add to list

				csStatus = segmentAt('h', x, y + 1, w)->checkAndSet(t);
				if (csStatus == ISTARGET)
					return segmentAt('h', x, y + 1, w); //Target Found!
				if (csStatus == OK)
					neigh->push_back(segmentAt('h', x, y+1, w)); //Segment marked, add to list
			}
		}
	//}
	/*else{
		if (hv == 'h'){
			if (w % 2 == 0){ // W->E --->
				if (x < (N - 1)){
					csStatus = segmentAt('h', x + 1, y, w)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('h', x + 1, y, w);
					if (csStatus == OK)
						neigh->push_back(segmentAt('h', x + 1, y, w));
				}
				if (y > 0){
					csStatus = segmentAt('v', x + 1, y - 1, w)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('v', x + 1, y - 1, w); //Target Found!
					if (csStatus == OK)
						neigh->push_back(segmentAt('v', x + 1, y - 1, w)); //Segment marked, add to list
				}
				if (y < N){
					csStatus = segmentAt('v', x + 1, y, w+1)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('v', x + 1, y, w+1); //Target Found!
					if (csStatus == OK)
						neigh->push_back(segmentAt('v', x + 1, y, w+1)); //Segment marked, add to list
				}
			}
			else{ // E->W <----
				if (x > 0){ // horiz left
					csStatus = segmentAt('h', x - 1, y, w)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('h', x - 1, y, w); //Target Found!
					if (csStatus == OK)
						neigh->push_back(segmentAt('h', x - 1, y, w)); //Segment marked, add to list
				}
				if (y > 0){
					csStatus = segmentAt('v', x, y - 1, w-1)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('v', x, y - 1, w-1); //Target Found!
					if (csStatus == OK)
						neigh->push_back(segmentAt('v', x, y - 1, w-1)); //Segment marked, add to list
				}
				if (y < N){
					csStatus = segmentAt('v', x, y, w)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('v', x, y, w); //Target Found!
					if (csStatus == OK)
						neigh->push_back(segmentAt('v', x, y, w)); //Segment marked, add to list
				}
			}
		}
		else {
			if (w % 2 == 0){ // N->S V
				if (y > 0){
					csStatus = segmentAt('v', x, y - 1, w)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('v', x, y - 1, w); //Target Found!
					if (csStatus == OK)
						neigh->push_back(segmentAt('v', x, y - 1, w)); //Segment marked, add to list
				}
			
				if (x > 0){
					csStatus = segmentAt('h', x - 1, y, w+1)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('h', x - 1, y, w+1); //Target Found!
					if (csStatus == OK)
						neigh->push_back(segmentAt('h', x - 1, y, w+1)); //Segment marked, add to list
				}
				if (x<N){
					csStatus = segmentAt('h', x, y, w)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('h', x, y, w); //Target Found!
					if (csStatus == OK)
						neigh->push_back(segmentAt('h', x, y, w)); //Segment marked, add to list
				}
			}
			else{ // S->N ^
				if (y < (N - 1)){
					csStatus = segmentAt('v', x, y + 1, w)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('v', x, y + 1, w); //Target Found!
					if (csStatus == OK)
						neigh->push_back(segmentAt('v', x, y + 1, w)); //Segment marked, add to list
				}
				if (x > 0){
					csStatus = segmentAt('h', x - 1, y + 1, w)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('h', x - 1, y + 1, w); //Target Found!
					if (csStatus == OK)
						neigh->push_back(segmentAt('h', x - 1, y + 1, w)); //Segment marked, add to list
				}
				if (x<N){
					csStatus = segmentAt('h', x, y + 1, w-1)->checkAndSet(t);
					if (csStatus == ISTARGET)
						return segmentAt('h', x, y + 1, w-1); //Target Found!
					if (csStatus == OK)
						neigh->push_back(segmentAt('h', x, y + 1, w-1)); //Segment marked, add to list
				}
			}

		}

	}*/
	return nullptr;
}

bool Channel::route(pin src, pin dest){
	//Mark available destination segments as targets
	bool isOK = false;
	for (int i = 0; i < W; i++){
		Segment* seg = segmentAt(dest, i);
		if (seg->isUnused()){
			seg->setState(TARGET);
			isOK = true;
		}
	}

	//No destination segments available - return a failure
	if (!isOK)
		return false;
	else isOK = false;

//	event_loop(NULL, NULL, NULL, drawscreen);
	std::list<Segment*> expansion;
	std::list<Segment*> temp;

	//Test source segments
	for (int i = 0; i < W; i++){
		Segment* seg = segmentAt(src, i);
		//Segment is also a target segment - done!
		if (seg->isTarget()){
			seg->setSource(src);
			seg->setState(USED);
			seg->setDest(dest);
			return true;
		}
		//Set unused or common-source segments to l=0 and add to list
		if (seg->isUnused() || (seg->isUsed() && seg->getSource() == src)){
			isOK = true;
			seg->setLength(0);
			seg->setSource(src);
			expansion.push_back(seg);
			if (seg->isUnused()) seg->setState(ROUTING);
		}
	}
	//No available segments next to source pin
	if (!isOK)
		return false;

	Segment* toProcess, * trace=nullptr;
	while (!expansion.empty() && trace == nullptr){
//		event_loop(NULL, NULL, NULL, drawscreen);
		toProcess = expansion.front();
		expansion.pop_front();
		trace=findSetAvailableNeighbours(toProcess, &temp);
		if (tryHard) temp=randomizeList(temp);
		expansion.splice(expansion.end(), temp);
	}
	if (trace == nullptr) return false; // No path from source to dest found

	//traceback
	trace->setSource(src);
	trace->setDest(dest);
	traceback(trace);


	//event_loop(NULL, NULL, NULL, drawscreen);
	return true;

}
Segment* Channel::segmentAt(pin p, int w){
	switch (std::get<2>(p)){
	case 1:
		return &(horiz[std::get<0>(p)][std::get<1>(p)][w]);
	case 2:
		return &(vert[std::get<0>(p)+1][std::get<1>(p)][w]);
	case 3:
		return &(horiz[std::get<0>(p)][std::get<1>(p)+1][w]);
	case 4:
		return &(vert[std::get<0>(p)][std::get<1>(p)][w]);
	default:
		return nullptr;
	}

}
Segment* Channel::segmentAt(bool horiz, int x, int y, int w){
	if (horiz)
		return &(this->horiz[x][y][w]);
	else
		return &(vert[x][y][w]);
}
Segment* Channel::segmentAt(char hv, int x, int y, int w){
	if (hv=='h')
		return &(this->horiz[x][y][w]);
	else
		return &(vert[x][y][w]);
}

Segment* Channel::segmentAt(wire w){
	if (std::get<0>(w) == 'h')
		return &(horiz[std::get<1>(w)][std::get<2>(w)][std::get<3>(w)]);
	else
		return &(vert[std::get<1>(w)][std::get<2>(w)][std::get<3>(w)]);
}


void Channel::traceback(Segment * dest){
	pin src = dest->getSource();
	//int len = t->getLength();
	char hv = std::get<0>(dest->getIndex());
	int x = std::get<1>(dest->getIndex());
	int y = std::get<2>(dest->getIndex());
	int w = std::get<3>(dest->getIndex());
	int l = dest->getLength();
	if (l == 0) return;

	//if (MODE == BIDIR){
		if (hv == 'h'){
		//First pass - look for already used segments
			if (x > 0 && segmentAt('h', x - 1, y, w)->getLength() == l - 1 
					&& segmentAt('h', x - 1, y, w)->isUsed()){
				traceback(segmentAt('h', x - 1, y, w));
			}
			else if (x < (N - 1) && segmentAt('h', x + 1, y, w)->getLength() == l - 1
					&& segmentAt('h', x + 1, y, w)->isUsed()){
				traceback(segmentAt('h', x + 1, y, w));
			}
			else if (y > 0 && segmentAt('v', x, y - 1, w)->getLength() == l - 1
					&& segmentAt('v', x, y - 1, w)->isUsed()){
				traceback(segmentAt('v', x, y - 1, w));
			}
			else if (y > 0 && segmentAt('v', x + 1, y - 1, w)->getLength() == l - 1
					&& segmentAt('v', x + 1, y - 1, w)->isUsed()){
				traceback(segmentAt('v', x + 1, y - 1, w));
			}
			else if (y < N && segmentAt('v', x, y, w)->getLength() == l - 1
					&& segmentAt('v', x, y, w)->isUsed()){
				traceback(segmentAt('v', x, y, w));
			}
			else if (y < N && segmentAt('v', x + 1, y, w)->getLength() == l - 1
					&& segmentAt('v', x + 1, y, w)->isUsed()){
				traceback(segmentAt('v', x + 1, y, w));
			}
		//Second Pass - take any route
			else if (x > 0 && segmentAt('h', x - 1, y, w)->getLength() == l - 1){
				segmentAt('h', x - 1, y, w)->setState(USED);
				segmentAt('h', x - 1, y, w)->setSource(src);
				traceback(segmentAt('h', x - 1, y, w));
			}
			else if (x < (N - 1) && segmentAt('h', x + 1, y, w)->getLength() == l - 1){
				segmentAt('h', x + 1, y, w)->setState(USED);
				segmentAt('h', x + 1, y, w)->setSource(src);
				traceback(segmentAt('h', x + 1, y, w));
			}
			else if (y > 0 && segmentAt('v', x, y - 1, w)->getLength() == l - 1){
				segmentAt('v', x, y - 1, w)->setState(USED);
				segmentAt('v', x, y - 1, w)->setSource(src);
				traceback(segmentAt('v', x, y - 1, w));
			}
			else if (y > 0 && segmentAt('v', x + 1, y - 1, w)->getLength() == l - 1){
				segmentAt('v', x + 1, y - 1, w)->setState(USED);
				segmentAt('v', x + 1, y - 1, w)->setSource(src);
				traceback(segmentAt('v', x + 1, y - 1, w));
			}
			else if (y < N && segmentAt('v', x, y, w)->getLength() == l - 1){
				segmentAt('v', x, y, w)->setState(USED);
				segmentAt('v', x, y, w)->setSource(src);
				traceback(segmentAt('v', x, y, w));
			}
			else if (y < N && segmentAt('v', x + 1, y, w)->getLength() == l - 1){
				segmentAt('v', x + 1, y, w)->setState(USED);
				segmentAt('v', x + 1, y, w)->setSource(src);
				traceback(segmentAt('v', x + 1, y, w));
			}
		}
		else{
			//First Pass - try to reuse segements
			if (y > 0 && segmentAt('v', x, y - 1, w)->getLength() == l - 1
					&& segmentAt('v', x, y - 1, w)->isUsed()){
				traceback(segmentAt('v', x, y - 1, w));
			}
			else if (y < (N - 1) && segmentAt('v', x, y + 1, w)->getLength() == l - 1
					&& segmentAt('v', x, y + 1, w)->isUsed()){
				traceback(segmentAt('v', x, y + 1, w)); 
			}
			else if (x > 0 && segmentAt('h', x - 1, y, w)->getLength() == l - 1
					&& segmentAt('h', x - 1, y, w)->isUsed()){
				traceback(segmentAt('h', x - 1, y, w));
			}
			else if (x>0 && segmentAt('h', x - 1, y + 1, w)->getLength() == l - 1
					&& segmentAt('h', x - 1, y + 1, w)->isUsed()){
				traceback(segmentAt('h', x - 1, y + 1, w));
			}
			else if (x < N && segmentAt('h', x, y, w)->getLength() == l - 1
					&& segmentAt('h', x, y, w)->isUsed()){
				traceback(segmentAt('h', x, y, w));
			}
			else if (x<N && segmentAt('h', x, y + 1, w)->getLength() == l - 1
					&& segmentAt('h', x, y + 1, w)->isUsed()){
				traceback(segmentAt('h', x, y + 1, w));
			}
			//Second Pass - no segments to reuse
			if (y > 0 && segmentAt('v', x, y - 1, w)->getLength() == l - 1){
				segmentAt('v', x, y - 1, w)->setState(USED);
				segmentAt('v', x, y - 1, w)->setSource(src);
				traceback(segmentAt('v', x, y - 1, w));
			}
			else if (y < (N - 1) && segmentAt('v', x, y + 1, w)->getLength() == l - 1){
				segmentAt('v', x, y + 1, w)->setState(USED);
				segmentAt('v', x, y + 1, w)->setSource(src);
				traceback(segmentAt('v', x, y + 1, w));
			}
			else if (x > 0 && segmentAt('h', x - 1, y, w)->getLength() == l - 1){
				segmentAt('h', x - 1, y, w)->setState(USED);
				segmentAt('h', x - 1, y, w)->setSource(src);
				traceback(segmentAt('h', x - 1, y, w));
			}
			else if (x>0 && segmentAt('h', x - 1, y + 1, w)->getLength() == l - 1){
				segmentAt('h', x - 1, y + 1, w)->setState(USED);
				segmentAt('h', x - 1, y + 1, w)->setSource(src);
				traceback(segmentAt('h', x - 1, y + 1, w));
			}
			else if (x < N && segmentAt('h', x, y, w)->getLength() == l - 1){
				segmentAt('h', x, y, w)->setState(USED);
				segmentAt('h', x, y, w)->setSource(src);
				traceback(segmentAt('h', x, y, w));
			}
			else if (x<N && segmentAt('h', x, y + 1, w)->getLength() == l - 1){
				segmentAt('h', x, y + 1, w)->setState(USED);
				segmentAt('h', x, y + 1, w)->setSource(src);
				traceback(segmentAt('h', x, y + 1, w));
			}
		}
	//}
	/*else{
		//The unidirectional case!
		if (hv == 'h'){
			if (w % 2 == 1){ // W->E --->
				//First Pass - try to reuse
				if (x < (N - 1) && segmentAt('h', x + 1, y, w)->getLength() == l - 1
					&& segmentAt('h', x + 1, y, w)->isUsed()){
					traceback(segmentAt('h', x + 1, y, w));
				}
				else if (y > 0 && segmentAt('v', x + 1, y - 1, w)->getLength() == l - 1
					&& segmentAt('v', x + 1, y - 1, w)->isUsed()){
					traceback(segmentAt('v', x + 1, y - 1, w)); //Segment marked, add to list
				}
				else if (y < N && segmentAt('v', x + 1, y, w - 1)->getLength() == l - 1
					&& segmentAt('v', x + 1, y, w - 1)->isUsed()){
					traceback(segmentAt('v', x + 1, y, w - 1)); //Segment marked, add to list
				}
				//Second pass
				else if (x < (N - 1) && segmentAt('h', x + 1, y, w)->getLength() == l - 1){
					segmentAt('h', x + 1, y, w)->setState(USED);
					segmentAt('h', x + 1, y, w)->setSource(src);
					traceback(segmentAt('h', x + 1, y, w));
				}
				else if (y > 0 && segmentAt('v', x + 1, y - 1, w)->getLength() == l - 1){
					segmentAt('v', x + 1, y - 1, w)->setState(USED);
					segmentAt('v', x + 1, y - 1, w)->setSource(src);
					traceback(segmentAt('v', x + 1, y - 1, w)); //Segment marked, add to list
				}
				else if (y < N && segmentAt('v', x + 1, y, w - 1)->getLength() == l - 1){
					segmentAt('v', x + 1, y, w - 1)->setState(USED);
					segmentAt('v', x + 1, y, w - 1)->setSource(src);
					traceback(segmentAt('v', x + 1, y, w - 1)); //Segment marked, add to list
				}
			}
			else{ // E->W <----
				if (x > 0 && segmentAt('h', x - 1, y, w)->getLength() == l - 1
					&& segmentAt('h', x - 1, y, w)->isUsed()){
					traceback(segmentAt('h', x - 1, y, w)); //Segment marked, add to list
				}
				else if (y > 0 && segmentAt('v', x, y - 1, w + 1)->getLength() == l - 1
					&& segmentAt('v', x, y - 1, w + 1)->isUsed()){
					traceback(segmentAt('v', x, y - 1, w + 1)); //Segment marked, add to list
				}
				else if (y < N && segmentAt('v', x, y, w)->getLength() == l - 1
					&& segmentAt('v', x, y, w)->isUsed()){
					traceback(segmentAt('v', x, y, w)); //Segment marked, add to list
				}
				else if (x > 0 && segmentAt('h', x - 1, y, w)->getLength() == l - 1){
					segmentAt('h', x - 1, y, w)->setState(USED);
					segmentAt('h', x - 1, y, w)->setSource(src);
					traceback(segmentAt('h', x - 1, y, w)); //Segment marked, add to list
				}
				else if (y > 0 && segmentAt('v', x, y - 1, w + 1)->getLength() == l - 1){
					segmentAt('v', x, y - 1, w + 1)->setState(USED);
					segmentAt('v', x, y - 1, w + 1)->setSource(src);
					traceback(segmentAt('v', x, y - 1, w + 1)); //Segment marked, add to list
				}
				else if (y < N && segmentAt('v', x, y, w)->getLength() == l - 1){
					segmentAt('v', x, y, w)->setState(USED);
					segmentAt('v', x, y, w)->setSource(src);
					traceback(segmentAt('v', x, y, w)); //Segment marked, add to list
				}
			}
		}
		else {
			if (w % 2 == 1){ // N->S V
				if (y > 0 && segmentAt('v', x, y - 1, w)->getLength() == l - 1
					&& segmentAt('v', x, y - 1, w)->isUsed()){
					traceback(segmentAt('v', x, y - 1, w)); //Segment marked, add to list
				}

				else if (x > 0 && segmentAt('h', x - 1, y, w - 1)->getLength() == l - 1
					&& segmentAt('h', x - 1, y, w - 1)->isUsed()){
					traceback(segmentAt('h', x - 1, y, w - 1)); //Segment marked, add to list
				}
				else if (x<N && segmentAt('h', x, y, w)->getLength() == l - 1
					&& segmentAt('h', x, y, w)->isUsed()){
					traceback(segmentAt('h', x, y, w)); //Segment marked, add to list
				}
				else if (y > 0 && segmentAt('v', x, y - 1, w)->getLength() == l - 1){
					segmentAt('v', x, y - 1, w)->setState(USED);
					segmentAt('v', x, y - 1, w)->setSource(src);
					traceback(segmentAt('v', x, y - 1, w)); //Segment marked, add to list
				}

				else if (x > 0 && segmentAt('h', x - 1, y, w - 1)->getLength() == l - 1){
					segmentAt('h', x - 1, y, w - 1)->setState(USED);
					segmentAt('h', x - 1, y, w - 1)->setSource(src);
					traceback(segmentAt('h', x - 1, y, w - 1)); //Segment marked, add to list
				}
				else if (x<N && segmentAt('h', x, y, w)->getLength() == l - 1){
					segmentAt('h', x, y, w)->setState(USED);
					segmentAt('h', x, y, w)->setSource(src);
					traceback(segmentAt('h', x, y, w)); //Segment marked, add to list
				}
			}
			else{ // S->N ^
				if (y < (N - 1) && segmentAt('v', x, y + 1, w)->getLength() == l - 1
					&& segmentAt('v', x, y + 1, w)->isUsed()){
					traceback(segmentAt('v', x, y + 1, w)); //Segment marked, add to list
				}
				else if (x > 0 && segmentAt('h', x - 1, y + 1, w)->getLength() == l - 1
					&& segmentAt('h', x - 1, y + 1, w)->isUsed()){
					traceback(segmentAt('h', x - 1, y + 1, w)); //Segment marked, add to list
				}
				else if (x<N && segmentAt('h', x, y + 1, w + 1)->getLength() == l - 1
					&& segmentAt('h', x, y + 1, w + 1)->isUsed()){
					traceback(segmentAt('h', x, y + 1, w + 1)); //Segment marked, add to list
				}
				else if (y < (N - 1) && segmentAt('v', x, y + 1, w)->getLength() == l - 1){
					segmentAt('v', x, y + 1, w)->setState(USED);
					segmentAt('v', x, y + 1, w)->setSource(src);
					traceback(segmentAt('v', x, y + 1, w)); //Segment marked, add to list
				}
				else if (x > 0 && segmentAt('h', x - 1, y + 1, w)->getLength() == l - 1){
					segmentAt('h', x - 1, y + 1, w)->setState(USED);
					segmentAt('h', x - 1, y + 1, w)->setSource(src);
					traceback(segmentAt('h', x - 1, y + 1, w)); //Segment marked, add to list
				}
				else if (x<N && segmentAt('h', x, y + 1, w + 1)->getLength() == l - 1){
					segmentAt('h', x, y + 1, w + 1)->setState(USED);
					segmentAt('h', x, y + 1, w + 1)->setSource(src);
					traceback(segmentAt('h', x, y + 1, w + 1)); //Segment marked, add to list
				}
			}

		}
	}*/
	return;
}


int Channel::routingSegmentsUsed() {
	int count = 0;
	for (int i = 0; i < N; i ++ ) {
		for (int j = 0; j < N + 1; j++) {
			for (int k = 0; k < W; k++) {
				if (vert[j][i][k].isUsed()) count++;
				if (horiz[i][j][k].isUsed()) count++;
			}
		}
	}
	return count;
}

int Channel::maxW() {
	int count = 0;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N + 1; j++) {
			int temp = 0;
			for (int k = 0; k < W; k++) {
				if (horiz[i][j][k].isUsed()) temp++;
			}
			if (temp > count) count = temp;
		}
	}				
	for (int i = 0; i < N+1; i++) {
		for (int j = 0; j < N; j++) {
			int temp = 0;
			for (int k = 0; k < W; k++) {
				if (vert[i][j][k].isUsed()) temp++;
			}
			if (temp > count) count = temp;
		}
	}

	return count;
}
void Channel::tryHarder(bool b){
	tryHard=b;
}


void drawSwitchConnections(bool isHoriz, int x, int y, int w){

	Segment * seg = utilvars::routing->segmentAt(isHoriz, x, y, w);
	if (!seg->isUsed()) return;
	
	pin src = seg->getSource();
	bool bp = (src == std::make_tuple(0, 2, 4));
	int subsq = 2 * utilvars::graphw + 1;
	setcolor(utilvars::colormap[src]);

	//if (MODE == BIDIR){
    //If Cross connection
		if (isHoriz){
			//left side
			if (x > 0 && utilvars::routing->segmentAt('h', x - 1, y, w)->getSource() == src && utilvars::routing->segmentAt('h', x - 1, y, w)->isUsed()){
				drawline((2 * x + 1)*subsq, subsq*y * 2 + 2 * w + 1, (x) * 2 * subsq - 1, subsq*y * 2 + 2 * w + 1);
			}
			if (y > 0 && utilvars::routing->segmentAt('v', x, y - 1, w)->getSource() == src && utilvars::routing->segmentAt('v', x, y - 1, w)->isUsed()){
				drawline((2 * x + 1)*subsq, subsq*y * 2 + 2 * w + 1, subsq*x * 2 + 2 * w + 1, (y) * 2 * subsq - 1);
			}
			if (y < utilvars::graphn && utilvars::routing->segmentAt('v', x, y, w)->getSource() == src && utilvars::routing->segmentAt('v', x, y, w)->isUsed()){
				drawline((2 * x + 1)*subsq, subsq*y * 2 + 2 * w + 1, subsq*x * 2 + 2 * w + 1, (2 * y + 1)*subsq);
			}
			//right side
			if (x < (utilvars::graphn - 1) && utilvars::routing->segmentAt('h', x + 1, y, w)->getSource() == src && utilvars::routing->segmentAt('h', x + 1, y, w)->isUsed()){
			//	drawline((x + 1) * 2 * subsq - 1, subsq*y * 2 + 2 * w + 1, (x - 1) * 2 * subsq - 1, subsq*y * 2 + 2 * w + 1);
			}
			if (y > 0 && utilvars::routing->segmentAt('v', x + 1, y - 1, w)->getSource() == src && utilvars::routing->segmentAt('v', x + 1, y - 1, w)->isUsed()){
				drawline((x + 1) * 2 * subsq - 1, subsq*y * 2 + 2 * w + 1, subsq*(x+1) * 2 + 2 * w + 1, (y)* 2 * subsq - 1);
			}
			if (y < utilvars::graphn && utilvars::routing->segmentAt('v', x + 1, y, w)->getSource() == src && utilvars::routing->segmentAt('v', x + 1, y, w)->isUsed()){
				drawline((x + 1) * 2 * subsq - 1, subsq*y * 2 + 2 * w + 1, subsq*(x+1) * 2 + 2 * w + 1, (2 * y + 1)*subsq);
			}

		}
		else{
			if (y > 0 && utilvars::routing->segmentAt('v', x, y - 1, w)->getSource() == src && utilvars::routing->segmentAt('v', x, y - 1, w)->isUsed())
				drawline(subsq*x * 2 + 2 * w + 1, (2 * y + 1)*subsq, subsq*x * 2 + 2 * w + 1, (y)* 2 * subsq - 1);
		}

	//}
    //else Wilton
	/*else{
		if (isHoriz){
			if (w % 2 == 0){ // W->E --->
				if (x < (utilvars::graphn - 1) && utilvars::routing->segmentAt('h', x + 1, y, w)->getSource() == src && utilvars::routing->segmentAt('h', x + 1, y, w)->isUsed()){
					drawline((x + 1) * 2 * subsq - 1, subsq*y * 2 + 2 * w + 1, (2 * x + 3)*subsq, subsq*y * 2 + 2 * w + 1);
				}
				if (y > 0 && utilvars::routing->segmentAt('v', x + 1, y - 1, w)->getSource() == src && utilvars::routing->segmentAt('v', x + 1, y - 1, w)->isUsed()){
					drawline((x + 1) * 2 * subsq - 1, subsq*y * 2 + 2 * w + 1, subsq*(x + 1) * 2 + 2 * w + 1, (y)* 2 * subsq - 1);
				}
				if (y < utilvars::graphn && utilvars::routing->segmentAt('v', x + 1, y, w + 1)->getSource() == src && utilvars::routing->segmentAt('v', x + 1, y, w + 1)->isUsed()){
					drawline((x + 1) * 2 * subsq - 1, subsq*y * 2 + 2 * w + 1, subsq*(x + 1) * 2 + 2 * (w+1) + 1, (2 * y + 1)*subsq);
				}
			}
			else{ // E->W <----
				if (x > 0 && utilvars::routing->segmentAt('h', x - 1, y, w)->getSource() == src && utilvars::routing->segmentAt('h', x - 1, y, w)->isUsed()){
					drawline((2 * x + 1)*subsq, subsq*y * 2 + 2 * w + 1, (x)* 2 * subsq - 1, subsq*y * 2 + 2 * w + 1);
				}
				if (y > 0 && utilvars::routing->segmentAt('v', x, y - 1, w - 1)->getSource() == src && utilvars::routing->segmentAt('v', x, y - 1, w - 1)->isUsed()){
					drawline((2 * x + 1)*subsq, subsq*y * 2 + 2 * w + 1, subsq*x * 2 + 2 * (w-1) + 1, (y)* 2 * subsq - 1);
				}
				if (y < utilvars::graphn && utilvars::routing->segmentAt('v', x, y, w)->getSource() == src && utilvars::routing->segmentAt('v', x, y, w)->isUsed()){
					drawline((2 * x + 1)*subsq, subsq*y * 2 + 2 * w + 1, subsq*x * 2 + 2 * w + 1, (2 * y + 1)*subsq);
				}
			}
		}
		else {
			if (w % 2 == 0){ // N->S V
				if (y > 0 && utilvars::routing->segmentAt('v', x, y - 1, w)->getSource() == src && utilvars::routing->segmentAt('v', x, y - 1, w)->isUsed()){
					drawline(subsq*x * 2 + 2 * w + 1, (2 * y + 1)*subsq, subsq*x * 2 + 2 * w + 1, (y)* 2 * subsq - 1);
				}
				if (x > 0 && utilvars::routing->segmentAt('h', x - 1, y, w + 1)->getSource() == src && utilvars::routing->segmentAt('h', x - 1, y, w + 1)->isUsed()){
					drawline(subsq*x * 2 + 2 * w + 1, (2 * y + 1)*subsq, (x) * 2 * subsq - 1, subsq*y * 2 + 2 * (w+1) + 1);
				}
				if (x<utilvars::graphn && utilvars::routing->segmentAt('h', x, y, w)->getSource() == src && utilvars::routing->segmentAt('h', x, y, w)->isUsed()){
					drawline(subsq*x * 2 + 2 * w + 1, (2 * y + 1)*subsq, (2 * x + 1)*subsq, subsq*y * 2 + 2 * w + 1);
				}
			}
			else{ // S->N ^
				if (y < (utilvars::graphn - 1) && utilvars::routing->segmentAt('v', x, y + 1, w)->getSource() == src && utilvars::routing->segmentAt('v', x, y + 1, w)->isUsed()){
					drawline(subsq*x * 2 + 2 * w + 1, (y + 1) * 2 * subsq - 1, subsq*x * 2 + 2 * w + 1, (2 * y + 3)*subsq);
				}
				if (x > 0 && utilvars::routing->segmentAt('h', x - 1, y + 1, w)->getSource() == src && utilvars::routing->segmentAt('h', x - 1, y + 1, w)->isUsed()){
					drawline(subsq*x * 2 + 2 * w + 1, (y + 1) * 2 * subsq - 1, (x) * 2 * subsq - 1, subsq*(y+1)* 2 + 2 * w + 1);
					}
				if (x<utilvars::graphn && utilvars::routing->segmentAt('h', x, y + 1, w - 1)->getSource() == src && utilvars::routing->segmentAt('h', x, y + 1, w - 1)->isUsed()){
					drawline(subsq*x * 2 + 2 * w + 1, (y + 1) * 2 * subsq - 1, (2 * x + 1)*subsq, subsq*(y+1)* 2 + 2 *(w-1) + 1);
				}
			}
		}
	}*/
}

void drawWireSegment(bool isHoriz, int x, int y, int w, enum color_types c){
	int subsq = 2 * utilvars::graphw + 1;

	setcolor(c);
	if (utilvars::routing->segmentAt(isHoriz, x, y, w)->getState() == ROUTING) setcolor(YELLOW);
	if (utilvars::routing->segmentAt(isHoriz, x, y, w)->getState() == TARGET) setcolor(RED);
	if (utilvars::routing->segmentAt(isHoriz, x, y, w)->getState() == USED) {
		setcolor(utilvars::colormap[utilvars::routing->segmentAt(isHoriz, x, y, w)->getSource()]);
		drawSwitchConnections(isHoriz, x, y, w);
	}
	if (isHoriz){
		drawline((x + 1) * 2 * subsq - 1, subsq*y * 2 + 2 * w + 1, (2 * x + 1)*subsq, subsq*y * 2 + 2 * w + 1);
	}
	else{
		drawline(subsq*x * 2 + 2 * w + 1, (y + 1) * 2 * subsq - 1, subsq*x * 2 + 2 * w + 1, (2 * y + 1)*subsq);
	}

}

void drawPinToWire(pin p, int w, enum color_types c){
	if (c==NUM_COLOR)	setcolor(utilvars::colormap[p]);
	else				setcolor(c);
	int x = std::get<0>(p);
	int y = std::get<1>(p);
	int o = std::get<2>(p);
	int subsq = 2 * utilvars::graphw + 1;

	//fillrect((2 * i + 1)*subsq, (2 * j + 1)*subsq, 2 * (i + 1)*subsq - 1, 2 * (j + 1)*subsq - 1);
	
	switch (o){
	case 1:
		drawline((2 * x + 1)*subsq + 1, (2 * y + 1)*subsq, (2 * x + 1)*subsq + 1, subsq*y * 2 + 2 * w + 1);
		break; 
	case 2:
		drawline(2 * (x + 1)*subsq - 1, 2 * (y + 1)*subsq - 2, subsq*(x+1) * 2 + 2 * w + 1, 2 * (y + 1)*subsq - 2);
		break;
	case 3:
		drawline(2 * (x + 1)*subsq - 2, 2 * (y + 1)*subsq - 1, 2 * (x + 1)*subsq - 2, subsq*(y+1)* 2 + 2 * w + 1 );
		break;
	case 4:
		drawline((2 * x + 1)*subsq, (2 * y + 1)*subsq + 1, subsq*x * 2 + 2 * w + 1, (2 * y + 1)*subsq + 1);
	default:;
	}
}

void drawscreen(){
	//extern int chipn, chipw;
	set_draw_mode(DRAW_NORMAL);
	clearscreen();

	setlinestyle(SOLID);
	setlinewidth(1);

	setcolor(LIGHTGREY);
	
	int subsq = 2 * utilvars::graphw + 1;

	for (int i = 0; i < utilvars::graphn; i++){
		for (int j = 0; j < utilvars::graphn + 1; j++){
			for (int k = 0; k < utilvars::graphw; k++){
				//Draw the wires
				setcolor(LIGHTGREY);
				//drawline(subsq*j * 2 + 2 * k + 1, (i + 1) * 2 * subsq - 1, subsq*j * 2 + 2 * k + 1, (2 * i + 1)*subsq);
				//drawline( (i + 1) * 2 * subsq - 1,subsq*j * 2 + 2 * k + 1, (2 * i + 1)*subsq, subsq*j * 2 + 2 * k + 1);
				drawWireSegment(true, i, j, k, LIGHTGREY);
				drawWireSegment(false, j, i, k, LIGHTGREY);
			}
		}
		for (int j = 0; j < utilvars::graphn; j++){
			//Draw the logic blocks
			setcolor(DARKGREY);
			fillrect((2 * i + 1)*subsq, (2 * j + 1)*subsq, 2 *(i+1)*subsq - 1, 2 *(j+1)*subsq - 1);
			for (int k = 1; k < 5; k++){
				pin p = std::make_tuple(i, j, k);
				bool ok = p == std::make_tuple(0, 2, 4);
				for (int w = 0; w < utilvars::graphw; w++){
					if (utilvars::routing->segmentAt(p, w)->getSource() == p)
						drawPinToWire(p, w, utilvars::colormap[utilvars::routing->segmentAt(p, w)->getSource()]);
					if (utilvars::routing->segmentAt(p, w)->getDest() == p)
						drawPinToWire(p, w, utilvars::colormap[utilvars::routing->segmentAt(p, w)->getSource()]);
						
				}
			}
		}
	}
}

int parseInputFile(char * fname, int * n, int * w, std::list<Connection> * connlist){
	std::ifstream fs(fname);
	string temp;
	int item [6];
	size_t idx = 0;

	if (fs.fail()) {
		cerr << "Error: Couldn't open file \"" << fname << "\"" << endl;
		return -1;
	}

	//get the value for n
	std::getline(fs, temp);
	if (fs.eof()) {
		cerr << "Error: missing parameter 'n' in file \"" << fname << "\"" << endl;
		return -1;
	}
	try{ *n = std::stoi(temp); } //ensure conversion from string to int
	catch (const std::invalid_argument& ia) {
		cerr << "Error: '" << temp << "' is not a valid decimal integer" << endl;
		return -1;
	}
	//cout << "n=" << temp << endl;
	utilvars::graphn = *n;

	//get the value for w
	std::getline(fs, temp);
	if (fs.eof()) {
		cerr << "Error: missing parameter 'w' in file \"" << fname << "\"" << endl;
		return -1;
	}
	try{ *w = std::stoi(temp); }
	catch (const std::invalid_argument& ia) {
		cerr << "Error: '" << temp << "' is not a valid decimal integer" << endl;
		return -1;
	}
	//cout << "w=" << temp << endl;
	utilvars::graphw = *w;

	std::getline(fs, temp);
	while (!fs.eof()){
	//	cout << "'" << temp << "'" << endl;
		for (int i = 0; i < 6; i++){
			try{ item[i] = std::stoi(temp, &idx); }
			catch (const std::invalid_argument& ia) {
				cerr << "Error: not a valid decimal integer" << endl;
				return -1;
			}
			temp.erase(0, idx);
		}
		Connection newconn(item);
		if (newconn.isEOPL()) break;
		connlist->push_back(newconn);
		std::getline(fs, temp);
	}

	fs.close();
	//connlist.unique(&Connection::operator==);
	
	return 0;
}

int main(int argc , char ** argv){

	int chipn, chipw;
	int attempts = 0;
	std::list<Connection> connlist;

	if (argc < 2) {
		cerr << "Error: Missing filename! Use " << argv[0] << " <filename>" << std::endl;
		return -1;
	}

    //Parse the inputfile

	if (parseInputFile(argv[1], &chipn, &chipw, &connlist) != 0) return -1;
	
    printConnList(connlist);


	//Implement Lee Maze router

	//Display the route
    	init_graphics("Maze Routing", WHITE);
	//set_visible_world(utilvars::initial_coords);
	init_world(0, 0, (chipw * 2 + 1)*(2 * chipn + 1), (chipw * 2 + 1)*(2 * chipn + 1));
	std::list<Connection>::iterator iter = connlist.begin();
	utilvars::routing = new Channel(chipn, chipw);

	int cindex = YELLOW + 1;
	for (iter; iter != connlist.end(); iter++){
		//sourcelist.push_back(iter->src());
		utilvars::colormap.emplace(iter->src(), (color_types)cindex);
		cindex++;
		if (cindex%NUM_COLOR==0) cindex = YELLOW + 1;
	}


	iter = connlist.begin();
	while (iter != connlist.end() && attempts<MAXATTEMPTS){
		if (DRAWEACHROUTE)
			event_loop(NULL, NULL, NULL, drawscreen);

		cout << "Attempting Route: ";
		iter->print();

		//bool bar = utilvars::routing->route(iter->src(), iter->dest());

		if (!utilvars::routing->route(iter->src(), iter->dest())){
			attempts++;
			if (attempts >= MAXATTEMPTS/2) utilvars::routing->tryHarder(true);
			string message = "Resetting routing - Attempt #" + std::to_string(attempts + 1) + "...";
			//update_message(message);
			//event_loop(NULL, NULL, NULL, drawscreen);
			popToFront(&connlist, iter);
			iter = connlist.begin();
			utilvars::routing->resetRouting();
		}
		else{
			utilvars::routing->clearAttempt();
			iter++;
			update_message("...");
		}
	}
	if (attempts == MAXATTEMPTS){
		string message = "Could not route after " + std::to_string(MAXATTEMPTS) + " attempts. Giving up.";
		update_message(message.c_str());
	}
	else	update_message("Done!");
	//cout << "Using mode " << ((MODE==BIDIR) ? "Bidir" : "Unidir") << endl;
	cout << "Used " << utilvars::routing->routingSegmentsUsed() << " wire segments." << endl;
	cout << "Widest channel used: " << utilvars::routing->maxW() << endl;
	event_loop(NULL, NULL, NULL, drawscreen);
	return 0;
}
