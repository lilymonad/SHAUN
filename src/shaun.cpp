#include <SHAUN/shaun.hpp>
#include <utility>
#include <algorithm>
#include <set>
#include <sstream>
#include <iostream>
#include <SHAUN/visitor.hpp>
#include <SHAUN/exception.hpp>

const char * NULL_STRING = "";

#define VISIT_FUN(x) void x::visited(visitor& v) { v.visit_ ## x (*this); }
#define OBJ_GET(x) template<>                                        \
    x& object::get<x> (const std::string& name)                      \
    {                                                                \
        if (get_variable(name)->type() == Type::x)                   \
            return *(static_cast<x*>(get_variable(name)));           \
                                                                     \
        throw type_error(Type::x, get_variable(name)->type(), name); \
    }                                                                \
    template<>                                                       \
    const x& object::get<x> (const std::string& name) const          \
    {                                                                \
        if (get_variable(name)->type() == Type::x)                   \
            return *(static_cast<const x*>(get_variable(name)));     \
                                                                     \
        throw type_error(Type::x, get_variable(name)->type(), name); \
    }\
    template<>\
    x object::get_with_default<x>(const x& def, const std::string& name) const\
    {\
      try\
      {\
        return get<x>(name);\
      }\
      catch (...)\
      {\
        return def;\
      }\
    }

#define OBJ_GET_DEF_BOOL(x) template<>\
    x object::get_with_default<x>(const x& def, const std::string& name) const\
    {\
      try\
      {\
        return get<boolean>(name);\
      }\
      catch (...)\
      {\
        return def;\
      }\
    }
#define OBJ_GET_DEF_NUM(x) template<>\
    x object::get_with_default<x>(const x& def, const std::string& name) const\
    {\
      try\
      {\
        return get<number>(name);\
      }\
      catch (...)\
      {\
        return def;\
      }\
    }


#define OBJ_ADD(x) template<>                                               \
    void object::add<x> (const std::string& name, const x& v)               \
    {                                                                       \
        x * ptr = new x(v);                                                 \
        std::shared_ptr<shaun> to_add(ptr);                                 \
        variables_.insert(std::make_pair(name, to_add));                    \
    }                                                                       \
                                                                            \
    template<>                                                              \
    void object::add<x> (std::pair<std::string, const x&> pair)                    \
    {                                                                       \
        x * ptr = new x(pair.second);                                       \
        std::shared_ptr<shaun> to_add(ptr);                                 \
        variables_.insert(std::make_pair(pair.first, to_add));              \
    }

#define OBJ_ADD_PRIM_TO_BOOLEAN(x) template<>                               \
  void object::add<x> (const std::string& name, const x& v)                 \
  {\
    shaun * ptr = new boolean(v);\
    std::shared_ptr<shaun> to_add(ptr);\
    variables_.insert(std::make_pair(name, to_add));\
  }                                                                       \
                                                                            \
    template<>                                                              \
    void object::add<x> (std::pair<std::string, const x&> pair)                    \
    {                                                                       \
        shaun * ptr = new boolean(pair.second);                                       \
        std::shared_ptr<shaun> to_add(ptr);                                 \
        variables_.insert(std::make_pair(pair.first, to_add));              \
    }

#define OBJ_ADD_PRIM_TO_NUMBER(x) template<>                               \
  void object::add<x> (const std::string& name, const x& v)                 \
  {\
    shaun * ptr = new number(v);\
    std::shared_ptr<shaun> to_add(ptr);\
    variables_.insert(std::make_pair(name, to_add));\
  }                                                                       \
                                                                            \
    template<>                                                              \
    void object::add<x> (std::pair<std::string, const x&> pair)                    \
    {                                                                       \
        shaun * ptr = new number(pair.second);                                       \
        std::shared_ptr<shaun> to_add(ptr);                                 \
        variables_.insert(std::make_pair(pair.first, to_add));              \
    }



#define OBJ_ADD_PRIM_TO_STRING(x) template<>                               \
  void object::add<x> (const std::string& name, x const & v)                 \
  {\
    shaun * ptr = new string(v);\
    std::shared_ptr<shaun> to_add(ptr);\
    variables_.insert(std::make_pair(name, to_add));\
  }                                                                       \
                                                                            \
    template<>                                                              \
    void object::add<x> (std::pair<std::string, x const &> pair)                    \
    {                                                                       \
        shaun * ptr = new string(pair.second);                                       \
        std::shared_ptr<shaun> to_add(ptr);                                 \
        variables_.insert(std::make_pair(pair.first, to_add));              \
    }





