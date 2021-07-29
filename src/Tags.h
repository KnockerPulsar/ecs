#pragma once
#include <cstddef>
namespace pong
{
    // For use with system internals, because I'm lazy and can't use templates
    enum tags
    {
        null,
        indep,
        coll,
        particle,
        
    };

    // https://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
    struct TagsHashClass
    {
        template <typename T>
        std::size_t operator()(T t) const noexcept 
        {
            return (int)(t);
        }
    };

}