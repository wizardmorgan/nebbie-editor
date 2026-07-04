#include "nebbie/session.hpp"
#include "nebbie/zone_graph.hpp"
#include "nebbie/io.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: nebbie-session-tests <lib-directory>\n";
            return 1;
        }

        nebbie::World world;
        nebbie::LibContext context;
        const auto lib = std::filesystem::path(argv[1]);
        nebbie::load_lib(world, lib, context);

        const auto workspace = nebbie::workspace_dir(lib);
        nebbie::save_snapshot(world, context, workspace, "test");
        if (!std::filesystem::exists(workspace / "myst.wld")) {
            throw std::runtime_error("workspace snapshot missing myst.wld");
        }

        nebbie::World restored;
        nebbie::LibContext restored_context;
        nebbie::restore_snapshot(restored, restored_context, workspace);
        if (restored.rooms.size() != world.rooms.size()) {
            throw std::runtime_error("restore room count mismatch");
        }

        const std::string version_id = nebbie::create_version(world, context, lib, "unit-test");
        const auto versions = nebbie::list_versions(lib);
        if (versions.empty()) {
            throw std::runtime_error("expected at least one version");
        }

        if (!world.zones.empty()) {
            const int zone_num = world.zones.front().num;
            const auto rooms = nebbie::rooms_in_zone(world, zone_num);
            const auto graph = nebbie::build_zone_graph(world, zone_num);
            if (graph.nodes.size() != rooms.size()) {
                throw std::runtime_error("zone graph node count mismatch");
            }
            const std::string dot = nebbie::zone_graph_to_dot(graph);
            if (dot.find("digraph") == std::string::npos) {
                throw std::runtime_error("invalid dot output");
            }
        }

        std::filesystem::remove_all(nebbie::session_root(lib));
        std::cout << "session/zone_graph tests OK\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAILED: " << ex.what() << '\n';
        return 1;
    }
}
