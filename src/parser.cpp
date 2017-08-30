#include <SHAUN/parser.hpp>

namespace shaun
{

class parser
{
public:
    using char_type = char;

    parser() = delete;
    parser(const std::string& str);
    parser(std::istream& iss);

    ~parser();

    object parse();

private:

    void skipws();
    void skip_comment();
    void forward();
    

    object parse_object();
    std::pair<std::string, shaun*> parse_variable();
    std::string parse_name();
    shaun* parse_value();
    string parse_string();
    number parse_number();
    boolean parse_boolean();
    list parse_list();

    template<typename T> int signum(T val);
    
    std::istream * iss_;
    size_t line_;
    size_t column_;
    bool new_iss;
};

#define PARSE_ERROR(str) throw parse_error(line_, column_, #str)
#define PARSE_ASSERT(cond, str) if (!(cond)) PARSE_ERROR(str)
#define TAKE_TO(c) while (iss_->good() && iss_->peek() != c) forward()


  parser::parser(const std::string& str)
  {
    new_iss = true;
    iss_ = new std::istringstream(str);
  }

  parser::parser(std::istream& iss)
    {
      new_iss = false;
        iss_ = &iss;
    }

  parser::~parser()
    {
      if (new_iss)
        delete iss_;
    }

    object parser::parse()
    {
        // Changing the locale for "C" number parsing system
        // the old locale is saved to be restored after the
        // parsing, as some applications use a different one
        char * old_locale = setlocale(LC_NUMERIC, NULL);
        setlocale(LC_NUMERIC, "C");

        // init the parsing state
        line_ = column_ = 0;
        
        // loop through the whole stream
        while (iss_->good())
        {
            skipws();

            char_type c = iss_->peek();
            if (c == '{')
            {
        setlocale(LC_NUMERIC, old_locale);
                return parse_object();
            }
            else if (isalpha(c))
            {
                object parsed;

                while (iss_->good())
                {
                    auto sh = parse_variable();
                    parsed.add<const shaun&>(sh.first, *sh.second);
                    delete sh.second;
                    skipws();
                }

        setlocale(LC_NUMERIC, old_locale);
                return parsed;
            }
            else
            {
                PARSE_ERROR(root must be an object);
            }

            forward();
        }

        PARSE_ERROR("couldn't parse this string");
        

    }

