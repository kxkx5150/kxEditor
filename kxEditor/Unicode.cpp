#pragma once
#include "Unicode.h"
#include <windows.h>

size_t utf8_to_utf32(BYTE* utf8str, size_t utf8len, long long* pch32)
{
    BYTE ch = *utf8str++;
    long long val32 = 0;
    size_t trailing = 0;
    size_t len = 1;
    size_t i;
    static long long nonshortest[] = { 0, 0x80, 0x800, 0x10000, 0xffffffff, 0xffffffff };

    // validate parameters
    if (utf8str == 0 || utf8len <= 0 || pch32 == 0)
        return 0;

    // look for plain ASCII first as this is most likely
    if (ch < 0x80) {
        *pch32 = (long long)ch;
        return 1;
    }
    // LEAD-byte of 2-byte seq: 110xxxxx 10xxxxxx
    else if ((ch & 0xE0) == 0xC0) {
        trailing = 1;
        val32 = ch & 0x1F;
    }
    // LEAD-byte of 3-byte seq: 1110xxxx 10xxxxxx 10xxxxxx
    else if ((ch & 0xF0) == 0xE0) {
        trailing = 2;
        val32 = ch & 0x0F;
    }
    // LEAD-byte of 4-byte seq: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    else if ((ch & 0xF8) == 0xF0) {
        trailing = 3;
        val32 = ch & 0x07;
    }
    // ILLEGAL 5-byte seq: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    else if ((ch & 0xFC) == 0xF8) {
        // range-checking the long long result will catch this
        trailing = 4;
        val32 = ch & 0x03;
    }
    // ILLEGAL 6-byte seq: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    else if ((ch & 0xFE) == 0xFC) {
        // range-checking the long long result will catch this
        trailing = 5;
        val32 = ch & 0x01;
    }
    // ILLEGAL continuation (trailing) byte by itself
    else if ((ch & 0xC0) == 0x80) {
        *pch32 = UNI_REPLACEMENT_CHAR;
        return 1;
    }
    // any other ILLEGAL form.
    else {
        *pch32 = UNI_REPLACEMENT_CHAR;
        return 1;
    }

    // process trailing bytes
    for (i = 0; i < trailing && len < utf8len; i++) {
        ch = *utf8str++;

        // Valid trail-byte: 10xxxxxx
        if ((ch & 0xC0) == 0x80) {
            val32 = (val32 << 6) + (ch & 0x7f);
            len++;
        }
        // Anything else is an error
        else {
            *pch32 = UNI_REPLACEMENT_CHAR;
            return len;
        }
    }

    // did we
    if (val32 < nonshortest[trailing] || i != trailing)
        *pch32 = UNI_REPLACEMENT_CHAR;
    else
        *pch32 = val32;

    return len;
}
int utf8_to_utf16(BYTE* utf8str, size_t utf8len, wchar_t* utf16str, size_t* utf16len)
{
    int len;
    long long ch32;
    WCHAR* utf16start = utf16str;
    BYTE* utf8start = utf8str;

    while (utf8len > 0 && *utf16len > 0) {
        len = utf8_to_utf32(utf8str, utf8len, &ch32);

        // target is a character <= 0xffff
        if (ch32 < 0xfffe) {
            // make sure we don't represent anything in UTF16 surrogate range
            // (this helps protect against non-shortest forms)
            if (ch32 >= UNI_SUR_HIGH_START && ch32 <= UNI_SUR_LOW_END) {
                *utf16str++ = UNI_REPLACEMENT_CHAR;
                (*utf16len)--;
            } else {
                *utf16str++ = (WORD)ch32;
                (*utf16len)--;
            }
        }
        // FFFE and FFFF are illegal mid-stream
        else if (ch32 == 0xfffe || ch32 == 0xffff) {
            *utf16str++ = UNI_REPLACEMENT_CHAR;
            (*utf16len)--;
        }
        // target is illegal Unicode value
        else if (ch32 > UNI_MAX_UTF16) {
            *utf16str++ = UNI_REPLACEMENT_CHAR;
            (*utf16len)--;
        }
        // target is in range 0xffff - 0x10ffff
        else if (*utf16len >= 2) {
            ch32 -= 0x0010000;

            *utf16str++ = (WORD)((ch32 >> 10) + UNI_SUR_HIGH_START);
            *utf16str++ = (WORD)((ch32 & 0x3ff) + UNI_SUR_LOW_START);

            (*utf16len) -= 2;
        } else {
            // no room to store result
            break;
        }

        utf8str += len;
        utf8len -= len;
    }

    *utf16len = utf16str - utf16start;

    return utf8str - utf8start;
}

