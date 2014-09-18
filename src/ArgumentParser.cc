#include "ArgumentParser.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <stdexcept>

using namespace std;

namespace AHCParser {

ArgumentParser::ArgumentParser(const std::string & bin_name)
    : parsed_(false)
    , bin_name_(bin_name) {
}

ArgumentParser::~ArgumentParser() {}

void ArgumentParser::addArgument(
    ArgumentType argtype,
    const string & name,
    const string & metavar,
    const string & defval,
    const string & help,
    bool required) {

    if (parsed_) {
        throw runtime_error("ArgumentParser::addArgument(): adding new argument after parsing.");
    }

    if (args_.find(name) != args_.end()) {
        throw runtime_error("ArgumentParser::addArgument(): entry \"" + name + "\" is already exist.");
    }

    // value = defval
    Entry ent = {argtype, defval, metavar, defval, help, required};
    args_.insert(make_pair(name, ent));
}

void ArgumentParser::printUsage() const {
    int maxlen = 0;
    list<string> names;
    
    for (map<string, Entry>::const_iterator it = args_.begin(); it != args_.end(); ++it) {
        const string & name = it->first;
        int len = name.size() + it->second.metavar.size();
        maxlen = len > maxlen ? len : maxlen;
        names.push_back(name);
    }
    
    names.sort();
    
    cout << "Usage: " << bin_name_ << " [options]" << endl;
    cout << "Options:" << endl;
    for (list<string>::iterator it = names.begin(); it != names.end(); ++it) {
        const Entry & ent = args_.at(*it);
        cout
            << "    -" << *it
            << " " << ent.metavar
            << string(maxlen - it->size() - ent.metavar.size() + 4, ' ') // padding
            << (ent.required ? "[required] " : "")
            << ent.help
            << (!ent.required ? " (default: " + ent.defval + ")" : "")
            << endl;
    }
}

bool ArgumentParser::parseArgs(int argc, char * argv[]) {
    if (parsed_) {
        throw runtime_error("ArgumentParser::parseArgs(): trying argument parsing twice.");
    }

    // collecting required arguments
    list<string> required_list;
    for (map<string, Entry>::iterator it = args_.begin(); it != args_.end(); ++it) {
        if (it->second.required) {
            required_list.push_back(it->first);
        }
    }

    // parsing
    for (int i = 0; i < argc; ++i) {
        string arg(argv[i]);
        if (arg[0] == '-') {
            if (i+1 >= argc) {
                return false;
            }
            string name = arg.substr(1);
            if (args_.find(name) == args_.end()) {
                return false;
            }
            list<string>::iterator it = find(required_list.begin(), required_list.end(), name);
            if (it != required_list.end()) {
                required_list.erase(it);
            }
            Entry & ent = args_[name];
            ent.value = argv[i+1];
        }
    }

    //for (map<string, Entry>::const_iterator it = args_.begin(); it != args_.end(); ++it)
    //    cout << it->first << ' ' << it->second.value << endl;

    // check required arguments
    if (required_list.size() > 0) {
        return false;
    }

    // validate arguments
    for (map<string, Entry>::iterator it = args_.begin(); it != args_.end(); ++it) {
        Entry & ent = it->second;
        try {
            switch (ent.argtype) {
            case ARGTYPE_STRING:
                // do nothing
                break;
            case ARGTYPE_INT:
                stoi(ent.value);
                break;
            case ARGTYPE_REAL:
                stod(ent.value);
                break;
            default:
                throw runtime_error("ArgumentParser::parseArgs(): invalid argument type.");
                break;
            }
        } catch (...) {
            return false;
        }
    }

    parsed_ = true;
    return true;
}

string ArgumentParser::getString(const string & name) const {
    if (!parsed_) {
        throw runtime_error("ArgumentParser::getString(): retrieving argument before parsing.");
    }
    map<string, Entry>::const_iterator it = args_.find(name);
    if (it == args_.end()) {
        throw runtime_error("ArgumentParser::getString(): undefined argument: \"" + name + "\"");
    }
    if (it->second.argtype != ARGTYPE_STRING) {
        throw runtime_error("ArgumentParser::getString(): \"" + name + "\" is not a string argument.");
    }
    return it->second.value;
}

int ArgumentParser::getInt(const string & name) const {
    if (!parsed_) {
        throw runtime_error("ArgumentParser::getInt(): retrieving argument before parsing.");
    }
    map<string, Entry>::const_iterator it = args_.find(name);
    if (it == args_.end()) {
        throw runtime_error("ArgumentParser::getInt(): undefined argument: \"" + name + "\"");
    }
    if (it->second.argtype != ARGTYPE_INT) {
        throw runtime_error("ArgumentParser::getInt(): \"" + name + "\" is not a integer argument.");
    }
    return stoi(it->second.value);
}

double ArgumentParser::getReal(const string & name) const {
    if (!parsed_) {
        throw runtime_error("ArgumentParser::getReal(): retrieving argument before parsing.");
    }
    map<string, Entry>::const_iterator it = args_.find(name);
    if (it == args_.end()) {
        throw runtime_error("ArgumentParser::getReal(): undefined argument: \"" + name + "\"");
    }
    if (it->second.argtype != ARGTYPE_REAL) {
        throw runtime_error("ArgumentParser::getReal(): \"" + name + "\" is not a real argument.");
    }
    return stod(it->second.value);
}

} // namespace AHCParser

