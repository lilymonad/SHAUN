#ifndef VISITOR_HPP
#define VISITOR_HPP

#include "shaun.hpp"
#include <cstddef>

namespace shaun
{

class sweeper
{
public:
    sweeper(const object& object);

    template<class C>
    C& value();

    sweeper& get(const std::string& path);
    sweeper& next(const std::string& path);
    sweeper& operator[](size_t i);

    void rewind();
private:
    shaun * compute_path(const std::string& path);

    std::string current_name_;
    shaun * current_;
    object root_;
};

} // namespace

#endif