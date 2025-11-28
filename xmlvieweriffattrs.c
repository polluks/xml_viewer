#include "xmlvieweriffattrs.h"

#include <stdio.h>
#include <string.h>

#include <proto/dos.h>

#include "xmlviewerlocale.h"

#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

static ULONG ReadBE32Attr(const UBYTE *buffer)
{
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    ULONG value;

    memcpy(&value, buffer, sizeof(value));
    return value;
#else
    ULONG value;

    memcpy(&value, buffer, sizeof(value));
    return __builtin_bswap32(value);
#endif
}

static ULONG ReadBE16Attr(const UBYTE *buffer)
{
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    UWORD value;

    memcpy(&value, buffer, sizeof(value));
    return value;
#else
    UWORD value;

    memcpy(&value, buffer, sizeof(value));
    return __builtin_bswap16(value);
#endif
}

static LONG ReadBE16SAttr(const UBYTE *buffer)
{
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    WORD value;

    memcpy(&value, buffer, sizeof(value));
    return (LONG)value;
#else
    UWORD value;

    memcpy(&value, buffer, sizeof(value));
    value = __builtin_bswap16(value);
    return (LONG)((WORD)value);
#endif
}

static void AddEnumAttribute(struct xml_data *node_data, const char *name, const char *symbolic, ULONG value)
{
    char buffer[64];

    if (symbolic)
    {
        snprintf(buffer, sizeof(buffer), "%s", symbolic);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "%lu", (unsigned long)value);
    }

    AddAttribute(node_data, name, buffer);
}

static BOOL SkipBytes(BPTR fileIFF, ULONG bytes)
{
    return Seek(fileIFF, bytes, OFFSET_CURRENT) != -1;
}

