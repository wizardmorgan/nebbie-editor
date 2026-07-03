#pragma once

#include "world.hpp"

#include <string>
#include <vector>

namespace nebbie {

enum class ValidationSeverity {
    warning,
    error,
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::error;
    std::string category;
    std::string message;
};

struct ValidationReport {
    std::vector<ValidationIssue> issues;

    bool ok() const;
    std::size_t error_count() const;
    std::size_t warning_count() const;
};

ValidationReport validate_world(const World& world);

} // namespace nebbie
