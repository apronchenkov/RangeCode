#include "coder.h"
#include "adaptive_model.h"

#include <iostream>

int main()
{
    std::ios_base::sync_with_stdio(false);

    AdaptiveModel model(257);
    range_code::Decoder coder(std::cin.rdbuf());
    std::streambuf* ostringstream = std::cout.rdbuf();
    size_t symbol = coder.Get(model.cdf());
    while (symbol < 256) {
        ostringstream->sputc(static_cast<char>(symbol));
        std::cout.flush();
        model.Update(symbol);
        symbol = coder.Get(model.cdf());
    }
    return 0;
}
