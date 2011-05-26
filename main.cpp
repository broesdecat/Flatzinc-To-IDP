/*
 * Copyright 2011 Katholieke Universiteit Leuven
 *
 * Use of this software is governed by the GNU LGPLv3.0 license
 *
 * Written by Broes De Cat, K.U.Leuven, Departement
 * Computerwetenschappen, Celestijnenlaan 200A, B-3001 Leuven, Belgium
 */
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include "flatzincsupport/FlatZincMX.hpp"
using namespace std;

/**
 * Print help message
 **/
void usage() {
	cout << "Usage:\n"
		 << "   fz2idp [options] [filename]\n\n";
	cout << "Options:\n";
	cout << "    -v, --version        show version number and stop\n";
	cout << "    -h, --help           show this help message\n\n";
}

/** 
 * Parse command line options 
 **/
string read_options(int argc, char* argv[], bool& fromstdin) {
	string inputfile;
	argc--; argv++;
	int filesfound = 0;
	fromstdin = false;
	while(argc) {
		string str(argv[0]);
		argc--; argv++;
		if(str == "-v" || str == "--version")		{ cout << "fz2fodot 1.0.0\n"; exit(0);		}
		else if(str == "-h" || str == "--help")		{ usage(); exit(0);							}
		else if(str[0] == '-')						{ cerr <<"Unknown option " <<str; exit(0);	}
		else										{ inputfile = str;	filesfound++; 			}
	}
	if(filesfound==0){
		fromstdin = true;
	}else if(filesfound!=1){
		usage(); exit(0);
	}
	return inputfile;
}

int main(int argc, char* argv[]) {
	bool fromstdin = false;
	string inputfile = read_options(argc,argv, fromstdin);

	FZ::FlatZincMX* mx = new FZ::FlatZincMX();
	mx->parse(fromstdin, inputfile);
	mx->writeout();
	delete(mx);
	return 0;
}
