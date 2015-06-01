lib_LTLIBRARIES = libckylark.la

AM_CXXFLAGS = -I$(srcdir)/../include $(BOOST_CPPFLAGS) -DPKGDATADIR='"$(pkgdatadir)"'

libckylark_la_SOURCES = \
	BerkeleySignatureEstimator.cc \
	Dictionary.cc \
	FormatterFactory.cc \
	GeometricScalingFactor.cc \
	Grammar.cc \
	GZipStream.cc \
	HarmonicScalingFactor.cc \
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

libckylark_la_LDFLAGS = -version-info 0:4:0 $(BOOST_LDFLAGS) $(BOOST_IOSTREAMS_LIB)

