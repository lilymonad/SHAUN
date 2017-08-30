#ifndef shaun_HPP
#define shaun_HPP

#include <map>
#include <vector>
#include <memory>

namespace shaun
{
  
class visitor;

enum class Type : int { object, list, boolean, number, string, null };

std::string type_to_string(Type t);

class shaun
{
public:    
    virtual ~shaun() {}

    Type type() const;
    bool is_null() const;

    /**
     * returns the index of a potential child
     * node of the current node
     */
    virtual int index_of(shaun * child) const;

    /**
     * node part of the visitor pattern
     */
    virtual void visited(visitor& v) = 0;

    virtual operator bool() const;
    virtual operator char() const;
    virtual operator unsigned char() const;
    virtual operator short() const;
    virtual operator unsigned short() const;
    virtual operator int() const;
    virtual operator unsigned int() const;
    virtual operator long() const;
    virtual operator unsigned long() const;
    virtual operator wchar_t() const;
    virtual operator float() const;
    virtual operator double() const;
    virtual operator long double() const;
    virtual operator std::string() const;
    virtual operator const char *() const;

protected:
    shaun() = delete;
    shaun(Type t);

    Type type_;
};



class list : public shaun
{
public:
  
    using vector = std::vector<std::shared_ptr<shaun> >;
    using iterator = vector::iterator;
    using const_iterator = vector::const_iterator;
    
    list();
    list(const list& l);

    ~list();

    // cf. shaun::visited()
    void visited(visitor& v);

    /**
     * same as std::vector
     */
    void push_back(const shaun& elem);

    /**
     * c++11 foreach loop integration
     */
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    /**
     * returns the underlying vector of the list
     */
    const vector& elements();

    shaun& operator[](size_t i);
    const shaun& operator[](size_t i) const;

    /**
     * different from operator[]
     * as it can return a casted value
     * (i.e. list.at<double>(0) can return a double from a shaun::number)
     */
    template<class C>
    C& at(size_t i);
    template<class C>
    const C& at(size_t i) const;
    
    size_t size() const;
    virtual int index_of(shaun *child) const;

private:
    vector elements_;
};

class boolean : public shaun
{
public:
    boolean();
    boolean(const boolean& b);

    boolean(bool);
    boolean(char);
    boolean(unsigned char);
    boolean(short int);
    boolean(unsigned short int);
    boolean(int);
    boolean(unsigned int);
    boolean(long);
    boolean(unsigned long);
    boolean(wchar_t);
    boolean(float);
    boolean(double);
    boolean(long double);

    void visited(visitor& v);

 
    virtual operator bool() const;
    virtual operator char() const;
    virtual operator unsigned char() const;
    virtual operator short() const;
    virtual operator unsigned short() const;
    virtual operator int() const;
    virtual operator unsigned int() const;
    virtual operator long() const;
    virtual operator unsigned long() const;
    virtual operator wchar_t() const;
    virtual operator float() const;
    virtual operator double() const;
    virtual operator long double() const;

private:
    bool value;
};

class number : public shaun
{
public:
    number();
    number(const number& num);

    number(bool, const std::string& u = "");
    number(char, const std::string& u = "");
    number(unsigned char, const std::string& u = "");
    number(short int, const std::string& u = "");
    number(unsigned short int, const std::string& u = "");
    number(int, const std::string& u = "");
    number(unsigned int, const std::string& u = "");
    number(long, const std::string& u = "");
    number(unsigned long, const std::string& u = "");
    number(wchar_t, const std::string& u = "");
    number(float, const std::string& u = "");
    number(double, const std::string& u = "");
    number(long double, const std::string& u = "");

    void visited(visitor& v);

    const std::string& unit() const;
 
    virtual operator bool() const;
    virtual operator char() const;
    virtual operator unsigned char() const;
    virtual operator short() const;
    virtual operator unsigned short() const;
    virtual operator int() const;
    virtual operator unsigned int() const;
    virtual operator long() const;
    virtual operator unsigned long() const;
    virtual operator wchar_t() const;
    virtual operator float() const;
    virtual operator double() const;
    virtual operator long double() const;

private:
    double value;
    std::string un;
};

class string : public shaun
{
public: 
    string();
    string(const string& str);
    string(const std::string& str);
    string(const char * str);

    void visited(visitor& v);

    operator std::string() const
    {
      return value;
    }

    operator const char *() const
    {
      return value.c_str();
    }

    bool operator==(const char * str)
    {
      return value == str;
    }

    bool operator==(const std::string& str)
    {
      return value == str;
    }

    size_t size() const
    {
      return value.size();
    }
private:
    std::string value;
};

class null : public shaun
{
public:
    null();

    void visited(visitor& v);

    static null Null;
};

class object : public shaun
{
public:
    using map = std::map<std::string, std::shared_ptr<shaun> >;
    using iterator = map::iterator;
    using const_iterator = map::const_iterator;

    friend class sweeper;

    object();
    object(const object& obj);

    ~object();
    void visited(visitor& v);

    object& operator=(const object& obj);

    template<class C>
    void add(std::pair<std::string, const C&> pair);

    void add(std::pair<std::string, const char*> pair);

    //template<class C>
    //void add(std::pair<std::string, const C> pair);

    template<class C>
    void add(const std::string& name, const C& ptr);
    
    void add(const std::string& name, const char *ptr);

    //template<class C>
    //void add(const std::string& name, const C ptr);

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    template<class C>
    C& get(const std::string& name);

    template<class C>
    C get_with_default(const C&, const std::string&) const;
    std::string get_with_default(const char * def, const std::string& name) const;

    template<class C>
    const C& get(const std::string& name) const;

    Type type_of(const std::string& name) const;
    
    size_t size() const;
    virtual int index_of(shaun *child) const;

    const std::map<std::string, std::shared_ptr<shaun> >& variables() const;
private:

    shaun * get_variable(const std::string& name) const;
    std::map<std::string, std::shared_ptr<shaun> > variables_;
};

} // namespace

#endif
