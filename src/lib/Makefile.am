lib_LTLIBRARIES = libckylark.la

AM_CXXFLAGS = -I$(srcdir)/../include $(BOOST_CPPFLAGS)

libckylark_la_SOURCES = \
	ArgumentParser.cc \
	BerkeleySignatureEstimator.cc \
	Dictionary.cc \
	FormatterFactory.cc \
	Grammar.cc \
	GZipStream.cc \
	LAPCFGParser.cc \
	Lattice.cc \
	Lexicon.cc \
	M1Lexicon.cc \
	M1ModelProjector.cc \
	Mapping.cc \
	MaxScalingFactor.cc \
	ModelProjector.cc \
	OOVLexiconSmoother.cc \
	ParserFactory.cc \
	PLFLatticeLoader.cc \
	POSTagFormatter.cc \
	SExprFormatter.cc \
	StdStream.cc \
	StreamFactory.cc \
	TagSet.cc \
	TextStream.cc \
	Timer.cc \
	Tracer.cc

libckylark_la_LIBADD = $(BOOST_IOSTREAMS_LIBADD)
libckylark_la_LDFLAGS = -version-info 0:0:0 $(BOOST_IOSTREAMS_LDFLAGS)
