/*
 * DirectDraw Surface image decoder
 * Copyright (C) 2015 Vittorio Giovara <vittorio.giovara@gmail.com>
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

/**
 * @file
 * DDS decoder
 *
 * https://msdn.microsoft.com/en-us/library/bb943982%28v=vs.85%29.aspx
 */

#include <stdint.h>

#include "libavutil/libm.h"
#include "libavutil/imgutils.h"

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "texturedsp.h"
#include "thread.h"

#define DDPF_FOURCC    (1 <<  2)
#define DDPF_PALETTE   (1 <<  5)
#define DDPF_NORMALMAP (1U << 31)

enum DDSPostProc {
    DDS_NONE = 0,
    DDS_ALPHA_EXP,
    DDS_NORMAL_MAP,
    DDS_RAW_YCOCG,
    DDS_SWAP_ALPHA,
    DDS_SWIZZLE_A2XY,
    DDS_SWIZZLE_RBXG,
    DDS_SWIZZLE_RGXB,
    DDS_SWIZZLE_RXBG,
    DDS_SWIZZLE_RXGB,
    DDS_SWIZZLE_XGBR,
    DDS_SWIZZLE_XRBG,
    DDS_SWIZZLE_XGXR,
};

enum DDSDXGIFormat {
    DXGI_FORMAT_R16G16B16A16_TYPELESS       =  9,
    DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
    DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
    DXGI_FORMAT_R16G16B16A16_UINT           = 12,
    DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
    DXGI_FORMAT_R16G16B16A16_SINT           = 14,

    DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
    DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
    DXGI_FORMAT_R8G8B8A8_UINT               = 30,
    DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
    DXGI_FORMAT_R8G8B8A8_SINT               = 32,

    DXGI_FORMAT_BC1_TYPELESS                = 70,
    DXGI_FORMAT_BC1_UNORM                   = 71,
    DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
    DXGI_FORMAT_BC2_TYPELESS                = 73,
    DXGI_FORMAT_BC2_UNORM                   = 74,
    DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
    DXGI_FORMAT_BC3_TYPELESS                = 76,
    DXGI_FORMAT_BC3_UNORM                   = 77,
    DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
    DXGI_FORMAT_BC4_TYPELESS                = 79,
    DXGI_FORMAT_BC4_UNORM                   = 80,
    DXGI_FORMAT_BC4_SNORM                   = 81,
    DXGI_FORMAT_BC5_TYPELESS                = 82,
    DXGI_FORMAT_BC5_UNORM                   = 83,
    DXGI_FORMAT_BC5_SNORM                   = 84,
    DXGI_FORMAT_B5G6R5_UNORM                = 85,
    DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
    DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
    DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
    DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
};

