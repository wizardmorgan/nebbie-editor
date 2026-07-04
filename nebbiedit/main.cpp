#include "nebbie/io.hpp"
#include "nebbie/validate.hpp"
#include "nebbie/world.hpp"

#include "cli_parse.hpp"
#include "shell.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <cctype>

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
        << "  nebbiedit mob list\n"
        << "  nebbiedit mob show <vnum>\n"
        << "  nebbiedit obj list\n"
        << "  nebbiedit obj show <vnum>\n"
        << "  nebbiedit shop list\n"
        << "  nebbiedit shop show <vnum>\n"
        << "  nebbiedit spe list\n"
        << "  nebbiedit spe show <vnum>\n"
        << "  nebbiedit dam list\n"
        << "  nebbiedit dam show <attack-type>\n"
        << "  nebbiedit social list\n"
        << "  nebbiedit social show <act-nr>\n"
        << "  nebbiedit pose list\n"
        << "  nebbiedit pose show <level>\n"
        << "  nebbiedit guild list\n"
        << "  nebbiedit guild show <name>\n"
        << "  nebbiedit validate <lib-directory>\n"
        << "  nebbiedit check mob <myst.mob-path>\n"
        << "  nebbiedit check obj <myst.obj-path>\n"
        << "  nebbiedit check wld <myst.wld-path>\n"
        << "  nebbiedit check lib <lib-directory>\n"
        << "  nebbiedit edit <lib-directory>\n"
        << "  nebbiedit room set <lib-directory> <vnum> [--name T] [--desc T] [--sector N]\n"
        << "  nebbiedit mob set <lib-directory> <vnum> [--short T] [--level N] [--alignment N]\n"
        << "  nebbiedit obj set <lib-directory> <vnum> [--short T] [--cost N] [--weight N]\n"
        << "  (append --force to one-shot set commands to save despite validation errors)\n"
        << "  nebbiedit convert zon roundtrip <lib-directory> <output-directory>\n"
        << "  nebbiedit convert lib roundtrip <lib-directory> <output-directory>\n\n"
        << "Reference server: https://github.com/NebbieArcane/Server\n"
        << "Test server fork: https://github.com/wizardmorgan/nebbietest\n";
}

