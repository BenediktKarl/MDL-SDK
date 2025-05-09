/******************************************************************************
 * Copyright (c) 2011-2025, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "pch.h"

#include "compilercore_allocator.h"
#include "compilercore_wchar_support.h"

namespace mi {
namespace mdl {

// Get the length of an utf16 encoded string in code points, not in words.
size_t utf16_len(wchar_t const *p)
{
    size_t l = 0;

    for (;;) {
        wchar_t wc = *p++;

        if (wc == L'\0') {
            return l;
        }
        ++l;
        if (0xD800 <= wc && wc <= 0xDBFF) {
            wc = *p;
            if (0xDC00 <= wc && wc <= 0xDFFF) {
                // assume valid surrogate
                ++p;
            }
        }
    }
}

// Get the length of an utf8 encoded string in code points, not in bytes.
size_t utf8_len(char const *p)
{
    size_t l = 0;

    for (;;) {
        char c = *p++;

        if (c == '\0') {
            return l;
        }
        ++l;
        if (c <= 0x7F) {
            // 0xxxxxxx
        } else if ((c & 0xF8) == 0xF0) {
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            p += 3;
        } else if ((c & 0xF0) == 0xE0) {
            // 1110xxxx 10xxxxxx 10xxxxxx
            p += 2;
        } else if ((c & 0xE0) == 0xC0) {
            // 110xxxxx 10xxxxxx
            p += 1;
        }
    }
}

// Converts a utf16 string into an utf8 encoded string.
char const *utf16_to_utf8(string &res, wchar_t const *src)
{
    res.clear();

    for (wchar_t const *p = src; *p != L'\0'; ++p) {
        unsigned code = *p;

        if (code <= 0x7F) {
            // 0xxxxxxx
            res += char(code);
        } else if (code <= 0x7FF) {
            // 110xxxxx 10xxxxxx
            unsigned high = code >> 6;
            unsigned low  = code & 0x3F;
            res += char(0xC0 + high);
            res += char(0x80 + low);
        } else if (0xD800 <= code && code <= 0xDBFF && 0xDC00 <= p[1] && p[1] <= 0xDFFF) {
            // surrogate pair, 0x10000 to 0x10FFFF
            unsigned high = code & 0x3FF;
            unsigned low  = p[1] & 0x3FF;
            code = 0x10000 + ((high << 10) | low);

            if (code <= 0x10FFFF) {
                // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                unsigned high = (code >> 18) & 0x07;
                unsigned mh   = (code >> 12) & 0x3F;
                unsigned ml   = (code >> 6) & 0x3F;
                unsigned low  = code & 0x3F;
                res += char(0xF0 + high);
                res += char(0x80 + mh);
                res += char(0x80 + ml);
                res += char(0x80 + low);
            } else {
                // error, replace by (U+FFFD) (or EF BF BD in UTF-8)
                res += char(0xEF);
                res += char(0xBF);
                res += char(0xBD);
            }
            ++p;
        } else if (code <= 0xFFFF) {
            if (code < 0xD800 || code > 0xDFFF) {
                // 1110xxxx 10xxxxxx 10xxxxxx
                unsigned high   = code >> 12;
                unsigned middle = (code >> 6) & 0x3F;
                unsigned low    = code & 0x3F;
                res += char(0xE0 + high);
                res += char(0x80 + middle);
                res += char(0x80 + low);
            } else {
                // forbidden surrogate part, replace by (U+FFFD) (or EF BF BD in UTF-8)
                res += char(0xEF);
                res += char(0xBF);
                res += char(0xBD);
            }
        } else if (code <= 0x10FFFF) {
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            unsigned high = (code >> 18) & 0x07;
            unsigned mh   = (code >> 12) & 0x3F;
            unsigned ml   = (code >> 6) & 0x3F;
            unsigned low  = code & 0x3F;
            res += char(0xF0 + high);
            res += char(0x80 + mh);
            res += char(0x80 + ml);
            res += char(0x80 + low);
        } else {
            // error, replace by (U+FFFD) (or EF BF BD in UTF-8)
            res += char(0xEF);
            res += char(0xBF);
            res += char(0xBD);
        }
    }
    return res.c_str();
}

// Converts one utf8 character to a utf32 encoded unicode character.
char const *utf8_to_unicode_char(char const *up, unsigned &res)
{
    bool error = false;
    unsigned char ch = up[0];

    // find start code: either 0xxxxxxx or 11xxxxxx
    while ((ch >= 0x80) && ((ch & 0xC0) != 0xC0)) {
        ++up;
        ch = up[0];
    }

    if (ch <= 0x7F) {
        // 0xxxxxxx
        res = ch;
        up += 1;
    } else if ((ch & 0xF8) == 0xF0) {
        // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        unsigned c1 = ch & 0x07; ch = up[1]; error |= (ch & 0xC0) != 0x80;
        unsigned c2 = ch & 0x3F; ch = up[2]; error |= (ch & 0xC0) != 0x80;
        unsigned c3 = ch & 0x3F; ch = up[3]; error |= (ch & 0xC0) != 0x80;
        unsigned c4 = ch & 0x3F;
        res = (c1 << 18) | (c2 << 12) | (c3 << 6) | c4;

        // must be U+10000 .. U+10FFFF
        error |= (res < 0x1000) || (res > 0x10FFFF);

        // Because surrogate code points are not Unicode scalar values, any UTF-8 byte
        // sequence that would otherwise map to code points U+D800..U+DFFF is illformed
        error |= (0xD800 <= res) && (res <= 0xDFFF);

        if (!error) {
            up += 4;
        } else {
            res = 0xFFFD;  // replacement character
            up += 1;
        }
    } else if ((ch & 0xF0) == 0xE0) {
        // 1110xxxx 10xxxxxx 10xxxxxx
        unsigned c1 = ch & 0x0F; ch = up[1]; error |= (ch & 0xC0) != 0x80;
        unsigned c2 = ch & 0x3F; ch = up[2]; error |= (ch & 0xC0) != 0x80;
        unsigned c3 = ch & 0x3F;
        res = (c1 << 12) | (c2 << 6) | c3;

        // must be U+0800 .. U+FFFF
        error |= res < 0x0800;

        // Because surrogate code points are not Unicode scalar values, any UTF-8 byte
        // sequence that would otherwise map to code points U+D800..U+DFFF is illformed
        error |= (0xD800 <= res) && (res <= 0xDFFF);

        if (!error) {
            up += 3;
        } else {
            res = 0xFFFD;  // replacement character
            up += 1;
        }
    } else if ((ch & 0xE0) == 0xC0) {
        // 110xxxxx 10xxxxxx
        unsigned c1 = ch & 0x1F; ch = up[1]; error |= (ch & 0xC0) != 0x80;
        unsigned c2 = ch & 0x3F;
        res = (c1 << 6) | c2;

        // must be U+0080 .. U+07FF
        error |= res < 0x80;

        if (!error) {
            up += 2;
        } else {
            res = 0xFFFD;  // replacement character
            up += 1;
        }
    } else {
        // error
        res = 0xFFFD;  // replacement character
        up += 1;
    }
    return up;
}

// Converts a utf8 encoded string into a utf16 encoded string.
wchar_t const *utf8_to_utf16(wstring &res, char const *src)
{
    res.clear();

    while (*src != '\0') {
        unsigned unicode_char;

        src = utf8_to_unicode_char(src, unicode_char);
        utf16_append(res, unicode_char);
    }
    return res.c_str();
}

// Converts a utf8 encoded string into a utf32 encoded string.
unsigned const *utf8_to_utf32(u32string &res, char const *src)
{
    res.clear();

    while (*src != '\0') {
        unsigned unicode_char;

        src = utf8_to_unicode_char(src, unicode_char);
        res.append(unicode_char);
    }
    return res.c_str();
}

// Converts a u32 string into an utf8 encoded string.
char const *utf32_to_utf8(string &res, unsigned const *src)
{
    res.clear();

    for (unsigned const *p = src; *p != L'\0'; ++p) {
        unsigned code = *p;

        if (code <= 0x7F) {
            // 0xxxxxxx
            res += char(code);
        } else if (code <= 0x7FF) {
            // 110xxxxx 10xxxxxx
            unsigned high = code >> 6;
            unsigned low  = code & 0x3F;
            res += char(0xC0 + high);
            res += char(0x80 + low);
        } else if (code <= 0xFFFF) {
            if (code < 0xD800 || code > 0xDFFF) {
                // 1110xxxx 10xxxxxx 10xxxxxx
                unsigned high   = code >> 12;
                unsigned middle = (code >> 6) & 0x3F;
                unsigned low    = code & 0x3F;
                res += char(0xE0 + high);
                res += char(0x80 + middle);
                res += char(0x80 + low);
            } else {
                // forbidden surrogate part, replace by (U+FFFD) (or EF BF BD in UTF-8)
                res += char(0xEF);
                res += char(0xBF);
                res += char(0xBD);
            }
        } else if (code <= 0x10FFFF) {
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            unsigned high = (code >> 18) & 0x07;
            unsigned mh   = (code >> 12) & 0x3F;
            unsigned ml   = (code >> 6) & 0x3F;
            unsigned low  = code & 0x3F;
            res += char(0xF0 + high);
            res += char(0x80 + mh);
            res += char(0x80 + ml);
            res += char(0x80 + low);
        } else {
            // error, replace by (U+FFFD) (or EF BF BD in UTF-8)
            res += char(0xEF);
            res += char(0xBF);
            res += char(0xBD);
        }
    }
    return res.c_str();
}

// Add an unicode utf32 character to an utf16 string.
void utf16_append(wstring &s, unsigned c)
{
    // assume only valid utf32 characters added
    if (c < 0x10000) {
        s += wchar_t(c);
    } else {
            // encode as surrogate pair
        c -= 0x10000;
        s.append(wchar_t((c >> 10) + 0xD800));
        s.append(wchar_t((c & 0x3FF) + 0xDC00));
    }
}

// Get the next unicode character form an utf16 string.
unsigned utf16_next(wchar_t const *&p)
{
    unsigned c = unsigned(*p++);

    if (0xD800 <= c && c <= 0xDCFF) {
        // surrogate pair
        unsigned np = unsigned(*p);

        if (c <= 0xDBFF && 0xDC00 <= np && np <= 0xDFFF) {
            // valid pair
            ++p;
            c = (c - 0xD800) << 10;
            np -= 0xDC00;
            c = (c | np) + 0x10000;
        } else {
            // error, replace by (U+FFFD)
            c = 0xFFFD;
        }
    }
    return c;
}

// Add an unicode utf32 character to an utf8 string.
void utf8_append(string &s, unsigned c)
{
    // assume only valid utf32 characters added
    if (c <= 0x7F) {
        // 0xxxxxxx
        s += char(c);
    } else if (c <= 0x7FF) {
        // 110xxxxx 10xxxxxx
        unsigned high = c >> 6;
        unsigned low = c & 0x3F;
        s += char(0xC0 + high);
        s += char(0x80 + low);
    } else if (c <= 0xFFFF) {
        if (c < 0xD800 || c > 0xDFFF) {
            // 1110xxxx 10xxxxxx 10xxxxxx
            unsigned high = c >> 12;
            unsigned middle = (c >> 6) & 0x3F;
            unsigned low = c & 0x3F;
            s += char(0xE0 + high);
            s += char(0x80 + middle);
            s += char(0x80 + low);
        }
    } else if (c <= 0x10FFFF) {
        // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        unsigned high = (c >> 18) & 0x07;
        unsigned mh = (c >> 12) & 0x3F;
        unsigned ml = (c >> 6) & 0x3F;
        unsigned low = c & 0x3F;
        s += char(0xF0 + high);
        s += char(0x80 + mh);
        s += char(0x80 + ml);
        s += char(0x80 + low);
    } else {
        // error, replace by (U+FFFD) (or EF BF BD in UTF-8)
        s += char(0xEF);
        s += char(0xBF);
        s += char(0xBD);
    }

}

// Get the next unicode character from an utf8 string.
unsigned utf8_next(char const *&p)
{
    unsigned c = unsigned(*p++);

    if (c <= 0x7F) {
        // nothing to do, first 127 chars are the same in ascii and utf8
        // 0xxxxxxx or end of file character
    } else if ((c & 0xF8) == 0xF0) {
        // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        bool error = false;

        int c1 = c & 0x07; c = unsigned(*p++); error |= (c & 0xC0) != 0x80;
        int c2 = c & 0x3F; c = unsigned(*p++); error |= (c & 0xC0) != 0x80;
        int c3 = c & 0x3F; c = unsigned(*p++); error |= (c & 0xC0) != 0x80;
        int c4 = c & 0x3F;
        c = (c1 << 18) | (c2 << 12) | (c3 << 6) | c4;

        // must be U+10000 .. U+10FFFF
        error |= (c < 0x1000) || (c > 0x10FFFF);

        // Because surrogate code points are not Unicode scalar values, any UTF-8 byte
        // sequence that would otherwise map to code points U+D800..U+DFFF is illformed
        error |= (0xD800 <= c) && (c <= 0xDFFF);

        if (error) {
            c = 0xFFFD;  // replacement character
        }
    } else if ((c & 0xF0) == 0xE0) {
        // 1110xxxx 10xxxxxx 10xxxxxx
        bool error = false;

        int c1 = c & 0x0F; c = unsigned(*p++); error |= (c & 0xC0) != 0x80;
        int c2 = c & 0x3F; c = unsigned(*p++); error |= (c & 0xC0) != 0x80;
        int c3 = c & 0x3F;
        c = (c1 << 12) | (c2 << 6) | c3;

        // must be U+0800 .. U+FFFF
        error |= c < 0x0800;

        // Because surrogate code points are not Unicode scalar values, any UTF-8 byte
        // sequence that would otherwise map to code points U+D800..U+DFFF is illformed
        error |= (0xD800 <= c) && (c <= 0xDFFF);

        if (error) {
            c = 0xFFFD;  // replacement character
        }
    } else if ((c & 0xE0) == 0xC0) {
        // 110xxxxx 10xxxxxx
        bool error = false;

        int c1 = c & 0x1F; c = unsigned(*p++); error |= (c & 0xC0) != 0x80;
        int c2 = c & 0x3F;
        c = (c1 << 6) | c2;

        // must be U+0080 .. U+07FF
        error |= c < 0x80;

        if (error) {
            c = 0xFFFD;  // replacement character
        }
    } else {
        // error
        c = 0xFFFD;  // replacement character
    }
    return c;
}

}  // mdl
}  // mi
