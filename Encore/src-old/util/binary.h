#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <string>

namespace encore {

    /*
     * Custom byteswap implementation
     *
     * This implementation supports byteswapping floating-point values,
     * in addition to integral values, which simplifies binary stream implementations.
     */

#define HAS_BYTESWAP (__cpp_lib_byteswap >= 202110L)

#if !HAS_BYTESWAP
    namespace impl {
        constexpr inline uint16_t byteswap_16(uint16_t value) noexcept {
            value = (value & 0xFF00) >> 8 | (value & 0x00FF) << 8;
            return value;
        }

        constexpr inline uint32_t byteswap_32(uint32_t value) noexcept {
            value = ((value & 0xFFFF0000) >> 16) | ((value & 0x0000FFFF) << 16);
            value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
            return value;
        }

        constexpr inline uint64_t byteswap_64(uint64_t value) noexcept {
            value = ((value & 0xFFFFFFFF00000000) >> 32)
                | ((value & 0x00000000FFFFFFFF) << 32);
            value = ((value & 0xFFFF0000FFFF0000) >> 16)
                | ((value & 0x0000FFFF0000FFFF) << 16);
            value =
                ((value & 0xFF00FF00FF00FF00) >> 8) | ((value & 0x00FF00FF00FF00FF) << 8);
            return value;
        }
    }
#endif

    /// Custom byteswap which allows floating-point values.
    template <typename T>
        [[nodiscard]]
        constexpr inline T byteswap(const T value) noexcept requires std::integral<T>
        || std::floating_point<T> {
#if HAS_BYTESWAP // Take advantage of std::byteswap if available, for intrinsics
        if constexpr (std::integral<T>) {
            return std::byteswap(value);
        } else if constexpr (sizeof(T) == 4) {
            return std::bit_cast<T>(std::byteswap(std::bit_cast<uint32_t>(value)));
        } else {
            static_assert(sizeof(T) == 8, "Unexpected float size");
            return std::bit_cast<T>(std::byteswap(std::bit_cast<uint64_t>(value)));
        }
#else
        if constexpr (sizeof(T) == 1) {
            return value;
        } else if constexpr (sizeof(T) == 2) {
            return std::bit_cast<T>(impl::byteswap_16(std::bit_cast<uint16_t>(value)));
        } else if constexpr (sizeof(T) == 4) {
            return std::bit_cast<T>(impl::byteswap_32(std::bit_cast<uint32_t>(value)));
        } else {
            static_assert(sizeof(T) == 8, "Unexpected value size");
            return std::bit_cast<T>(impl::byteswap_64(std::bit_cast<uint64_t>(value)));
        }
#endif
    }

#undef HAS_BYTESWAP

    /*
     * Endianness conversion
     *
     * note(Nate): convert_endian, to_endian, and from_endian all techically function
     * identically, but i was having a hard time wrapping my head around that, so i made
     * them distinct for clarity
     */

    // We don't support anything other than little- or big-endian
    static_assert(
        std::endian::native == std::endian::little
        || std::endian::native == std::endian::big
    );

    /// Converts a value with the given source endianness to a value with the target
    /// endianness.
    template <std::endian SourceEndian, std::endian TargetEndian>
    [[nodiscard]]
    constexpr inline auto convert_endian(auto value) noexcept {
        if constexpr (SourceEndian == TargetEndian) {
            return value;
        } else {
            return byteswap(value);
        }
    }

    /// Converts a value of native endianness to a value with the target endianness.
    template <std::endian TargetEndian>
    [[nodiscard]]
    constexpr inline auto to_endian(auto value) noexcept {
        return convert_endian<std::endian::native, TargetEndian>(value);
    }

    /// Converts a value with the given source endianness to a value of native endianness.
    template <std::endian SourceEndian>
    [[nodiscard]]
    constexpr inline auto from_endian(auto value) noexcept {
        return convert_endian<SourceEndian, std::endian::native>(value);
    }

