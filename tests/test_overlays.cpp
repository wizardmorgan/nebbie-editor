#include "nebbie/io.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {

void expect_true(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: nebbie-overlay-tests <lib-directory>\n";
            return 1;
        }

        nebbie::World world;
        nebbie::LibContext context;
        nebbie::load_lib(world, argv[1], context);

        expect_true(context.has_overlays(), "expected overlay metadata after load");
        expect_true(!context.room_overlay_vnums.empty(), "expected room overlays");
        expect_true(!context.object_overlay_vnums.empty(), "expected object overlays");

        const nebbie::Room* room = world.find_room(3001);
        expect_true(room != nullptr, "expected room #3001");
        expect_true(room->name == "Stanza overlay", "room overlay name mismatch");

        const nebbie::GameObject* obj = world.find_object(1);
        expect_true(obj != nullptr, "expected object #1");
        expect_true(obj->short_descr == "Elmo overlay", "object overlay short description mismatch");

        std::cout << "overlay tests ok\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "overlay tests failed: " << ex.what() << '\n';
        return 1;
    }
}
