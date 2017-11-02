#define DO_FULL_SPREADING 0 //[TODO] change to part 3 or change into an argument

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <list>

#include <vector>
#include <algorithm>
#include <iterator>
#include <math.h>
#include <random>
#include <cfloat>

#include "graphics.h"
#include "SuiteSparse/UMFPACK/Include/umfpack.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

class Block {
private:
	int num;
	std::list<int> nets;
	std::list<std::pair<Block *, double>> connections;

	double x;
	double y;
	bool fixed;
	bool real;
    int grid_index;

public:
	Block();
	Block(int i);
	Block(int i, std::list<int> l);
	Block(int i, double xx, double yy);
	Block(int i, double xx, double yy, std::list<int> l);
	Block(int i, double xx, double yy, std::list<int> l, bool r);

	bool hasNet(int i);
	void addNet(int i);
	void setX(double xx); 
	void setY(double yy);
    void setGridIndex(int gridIndex);
	double getX(); 
	double getY();
	int getIntX(); 
	int getIntY();    
	int getBlockNum();
	void setFixed(bool f=true);
	bool isFixed();
	bool isReal();
	void addConnection(std::pair<Block *, double> c);
	void addConnection(Block * b, double w);
	void deleteConnection(Block * b);
	double getSumWeights();
	double getWeight(Block * b);
	void print();

};

class Net {
private:
	std::list<Block *> blocks;
	int idx;
	double x; 
	double y;
	bool real;


public:
	Net();
	Net(int i);
	Net(int i, bool r);

	void buildBlockList(std::list<Block> * blklst);
	void buildConnections();// std::list<Block> * blklst);
	double HPWL ();
	void print();
	void placeNet();
};

bool isFixed(Block * b);

Net::Net() {
	idx = -1;
	real = true;
}

Net::Net(int i) {
	idx = i;
	real = true;
}
Net::Net(int i, bool r) {
	idx = i;
	real = r;
}

void Net::buildBlockList(std::list<Block> * blklst) {
	for (auto& x: *blklst)
		if (x.hasNet(idx)) blocks.emplace_back(&x);

}
void Net::print() {
	if (idx != -1 && blocks.begin() != blocks.end()) {
		std::list<Block *>::iterator it = blocks.begin();
		cout << "Net " << idx << " is connected to blocks (" << (*it)->getBlockNum();

		for (it++; it != blocks.end(); it++)
			cout << ", " << (*it)->getBlockNum();
		cout << ")" << endl;
	}
}
/* buildConnections
 * for the given net, connect all blocks using the clique model with weights 2/p
 */

void Net::buildConnections(){//std::list<Block> * blklst) {
	double w = 2.0 / blocks.size();

	//for (std::list<Block>::iterator ita = blklst->begin(); ita != blklst->end(); ita++) {
	for (auto& x:blocks){
		for (auto& y:blocks) {
			if (x != y) {
				x->addConnection(y, w);
			}
		}
	}
}

namespace commonvars{
	int maxNetNum = 0;
    int gridSize;
	int numFreeBlocks = 0;
	std::list<Block> allBlocks;
	std::list<Net> allNets;
	std::vector<std::list<Block *>> blocksAt;
	std::list<Block> tempRouting;
	void updateBlocksAt();
	std::list<Block *> getBlocksAt(double x, double y); 
	std::list<Block *> getFreeBlocksAt(double x, double y); 
}

double Net::HPWL(){
	double xmin = commonvars::gridSize;
	double ymin = commonvars::gridSize;
	double xmax = 0; 
	double ymax = 0; 
	
	if (real) {
		for (auto& b : blocks) {
			if (b->getX() < xmin) xmin = b->getX();
			if (b->getX() > xmax) xmax = b->getX();
			if (b->getY() < ymin) ymin = b->getY();
			if (b->getY() > ymax) ymax = b->getY();
		}
		return xmax + ymax - ymin - xmin;
	}
	else return 0;
}

void Net::placeNet() {
	const int size = blocks.size();
	double weight = 2.0 / size;
	std::vector<double> b;

	b.resize(size);
	int i = 0;
	for (auto& x : blocks) {
		if (x->isFixed()) {
			for (int j = 0; j < size; j++)
				if (j != i) b[j] += weight*x->getX();
		}
		i++;
	}
}
        

Block::Block() {
	num = -1;
	x = -1;
	y = -1;
    grid_index = -1;
	fixed = false;
	real = true;
}