    /*
     * Binary stream wrappers
     *
     * Because istream/ostream were designed for text and not binary data.
     * Thanks, C++.
     */

    template <typename T>
    concept closable_stream = std::derived_from<T, std::ios> && requires(T stream) {
        { stream.close() } -> std::same_as<void>;
    };

    template <std::derived_from<std::ios> Stream>
    class binstream_base {
    protected:
        std::unique_ptr<Stream> mStream;

    public:
        using iostate = typename Stream::iostate;

        template <class... Ts>
        binstream_base(Ts &&..._Val)
            : mStream(std::make_unique<Stream, Ts...>(std::forward<Ts>(_Val)...)) {}

        binstream_base(std::unique_ptr<Stream> &&stream) : mStream(std::move(stream)) {}

        binstream_base(const binstream_base &) = delete;
        binstream_base(binstream_base &&) = default;

        binstream_base &operator=(const binstream_base &) = delete;
        binstream_base &operator=(binstream_base &&) = default;

        [[nodiscard]]
        bool good() const noexcept(noexcept(mStream->good())) {
            return mStream->good();
        }

        [[nodiscard]]
        bool eof() const noexcept(noexcept(mStream->eof())) {
            return mStream->eof();
        }

        [[nodiscard]]
        bool fail() const noexcept(noexcept(mStream->fail())) {
            return mStream->fail();
        }

        [[nodiscard]]
        bool bad() const noexcept(noexcept(mStream->bad())) {
            return mStream->bad();
        }

        explicit operator bool() const noexcept(noexcept(mStream->operator bool())) {
            return mStream->operator bool();
        }

        [[nodiscard]]
        bool operator!() const noexcept(noexcept(mStream->operator!())) {
            return mStream->operator!();
        }

        [[nodiscard]]
        iostate rdstate() const {
            return mStream->rdstate();
        }

        void setstate(iostate state) { mStream->setstate(state); }

        void clear(iostate state = Stream::goodbit) { mStream->clear(state); }

        void close()
            noexcept(noexcept(mStream->close())) requires closable_stream<Stream> {
            mStream->close();
        }
    };

    /// Reads binary values from a stream using the specified endianness.
    template <std::derived_from<std::istream> Stream, std::endian SourceEndian>
    class bin_istream : public binstream_base<Stream> {
    public:
        static constexpr std::endian endianness = SourceEndian;

        template <class... Ts>
        bin_istream(Ts &&..._Val) : binstream_base<Stream>(std::forward<Ts>(_Val)...) {}

        bin_istream(std::unique_ptr<Stream> &&stream)
            : binstream_base<Stream>(std::move(stream)) {}

        /// Allows reading raw data from the underlying stream, without endianness
        /// considerations.
        [[maybe_unused]]
        bin_istream &read_raw(void *data, std::streamsize size) {
            binstream_base<Stream>::mStream->read(reinterpret_cast<char *>(data), size);
            return *this;
        }

#define BINSTREAM_READ_OP(valueType)                                                     \
    [[maybe_unused]]                                                                     \
    bin_istream &operator>>(valueType &value) {                                          \
        read_raw(&value, sizeof(value));                                                 \
        value = from_endian<endianness>(value);                                          \
        return *this;                                                                    \
    }

        BINSTREAM_READ_OP(char)
        BINSTREAM_READ_OP(wchar_t)
        BINSTREAM_READ_OP(char8_t)
        BINSTREAM_READ_OP(char16_t)
        BINSTREAM_READ_OP(char32_t)

        BINSTREAM_READ_OP(unsigned char)
        BINSTREAM_READ_OP(unsigned short)
        BINSTREAM_READ_OP(unsigned int)
        BINSTREAM_READ_OP(unsigned long)
        BINSTREAM_READ_OP(unsigned long long)

        BINSTREAM_READ_OP(signed char)
        BINSTREAM_READ_OP(signed short)
        BINSTREAM_READ_OP(signed int)
        BINSTREAM_READ_OP(signed long)
        BINSTREAM_READ_OP(signed long long)

