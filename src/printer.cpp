#include <SHAUN/printer.hpp>

namespace shaun
{

printer::printer(std::ostream * stream)
{
	stream_ = stream;
}

printer::~printer()
{
	
}

void printer::visit(shaun& s)
{
    if (stream_)
        visitor::visit(s);
}

void printer::visit_null(null& n)
{
    *stream_ << "null";
}

void printer::visit_boolean(boolean& b)
{
    *stream_ << (b ? "true" : "false");
}

void printer::visit_number(number& n)
{
    *stream_ << (double)n;
    *stream_ << n.unit();
}

void printer::visit_string(string& s)
{
    std::string ss = s;
    std::string to_print;
    for (const auto &c : ss)
    {
      switch (c)
      {
        case '\\':
        case '"':
          to_print.push_back('\\');
          break;
        default:
          break;
      }
      to_print.push_back(c);
    }

    *stream_ << '"' << to_print << '"';
}

void printer::visit_list(list& l)
{
    *stream_ << "[ ";
    indent_levels_.push_back(2);

    for (unsigned int i = 0; i < l.size(); ++i)
    {
        visit(l[i]);
        *stream_ << '\n';
        
        if (i == l.size() - 1)
            indent_levels_.pop_back();

        print_spaces(spaces());
    }

    *stream_ << ']';
}

void printer::visit_object(object& o)
{
    *stream_ << "{ ";
    indent_levels_.push_back(2);
    unsigned int i = 0;

    for (std::pair<std::string, std::shared_ptr<shaun> > p : o.variables())
    {
        *stream_ << p.first;
        *stream_ << ": ";

        indent_levels_.push_back(p.first.size() + 2);

        visit(*(p.second));

        *stream_ << '\n';
        indent_levels_.pop_back();

        if (++i == o.size())
            indent_levels_.pop_back();

        print_spaces(spaces());
    }

    *stream_ << '}';
}

unsigned int printer::spaces()
{
    unsigned int ret = 0;
    for (unsigned int n : indent_levels_)
        ret += n;

    return ret;
}

void printer::print_spaces(long long int n)
{
    for (long int i = 0; i < n; ++i)
        *stream_ << ' ';
}

} // namespace
