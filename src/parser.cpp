#include <SHAUN/parser.hpp>

#define PARSE_ERROR(str) throw parse_error(line_, column_, #str)
#define PARSE_ASSERT(cond, str) if (!(cond)) PARSE_ERROR(str)
#define TAKE_TO(c) while (iss_.good() && iss_.peek() != c) forward()

namespace shaun
{

  parser::parser() { parsed = 0; }
  parser::parser(const parser::String& str)
  {
    parsed = 0; 
    iss_.str(str);
  }

  parser::parser(const parser::Stream& iss)
    {
        parsed = 0; 
        iss_.str(iss.str());
    }

  parser::~parser()
    {
        if (parsed)
            delete parsed;
    }

    object& parser::parse()
    {
        // Changing the locale for "C" number parsing system
        // the old locale is saved to be restored after the
        // parsing, as some applications use a different one
        char * old_locale = setlocale(LC_NUMERIC, NULL);
        setlocale(LC_NUMERIC, "C");

        // init the parsing state
        line_ = column_ = 0;
        if (parsed) delete parsed;
        parsed = 0;
        
        // loop through the whole stream
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
        
        setlocale(LC_NUMERIC, old_locale);

        return *parsed;
    }

    object& parser::parse(const parser::String& str)
    {
        iss_.str(str);
        return parse();
    }

    object& parser::parse(const parser::Stream& iss)
    {
        iss_.str(iss.str());
        return parse();
    }

    object& parser::parse_file(const parser::String& str)
    {
        std::ifstream file(str);
        parser::String to_parse;

        file.seekg(0, std::ios::end);
        to_parse.reserve(file.tellg());
        file.seekg(0, std::ios::beg);

        to_parse.assign(std::istreambuf_iterator<char_type>(file)
            , std::istreambuf_iterator<char_type>());

        return parse(to_parse);
    }


    void parser::skipws()
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

    void parser::skip_comment()
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

    void parser::forward()
    {
        ++column_;
        if (iss_.peek() == '\n')
        {
            ++line_;
            column_ = 0;
        }

        iss_.ignore();
    }

    object * parser::parse_object()
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

    std::pair<parser::String, shaun*> parser::parse_variable()
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

    parser::String parser::parse_name()
    {
        PARSE_ASSERT(isalpha(iss_.peek()) || iss_.peek() == '_', names must start with a letter or '_');
        parser::String ret;
        char_type c;
        while (isalnum(c = iss_.peek()) || c == '_')
        {
            ret.push_back(c);
            forward();
        }

        PARSE_ASSERT(ret != "true" && ret != "false"
            , true or false are invalid names);
        return ret;
    }

    shaun * parser::parse_value()
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
        
        parser::String err = "illegal character:  ";
        err[err.size() - 1] = c;
        throw parse_error(line_, column_, err);
    }

    string * parser::parse_string()
    {
        PARSE_ASSERT(iss_.peek() == '"', expected string value);
        size_t start_col = column_;
        forward();

        parser::String str;
        char_type c = '\0';
        bool get_line = false;
        bool nows     = false;

        // get a normal string OR the first line of a multiline string
        while (iss_.good() && (c = iss_.peek()) != '\n' && c != '"')
        {
            if (!isspace(c)) nows = true;

            // skipped characters
            if (c == '\\')
            {
              forward();
              str.push_back(iss_.peek());
            }
            else
            {
              str.push_back(c);
            }
            forward();
        }

        // if only spaces and multiline string, discard spaces
        if (!nows && c == '\n')
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
                // skipped characters
                if (c == '\\')
                {
                  forward();
                  str.push_back(iss_.peek());
                }
                else
                  str.push_back(c);

                get_line = true;
            }

            forward();
        }

        // discard last new line, if existing
        if (str[str.size() - 1] == '\n') str.resize(str.size() - 1);

        PARSE_ASSERT(iss_.good(), unexpected EOF while parsing string);

        forward();        

        return new string(str);
    }

    number * parser::parse_number()
    {
        parser::String num;
        parser::String unit;
        char_type c;
        while (iss_.good() && ((c = iss_.peek()) == 'E'
            || c == 'e'
            || c == '-'
            || isdigit(c)
            || c == '.'
            || c == '+'))
        {
            num.push_back(c);
            forward();
        }

        size_t before_unit = iss_.tellg();
        skipws();
        try
        {
            unit = parse_name();
        }
        catch (...)
        {
            unit = "none";
            iss_.seekg(before_unit, iss_.beg);
        }
        skipws();

        try
        {
            double dbl = std::stod(num);
                if (iss_.peek() == ':')
                {
                    iss_.seekg(before_unit, iss_.beg);
                    return new number(dbl, "");
                }
                else
                {
                    return new number(dbl, unit);
                }
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

    boolean * parser::parse_boolean()
    {
        parser::String ret;
        char_type c;
        while (isalpha(c = iss_.peek()))
        {
            ret.push_back(c);
            forward();
        }

        PARSE_ASSERT(ret == "true" || ret == "false", expected boolean value);
        return new boolean(ret == "true");
    }

    list * parser::parse_list()
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

    template<typename T> int parser::signum(T val)
    {
        return (T(0) < val) - (val < T(0));
    }

} // namespace