Block::Block(int i) {
	num = i;
	x = -1;
	y = -1;
    grid_index =-1;
	fixed = false;
	real = true;
}

Block::Block(int i, std::list<int> l) {
	num = i;
	x = -1;
	y = -1;
    grid_index =-1;
	nets = l;
	fixed = false;
	real = true;

}

Block::Block(int i, double xx, double yy) {
	num = i;
	x = xx; 
    y = yy;
    grid_index =-1;    
	fixed = false;
	real = true;

}

Block::Block(int i, double xx, double yy, std::list<int> l) {
	num = i;
	x = xx; 
	y = yy;
	nets = l;
    grid_index =-1;    
	fixed = false;
	real = true;

}
Block::Block(int i, double xx, double yy, std::list<int> l, bool r) {
	num = i;
	x = xx;
	y = yy;
	nets = l;
    grid_index =-1;    
	fixed = false;
	real = r;

}

bool Block::hasNet(int i) {
	for (std::list<int>::iterator it = nets.begin(); it != nets.end(); it++)
		if (*it == i) return true;
	return false;
}

void Block::addNet(int i) {
	nets.push_back(i);
}

int Block::getBlockNum() { return num; }

void Block::setX(double xx) { x = xx; } 
void Block::setY(double yy) { y = yy; }
void Block::setGridIndex(int gridIndex) { grid_index = gridIndex; }
double Block::getX() { return x; }
double Block::getY() { return y; }
int Block::getIntX() {
    double tempx = x + 0.5;
    int tempIntx = (int)tempx;
    return tempIntx; 
}
int Block::getIntY() {
    double tempy = y + 0.5;
    int tempInty = (int)tempy;
    return tempInty; 
}
void Block::setFixed(bool f) {
	fixed = f;
}
bool Block::isFixed() { return fixed; }
bool Block::isReal() { return real; }

void Block::addConnection(std::pair<Block *, double> c) {
	connections.push_back(c);
}
void Block::addConnection(Block * b, double w) {
	connections.emplace_back(b, w);
}
void Block::deleteConnection(Block * b) {
	std::list<std::pair<Block*, double>>::iterator it = connections.begin();
	while (it != connections.end()) {
		if (it->first == b) {
			connections.erase(it);
			it = connections.begin();
		}
		else it++;
	}
}

double Block::getSumWeights() {
	double sum = 0;
	for (auto& x:connections){
		sum += x.second;
	}
	return sum;
}

double Block::getWeight(Block * b) {
	double sum = 0;
	for (auto& x : connections) {
		if (x.first==b)
			sum += x.second;
	}
	return sum;
}

void Block::print() {
	if (num != -1) {
		//std::list<int>::iterator it = nets.begin();
		cout << "Block num " << num << " x " << x << " y " << y <<" fixed " << fixed << " real " << real << " grid_index " << grid_index << endl; // " is connected to nets (" << *it;

		//for (it++; it != nets.end(); it++)
		//	cout << ", " << *it;
		//cout << ")" << endl;
	}
}


bool isFixed(Block * b) { return b->isFixed(); }


void commonvars::updateBlocksAt() {
	blocksAt.clear();
	blocksAt.resize((commonvars::gridSize + 1) * (commonvars::gridSize + 1));
	for (auto& b : allBlocks) { 
		if (b.getX() != -1 && b.getY() != -1){
            int binIndex = b.getIntX() * (commonvars::gridSize + 1) + b.getIntY();
            b.setGridIndex(binIndex);
			blocksAt[binIndex].push_back(&b);
        }
	}
}

std::list<Block *> commonvars::getBlocksAt(double x, double y) {
    double tempx = x + 0.5;
    double tempy = y + 0.5;
    int tempIntx = int(tempx);
    int tempInty = int(tempy);
    int binIndex = tempIntx * (commonvars::gridSize + 1) + tempInty;
	return blocksAt[binIndex];
}
std::list<Block *> commonvars::getFreeBlocksAt(double x, double y) { 
	std::list <Block *> ret;
    double tempx = x + 0.5;
    double tempy = y + 0.5;
    int tempIntx = int(tempx);
    int tempInty = int(tempy);
    int binIndex = tempIntx * (commonvars::gridSize + 1) + tempInty;    
	for (auto & b : blocksAt[binIndex]) {
		if (!b->isFixed()) {
			ret.emplace_back(b);
		}
	}
	return ret;
}

