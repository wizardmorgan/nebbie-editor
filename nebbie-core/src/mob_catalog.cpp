#include "nebbie/mob_catalog.hpp"

#include <cctype>
#include <sstream>
#include <stdexcept>

namespace nebbie {

namespace {

std::vector<MobFlagDef> make_act_flags() {
    return {
        {1, "ACT_SPEC", "SPEC"},
        {2, "ACT_SENTINEL", "SENTINEL"},
        {4, "ACT_SCAVENGER", "SCAVENGER"},
        {8, "ACT_ISNPC", "ISNPC"},
        {16, "ACT_NICE_THIEF", "NICE-THIEF"},
        {32, "ACT_AGGRESSIVE", "AGGRESSIVE"},
        {64, "ACT_STAY_ZONE", "STAY-ZONE"},
        {128, "ACT_WIMPY", "WIMPY"},
        {256, "ACT_ANNOYING", "ANNOYING"},
        {512, "ACT_HATEFUL", "HATEFUL"},
        {1024, "ACT_AFRAID", "AFRAID"},
        {2048, "ACT_IMMORTAL", "IMMORTAL"},
        {4096, "ACT_HUNTING", "HUNTING"},
        {8192, "ACT_DEADLY", "DEADLY"},
        {16384, "ACT_POLYSELF", "POLYMORPHED"},
        {32768, "ACT_META_AGG", "META_AGGRESSIVE"},
        {65536, "ACT_GUARDIAN", "GUARDING"},
        {131072, "ACT_ILLUSION", "ILLUSION"},
        {262144, "ACT_HUGE", "HUGE"},
        {524288, "ACT_SCRIPT", "SCRIPT"},
        {1048576, "ACT_GREET", "GREET"},
        {2097152, "ACT_MAGIC_USER", "MAGIC-USER"},
        {4194304, "ACT_WARRIOR", "WARRIOR"},
        {8388608, "ACT_CLERIC", "CLERIC"},
        {16777216, "ACT_THIEF", "THIEF"},
        {33554432, "ACT_DRUID", "DRUID"},
        {67108864, "ACT_MONK", "MONK"},
        {134217728, "ACT_BARBARIAN", "BARBARIAN"},
        {268435456, "ACT_PALADIN", "PALADIN"},
        {536870912, "ACT_RANGER", "RANGER"},
        {1073741824, "ACT_PSI", "PSIONIST"},
        {2147483648L, "ACT_ARCHER", "ARCHER"},
    };
}

std::vector<MobFlagDef> make_affected_flags() {
    return {
        {1, "AFF_BLIND", "BLIND"},
        {2, "AFF_INVISIBLE", "INVISIBLE"},
        {4, "AFF_DETECT_EVIL", "DETECT-EVIL"},
        {8, "AFF_DETECT_INVISIBLE", "DETECT-INVISIBLE"},
        {16, "AFF_DETECT_MAGIC", "DETECT-MAGIC"},
        {32, "AFF_SENSE_LIFE", "SENCE-LIFE"},
        {64, "AFF_LIFE_PROT", "LIFE-PROT"},
        {128, "AFF_SANCTUARY", "SANCTUARY"},
        {256, "AFF_DRAGON_RIDE", "DRAGON RIDE"},
        {512, "AFF_GROWTH", "GROWTH"},
        {1024, "AFF_CURSE", "CURSE"},
        {2048, "AFF_FLYING", "FLYING"},
        {4096, "AFF_POISON", "POISON"},
        {8192, "AFF_TREE_TRAVEL", "TREE TRAVEL"},
        {16384, "AFF_PARALYSIS", "PARALYSIS"},
        {32768, "AFF_INFRAVISION", "INFRAVISION"},
        {65536, "AFF_WATERBREATH", "WATER-BREATH"},
        {131072, "AFF_SLEEP", "SLEEP"},
        {262144, "AFF_TRAVELLING", "TRAVELLING"},
        {524288, "AFF_SNEAK", "SNEAK"},
        {1048576, "AFF_HIDE", "HIDE"},
        {2097152, "AFF_SILENCE", "SILENCE"},
        {4194304, "AFF_CHARM", "CHARM"},
        {8388608, "AFF_FOLLOW", "FOLLOW"},
        {16777216, "AFF_PROTECT_FROM_EVIL", "PROTECT-FROM-EVIL"},
        {33554432, "AFF_TRUE_SIGHT", "TRUE SIGHT"},
        {67108864, "AFF_SCRYING", "SCRYING"},
        {134217728, "AFF_FIRESHIELD", "FIRESHIELD"},
        {268435456, "AFF_GROUP", "GROUP"},
        {536870912, "AFF_TELEPATHY", "TELEPATHY"},
        {1073741824, "AFF_GLOBE_DARKNESS", "DARKNESS"},
        {2147483648L, "AFF_UNDEF_AFF_1", "UNDEFINED"},
    };
}

std::vector<MobFlagDef> make_immunity_flags() {
    return {
        {1, "IMM_FIRE", "FIRE"},
        {2, "IMM_COLD", "COLD"},
        {4, "IMM_ELEC", "ELECTRICITY"},
        {8, "IMM_ENERGY", "ENERGY"},
        {16, "IMM_BLUNT", "BLUNT"},
        {32, "IMM_PIERCE", "PIERCE"},
        {64, "IMM_SLASH", "SLASH"},
        {128, "IMM_ACID", "ACID"},
        {256, "IMM_POISON", "POISON"},
        {512, "IMM_DRAIN", "DRAIN"},
        {1024, "IMM_SLEEP", "SLEEP"},
        {2048, "IMM_CHARM", "CHARM"},
        {4096, "IMM_HOLD", "HOLD"},
        {8192, "IMM_NONMAG", "NON-MAGIC"},
        {16384, "IMM_PLUS1", "+1"},
        {32768, "IMM_PLUS2", "+2"},
        {65536, "IMM_PLUS3", "+3"},
        {131072, "IMM_PLUS4", "+4"},
    };
}

const char* kRaceNames[] = {
    "Halfbreed",      "Human",          "Elven",          "Dwarf",          "Halfling",
    "Gnome",          "Reptile",        "Special",        "Lycanth",        "Dragon",
    "Undead",         "Orc",            "Insect",         "Arachnid",       "Dinosaur",
    "Fish",           "Bird",           "Giant",          "Predator",       "Parasite",
    "Slime",          "Demon",          "Snake",          "Herbiv",         "Tree",
    "Veggie",         "Element",        "Planar",         "Devil",          "Ghost",
    "Goblin",         "Troll",          "Vegman",         "Mindflayer",     "Primate",
    "Enfan",          "Dark Elf",       "Golem",          "Skexie",         "Trogman",
    "Patryn",         "Labrat",         "Sartan",         "Tytan",          "Smurf",
    "Roo",            "Horse",          "Draagdim",       "Astral",         "God",
    "Giant Hill",     "Giant Frost",    "Giant Fire",     "Giant Cloud",    "Giant Storm",
    "Giant Stone",    "Dragon Red",     "Dragon Black",   "Dragon Green",   "Dragon White",
    "Dragon Blue",    "Dragon Silver",  "Dragon Gold",    "Dragon Bronze",  "Dragon Copper",
    "Dragon Brass",   "Undead Vampire", "Undead Lich",    "Undead Wight",   "Undead Ghast",
    "Undead Spectre", "Undead Zombie",  "Undead Skeleton","Undead Ghoul",   "Half Elven",
    "Half Ogre",      "Half Orc",       "Half Giant",     "Lizardman",      "Dark Dwarf",
    "Deep Gnome",     "Gnoll",          "Gold Elf",       "Wild Elf",       "Sea Elf",
};

const char* kPositionNames[] = {
    "Dead", "Mortally wounded", "Incapacitated", "Stunned", "Sleeping",
    "Resting", "Sitting", "Fighting", "Standing", "Mounted",
};

} // namespace

std::vector<MobFlagDef> mob_act_flags() {
    return make_act_flags();
}

std::vector<MobFlagDef> mob_affected_flags() {
    return make_affected_flags();
}

std::vector<MobFlagDef> mob_immunity_flags() {
    return make_immunity_flags();
}

std::vector<std::pair<int, std::string>> mob_position_choices() {
    std::vector<std::pair<int, std::string>> choices;
    for (int i = 0; i < 10; ++i) {
        choices.emplace_back(i, kPositionNames[i]);
    }
    return choices;
}

std::vector<std::pair<int, std::string>> mob_sex_choices() {
    return {
        {0, "Neutro"},
        {1, "Maschio"},
        {2, "Femmina"},
    };
}

std::vector<std::pair<int, std::string>> mob_race_choices() {
    std::vector<std::pair<int, std::string>> choices;
    const std::size_t count = sizeof(kRaceNames) / sizeof(kRaceNames[0]);
    for (std::size_t i = 0; i < count; ++i) {
        choices.emplace_back(static_cast<int>(i), kRaceNames[i]);
    }
    return choices;
}

std::vector<std::pair<char, std::string>> mob_type_choices() {
    return {
        {'S', "S — Simple (hit dice)"},
        {'A', "A — Advanced (hit bonus)"},
        {'N', "N — Advanced (no mult_att)"},
        {'B', "B — Advanced + HUGE"},
        {'L', "L — Advanced + suoni"},
    };
}

std::string mob_race_name(const int race) {
    const std::size_t count = sizeof(kRaceNames) / sizeof(kRaceNames[0]);
    if (race >= 0 && static_cast<std::size_t>(race) < count) {
        return kRaceNames[race];
    }
    return "Race #" + std::to_string(race);
}

std::string mob_position_name(const int position) {
    if (position >= 0 && position < 10) {
        return kPositionNames[position];
    }
    return "Position #" + std::to_string(position);
}

std::string mob_sex_name(const int sex) {
    switch (sex) {
    case 1:
        return "Maschio";
    case 2:
        return "Femmina";
    default:
        return "Neutro";
    }
}

std::string mob_type_name(const char mobtype) {
    for (const auto& [value, label] : mob_type_choices()) {
        if (value == mobtype) {
            return label;
        }
    }
    return std::string("Tipo ") + mobtype;
}

bool mob_uses_hit_dice(const char mobtype) {
    return mobtype == 'S';
}

bool mob_type_uses_mult_att(const char mobtype) {
    return mobtype == 'A' || mobtype == 'B' || mobtype == 'L';
}

bool mob_type_uses_sounds(const char mobtype) {
    return mobtype == 'L';
}

DiceValues parse_dice(const std::string& text) {
    DiceValues dice;
    std::size_t pos = 0;
    while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
        ++pos;
    }
    if (pos >= text.size()) {
        return dice;
    }

