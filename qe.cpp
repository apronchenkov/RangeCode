#include "coder.h"
#include "quality_model.h"

#include <iostream>

int main()
{
    std::ios_base::sync_with_stdio(false);

    const std::string alphabet = "\n !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    QualityModel model(alphabet.size());
    std::array<unsigned char, 256> alphabet_map;
    {
        for (unsigned char& c : alphabet_map) { c = 0x00; }
        unsigned char index = 0;
        for (char ch : alphabet) {
            alphabet_map[static_cast<unsigned char>(ch)] = index++;
        }
    }

    range_code::Encoder coder(std::cout.rdbuf());
    std::streambuf* istreambuf = std::cin.rdbuf();
    for (int byte = istreambuf->sgetc(); byte != EOF; byte = istreambuf->snextc()) {
        const auto symbol = alphabet_map.at(byte);
        coder.Put(symbol, model.cdf());
        model.Update(symbol);
    }

    return 0;
}