int parseInputFile(char * fname){
	std::ifstream fs(fname);
	string temp;
	int blocknum;
	int tempint;
	double tempdouble;
	std::list<int> templist;
	size_t idx = 0;

	if (fs.fail()) {
		cerr << "Error: Couldn't open file \"" << fname << "\"" << endl;
		return -1;
	}

	//Read the Blocks + Nets
	do {
		std::getline(fs, temp);
		if (fs.eof()) {
			cerr << "Error: block list in file \"" << fname << "\" should be terminated with -1" << endl;
			return -1;
		}
		//Get block number
		try { blocknum = std::stoi(temp, &idx); } //ensure conversion from string to int
		catch (const std::invalid_argument& ia) {
			cerr << "Error: '" << temp << "' is not a valid decimal integer" << endl;
			return -1;
		}
		temp.erase(0, idx);
		if (blocknum != -1) {
			templist.clear();
			do {
				//Get nets for the block number
				try { tempint = std::stoi(temp, &idx); } //ensure conversion from string to int
				catch (const std::invalid_argument& ia) {
					cerr << "Error: '" << temp << "' is not a valid decimal integer" << endl;
					return -1;
				}
				temp.erase(0, idx);
				if (tempint != -1) templist.push_back(tempint);
				if (tempint > commonvars::maxNetNum) commonvars::maxNetNum = tempint;
			} while (tempint != -1);
			commonvars::allBlocks.emplace_back(blocknum, templist);
		}
	} while (blocknum != -1);
	
	commonvars::numFreeBlocks = commonvars::allBlocks.size();

	//Read the Fixed Blocks along with x and y coordinates
	do {
		std::getline(fs, temp);
		if (fs.eof()) {
			cerr << "Error: block location list in file \"" << fname << "\" should be terminated with -1" << endl;
			return -1;
		}
		//Get block number
		try { blocknum = std::stoi(temp, &idx); } //ensure conversion from string to int
		catch (const std::invalid_argument& ia) {
			cerr << "Error: '" << temp << "' is not a valid decimal integer" << endl;
			return -1;
		}
		temp.erase(0, idx);
		if (blocknum != -1) {
			std::list<Block>::iterator it = commonvars::allBlocks.begin();
			for (; it != commonvars::allBlocks.end() && it->getBlockNum() != blocknum; it++);
			//x position
			try { tempdouble = std::stod(temp, &idx); }
			catch (const std::invalid_argument& ia) {
				cerr << "Error: '" << temp << "' is not a valid decimal integer" << endl;
				return -1;
			}
			it->setX(tempdouble);

			temp.erase(0, idx);
			//y position
			try { tempdouble = std::stod(temp, &idx); }
			catch (const std::invalid_argument& ia) {
				cerr << "Error: '" << temp << "' is not a valid decimal integer" << endl;
				return -1;
			}
			temp.erase(0, idx);
			it->setY(tempdouble);
			it->setFixed();
			commonvars::numFreeBlocks--;
		}
	} while (blocknum != -1);

	fs.close();
	
	return 0;
}//parseInputFile

void initialPlace(std::list<Block> * blocks) {

	std::vector<double> bx;
	std::vector<double> by;
	std::vector<int> Ap;
	std::vector<int> Ai;
	std::vector<double> A;
	std::vector<double> x;
	std::vector<double> y;

	bx.resize(commonvars::numFreeBlocks);
	by.resize(commonvars::numFreeBlocks);
	x.resize(commonvars::numFreeBlocks);
	y.resize(commonvars::numFreeBlocks);
	Ap.resize(commonvars::numFreeBlocks+1);
	Ap[0] = 0;
    
	unsigned int i = 0;
	for (auto& blki:*blocks){
		int j = 0;
		if (!blki.isFixed()) {
			Ap[i + 1] = Ap[i];
			for (auto& blkj : *blocks) {
				if (&blki == &blkj) {
					Ap[i + 1]++;
					Ai.push_back(j);
					A.push_back(blki.getSumWeights());
					j++;
				}
				else if (blkj.isFixed()) {
					bx[i] += blki.getWeight(&blkj)*blkj.getX();
					by[i] += blki.getWeight(&blkj)*blkj.getY();
				}
				else {
					double w = blki.getWeight(&blkj);
					if (w != 0) {
						Ap[i + 1]++;
						Ai.push_back(j);
						A.push_back(-w);
					}
					j++;
				}
			}

			i++;
		}
	}
	void * Symbolic, * Numeric;

	(void)umfpack_di_symbolic(commonvars::numFreeBlocks, commonvars::numFreeBlocks, Ap.data(), Ai.data(), A.data(), &Symbolic, nullptr, nullptr);
	(void)umfpack_di_numeric(Ap.data(), Ai.data(), A.data(), Symbolic, &Numeric, nullptr, nullptr);
	umfpack_di_free_symbolic(&Symbolic);
	(void)umfpack_di_solve(UMFPACK_A, Ap.data(), Ai.data(), A.data(), x.data(), bx.data(), Numeric, nullptr, nullptr);
	(void)umfpack_di_solve(UMFPACK_A, Ap.data(), Ai.data(), A.data(), y.data(), by.data(), Numeric, nullptr, nullptr);
	umfpack_di_free_numeric(&Numeric);
    i=0;
    for (auto& b:*blocks){
        if (!b.isFixed()){
            b.setX(x[i]);
            b.setY(y[i]);
            i++;
        }
    }
	commonvars::updateBlocksAt();
	
}//initialPlace

