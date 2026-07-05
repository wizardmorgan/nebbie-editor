#include "nebbie/validate.hpp"

#include <algorithm>

namespace nebbie {

namespace {

constexpr long kNowhere = -1;

void add_issue(ValidationReport& report,
               ValidationSeverity severity,
               const std::string& category,
               const std::string& message,
               ValidationTarget target = ValidationTarget::none,
               long target_vnum = 0,
               int zone_num = 0,
               int reset_index = -1) {
    ValidationIssue issue;
    issue.severity = severity;
    issue.category = category;
    issue.message = message;
    issue.target = target;
    issue.target_vnum = target_vnum;
    issue.zone_num = zone_num;
    issue.reset_index = reset_index;
    report.issues.push_back(std::move(issue));
}

bool has_mobile(const World& world, long vnum) {
    return vnum > 0 && world.mobiles.find(vnum) != world.mobiles.end();
}

bool has_object(const World& world, long vnum) {
    return vnum > 0 && world.objects.find(vnum) != world.objects.end();
}

bool has_room(const World& world, long vnum) {
    return vnum > 0 && world.rooms.find(vnum) != world.rooms.end();
}

bool room_in_zone(long vnum, const Zone& zone) {
    return vnum >= zone.bottom && vnum <= zone.top;
}

void validate_rooms(const World& world, ValidationReport& report) {
    for (const auto& [vnum, room] : world.rooms) {
        if (vnum != room.vnum) {
            add_issue(report,
                      ValidationSeverity::error,
                      "room",
                      "room map key " + std::to_string(vnum)
                          + " differs from entry vnum " + std::to_string(room.vnum),
                      ValidationTarget::room,
                      vnum);
        }

        bool in_any_zone = world.zones.empty();
        for (const auto& zone : world.zones) {
            if (room_in_zone(vnum, zone)) {
                in_any_zone = true;
                break;
            }
        }
        if (!in_any_zone && !world.zones.empty()) {
            add_issue(report,
                      ValidationSeverity::warning,
                      "room",
                      "room " + std::to_string(vnum) + " is outside all zone ranges",
                      ValidationTarget::room,
                      vnum);
        }

        for (std::size_t i = 0; i < room.exits.size(); ++i) {
            const auto& exit = room.exits[i];
            if (exit.to_room <= 0 || exit.to_room == kNowhere) {
                continue;
            }
            if (!has_room(world, exit.to_room)) {
                add_issue(report,
                          ValidationSeverity::error,
                          "room",
                          "room " + std::to_string(vnum) + " exit " + std::to_string(i)
                              + " points to missing room " + std::to_string(exit.to_room),
                          ValidationTarget::room,
                          vnum);
            }
        }
    }
}

void validate_resets(const World& world, ValidationReport& report) {
    if (world.zones.empty()) {
        return;
    }

    for (const auto& zone : world.zones) {
        for (std::size_t i = 0; i < zone.commands.size(); ++i) {
            const auto& cmd = zone.commands[i];
            if (cmd.command == '*' || cmd.command == ';') {
                continue;
            }

            const std::string where = "zone " + std::to_string(zone.num)
                                      + " reset[" + std::to_string(i) + "] ";

            switch (cmd.command) {
            case 'M':
                if (!world.mobiles.empty() && cmd.arg1 > 0 && !has_mobile(world, cmd.arg1)) {
                    add_issue(report,
                              ValidationSeverity::error,
                              "reset",
                              where + "M references missing mobile " + std::to_string(cmd.arg1),
                              ValidationTarget::zone,
                              cmd.arg1,
                              zone.num,
                              static_cast<int>(i));
                }
                if (!world.rooms.empty() && cmd.arg3 > 0 && !has_room(world, cmd.arg3)) {
                    add_issue(report,
                              ValidationSeverity::error,
                              "reset",
                              where + "M references missing room " + std::to_string(cmd.arg3),
                              ValidationTarget::zone,
                              cmd.arg3,
                              zone.num,
                              static_cast<int>(i));
                }
                break;
            case 'C':
                if (!world.mobiles.empty() && cmd.arg1 > 0 && !has_mobile(world, cmd.arg1)) {
                    add_issue(report,
                              ValidationSeverity::error,
                              "reset",
                              where + "C references missing mobile " + std::to_string(cmd.arg1));
                }
                break;
            case 'O':
                if (!world.objects.empty() && cmd.arg1 > 0 && !has_object(world, cmd.arg1)) {
                    add_issue(report,
                              ValidationSeverity::error,
                              "reset",
                              where + "O references missing object " + std::to_string(cmd.arg1));
                }
                if (!world.rooms.empty() && cmd.arg3 > 0 && !has_room(world, cmd.arg3)) {
                    add_issue(report,
                              ValidationSeverity::error,
                              "reset",
                              where + "O references missing room " + std::to_string(cmd.arg3));
                }
                break;
            case 'G':
            case 'E':
                if (!world.objects.empty() && cmd.arg1 > 0 && !has_object(world, cmd.arg1)) {
                    add_issue(report,
                              ValidationSeverity::error,
                              "reset",
                              where + std::string(1, cmd.command) + " references missing object "
                                  + std::to_string(cmd.arg1));
                }
                break;
            case 'P':
                if (!world.objects.empty() && cmd.arg1 > 0 && !has_object(world, cmd.arg1)) {
                    add_issue(report,
                              ValidationSeverity::error,
                              "reset",
                              where + "P references missing object " + std::to_string(cmd.arg1));
                }
                if (!world.objects.empty() && cmd.arg3 > 0 && !has_object(world, cmd.arg3)) {
                    add_issue(report,
                              ValidationSeverity::error,
                              "reset",
                              where + "P references missing container object "
                                  + std::to_string(cmd.arg3));
                }
                break;
            case 'D':
                if (!world.rooms.empty() && cmd.arg1 > 0 && !has_room(world, cmd.arg1)) {
                    add_issue(report,
                              ValidationSeverity::error,
                              "reset",
                              where + "D references missing room " + std::to_string(cmd.arg1));
                }
                break;
            default:
                break;
            }
        }
    }
}

void validate_shops(const World& world, ValidationReport& report) {
    for (const auto& shop : world.shops) {
        const std::string where = "shop " + std::to_string(shop.vnum) + " ";

        if (!world.mobiles.empty() && shop.keeper > 0 && !has_mobile(world, shop.keeper)) {
            add_issue(report,
                      ValidationSeverity::error,
                      "shop",
                      where + "keeper mobile " + std::to_string(shop.keeper) + " not found",
                      ValidationTarget::shop,
                      shop.vnum);
        }
        if (!world.rooms.empty() && shop.in_room > 0 && !has_room(world, shop.in_room)) {
            add_issue(report,
                      ValidationSeverity::error,
                      "shop",
                      where + "room " + std::to_string(shop.in_room) + " not found",
                      ValidationTarget::shop,
                      shop.vnum);
        }
        if (!world.objects.empty()) {
            for (int obj_vnum : shop.producing) {
                if (obj_vnum > 0 && !has_object(world, obj_vnum)) {
                    add_issue(report,
                              ValidationSeverity::error,
                              "shop",
                              where + "produces missing object " + std::to_string(obj_vnum),
                              ValidationTarget::shop,
                              shop.vnum);
                }
            }
        }
    }
}

void validate_guilds(const World& world, ValidationReport& report) {
    for (const auto& guild : world.guilds) {
        const std::string where = "guild " + guild.base_filename + " ";

        if (!world.mobiles.empty()) {
            if (guild.guard_mob > 0 && !has_mobile(world, guild.guard_mob)) {
                add_issue(report,
                          ValidationSeverity::error,
                          "guild",
                          where + "guard mob " + std::to_string(guild.guard_mob) + " not found");
            }
            if (guild.banker_mob > 0 && !has_mobile(world, guild.banker_mob)) {
                add_issue(report,
                          ValidationSeverity::error,
                          "guild",
                          where + "banker mob " + std::to_string(guild.banker_mob) + " not found");
            }
            if (guild.banker_xp_mob > 0 && !has_mobile(world, guild.banker_xp_mob)) {
                add_issue(report,
                          ValidationSeverity::error,
                          "guild",
                          where + "xp banker mob " + std::to_string(guild.banker_xp_mob)
                              + " not found");
            }
        }
        if (!world.objects.empty() && guild.member_book_obj > 0
            && !has_object(world, guild.member_book_obj)) {
            add_issue(report,
                      ValidationSeverity::error,
                      "guild",
                      where + "member book object " + std::to_string(guild.member_book_obj)
                          + " not found");
        }
        if (!world.rooms.empty()) {
            if (guild.guard_room > 0 && !has_room(world, guild.guard_room)) {
                add_issue(report,
                          ValidationSeverity::error,
                          "guild",
                          where + "guard room " + std::to_string(guild.guard_room) + " not found");
            }
            if (guild.bank_room > 0 && !has_room(world, guild.bank_room)) {
                add_issue(report,
                          ValidationSeverity::error,
                          "guild",
                          where + "bank room " + std::to_string(guild.bank_room) + " not found");
            }
            if (guild.bank_xp_room > 0 && !has_room(world, guild.bank_xp_room)) {
                add_issue(report,
                          ValidationSeverity::error,
                          "guild",
                          where + "xp bank room " + std::to_string(guild.bank_xp_room)
                              + " not found");
            }
        }
    }
}

void validate_special_procs(const World& world, ValidationReport& report) {
    for (const auto& spe : world.special_procs) {
        const std::string where = std::string(1, spe.type) + " " + std::to_string(spe.vnum)
                                  + " (" + spe.procedure + ") ";

        switch (spe.type) {
        case 'm':
            if (!world.mobiles.empty() && !has_mobile(world, spe.vnum)) {
                add_issue(report,
                          ValidationSeverity::error,
                          "special",
                          where + "mobile not found");
            }
            break;
        case 'o':
            if (!world.objects.empty() && !has_object(world, spe.vnum)) {
                add_issue(report,
                          ValidationSeverity::error,
                          "special",
                          where + "object not found");
            }
            break;
        case 'r':
            if (!world.rooms.empty() && !has_room(world, spe.vnum)) {
                add_issue(report,
                          ValidationSeverity::error,
                          "special",
                          where + "room not found");
            }
            break;
        default:
            break;
        }
    }
}

void validate_socials(const World& world, ValidationReport& report) {
    for (std::size_t i = 0; i < world.social_messages.size(); ++i) {
        for (std::size_t j = i + 1; j < world.social_messages.size(); ++j) {
            if (world.social_messages[i].act_nr == world.social_messages[j].act_nr) {
                add_issue(report,
                          ValidationSeverity::warning,
                          "social",
                          "duplicate act_nr " + std::to_string(world.social_messages[i].act_nr));
                break;
            }
        }
    }
}

} // namespace

bool ValidationReport::ok() const {
    return error_count() == 0;
}

std::size_t ValidationReport::error_count() const {
    std::size_t count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::error) {
            ++count;
        }
    }
    return count;
}

std::size_t ValidationReport::warning_count() const {
    std::size_t count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::warning) {
            ++count;
        }
    }
    return count;
}

ValidationReport validate_world(const World& world) {
    ValidationReport report;
    validate_rooms(world, report);
    validate_resets(world, report);
    validate_shops(world, report);
    validate_guilds(world, report);
    validate_special_procs(world, report);
    validate_socials(world, report);
    return report;
}

} // namespace nebbie
