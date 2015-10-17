#include "sweeper.hpp"
#include <sstream>
#include <string>

namespace shaun
{

sweeper::sweeper(shaun * root)
{
    root_ = root;
    name_ = "root";
}

sweeper::sweeper(const sweeper& swp)
{
    name_ = swp.name_;
    root_ = swp.root_;
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
                throw ("expected object but " + name + " has type " + type_to_string(ret->type()));

            name.assign(first, second);
            ret = static_cast<object*>(ret)->get_variable(name);

            break;
        }

        if (*(second) == ':' || *(second) == '[')
        {
            if (ret->type() != Type::object)
                throw ("expected object but " + name + " has type " + type_to_string(ret->type()));

            name.assign(first, second);

            ret = static_cast<object*>(ret)->get_variable(name);

            if (*(second) == '[')
            {
                if (ret->type() != Type::list)
                    throw ("expected list but " + name + " has type " + type_to_string(ret->type()));

                first = ++second;
                while (*second != ']') ++second;
                

                int index = std::stoi(std::string(first, second));

                try
                {
                    ret = static_cast<list*>(ret)->elements().at(index).get();
                }
                catch (...)
                {
                    throw ("index out of range");
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
        throw (name_ + " is not a list");

    try
    {
        next_.reset(new sweeper(static_cast<list*>(root_)->elements().at(i).get()));
    }
    catch (...)
    {
        throw ("index out of range");
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
            throw ("expected " + type_to_string(Type::TYPE) + " but " + name_ + " has type " + type_to_string(root_->type()));\
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

} // namespace