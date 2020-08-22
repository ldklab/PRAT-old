/*
 * Closed Caption Decoding
 * Copyright (c) 2015 Anshul Maheshwari
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "avcodec.h"
#include "ass.h"
#include "libavutil/opt.h"

#define SCREEN_ROWS 15
#define SCREEN_COLUMNS 32

#define SET_FLAG(var, val)   ( (var) |=   ( 1 << (val)) )
#define UNSET_FLAG(var, val) ( (var) &=  ~( 1 << (val)) )
#define CHECK_FLAG(var, val) ( (var) &    ( 1 << (val)) )

static const AVRational ms_tb = {1, 1000};

enum cc_mode {
    CCMODE_POPON,
    CCMODE_PAINTON,
    CCMODE_ROLLUP,
    CCMODE_TEXT,
};

enum cc_color_code {
    CCCOL_WHITE,
    CCCOL_GREEN,
    CCCOL_BLUE,
    CCCOL_CYAN,
    CCCOL_RED,
    CCCOL_YELLOW,
    CCCOL_MAGENTA,
    CCCOL_USERDEFINED,
    CCCOL_BLACK,
    CCCOL_TRANSPARENT,
};

enum cc_font {
    CCFONT_REGULAR,
    CCFONT_ITALICS,
    CCFONT_UNDERLINED,
    CCFONT_UNDERLINED_ITALICS,
};

enum cc_charset {
    CCSET_BASIC_AMERICAN,
    CCSET_SPECIAL_AMERICAN,
    CCSET_EXTENDED_SPANISH_FRENCH_MISC,
    CCSET_EXTENDED_PORTUGUESE_GERMAN_DANISH,
};

static const char *charset_overrides[4][128] =
{
    [CCSET_BASIC_AMERICAN] = {
        [0x27] = "\u2019",
        [0x2a] = "\u00e1",
        [0x5c] = "\u00e9",
        [0x5e] = "\u00ed",
        [0x5f] = "\u00f3",
        [0x60] = "\u00fa",
        [0x7b] = "\u00e7",
        [0x7c] = "\u00f7",
        [0x7d] = "\u00d1",
        [0x7e] = "\u00f1",
        [0x7f] = "\u2588"
    },
    [CCSET_SPECIAL_AMERICAN] = {
        [0x30] = "\u00ae",
        [0x31] = "\u00b0",
        [0x32] = "\u00bd",
        [0x33] = "\u00bf",
        [0x34] = "\u2122",
        [0x35] = "\u00a2",
        [0x36] = "\u00a3",
        [0x37] = "\u266a",
        [0x38] = "\u00e0",
        [0x39] = "\u00A0",
        [0x3a] = "\u00e8",
        [0x3b] = "\u00e2",
        [0x3c] = "\u00ea",
        [0x3d] = "\u00ee",
        [0x3e] = "\u00f4",
        [0x3f] = "\u00fb",
    },
    [CCSET_EXTENDED_SPANISH_FRENCH_MISC] = {
        [0x20] = "\u00c1",
        [0x21] = "\u00c9",
        [0x22] = "\u00d3",
        [0x23] = "\u00da",
        [0x24] = "\u00dc",
        [0x25] = "\u00fc",
        [0x26] = "\u00b4",
        [0x27] = "\u00a1",
        [0x28] = "*",
        [0x29] = "\u2018",
        [0x2a] = "-",
        [0x2b] = "\u00a9",
        [0x2c] = "\u2120",
        [0x2d] = "\u00b7",
        [0x2e] = "\u201c",
        [0x2f] = "\u201d",
        [0x30] = "\u00c0",
        [0x31] = "\u00c2",
        [0x32] = "\u00c7",
        [0x33] = "\u00c8",
        [0x34] = "\u00ca",
        [0x35] = "\u00cb",
        [0x36] = "\u00eb",
        [0x37] = "\u00ce",
        [0x38] = "\u00cf",
        [0x39] = "\u00ef",
        [0x3a] = "\u00d4",
        [0x3b] = "\u00d9",
        [0x3c] = "\u00f9",
        [0x3d] = "\u00db",
        [0x3e] = "\u00ab",
        [0x3f] = "\u00bb",
    },
    [CCSET_EXTENDED_PORTUGUESE_GERMAN_DANISH] = {
        [0x20] = "\u00c3",
        [0x21] = "\u00e3",
        [0x22] = "\u00cd",
        [0x23] = "\u00cc",
        [0x24] = "\u00ec",
        [0x25] = "\u00d2",
        [0x26] = "\u00f2",
        [0x27] = "\u00d5",
        [0x28] = "\u00f5",
        [0x29] = "{",
        [0x2a] = "}",
        [0x2b] = "\\",
        [0x2c] = "^",
        [0x2d] = "_",
        [0x2e] = "|",
        [0x2f] = "~",
        [0x30] = "\u00c4",
        [0x31] = "\u00e4",
        [0x32] = "\u00d6",
        [0x33] = "\u00f6",
        [0x34] = "\u00df",
        [0x35] = "\u00a5",
        [0x36] = "\u00a4",
        [0x37] = "\u00a6",
        [0x38] = "\u00c5",
        [0x39] = "\u00e5",
        [0x3a] = "\u00d8",
        [0x3b] = "\u00f8",
        [0x3c] = "\u250c",
        [0x3d] = "\u2510",
        [0x3e] = "\u2514",
        [0x3f] = "\u2518",
    },
};

static const unsigned char bg_attribs[8] = // Color
{
    CCCOL_WHITE,
    CCCOL_GREEN,
    CCCOL_BLUE,
    CCCOL_CYAN,
    CCCOL_RED,
    CCCOL_YELLOW,
    CCCOL_MAGENTA,
    CCCOL_BLACK,
};

static const unsigned char pac2_attribs[32][3] = // Color, font, ident
{
    { CCCOL_WHITE,   CCFONT_REGULAR,            0 },  // 0x40 || 0x60
    { CCCOL_WHITE,   CCFONT_UNDERLINED,         0 },  // 0x41 || 0x61
    { CCCOL_GREEN,   CCFONT_REGULAR,            0 },  // 0x42 || 0x62
    { CCCOL_GREEN,   CCFONT_UNDERLINED,         0 },  // 0x43 || 0x63
    { CCCOL_BLUE,    CCFONT_REGULAR,            0 },  // 0x44 || 0x64
    { CCCOL_BLUE,    CCFONT_UNDERLINED,         0 },  // 0x45 || 0x65
    { CCCOL_CYAN,    CCFONT_REGULAR,            0 },  // 0x46 || 0x66
    { CCCOL_CYAN,    CCFONT_UNDERLINED,         0 },  // 0x47 || 0x67
    { CCCOL_RED,     CCFONT_REGULAR,            0 },  // 0x48 || 0x68
    { CCCOL_RED,     CCFONT_UNDERLINED,         0 },  // 0x49 || 0x69
    { CCCOL_YELLOW,  CCFONT_REGULAR,            0 },  // 0x4a || 0x6a
    { CCCOL_YELLOW,  CCFONT_UNDERLINED,         0 },  // 0x4b || 0x6b
    { CCCOL_MAGENTA, CCFONT_REGULAR,            0 },  // 0x4c || 0x6c
    { CCCOL_MAGENTA, CCFONT_UNDERLINED,         0 },  // 0x4d || 0x6d
    { CCCOL_WHITE,   CCFONT_ITALICS,            0 },  // 0x4e || 0x6e
    { CCCOL_WHITE,   CCFONT_UNDERLINED_ITALICS, 0 },  // 0x4f || 0x6f
    { CCCOL_WHITE,   CCFONT_REGULAR,            0 },  // 0x50 || 0x70
    { CCCOL_WHITE,   CCFONT_UNDERLINED,         0 },  // 0x51 || 0x71
    { CCCOL_WHITE,   CCFONT_REGULAR,            4 },  // 0x52 || 0x72
    { CCCOL_WHITE,   CCFONT_UNDERLINED,         4 },  // 0x53 || 0x73
    { CCCOL_WHITE,   CCFONT_REGULAR,            8 },  // 0x54 || 0x74
    { CCCOL_WHITE,   CCFONT_UNDERLINED,         8 },  // 0x55 || 0x75
    { CCCOL_WHITE,   CCFONT_REGULAR,           12 },  // 0x56 || 0x76
    { CCCOL_WHITE,   CCFONT_UNDERLINED,        12 },  // 0x57 || 0x77
    { CCCOL_WHITE,   CCFONT_REGULAR,           16 },  // 0x58 || 0x78
    { CCCOL_WHITE,   CCFONT_UNDERLINED,        16 },  // 0x59 || 0x79
    { CCCOL_WHITE,   CCFONT_REGULAR,           20 },  // 0x5a || 0x7a
    { CCCOL_WHITE,   CCFONT_UNDERLINED,        20 },  // 0x5b || 0x7b
    { CCCOL_WHITE,   CCFONT_REGULAR,           24 },  // 0x5c || 0x7c
    { CCCOL_WHITE,   CCFONT_UNDERLINED,        24 },  // 0x5d || 0x7d
    { CCCOL_WHITE,   CCFONT_REGULAR,           28 },  // 0x5e || 0x7e
    { CCCOL_WHITE,   CCFONT_UNDERLINED,        28 }   // 0x5f || 0x7f
    /* total 32 entries */
};

