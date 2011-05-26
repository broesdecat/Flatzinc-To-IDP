/*
 * Copyright 2011 Katholieke Universiteit Leuven
 *
 * Use of this software is governed by the GNU LGPLv3.0 license
 *
 * Written by Broes De Cat, K.U.Leuven, Departement
 * Computerwetenschappen, Celestijnenlaan 200A, B-3001 Leuven, Belgium
 */
#include "flatzincsupport/InsertWrapper.hpp"

#include <assert.h>
#include <string>
#include <iostream>

#include "flatzincsupport/FZDatastructs.hpp"
#include "flatzincsupport/fzexception.hpp"

using namespace std;
using namespace FZ;

// Default ID is hardcoded
int defaultdefID = 0;

string namespacename = "flatzinc";
string vocname = "V";
string theoryname = "T";
string structname = "S";
string procname = "main";

InsertWrapper::InsertWrapper(){
	name2type["bool2int"] = bool2int;
	name2type["bool_and"] = booland;
	name2type["bool_clause"] = boolclause;
	name2type["bool_eq"] = booleq;
	name2type["bool_eq_reif"] = booleqr;
	name2type["bool_le"] = boolle;
	name2type["bool_le_reif"] = booller;
	name2type["bool_lt"] = boollt;
	name2type["bool_lt_reif"] = boolltr;
	name2type["bool_not"] = boolnot;
	name2type["bool_or"] = boolor;
	name2type["bool_xor"] = boolxor;

	name2type["int_abs"] = intabs;
	name2type["int_div"] = intdiv;
	name2type["int_eq"] = inteq;
	name2type["int_eq_reif"] = inteqr;
	name2type["int_le"] = intle;
	name2type["int_le_reif"] = intler;
	name2type["int_lt"] = intlt;
	name2type["int_lt_reif"] = intltr;
	name2type["int_max"] = intmax;
	name2type["int_min"] = intmin;
	name2type["int_mod"] = intmod;
	name2type["int_ne"] = intne;
	name2type["int_ne_reif"] = intner;
	name2type["int_plus"] = intplus;
	name2type["int_times"] = inttimes;
	name2type["int_lin_eq"] = intlineq;
	name2type["int_lin_eq_reif"] = intlineqr;
	name2type["int_lin_le"] = intlinle;
	name2type["int_lin_le_reif"] = intlinler;
	name2type["int_lin_ne"] = intlinne;
	name2type["int_lin_ne_reif"] = intlinner;

	name2type["set_card"] = setcard;
	name2type["set_diff"] = setdiff;
	name2type["set_eq"] = seteq;
	name2type["set_eq_reif"] = seteqr;
	name2type["set_in"] = setin;
	name2type["set_in_reif"] = setinr;
	name2type["set_intersect"] = setintersect;
	name2type["set_le"] = setle;
	name2type["set_lt"] = setlt;
	name2type["set_ne"] = setne;
	name2type["set_ne_reif"] = setner;
	name2type["set_subset"] = setsubset;
	name2type["set_subset_reif"] = setsubsetr;
	name2type["set_symdiff"] = setsymmdiff;
	name2type["set_union"] = setunion;

	name2type["array_bool_and"] = arraybooland;
	name2type["array_bool_element"] = arrayboolelement;
	name2type["array_bool_or"] = arrayboolor;
	name2type["array_int_element"] = arrayintelement;
	name2type["array_set_element"] = arraysetelement;
	name2type["array_var_bool_element"] = arrayvarboolelement;
	name2type["array_var_int_element"] = arrayvarintelement;
	name2type["array_var_set_element"] = arrayvarsetelement;
}

InsertWrapper::~InsertWrapper() {
	for(map<int, stringstream*>::iterator i=definitions.begin(); i!=definitions.end(); ++i){
		delete((*i).second);
	}
}

void InsertWrapper::start(){
	cout <<"/* Automated transformation from a flatzinc model into FO(.). */\n\n";
	cout <<"#include <mx>\n\n";
	cout <<"namespace " <<namespacename <<"{\n";
	voc <<"vocabulary " <<vocname <<"{\n";
	theory <<"theory " <<theoryname <<": " <<vocname <<"{\n";
	structure <<"structure " <<structname <<": " <<vocname <<"{\n";
	main <<"procedure " <<procname <<"() {\n";
}

