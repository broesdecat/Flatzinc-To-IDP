/*
 * Copyright 2011 Katholieke Universiteit Leuven
 *
 * Use of this software is governed by the GNU LGPLv3.0 license
 *
 * Written by Broes De Cat, K.U.Leuven, Departement
 * Computerwetenschappen, Celestijnenlaan 200A, B-3001 Leuven, Belgium
 */
#ifndef FLATZINCMX_HPP_
#define FLATZINCMX_HPP_

#include <vector>
#include <string>

namespace FZ{
class InsertWrapper;

class FlatZincMX {
private:
	InsertWrapper* data;

	const InsertWrapper& getData() const { return *data; }
public:
	FlatZincMX();
	virtual ~FlatZincMX();

	void parse(bool readfromstdin, const std::string& inputfile);
	void writeout();
};
}

#endif /* FLATZINCMX_HPP_ */