/*
void simpleSpreading() {

	double x = 0; 
	double y = 0;
	std::list<Block *> blst;
    std::list<Block *> hblst;
    std::list<Block *> vblst;
	double virtualWeight = commonvars::maxNetNum * 30.0 /commonvars::allBlocks.size();

    //create a list of blocks
	for (x = 0; x <= commonvars::gridSize; x++) { 
		for (y = 0; y <= commonvars::gridSize; y++) { 
			blst = commonvars::getFreeBlocksAt(x, y);
        }
    }

    struct CompareBlockX{
        bool operator()(Block * lhs, Block * rhs) {return lhs->x < rhs->x;}
    };

    struct CompareBlockY{
        bool operator()(Block * lhs, Block * rhs) {return lhs->y < rhs->y;}
    };
    
    //sort list vertically

    //sort list horizontally


 //Q1
	blst.clear();
	for (double i = 0; i < x; i++) {
		for (double j = 0; j < y; j++) {
			blst.splice(blst.end(), commonvars::getBlocksAt(i, j));
		}
	}
	blst.remove_if(isFixed);
	for (auto& b : blst) {
		ilst.push_back(b->getBlockNum());
	}
	commonvars::allBlocks.emplace_back(commonvars::allBlocks.size() + 1, (commonvars::gridSize / 4), (commonvars::gridSize / 4) , ilst, false);
	commonvars::allBlocks.back().setFixed();
	for (auto & b : blst) {
		b->addConnection(&commonvars::allBlocks.back(), virtualWeight);
	}
//Q2
	blst.clear();
	ilst.clear();
	for (double i = x; i < (commonvars::gridSize + 1); i++) {
		for (double j = 0; j < y; j++) {
			blst.splice(blst.end(), commonvars::getBlocksAt(i, j));
		}
	}
	blst.remove_if(isFixed);
	for (auto& b : blst) {
		ilst.push_back(b->getBlockNum());
	}
	commonvars::allBlocks.emplace_back(commonvars::allBlocks.size() + 1,  (commonvars::gridSize / 4) * 3,  (commonvars::gridSize / 4), ilst, false);
	commonvars::allBlocks.back().setFixed();
	for (auto & b : blst) {
		b->addConnection(&commonvars::allBlocks.back(), virtualWeight);
	}
//Q3
	blst.clear();
	ilst.clear();
	for (double i = 0; i < x; i++) {
		for (double j = y; j < (commonvars::gridSize + 1); j++) {
			blst.splice(blst.end(), commonvars::getBlocksAt(i, j));
		}
	}
	blst.remove_if(isFixed);
	for (auto& b : blst) {
		ilst.push_back(b->getBlockNum());
	}
	commonvars::allBlocks.emplace_back(commonvars::allBlocks.size() + 1,  (commonvars::gridSize / 4),  (commonvars::gridSize / 4) * 3, ilst, false);
	commonvars::allBlocks.back().setFixed();
	for (auto & b : blst) {
		b->addConnection(&commonvars::allBlocks.back(), virtualWeight);
	}
//Q4
	blst.clear();
	ilst.clear();
	for (double i = x; i < (commonvars::gridSize + 1); i++) { 
		for (double j = y; j < (commonvars::gridSize + 1); j++) { 
			blst.splice(blst.end(), commonvars::getBlocksAt(i, j));
		}
	}
	blst.remove_if(isFixed);
	for (auto& b : blst) {
		ilst.push_back(b->getBlockNum());
	}
	commonvars::allBlocks.emplace_back(commonvars::allBlocks.size() + 1,  (commonvars::gridSize / 4) * 3,  (commonvars::gridSize / 4) * 3, ilst, false);
	commonvars::allBlocks.back().setFixed();
	for (auto & b : blst) {
		b->addConnection(&commonvars::allBlocks.back(), virtualWeight);
	}
	initialPlace(&commonvars::allBlocks)

}
*/

