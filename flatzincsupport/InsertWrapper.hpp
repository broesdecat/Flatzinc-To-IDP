/*
 * Copyright 2011 Katholieke Universiteit Leuven
 *
 * Use of this software is governed by the GNU LGPLv3.0 license
 *
 * Written by Broes De Cat, K.U.Leuven, Departement
 * Computerwetenschappen, Celestijnenlaan 200A, B-3001 Leuven, Belgium
 */
#ifndef INSERTWRAPPER_HPP_
#define INSERTWRAPPER_HPP_

#include <vector>
#include <string>
#include <sstream>
#include <map>
#include "flatzincsupport/FZDatastructs.hpp"

namespace FZ{

enum CONSTRAINT_TYPE {
	bool2int,

	booland, boolclause, booleq, booleqr, boolle, booller, boollt, boolltr, boolnot, boolor, boolxor,

	intabs, intdiv, inteq, inteqr, intle, intler, intlt, intltr, intmax, intmin, intmod, intne, intner, intplus, inttimes,
	intlineq, intlineqr, intlinle, intlinler, intlinne, intlinner,

	setcard, setdiff, seteq, seteqr, setin, setinr, setintersect, setle, setlt, setne, setner, setsubset, setsubsetr,
	setsymmdiff, setunion,

	arraybooland, arrayboolelement, arrayboolor, arrayintelement, arraysetelement, arrayvarboolelement, arrayvarintelement, arrayvarsetelement
};

enum ARG_TYPE { ARG_BOOL, ARG_INT, ARG_SET, ARG_ARRAY_OF_SET, ARG_ARRAY_OF_INT, ARG_ARRAY_OF_BOOL };

class InsertWrapper {
private:
	std::stringstream voc, theory, structure, main;
	std::map<int, std::stringstream*> definitions;
	std::map<std::string, CONSTRAINT_TYPE> name2type;
	void addFunc(const std::string& func, const std::vector<Expression*>& origargs);
	void parseArgs(std::vector<Expression*>& origargs, std::vector<std::string>& args, std::vector<std::string>& mappednames, const std::vector<ARG_TYPE>& expectedtypes);

	void writeEquiv(Constraint* var, bool conj);

	void parseBool(const Expression& expr, std::stringstream& ss);
	std::string parseInt(const Expression& expr, std::stringstream& ss);
	std::string parseSet(Expression& expr, std::stringstream& ss);
	std::string parseArray(VAR_TYPE type, Expression& expr, std::stringstream& ss);

	std::string createSuperType(const std::vector<std::string>& subtypes);

public:
	InsertWrapper();
	virtual ~InsertWrapper();

	void start	();
	void finish	();

	void add	(Var* var);
	void add	(Constraint* var);
	void add	(Search* var);
};
}

#endif /* INSERTWRAPPER_HPP_ */
