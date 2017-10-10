#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <list>
#include <tuple>
#include <map>

#include "graphics.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

#define MAXSEGLENGTH 8191

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

extern void drawscreen();
extern std::list<Segment *> randomizeList(std::list<Segment *> l);


namespace utilvars{
	extern int graphn, graphw;
	extern Channel * routing;
	extern std::map<pin, enum color_types> colormap;
}

void printConnList(std::list<Connection> connlist){
	std::list<Connection>::iterator it = connlist.begin();

	for (it; it != connlist.end(); it++){
		it->print();
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
	//utilvars::graphn = *n;

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
	//utilvars::graphw = *w;

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
	return 0;
}