        BINSTREAM_READ_OP(float)
        BINSTREAM_READ_OP(double)
        BINSTREAM_READ_OP(long double)

        [[maybe_unused]]
        bin_istream &operator>>(bool &value) {
            uint8_t read = 0;
            (*this) >> read;
            value = read != 0;
            return *this;
        }

#undef BINSTREAM_READ_OP
    };

    /// Reads binary values from a stream, in little-endian byte order.
    template <std::derived_from<std::istream> Stream>
    using bin_istream_le = bin_istream<Stream, std::endian::little>;

    /// Reads binary values from a stream, in big-endian byte order.
    template <std::derived_from<std::istream> Stream>
    using bin_istream_be = bin_istream<Stream, std::endian::big>;

    /// Reads binary values from a stream, in native-endian byte order.
    template <std::derived_from<std::istream> Stream>
    using bin_istream_native = bin_istream<Stream, std::endian::native>;

    /// Reads binary values from a file stream, in little-endian byte order.
    using bin_ifstream_le = bin_istream_le<std::ifstream>;

    /// Reads binary values from a file stream, in big-endian byte order.
    using bin_ifstream_be = bin_istream_be<std::ifstream>;

    /// Reads binary values from a file stream, in native-endian byte order.
    using bin_ifstream_native = bin_istream_native<std::ifstream>;

    /// Writes binary values to a stream using the specified endianness.
    template <std::derived_from<std::ostream> Stream, std::endian TargetEndian>
    class bin_ostream : public binstream_base<Stream> {
    public:
        static constexpr std::endian endianness = TargetEndian;

        template <class... Ts>
        bin_ostream(Ts &&..._Val) : binstream_base<Stream>(std::forward<Ts>(_Val)...) {}

        bin_ostream(std::unique_ptr<Stream> &&stream)
            : binstream_base<Stream>(std::move(stream)) {}

        /// Allows writing raw data to the underlying stream, without endianness
        /// considerations.
        [[maybe_unused]]
        bin_ostream &write_raw(const void *data, std::streamsize size) {
            binstream_base<Stream>::mStream->write(
                reinterpret_cast<const char *>(data), size
            );
            return *this;
        }

#define BINSTREAM_WRITE_OP(valueType)                                                    \
    [[maybe_unused]]                                                                     \
    bin_ostream &operator<<(const valueType value) {                                     \
        const valueType converted = to_endian<endianness>(value);                        \
        write_raw(&converted, sizeof(converted));                                        \
        return *this;                                                                    \
    }

        BINSTREAM_WRITE_OP(char)
        BINSTREAM_WRITE_OP(wchar_t)
        BINSTREAM_WRITE_OP(char8_t)
        BINSTREAM_WRITE_OP(char16_t)
        BINSTREAM_WRITE_OP(char32_t)

        BINSTREAM_WRITE_OP(unsigned char)
        BINSTREAM_WRITE_OP(unsigned short)
        BINSTREAM_WRITE_OP(unsigned int)
        BINSTREAM_WRITE_OP(unsigned long)
        BINSTREAM_WRITE_OP(unsigned long long)

        BINSTREAM_WRITE_OP(signed char)
        BINSTREAM_WRITE_OP(signed short)
        BINSTREAM_WRITE_OP(signed int)
        BINSTREAM_WRITE_OP(signed long)
        BINSTREAM_WRITE_OP(signed long long)

        BINSTREAM_WRITE_OP(float)
        BINSTREAM_WRITE_OP(double)
        BINSTREAM_WRITE_OP(long double)

        [[maybe_unused]]
        bin_ostream &operator<<(const bool value) {
            operator<<(static_cast<unsigned char>(value));
            return *this;
        }

#undef BINSTREAM_WRITE_OP
    };

    /// Writes binary values to a stream, in little-endian byte order.
    template <std::derived_from<std::ostream> Stream>
    using bin_ostream_le = bin_ostream<Stream, std::endian::little>;

