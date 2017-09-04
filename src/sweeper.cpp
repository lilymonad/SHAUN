#include <SHAUN/sweeper.hpp>
#include <SHAUN/exception.hpp>
#include <sstream>
#include <string>

namespace shaun
{

null sweeper::static_null;
sweeper sweeper::null_sweeper(static_null);

sweeper::sweeper(shaun &root) : name_("root"), root_(&root)
{
}

sweeper::sweeper(const sweeper& swp) : name_(swp.name_), root_(swp.root_)
{
}

sweeper::~sweeper()
{
}

shaun * sweeper::compute_path(const std::string& path) const
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

sweeper& sweeper::at(size_t i)
{
    if (root_->type() != Type::list)
        throw type_error(Type::list, root_->type(), name_);

    try
    {
        next_.reset(new sweeper(*(static_cast<list*>(root_)->elements().at(i).get())));
    }
    catch (...)
    {
        throw list_index_error();
    }
    
    return *next_;
}

sweeper& sweeper::operator[](size_t i)
{
  try
  {
    return at(i);
  }
  catch (...)
  {
    return null_sweeper;
  }
}

sweeper& sweeper::get(const std::string& path)
{
    next_.reset(new sweeper(*compute_path(path)));
    return *next_;
}

sweeper& sweeper::operator()(const std::string& path)
{
  try
  {
    return get(path);
  }
  catch (...)
  {
    return null_sweeper;
  }
}

#define VALUE(TYPE) template<>\
    TYPE sweeper::value() const    \
    {                      \
        if (root_->type() != Type::TYPE)\
            throw type_error(Type::TYPE, root_->type(), name_);\
                           \
        return static_cast<TYPE>(*root_);\
    }\
    template<> TYPE sweeper::with_default(TYPE def) const { try { return value<TYPE>(); } catch (...) { return def; } }\
\
    template<>\
    const TYPE & sweeper::value() const    \
    {                      \
        if (root_->type() != Type::TYPE)\
            throw type_error(Type::TYPE, root_->type(), name_);\
                           \
        return static_cast<const TYPE&>(*root_);\
    }\
    template<> const TYPE & sweeper::with_default(const TYPE& def) const { try { return value<const TYPE&>(); } catch (...) { return def; } }\
\
    template<>\
    TYPE & sweeper::value() const    \
    {                      \
        if (root_->type() != Type::TYPE)\
            throw type_error(Type::TYPE, root_->type(), name_);\
                           \
        return static_cast<TYPE&>(*root_);\
    }\
    template<> TYPE & sweeper::with_default(TYPE& def) const { try { return value<TYPE&>(); } catch (...) { return def; } }



#define VALUE_FROM_BOOL(TYPE) template<>\
    TYPE sweeper::value() const\
    {\
      if (root_->type() != Type::boolean)\
        throw type_error(Type::boolean, root_->type(), name_);\
      return *static_cast<boolean*>(root_);\
    } template<>\
TYPE sweeper::with_default(TYPE def) const { try { return value<TYPE>(); } catch (...) { return def; } }

#define VALUE_FROM_NUM(TYPE) template<>\
  TYPE sweeper::value() const\
{\
  if (root_->type() != Type::number)\
    throw type_error(Type::number, root_->type(), name_);\
  return *static_cast<number*>(root_);\
} template<>\
TYPE sweeper::with_default(TYPE def) const { try { return value<TYPE>(); } catch (...) { return def; } }

#define VALUE_FROM_STRING(TYPE) template<>\
  TYPE sweeper::value() const\
{\
  if (root_->type() != Type::string)\
    throw type_error(Type::string, root_->type(), name_);\
  return *static_cast<string*>(root_);\
} template<>\
TYPE sweeper::with_default(TYPE def) const { try { return value<TYPE>(); } catch (...) { return def; } }

VALUE(null)
VALUE(boolean)
VALUE(number)
VALUE(string)
VALUE(object)
VALUE(list)
VALUE_FROM_BOOL(bool)
VALUE_FROM_NUM(char)
VALUE_FROM_NUM(unsigned char)
VALUE_FROM_NUM(short)
VALUE_FROM_NUM(unsigned short)
VALUE_FROM_NUM(int)
VALUE_FROM_NUM(unsigned int)
VALUE_FROM_NUM(long)
VALUE_FROM_NUM(unsigned long)
VALUE_FROM_NUM(wchar_t)
VALUE_FROM_NUM(float)
VALUE_FROM_NUM(double)
VALUE_FROM_NUM(long double)
VALUE_FROM_STRING(const char *)
VALUE_FROM_STRING(std::string)

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
    case Type::string: return (*static_cast<string*>(root_)).size();
    case Type::object: return static_cast<object*>(root_)->size();
    case Type::list: return static_cast<list*>(root_)->size();
    case Type::boolean: return (size_t)((bool)(*static_cast<boolean*>(root_)));
    default: return 0;
  }
}

} // namespace