void simpleOverlap() {
	int n = commonvars::numFreeBlocks;
	double x = 0; 
	double y = 0;
	int sum = 0;
	std::list<Block *> blst;
	std::list<int> ilst;
	double virtualWeight = commonvars::maxNetNum * 10.0 /commonvars::allBlocks.size();

/*
    while (sum*1.0 / n < 0.25) {
	  for (int i = 0; i < (commonvars::gridSize + 1); i++) {
        for (int j = 0; j < (commonvars::gridSize + 1); j++) {
			blst = commonvars::getBlocksAt(i, j);
			blst.remove_if(isFixed);
			sum += blst.size();
		}
        y++;
      }
        
        x++;              
	}
*/

    while (sum*1.0 / n < 0.5) {
		for (double i = 0; i < (commonvars::gridSize + 1); i++) {
			blst = commonvars::getBlocksAt(x, i);
			blst.remove_if(isFixed);
			sum += blst.size();
		}
		    x++;
	}
	sum = 0;
	while (sum*1.0 / n < 0.5) {
		for (double i = 0; i < (commonvars::gridSize + 1); i++) { 
			blst = commonvars::getBlocksAt(i, y);
			blst.remove_if(isFixed);
			sum += blst.size();
		}
		    y++;
	}

    x = x - 1;
	cout << "x = " << x << "; y = " << y << endl;

//Q1
	blst.clear();

	for (double i = 0; i < x; i++) {
		for (double j = 0; j < y; j++) {
			blst.splice(blst.end(), commonvars::getBlocksAt(i, j));
		}
	}
	blst.remove_if(isFixed);
	for (auto& b : blst) {
		ilst.push_back(b->getBlockNum());
	}
	commonvars::allBlocks.emplace_back(commonvars::allBlocks.size() + 1, (commonvars::gridSize / 4), (commonvars::gridSize / 4) , ilst, false);
	commonvars::allBlocks.back().setFixed();
	for (auto & b : blst) {
		b->addConnection(&commonvars::allBlocks.back(), virtualWeight);
	}
//Q2
	blst.clear();
	ilst.clear();
	for (double i = x; i < (commonvars::gridSize + 1); i++) {
		for (double j = 0; j < y; j++) {
			blst.splice(blst.end(), commonvars::getBlocksAt(i, j));
		}
	}
	blst.remove_if(isFixed);
	for (auto& b : blst) {
		ilst.push_back(b->getBlockNum());
	}
	commonvars::allBlocks.emplace_back(commonvars::allBlocks.size() + 1,  (commonvars::gridSize / 4) * 3,  (commonvars::gridSize / 4), ilst, false);
	commonvars::allBlocks.back().setFixed();
	for (auto & b : blst) {
		b->addConnection(&commonvars::allBlocks.back(), virtualWeight);
	}
//Q3
	blst.clear();
	ilst.clear();
	for (double i = 0; i < x; i++) {
		for (double j = y; j < (commonvars::gridSize + 1); j++) {
			blst.splice(blst.end(), commonvars::getBlocksAt(i, j));
		}
	}
	blst.remove_if(isFixed);
	for (auto& b : blst) {
		ilst.push_back(b->getBlockNum());
	}
	commonvars::allBlocks.emplace_back(commonvars::allBlocks.size() + 1,  (commonvars::gridSize / 4),  (commonvars::gridSize / 4) * 3, ilst, false);
	commonvars::allBlocks.back().setFixed();
	for (auto & b : blst) {
		b->addConnection(&commonvars::allBlocks.back(), virtualWeight);
	}
//Q4
	blst.clear();
	ilst.clear();
	for (double i = x; i < (commonvars::gridSize + 1); i++) { 
		for (double j = y; j < (commonvars::gridSize + 1); j++) { 
			blst.splice(blst.end(), commonvars::getBlocksAt(i, j));
		}
	}
	blst.remove_if(isFixed);
	for (auto& b : blst) {
		ilst.push_back(b->getBlockNum());
	}
	commonvars::allBlocks.emplace_back(commonvars::allBlocks.size() + 1,  (commonvars::gridSize / 4) * 3,  (commonvars::gridSize / 4) * 3, ilst, false);
	commonvars::allBlocks.back().setFixed();
	for (auto & b : blst) {
		b->addConnection(&commonvars::allBlocks.back(), virtualWeight);
	}
	initialPlace(&commonvars::allBlocks);

}