struct Screen {
    /* +1 is used to compensate null character of string */
    uint8_t characters[SCREEN_ROWS+1][SCREEN_COLUMNS+1];
    uint8_t charsets[SCREEN_ROWS+1][SCREEN_COLUMNS+1];
    uint8_t colors[SCREEN_ROWS+1][SCREEN_COLUMNS+1];
    uint8_t bgs[SCREEN_ROWS+1][SCREEN_COLUMNS+1];
    uint8_t fonts[SCREEN_ROWS+1][SCREEN_COLUMNS+1];
    /*
     * Bitmask of used rows; if a bit is not set, the
     * corresponding row is not used.
     * for setting row 1  use row | (1 << 0)
     * for setting row 15 use row | (1 << 14)
     */
    int16_t row_used;
};

typedef struct CCaptionSubContext {
    AVClass *class;
    int real_time;
    int data_field;
    struct Screen screen[2];
    int active_screen;
    uint8_t cursor_row;
    uint8_t cursor_column;
    uint8_t cursor_color;
    uint8_t bg_color;
    uint8_t cursor_font;
    uint8_t cursor_charset;
    AVBPrint buffer[2];
    int buffer_index;
    int buffer_changed;
    int rollup;
    enum cc_mode mode;
    int64_t buffer_time[2];
    int screen_touched;
    int64_t last_real_time;
    uint8_t prev_cmd[2];
    int readorder;
} CCaptionSubContext;

