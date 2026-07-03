#include "nebbie/edit.hpp"
#include "nebbie/io.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: nebbie-mvp-tests <source-lib> <work-lib>\n";
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

        nebbie::RoomEdit room_edit;
        room_edit.name = "Stanza MVP";
        room_edit.description = "Modificata dal test MVP.";
        if (!nebbie::edit_room(world, 3001, room_edit)) {
            throw std::runtime_error("room 3001 not found");
        }

        nebbie::MobEdit mob_edit;
        mob_edit.short_descr = "Puff MVP";
        mob_edit.level = 30;
        if (!nebbie::edit_mob(world, 1, mob_edit)) {
            throw std::runtime_error("mobile 1 not found");
        }

        nebbie::ObjEdit obj_edit;
        obj_edit.short_descr = "Elmo MVP";
        obj_edit.cost = 42;
        if (!nebbie::edit_object(world, 1, obj_edit)) {
            throw std::runtime_error("object 1 not found");
        }

        nebbie::save_lib(world, context);

        nebbie::World reloaded;
        nebbie::load_lib(reloaded, work);
        const nebbie::Room* room = reloaded.find_room(3001);
        const nebbie::Mobile* mob = reloaded.find_mobile(1);
        const nebbie::GameObject* obj = reloaded.find_object(1);
        if (!room || room->name != "Stanza MVP") {
            throw std::runtime_error("room edit not persisted");
        }
        if (!mob || mob->short_descr != "Puff MVP" || mob->level != 30) {
            throw std::runtime_error("mob edit not persisted");
        }
        if (!obj || obj->short_descr != "Elmo MVP" || obj->cost != 42) {
            throw std::runtime_error("object edit not persisted");
        }

        std::cout << "OK\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAILED: " << ex.what() << '\n';
        return 1;
    }
}
