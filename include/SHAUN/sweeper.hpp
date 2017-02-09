#ifndef VISITOR_HPP
#define VISITOR_HPP

#include <SHAUN/shaun.hpp>
#include <cstddef>

namespace shaun
{

class sweeper
{
public:
    sweeper(shaun * root);
    sweeper(const sweeper& swp);
    ~sweeper();
    
    sweeper& operator=(const sweeper& swp);

    /**
     * returns the value of the current shaun item
     */
    template<class C>
    C& value() const;

    /**
     * returns the type of the current shaun item
     */
    Type type() const;

    /**
     * tells wether the item is null
     */
    bool is_null() const;

    /**
     * returns the size of the current shaun item
     * according to the shaun::size() method
     */
    size_t size() const;

    /**
     * walks through a path, changing the
     * pointed shaun item
     */
    sweeper& get(const std::string& path);

    /**
     * gets to a shaun::list's item i
     */
    sweeper& operator[](size_t i);

    /**
     * sweeper::get() wrapper
     */
    sweeper& operator()(const std::string& path);

private:
    shaun * compute_path(const std::string& path);

    std::string name_;
    shaun * root_;
    
    std::unique_ptr<sweeper> next_;
};

} // namespace

#endif