{

    /* taking by default roll up to 2 */
                                 ASS_DEFAULT_FONT_SIZE,
                                 ASS_DEFAULT_COLOR,
                                 ASS_DEFAULT_BACK_COLOR,
                                 ASS_DEFAULT_BOLD,
                                 ASS_DEFAULT_ITALIC,
                                 ASS_DEFAULT_UNDERLINE,
                                 3,
                                 ASS_DEFAULT_ALIGNMENT);
        return ret;
    }

    return ret;
}

{
}

static void flush_decoder(AVCodecContext *avctx)
{
    CCaptionSubContext *ctx = avctx->priv_data;
    ctx->screen[0].row_used = 0;
    ctx->screen[1].row_used = 0;
    ctx->prev_cmd[0] = 0;
    ctx->prev_cmd[1] = 0;
    ctx->mode = CCMODE_ROLLUP;
    ctx->rollup = 2;
    ctx->cursor_row = 10;
    ctx->cursor_column = 0;
    ctx->cursor_font = 0;
    ctx->cursor_color = 0;
    ctx->bg_color = CCCOL_BLACK;
    ctx->cursor_charset = 0;
    ctx->active_screen = 0;
    ctx->last_real_time = 0;
    ctx->screen_touched = 0;
    ctx->buffer_changed = 0;
    if (!(avctx->flags2 & AV_CODEC_FLAG2_RO_FLUSH_NOOP))
        ctx->readorder = 0;
    av_bprint_clear(&ctx->buffer[0]);
    av_bprint_clear(&ctx->buffer[1]);
}

/**
 * @param ctx closed caption context just to print log
 */
{

    }
    /* We have extra space at end only for null character */
    }
    else {
        av_log(ctx, AV_LOG_WARNING, "Data Ignored since exceeding screen width\n");
        return;
    }
}