    auto read_int = [&]() {
        std::size_t start = pos;
        if (text[pos] == '-' || text[pos] == '+') {
            ++pos;
        }
        while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos]))) {
            ++pos;
        }
        if (start == pos) {
            throw std::runtime_error("invalid dice token");
        }
        return std::stoi(text.substr(start, pos - start));
    };

    dice.number = read_int();
    if (pos < text.size() && (text[pos] == 'd' || text[pos] == 'D')) {
        ++pos;
        dice.size = read_int();
    } else {
        dice.size = 1;
    }
    if (pos < text.size() && (text[pos] == '+' || text[pos] == '-')) {
        const int sign = text[pos] == '-' ? -1 : 1;
        ++pos;
        dice.plus = sign * read_int();
    }
    return dice;
}

std::string format_dice(const DiceValues& dice) {
    std::ostringstream oss;
    oss << dice.number << 'd' << dice.size;
    if (dice.plus > 0) {
        oss << '+' << dice.plus;
    } else if (dice.plus < 0) {
        oss << dice.plus;
    }
    return oss.str();
}

long flags_value_from_selection(const std::vector<MobFlagDef>& defs, const std::vector<bool>& selected) {
    long value = 0;
    for (std::size_t i = 0; i < defs.size() && i < selected.size(); ++i) {
        if (selected[i]) {
            value |= defs[i].value;
        }
    }
    return value;
}

std::vector<bool> flags_selection_from_value(const std::vector<MobFlagDef>& defs, const long value) {
    std::vector<bool> selected(defs.size(), false);
    for (std::size_t i = 0; i < defs.size(); ++i) {
        selected[i] = (value & defs[i].value) != 0;
    }
    return selected;
}

} // namespace nebbie
