#pragma once

#include <ctime>

namespace nebbie {

inline bool utc_tm_from_time_t(const std::time_t t, std::tm& out) {
#if defined(_WIN32) && defined(_MSC_VER)
    return gmtime_s(&out, &t) == 0;
#else
#if defined(_WIN32)
    const std::tm* tmp = std::gmtime(&t);
    if (tmp == nullptr) {
        return false;
    }
    out = *tmp;
    return true;
#else
    return gmtime_r(&t, &out) != nullptr;
#endif
#endif
}

} // namespace nebbie