    /// Writes binary values to a stream, in big-endian byte order.
    template <std::derived_from<std::ostream> Stream>
    using bin_ostream_be = bin_ostream<Stream, std::endian::big>;

    /// Writes binary values to a stream, in native-endian byte order.
    template <std::derived_from<std::ostream> Stream>
    using bin_ostream_native = bin_ostream<Stream, std::endian::native>;

    /// Writes binary values to a file stream, in little-endian byte order.
    using bin_ofstream_le = bin_ostream_le<std::ofstream>;

    /// Writes binary values to a file stream, in big-endian byte order.
    using bin_ofstream_be = bin_ostream_be<std::ofstream>;

    /// Writes binary values to a file stream, in native-endian byte order.
    using bin_ofstream_native = bin_ostream_native<std::ofstream>;

    /*
     * Binary stream concepts and freestanding operators
     *
     * Operators for reading/writing std types go here.
     */

    /// Specifies that a type is readable from a bin_istream using the >> operator.
    template <typename T, typename Stream, std::endian Endian>
    concept binary_readable = requires(T t, bin_istream<Stream, Endian> stream) {
        stream >> t;
    };

    /// Specifies that a type is writable to a bin_ostream using the << operator.
    template <typename T, typename Stream, std::endian Endian>
    concept binary_writable = requires(T t, bin_ostream<Stream, Endian> stream) {
        stream << t;
    };

    /// Reads a vector of elements from the stream.
    template <
        std::derived_from<std::istream> Stream,
        std::endian Endian,
        binary_readable<Stream, Endian> T>
    [[maybe_unused]]
    bin_istream<Stream, Endian> &
    operator>>(bin_istream<Stream, Endian> &stream, std::vector<T> &value) {
        size_t length = 0;
        stream >> length;

        value.resize(length);
        for (auto &item : value) {
            stream >> item;
        }

        return stream;
    }

    /// Writes a vector of elements to the stream.
    template <
        std::derived_from<std::ostream> Stream,
        std::endian Endian,
        binary_writable<Stream, Endian> T>
    [[maybe_unused]]
    bin_ostream<Stream, Endian> &
    operator<<(bin_ostream<Stream, Endian> &stream, const std::vector<T> &value) {
        stream << (size_t)value.size();

        for (auto &item : value) {
            stream << item;
        }

        return stream;
    }

    /// Reads a string from the stream.
    template <
        std::derived_from<std::istream> Stream,
        std::endian Endian,
        typename Char,
        typename Traits,
        typename Alloc>
    [[maybe_unused]]
    bin_istream<Stream, Endian> &operator>>(
        bin_istream<Stream, Endian> &stream, std::basic_string<Char, Traits, Alloc> &value
    ) requires binary_readable<Char, Stream, Endian> {
        size_t length = 0;
        stream >> length;

        value.resize(length, '\0');
        if constexpr (std::endian::native == Endian || sizeof(Char) == 1) {
            // Read raw data directly
            stream.read_raw(value.data(), value.length() * sizeof(Char));
        } else {
            // Read one character at a time for correct endianness
            for (auto &character : value) {
                stream >> character;
            }
        }

        return stream;
    }

    /// Writes a string to the stream.
    template <
        std::derived_from<std::ostream> Stream,
        std::endian Endian,
        typename Char,
        typename Traits,
        typename Alloc>
    [[maybe_unused]]
    bin_ostream<Stream, Endian> &operator<<(
        bin_ostream<Stream, Endian> &stream,
        const std::basic_string<Char, Traits, Alloc> &value
    ) requires binary_writable<Char, Stream, Endian> {
        stream << (size_t)value.size();

        if constexpr (std::endian::native == Endian || sizeof(Char) == 1) {
            // Write raw data directly
            stream.write_raw(value.data(), value.length() * sizeof(Char));
        } else {
            // Write one character at a time for correct endianness
            for (auto &character : value) {
                stream << character;
            }
        }

        return stream;
    }

}