namespace shaun
{

std::ostream& operator<<(std::ostream& out, const exception& err)
{
    out << err.to_string();

    return out;
}

shaun::shaun(Type t) : type_(t)
{

}

bool shaun::is_null() const
{
    return type_ != Type::null;
}

int shaun::index_of(shaun *child) const
{
    return -1;
}

Type shaun::type() const
{
    return type_;
}

#define PRIM_CAST(type)\
  shaun::operator type() const { return 0; }\
  boolean::operator type() const { return value; }\
  number::operator type() const { return value; }

PRIM_CAST(bool)
PRIM_CAST(char)
PRIM_CAST(unsigned char)
PRIM_CAST(short)
PRIM_CAST(unsigned short)
PRIM_CAST(int)
PRIM_CAST(unsigned int)
PRIM_CAST(long)
PRIM_CAST(unsigned long)
PRIM_CAST(wchar_t)
PRIM_CAST(float)
PRIM_CAST(double)
PRIM_CAST(long double)

shaun::operator std::string() const
{
  return std::string();
}

shaun::operator const char *() const
{
  return NULL_STRING;
}

/*****************************
 *
 *     object functions
 *
 *****************************/

VISIT_FUN(object)

object::object() : shaun(Type::object)
{
}

object::object(const object& obj) : shaun(Type::object), variables_(obj.variables_)
{
}

object::~object()
{
}

object& object::operator=(const object& org)
{
    variables_ = org.variables_;
    return *this;
}

shaun * object::get_variable(const std::string& name) const
{
    try
    {
        return variables_.at(name).get();
    }
    catch (...)
    {
        return &null::Null;
    }
}

OBJ_GET(number)
OBJ_GET(object)
OBJ_GET(boolean)
OBJ_GET(string)
OBJ_GET(list)
OBJ_GET_DEF_BOOL(bool)

OBJ_GET_DEF_NUM(char)
OBJ_GET_DEF_NUM(unsigned char)
OBJ_GET_DEF_NUM(short int)
OBJ_GET_DEF_NUM(unsigned short int)
OBJ_GET_DEF_NUM(int)
OBJ_GET_DEF_NUM(unsigned int)
OBJ_GET_DEF_NUM(long)
OBJ_GET_DEF_NUM(unsigned long)
OBJ_GET_DEF_NUM(wchar_t)
OBJ_GET_DEF_NUM(float)
OBJ_GET_DEF_NUM(double)
OBJ_GET_DEF_NUM(long double)

template<>
std::string object::get_with_default<std::string>(const std::string& def, const std::string& name) const
{
  try
  {
    return get<string>(name);
  }
  catch (...)
  {
    return def;
  }
}

std::string object::get_with_default(const char * def, const std::string& name) const
{
  try
  {
    return get<string>(name);
  }
  catch (...)
  {
    return std::string(def);
  }
}

OBJ_ADD(number)
OBJ_ADD(object)
OBJ_ADD(boolean)
OBJ_ADD(string)
OBJ_ADD(list)
OBJ_ADD_PRIM_TO_BOOLEAN(bool)

OBJ_ADD_PRIM_TO_NUMBER(char)
OBJ_ADD_PRIM_TO_NUMBER(unsigned char)
OBJ_ADD_PRIM_TO_NUMBER(short int)
OBJ_ADD_PRIM_TO_NUMBER(unsigned short int)
OBJ_ADD_PRIM_TO_NUMBER(int)
OBJ_ADD_PRIM_TO_NUMBER(unsigned int)
OBJ_ADD_PRIM_TO_NUMBER(long)
OBJ_ADD_PRIM_TO_NUMBER(unsigned long)
OBJ_ADD_PRIM_TO_NUMBER(wchar_t)
OBJ_ADD_PRIM_TO_NUMBER(float)
OBJ_ADD_PRIM_TO_NUMBER(double)
OBJ_ADD_PRIM_TO_NUMBER(long double)

OBJ_ADD_PRIM_TO_STRING(std::string)
//OBJ_ADD_PRIM_TO_STRING(const char *)

void object::add(const std::string& name, const char *v)
{
    shaun * ptr = new string(v);
    std::shared_ptr<shaun> to_add(ptr);
    variables_.insert(std::make_pair(name, to_add));
}

void object::add(std::pair<std::string, const char*> pair)
{
    shaun * ptr = new string(pair.second);
    std::shared_ptr<shaun> to_add(ptr);
    variables_.insert(std::make_pair(pair.first, to_add));
}
  
object::iterator object::begin()
{
    return variables_.begin();
}

object::iterator object::end()
{
    return variables_.end();
}

object::const_iterator object::begin() const
{
    return variables_.begin();
}

object::const_iterator object::end() const
{
    return variables_.end();
}

template<> void object::add<null> (const std::string&, const null&) { }
template<> void object::add<null> (std::pair<std::string, const null&>) { }

template<>
void object::add<shaun> (const std::string& name, const shaun& ptr)
{
    switch (ptr.type())
    {
        case Type::number:
            variables_.insert(std::make_pair(name, std::shared_ptr<shaun>(new number(static_cast<const number&>(ptr)))));
        break;
        case Type::object:
            variables_.insert(std::make_pair(name, std::shared_ptr<shaun>(new object(static_cast<const object&>(ptr)))));
        break;
        case Type::boolean:
            variables_.insert(std::make_pair(name, std::shared_ptr<shaun>(new boolean(static_cast<const boolean&>(ptr)))));
        break;
        case Type::string:
            variables_.insert(std::make_pair(name, std::shared_ptr<shaun>(new string(static_cast<const string&>(ptr)))));
        break;
        case Type::list:
            variables_.insert(std::make_pair(name, std::shared_ptr<shaun>(new list(static_cast<const list&>(ptr)))));
        break;

        // no pollution
        case Type::null:
        default: break;
    }
}

template<>
void object::add<shaun> (std::pair<std::string, const shaun&> pair)
{
    add(pair.first, pair.second);
}

Type object::type_of(const std::string& name) const
{
    return get_variable(name)->type();
}

size_t object::size() const
{
  return variables_.size();
}

const std::map<std::string, std::shared_ptr<shaun> >& object::variables() const
{
    return variables_;
}

int object::index_of(shaun * child) const
{
    std::set<shaun*> elems;
    std::transform(variables_.begin(), variables_.end(), std::inserter(elems, elems.begin())
                   , [](const std::pair<std::string, std::shared_ptr<shaun> >& p) { return p.second.get(); });
    return std::distance(elems.begin(), elems.find(child));
}

/*****************************
 *
 *     list functions
 *
 *****************************/

VISIT_FUN(list)

list::list() : shaun(Type::list)
{
}

list::list(const list& l) : shaun(Type::list), elements_(l.elements_)
{
}

list::~list()
{
}

void list::push_back(const shaun& ptr)
{
    switch (ptr.type())
    {
        case Type::number:
            elements_.push_back(std::shared_ptr<shaun>(new number(static_cast<const number&>(ptr))));
        break;
        case Type::object:
            elements_.push_back(std::shared_ptr<shaun>(new object(static_cast<const object&>(ptr))));
        break;
        case Type::boolean:
            elements_.push_back(std::shared_ptr<shaun>(new boolean(static_cast<const boolean&>(ptr))));
        break;
        case Type::string:
            elements_.push_back(std::shared_ptr<shaun>(new string(static_cast<const string&>(ptr))));
        break;
        case Type::list:
            elements_.push_back(std::shared_ptr<shaun>(new list(static_cast<const list&>(ptr))));
        break;

        // no pollution
        case Type::null:
        default: break;
    }
}

list::iterator list::begin()
{
    return elements_.begin();
}

list::iterator list::end()
{
    return elements_.end();
}

list::const_iterator list::begin() const
{
    return elements_.begin();
}

list::const_iterator list::end() const
{
    return elements_.end();
}

const std::vector<std::shared_ptr<shaun> >& list::elements()
{
    return elements_;
}

shaun& list::operator[](size_t i)
{
    return *elements_[i];
}

const shaun& list::operator[](size_t i) const
{
    return *elements_[i];
}

#define LIST_AT(TYPE) template<>\
    TYPE& list::at<TYPE>(size_t i)\
    {\
        shaun * ptr = elements_[i].get();\
        if (ptr->type() != Type::TYPE)\
            throw type_error(Type::TYPE, ptr->type(), "list element");\
\
        return *static_cast<TYPE*>(ptr);\
    }\
    template<>\
    const TYPE& list::at<TYPE>(size_t i) const\
    {\
        shaun * ptr = elements_[i].get();\
        if (ptr->type() != Type::TYPE)\
            throw type_error(Type::TYPE, ptr->type(), "list element");\
\
        return *static_cast<const TYPE*>(ptr);\
    }

LIST_AT(list)
LIST_AT(object)
LIST_AT(number)
LIST_AT(boolean)
LIST_AT(string)

size_t list::size() const
{
  return elements_.size();
}

int list::index_of(shaun *child) const
{
    return std::distance(elements_.begin(), std::find(elements_.begin(), elements_.end(), std::shared_ptr<shaun>(child)));
}

/*****************************
 *
 *     boolean functions
 *
 *****************************/

VISIT_FUN(boolean)

boolean::boolean() : shaun(Type::boolean), value(false)
{
}

boolean::boolean(const boolean& b) : shaun(Type::boolean), value(b.value)
{
}

#define BOOLEAN_PRIM_CTOR(x) boolean::boolean(x val) : shaun(Type::boolean), value(val) {}
BOOLEAN_PRIM_CTOR(bool)
BOOLEAN_PRIM_CTOR(char)
BOOLEAN_PRIM_CTOR(unsigned char)
BOOLEAN_PRIM_CTOR(short int)
BOOLEAN_PRIM_CTOR(unsigned short int)
BOOLEAN_PRIM_CTOR(int)
BOOLEAN_PRIM_CTOR(unsigned int)
BOOLEAN_PRIM_CTOR(long)
BOOLEAN_PRIM_CTOR(unsigned long)
BOOLEAN_PRIM_CTOR(wchar_t)
BOOLEAN_PRIM_CTOR(float)
BOOLEAN_PRIM_CTOR(double)
BOOLEAN_PRIM_CTOR(long double)


/*****************************
 *
 *     number functions
 *
 *****************************/

VISIT_FUN(number)

number::number() : shaun(Type::number), value(0), un("")
{
}

number::number(const number& num) : shaun(Type::number)
{
    value = num.value;
    un    = num.un;
}

#define NUMBER_PRIM_CTOR(x) number::number(x val, const std::string& u) : shaun(Type::number), value(val), un(u) {}
NUMBER_PRIM_CTOR(bool)
NUMBER_PRIM_CTOR(char)
NUMBER_PRIM_CTOR(unsigned char)
NUMBER_PRIM_CTOR(short int)
NUMBER_PRIM_CTOR(unsigned short int)
NUMBER_PRIM_CTOR(int)
NUMBER_PRIM_CTOR(unsigned int)
NUMBER_PRIM_CTOR(long)
NUMBER_PRIM_CTOR(unsigned long)
NUMBER_PRIM_CTOR(wchar_t)
NUMBER_PRIM_CTOR(float)
NUMBER_PRIM_CTOR(double)
NUMBER_PRIM_CTOR(long double)

const std::string& number::unit() const
{
    return un;
}

/*****************************
 *
 *     string functions
 *
 *****************************/

VISIT_FUN(string)

string::string() : shaun(Type::string)
{
}

string::string(const string& str) : shaun(Type::string), value(str.value)
{
}

string::string(const std::string& str) : shaun(Type::string), value(str)
{
}

string::string(const char * str) : shaun(Type::string), value(str)
{
}
                                

/*****************************
 *
 *     null functions
 *
 *****************************/

VISIT_FUN(null)

null::null() : shaun(Type::null)
{
}

null null::Null;

std::string type_to_string(Type t)
{
    switch (t)
    {
        case Type::object: return "object";
        case Type::list: return "list";
        case Type::boolean: return "boolean";
        case Type::number: return "number";
        case Type::string: return "string";
        case Type::null: return "null";
        default: return "unknown";
    }
}

}  // namespace