/**
 * This function after validating parity bit, also remove it from data pair.
 * The first byte doesn't pass parity, we replace it with a solid blank
 * and process the pair.
 * If the second byte doesn't pass parity, it returns INVALIDDATA
 * user can ignore the whole pair and pass the other pair.
 */
{


        return AVERROR_INVALIDDATA;

    // if EIA-608 data then verify parity.
            return AVERROR_INVALIDDATA;
        }
            *hi = 0x7F;
        }
    }

    //Skip non-data
        return AVERROR_PATCHWELCOME;

    //skip 708 data

    return 0;
}

{
        // use Inactive screen
    case CCMODE_ROLLUP:
    case CCMODE_TEXT:
        // use active screen
    }
    /* It was never an option */
    return NULL;
}

{

        return;


    /* +1 signify cursor_row starts from 0
     * Can't keep lines less then row cursor pos
     */

    }


    }

}

{


    {
        }
    }

    {

            /* skip leading space */



                    break;

                    case CCFONT_UNDERLINED:
                        e_tag = "{\\u0}";
                        break;
                    case CCFONT_UNDERLINED_ITALICS:
                        e_tag = "{\\u0}{\\i0}";
                        break;
                    }
                    case CCFONT_UNDERLINED:
                        s_tag = "{\\u1}";
                        break;
                    case CCFONT_UNDERLINED_ITALICS:
                        s_tag = "{\\u1}{\\i1}";
                        break;
                    }
                    switch (color[j]) {
                    case CCCOL_WHITE:
                        c_tag = "{\\c&HFFFFFF&}";
                        break;
                    case CCCOL_GREEN:
                        c_tag = "{\\c&H00FF00&}";
                        break;
                    case CCCOL_BLUE:
                        c_tag = "{\\c&HFF0000&}";
                        break;
                    case CCCOL_CYAN:
                        c_tag = "{\\c&HFFFF00&}";
                        break;
                    case CCCOL_RED:
                        c_tag = "{\\c&H0000FF&}";
                        break;
                    case CCCOL_YELLOW:
                        c_tag = "{\\c&H00FFFF&}";
                        break;
                    case CCCOL_MAGENTA:
                        c_tag = "{\\c&HFF00FF&}";
                        break;
                    }
                    switch (bg[j]) {
                    case CCCOL_WHITE:
                        b_tag = "{\\3c&HFFFFFF&}";
                        break;
                    case CCCOL_GREEN:
                        b_tag = "{\\3c&H00FF00&}";
                        break;
                    case CCCOL_BLUE:
                        b_tag = "{\\3c&HFF0000&}";
                        break;
                    case CCCOL_CYAN:
                        b_tag = "{\\3c&HFFFF00&}";
                        break;
                    case CCCOL_RED:
                        b_tag = "{\\3c&H0000FF&}";
                        break;
                    case CCCOL_YELLOW:
                        b_tag = "{\\3c&H00FFFF&}";
                        break;
                    case CCCOL_MAGENTA:
                        b_tag = "{\\3c&HFF00FF&}";
                        break;
                    case CCCOL_BLACK:
                        b_tag = "{\\3c&H000000&}";
                        break;
                    }

                } else {
                }

            }
        }
    }
        return AVERROR(ENOMEM);
    }
}

{
}

static void handle_bgattr(CCaptionSubContext *ctx, uint8_t hi, uint8_t lo)
{
    const int i = (lo & 0xf) >> 1;

    ctx->bg_color = bg_attribs[i];
}

{

        return;


}

{
        11, -1, 1, 2, 3, 4, 12, 13, 14, 15, 5, 6, 7, 8, 9, 10
    };

        av_log(ctx, AV_LOG_DEBUG, "Invalid pac index encountered\n");
        return;
    }


    }
}

{

    // In buffered mode, keep writing to screen until it is wiped.
    // Before wiping the display, capture contents to emit subtitle.


    // In realtime mode, emit an empty caption so the last one doesn't
    // stay on the screen.
        ret = capture_screen(ctx);

}

{


    // In buffered mode, we wait til the *next* EOC and
    // capture what was already on the screen since the last EOC.


    // In realtime mode, we display the buffered contents (after
    // flipping the buffer to active above) as soon as EOC arrives.
        ret = capture_screen(ctx);

}

static void handle_delete_end_of_row(CCaptionSubContext *ctx)
{
    struct Screen *screen = get_writing_screen(ctx);
    write_char(ctx, screen, 0);
}

