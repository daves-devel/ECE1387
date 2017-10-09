#include <iostream>
#include <fstream>
#include "graphics.h"

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

int main(int arg , char ** argv){

    //Parse the inputfile

	//Implement Lee Maze router

	//Display the route
	return 0;
}
