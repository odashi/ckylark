#include <ckylark/StreamFactory.h>

#include <ckylark/StdStream.h>
#include <ckylark/TextStream.h>
#include <ckylark/GZipStream.h>

#include <ckylark/Tracer.h>

#include <stdexcept>

using namespace std;

namespace Ckylark {

shared_ptr<InputStream> StreamFactory::createInputStream(const string & path) {
    // try loading stdin
    if (path == "/dev/stdin") {
        Tracer::println(1, "input: STDIN");
        return shared_ptr<InputStream>(new StdInputStream());
    }

    // try loading basic text stream
    try {
        InputStream * stream = new TextInputStream(path);
        Tracer::println(1, "input path: " + path + " (mode=text)");
        return shared_ptr<InputStream>(stream);
    } catch (...) {
    }

    // try loading gzip stream
    string path_gz = path + ".gz";
    try {
        InputStream * stream = new GZipInputStream(path_gz);
        Tracer::println(1, "input path: " + path_gz + " (mode=gzip)");
        return shared_ptr<InputStream>(stream);
    } catch (exception & ex) {
    }

    // all cases are failed
    throw runtime_error("StreamFactory::createInputStream: cannot open file: " + path);
}

shared_ptr<OutputStream> StreamFactory::createOutputStream(const string & path) {
    // try loading stdout
    if (path == "/dev/stdout") {
        Tracer::println(1, "output: STDOUT");
        return shared_ptr<OutputStream>(new StdOutputStream);
    }
    
    // try loading basic text stream
    try {
        OutputStream * stream = new TextOutputStream(path);
        Tracer::println(1, "output path: " + path + " (mode=text)");
        return shared_ptr<OutputStream>(stream);
    } catch (...) {
    }

    // all cases are failed
    throw runtime_error("StreamFactory::createOutputStream: cannot open file: " + path);
}

} // namespace Ckylark

