#ifndef CKYLARK_ARGUMENT_PARSER_H_
#define CKYLARK_ARGUMENT_PARSER_H_

#include <map>
#include <string>

namespace Ckylark {

class ArgumentParser {

    enum ArgumentType {
        ARGTYPE_SWITCH,
        ARGTYPE_STRING,
        ARGTYPE_INTEGER,
        ARGTYPE_REAL,
    }; // enum ArgumentType
    
    struct Entry {
        // settings
        ArgumentType argtype;
        std::string help;
        bool required;
        // values
        bool sw_value, sw_default;
        std::string str_value, str_default;
        int int_value, int_default;
        double real_value, real_default;
    }; // struct Entry

    ArgumentParser() = delete;
    ArgumentParser(const ArgumentParser &) = delete;
    ArgumentParser & operator=(const ArgumentParser &) = delete;

public:
    ArgumentParser(const std::string & bin_name);
    ~ArgumentParser();

    void addSwitchArgument(
        const std::string & name,
        const std::string & help);

    void addStringArgument(
        const std::string & name,
        const std::string & default_value,
        const std::string & help,
        bool required);

    void addIntegerArgument(
        const std::string & name,
        int default_value,
        const std::string & help,
        bool required);

    void addRealArgument(
        const std::string & name,
        double default_value,
        const std::string & help,
        bool required);

    void printUsage() const;

    bool parseArgs(int argc, char * argv[]);

    bool getSwitch(const std::string & name) const;
    std::string getString(const std::string & name) const;
    int getInteger(const std::string & name) const;
    double getReal(const std::string & name) const;

private:
    bool parsed_;
    std::string usage_;
    std::map<std::string, Entry> args_;

    void checkNewArgument(const std::string & name) const;

    std::string getArgumentTypeText(ArgumentType argtype) const;
    
    std::string getMetavar(ArgumentType argtype) const;

    const Entry & getEntry(const std::string & name, ArgumentType argtype) const;

}; // class ArgumentParser

} // namespace Ckylark

#endif // CKYLARK_ARGUMENT_PARSER_H_

