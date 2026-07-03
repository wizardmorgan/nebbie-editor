#include "nebbie/edit.hpp"
#include "nebbie/io.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: nebbie-create-tests <source-lib> <work-lib>\n";
            return 1;
        }

        const auto source = std::filesystem::path(argv[1]);
        const auto work = std::filesystem::path(argv[2]);

        std::error_code ec;
        std::filesystem::remove_all(work, ec);
        std::filesystem::create_directories(work);
        for (const auto& entry : std::filesystem::directory_iterator(source)) {
            if (entry.is_regular_file()) {
                std::filesystem::copy_file(entry.path(), work / entry.path().filename(),
                                           std::filesystem::copy_options::overwrite_existing);
            }
        }

        nebbie::World world;
        nebbie::LibContext context;
        nebbie::load_lib(world, work, context);

        if (!nebbie::create_room(world, 3002)) {
            throw std::runtime_error("create_room failed");
        }
        if (!nebbie::create_mob(world, 99)) {
            throw std::runtime_error("create_mob failed");
        }
        if (!nebbie::create_object(world, 99)) {
            throw std::runtime_error("create_object failed");
        }

        nebbie::ExitEdit exit;
        exit.direction = 1;
        exit.to_room = 3002;
        exit.description = "Un passaggio verso est.";
        exit.keyword = "passaggio";
        exit.exit_info = 0;
        exit.key = -1;
        if (!nebbie::set_room_exit(world, 3001, exit)) {
            throw std::runtime_error("set_room_exit failed");
        }

        nebbie::save_lib(world, context);

        nebbie::World reloaded;
        nebbie::load_lib(reloaded, work);
        if (!reloaded.find_room(3002)) {
            throw std::runtime_error("created room not persisted");
        }
        if (!reloaded.find_mobile(99)) {
            throw std::runtime_error("created mob not persisted");
        }
        if (!reloaded.find_object(99)) {
            throw std::runtime_error("created object not persisted");
        }
        const nebbie::Room* room = reloaded.find_room(3001);
        if (!room || !nebbie::find_room_exit(*room, 1)) {
            throw std::runtime_error("exit not persisted");
        }
        if (!nebbie::entity_matches(3001, room->name, "3001")) {
            throw std::runtime_error("entity_matches failed");
        }
        if (nebbie::entity_matches(3001, room->name, "inesistente")) {
            throw std::runtime_error("entity_matches false positive");
        }

        std::cout << "OK\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAILED: " << ex.what() << '\n';
        return 1;
    }
}
