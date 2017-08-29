#ifndef PARSER_HPP
#define PARSER_HPP

#include <clocale>
#include <cctype>
#include <sstream>
#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include <stack>
#include <utility>
#include <iostream>
#include <SHAUN/shaun.hpp>
#include <SHAUN/exception.hpp>

namespace shaun
{

class parser
{
public:
    typedef char char_type;
    typedef std::char_traits<char_type> char_traits;
    typedef std::basic_string<char_type, char_traits> String;
    typedef std::basic_istringstream<char_type, char_traits> Stream;

    parser();
    parser(const String& str);
    parser(const Stream& iss);

    ~parser();

    object& parse();
    

    object& parse(const String& str);
    

    object& parse(const Stream& iss);
    

    object& parse_file(const String& str);
    

private:

    void skipws();
    

    void skip_comment();
    

    void forward();
    

    object * parse_object();
    

    std::pair<String, shaun*> parse_variable();
    

    String parse_name();
    

    shaun * parse_value();
    

    string * parse_string();
    

    number * parse_number();
    

    boolean * parse_boolean();
    

    list * parse_list();
    

    template<typename T> int signum(T val);
    

    Stream iss_;
    size_t line_;
    size_t column_;

    object * parsed;
};

} // namespace

#endif