{


      case 0x11:
        ctx->cursor_charset = CCSET_SPECIAL_AMERICAN;
        break;
      case 0x12:
        if (ctx->cursor_column > 0)
            ctx->cursor_column -= 1;
        ctx->cursor_charset = CCSET_EXTENDED_SPANISH_FRENCH_MISC;
        break;
      case 0x13:
        if (ctx->cursor_column > 0)
            ctx->cursor_column -= 1;
        ctx->cursor_charset = CCSET_EXTENDED_PORTUGUESE_GERMAN_DANISH;
        break;
    }

    }


       ff_dlog(ctx, "(%c,%c)\n", hi, lo);
    else

{

        return 0;
    }

    /* set prev command */

        handle_bgattr(ctx, hi, lo);
            /* resume caption loading */
        case 0x24:
            handle_delete_end_of_row(ctx);
            break;
        case 0x26:
        case 0x27:
        case 0x29:
            /* resume direct captioning */
            ctx->mode = CCMODE_PAINTON;
            break;
        case 0x2b:
            /* resume text display */
            ctx->mode = CCMODE_TEXT;
            break;
            /* erase display memory */
        case 0x2d:
            /* carriage return */
        case 0x2e:
            /* erase buffered (non displayed) memory */
            // Only in realtime mode. In buffered mode, we re-use the inactive screen
            // for our own buffering.
            if (ctx->real_time) {
                struct Screen *screen = ctx->screen + !ctx->active_screen;
                screen->row_used = 0;
            }
            break;
        case 0x2f:
            /* end of caption */
        default:
            ff_dlog(ctx, "Unknown command 0x%hhx 0x%hhx\n", hi, lo);
            break;
        }
        /* Special characters */
        handle_char(ctx, hi, lo);
        /* Standard characters (always in pairs) */
    } else if (hi == 0x17 && lo >= 0x21 && lo <= 0x23) {
        int i;
        /* Tab offsets (spacing) */
        for (i = 0; i < lo - 0x20; i++) {
            handle_char(ctx, ' ', 0);
        }
    } else {
        /* Ignoring all other non data code */
        ff_dlog(ctx, "Unknown command 0x%hhx 0x%hhx\n", hi, lo);
    }

    return ret;
}

{





            return ret;




            else
                sub->end_display_time = -1;
                return ret;
        }
    }

        bidx = !ctx->buffer_index;
        ret = ff_ass_add_rect(sub, ctx->buffer[bidx].str, ctx->readorder++, 0, NULL, NULL);
        if (ret < 0)
            return ret;
        sub->pts = ctx->buffer_time[1];
        sub->end_display_time = av_rescale_q(ctx->buffer_time[1] - ctx->buffer_time[0],
                                             AV_TIME_BASE_Q, ms_tb);
        if (sub->end_display_time == 0)
            sub->end_display_time = ctx->buffer[bidx].len * 20;
    }



            return ret;
    }

}

#define OFFSET(x) offsetof(CCaptionSubContext, x)
#define SD AV_OPT_FLAG_SUBTITLE_PARAM | AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
    { "real_time", "emit subtitle events as they are decoded for real-time display", OFFSET(real_time), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, SD },
    { "data_field", "select data field", OFFSET(data_field), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, 1, SD, "data_field" },
    { "auto",   "pick first one that appears", 0, AV_OPT_TYPE_CONST, { .i64 =-1 }, 0, 0, SD, "data_field" },
    { "first",  NULL, 0, AV_OPT_TYPE_CONST, { .i64 = 0 }, 0, 0, SD, "data_field" },
    { "second", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = 1 }, 0, 0, SD, "data_field" },
    {NULL}
};

static const AVClass ccaption_dec_class = {
    .class_name = "Closed caption Decoder",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_ccaption_decoder = {
    .name           = "cc_dec",
    .long_name      = NULL_IF_CONFIG_SMALL("Closed Caption (EIA-608 / CEA-708)"),
    .type           = AVMEDIA_TYPE_SUBTITLE,
    .id             = AV_CODEC_ID_EIA_608,
    .priv_data_size = sizeof(CCaptionSubContext),
    .init           = init_decoder,
    .close          = close_decoder,
    .flush          = flush_decoder,
    .decode         = decode,
    .priv_class     = &ccaption_dec_class,
    .capabilities   = AV_CODEC_CAP_DELAY,
};