double wireusage (std::list<Net> * nets){
    double sum = 0;
    for(auto& n:*nets){
        sum += n.HPWL();
    }
    return sum;
}

void removeVirtualBlocks(std::list<Block> * blocks) {
	std::list<Block>::iterator it;
	for (it = blocks->begin(); it != blocks->end(); it++) {
		if (!it->isReal()){
			for (auto& b : *blocks) {
				b.deleteConnection(&(*it));
			}
			std::list<Block>::iterator temp = it;
			blocks->erase(temp);
			it = blocks->begin();
		}
	}

}

void removeFixedBlocks(std::list<Block *> * blocks) {
    if (blocks->size() !=0){
        std::list<Block*>::iterator it = blocks->begin();
        while (blocks->size() != 0 && it != blocks->end()){
            if ((*it)->isFixed()){
                blocks->erase(it);
                it = blocks->begin();
            }
            else it++;
        }
    }
}

void recurseRemoveOverlap(std::list<Block> * blocks, int i) {
	removeVirtualBlocks(blocks);
	const int n = 1 << i;
	double virtualWeight = commonvars::maxNetNum * 2.0 / commonvars::allBlocks.size();

	std::vector<std::list<Block*>> blocksInRegion;
	std::list<Block *> templst;
	blocksInRegion.resize(n*n);

	std::vector<double> xp; 
	std::vector<double> yp; 
	xp.resize(n);
	yp.resize(n);

	int sum = 0;
	int j = 0, k=0;
	double x, y;

	for (x = 0; x <= commonvars::gridSize; x++) { 
		for (y = 0; y <= commonvars::gridSize; y++) { 
			templst = commonvars::getFreeBlocksAt(x, y);
			sum += templst.size();
		}
		if (sum >= commonvars::numFreeBlocks*(j + 1) / n) {
			xp[j] = x;
			j++;
		}
	}
	j = 0;
	sum = 0;
	for (y = 0; y <= commonvars::gridSize; y++) {
		for (x = 0; x <= commonvars::gridSize; x++) { 
			templst = commonvars::getBlocksAt(x, y);
			removeFixedBlocks(&templst);
			sum += templst.size();
		}
		if (sum >= commonvars::numFreeBlocks*(j + 1) / n) {
			yp[j] = y;
			j++;
		}
	}
	j = 0;

	for (x = 0; x <= commonvars::gridSize; x++) {
		if (x > xp[j] && j < n-1) j++; 
		k = 0;
		for (y = 0; y <= commonvars::gridSize; y++) {
			if (y > yp[k] && k < n - 1) k++;
			blocksInRegion[j*n + k].splice(blocksInRegion[j*n + k].end(), commonvars::getBlocksAt(x, y));
		}
	}
	
	sum = 0;
	for (x = 0; x < n; x++) {
		for (y = 0; y < n; y++) {
			blocksInRegion[x*n + y].remove_if(isFixed);
			if (blocksInRegion[x*n + y].size()>2) sum += blocksInRegion[x*n + y].size() - 2;
			std::list<int> u;
			commonvars::allBlocks.emplace_back(commonvars::allBlocks.size() + 1, (commonvars::gridSize * x + 5) / n, (commonvars::gridSize * y + 5) / n, u, false);
			commonvars::allBlocks.back().setFixed();
			for (auto& b : blocksInRegion[x*n + y]) {
				b->addConnection(&commonvars::allBlocks.back(), virtualWeight);
			}
		}
	}

	initialPlace(blocks);

//	event_loop(NULL, NULL, NULL, drawscreen);

	if (sum*1.0/commonvars::numFreeBlocks > 0.15)	recurseRemoveOverlap(blocks, i+1); //[TODO] adjust based on criteria
}

