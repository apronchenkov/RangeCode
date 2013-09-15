#include "coder.h"
#include "adaptive_model.h"

#include <iostream>

int main()
{
    std::ios_base::sync_with_stdio(false);

    AdaptiveModel model(256);
    range_code::Decoder coder(std::cin.rdbuf());
    std::streambuf* ostringstream = std::cout.rdbuf();
    try {
        for (;;) {
            const auto symbol = coder.Get(model.cdf());
            ostringstream->sputc(static_cast<char>(symbol));
            model.Update(symbol);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << '\n';
        return -1;
    }
    return 0;
}
