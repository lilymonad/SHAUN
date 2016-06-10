#include <SHAUN/sweeper.hpp>
#include <SHAUN/exception.hpp>
#include <sstream>
#include <string>

namespace shaun
{

sweeper::sweeper(shaun * root) : root_(root), name_("root")
{
}

sweeper::sweeper(const sweeper& swp) : root_(swp.root_), name_(swp.name_)
{
}

sweeper::~sweeper()
{
}

shaun * sweeper::compute_path(const std::string& path)
{
    std::string::const_iterator first, second;
    std::string name = name_;
    shaun * ret = root_;

    first = second = path.begin();
    while (first != path.end())
    {
        if (second == path.end())
        {
            if (ret->type() != Type::object)
                throw type_error(Type::object, ret->type(), name);

            name.assign(first, second);
            ret = static_cast<object*>(ret)->get_variable(name);

            break;
        }

        if (*(second) == ':' || *(second) == '[')
        {
            if (ret->type() != Type::object)
                throw type_error(Type::object, ret->type(), name);

            name.assign(first, second);

            ret = static_cast<object*>(ret)->get_variable(name);

            if (*(second) == '[')
            {
                if (ret->type() != Type::list)
                    throw type_error(Type::object, ret->type(), name);

                first = ++second;
                while (*second != ']') ++second;
                

                int index = std::stoi(std::string(first, second));

                try
                {
                    ret = static_cast<list*>(ret)->elements().at(index).get();
                }
                catch (...)
                {
                    throw list_index_error();
                }

                ++second;
            }

            first = ++second;
        }
        else
            ++second;
    }

    return ret;
}

sweeper& sweeper::operator[](size_t i)
{
    if (root_->type() != Type::list)
        throw type_error(Type::list, root_->type(), name_);

    try
    {
        next_.reset(new sweeper(static_cast<list*>(root_)->elements().at(i).get()));
    }
    catch (...)
    {
        throw list_index_error();
    }
    
    return *next_;
}

sweeper& sweeper::get(const std::string& path)
{
    next_.reset(new sweeper(compute_path(path)));
    return *next_;
}

sweeper& sweeper::operator()(const std::string& path)
{
    return get(path);
}

#define VALUE(TYPE) template<>\
    TYPE& sweeper::value() const    \
    {                      \
        if (root_->type() != Type::TYPE)\
            throw type_error(Type::TYPE, root_->type(), name_);\
                           \
        return *static_cast<TYPE*>(root_);\
    }

VALUE(null)
VALUE(boolean)
VALUE(number)
VALUE(string)
VALUE(object)
VALUE(list)

Type sweeper::type() const
{
    return root_->type();
}

bool sweeper::is_null() const
{
  return root_->is_null();
}

size_t sweeper::size() const
{
  switch (root_->type())
  {
    case Type::null: return 0;
    case Type::number: return (size_t)((double)(*static_cast<number*>(root_)));
    case Type::string: return ((std::string)(*static_cast<string*>(root_))).size();
    case Type::object: return static_cast<object*>(root_)->size();
    case Type::list: return static_cast<list*>(root_)->size();
    case Type::boolean: return (size_t)((bool)(*static_cast<boolean*>(root_)));
    default: return 0;
  }
}

} // namespace