    void parser::skipws()
    {
        while (iss_->good() && (isspace(iss_->peek())
                            || iss_->peek() == ','
                            || iss_->peek() == '('
                            || iss_->peek() == '/'))
        {
            switch (iss_->peek())
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
        switch (iss_->peek())
        {
            case '(':
                TAKE_TO(')');
                break;
            case '/':
                forward();
                if (iss_->peek() == '/') { TAKE_TO('\n'); return; }
                if (iss_->peek() == '*')
                {
                    while (iss_->good() && iss_->peek() != '/')
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
        if (iss_->peek() == '\n')
        {
            ++line_;
            column_ = 0;
        }

        iss_->ignore();
    }

    object parser::parse_object()
    {
        PARSE_ASSERT(iss_->peek() == '{', expected object value);
        object obj;

        forward();
        skipws();

        while (iss_->good() && iss_->peek() != '}')
        {
          auto sh = parse_variable();
            obj.add<const shaun&>(sh.first, *sh.second);
            delete sh.second;
            skipws();
        }

        forward();
        return obj;
    }

    std::pair<std::string, shaun*> parser::parse_variable()
    {
        PARSE_ASSERT(isalpha(iss_->peek()), invalid variable name);
        
        std::string name = parse_name();

        skipws();
        PARSE_ASSERT(iss_->peek() == ':', expected variable separator ':');
        forward();
        skipws();

        return std::make_pair(name, parse_value());
    }

    std::string parser::parse_name()
    {
        PARSE_ASSERT(isalpha(iss_->peek()) || iss_->peek() == '_', names must start with a letter or '_');
        std::string ret;
        char_type c;
        while (isalnum(c = iss_->peek()) || c == '_')
        {
            ret.push_back(c);
            forward();
        }

        PARSE_ASSERT(ret != "true" && ret != "false"
            , true or false are invalid names);
        return ret;
    }

    shaun* parser::parse_value()
    {
        shaun * ret = 0;

        char_type c = iss_->peek();
        if (c == '"')
        {
            ret = new string(parse_string());
        }
        
        if (isdigit(c) || c == '-' || c == '.')
        {
            ret = new number(parse_number());
        }
        
        if (c == '{')
        {
            ret = new object(parse_object());
        }
        
        if (c == 't' || c == 'f')
        {
            ret = new boolean(parse_boolean());
        }
        
        if (c == '[')
        {
            ret = new list(parse_list());
        }
        
        if (ret)
          return ret;

        std::string err = "illegal character:  ";
        err[err.size() - 1] = c;
        throw parse_error(line_, column_, err);
    }

    string parser::parse_string()
    {
        PARSE_ASSERT(iss_->peek() == '"', expected string value);
        size_t start_col = column_;
        forward();

        std::string str;
        char_type c = '\0';
        bool get_line = false;
        bool nows     = false;

        // get a normal string OR the first line of a multiline string
        while (iss_->good() && (c = iss_->peek()) != '\n' && c != '"')
        {
            if (!isspace(c)) nows = true;

            // skipped characters
            if (c == '\\')
            {
              forward();
              str.push_back(iss_->peek());
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
        while (iss_->good() && (c = iss_->peek()) != '"')
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
                  str.push_back(iss_->peek());
                }
                else
                  str.push_back(c);

                get_line = true;
            }

            forward();
        }

        // discard last new line, if existing
        if (str[str.size() - 1] == '\n') str.resize(str.size() - 1);

        PARSE_ASSERT(iss_->good(), unexpected EOF while parsing string);

        forward();        

        return string(str);
    }

    number parser::parse_number()
    {
        std::string num;
        std::string unit;
        char_type c;
        while (iss_->good() && ((c = iss_->peek()) == 'E'
            || c == 'e'
            || c == '-'
            || isdigit(c)
            || c == '.'
            || c == '+'))
        {
            num.push_back(c);
            forward();
        }

        size_t before_unit = iss_->tellg();
        skipws();
        try
        {
            unit = parse_name();
        }
        catch (...)
        {
            unit = "none";
            iss_->seekg(before_unit, iss_->beg);
        }
        skipws();

        try
        {
            double dbl = std::stod(num);
                if (iss_->peek() == ':')
                {
                    iss_->seekg(before_unit, iss_->beg);
                    return number(dbl, "");
                }
                else
                {
                    return number(dbl, unit);
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

    boolean parser::parse_boolean()
    {
        std::string ret;
        char_type c;
        while (isalpha(c = iss_->peek()))
        {
            ret.push_back(c);
            forward();
        }

        PARSE_ASSERT(ret == "true" || ret == "false", expected boolean value);
        return boolean(ret == "true");
    }

    list parser::parse_list()
    {
        PARSE_ASSERT(iss_->peek() == '[', expected list value);
        list ret;

        forward();
        skipws();
        while (iss_->good() && iss_->peek() != ']')
        {
          shaun * sh = parse_value();
            ret.push_back(*sh);
            delete sh;
            skipws();
        }

        forward();
        return ret;
    }

    template<typename T> int parser::signum(T val)
    {
        return (T(0) < val) - (val < T(0));
    }

    object parse(const std::string& str)
    {
        parser p(str);
        return p.parse();
    }

    object parse(std::istream& str)
    {
        parser p(str);
        return p.parse();
    }

    object parse_file(const std::string& str)
    {
      std::ifstream file(str);
        return parser(file).parse();
    }
} // namespace

