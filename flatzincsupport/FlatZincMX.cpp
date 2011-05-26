/*
 * Copyright 2011 Katholieke Universiteit Leuven
 *
 * Use of this software is governed by the GNU LGPLv3.0 license
 *
 * Written by Broes De Cat, K.U.Leuven, Departement
 * Computerwetenschappen, Celestijnenlaan 200A, B-3001 Leuven, Belgium
 */
#include "flatzincsupport/FlatZincMX.hpp"

#include <cstdio>
#include <iostream>

#include "flatzincsupport/FZDatastructs.hpp"
#include "flatzincsupport/InsertWrapper.hpp"
#include "flatzincsupport/flatzincparser.h"
#include "flatzincsupport/fzexception.hpp"

using namespace std;
using namespace FZ;

extern InsertWrapper* wrapper;
extern FILE* fzin;
extern int fzparse(void);

FlatZincMX::FlatZincMX(): data(new InsertWrapper()) {
	wrapper = data;
}

FlatZincMX::~FlatZincMX() {
	delete data;
}

void FlatZincMX::parse(bool readfromstdin, const std::string& inputfile){
	int result = 0;
	if(readfromstdin){
		fzin = stdin;
		result = fzparse();
	}else{
		fzin = fopen(inputfile.c_str(),"r");
		if(fzin){
			result = fzparse();
			fclose(fzin);
		}else{
			throw fzexception("File could not be opened, aborting.\n");
		}
	}
	if(result!=0){
		throw fzexception("Unspecified parsing error.\n");
	}
}

void FlatZincMX::writeout(){
}
