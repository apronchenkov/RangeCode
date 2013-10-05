#pragma once

#include <algorithm>
#include <cstdint>
#include <istream>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace range_code {

inline void GenerateCdf(const std::vector<uint32_t>& symbols_count, std::vector<uint32_t>* const cdf)
{
    if (symbols_count.empty()) {
        throw std::invalid_argument("range_code: GenerateCdf: symbols_count is empty.");
    }

    cdf->clear();
    uint32_t partial_sum = 0;
    for (uint32_t symbol_count : symbols_count) {
        cdf->push_back(partial_sum);
        partial_sum += symbol_count;
        // assert (no overflow)
    }
    if (partial_sum >= 0x1000000) {
        throw std::invalid_argument("range_code: GenerateCdf: total_count must be < 2**24.");
    }

    for (uint32_t& bound : *cdf) {
        bound = (static_cast<uint64_t>(bound) << 32) / partial_sum;
    }
    cdf->push_back(0xffffffff);
}

class Encoder {
public:
    explicit Encoder(std::streambuf* ostreambuf);
    ~Encoder();

    void Put(size_t symbol, const std::vector<uint32_t>& cdf);

private:
    void PropagateCarry();
    void Renormalize();
    void PushByte(uint8_t byte);
    void WriteByte(uint8_t byte);
    void WriteByten(uint8_t byte, size_t count);

    uint64_t range_size_;
    uint32_t range_begin_;

    uint8_t carry_acceptor_;
    size_t carry_count_;

    std::streambuf* ostreambuf_;
};


inline Encoder::Encoder(std::streambuf* ostreambuf)
  : range_size_(0x100000000ULL)
  , range_begin_(0)
  , carry_acceptor_(0xff)
  , carry_count_(0)
  , ostreambuf_(ostreambuf)
{ }

inline Encoder::~Encoder()
{
    if (carry_count_ > 0) {
        WriteByte(carry_acceptor_);
        WriteByten(0xff, carry_count_ - 1);
    }

    // TODO (apronchenkov): It seams that the following could be done much efficiently!
    WriteByte(0xff & (range_begin_ >> 24));
    WriteByte(0xff & (range_begin_ >> 16));
    WriteByte(0xff & (range_begin_ >> 8));
    WriteByte(0xff & (range_begin_ >> 0));
}

inline void Encoder::Put(size_t symbol, const std::vector<uint32_t>& cdf)
{
    const uint64_t subrange_begin = (range_size_ * cdf.at(symbol)) >> 32;
    const uint64_t subrange_end = (range_size_ * cdf.at(symbol + 1)) >> 32;
    range_size_ = subrange_end - subrange_begin;

    const uint32_t old_range_begin = range_begin_;
    range_begin_ += subrange_begin;
    if (range_begin_ < old_range_begin) {
        PropagateCarry();
    }
    Renormalize();
}

inline void Encoder::PropagateCarry()
{
    WriteByte(carry_acceptor_ + 1);
    if (carry_count_ > 1) {
        WriteByten(0x00, carry_count_ - 2);
        carry_acceptor_ = 0x00;
        carry_count_ = 1;
    } else {
        carry_acceptor_ = 0xff;
        carry_count_ = 0;
    }
}

inline void Encoder::Renormalize()
{
    while (range_size_ <= 0x1000000) {
        PushByte(range_begin_ >> 24);
        range_begin_ <<= 8;
        range_size_ <<= 8;
    }
}

inline void Encoder::PushByte(uint8_t byte)
{
    if (byte == 0xff) {
        ++carry_count_;

    } else {
        if (carry_count_ > 0) {
            WriteByte(carry_acceptor_);
            WriteByten(0xff, carry_count_ - 1);
        }
        carry_acceptor_ = byte;
        carry_count_ = 1;
    }
}

inline void Encoder::WriteByte(uint8_t byte)
{
    if (EOF == ostreambuf_->sputc(byte)) {
        throw std::ios_base::failure("range_code: Encoder.WriteByte: fail");
    }
}

inline void Encoder::WriteByten(uint8_t byte, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        WriteByte(byte);
    }
}

class Decoder {
public:
    explicit Decoder(std::streambuf* istreambuf);

    size_t Get(const std::vector<uint32_t>& cdf);

private:
    void Renormalize();
    uint8_t ReadByte();

    uint64_t range_size_;
    uint32_t range_begin_;
    uint32_t value_;

    std::streambuf* istreambuf_;
};


template <class Iterator, class Predicate>
Iterator FindIf(Iterator first, Iterator last, Predicate pred)
{
    auto count = std::distance(first,last);
    while (count > 0) {
        const auto step = count / 2;
        const auto it = std::next(first, step);
        if (!pred(*it)) {
            first = std::next(it);
            count -= step + 1;
        } else {
            count=step;
        }
    }
    return first;
}


inline Decoder::Decoder(std::streambuf* istreambuf)
  : range_size_(1)
  , range_begin_(0)
  , value_(0)
  , istreambuf_(istreambuf)
{ }

inline size_t Decoder::Get(const std::vector<uint32_t>& cdf)
{
    if (cdf.size() < 2 || cdf.front() != 0 || cdf.back() != 0xffffffff) {
        throw std::invalid_argument("range_code: Decoder.Get: Bad cdf.");
    }
    Renormalize();

    const uint64_t range_size = range_size_;
    const uint32_t pivot = value_ - range_begin_;

    const auto it = FindIf(cdf.begin() + 1, cdf.end(),
        [pivot, range_size] (uint32_t bound) {
            return pivot < (bound * range_size) >> 32;
        });

    if (it == cdf.end()) {
        throw std::ios_base::failure("range_code: Decoder.Get: Inconsistent input stream.");
    }

    const uint64_t subrange_begin = (range_size * (*std::prev(it))) >> 32;
    const uint64_t subrange_end = (range_size * (*it)) >> 32;

    range_size_ = subrange_end - subrange_begin;
    range_begin_ += subrange_begin;
    return std::distance(cdf.begin(), std::prev(it));
}

inline void Decoder::Renormalize()
{
    while (range_size_ <= 0x01000000) {
        value_ = (value_ << 8) | ReadByte();
        range_begin_ <<= 8;
        range_size_ <<= 8;
    }
}

inline uint8_t Decoder::ReadByte()
{
    const int byte = istreambuf_->sbumpc();
    if (byte == EOF) {
        throw std::ios_base::failure("range_code: Decoder.ReadByte: fail");
    }
    return byte;
}

} // namespace range_code
