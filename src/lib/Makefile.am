lib_LTLIBRARIES = libckylark.la

AM_CXXFLAGS = -I$(srcdir)/../include

libckylark_la_SOURCES = \
    ArgumentParser.cc \
    BerkeleySignatureEstimator.cc \
    Dictionary.cc \
    Grammar.cc \
    GZipStream.cc \
    LAPCFGParser.cc \
    Lattice.cc \
    Lexicon.cc \
    Mapping.cc \
    MaxScalingFactor.cc \
    ModelProjector.cc \
    OOVLexiconSmoother.cc \
    ParserFactory.cc \
    PLFLatticeLoader.cc \
    SExprFormatter.cc \
    StdStream.cc \
    StreamFactory.cc \
    TagSet.cc \
    TextStream.cc \
    Timer.cc \
    Tracer.cc

libckylark_la_LIBADD = -lboost_iostreams
libckylark_la_LDFLAGS = -version-info 0:0:0
