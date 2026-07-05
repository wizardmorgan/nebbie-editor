#pragma once

// Qt 6.5 + MSVC 19.40+ (VS 2022 17.10+) removed stdext helpers that older Qt headers reference.
#if defined(_MSC_VER) && (_MSC_VER >= 1940)
#include <cstddef>
#include <stdexcept>

namespace stdext {

[[noreturn]] inline void throw_array_range(const char* message) {
    throw std::out_of_range(message);
}

// Raw-pointer stand-in matching Qt's non-MSVC QT_MAKE_CHECKED_ARRAY_ITERATOR path.
template <typename T, typename Sz>
inline T* make_checked_array_iterator(T* ptr, Sz /*size*/, Sz offset = 0) {
    return ptr + offset;
}

} // namespace stdext
#endif
