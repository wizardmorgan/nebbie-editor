#include "nebbie/obj_catalog.hpp"

namespace nebbie {

namespace {

const char* kItemTypes[] = {
    "UNDEFINED", "LIGHT", "SCROLL", "WAND", "STAFF", "WEAPON", "FIRE WEAPON", "MISSILE",
    "TREASURE", "ARMOR", "POTION", "WORN", "OTHER", "TRASH", "TRAP", "CONTAINER", "NOTE",
    "LIQUID CONTAINER", "KEY", "FOOD", "MONEY", "PEN", "BOAT", "AUDIO", "BOARD", "TREE",
    "ROCK", "MINED GEM", "MINED MINERAL", "BAR", "JEWEL",
};

const char* kWearBits[] = {
    "TAKE", "FINGER", "NECK", "BODY", "HEAD", "LEGS", "FEET", "HANDS", "ARMS",
    "SHIELD", "ABOUT", "WAIST", "WRIST", "WIELD", "HOLD", "THROW", "LIGHT-SOURCE",
    "BACK", "EARS", "EYES",
};

const char* kExtraBits[] = {
    "GLOW", "HUM", "METAL", "MINERAL", "ORGANIC", "INVISIBLE", "MAGIC", "NODROP",
    "BLESS", "ANTI-GOOD", "ANTI-EVIL", "ANTI-NEUTRAL", "ANTI-CLERIC", "ANTI-MAGE",
    "ANTI-THIEF", "ANTI-WARRIOR", "BRITTLE", "RESISTANT", "ARTIFACT", "ANTI-MEN",
    "ANTI-WOMEN", "ANTI-SUN", "ANTI-BARBARIAN", "ANTI-RANGER", "ANTI-PALADIN",
    "ANTI-PSIONIST", "ANTI-MONK", "ANTI-DRUID", "ONLY-CLASS", "DIG", "SCYTHE",
    "ANTI-SORCERER",
};

const char* kExtraBits2[] = {
    "QUEST-ITEM", "EDIT", "NO-LOCATE", "PERSONAL", "HAS-GEMS", "NO-PRINCE", "ONLY-PRINCE",
};

const char* kApplyTypes[] = {
    "NONE", "STR", "DEX", "INT", "WIS", "CON", "CHR", "SEX", "LEVEL", "AGE",
    "CHAR_WEIGHT", "CHAR_HEIGHT", "MANA", "HIT", "MOVE", "GOLD", "EXP", "ARMOR",
    "HITROLL", "DAMROLL", "SAVING_PARA", "SAVING_ROD", "SAVING_PETRI", "SAVING_BREATH",
    "SAVING_SPELL", "SAVING_ALL", "RESISTANCE", "SUSCEPTIBILITY", "IMMUNITY",
    "SPELL AFFECT", "WEAPON SPELL", "EAT SPELL", "BACKSTAB", "KICK", "SNEAK", "HIDE",
    "BASH", "PICK", "STEAL", "TRACK", "HIT-N-DAM", "SPELLFAIL", "ATTACKS", "HASTE",
    "SLOW", "SPELL AFFECT 2", "FIND-TRAPS", "RIDE", "RACE-SLAYER", "ALIGN-SLAYER",
    "MANA-REGEN", "HIT-REGEN", "MOVE-REGEN", "MOD-THIRST", "MOD-HUNGER", "MOD-DRUNK",
    "T_STR", "T_INT", "T_DEX", "T_WIS", "T_CON", "T_CHR", "T_HPS", "T_MOVE", "T_MANA",
    "SPELLPOWER", "HIT-N-SP",
};

std::vector<MobFlagDef> make_bit_flags(const char* const* names, std::size_t count) {
    std::vector<MobFlagDef> defs;
    defs.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        MobFlagDef def;
        def.value = 1L << i;
        def.name = names[i];
        def.label = names[i];
        defs.push_back(def);
    }
    return defs;
}

} // namespace

std::vector<std::pair<int, std::string>> obj_type_choices() {
    std::vector<std::pair<int, std::string>> choices;
    const std::size_t count = sizeof(kItemTypes) / sizeof(kItemTypes[0]);
    for (std::size_t i = 0; i < count; ++i) {
        choices.emplace_back(static_cast<int>(i), kItemTypes[i]);
    }
    return choices;
}

std::vector<MobFlagDef> obj_wear_flags() {
    return make_bit_flags(kWearBits, sizeof(kWearBits) / sizeof(kWearBits[0]));
}

std::vector<MobFlagDef> obj_extra_flags() {
    return make_bit_flags(kExtraBits, sizeof(kExtraBits) / sizeof(kExtraBits[0]));
}

std::vector<MobFlagDef> obj_extra_flags2() {
    return make_bit_flags(kExtraBits2, sizeof(kExtraBits2) / sizeof(kExtraBits2[0]));
}

std::vector<std::pair<int, std::string>> obj_apply_type_choices() {
    std::vector<std::pair<int, std::string>> choices;
    const std::size_t count = sizeof(kApplyTypes) / sizeof(kApplyTypes[0]);
    for (std::size_t i = 0; i < count; ++i) {
        choices.emplace_back(static_cast<int>(i), kApplyTypes[i]);
    }
    return choices;
}

std::string obj_type_name(const int type_flag) {
    const std::size_t count = sizeof(kItemTypes) / sizeof(kItemTypes[0]);
    if (type_flag >= 0 && static_cast<std::size_t>(type_flag) < count) {
        return kItemTypes[type_flag];
    }
    return "Type #" + std::to_string(type_flag);
}

std::string obj_apply_type_name(const int location) {
    const std::size_t count = sizeof(kApplyTypes) / sizeof(kApplyTypes[0]);
    if (location >= 0 && static_cast<std::size_t>(location) < count) {
        return kApplyTypes[location];
    }
    return "Apply #" + std::to_string(location);
}

} // namespace nebbie
