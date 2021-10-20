namespace u16_combining_characters {
//300-36F
//483-489
//7EB-7F3
//135F-135F
//1A7F-1A7F
//1B6B-1B73
//1DC0-1DE6
//1DFD-1DFF
//20D0-20F0
//2CEF-2CF1
//2DE0-2DFF
//3099-309A
//A66F-A672
//A67C-A67D
//A6F0-A6F1
//A8E0-A8F1
//FE20-FE26
//101FD-101FD
//1D165-1D169
//1D16D-1D172
//1D17B-1D182
//1D185-1D18B
//1D1AA-1D1AD
//1D242-1D244
const char16_t u300start = 0x0300;
const char16_t u300end = 0x036F;

constexpr bool is_combining_characters(char16_t c)
{
    bool flg = false;
    if (u300start <= c && c <= u300end) {
        flg = true;
    }

    return flg;
}
}
namespace u16_surrogatepair {
const char16_t high_surrogate_first = 0xd800;
const char16_t high_surrogate_last = 0xdbff;
const char16_t low_surrogate_first = 0xdc00;
const char16_t low_surrogate_last = 0xdfff;
const char32_t codepoint_surrogatepair_first = 0x010000;
const char32_t codepoint_surrogatepair_last = 0x10ffff;

// 文字が上位サロゲートであれば真を返します。
constexpr bool is_high_surrogate(char16_t c)
{
    return high_surrogate_first <= c && c <= high_surrogate_last;
}

// 文字が下位サロゲートであれば真を返します。
constexpr bool is_low_surrogate(char16_t c)
{
    return low_surrogate_first <= c && c <= low_surrogate_last;
}
constexpr bool is_surrogatepair(char16_t c)
{
    return is_high_surrogate(c) || is_low_surrogate(c);
}
}