#include "coder.h"
#include "adaptive_model.h"

#include <iostream>

int main()
{
    std::ios_base::sync_with_stdio(false);

    AdaptiveModel model(256);
    range_code::Encoder coder(std::cout.rdbuf());
    std::streambuf* istreambuf = std::cin.rdbuf();
    for (int byte = istreambuf->sgetc(); byte != EOF; byte = istreambuf->snextc()) {
        coder.Put(byte, model.cdf());
        model.Update(byte);
    }

    return 0;
}
