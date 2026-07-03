#include "nebbie/io.hpp"
#include "nebbie/world.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace {

nebbie::World g_world;

void usage() {
    std::cout
        << "Nebbie Arcane World Editor (CLI)\n\n"
        << "Usage:\n"
        << "  nebbiedit info <lib-directory>\n"
        << "  nebbiedit load <lib-directory>\n"
        << "  nebbiedit zone list\n"
        << "  nebbiedit zone show <zone-number>\n"
        << "  nebbiedit room show <vnum>\n"
        << "  nebbiedit convert zon roundtrip <lib-directory> <output-directory>\n\n"
        << "Reference server: https://github.com/NebbieArcane/Server\n"
        << "Test server fork: https://github.com/wizardmorgan/nebbietest\n";
}

void print_info(const nebbie::World& world) {
    std::cout << "Zones: " << world.zones.size() << '\n';
    std::cout << "Rooms: " << world.rooms.size() << '\n';
}

bool run(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return false;
    }

    const std::string cmd = argv[1];

    try {
        if (cmd == "info" || cmd == "load") {
            if (argc < 3) {
                usage();
                return false;
            }
            nebbie::load_lib(g_world, argv[2], [](const std::string& msg) {
                std::cout << msg << '\n';
            });
            print_info(g_world);
            return true;
        }

        if (cmd == "zone") {
            if (argc < 3) {
                usage();
                return false;
            }
            const std::string sub = argv[2];
            if (sub == "list") {
                for (const auto& zone : g_world.zones) {
                    std::cout << zone.num << " " << zone.name
                              << " [" << zone.bottom << "-" << zone.top << "]"
                              << " resets=" << zone.commands.size() << '\n';
                }
                return true;
            }
            if (sub == "show" && argc >= 4) {
                const int num = std::stoi(argv[3]);
                for (const auto& zone : g_world.zones) {
                    if (zone.num != num) {
                        continue;
                    }
                    std::cout << "#" << zone.num << " " << zone.name << '\n';
                    std::cout << "Range: " << zone.bottom << "-" << zone.top << '\n';
                    std::cout << "Lifespan: " << zone.lifespan
                              << " Reset mode: " << zone.reset_mode << '\n';
                    for (const auto& reset : zone.commands) {
                        if (reset.command == '*') {
                            std::cout << "*" << reset.raw_line;
                            continue;
                        }
                        std::cout << reset.command << ' '
                                  << reset.if_flag << ' '
                                  << reset.arg1 << ' '
                                  << reset.arg2 << ' '
                                  << reset.arg3 << ' '
                                  << reset.arg4 << '\n';
                    }
                    return true;
                }
                std::cerr << "Zone not loaded: " << num << '\n';
                return false;
            }
        }

        if (cmd == "room" && argc >= 4 && std::string(argv[2]) == "show") {
            const long vnum = std::stol(argv[3]);
            const nebbie::Room* room = g_world.find_room(vnum);
            if (!room) {
                std::cerr << "Room not loaded: " << vnum << '\n';
                return false;
            }
            std::cout << "#" << room->vnum << " " << room->name << '\n';
            std::cout << room->description << '\n';
            std::cout << "Flags: " << room->room_flags
                      << " Sector: " << room->sector_type << '\n';
            std::cout << "Exits: " << room->exits.size() << '\n';
            return true;
        }

        if (cmd == "convert" && argc >= 5 && std::string(argv[2]) == "zon") {
            const std::string mode = argv[3];
            if (mode == "roundtrip") {
                nebbie::World original;
                nebbie::load_lib(original, argv[4]);
                const std::filesystem::path out = argv[5];
                std::filesystem::create_directories(out);
                nebbie::save_myst_zon(original, out / "myst.zon");
                nebbie::save_myst_wld(original, out / "myst.wld");

                nebbie::World roundtrip;
                nebbie::load_lib(roundtrip, out);
                if (roundtrip.zones.size() != original.zones.size()) {
                    throw std::runtime_error("zone count mismatch after roundtrip");
                }
                if (roundtrip.rooms.size() != original.rooms.size()) {
                    throw std::runtime_error("room count mismatch after roundtrip");
                }
                std::cout << "Round-trip OK in " << out << '\n';
                return true;
            }
        }

        usage();
        return false;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return false;
    }
}

} // namespace

int main(int argc, char** argv) {
    return run(argc, argv) ? 0 : 1;
}
