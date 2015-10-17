#ifndef PARSER_HPP
#define PARSER_HPP

#include <cctype>
#include <sstream>
#include <string>
#include <istream>
#include <ostream>
#include <stack>
#include <utility>
#include "shaun.hpp"


#define PARSE_ERROR(str) throw parse_error(line_, column_, #str)
#define PARSE_ASSERT(cond, str) if (!(cond)) PARSE_ERROR(str)
#define TAKE_TO(c) while (iss_.good() && iss_.peek() != c) forward()

namespace shaun
{

class parse_error
{
public:
    parse_error(size_t line, size_t column, const std::string& error)
    {
        line_ = line+1;
        column_ = column+1;
        error_ = error;
    }

private:
    size_t line_;
    size_t column_;
    std::string error_;

    friend std::ostream& operator<<(std::ostream& out, parse_error& err);
};


    std::ostream& operator<<(std::ostream& out, parse_error& err)
    {
        out << "parse error at line: " << err.line_
            << " column: " << err.column_
            << " (" << err.error_ << ")";

        return out;
    }

class parser
{
public:
    typedef char char_type;
    typedef std::char_traits<char_type> char_traits;
    typedef std::basic_string<char_type, char_traits> String;
    typedef std::basic_istringstream<char_type, char_traits> Stream;

    parser() { parsed = 0; }
    parser(const String& str)
    {
         parsed = 0; 
        iss_.str(str);
    }

    parser(const Stream& iss)
    {
        parsed = 0; 
        iss_.str(iss.str());
    }

    ~parser()
    {
        if (parsed)
            delete parsed;
    }

    object& parse()
    {
        line_ = column_ = 0;
        if (parsed) delete parsed;
        parsed = 0;
        
        while (iss_.good())
        {
            skipws();

            char_type c = iss_.peek();
            if (c == '{')
            {
                parsed = parse_object();
                break;
            }
            else if (isalpha(c))
            {
                parsed = new object();

                while (iss_.good())
                {
                    parsed->add<shaun>(parse_variable());
                    skipws();
                }

                break;
            }
            else
            {
                PARSE_ERROR(root must be an object);
            }

            forward();
        }

        PARSE_ASSERT(parsed, what ?);

        return *parsed;
    }

    object& parse(const String& str)
    {
        iss_.str(str);
        return parse();
    }

    object& parse(const Stream& iss)
    {
        iss_.str(iss.str());
        return parse();
    }

private:

    void skipws()
    {
        while (iss_.good() && (isspace(iss_.peek())
                            || iss_.peek() == ','
                            || iss_.peek() == '('
                            || iss_.peek() == '/'))
        {
            switch (iss_.peek())
            {
                case '(':
                case '/': skip_comment();
                default: break;
            }

            forward();
        }
    }

    void skip_comment()
    {
        switch (iss_.peek())
        {
            case '(':
                TAKE_TO(')');
                break;
            case '/':
                forward();
                if (iss_.peek() == '/') { TAKE_TO('\n'); return; }
                if (iss_.peek() == '*')
                {
                    while (iss_.good() && iss_.peek() != '/')
                    {
                        TAKE_TO('*');
                        forward();
                    }
                    return;
                } break;
            default: break;
        }
    }

    void forward()
    {
        ++column_;
        if (iss_.peek() == '\n')
        {
            ++line_;
            column_ = 0;
        }

        iss_.ignore();
    }

    object * parse_object()
    {
        PARSE_ASSERT(iss_.peek() == '{', expected object value);
        object * obj = new object();

        forward();
        skipws();

        while (iss_.good() && iss_.peek() != '}')
        {
            obj->add<shaun>(parse_variable());
            skipws();
        }

        forward();
        return obj;
    }

