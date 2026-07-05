#include "nebbie/io.hpp"

#include "nebbie/fread.hpp"
#include "nebbie/file_io.hpp"

#include <cstdio>
#include <string>

namespace nebbie {

namespace {

void fwrite_string(FILE* fp, const std::string& value) {
    std::fprintf(fp, "%s~\n", value.c_str());
}

long parse_shop_vnum(const std::string& header) {
    if (header.empty() || header[0] != '#') {
        throw ParseError("Invalid shop header: " + header);
    }
    return std::stol(header.substr(1));
}

float read_float_line(FILE* fp) {
    while (true) {
        const auto line = fread_line(fp);
        if (line.empty()) {
            if (std::feof(fp)) {
                throw ParseError("Unexpected end of file in myst.shp float");
            }
            continue;
        }
        float value = 0.0f;
        if (std::sscanf(line.c_str(), "%f", &value) == 1) {
            return value;
        }
        throw ParseError("Invalid float line in myst.shp: [" + line + "]");
    }
}

void read_shop_entry(FILE* fp, Shop& shop) {
    for (int i = 0; i < MAX_SHOP_PROD; ++i) {
        shop.producing[i] = static_cast<int>(fread_number(fp));
    }

    shop.profit_buy = read_float_line(fp);
    shop.profit_sell = read_float_line(fp);

    for (int i = 0; i < MAX_SHOP_TRADE; ++i) {
        shop.trade_types[i] = static_cast<int>(fread_number(fp));
    }

    shop.no_such_item1 = fread_string(fp);
    shop.no_such_item2 = fread_string(fp);
    shop.do_not_buy = fread_string(fp);
    shop.missing_cash1 = fread_string(fp);
    shop.missing_cash2 = fread_string(fp);
    shop.message_buy = fread_string(fp);
    shop.message_sell = fread_string(fp);

    shop.temper1 = static_cast<int>(fread_number(fp));
    shop.temper2 = static_cast<int>(fread_number(fp));
    shop.keeper = static_cast<int>(fread_number(fp));
    shop.with_who = static_cast<int>(fread_number(fp));
    shop.in_room = static_cast<int>(fread_number(fp));
    shop.open1 = static_cast<int>(fread_number(fp));
    shop.close1 = static_cast<int>(fread_number(fp));
    shop.open2 = static_cast<int>(fread_number(fp));
    shop.close2 = static_cast<int>(fread_number(fp));
}

void write_shop_entry(FILE* fp, const Shop& shop) {
    std::fprintf(fp, "#%ld~\n", shop.vnum);

    for (int i = 0; i < MAX_SHOP_PROD; ++i) {
        std::fprintf(fp, "%d\n", shop.producing[i]);
    }

    std::fprintf(fp, "%g\n%g\n", shop.profit_buy, shop.profit_sell);

    for (int i = 0; i < MAX_SHOP_TRADE; ++i) {
        std::fprintf(fp, "%d\n", shop.trade_types[i]);
    }

    fwrite_string(fp, shop.no_such_item1);
    fwrite_string(fp, shop.no_such_item2);
    fwrite_string(fp, shop.do_not_buy);
    fwrite_string(fp, shop.missing_cash1);
    fwrite_string(fp, shop.missing_cash2);
    fwrite_string(fp, shop.message_buy);
    fwrite_string(fp, shop.message_sell);

    std::fprintf(fp, "%d\n", shop.temper1);
    std::fprintf(fp, "%d\n", shop.temper2);
    std::fprintf(fp, "%d\n", shop.keeper);
    std::fprintf(fp, "%d\n", shop.with_who);
    std::fprintf(fp, "%d\n", shop.in_room);
    std::fprintf(fp, "%d\n", shop.open1);
    std::fprintf(fp, "%d\n", shop.close1);
    std::fprintf(fp, "%d\n", shop.open2);
    std::fprintf(fp, "%d\n", shop.close2);
}

} // namespace

void load_myst_shp(World& world, const std::filesystem::path& path, ProgressCallback progress) {
    world.shops.clear();

    FILE* fp = open_file_read(path, "shop file");
    if (progress) {
        progress("Loading " + path.string());
    }

    while (true) {
        const std::string header = fread_string(fp);
        if (header.empty()) {
            continue;
        }
        if (header[0] == '$') {
            break;
        }
        if (header[0] != '#') {
            throw ParseError("Expected shop header starting with #");
        }

        Shop shop;
        shop.vnum = parse_shop_vnum(header);
        read_shop_entry(fp, shop);
        world.shops.push_back(std::move(shop));
    }

    std::fclose(fp);
}

void save_myst_shp(const World& world, const std::filesystem::path& path, ProgressCallback progress) {
    FILE* fp = open_file_write(path, "shop file");
    if (progress) {
        progress("Writing " + path.string());
    }

    for (const auto& shop : world.shops) {
        write_shop_entry(fp, shop);
    }

    std::fprintf(fp, "$~\n");
    std::fclose(fp);
}

} // namespace nebbie
