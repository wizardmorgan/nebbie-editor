#pragma once

// Qt 6.5 headers reference stdext:: on some MSVC toolsets; ensure legacy helpers resolve.
#if defined(_MSC_VER)
#include <xutility>
#endif