typedef struct DDSContext {
    TextureDSPContext texdsp;
    GetByteContext gbc;

    int compressed;
    int paletted;
    int bpp;
    enum DDSPostProc postproc;

    const uint8_t *tex_data; // Compressed texture
    int tex_ratio;           // Compression ratio
    int slice_count;         // Number of slices for threaded operations

    /* Pointer to the selected compress or decompress function. */
    int (*tex_funct)(uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
} DDSContext;

{

    /* Alternative DDS implementations use reserved1 as custom header. */

    /* Now the real DDPF starts. */
        av_log(avctx, AV_LOG_ERROR, "Invalid pixel format header %d.\n", size);
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_WARNING,
               "Disabling invalid palette flag for compressed dds.\n");
        ctx->paletted = 0;
    }





            else
            break;
            /* This format may be considered as a normal map,
             * but it is handled differently in a separate postproc. */
        case MKTAG('B', 'C', '4', 'U'):
            /* RGT2 variant with swapped R and G (3Dc)*/
        case MKTAG('B', 'C', '5', 'U'):
            ctx->tex_ratio = 16;
            ctx->tex_funct = ctx->texdsp.rgtc2u_block;
            break;
            /* ATI Palette8, same as normal palette */
        case MKTAG('D', 'X', '1', '0'):
            /* DirectX 10 extra header */

                       "Found array of size %d (ignored).\n", array);

            /* Only BC[1-5] are actually compressed. */

            /* RGB types. */
            case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            case DXGI_FORMAT_R16G16B16A16_FLOAT:
            case DXGI_FORMAT_R16G16B16A16_UNORM:
            case DXGI_FORMAT_R16G16B16A16_UINT:
            case DXGI_FORMAT_R16G16B16A16_SNORM:
            case DXGI_FORMAT_R16G16B16A16_SINT:
                avctx->pix_fmt = AV_PIX_FMT_BGRA64;
                break;
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                avctx->colorspace = AVCOL_SPC_RGB;
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UINT:
            case DXGI_FORMAT_R8G8B8A8_SNORM:
            case DXGI_FORMAT_R8G8B8A8_SINT:
                avctx->pix_fmt = AV_PIX_FMT_BGRA;
                break;
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                avctx->colorspace = AVCOL_SPC_RGB;
            case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            case DXGI_FORMAT_B8G8R8A8_UNORM:
                avctx->pix_fmt = AV_PIX_FMT_RGBA;
                break;
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                avctx->colorspace = AVCOL_SPC_RGB;
            case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            case DXGI_FORMAT_B8G8R8X8_UNORM:
                avctx->pix_fmt = AV_PIX_FMT_RGBA; // opaque
                break;
            case DXGI_FORMAT_B5G6R5_UNORM:
                avctx->pix_fmt = AV_PIX_FMT_RGB565LE;
                break;
            /* Texture types. */
            case DXGI_FORMAT_BC1_UNORM_SRGB:
                avctx->colorspace = AVCOL_SPC_RGB;
            case DXGI_FORMAT_BC1_UNORM:
            case DXGI_FORMAT_BC2_UNORM_SRGB:
                avctx->colorspace = AVCOL_SPC_RGB;
            case DXGI_FORMAT_BC2_UNORM:
            case DXGI_FORMAT_BC3_UNORM_SRGB:
                avctx->colorspace = AVCOL_SPC_RGB;
            case DXGI_FORMAT_BC3_UNORM:
            case DXGI_FORMAT_BC4_UNORM:
            case DXGI_FORMAT_BC4_SNORM:
                ctx->tex_ratio = 8;
                ctx->tex_funct = ctx->texdsp.rgtc1s_block;
                break;
            case DXGI_FORMAT_BC5_UNORM:
            case DXGI_FORMAT_BC5_SNORM:
                ctx->tex_ratio = 16;
                ctx->tex_funct = ctx->texdsp.rgtc2s_block;
                break;
            default:
                av_log(avctx, AV_LOG_ERROR,
                       "Unsupported DXGI format %d.\n", dxgi);
                return AVERROR_INVALIDDATA;
            }
            break;
        default:
            av_log(avctx, AV_LOG_ERROR, "Unsupported %s fourcc.\n", av_fourcc2str(fourcc));
            return AVERROR_INVALIDDATA;
        }
        } else {
            av_log(avctx, AV_LOG_ERROR, "Unsupported palette bpp %d.\n", bpp);
            return AVERROR_INVALIDDATA;
        }
    } else {
        /*  4 bpp */
            avctx->pix_fmt = AV_PIX_FMT_PAL8;
        /*  8 bpp */
        /* 16 bpp */
            avctx->pix_fmt = AV_PIX_FMT_YA8;
            ctx->postproc = DDS_SWAP_ALPHA;
        }
            avctx->pix_fmt = AV_PIX_FMT_GRAY16LE;
        /* 24 bpp */
        /* 32 bpp */
        else if (bpp == 32 && r == 0xff && g == 0xff00 && b == 0xff0000 && a == 0xff000000)
            avctx->pix_fmt = AV_PIX_FMT_RGBA;
        /* give up */
        else {
            av_log(avctx, AV_LOG_ERROR, "Unknown pixel format "
                   "[bpp %d r 0x%x g 0x%x b 0x%x a 0x%x].\n", bpp, r, g, b, a);
            return AVERROR_INVALIDDATA;
        }
    }

    /* Set any remaining post-proc that should happen before frame is ready. */

    /* ATI/NVidia variants sometimes add swizzling in bpp. */
    }

    return 0;
}

                                     int slice, int thread_nb)
{

    /* When the frame height (in blocks) doesn't divide evenly between the
     * number of slices, spread the remaining blocks evenly between the first
     * operations */
    /* Add any extra blocks (one per slice) that have been added before this slice */

    /* Add an extra block if there are still remainder blocks to be accounted for */
        end_slice++;

        }
    }

}

{
    }
}

