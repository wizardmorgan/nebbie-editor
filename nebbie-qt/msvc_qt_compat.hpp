#pragma once

// Qt 6.5 + MSVC 19.40+ (VS 2022 17.10+) removed the stdext namespace helpers that older Qt headers reference.
#if defined(_MSC_VER) && (_MSC_VER >= 1940)
#include <stdexcept>

namespace stdext {
[[noreturn]] inline void throw_array_range(const char* message) {
    throw std::out_of_range(message);
}
} // namespace stdext
#endif
