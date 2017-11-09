#include <SHAUN/parser.hpp>
#include <cctype>
#include <iterator>

#define PARSE_ERROR(str) throw parse_error(it_->_lin, it_->_col, #str)
#define PARSE_ASSERT(cond, str) if (!(cond)) PARSE_ERROR(str)
#define TAKE_TO(c) while (iss_->good() && iss_->peek() != c) forward()

namespace shaun
{

class token
{
public:
  enum t { LBRACKET=0, RBRACKET=1, LHOOK=2, RHOOK=3, COMMENT=4, ATTRIB_SEP=5, STRING_LIT=6, NUMERIC_LIT=7, BOOLEAN_LIT=8, NULL_LIT=9, NAME=10 };
  t _type;
  std::string _string_value;
  double _double_value;
  bool _bool_value;
  int _lin; int _col;
};

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
    int escaped(int i)
    {
      switch (i)
      {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '0': return '\0';
        case '\\': return '\\';
        case '"': return '"';
        default: return i;
      }
    }

    void make_lexer()
    {
      std::vector<token> ret_;
      while (iss_->good())
      {
        char c = iss_->peek();

        if (c == '"')
        {
          ret_.push_back(lex_string());
        }
        else if (c == '.' || std::isdigit(c) || c == '+' || c == '-')
        {
          ret_.push_back(lex_double());
        }
        else if (c == '{') { token t; t._type = token::LBRACKET; ret_.push_back(t); forward(); }
        else if (c == '}') { token t; t._type = token::RBRACKET; ret_.push_back(t); forward(); }
        else if (c == '[') { token t; t._type = token::LHOOK; ret_.push_back(t); forward(); }
        else if (c == ']') { token t; t._type = token::RHOOK; ret_.push_back(t); forward(); }
        else if (c == ':') { token t; t._type = token::ATTRIB_SEP; ret_.push_back(t); forward(); }
        else if (std::isalpha(c))
        {
          ret_.push_back(lex_name());
        }
        
        skipws();
      }

      tokens_ = ret_;
      it_ = tokens_.begin();
    }

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
    void parse_null();

    token lex_double()
    {
      std::string num;
      char_type c;
      while (iss_->good() && ((c = iss_->peek()) == 'E'
          || c == 'e'
          || c == '-'
          || std::isdigit(c)
          || c == '.'
          || c == '+'))
      {
          num.push_back(c);
          forward();
      }

      token ret;
      ret._col = column_; ret._lin = line_;
      ret._double_value = std::stod(num);
      ret._type = token::NUMERIC_LIT;
      return ret;
    }

    token lex_string()
    {   
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
              str.push_back(escaped(iss_->peek()));
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
                  str.push_back(escaped(iss_->peek()));
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

        token ret;
        ret._col = column_; ret._lin = line_;
        ret._string_value = str;
        ret._type = token::STRING_LIT;
        return ret;
    }

    token lex_name()
    {
      std::string str;

      char_type c;
      while (std::isalnum(c = iss_->peek()) || c == '_')
      {
        str.push_back(c);
        forward();
      }

      token ret;
      ret._col = column_; ret._lin = line_;
      ret._string_value = str;
      if (str == "false" || str == "true")
        ret._type = token::BOOLEAN_LIT;
      else if (str == "null")
        ret._type = token::NULL_LIT;
      else
        ret._type = token::NAME;
      return ret;
    }

    template<typename T> int signum(T val);
    
    std::istream * iss_;
    std::vector<token> tokens_;
    std::vector<token>::iterator it_;
    size_t line_;
    size_t column_;
    bool new_iss;
};

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
        make_lexer();

        setlocale(LC_NUMERIC, old_locale);
        if (it_->_type == token::LBRACKET)
        {
          return parse_object();
        }
        else
        {
          object ret;
          while (it_ != tokens_.end())
          {
            auto sh = parse_variable();
            ret.add<const shaun&>(sh.first, *sh.second);
            delete sh.second;
            ++it_;
          }

          return ret;
        }
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
        PARSE_ASSERT(it_->_type == token::LBRACKET, expected object value);
        ++it_;
        object obj;

        while (it_->_type != token::RBRACKET)
        {
          auto sh = parse_variable();
            obj.add<const shaun&>(sh.first, *sh.second);
              delete sh.second;
          ++it_;
        }

        return obj;
    }

    std::pair<std::string, shaun*> parser::parse_variable()
    {
        std::string name = parse_name();
        ++it_;
        PARSE_ASSERT(it_->_type == token::ATTRIB_SEP, expected variable separator ':');
        ++it_;

        return std::make_pair(name, parse_value());
    }

    std::string parser::parse_name()
    {
        PARSE_ASSERT(it_->_type == token::NAME, expected a name);
        return it_->_string_value;
    }

    shaun* parser::parse_value()
    {
        shaun * ret = 0;

        switch (it_->_type)
        {
          case token::STRING_LIT:
            ret = new string(parse_string()); break;
          case token::NUMERIC_LIT:
            ret = new number(parse_number()); break;
          case token::LBRACKET:
            ret = new object(parse_object()); break;
          case token::BOOLEAN_LIT:
            ret = new boolean(parse_boolean()); break;
          case token::LHOOK:
            ret = new list(parse_list()); break;
          case token::NULL_LIT:
            ret = new null(); break;
          default: break;
        }
        
        if (ret)
          return ret;

        PARSE_ERROR(expected a value);
    }

    string parser::parse_string()
    {
      PARSE_ASSERT(it_->_type == token::STRING_LIT, expected a string);
      return string(it_->_string_value);
    }

    number parser::parse_number()
    {
      double num;
      std::string unit;
      PARSE_ASSERT(it_->_type == token::NUMERIC_LIT, expected a numeric value);
      num = it_->_double_value;
      if ((it_+1)->_type == token::NAME && (it_+2)->_type != token::ATTRIB_SEP)
      {
        unit = it_->_string_value;
        ++it_;
      }
      else
        unit = "";

      return number(num, unit);
    }

    boolean parser::parse_boolean()
    {
        PARSE_ASSERT(it_->_type == token::BOOLEAN_LIT, expected boolean value);
        return boolean(it_->_bool_value);
    }

    list parser::parse_list()
    {
        PARSE_ASSERT(it_->_type == token::LHOOK, expected list value);
        list ret;

        while ((++it_)->_type != token::RHOOK)
        {
          shaun * sh = parse_value();
          ret.push_back(*sh);
          delete sh;
        }
        return ret;
    }

    void parser::parse_null() {}

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