void InsertWrapper::finish(){
	voc <<"}\n";

	for(map<int, stringstream*>::const_iterator i=definitions.begin(); i!=definitions.end(); ++i){
		theory <<tab() <<"{\n" <<(*i).second->str() <<tab() <<"}\n";
	}

	theory <<"}\n";
	structure <<"}\n";
	main <<"}\n";

	cout <<voc.str();
	cout <<theory.str();
	cout <<structure.str();
	cout <<"}\n"; //namespace close
	cout <<main.str();
}

void InsertWrapper::add(Var* var){
	var->add(voc, theory, structure);
}

Identifier* createIdentifier(){
	vector<Expression*>* args = new vector<Expression*>();
	return new Identifier(new string(createSymbol()), args);
}

void InsertWrapper::parseBool(const Expression& expr, stringstream& ss){
	if(expr.type==EXPR_BOOL){
		ss <<(expr.boollit?"true":"false");
	}else if(expr.type==EXPR_ARRAYACCESS){
		ss <<*expr.arrayaccesslit->id <<"(" <<expr.arrayaccesslit->index <<")";
	}else if(expr.type==EXPR_IDENT){
		ss <<*expr.ident->name;
	}else{ throw fzexception("Unexpected type.\n"); }
}

string InsertWrapper::parseInt(const Expression& expr, stringstream& ss){
	string mappedtype;
	if(expr.type==EXPR_INT){
		ss <<expr.intlit;
	}else if(expr.type==EXPR_ARRAYACCESS){
		mappedtype = typeName(*expr.arrayaccesslit->id);
		ss <<*expr.arrayaccesslit->id <<"(" <<expr.arrayaccesslit->index <<")";
	}else if(expr.type==EXPR_IDENT){
		mappedtype = typeName(*expr.ident->name);
		ss <<*expr.ident->name;
	}else{ throw fzexception("Unexpected type.\n"); }
	return mappedtype;
}

string InsertWrapper::parseSet(Expression& expr, stringstream& ss){
	string mappedtype;
	if(expr.type==EXPR_SET){
		SetVar* var = NULL;
		if(expr.setlit->range){
			var = new SetVar(new IntVar(expr.setlit->begin, expr.setlit->end));
		}else{
			var = new SetVar(new IntVar(new vector<int>(*expr.setlit->values)));
		}
		var->expr = new Expression();
		var->expr->type = EXPR_SET;
		var->expr->setlit = expr.setlit;
		expr.setlit = NULL;
		var->id = createIdentifier();
		add(var);
		mappedtype = typeName(*var->id->name);
		ss <<*var->id->name;
		delete(var);
	}else if(expr.type==EXPR_ARRAYACCESS){
		mappedtype = typeName(*expr.arrayaccesslit->id);
		ss <<*expr.arrayaccesslit->id <<"(" <<expr.arrayaccesslit->index <<")";
	}else if(expr.type==EXPR_IDENT){
		mappedtype = typeName(*expr.ident->name);
		ss <<*expr.ident->name;
	}else{ throw fzexception("Unexpected type.\n"); }
	return mappedtype;
}

string InsertWrapper::parseArray(VAR_TYPE type, Expression& expr, stringstream& ss){
	string mappedtype;
	if(expr.type==EXPR_ARRAY){
		ArrayVar* var = new ArrayVar(type, expr.arraylit);
		expr.arraylit = NULL;
		var->id = createIdentifier();
		var->begin = 1;
		var->end = var->arraylit->exprs->size();
		add(var);
		mappedtype = typeName(*var->id->name);
		ss <<*var->id->name;
		delete(var);
	}else if(expr.type==EXPR_IDENT){
		mappedtype = typeName(*expr.ident->name);
		ss <<*expr.ident->name;
	}else{ throw fzexception("Unexpected type.\n"); }
	return mappedtype;
}