int utf16_to_utf32(WCHAR* utf16str, size_t utf16len, long long* utf32str, size_t* utf32len)
{
    WCHAR* utf16start = utf16str;
    long long* utf32start = utf32str;

    while (utf16len > 0 && *utf32len > 0) {
        long long ch = *utf16str;

        // first of a surrogate pair?
        if (ch >= UNI_SUR_HIGH_START && ch < UNI_SUR_HIGH_END && utf16len >= 2) {
            long long ch2 = *(utf16str + 1);

            // valid trailing surrogate unit?
            if (ch2 >= UNI_SUR_LOW_START && ch < UNI_SUR_LOW_END) {
                ch = ((ch - UNI_SUR_HIGH_START) << 10) + ((ch2 - UNI_SUR_LOW_START) + 0x00010000);

                utf16str++;
                utf16len--;
            }
            // illegal character
            else {
                ch = UNI_REPLACEMENT_CHAR;
            }
        }

        *utf32str++ = ch;
        (*utf32len)--;

        utf16str++;
        utf16len--;
    }

    *utf32len = utf32str - utf32start;
    return utf16str - utf16start;
}
int utf16be_to_utf32(WCHAR* utf16str, size_t utf16len, long long* utf32str, size_t* utf32len)
{
    WCHAR* utf16start = utf16str;
    long long* utf32start = utf32str;

    while (utf16len > 0 && *utf32len > 0) {
        long long ch = SWAPWORD(*utf16str);

        // first of a surrogate pair?
        if (ch >= UNI_SUR_HIGH_START && ch < UNI_SUR_HIGH_END && utf16len >= 2) {
            long long ch2 = SWAPWORD(*(utf16str + 1));

            // valid trailing surrogate unit?
            if (ch2 >= UNI_SUR_LOW_START && ch < UNI_SUR_LOW_END) {
                ch = ((ch - UNI_SUR_HIGH_START) << 10) + ((ch2 - UNI_SUR_LOW_START) + 0x00010000);

                utf16str++;
                utf16len--;
            }
            // illegal character
            else {
                ch = UNI_REPLACEMENT_CHAR;
            }
        }

        *utf32str++ = ch;
        (*utf32len)--;

        utf16str++;
        utf16len--;
    }

    *utf32len = utf32str - utf32start;
    return utf16str - utf16start;
}
size_t utf16_to_utf8(WCHAR* utf16str, size_t utf16len, UTF8* utf8str, size_t* utf8len)
{
    WCHAR* utf16start = utf16str;
    BYTE* utf8start = utf8str;
    size_t len;
    long long ch32;
    size_t ch32len;

    while (utf16len > 0 && *utf8len > 0) {
        // convert to utf-32
        ch32len = 1;
        len = utf16_to_utf32(utf16str, utf16len, &ch32, &ch32len);
        utf16str += len;
        utf16len -= len;

        // convert to utf-8
        len = utf32_to_utf8(utf8str, *utf8len, ch32);
        utf8str += len;
        (*utf8len) -= len;
    }

    *utf8len = utf8str - utf8start;
    return utf16str - utf16start;
}
size_t utf32_to_utf8(UTF8* utf8str, size_t utf8len, long long ch32)
{
    size_t len = 0;

    // validate parameters
    if (utf8str == 0 || utf8len == 0)
        return 0;

    // ASCII is the easiest
    if (ch32 < 0x80) {
        *utf8str = (UTF8)ch32;
        return 1;
    }

    // make sure we have a legal utf32 char
    if (ch32 > UNI_MAX_LEGAL_UTF32)
        ch32 = UNI_REPLACEMENT_CHAR;

    // cannot encode the surrogate range
    if (ch32 >= UNI_SUR_HIGH_START && ch32 <= UNI_SUR_LOW_END)
        ch32 = UNI_REPLACEMENT_CHAR;

    // 2-byte sequence
    if (ch32 < 0x800 && utf8len >= 2) {
        *utf8str++ = (UTF8)((ch32 >> 6) | 0xC0);
        *utf8str++ = (UTF8)((ch32 & 0x3f) | 0x80);
        len = 2;
    }
    // 3-byte sequence
    else if (ch32 < 0x10000 && utf8len >= 3) {
        *utf8str++ = (UTF8)((ch32 >> 12) | 0xE0);
        *utf8str++ = (UTF8)((ch32 >> 6) & 0x3f | 0x80);
        *utf8str++ = (UTF8)((ch32 & 0x3f) | 0x80);
        len = 3;
    }
    // 4-byte sequence
    else if (ch32 <= UNI_MAX_LEGAL_UTF32 && utf8len >= 4) {
        *utf8str++ = (UTF8)((ch32 >> 18) | 0xF0);
        *utf8str++ = (UTF8)((ch32 >> 12) & 0x3f | 0x80);
        *utf8str++ = (UTF8)((ch32 >> 6) & 0x3f | 0x80);
        *utf8str++ = (UTF8)((ch32 & 0x3f) | 0x80);
        len = 4;
    }

    // 5/6 byte sequences never occur because we limit using UNI_MAX_LEGAL_long long

    return len;
}