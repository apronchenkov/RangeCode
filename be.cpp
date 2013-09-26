#include "coder.h"
#include "base_model.h"

#include <iostream>

int encodeBase(int base)
{
    switch (base) {
    case 'N': return 1;
    case 'A': return 1;
    case 'T': return 2;
    case 'C': return 3;
    case 'G': return 4;
    default: return 0;
    }
}

int readByte(std::streambuf* const istreambuf)
{
    int c = istreambuf->sbumpc();
    int b0 = encodeBase(c);
    while (b0 == 0 && c != EOF) {
        c = istreambuf->sbumpc();
        b0 = encodeBase(c);
    }

    if (c == EOF) {
        return -1;
    }

    c = istreambuf->sbumpc();
    int b1 = 4*encodeBase(c);
    while (b1 == 0 && c != EOF) {
        c = istreambuf->sbumpc();
        b1 = 4*encodeBase(c);
    }

    if (c == EOF) {
        return b0;
    }

    c = istreambuf->sbumpc();
    int b2 = 16*encodeBase(c);
    while (b2 == 0 && c != EOF) {
        c = istreambuf->sbumpc();
        b2 = 16*encodeBase(c);
    }

    if (c == EOF) {
        return b0 + b1;
    }

    c = istreambuf->sgetc();
    int b3 = 64*encodeBase(c);
    while (b3 == 0 && c != EOF) {
        c = istreambuf->snextc();
        b3 = 64*encodeBase(c);
    }

    if (b0 + b1 + b2 + b3 >= 256) {
        return b0 + b1 + b2;
    }

    istreambuf->sbumpc();
    return b0 + b1 + b2 + b3;
}


int main()
{
    std::ios_base::sync_with_stdio(false);
    std::streambuf* const istreambuf = std::cin.rdbuf();
    std::streambuf* const ostreambuf = std::cout.rdbuf();

    BaseModel model;
    range_code::Encoder coder(ostreambuf);

    int byte = readByte(istreambuf);
    while (byte >= 0) {
        //ostreambuf->sputc(byte);
        coder.Put(byte, model.cdf());
        model.Update(byte);
        byte = readByte(istreambuf);
    }

    return 0;
}
