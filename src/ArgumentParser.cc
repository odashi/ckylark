#include "ArgumentParser.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <stdexcept>

using namespace std;

namespace AHCParser {

ArgumentParser::ArgumentParser(const string & usage)
    : parsed_(false)
    , usage_(usage) {
}

ArgumentParser::~ArgumentParser() {}

void ArgumentParser::checkNewArgument(const string & name) const {
    if (parsed_) {
        throw runtime_error("ArgumentParser::checkNewArgument(): this parser is already processed.");
    }
    if (args_.find(name) != args_.end()) {
        throw runtime_error("ArgumentParser::checkNewArgument(): argument \"" + name + "\" is already exist.");
    }
}

string ArgumentParser::getArgumentTypeText(ArgumentParser::ArgumentType argtype) const {
    switch (argtype) {
        case ARGTYPE_SWITCH: return "Switch";
        case ARGTYPE_STRING: return "String";
        case ARGTYPE_INTEGER: return "Integer";
        case ARGTYPE_REAL: return "Real";
        default: return "Unknown";
    }
}

string ArgumentParser::getMetavar(ArgumentParser::ArgumentType argtype) const {
    switch (argtype) {
        case ARGTYPE_SWITCH: return "";
        case ARGTYPE_STRING: return "STR";
        case ARGTYPE_INTEGER: return "INT";
        case ARGTYPE_REAL: return "FLOAT";
        default: return "";
    }
}

const ArgumentParser::Entry & ArgumentParser::getEntry(const string & name, ArgumentType argtype) const {
    if (!parsed_) {
        throw runtime_error("ArgumentParser::getArgument(): this parser is still not processed.");
    }

    map<string, Entry>::const_iterator it = args_.find(name);
    if (it == args_.end()) {
        throw runtime_error("ArgumentParser::getArgument(): argument -" + name + " not found.");
    }

    const Entry & ent = it->second;
    if (ent.argtype != argtype) {
        string repr1 = getArgumentTypeText(ent.argtype);
        string repr2 = getArgumentTypeText(argtype);
        throw runtime_error(
            "ArgumentParser::getArgument(): argument -" + name + " is " +
            repr1 + " type, not " + repr2 + " type.");
    }

    return ent;
}

void ArgumentParser::addSwitchArgument(
    const string & name,
    const string & help) {
    
    checkNewArgument(name);

    Entry ent;
    ent.argtype = ARGTYPE_SWITCH;
    ent.help = help;
    ent.required = false;
    ent.sw_value = false;

    args_.insert(make_pair(name, ent));
}

void ArgumentParser::addStringArgument(
    const string & name,
    const string & default_value,
    const string & help,
    bool required) {

    checkNewArgument(name);
    
    Entry ent;
    ent.argtype = ARGTYPE_STRING;
    ent.help = help;
    ent.required = required;
    ent.str_value = default_value;
    ent.str_default = default_value;

    args_.insert(make_pair(name, ent));
}

void ArgumentParser::addIntegerArgument(
    const string & name,
    int default_value,
    const string & help,
    bool required) {

    checkNewArgument(name);

    Entry ent;
    ent.argtype = ARGTYPE_INTEGER;
    ent.help = help;
    ent.required = required;
    ent.int_value = default_value;
    ent.int_default = default_value;

    args_.insert(make_pair(name, ent));
}

void ArgumentParser::addRealArgument(
    const string & name,
    double default_value,
    const string & help,
    bool required) {

    checkNewArgument(name);

    Entry ent;
    ent.argtype = ARGTYPE_REAL;
    ent.help = help;
    ent.required = required;
    ent.real_value = default_value;
    ent.real_default = default_value;

    args_.insert(make_pair(name, ent));
}

void ArgumentParser::printUsage() const {
    int maxlen = 0;
    list<string> names;
    
    for (map<string, Entry>::const_iterator it = args_.begin(); it != args_.end(); ++it) {
        const string & name = it->first;
        int len = name.size() + 5; // len(name) + len("FLOAT")
        maxlen = len > maxlen ? len : maxlen;
        names.push_back(name);
    }
    
    names.sort();
    
    cout << "Usage: " << usage_ << endl;
    cout << "Options:" << endl;
    for (list<string>::iterator it = names.begin(); it != names.end(); ++it) {
        const Entry & ent = args_.at(*it);
        string metavar = getMetavar(ent.argtype);
        cout
            << "    --" << *it
            << " " << metavar
            << string(maxlen - it->size() - metavar.size() + 4, ' ') // padding
            << (ent.required ? "[required] " : "")
            << ent.help;
        if (!ent.required) {
            switch (ent.argtype) {
                case ARGTYPE_STRING: cout << " (default: " << ent.str_default << ")"; break;
                case ARGTYPE_INTEGER: cout << " (default: " << ent.int_default << ")"; break;
                case ARGTYPE_REAL: cout << " (default: " << ent.real_default << ")"; break;
                default: ;
            }
        }
        cout << endl;
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
        string arg = argv[i];
        if (arg.substr(0, 2) == "--") {
            string name = arg.substr(2);
            map<string, Entry>::iterator it = args_.find(name);
            if (it == args_.end()) {
                cerr << "Unknown option: " << arg << endl;
                return false;
            }
            
            Entry & ent = it->second;
            if (ent.argtype == ARGTYPE_SWITCH) {
                ent.sw_value = true;
            } else {
                if (i + 1 >= argc) {
                    cerr << "Insufficient option: " << arg << endl;
                    return false;
                }
                ent.str_value = argv[i + 1];

                // validate and store value
                try {
                    switch (ent.argtype) {
                        case ARGTYPE_INTEGER: ent.int_value = stoi(ent.str_value); break;
                        case ARGTYPE_REAL: ent.real_value = stod(ent.str_value); break;
                        default: ;
                    }
                } catch (...) {
                    cerr << "Invalid format: " << arg << " " << ent.str_value << endl;
                    return false;
                }
            }

            // remove argument from required list
            list<string>::iterator it2 = find(required_list.begin(), required_list.end(), name);
            if (it2 != required_list.end()) {
                required_list.erase(it2);
            }
        }
    }

    //for (map<string, Entry>::const_iterator it = args_.begin(); it != args_.end(); ++it)
    //    cout << it->first << ' ' << it->second.str_value << " " << it->second.sw_value << endl;

    // check required arguments
    if (required_list.size() > 0) {
        cerr << "Required options:";
        for (list<string>::iterator it = required_list.begin(); it != required_list.end(); ++it) {
            cerr << " --" << *it;
        }
        cerr << endl;
        return false;
    }

    parsed_ = true;
    return true;
}

bool ArgumentParser::getSwitch(const string & name) const {
    const Entry & ent = getEntry(name, ARGTYPE_SWITCH);
    return ent.sw_value;
}

string ArgumentParser::getString(const string & name) const {
    const Entry & ent = getEntry(name, ARGTYPE_STRING);
    return ent.str_value;
}

int ArgumentParser::getInteger(const string & name) const {
    const Entry & ent = getEntry(name, ARGTYPE_INTEGER);
    return ent.int_value;
}

double ArgumentParser::getReal(const string & name) const {
    const Entry & ent = getEntry(name, ARGTYPE_REAL);
    return ent.real_value;
}

} // namespace AHCParser