void InsertWrapper::parseArgs(vector<Expression*>& origargs, vector<string>& args, vector<string>& mappednames, const vector<ARG_TYPE>& expectedtypes){
	if(origargs.size()!=expectedtypes.size()){
		throw fzexception("Incorrect number of arguments.\n");
	}
	unsigned int itype=0;
	for(vector<Expression*>::reverse_iterator i=origargs.rbegin(); i<origargs.rend(); ++i, ++itype){
		stringstream ss;
		ARG_TYPE expectedtype = expectedtypes[itype];
		Expression& expr = **i;
		string mappedtype;
		switch(expectedtype){
		case ARG_BOOL: parseBool(expr, ss); break;
		case ARG_INT: mappedtype = parseInt(expr, ss); break;
		case ARG_SET: mappedtype = parseSet(expr, ss); break;
		case ARG_ARRAY_OF_BOOL: mappedtype = parseArray(VAR_BOOL, expr, ss); break;
		case ARG_ARRAY_OF_INT: mappedtype = parseArray(VAR_INT, expr, ss); break;
		case ARG_ARRAY_OF_SET: mappedtype = parseArray(VAR_SET, expr, ss); break;
		}
		mappednames.push_back(mappedtype);
		args.push_back(ss.str());
	}
}

bool hasDefinitionAnnotation(const vector<Expression*>& args, int& definitionid){
	bool defined = false;
	for(vector<Expression*>::const_iterator i=args.begin(); i<args.end(); ++i){
		if((*i)->type==EXPR_IDENT && (*i)->ident->name->compare("inductivelydefined")==0){
			if((*i)->ident->arguments!=NULL){
				if((*i)->ident->arguments->size()==1 && (*(*i)->ident->arguments->begin())->type==EXPR_INT){
					definitionid = (*(*i)->ident->arguments->begin())->intlit;
				}else{ throw fzexception("Incorrect number of annotation arguments");}
			}else{
				definitionid = defaultdefID;
			}
			defined = true;
		}
	}
	return defined;
}

void InsertWrapper::writeEquiv(Constraint* var, bool conj){
	vector<string> args, typenames;
	vector<ARG_TYPE> types;
	int definitionID = 0;
	bool defined = hasDefinitionAnnotation(*var->annotations, definitionID);
	types.push_back(ARG_ARRAY_OF_BOOL); types.push_back(ARG_BOOL);
	parseArgs(*var->id->arguments, args, typenames, types);
	if(defined){
		if(definitions.find(definitionID)==definitions.end()){
			definitions[definitionID] = new stringstream();
		}
		*definitions.at(definitionID) <<tab() <<tab() <<args[1] <<" <- " <<(conj?"!":"?") <<"i: " <<args[0] <<"(i)" <<endst();
	}else{
		theory <<tab() <<tab() <<"(" <<(conj?"!":"?") <<"i: (" <<args[0] <<"(i)) <=> " <<args[1] <<endst();
	}
}

string InsertWrapper::createSuperType(const vector<string>& subtypes){
	string symbol = createSymbol();
	voc <<tab() <<"type " <<symbol <<" isa int contains ";
	bool begin = true;
	for(vector<string>::const_iterator i=subtypes.begin(); i<subtypes.end(); ++i){
		if(!begin){
			voc <<", ";
		}
		begin = false;
		voc <<*i;
	}
	voc <<"\n";
	return symbol;
}

string univquant(string var, string type){
	stringstream ss;
	ss <<"!"<<var <<"[" <<type <<"]" <<": ";
	return ss.str();
}

string existquant(string var, string type){
	stringstream ss;
	ss <<"?"<<var <<"[" <<type <<"]" <<": ";
	return ss.str();
}

