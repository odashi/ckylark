AM_CXXFLAGS = -I$(srcdir)/../include $(BOOST_CPPFLAGS)
LDADD = ../lib/libckylark.la $(BOOST_LDFLAGS) $(BOOST_IOSTREAMS_LIBS) $(BOOST_PROGRAM_OPTIONS_LIB)

bin_PROGRAMS = ckylark

ckylark_SOURCES = main.cc
ckylark_LDADD = $(LDADD)
