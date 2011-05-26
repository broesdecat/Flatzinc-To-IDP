# Copyright 2011 Katholieke Universiteit Leuven
# Use of this software is governed by the GNU LGPLv3.0 license
# Written by Broes De Cat, K.U.Leuven, Departement
# Computerwetenschappen, Celestijnenlaan 200A, B-3001 Leuven, Belgium
AM_CXXFLAGS = @AC_CXXFLAGS@
AM_LDFLAGS = @AC_LDFLAGS@
		
AM_YFLAGS = -d -v 

bin_PROGRAMS = fz2idp

BUILT_SOURCES = flatzincsupport/flatzincparser.h

fz2idp_SOURCES = \
		flatzincsupport/flatzinclexer.lpp flatzincsupport/flatzincparser.ypp\
		flatzincsupport/InsertWrapper.cpp flatzincsupport/InsertWrapper.hpp\
		flatzincsupport/FZDatastructs.hpp flatzincsupport/FZDatastructs.cpp\
		flatzincsupport/fzexception.hpp\
		flatzincsupport/FlatZincMX.hpp flatzincsupport/FlatZincMX.cpp\
		main.cpp

dist_doc_DATA = README TODO AUTHORS