BOOL ParseIffChunkAttributes(BPTR fileIFF, ULONG chunk_id, ULONG chunk_size, struct xml_data *node_data)
{
    ULONG consumed = 0;
    UBYTE buffer[256];

    switch (chunk_id)
    {
        case MAKE_ID('B', 'M', 'H', 'D'):
        {
            /* BitMapHeader */
            const ULONG bmhd_size = 20;

            if (chunk_size < bmhd_size)
                return FALSE;

            if (Read(fileIFF, buffer, bmhd_size) != (LONG)bmhd_size)
                return FALSE;

            consumed = bmhd_size;

            {
                char temp[32];
                ULONG width = ReadBE16Attr(buffer);
                ULONG height = ReadBE16Attr(buffer + 2);
                LONG pos_x = ReadBE16SAttr(buffer + 4);
                LONG pos_y = ReadBE16SAttr(buffer + 6);
                UBYTE planes = buffer[8];
                UBYTE masking = buffer[9];
                UBYTE compression = buffer[10];
                UWORD transparent = (UWORD)ReadBE16Attr(buffer + 12);
                UBYTE xAspect = buffer[14];
                UBYTE yAspect = buffer[15];
                WORD pageWidth = (WORD)ReadBE16SAttr(buffer + 16);
                WORD pageHeight = (WORD)ReadBE16SAttr(buffer + 18);

                snprintf(temp, sizeof(temp), "%lu", (unsigned long)width);
                AddAttribute(node_data, "width", temp);
                snprintf(temp, sizeof(temp), "%lu", (unsigned long)height);
                AddAttribute(node_data, "height", temp);
                snprintf(temp, sizeof(temp), "%ld", (long)pos_x);
                AddAttribute(node_data, "x", temp);
                snprintf(temp, sizeof(temp), "%ld", (long)pos_y);
                AddAttribute(node_data, "y", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)planes);
                AddAttribute(node_data, "planes", temp);

                switch (masking)
                {
                    case 0: AddEnumAttribute(node_data, "masking", "mskNone", masking); break;
                    case 1: AddEnumAttribute(node_data, "masking", "mskHasMask", masking); break;
                    case 2: AddEnumAttribute(node_data, "masking", "mskHasTransparentColor", masking); break;
                    case 3: AddEnumAttribute(node_data, "masking", "mskLasso", masking); break;
                    default: AddEnumAttribute(node_data, "masking", NULL, masking); break;
                }

                switch (compression)
                {
                    case 0: AddEnumAttribute(node_data, "compression", "cmpNone", compression); break;
                    case 1: AddEnumAttribute(node_data, "compression", "cmpByteRun1", compression); break;
                    default: AddEnumAttribute(node_data, "compression", NULL, compression); break;
                }

                snprintf(temp, sizeof(temp), "%u", (unsigned int)transparent);
                AddAttribute(node_data, "transparentColor", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)xAspect);
                AddAttribute(node_data, "xAspect", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)yAspect);
                AddAttribute(node_data, "yAspect", temp);
                snprintf(temp, sizeof(temp), "%d", (int)pageWidth);
                AddAttribute(node_data, "pageWidth", temp);
                snprintf(temp, sizeof(temp), "%d", (int)pageHeight);
                AddAttribute(node_data, "pageHeight", temp);
            }

            break;
        }
        case MAKE_ID('G', 'R', 'A', 'B'):
        {
            const ULONG grab_size = 4;

            if (chunk_size < grab_size)
                return FALSE;

            if (Read(fileIFF, buffer, grab_size) != (LONG)grab_size)
                return FALSE;

            consumed = grab_size;

            {
                char temp[32];
                LONG pos_x = ReadBE16SAttr(buffer);
                LONG pos_y = ReadBE16SAttr(buffer + 2);

                snprintf(temp, sizeof(temp), "%ld", (long)pos_x);
                AddAttribute(node_data, "x", temp);
                snprintf(temp, sizeof(temp), "%ld", (long)pos_y);
                AddAttribute(node_data, "y", temp);
            }
            break;
        }
        case MAKE_ID('D', 'E', 'S', 'T'):
        {
            const ULONG dest_size = 10;

            if (chunk_size < dest_size)
                return FALSE;

            if (Read(fileIFF, buffer, dest_size) != (LONG)dest_size)
                return FALSE;

            consumed = dest_size;

            {
                char temp[32];
                UBYTE depth = buffer[0];
                UBYTE pad = buffer[1];
                UWORD planePick = (UWORD)ReadBE16Attr(buffer + 2);
                UWORD planeOnOff = (UWORD)ReadBE16Attr(buffer + 4);
                UWORD planeMask = (UWORD)ReadBE16Attr(buffer + 6);

                snprintf(temp, sizeof(temp), "%u", (unsigned int)depth);
                AddAttribute(node_data, "depth", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)pad);
                AddAttribute(node_data, "pad1", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)planePick);
                AddAttribute(node_data, "planePick", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)planeOnOff);
                AddAttribute(node_data, "planeOnOff", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)planeMask);
                AddAttribute(node_data, "planeMask", temp);
            }
            break;
        }
        case MAKE_ID('C', 'M', 'A', 'P'):
        {
            ULONG num_entries = chunk_size / 3;
            ULONG remaining = chunk_size;
            UWORD idx = 0;

            while (remaining >= 3)
            {
                char temp[32];
                ULONG r, g, b;

                if (Read(fileIFF, buffer, 3) != 3)
                    return FALSE;

                consumed += 3;
                remaining -= 3;

                r = buffer[0];
                g = buffer[1];
                b = buffer[2];

                snprintf(temp, sizeof(temp), "%u", (unsigned int)r);
                AddAttribute(node_data, "r", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)g);
                AddAttribute(node_data, "g", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)b);
                AddAttribute(node_data, "b", temp);

                snprintf(temp, sizeof(temp), "%u", (unsigned int)idx);
                AddAttribute(node_data, "index", temp);

                idx++;

                if (remaining >= 3)
                {
                    if (!SkipBytes(fileIFF, 0))
                        return FALSE;
                }
            }

            snprintf(buffer, sizeof(buffer), "%lu", (unsigned long)num_entries);
            AddAttribute(node_data, "entries", (const char *)buffer);

            if (remaining > 0)
            {
                if (!SkipBytes(fileIFF, remaining))
                    return FALSE;
                consumed += remaining;
            }

            break;
        }
        case MAKE_ID('C', 'R', 'N', 'G'):
        {
            const ULONG crng_size = 8;

            if (chunk_size < crng_size)
                return FALSE;

            if (Read(fileIFF, buffer, crng_size) != (LONG)crng_size)
                return FALSE;

            consumed = crng_size;

            {
                char temp[32];
                UWORD rate = (UWORD)ReadBE16Attr(buffer + 2);
                UWORD active = (UWORD)ReadBE16Attr(buffer + 4);
                WORD low = (WORD)buffer[6];
                WORD high = (WORD)buffer[7];

                snprintf(temp, sizeof(temp), "%u", (unsigned int)buffer[0]);
                AddAttribute(node_data, "pad1", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)buffer[1]);
                AddAttribute(node_data, "rate", temp);
                snprintf(temp, sizeof(temp), "%d", (int)rate);
                AddAttribute(node_data, "rateFrames", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)active);
                AddAttribute(node_data, "active", temp);
                snprintf(temp, sizeof(temp), "%d", (int)low);
                AddAttribute(node_data, "low", temp);
                snprintf(temp, sizeof(temp), "%d", (int)high);
                AddAttribute(node_data, "high", temp);
            }

            break;
        }
        case MAKE_ID('V', 'B', 'M', 'P'):
        {
            const ULONG vbmp_size = 6;

            if (chunk_size < vbmp_size)
                return FALSE;

            if (Read(fileIFF, buffer, vbmp_size) != (LONG)vbmp_size)
                return FALSE;

            consumed = vbmp_size;

            {
                char temp[32];
                WORD direction = (WORD)ReadBE16SAttr(buffer);
                WORD dest_height = (WORD)ReadBE16SAttr(buffer + 2);
                UBYTE pad = buffer[4];
                UBYTE skip = buffer[5];

                snprintf(temp, sizeof(temp), "%d", (int)direction);
                AddAttribute(node_data, "direction", temp);
                snprintf(temp, sizeof(temp), "%d", (int)dest_height);
                AddAttribute(node_data, "destHeight", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)pad);
                AddAttribute(node_data, "pad", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)skip);
                AddAttribute(node_data, "skip", temp);
            }

            break;
        }
        case MAKE_ID('B', 'O', 'D', 'Y'):
        {
            char temp[32];

            snprintf(temp, sizeof(temp), "%lu", (unsigned long)chunk_size);
            AddAttribute(node_data, "dataBytes", temp);

            if (!SkipBytes(fileIFF, chunk_size))
                return FALSE;
            consumed = chunk_size;
            break;
        }
        case MAKE_ID('D', 'P', 'I', ' '):
        {
            const ULONG dpi_size = 4;

            if (chunk_size < dpi_size)
                return FALSE;

            if (Read(fileIFF, buffer, dpi_size) != (LONG)dpi_size)
                return FALSE;

            consumed = dpi_size;

            {
                char temp[32];
                ULONG dpi_x = ReadBE16Attr(buffer);
                ULONG dpi_y = ReadBE16Attr(buffer + 2);

                snprintf(temp, sizeof(temp), "%lu", (unsigned long)dpi_x);
                AddAttribute(node_data, "dpiX", temp);
                snprintf(temp, sizeof(temp), "%lu", (unsigned long)dpi_y);
                AddAttribute(node_data, "dpiY", temp);
            }

            break;
        }
        case MAKE_ID('A', 'N', 'H', 'D'):
        {
            const ULONG anhd_size = 8;
            ULONG remaining = chunk_size;

            if (chunk_size < anhd_size)
                return FALSE;

            if (Read(fileIFF, buffer, anhd_size) != (LONG)anhd_size)
                return FALSE;

            consumed = anhd_size;
            remaining -= anhd_size;

            {
                char temp[32];
                UBYTE operation = buffer[0];
                UBYTE masking = buffer[1];
                UWORD w = (UWORD)ReadBE16Attr(buffer + 2);
                UWORD h = (UWORD)ReadBE16Attr(buffer + 4);
                UBYTE x = buffer[6];
                UBYTE y = buffer[7];

                switch (operation)
                {
                    case 0: AddEnumAttribute(node_data, "operation", "direct", operation); break;
                    case 1: AddEnumAttribute(node_data, "operation", "XOR", operation); break;
                    case 2: AddEnumAttribute(node_data, "operation", "long delta", operation); break;
                    default: AddEnumAttribute(node_data, "operation", NULL, operation); break;
                }

                switch (masking)
                {
                    case 0: AddEnumAttribute(node_data, "masking", "normal", masking); break;
                    case 1: AddEnumAttribute(node_data, "masking", "XOR", masking); break;
                    case 2: AddEnumAttribute(node_data, "masking", "mask", masking); break;
                    case 3: AddEnumAttribute(node_data, "masking", "interval", masking); break;
                    default: AddEnumAttribute(node_data, "masking", NULL, masking); break;
                }

                snprintf(temp, sizeof(temp), "%u", (unsigned int)w);
                AddAttribute(node_data, "width", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)h);
                AddAttribute(node_data, "height", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)x);
                AddAttribute(node_data, "x", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)y);
                AddAttribute(node_data, "y", temp);
            }

            if (remaining >= 12)
            {
                if (Read(fileIFF, buffer, 12) != 12)
                    return FALSE;

                consumed += 12;
                remaining -= 12;

                {
                    char temp[32];
                    LONG seconds = (LONG)ReadBE32Attr(buffer + 4);
                    LONG microseconds = (LONG)ReadBE32Attr(buffer + 8);

                    snprintf(temp, sizeof(temp), "%u", (unsigned int)buffer[0]);
                    AddAttribute(node_data, "pad1", temp);
                    snprintf(temp, sizeof(temp), "%u", (unsigned int)buffer[1]);
                    AddAttribute(node_data, "pad2", temp);
                    snprintf(temp, sizeof(temp), "%u", (unsigned int)buffer[2]);
                    AddAttribute(node_data, "pad3", temp);
                    snprintf(temp, sizeof(temp), "%u", (unsigned int)buffer[3]);
                    AddAttribute(node_data, "pad4", temp);
                    snprintf(temp, sizeof(temp), "%ld", (long)seconds);
                    AddAttribute(node_data, "seconds", temp);
                    snprintf(temp, sizeof(temp), "%ld", (long)microseconds);
                    AddAttribute(node_data, "microseconds", temp);
                }
            }

            if (remaining >= 4)
            {
                if (Read(fileIFF, buffer, 4) != 4)
                    return FALSE;

                consumed += 4;
                remaining -= 4;

                AddAttribute(node_data, "pad", "present");
            }

            if (remaining > 0)
            {
                if (!SkipBytes(fileIFF, remaining))
                    return FALSE;
                consumed += remaining;
            }

            break;
        }
        case MAKE_ID('V', 'H', 'D', 'R'):
        {
            const ULONG vhdr_size = 18;

            if (chunk_size < vhdr_size)
                return FALSE;

            if (Read(fileIFF, buffer, vhdr_size) != (LONG)vhdr_size)
                return FALSE;

            consumed = vhdr_size;

            {
                char temp[64];
                ULONG oneShotHiSamples = ReadBE32Attr(buffer);
                ULONG repeatHiSamples = ReadBE32Attr(buffer + 4);
                ULONG samplesPerHiCycle = ReadBE32Attr(buffer + 8);
                UWORD samplesPerSec = (UWORD)ReadBE16Attr(buffer + 12);
                UBYTE ctOctave = buffer[14];
                UBYTE sCompression = buffer[15];
                LONG volume = (LONG)ReadBE32Attr(buffer + 16);

                snprintf(temp, sizeof(temp), "%lu", (unsigned long)oneShotHiSamples);
                AddAttribute(node_data, "oneShotHiSamples", temp);
                snprintf(temp, sizeof(temp), "%lu", (unsigned long)repeatHiSamples);
                AddAttribute(node_data, "repeatHiSamples", temp);
                snprintf(temp, sizeof(temp), "%lu", (unsigned long)samplesPerHiCycle);
                AddAttribute(node_data, "samplesPerHiCycle", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)samplesPerSec);
                AddAttribute(node_data, "samplesPerSec", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)ctOctave);
                AddAttribute(node_data, "ctOctave", temp);

                switch (sCompression)
                {
                    case 0: AddEnumAttribute(node_data, "sCompression", "none", sCompression); break;
                    case 1: AddEnumAttribute(node_data, "sCompression", "FIBDELTA", sCompression); break;
                    default: AddEnumAttribute(node_data, "sCompression", NULL, sCompression); break;
                }

                snprintf(temp, sizeof(temp), "%ld", (long)volume);
                AddAttribute(node_data, "volume", temp);
            }

            break;
        }
        case MAKE_ID('C', 'H', 'A', 'N'):
        {
            char temp[32];

            if (chunk_size < 2)
                return FALSE;

            if (Read(fileIFF, buffer, 2) != 2)
                return FALSE;

            consumed = 2;

            snprintf(temp, sizeof(temp), "%u", (unsigned int)((buffer[0] << 8) | buffer[1]));
            AddAttribute(node_data, "channels", temp);
            break;
        }
        case MAKE_ID('N', 'A', 'M', 'E'):
        case MAKE_ID('A', 'U', 'T', 'H'):
        case MAKE_ID('C', 'O', 'P', 'R'):
        case MAKE_ID('(', 'c', ')', ' '):
        {
            ULONG name_len = chunk_size;
            ULONG read_len = name_len < sizeof(buffer) ? name_len : sizeof(buffer) - 1;

            if (read_len > 0)
            {
                if (Read(fileIFF, buffer, read_len) != (LONG)read_len)
                    return FALSE;

                consumed = read_len;
                buffer[read_len] = '\0';
                AddAttribute(node_data, "text", (const char *)buffer);

                if (name_len > read_len)
                {
                    if (!SkipBytes(fileIFF, name_len - read_len))
                        return FALSE;
                    consumed += name_len - read_len;
                }
            }
            else
            {
                if (chunk_size > 0 && !SkipBytes(fileIFF, chunk_size))
                    return FALSE;
                consumed = chunk_size;
            }

            break;
        }
        case MAKE_ID('S', 'T', 'R', ' '):
        case MAKE_ID('C', 'M', 'N', 'T'):
        case MAKE_ID('A', 'N', 'N', 'O'):
        case MAKE_ID('A', 'C', 'C', 'N'):
        case MAKE_ID('F', 'I', 'L', 'E'):
        {
            ULONG read_len = chunk_size < sizeof(buffer) ? chunk_size : sizeof(buffer) - 1;

            if (read_len > 0)
            {
                if (Read(fileIFF, buffer, read_len) != (LONG)read_len)
                    return FALSE;
                consumed = read_len;
                buffer[read_len] = '\0';
                AddAttribute(node_data, "text", (const char *)buffer);

                if (chunk_size > read_len)
                {
                    if (!SkipBytes(fileIFF, chunk_size - read_len))
                        return FALSE;
                    consumed += chunk_size - read_len;
                }
            }
            else
            {
                if (chunk_size > 0 && !SkipBytes(fileIFF, chunk_size))
                    return FALSE;
                consumed = chunk_size;
            }

            break;
        }
        case MAKE_ID('S', 'M', 'U', 'S'):
        {
            const ULONG smus_size = 38;

            if (chunk_size < smus_size)
                return FALSE;

            if (Read(fileIFF, buffer, smus_size) != (LONG)smus_size)
                return FALSE;

            consumed = smus_size;

            {
                char temp[64];
                UBYTE instrumentType = buffer[0];
                UBYTE padding = buffer[1];
                UWORD precedence = (UWORD)ReadBE16Attr(buffer + 2);
                UBYTE basicTempo = buffer[4];
                UBYTE basicVolume = buffer[5];
                UBYTE basicTempoFraction = buffer[6];
                UBYTE basicVolumeFraction = buffer[7];

                snprintf(temp, sizeof(temp), "%u", (unsigned int)instrumentType);
                AddAttribute(node_data, "instrumentType", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)padding);
                AddAttribute(node_data, "padding", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)precedence);
                AddAttribute(node_data, "precedence", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)basicTempo);
                AddAttribute(node_data, "basicTempo", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)basicVolume);
                AddAttribute(node_data, "basicVolume", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)basicTempoFraction);
                AddAttribute(node_data, "basicTempoFraction", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)basicVolumeFraction);
                AddAttribute(node_data, "basicVolumeFraction", temp);
            }

            if (chunk_size > smus_size)
            {
                ULONG remaining = chunk_size - smus_size;
                ULONG skip_len = remaining < sizeof(buffer) ? remaining : sizeof(buffer);

                if (skip_len > 0)
                {
                    if (Read(fileIFF, buffer, skip_len) != (LONG)skip_len)
                        return FALSE;

                    consumed += skip_len;
                    remaining -= skip_len;

                    if (skip_len >= 2)
                    {
                        char temp[64];
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)((buffer[0] << 8) | buffer[1]));
                        AddAttribute(node_data, "firstInstrument", temp);
                    }
                }

                if (remaining > 0)
                {
                    if (!SkipBytes(fileIFF, remaining))
                        return FALSE;
                    consumed += remaining;
                }
            }

            break;
        }
        case MAKE_ID('F', 'A', 'N', 'T'):
        {
            const ULONG fant_header = 86;

            if (chunk_size < fant_header)
                return FALSE;

            if (Read(fileIFF, buffer, fant_header) != (LONG)fant_header)
                return FALSE;

            consumed = fant_header;

            {
                char temp[64];
                UWORD pointsPerObj = (UWORD)ReadBE16Attr(buffer);
                UWORD objsPerFrame = (UWORD)ReadBE16Attr(buffer + 2);
                UWORD screenDepth = (UWORD)ReadBE16Attr(buffer + 4);
                UWORD screenWidth = (UWORD)ReadBE16Attr(buffer + 6);
                UWORD screenHeight = (UWORD)ReadBE16Attr(buffer + 8);
                UWORD backColor = (UWORD)ReadBE16Attr(buffer + 10);
                ULONG sizeOfMovie = ReadBE32Attr(buffer + 12);
                UWORD numberOfFrames = (UWORD)ReadBE16Attr(buffer + 76);
                UWORD numberOfSounds = (UWORD)ReadBE16Attr(buffer + 78);
                UWORD numberOfBitMaps = (UWORD)ReadBE16Attr(buffer + 80);
                UWORD background = (UWORD)ReadBE16Attr(buffer + 82);
                UWORD speedOfMovie = (UWORD)ReadBE16Attr(buffer + 84);

                snprintf(temp, sizeof(temp), "%u", (unsigned int)pointsPerObj);
                AddAttribute(node_data, "pointsPerObj", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)objsPerFrame);
                AddAttribute(node_data, "objsPerFrame", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)screenDepth);
                AddAttribute(node_data, "screenDepth", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)screenWidth);
                AddAttribute(node_data, "screenWidth", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)screenHeight);
                AddAttribute(node_data, "screenHeight", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)backColor);
                AddAttribute(node_data, "backColor", temp);
                snprintf(temp, sizeof(temp), "%lu", (unsigned long)sizeOfMovie);
                AddAttribute(node_data, "sizeOfMovie", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)numberOfFrames);
                AddAttribute(node_data, "numberOfFrames", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)numberOfSounds);
                AddAttribute(node_data, "numberOfSounds", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)numberOfBitMaps);
                AddAttribute(node_data, "numberOfBitMaps", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)background);
                AddAttribute(node_data, "background", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)speedOfMovie);
                AddAttribute(node_data, "speedOfMovie", temp);
            }

            if (chunk_size > fant_header)
            {
                ULONG remaining = chunk_size - fant_header;
                ULONG read_len = remaining < sizeof(buffer) ? remaining : sizeof(buffer);

                if (read_len > 0)
                {
                    if (Read(fileIFF, buffer, read_len) != (LONG)read_len)
                        return FALSE;

                    consumed += read_len;
                    remaining -= read_len;

                    if (read_len >= 6)
                    {
                        char temp[64];
                        UWORD opcode = (UWORD)ReadBE16Attr(buffer);
                        ULONG parm = ReadBE32Attr(buffer + 2);

                        snprintf(temp, sizeof(temp), "%u", (unsigned int)opcode);
                        AddAttribute(node_data, "opcode", temp);
                        snprintf(temp, sizeof(temp), "%lu", (unsigned long)parm);
                        AddAttribute(node_data, "parm", temp);
                    }

                    if (read_len >= 10)
                    {
                        char temp[64];
                        UWORD tweenRate = (UWORD)ReadBE16Attr(buffer + 8);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)tweenRate);
                        AddAttribute(node_data, "tweenRate", temp);
                    }

                    if (read_len >= 14)
                    {
                        char temp[64];
                        WORD channel0 = (WORD)ReadBE16SAttr(buffer + 10);
                        WORD channel1 = (WORD)ReadBE16SAttr(buffer + 12);
                        snprintf(temp, sizeof(temp), "%d", (int)channel0);
                        AddAttribute(node_data, "channel0", temp);
                        snprintf(temp, sizeof(temp), "%d", (int)channel1);
                        AddAttribute(node_data, "channel1", temp);
                    }

                    if (read_len >= 16)
                    {
                        char temp[64];
                        UWORD numberOfPolys = (UWORD)ReadBE16Attr(buffer + 14);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)numberOfPolys);
                        AddAttribute(node_data, "numberOfPolys", temp);
                    }

                    if (read_len >= 86)
                    {
                        char temp[64];
                        WORD pan = (WORD)ReadBE16SAttr(buffer + 80);
                        WORD tilt = (WORD)ReadBE16SAttr(buffer + 82);
                        UWORD modes = (UWORD)ReadBE16Attr(buffer + 84);
                        snprintf(temp, sizeof(temp), "%d", (int)pan);
                        AddAttribute(node_data, "pan", temp);
                        snprintf(temp, sizeof(temp), "%d", (int)tilt);
                        AddAttribute(node_data, "tilt", temp);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)modes);
                        AddAttribute(node_data, "modes", temp);
                    }
                }

                if (remaining > 0)
                {
                    if (!SkipBytes(fileIFF, remaining))
                        return FALSE;
                    consumed += remaining;
                }
            }

            break;
        }
        case MAKE_ID('P', 'O', 'L', 'Y'):
        {
            const ULONG poly_header = 30;

            if (chunk_size < poly_header)
                return FALSE;

            if (Read(fileIFF, buffer, poly_header) != (LONG)poly_header)
                return FALSE;

            consumed = poly_header;

            {
                char temp[64];
                UWORD numberOfPoints = (UWORD)ReadBE16Attr(buffer);
                UWORD type = (UWORD)ReadBE16Attr(buffer + 2);
                UWORD color = (UWORD)ReadBE16Attr(buffer + 4);
                WORD left = (WORD)ReadBE16SAttr(buffer + 6);
                WORD top = (WORD)ReadBE16SAttr(buffer + 8);
                WORD right = (WORD)ReadBE16SAttr(buffer + 10);
                WORD bottom = (WORD)ReadBE16SAttr(buffer + 12);
                WORD depth = (WORD)ReadBE16SAttr(buffer + 14);
                UBYTE action = buffer[16];
                UBYTE priority = buffer[17];
                UWORD outlineColor = (UWORD)ReadBE16Attr(buffer + 18);
                WORD bitmapIndex = (WORD)ReadBE16SAttr(buffer + 20);
                UWORD realWidth = (UWORD)ReadBE16Attr(buffer + 22);
                UWORD realHeight = (UWORD)ReadBE16Attr(buffer + 24);
                UWORD textLength = (UWORD)ReadBE16Attr(buffer + 26);
                UWORD textJust = (UWORD)ReadBE16Attr(buffer + 28);

                snprintf(temp, sizeof(temp), "%u", (unsigned int)numberOfPoints);
                AddAttribute(node_data, "numberOfPoints", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)type);
                AddAttribute(node_data, "type", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)color);
                AddAttribute(node_data, "color", temp);
                snprintf(temp, sizeof(temp), "%d", (int)left);
                AddAttribute(node_data, "left", temp);
                snprintf(temp, sizeof(temp), "%d", (int)top);
                AddAttribute(node_data, "top", temp);
                snprintf(temp, sizeof(temp), "%d", (int)right);
                AddAttribute(node_data, "right", temp);
                snprintf(temp, sizeof(temp), "%d", (int)bottom);
                AddAttribute(node_data, "bottom", temp);
                snprintf(temp, sizeof(temp), "%d", (int)depth);
                AddAttribute(node_data, "depth", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)action);
                AddAttribute(node_data, "action", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)priority);
                AddAttribute(node_data, "priority", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)outlineColor);
                AddAttribute(node_data, "outlineColor", temp);
                snprintf(temp, sizeof(temp), "%d", (int)bitmapIndex);
                AddAttribute(node_data, "bitmapIndex", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)realWidth);
                AddAttribute(node_data, "realWidth", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)realHeight);
                AddAttribute(node_data, "realHeight", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)textLength);
                AddAttribute(node_data, "textLength", temp);
                snprintf(temp, sizeof(temp), "%u", (unsigned int)textJust);
                AddAttribute(node_data, "textJust", temp);
            }

            if (chunk_size > poly_header)
            {
                ULONG remaining = chunk_size - poly_header;
                ULONG read_len = remaining < sizeof(buffer) ? remaining : sizeof(buffer);

                if (read_len > 0)
                {
                    if (Read(fileIFF, buffer, read_len) != (LONG)read_len)
                        return FALSE;

                    consumed += read_len;
                    remaining -= read_len;

                    if (read_len > 0)
                    {
                        char temp[64];
                        UBYTE textID = buffer[0];

                        snprintf(temp, sizeof(temp), "%u", (unsigned int)textID);
                        AddAttribute(node_data, "textID", temp);
                    }

                    if (read_len > 2)
                    {
                        char temp[64];
                        UWORD textDelay = (UWORD)ReadBE16Attr(buffer + 2);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)textDelay);
                        AddAttribute(node_data, "textDelay", temp);
                    }

                    if (read_len > 4)
                    {
                        UBYTE sound = buffer[4];
                        char temp[64];
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)sound);
                        AddAttribute(node_data, "sound", temp);
                    }

                    if (read_len > 5)
                    {
                        char temp[64];
                        UBYTE soundChannel = buffer[5];
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)soundChannel);
                        AddAttribute(node_data, "soundChannel", temp);
                    }

                    if (read_len > 6)
                    {
                        char temp[64];
                        UBYTE polysInThisFrame = buffer[6];
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)polysInThisFrame);
                        AddAttribute(node_data, "polysInThisFrame", temp);
                    }

                    if (read_len > 8)
                    {
                        char temp[64];
                        UWORD polysID = (UWORD)ReadBE16Attr(buffer + 7);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)polysID);
                        AddAttribute(node_data, "polysID", temp);
                    }

                    if (read_len > 9)
                    {
                        char temp[64];
                        UWORD polysLoops = (UWORD)ReadBE16Attr(buffer + 9);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)polysLoops);
                        AddAttribute(node_data, "polysLoops", temp);
                    }

                    if (read_len > 11)
                    {
                        char temp[64];
                        UWORD polysJoints = (UWORD)ReadBE16Attr(buffer + 11);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)polysJoints);
                        AddAttribute(node_data, "polysJoints", temp);
                    }

                    if (read_len > 13)
                    {
                        char temp[64];
                        UWORD polysSpeed = (UWORD)ReadBE16Attr(buffer + 13);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)polysSpeed);
                        AddAttribute(node_data, "polysSpeed", temp);
                    }

                    if (read_len > 15)
                    {
                        char temp[64];
                        WORD abs_x = (WORD)ReadBE16SAttr(buffer + 15);
                        WORD abs_y = (WORD)ReadBE16SAttr(buffer + 17);
                        snprintf(temp, sizeof(temp), "%d", (int)abs_x);
                        AddAttribute(node_data, "abs_x", temp);
                        snprintf(temp, sizeof(temp), "%d", (int)abs_y);
                        AddAttribute(node_data, "abs_y", temp);
                    }

                    if (read_len > 23)
                    {
                        char temp[64];
                        WORD rel_x = (WORD)ReadBE16SAttr(buffer + 19);
                        WORD rel_y = (WORD)ReadBE16SAttr(buffer + 21);
                        snprintf(temp, sizeof(temp), "%d", (int)rel_x);
                        AddAttribute(node_data, "rel_x", temp);
                        snprintf(temp, sizeof(temp), "%d", (int)rel_y);
                        AddAttribute(node_data, "rel_y", temp);
                    }

                    if (read_len > 26)
                    {
                        char temp[64];
                        UBYTE operation = buffer[23];
                        switch (operation)
                        {
                            case 0: AddEnumAttribute(node_data, "operation", "copy", operation); break;
                            case 1: AddEnumAttribute(node_data, "operation", "fast", operation); break;
                            case 2: AddEnumAttribute(node_data, "operation", "phant", operation); break;
                            default: AddEnumAttribute(node_data, "operation", NULL, operation); break;
                        }

                        snprintf(temp, sizeof(temp), "%u", (unsigned int)buffer[24]);
                        AddAttribute(node_data, "polyPick", temp);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)buffer[25]);
                        AddAttribute(node_data, "polyOnOff", temp);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)buffer[26]);
                        AddAttribute(node_data, "polyMask", temp);
                    }

                    if (read_len > 29)
                    {
                        char temp[64];
                        UBYTE planePick = buffer[27];
                        UBYTE planeOnOff = buffer[28];
                        UBYTE planeMask = buffer[29];
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)planePick);
                        AddAttribute(node_data, "planePick", temp);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)planeOnOff);
                        AddAttribute(node_data, "planeOnOff", temp);
                        snprintf(temp, sizeof(temp), "%u", (unsigned int)planeMask);
                        AddAttribute(node_data, "planeMask", temp);
                    }
                }

                if (remaining > 0)
                {
                    if (!SkipBytes(fileIFF, remaining))
                        return FALSE;
                    consumed += remaining;
                }
            }

            break;
        }
        case MAKE_ID('C', 'S', 'T', 'R'):
        {
            ULONG read_len = chunk_size < sizeof(buffer) ? chunk_size : sizeof(buffer) - 1;

            if (read_len > 0)
            {
                if (Read(fileIFF, buffer, read_len) != (LONG)read_len)
                    return FALSE;
                consumed = read_len;
                buffer[read_len] = '\0';
                AddAttribute(node_data, "text", (const char *)buffer);

                if (chunk_size > read_len)
                {
                    if (!SkipBytes(fileIFF, chunk_size - read_len))
                        return FALSE;
                    consumed += chunk_size - read_len;
                }
            }
            else
            {
                if (chunk_size > 0 && !SkipBytes(fileIFF, chunk_size))
                    return FALSE;
                consumed = chunk_size;
            }

            break;
        }
        default:
        {
            if (!SkipBytes(fileIFF, chunk_size))
                return FALSE;
            consumed = chunk_size;
            break;
        }
    }

    if (consumed < chunk_size)
    {
        ULONG remaining = chunk_size - consumed;
        if (!SkipBytes(fileIFF, remaining))
            return FALSE;
    }

    return TRUE;
}