//VERY IMPORTANT: ALL PARSED VECTORS ARE REVERSED ORDER (TO HAVE FASTER PARSING)!!!!
void InsertWrapper::add(Constraint* var){
	vector<string> args, typenames;
	vector<ARG_TYPE> types;

	map<string, CONSTRAINT_TYPE>::const_iterator it = name2type.find(*var->id->name);
	if(it==name2type.end()){
		stringstream ss;
		ss <<"Constraint " <<*var->id->name <<" does not exist.\n";
		throw fzexception(ss.str());
	}
	switch ((*it).second) {
		case bool2int:{
			types.push_back(ARG_BOOL); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(" <<args[0] <<" <=> " <<args[1] <<" = 1) & (~" <<args[0] <<" <=> " <<args[1] <<" = 0)" <<endst();
			break;}
		case booland:{
			types.push_back(ARG_BOOL); types.push_back(ARG_BOOL); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(" <<args[0] <<" & " <<args[1] <<") <=>" <<args[2] <<endst();
			break;}
		case boolclause:{
			types.push_back(ARG_ARRAY_OF_BOOL); types.push_back(ARG_ARRAY_OF_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(?x: " <<args[0] <<"(x)) | (?x: ~" <<args[1] <<"(x))" <<endst();
			break;}
		case booleq:{
			types.push_back(ARG_BOOL); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[0] <<" <=> " <<args[1] <<endst();
			break;}
		case booleqr:{
			types.push_back(ARG_BOOL); types.push_back(ARG_BOOL); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(" <<args[0] <<" <=> " <<args[1] <<") <=>" <<args[2] <<endst();
			break;}
		case boolle:{
			types.push_back(ARG_BOOL); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"~"<<args[0] <<" | " <<args[1] <<endst();
			break;}
		case booller:{
			types.push_back(ARG_BOOL); types.push_back(ARG_BOOL); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(~"<<args[0] <<" | " <<args[1] <<") <=>" <<args[2] <<endst();
			break;}
		case boollt:{
			types.push_back(ARG_BOOL); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"~"<<args[0] <<" & " <<args[1] <<endst();
			break;}
		case boolltr:{
			types.push_back(ARG_BOOL); types.push_back(ARG_BOOL); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(~"<<args[0] <<" & " <<args[1] <<") <=>" <<args[2] <<endst();
			break;}
		case boolnot:{
			types.push_back(ARG_BOOL); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"~"<<args[0] <<" <=> " <<args[1] <<endst();
			break;}
		case boolor:{
			types.push_back(ARG_BOOL); types.push_back(ARG_BOOL); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(" <<args[0] <<" | " <<args[1] <<") <=>" <<args[2] <<endst();
			break;}
		case boolxor:{
			types.push_back(ARG_BOOL); types.push_back(ARG_BOOL); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(~" <<args[0] <<" <=> " <<args[1] <<") <=>" <<args[2] <<endst();
			break;}
		case intabs: {
			types.push_back(ARG_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"abs(" <<args[0] <<")= " <<args[1] <<endst();
			break;}
		case intdiv: {
			types.push_back(ARG_INT); types.push_back(ARG_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[0] <<" / " <<args[1] <<" = " <<args[2] <<endst();
			break;}
		case inteq: {
			types.push_back(ARG_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[0] <<" = " <<args[1] <<endst();
			break;}
		case inteqr: {
			types.push_back(ARG_INT); types.push_back(ARG_INT); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(" <<args[0] <<" = " <<args[1] <<") <=> " <<args[2] <<endst();
			break;}
		case intle: {
			types.push_back(ARG_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[0] <<" =< " <<args[1] <<endst();
			break;}
		case intler: {
			types.push_back(ARG_INT); types.push_back(ARG_INT); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(" <<args[0] <<" =< " <<args[1] <<") <=> " <<args[2] <<endst();
			break;}
		case intlt: {
			types.push_back(ARG_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[0] <<" < " <<args[1] <<endst();
			break;}
		case intltr: {
			types.push_back(ARG_INT); types.push_back(ARG_INT); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(" <<args[0] <<" < " <<args[1] <<") <=> " <<args[2]<<endst();
			break;}
		case intmax: {
			types.push_back(ARG_INT); types.push_back(ARG_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"max(" <<args[0] <<", " <<args[1] <<")= " <<args[2]<<endst();
			break;}
		case intmin: {
			types.push_back(ARG_INT); types.push_back(ARG_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"min(" <<args[0] <<", " <<args[1] <<")= " <<args[2] <<endst();
			break;}
		case intmod: {
			types.push_back(ARG_INT); types.push_back(ARG_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[0] <<" mod " <<args[1] <<" = " <<args[2] <<endst();
			break;}
		case intne: {
			types.push_back(ARG_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[0] <<" ~= " <<args[1] <<endst();
			break;}
		case intner: {
			types.push_back(ARG_INT); types.push_back(ARG_INT); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"(" <<args[0] <<" ~= " <<args[1] <<") <=> " <<args[2] <<endst();
			break;}
		case intplus: {
			types.push_back(ARG_INT); types.push_back(ARG_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[0] <<" + " <<args[1] <<" = " <<args[2] <<endst();
			break;}
		case inttimes: {
			types.push_back(ARG_INT); types.push_back(ARG_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[0] <<" * " <<args[1] <<" = " <<args[2] <<endst();
			break;}
		case intlineq: {
			types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"sum{x: "<<args[0] <<"(x) * " <<args[1] <<"(x) } = " <<args[2] <<endst();
			break;}
		case intlineqr: {
			types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_INT); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"sum{x: "<<args[0] <<"(x) * " <<args[1] <<"(x) } = " <<args[2] <<" <=> " <<args[3] <<endst();
			break;}
		case intlinle: {
			types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"sum{x: "<<args[0] <<"(x) * " <<args[1] <<"(x) } =< " <<args[2] <<endst();
			break;}
		case intlinler: {
			types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_INT); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"sum{x: "<<args[0] <<"(x) * " <<args[1] <<"(x) } =< " <<args[2] <<" <=> " <<args[3] <<endst();
			break;}
		case intlinne: {
			types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"sum{x: "<<args[0] <<"(x) * " <<args[1] <<"(x) } ~= " <<args[2] <<endst();
			break;}
		case intlinner: {
			types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_INT); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"sum{x: "<<args[0] <<"(x) * " <<args[1] <<"(x) } ~= " <<args[2] <<" <=> " <<args[3] <<endst();
			break;}
		case setcard:{
			types.push_back(ARG_SET); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<"#{x: " <<args[0] <<"(x)} = " <<args[1] <<endst();
			break;}
		case setdiff:{
			types.push_back(ARG_SET); types.push_back(ARG_SET); types.push_back(ARG_SET);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]); subtypes.push_back(typenames[2]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<univquant("x", gentype)  <<"(" <<args[0] <<"(x) & ~" <<args[1] <<"(x)) <=> " <<args[2] <<"(x)" <<endst();
			break;}
		case seteq:{
			types.push_back(ARG_SET); types.push_back(ARG_SET);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<univquant("x", gentype) <<args[0] <<"(x) <=> " <<args[1] <<"(x)" <<endst();
			break;}
		case seteqr:{
			types.push_back(ARG_SET); types.push_back(ARG_SET); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<args[2] <<"<=> (" <<univquant("x", gentype) <<args[0] <<"(x) <=> " <<args[1] <<"(x))" <<endst();
			break;}
		case setin:{
			types.push_back(ARG_INT); types.push_back(ARG_SET);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[1] <<"(" <<args[0] <<")" <<endst();
			break;}
		case setinr:{
			types.push_back(ARG_INT); types.push_back(ARG_SET); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[2] <<"<=> " <<args[1] <<"(" <<args[0] <<")" <<endst();
			break;}
		case setintersect:{
			types.push_back(ARG_SET); types.push_back(ARG_SET); types.push_back(ARG_SET);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]); subtypes.push_back(typenames[2]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<univquant("x", gentype) <<"(" <<args[0] <<"(x) & " <<args[1] <<"(x)) <=> " <<args[2] <<"(x)" <<endst();
			break;}
		case setle:{
			types.push_back(ARG_SET); types.push_back(ARG_SET);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<"(" <<univquant("x", gentype) <<args[0] <<"(x)" <<" => " <<args[1] <<"(x))"
					<<" | "
					<<args[0] <<"(min{x: (" <<args[0] <<"(x)" <<" & ~"<<args[1] <<"(x)" << ")| ( ~"<<args[0] <<"(x)" <<" & " <<args[1] <<"(x)" <<")})"
					<<endst();
			break;}
		case setlt:{
			types.push_back(ARG_SET); types.push_back(ARG_SET);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<"((" <<univquant("x", gentype) <<args[0] <<"(x)" <<" => " <<args[1] <<"(x))" <<" & ?x: ~" <<args[0] <<"(x)" <<" & " <<args[0] <<"(x)" <<")"
					<<" | "
					<<args[0] <<"(min{x: (" <<args[0] <<"(x)" <<" & ~"<<args[1] <<"(x)" << ")| ( ~"<<args[0] <<"(x)" <<" & " <<args[1] <<"(x)" <<")})"
					<<endst();
			break;}
		case setne:{
			types.push_back(ARG_SET); types.push_back(ARG_SET);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<existquant("x", gentype) <<args[0] <<"(x) <=> ~" <<args[1] <<"(x)" <<endst();
			break;}
		case setner:{
			types.push_back(ARG_SET); types.push_back(ARG_SET); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<args[2] <<"<=> (" <<existquant("x", gentype) <<args[0] <<"(x) <=> ~" <<args[1] <<"(x))" <<endst();
			break;}
		case setsubset:{
			types.push_back(ARG_SET); types.push_back(ARG_SET);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<univquant("x", gentype) <<args[0] <<"(x) => " <<args[1] <<"(x)" <<endst();
			break;}
		case setsubsetr:{
			types.push_back(ARG_SET); types.push_back(ARG_SET); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<args[2] <<"<=> (" <<univquant("x", gentype) <<args[0] <<"(x) => " <<args[1] <<"(x))" <<endst();
			break;}
		case setsymmdiff:{
			types.push_back(ARG_SET); types.push_back(ARG_SET); types.push_back(ARG_SET);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]); subtypes.push_back(typenames[2]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<univquant("x", gentype) <<"((" <<args[0] <<"(x) &~" <<args[1] <<"(x)" <<" )|( ~" <<args[0] <<"(x) & " <<args[1] <<"(x)" <<")) <=> " <<args[2] <<"(x)" <<endst();
			break;}
		case setunion:{
			types.push_back(ARG_SET); types.push_back(ARG_SET); types.push_back(ARG_SET);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[0]); subtypes.push_back(typenames[1]); subtypes.push_back(typenames[2]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<univquant("x", gentype) <<"(" <<args[0] <<"(x) | " <<args[1] <<"(x)) <=> " <<args[2] <<"(x)" <<endst();
			break;}
		case arraybooland:{
			writeEquiv(var, true);
			break;}
		case arrayvarboolelement: //same as next
		case arrayboolelement:{
			types.push_back(ARG_INT); types.push_back(ARG_ARRAY_OF_BOOL); types.push_back(ARG_BOOL);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[1] <<"(" <<args[0] <<") <=> " <<args[2] <<endst();
			theory <<tab() <<"MIN[:" <<indextypeName(args[1]) <<"] =< " <<args[0] <<"=< MAX[:" <<indextypeName(args[1]) <<"]" <<endst();
			break;}
		case arrayboolor:{
			writeEquiv(var, false);
			break;}
		case arrayvarintelement: //same as next
		case arrayintelement:{
			types.push_back(ARG_INT); types.push_back(ARG_ARRAY_OF_INT); types.push_back(ARG_INT);
			parseArgs(*var->id->arguments, args, typenames, types);
			theory <<tab() <<args[1] <<"(" <<args[0] <<") = " <<args[2] <<".\n";
			theory <<tab() <<"MIN[:" <<typeName(args[1]) <<"] =< " <<args[0] <<"=< MAX[:" <<typeName(args[1]) <<"]" <<endst();
			break;}
		case arrayvarsetelement: //same as next
		case arraysetelement:{
			types.push_back(ARG_INT); types.push_back(ARG_ARRAY_OF_SET); types.push_back(ARG_SET);
			parseArgs(*var->id->arguments, args, typenames, types);
			vector<string> subtypes; subtypes.push_back(typenames[1]); subtypes.push_back(typenames[2]);
			string gentype = createSuperType(subtypes);
			theory <<tab() <<univquant("x", gentype) <<"(" <<term(args[1], args[0], "x") <<" <=> " <<term(args[2], "x") <<").\n";
			theory <<tab() <<"MIN[:" <<typeName(args[1]) <<"] =< " <<args[0] <<"=< MAX[:" <<typeName(args[1]) <<"]" <<endst();
			break;}
	}
}

void InsertWrapper::add(Search* search){
	switch(search->type){
	case SOLVE_SATISFY:
		main <<tab() <<term("mx::printsol", theoryname, structname) <<"\n";
		break;
	case SOLVE_MINIMIZE:
		throw fzexception("Not supported by grounder yet.\n");
		break;
	case SOLVE_MAXIMIZE:
		throw fzexception("Not supported by grounder yet.\n");
		break;
	}
}