void print_info(const nebbie::World& world) {
    std::cout << "Zones: " << world.zones.size() << '\n';
    std::cout << "Rooms: " << world.rooms.size() << '\n';
    std::cout << "Mobiles: " << world.mobiles.size() << '\n';
    std::cout << "Objects: " << world.objects.size() << '\n';
    std::cout << "Shops: " << world.shops.size() << '\n';
    std::cout << "Special procs: " << world.special_procs.size() << '\n';
    std::cout << "Damage messages: " << world.damage_messages.size() << '\n';
    std::cout << "Socials: " << world.social_messages.size() << '\n';
    std::cout << "Poses: " << world.pose_entries.size() << '\n';
    std::cout << "Guilds: " << world.guilds.size() << '\n';
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

        if (cmd == "mob") {
            if (argc < 3) {
                usage();
                return false;
            }
            const std::string sub = argv[2];
            if (sub == "list") {
                for (const auto& [vnum, mob] : g_world.mobiles) {
                    std::cout << vnum << " " << mob.short_descr << '\n';
                }
                return true;
            }
            if (sub == "show" && argc >= 4) {
                const long vnum = std::stol(argv[3]);
                const nebbie::Mobile* mob = g_world.find_mobile(vnum);
                if (!mob) {
                    std::cerr << "Mobile not loaded: " << vnum << '\n';
                    return false;
                }
                std::cout << "#" << mob->vnum << " " << mob->short_descr << '\n';
                std::cout << "Type: " << mob->mobtype
                          << " Level: " << mob->level
                          << " Alignment: " << mob->alignment << '\n';
                std::cout << "Hit: " << mob->hit_dice
                          << " Dam: " << mob->dam_dice << '\n';
                return true;
            }
        }

        if (cmd == "obj") {
            if (argc < 3) {
                usage();
                return false;
            }
            const std::string sub = argv[2];
            if (sub == "list") {
                for (const auto& [vnum, obj] : g_world.objects) {
                    std::cout << vnum << " " << obj.short_descr << '\n';
                }
                return true;
            }
            if (sub == "show" && argc >= 4) {
                const long vnum = std::stol(argv[3]);
                const nebbie::GameObject* obj = g_world.find_object(vnum);
                if (!obj) {
                    std::cerr << "Object not loaded: " << vnum << '\n';
                    return false;
                }
                std::cout << "#" << obj->vnum << " " << obj->short_descr << '\n';
                std::cout << "Type: " << obj->type_flag
                          << " Weight: " << obj->weight
                          << " Cost: " << obj->cost << '\n';
                std::cout << "Affects: " << obj->affects.size()
                          << " Extras: " << obj->extra_descs.size() << '\n';
                return true;
            }
        }

        if (cmd == "shop") {
            if (argc < 3) {
                usage();
                return false;
            }
            const std::string sub = argv[2];
            if (sub == "list") {
                for (const auto& shop : g_world.shops) {
                    std::cout << shop.vnum << " keeper=" << shop.keeper
                              << " room=" << shop.in_room << '\n';
                }
                return true;
            }
            if (sub == "show" && argc >= 4) {
                const long vnum = std::stol(argv[3]);
                for (const auto& shop : g_world.shops) {
                    if (shop.vnum != vnum) {
                        continue;
                    }
                    std::cout << "#" << shop.vnum << " keeper=" << shop.keeper
                              << " room=" << shop.in_room << '\n';
                    std::cout << "Buy x" << shop.profit_buy
                              << " Sell x" << shop.profit_sell << '\n';
                    std::cout << "Hours: " << shop.open1 << "-" << shop.close1
                              << ", " << shop.open2 << "-" << shop.close2 << '\n';
                    return true;
                }
                std::cerr << "Shop not loaded: " << vnum << '\n';
                return false;
            }
        }

        if (cmd == "spe") {
            if (argc < 3) {
                usage();
                return false;
            }
            const std::string sub = argv[2];
            if (sub == "list") {
                for (const auto& spe : g_world.special_procs) {
                    std::cout << static_cast<char>(std::toupper(spe.type))
                              << ' ' << spe.vnum << ' ' << spe.procedure;
                    if (!spe.params.empty()) {
                        std::cout << ' ' << spe.params;
                    }
                    std::cout << '\n';
                }
                return true;
            }
            if (sub == "show" && argc >= 4) {
                const long vnum = std::stol(argv[3]);
                bool found = false;
                for (const auto& spe : g_world.special_procs) {
                    if (spe.vnum != vnum) {
                        continue;
                    }
                    found = true;
                    std::cout << static_cast<char>(std::toupper(spe.type))
                              << ' ' << spe.vnum << ' ' << spe.procedure << '\n';
                    if (!spe.params.empty()) {
                        std::cout << "Params: " << spe.params << '\n';
                    }
                }
                if (!found) {
                    std::cerr << "No special proc for vnum: " << vnum << '\n';
                    return false;
                }
                return true;
            }
        }

        if (cmd == "dam") {
            if (argc < 3) {
                usage();
                return false;
            }
            const std::string sub = argv[2];
            if (sub == "list") {
                for (const auto& msg : g_world.damage_messages) {
                    std::cout << msg.attack_type << " hit="
                              << msg.hit_attacker.substr(0, 40) << "...\n";
                }
                return true;
            }
            if (sub == "show" && argc >= 4) {
                const int type = std::stoi(argv[3]);
                for (const auto& msg : g_world.damage_messages) {
                    if (msg.attack_type != type) {
                        continue;
                    }
                    std::cout << "Attack type " << msg.attack_type << '\n';
                    std::cout << "Die: " << msg.die_attacker << '\n';
                    std::cout << "Hit: " << msg.hit_attacker << '\n';
                    return true;
                }
                std::cerr << "Damage message not loaded: " << type << '\n';
                return false;
            }
        }

        if (cmd == "social") {
            if (argc < 3) {
                usage();
                return false;
            }
            const std::string sub = argv[2];
            if (sub == "list") {
                for (const auto& msg : g_world.social_messages) {
                    std::cout << msg.act_nr << " hide=" << msg.hide
                              << " " << msg.char_no_arg.substr(0, 40) << '\n';
                }
                return true;
            }
            if (sub == "show" && argc >= 4) {
                const int act_nr = std::stoi(argv[3]);
                for (const auto& msg : g_world.social_messages) {
                    if (msg.act_nr != act_nr) {
                        continue;
                    }
                    std::cout << "#" << msg.act_nr
                              << " hide=" << msg.hide
                              << " min_pos=" << msg.min_victim_position << '\n';
                    std::cout << "No arg: " << msg.char_no_arg << '\n';
                    if (!msg.char_found.empty()) {
                        std::cout << "Found: " << msg.char_found << '\n';
                    }
                    return true;
                }
                std::cerr << "Social not loaded: " << act_nr << '\n';
                return false;
            }
        }

        if (cmd == "pose") {
            if (argc < 3) {
                usage();
                return false;
            }
            const std::string sub = argv[2];
            if (sub == "list") {
                for (const auto& entry : g_world.pose_entries) {
                    std::cout << entry.level << " "
                              << entry.poser_msg[0].substr(0, 40) << '\n';
                }
                return true;
            }
            if (sub == "show" && argc >= 4) {
                const int level = std::stoi(argv[3]);
                for (const auto& entry : g_world.pose_entries) {
                    if (entry.level != level) {
                        continue;
                    }
                    std::cout << "Level " << entry.level << '\n';
                    for (int i = 0; i < 4; ++i) {
                        std::cout << "Class " << i << ": " << entry.poser_msg[i] << '\n';
                    }
                    return true;
                }
                std::cerr << "Pose not loaded: level " << level << '\n';
                return false;
            }
        }

        if (cmd == "guild") {
            if (argc < 3) {
                usage();
                return false;
            }
            const std::string sub = argv[2];
            if (sub == "list") {
                for (const auto& guild : g_world.guilds) {
                    std::cout << guild.base_filename
                              << " guard=" << guild.guard_mob
                              << " bank=" << guild.banker_mob << '\n';
                }
                return true;
            }
            if (sub == "show" && argc >= 4) {
                const std::string name = argv[3];
                for (const auto& guild : g_world.guilds) {
                    if (guild.base_filename != name) {
                        continue;
                    }
                    std::cout << guild.base_filename << '\n';
                    std::cout << "Guard: mob=" << guild.guard_mob
                              << " room=" << guild.guard_room
                              << " dir=" << guild.guard_dir << '\n';
                    std::cout << "Bank: mob=" << guild.banker_mob
                              << " room=" << guild.bank_room << '\n';
                    std::cout << "XP bank: mob=" << guild.banker_xp_mob
                              << " room=" << guild.bank_xp_room << '\n';
                    std::cout << "Member book: " << guild.member_book_obj << '\n';
                    return true;
                }
                std::cerr << "Guild not loaded: " << name << '\n';
                return false;
            }
        }

        if (cmd == "check") {
            if (argc < 4) {
                usage();
                return false;
            }
            const std::string kind = argv[2];
            if (kind == "mob") {
                try {
                    nebbie::World world;
                    nebbie::load_myst_mob(world, argv[3]);
                    std::cout << "OK: " << world.mobiles.size() << " mobiles in " << argv[3] << '\n';
                    return true;
                } catch (const std::exception& ex) {
                    std::cerr << "FAILED: " << ex.what() << '\n';
                    return false;
                }
            }
            if (kind == "obj") {
                try {
                    nebbie::World world;
                    nebbie::load_myst_obj(world, argv[3]);
                    std::cout << "OK: " << world.objects.size() << " objects in " << argv[3] << '\n';
                    return true;
                } catch (const std::exception& ex) {
                    std::cerr << "FAILED: " << ex.what() << '\n';
                    return false;
                }
            }
            if (kind == "wld") {
                try {
                    nebbie::World world;
                    nebbie::load_myst_wld(world, argv[3]);
                    std::cout << "OK: " << world.rooms.size() << " rooms in " << argv[3] << '\n';
                    if (const nebbie::Room* room0 = world.find_room(0)) {
                        std::cout << "  room #0: " << room0->name << '\n';
                    }
                    return true;
                } catch (const std::exception& ex) {
                    std::cerr << "FAILED: " << ex.what() << '\n';
                    return false;
                }
            }
            if (kind == "lib") {
                try {
                    nebbie::World world;
                    nebbie::LibContext context;
                    nebbie::load_lib(world, argv[3], context, [](const std::string& msg) {
                        std::cout << msg << '\n';
                    });
                std::cout << "OK: lib loaded from " << argv[3] << '\n';
                std::cout << "  zones=" << world.zones.size()
                          << " rooms=" << world.rooms.size()
                          << " mobiles=" << world.mobiles.size()
                          << " objects=" << world.objects.size() << '\n';
                constexpr const char* kFiles[] = {
                    "myst.zon", "myst.wld", "myst.mob", "myst.obj", "myst.shp", "myst.spe",
                    "myst.dam", "myst.act", "myst.pos", "myst.gui",
                };
                const std::filesystem::path root = argv[3];
                for (const char* file : kFiles) {
                    std::cout << "  " << file << ": "
                              << (std::filesystem::exists(root / file) ? "yes" : "no") << '\n';
                }
                return true;
                } catch (const std::exception& ex) {
                    std::cerr << "FAILED: " << ex.what() << '\n';
                    return false;
                }
            }
            usage();
            return false;
        }

        if (cmd == "validate") {
            if (argc < 3) {
                usage();
                return false;
            }
            nebbie::World world;
            nebbie::load_lib(world, argv[2], [](const std::string& msg) {
                std::cout << msg << '\n';
            });
            const nebbie::ValidationReport report = nebbie::validate_world(world);
            for (const auto& issue : report.issues) {
                const char* level = issue.severity == nebbie::ValidationSeverity::error
                                        ? "ERROR"
                                        : "WARN";
                std::cout << level << " [" << issue.category << "] " << issue.message << '\n';
            }
            std::cout << report.error_count() << " error(s), "
                      << report.warning_count() << " warning(s)\n";
            return report.ok();
        }

        if (cmd == "edit") {
            if (argc < 3) {
                usage();
                return false;
            }
            return nebbiedit::run_shell(argv[2]) == 0;
        }

        if (cmd == "room" && argc >= 5 && std::string(argv[2]) == "set") {
            const std::vector<std::string> args(argv + 1, argv + argc);
            const auto flags = nebbiedit::parse_flags(args, 4);
            const bool force = flags.count("force") > 0;
            const long vnum = std::stol(argv[4]);
            return nebbiedit::run_room_set(argv[3], vnum, flags, force);
        }

        if (cmd == "mob" && argc >= 5 && std::string(argv[2]) == "set") {
            const std::vector<std::string> args(argv + 1, argv + argc);
            const auto flags = nebbiedit::parse_flags(args, 4);
            const bool force = flags.count("force") > 0;
            const long vnum = std::stol(argv[4]);
            return nebbiedit::run_mob_set(argv[3], vnum, flags, force);
        }

        if (cmd == "obj" && argc >= 5 && std::string(argv[2]) == "set") {
            const std::vector<std::string> args(argv + 1, argv + argc);
            const auto flags = nebbiedit::parse_flags(args, 4);
            const bool force = flags.count("force") > 0;
            const long vnum = std::stol(argv[4]);
            return nebbiedit::run_obj_set(argv[3], vnum, flags, force);
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

        if (cmd == "convert" && argc >= 5 && std::string(argv[2]) == "lib") {
            const std::string mode = argv[3];
            if (mode == "roundtrip") {
                nebbie::World original;
                nebbie::LibContext context;
                nebbie::load_lib(original, argv[4], context);
                const std::filesystem::path out = argv[5];
                std::filesystem::create_directories(out);
                nebbie::LibContext out_ctx = context;
                out_ctx.root = out;
                nebbie::save_lib(original, out_ctx);

                nebbie::World roundtrip;
                nebbie::load_lib(roundtrip, out);
                if (roundtrip.zones.size() != original.zones.size()
                    || roundtrip.rooms.size() != original.rooms.size()
                    || roundtrip.mobiles.size() != original.mobiles.size()
                    || roundtrip.objects.size() != original.objects.size()
                    || roundtrip.shops.size() != original.shops.size()
                    || roundtrip.special_procs.size() != original.special_procs.size()
                    || roundtrip.damage_messages.size() != original.damage_messages.size()
                    || roundtrip.social_messages.size() != original.social_messages.size()
                    || roundtrip.pose_entries.size() != original.pose_entries.size()
                    || roundtrip.guilds.size() != original.guilds.size()) {
                    throw std::runtime_error("count mismatch after lib roundtrip");
                }
                std::cout << "Lib round-trip OK in " << out << '\n';
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