{

        /* Alpha-exponential mode divides each channel by the maximum
         * R, G or B value, and stores the multiplying factor in the
         * alpha channel. */


        }
        break;
        /* Normal maps work in the XYZ color space and they encode
         * X in R or in A, depending on the texture type, Y in G and
         * derive Z with a square root of the distance.
         *
         * http://www.realtimecollisiondetection.net/blog/?p=28 */



        }
        break;
        /* Data is Y-Co-Cg-A and not RGBA, but they are represented
         * with the same masks in the DDPF header. */


        }
        break;
    case DDS_SWAP_ALPHA:
        /* Alpha and Luma are stored swapped. */
        av_log(avctx, AV_LOG_DEBUG, "Post-processing swapped Luma/Alpha.\n");

        for (i = 0; i < frame->linesize[0] * frame->height; i += 2) {
            uint8_t *src = frame->data[0] + i;
            FFSWAP(uint8_t, src[0], src[1]);
        }
        break;
        /* Swap R and G, often used to restore a standard RGTC2. */
        break;
        /* Swap G and A, then B and new A (G). */
        break;
        /* Swap B and A. */
        break;
        /* Swap G and A. */
        break;
        /* Swap R and A (misleading name). */
        break;
        /* Swap B and A, then R and new A (B). */
        break;
        /* Swap G and A, then R and new A (G), then new R (G) and new G (A).
         * This variant does not store any B component. */
        break;
        /* Swap G and A, then R and new A (G). */
        break;
    }

                      int *got_frame, AVPacket *avpkt)
{


        av_log(avctx, AV_LOG_ERROR, "Frame is too small (%d).\n",
               bytestream2_get_bytes_left(gbc));
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Invalid DDS header.\n");
        return AVERROR_INVALIDDATA;
    }


        av_log(avctx, AV_LOG_ERROR, "Invalid image size %dx%d.\n",
               avctx->width, avctx->height);
        return ret;
    }

    /* Since codec is based on 4x4 blocks, size is aligned to 4. */


    /* Extract pixel format information, considering additional elements
     * in reserved1 and reserved2. */
        return ret;

        return ret;

                                   avctx->coded_height / TEXTURE_BLOCK_H);

            av_log(avctx, AV_LOG_ERROR,
                   "Compressed Buffer is too small (%d < %d).\n",
                   bytestream2_get_bytes_left(gbc), size);
            return AVERROR_INVALIDDATA;
        }

        /* Use the decompress function on the texture, one block per thread. */
        uint8_t *dst = frame->data[0];
        int x, y, i;

        /* Use the first 64 bytes as palette, then copy the rest. */
        bytestream2_get_buffer(gbc, frame->data[1], 16 * 4);
        for (i = 0; i < 16; i++) {
            AV_WN32(frame->data[1] + i*4,
                    (frame->data[1][2+i*4]<<0)+
                    (frame->data[1][1+i*4]<<8)+
                    (frame->data[1][0+i*4]<<16)+
                    ((unsigned)frame->data[1][3+i*4]<<24)
            );
        }
        frame->palette_has_changed = 1;

        if (bytestream2_get_bytes_left(gbc) < frame->height * frame->width / 2) {
            av_log(avctx, AV_LOG_ERROR, "Buffer is too small (%d < %d).\n",
                   bytestream2_get_bytes_left(gbc), frame->height * frame->width / 2);
            return AVERROR_INVALIDDATA;
        }

        for (y = 0; y < frame->height; y++) {
            for (x = 0; x < frame->width; x += 2) {
                uint8_t val = bytestream2_get_byte(gbc);
                dst[x    ] = val & 0xF;
                dst[x + 1] = val >> 4;
            }
            dst += frame->linesize[0];
        }
    } else {

            /* Use the first 1024 bytes as palette, then copy the rest. */
                        (frame->data[1][2+i*4]<<0)+
                        (frame->data[1][1+i*4]<<8)+
                        (frame->data[1][0+i*4]<<16)+
                        ((unsigned)frame->data[1][3+i*4]<<24)
                );

        }

            av_log(avctx, AV_LOG_ERROR, "Buffer is too small (%d < %d).\n",
                   bytestream2_get_bytes_left(gbc), frame->height * linesize);
            return AVERROR_INVALIDDATA;
        }

                            gbc->buffer, linesize,
                            linesize, frame->height);
    }

    /* Run any post processing here if needed. */

    /* Frame is ready to be output. */

}

AVCodec ff_dds_decoder = {
    .name           = "dds",
    .long_name      = NULL_IF_CONFIG_SMALL("DirectDraw Surface image decoder"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_DDS,
    .decode         = dds_decode,
    .priv_data_size = sizeof(DDSContext),
    .capabilities   = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_SLICE_THREADS,
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE
};
