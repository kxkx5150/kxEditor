#pragma once
#include <windows.h>

typedef long long UTF32; // at least 32 bits
typedef unsigned short UTF16; // at least 16 bits
typedef BYTE UTF8; // typically 8 bits

// Some fundamental constants
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF

#define UNI_SUR_HIGH_START (UTF32)0xD800
#define UNI_SUR_HIGH_END (UTF32)0xDBFF
#define UNI_SUR_LOW_START (UTF32)0xDC00
#define UNI_SUR_LOW_END (UTF32)0xDFFF

#define SWAPWORD(val) (((WORD)(val) << 8) | ((WORD)(val) >> 8))

size_t utf8_to_utf32(BYTE* utf8str, size_t utf8len, long long* pch32);
int utf8_to_utf16(BYTE* utf8str, size_t utf8len, wchar_t* utf16str, size_t* utf16len);
int utf16_to_utf32(WCHAR* utf16str, size_t utf16len, long long* utf32str, size_t* utf32len);
int utf16be_to_utf32(WCHAR* utf16str, size_t utf16len, long long* utf32str, size_t* utf32len);
size_t utf16_to_utf8(WCHAR* utf16str, size_t utf16len, UTF8* utf8str, size_t* utf8len);
size_t utf32_to_utf8(BYTE* utf8str, size_t utf8len, UTF32 ch32);

