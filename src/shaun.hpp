#ifndef shaun_HPP
#define shaun_HPP

#include <map>
#include <vector>
#include <memory>
#include "shaun.hpp"

namespace shaun
{

enum class Type : int { object, list, boolean, number, string, null };

std::string type_to_string(Type t);

class shaun
{
public:    
    virtual ~shaun() {}

    Type type();

    bool is_null();
protected:
    Type type_;
};



class list : public shaun
{
public:
    list();
    list(const list& l);
    ~list();
    Type type();

    void push_back(shaun * elem);
    const std::vector<std::shared_ptr<shaun> >& elements();

    shaun * operator[](size_t i);

    template<class C>
    C& at(size_t i);
private:
    std::vector<std::shared_ptr<shaun> > elements_;
};

class boolean : public shaun
{
public:
    boolean();
    boolean(const boolean& b);
    boolean(bool yes);
    Type type();

    operator bool() const { return value; } 
private:
    bool value;
};

class number : public shaun
{
public:
    enum class Unit : int { deg, rad, none };

    number();
    number(const number& num);
    number(double val, Unit u);
    Type type();

    Unit unit();
    operator double() const { return value; }

    
private:
    double value;
    Unit un;
};

class string : public shaun
{
public: 
    string();
    string(const string& str);
    string(const std::string& str);
    Type type();

    explicit operator std::string() const
    {
        return value;
    }
private:
    std::string value;
};

class null : public shaun
{
public:
    null();
    Type type();

    bool is_null();

    static null Null;
};

class object : public shaun
{
public:
    friend class sweeper;
    object();
    object(const object& obj);
    ~object();
    Type type();

    object& operator=(const object& obj);

    template<class C>
    void add(std::pair<std::string, C *> pair);

    template<class C>
    void add(const std::string& name, C * ptr);

    template<class C>
    C& get(const std::string& name);

    Type type_of(const std::string& name);
private:

    shaun * get_variable(const std::string& name);
    std::map<std::string, std::shared_ptr<shaun> > variables_;
};

} // namespace

#endif