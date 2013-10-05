#include "coder.h"
#include "quality_model.h"

#include <array>
#include <iostream>

int main()
{
    std::ios_base::sync_with_stdio(false);

    const std::string alphabet = "\n !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\0";
    QualityModel model(alphabet.size());
    std::array<unsigned char, 256> alphabet_map;
    {
        for (unsigned char& c : alphabet_map) { c = 0x00; }
        unsigned char index = 0;
        for (char ch : alphabet) {
            alphabet_map[static_cast<unsigned char>(ch)] = index++;
        }
    }

    range_code::Decoder coder(std::cin.rdbuf());
    std::streambuf* ostringstream = std::cout.rdbuf();

    size_t symbol = coder.Get(model.cdf());
    while (symbol != alphabet.size() - 1) {
        ostringstream->sputc(alphabet.at(symbol));
        model.Update(symbol);
        symbol = coder.Get(model.cdf());
    }
    return 0;
}
