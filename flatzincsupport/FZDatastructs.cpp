/*
 * Copyright 2011 Katholieke Universiteit Leuven
 *
 * Use of this software is governed by the GNU LGPLv3.0 license
 *
 * Written by Broes De Cat, K.U.Leuven, Departement
 * Computerwetenschappen, Celestijnenlaan 200A, B-3001 Leuven, Belgium
 */
#include "flatzincsupport/FZDatastructs.hpp"

#include <assert.h>

#include "flatzincsupport/fzexception.hpp"

using namespace std;
using namespace FZ;

int tempsymbol = 0;

string FZ::createSymbol(){
	stringstream ss;
	ss <<"temp" <<tempsymbol++;
	return ss.str();
}

string FZ::typeName(const string& name){
	stringstream ss;
	ss <<name <<"_type";
	return ss.str();
}

string FZ::indextypeName(const string& name){
	stringstream ss;
	ss <<name <<"_indextype";
	return ss.str();
}

const char* FZ::tab() { return "\t"; }
const char* FZ::endl() { return "\n"; }
const char* FZ::endst() { return ".\n"; }

void Var::add(std::stringstream& voc, std::stringstream& theory, std::stringstream& structure){
	if(type!=VAR_BOOL){ throw fzexception("Incorrect type.\n"); }

	voc <<tab() <<getName() <<"\n";
	if(expr!=NULL){
		if(expr->type==EXPR_BOOL){
			structure <<tab() <<getName() <<" = " <<(expr->boollit?"true":"false") <<"\n";
		}else if(expr->type==EXPR_ARRAYACCESS){
			theory <<tab() <<getName() << " <=> " <<*expr->arrayaccesslit->id <<"(" <<expr->arrayaccesslit->index <<")" <<endst();
		}else if(expr->type==EXPR_IDENT){
			theory <<tab() <<getName() << " <=> " <<*expr->ident->name <<endst();
		}else{ throw fzexception("Unexpected type.\n"); }
	}
}

template<class T>
void createIntType(T* var, const string& type, std::stringstream& voc, std::stringstream& structure, bool nonint = false){
	voc <<tab() <<"type " <<type <<" isa int\n";

	//values
	if(var->enumvalues){
		structure <<tab() <<type <<" = {";
		bool begin = true;
		for(vector<int>::const_iterator i=var->values->begin(); i<var->values->end(); ++i){
			if(!begin){
				structure <<"; ";
			}
			begin = false;
			structure <<*i;
		}
		structure <<"}\n";
	}else if(var->range){
		structure <<tab() <<type <<" = {" <<var->begin <<".." <<var->end <<"}\n";
	}else if(nonint){
		throw fzexception("Cannot have plain int as inner type of a set.\n");
	}
}

void IntVar::add(std::stringstream& voc, std::stringstream& theory, std::stringstream& structure){
	if(type!=VAR_INT){ throw fzexception("Incorrect type.\n"); }

	createIntType(this, typeName(getName()), voc, structure);
	voc <<tab() <<getName() <<": " <<typeName(getName()) <<"\n";

	if(expr!=NULL){
		if(expr->type==EXPR_INT){
			structure <<tab() <<getName() <<" = " <<expr->intlit <<"\n";
		}else if(expr->type==EXPR_ARRAYACCESS){
			theory <<tab() <<getName() <<" = " <<*expr->arrayaccesslit->id <<"(" <<expr->arrayaccesslit->index <<")" <<endst();
		}else if(expr->type==EXPR_IDENT){
			theory <<tab() <<getName() <<" = " <<*expr->ident->name <<endst();
		}else{ throw fzexception("Unexpected type.\n"); }
	}
}

void SetVar::add(std::stringstream& voc, std::stringstream& theory, std::stringstream& structure){
	if(type!=VAR_SET){ throw fzexception("Incorrect type.\n"); }

	const string& name = getName();

	createIntType(this->var, typeName(name), voc, structure, true);
	voc <<tab() <<name <<"(" <<typeName(name) <<")\n";

	if(expr!=NULL){
		if(expr->type==EXPR_SET){
			if(expr->setlit->range){
				structure <<tab() <<name <<" = {" <<expr->setlit->begin <<".." <<expr->setlit->end <<"}\n";
			}else{
				structure <<tab() <<name <<" = {";
				bool begin = true;
				for(vector<int>::const_iterator i=expr->setlit->values->begin(); i<expr->setlit->values->end(); ++i){
					if(!begin){
						structure <<"; ";
					}
					begin = false;
					structure <<*i;
				}
				structure <<"}\n";
			}
		}else if(expr->type==EXPR_ARRAYACCESS){
			const ArrayAccess& arrlit = *expr->arrayaccesslit;
			string symbol = createSymbol();
			voc <<tab() <<"type " <<symbol <<" isa int contains " <<typeName(name) <<", " <<typeName(*arrlit.id) <<"\n";
			theory <<tab() <<"!x[" <<symbol <<"]: " <<"(" <<term(name, "x") <<" <=> " <<term(*arrlit.id, arrlit.index, "x") <<")" <<endst();
		}else if(expr->type==EXPR_IDENT){
			const Identifier& id = *expr->ident;
			string symbol = createSymbol();
			voc <<tab() <<"type " <<symbol <<" isa int contains " <<typeName(name) <<", " <<typeName(*id.name) <<"\n";
			theory <<tab() <<"!x[" <<symbol <<"]: " <<"(" <<term(name, "x") <<" <=> " <<term(*id.name, "x") <<")" <<endst();
		}else{ throw fzexception("Unexpected type.\n"); }
	}
}

