#pragma once

#include "world.hpp"

#include <string>
#include <vector>

namespace nebbie {

enum class ValidationSeverity {
    warning,
    error,
};

enum class ValidationTarget {
    none,
    room,
    mob,
    object,
    zone,
    shop,
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::error;
    std::string category;
    std::string message;
    ValidationTarget target = ValidationTarget::none;
    long target_vnum = 0;
    int zone_num = 0;
    int reset_index = -1;
};

struct ValidationReport {
    std::vector<ValidationIssue> issues;

    bool ok() const;
    std::size_t error_count() const;
    std::size_t warning_count() const;
};

ValidationReport validate_world(const World& world);

} // namespace nebbie