    std::pair<String, shaun*> parse_variable()
    {
        PARSE_ASSERT(isalpha(iss_.peek()), invalid variable name);
        
        std::string name = parse_name();

        skipws();
        PARSE_ASSERT(iss_.peek() == ':', expected variable separator ':');
        forward();
        skipws();

        shaun * value = parse_value();

        return std::make_pair(name, value);
    }

    String parse_name()
    {
        String ret;
        char_type c;
        while (isalnum(c = iss_.peek()) || c == '_')
        {
            ret.push_back(c);
            forward();
        }

        PARSE_ASSERT(ret != "true" && ret != "false", true or false are invalid names);
        return ret;
    }

    shaun * parse_value()
    {
        char_type c = iss_.peek();
        if (c == '"')
        {
            return parse_string();
        }
        
        if (isdigit(c) || c == '-' || c == '.')
        {
            return parse_number();
        }
        
        if (c == '{')
        {
            return parse_object();
        }
        
        if (c == 't' || c == 'f')
        {
            return parse_boolean();
        }
        
        if (c == '[')
        {
            return parse_list();
        }
        
        String err = "illegal character:  ";
        err[err.size() - 1] = c;
        throw parse_error(line_, column_, err);
    }

    string * parse_string()
    {
        PARSE_ASSERT(iss_.peek() == '"', expected string value);
        size_t start_col = column_;
        forward();

        String str;
        char_type c;
        bool get_line = false;
        bool nows     = false;

        // get a normal string OR the first line of a multiline string
        while (iss_.good() && (c = iss_.peek()) != '\n' && c != '"')
        {
            if (!isspace(c)) nows = true;

            str.push_back(c);
            forward();
        }

        // if only spaces and multiline string, discard spaces
        if (nows && c == '\n')
        {
            str.resize(0);
        }
        
        if (c == '\n') forward();

        // this loop for multiline strings
        while (iss_.good() && (c = iss_.peek()) != '"')
        {
            // each new line resets the "spaces counter"
            if (c == '\n')
            {
                str.push_back(c);
                get_line = false;
            }
            else if (column_ >= start_col || get_line || !isspace(c))
            {
                str.push_back(c);
                get_line = true;
            }

            forward();
        }

        // discard last new line, if existing
        if (str[str.size() - 1] == '\n') str.resize(str.size() - 1);

        finish:
        PARSE_ASSERT(iss_.good(), unexpected EOF while parsing string);

        forward();        

        return new string(str);
    }

    number * parse_number()
    {
        String num;
        String unit;
        char_type c;
        while (iss_.good() && ((c = iss_.peek()) == 'E' || c == 'e' || c == '-' || isdigit(c) || c == '.' || c == '+'))
        {
            num.push_back(c);
            forward();
        }

        while (iss_.good() && isalpha(c = iss_.peek()))
        {
            unit.push_back(c);
            forward();
        }

        try
        {
            double dbl = stod(num);
            if (unit == "deg")
                return new number(dbl, number::Unit::deg);
            else if (unit == "rad")
                return new number(dbl, number::Unit::rad);
            else
                return new number(dbl, number::Unit::none);
        }
        catch (const std::invalid_argument&)
        {
            PARSE_ERROR(invalid number format);
        }
        catch (...)
        {
            PARSE_ERROR(error parsing a number);
        }
    }

    boolean * parse_boolean()
    {
        String ret;
        char_type c;
        while (isalpha(c = iss_.peek()))
        {
            ret.push_back(c);
            forward();
        }

        PARSE_ASSERT(ret == "true" || ret == "false", expected boolean value);
        return new boolean(ret == "true");
    }

    list * parse_list()
    {
        PARSE_ASSERT(iss_.peek() == '[', expected list value);
        list * ret = new list();

        forward();
        skipws();
        while (iss_.good() && iss_.peek() != ']')
        {
            ret->push_back(parse_value());
            skipws();
        }

        forward();
        return ret;
    }

    Stream iss_;
    size_t line_;
    size_t column_;

    object * parsed;
};

} // namespace

#endif