void ArrayVar::add(std::stringstream& voc, std::stringstream& theory, std::stringstream& structure){
	if(type!=VAR_ARRAY || begin!=1 || end<begin){ throw fzexception("Incorrect type.\n"); }

	const string& name = getName();

	//Mapping from
	voc <<tab() <<"type " <<indextypeName(name) <<" isa int\n";
	structure <<tab() <<indextypeName(name) <<" = {" <<begin <<".." <<end <<"}\n";

	//Mapping to
	VAR_TYPE expectedtype = rangetype;
	IntRange* intrange = NULL;
	if(rangevar!=NULL){
		expectedtype = rangevar->type;
		if(expectedtype == VAR_BOOL){
		}else if(expectedtype == VAR_INT){
			intrange = new IntRange(dynamic_cast<IntVar*>(this->rangevar));
		}else if(expectedtype == VAR_SET){
			intrange = new IntRange((dynamic_cast<SetVar*>(this->rangevar))->var);
		}else{ throw fzexception("Incorrect/unsupported type.\n"); }
	}else{
		// IMPORTANT: Expecting the grounder to find out the necessary information, so we don't have to parse it here!
		if(expectedtype == VAR_INT){
			intrange = new IntRange();
		}else if(expectedtype == VAR_SET){
			intrange = new IntRange();
		}
	}

	switch(expectedtype){
	case VAR_BOOL:
		voc <<tab() <<term(name, indextypeName(name)) <<"\n";
		break;
	case VAR_INT:
		assert(intrange!=NULL);
		createIntType(intrange, typeName(name), voc, structure);
		voc <<tab() <<term(name, indextypeName(name)) <<": " <<typeName(name) <<"\n";
		break;
	case VAR_SET:
		assert(intrange!=NULL);
		createIntType(intrange, typeName(name), voc, structure);
		voc <<tab() <<term(name, indextypeName(name), typeName(name)) <<"\n";
		break;
	default:
		assert(false);
	}
	if(intrange!=NULL){ delete(intrange); }

	// instantiated values
	if(arraylit!=NULL){
		stringstream arraytrue, arrayfalse;
		if(arraylit->exprs->size()==0){
			structure <<tab() <<name <<" = { }\n";
		}else{
			if((int)arraylit->exprs->size() != end){
				throw fzexception("Incorrect nb of expressions.\n");
			}
			arraytrue <<tab() <<name <<"<ct> = { ";
			arrayfalse <<tab() <<name <<"<cf> = { ";
			int index = 1;
			bool addedtrue = false, addedfalse = false; //HACK because grounder does not support empty tuples without types
			for(vector<Expression*>::const_iterator i=arraylit->exprs->begin(); i<arraylit->exprs->end(); ++i, ++index){
				Expression* expr = *i;
				assert(expr!=NULL);
				switch(expectedtype){
				case VAR_BOOL:
					if(expr->type==EXPR_BOOL){
						if(expr->boollit){
							arraytrue <<index <<"; "; addedtrue = true;
						}else{
							arrayfalse <<index <<"; "; addedfalse = true;
						}
					}else if(expr->type==EXPR_ARRAYACCESS){
						theory <<tab() <<term(name, index) <<" <=> " <<*expr->arrayaccesslit->id <<"(" <<expr->arrayaccesslit->index <<")" <<endst();
					}else if(expr->type==EXPR_IDENT){
						theory <<tab() <<term(name, index) << " <=> " <<*expr->ident->name <<endst();
					}else{ throw fzexception("Unexpected type.\n"); }
					break;
				case VAR_INT:
					if(expr->type==EXPR_INT){
						arraytrue <<index <<"->" <<expr->intlit <<"; "; addedtrue = true;
					}else if(expr->type==EXPR_ARRAYACCESS){
						theory <<tab() <<term(name, index) <<" = " <<term(*expr->arrayaccesslit->id, expr->arrayaccesslit->index) <<endst();
					}else if(expr->type==EXPR_IDENT){
						theory <<tab() <<term(name, index) <<" = " <<*expr->ident->name <<endst();
					}else{ throw fzexception("Unexpected type.\n"); }
					break;
				case VAR_SET:
					if(expr->type==EXPR_SET){
						if(expr->setlit->range){
							theory <<tab() <<"!x: (" <<term(name, index, "x") <<" <=> " <<expr->setlit->begin <<" =< x =< " <<expr->setlit->end <<")"<<endst();
						}else{
							theory <<tab() <<"!x: " <<term(name, index) <<" = x <=> (";
							bool begin = true;
							for(vector<int>::const_iterator j=expr->setlit->values->begin(); j<expr->setlit->values->end(); ++j){
								if(!begin){
									theory <<" | ";
								}
								begin = false;
								theory <<"x=" <<*j;
							}
							theory <<")" <<endst();
						}
					}else if(expr->type==EXPR_ARRAYACCESS){
						voc <<tab() <<"type " <<createSymbol() <<" isa int contains " <<typeName(name) <<", " <<typeName(*expr->arrayaccesslit->id) <<"\n";
						theory <<tab() <<"!x: (" <<term(name, index, "x") <<" <=> " <<term(*expr->arrayaccesslit->id, expr->arrayaccesslit->index, var) <<")" <<endst();
					}else if(expr->type==EXPR_IDENT){
						voc <<tab() <<"type " <<createSymbol() <<" isa int contains " <<typeName(name) <<", " <<typeName(*expr->ident->name) <<"\n";
						theory <<tab() <<"! x:(" <<term(name, index, "x") <<" <=> " <<term(*expr->ident->name, "x") <<")" <<endst();
					}else{ throw fzexception("Unexpected type.\n"); }
					break;
				default:
					assert(false);
				}
			}
			arraytrue <<" }\n";
			arrayfalse <<" }\n";
			if(addedtrue) {
				structure <<arraytrue.str();
			}
			if(addedfalse){
				structure <<arrayfalse.str();
			}
		}
	}
}