void drawscreen(){
	std::list<Block> * bks;
	if (commonvars::tempRouting.size() != 0) bks = &commonvars::tempRouting;
	else bks = &commonvars::allBlocks;
	set_draw_mode(DRAW_NORMAL);
	clearscreen();

	setlinestyle(SOLID);
	setlinewidth(1);
    setfontsize(24);

	setcolor(BLACK); //[TODO] change colour
	
	for (double i = 0; i < commonvars::gridSize + 1; i++) {
		drawline(i * 10.0, 0.0, i * 10.0, commonvars::gridSize * 10.0); 
		drawline(0, i * 10.0, commonvars::gridSize * 10.0, i * 10.0);
	}

	setcolor(RED);

	for (auto& b : *(bks)) {
		if (b.isReal())
			for (auto& bp : *(bks)) {
				if (b.getWeight(&bp) != 0) {
					if (!bp.isReal()) setcolor(GREEN);
					else setcolor(RED);
					drawline(b.getX() * 10.0 + 5.0, b.getY() * 10.0 + 5.0, bp.getX() * 10.0 + 5.0, bp.getY() * 10.0 + 5.0); //[TODO] adjust to be based on N
				}
			}
	}

	setcolor(BLUE);

	for (auto& b : *(bks)) {
		if (b.isReal()){
            setcolor(BLUE);
		    std::string s = std::to_string(b.getBlockNum());
		    const char * c_string_block_num = s.c_str();
			drawtext(b.getX() * 10.0 + 5.0, b.getY() * 10.0 + 5.0, c_string_block_num,150); //[TODO] adjust to be based on N
		}
		else{
            setcolor(GREEN);
		    std::string s = std::to_string(b.getBlockNum());
		    const char * c_string_block_num = s.c_str();
			drawtext(b.getX() * 10.0 + 5.0, b.getY() * 10.0 + 5.0, c_string_block_num,150); //[TODO] adjust to be based on N
		}

	}
	
}

int main(int argc, char** argv) {


	if (argc < 2) {
		cerr << "Error: Missing filename! Use " << argv[0] << " <filename>" << std::endl;
		return -1;
	}

	//if (argc > 3 && std::string(argv[2]) == "-swap") randomSwaps = atoi(argv[3]); //[TODO] change for recursive
	/*if (argc > 4 && std::string(argv[2]) == "-swap") seed = atoi(argv[4]);
	else {
		std::random_device rd;
		seed = rd();
	}*/

	parseInputFile(argv[1]);
	for (auto& x : commonvars::allBlocks)
		x.print();

	cout << "Found " << commonvars::allBlocks.size() << " blocks." << endl;
	cout << "Found a max of " << commonvars::maxNetNum << " nets." << endl;
    commonvars::gridSize = ceil(sqrt(commonvars::allBlocks.size()));
	cout << "Grid size is " << commonvars::gridSize << endl;

	init_graphics("Assignment 2: AP", WHITE);
	init_world(0, 0, commonvars::gridSize * 10, commonvars::gridSize * 10);

	for (int i = 1; i <= commonvars::maxNetNum; i++) {
		commonvars::allNets.emplace_back(i);
		commonvars::allNets.back().buildBlockList(&commonvars::allBlocks);
//		commonvars::allNets.back().print();
	}

	commonvars::tempRouting.clear();

	for (auto& x : commonvars::allNets) {
		x.buildConnections();// &commonvars::allBlocks);
	}

	initialPlace(&commonvars::allBlocks);
    for (auto& x : commonvars::allBlocks)
		x.print();
    
    cout << "Used "<< wireusage(&commonvars::allNets) << " units of wiring. " << endl;

	event_loop(NULL, NULL, NULL, drawscreen);

	simpleOverlap();

    for (auto& x : commonvars::allBlocks)
		x.print();

	cout << "Used " << wireusage(&commonvars::allNets) << " units of wiring. (basic spread)" << endl;

	event_loop(NULL, NULL, NULL, drawscreen);

#if DO_FULL_SPREADING
	recurseRemoveOverlap(&commonvars::allBlocks, 2);

	cout << "Used " << wireusage(&commonvars::allNets) << " units of wiring. (full spread)" << endl;

	event_loop(NULL, NULL, NULL, drawscreen);
#endif
	return 0;
}
