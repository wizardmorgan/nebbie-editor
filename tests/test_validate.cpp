#include "nebbie/io.hpp"
#include "nebbie/validate.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: nebbie-validate-tests <lib-directory>\n";
            return 1;
        }

        nebbie::World world;
        nebbie::load_lib(world, argv[1]);
        const nebbie::ValidationReport report = nebbie::validate_world(world);
        for (const auto& issue : report.issues) {
            const char* level = issue.severity == nebbie::ValidationSeverity::error ? "ERROR" : "WARN";
            std::cerr << level << " [" << issue.category << "] " << issue.message << '\n';
        }

        if (!report.ok()) {
            throw std::runtime_error("validation failed with " + std::to_string(report.error_count())
                                     + " error(s)");
        }

        std::cout << "OK (" << report.warning_count() << " warning(s))\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAILED: " << ex.what() << '\n';
        return 1;
    }
}
