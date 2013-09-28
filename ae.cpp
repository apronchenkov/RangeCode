#include "coder.h"
#include "adaptive_model.h"

#include <iostream>

int main()
{
    std::ios_base::sync_with_stdio(false);

    AdaptiveModel model(257);
    range_code::Encoder coder(std::cout.rdbuf());
    std::streambuf* istreambuf = std::cin.rdbuf();
    for (int byte = istreambuf->sgetc(); byte != EOF; byte = istreambuf->snextc()) {
        coder.Put(byte, model.cdf());
        model.Update<24>(byte);
    }
    coder.Put(256, model.cdf());
    return 0;
}
