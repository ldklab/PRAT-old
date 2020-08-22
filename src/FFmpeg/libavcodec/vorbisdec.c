/**
 * @file
 * Vorbis I decoder
 * @author Denes Balatoni  ( dbalatoni programozo hu )
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
 * Vorbis I decoder
 * @author Denes Balatoni  ( dbalatoni programozo hu )
 */

#include <inttypes.h>
#include <math.h>

#include "libavutil/avassert.h"
#include "libavutil/float_dsp.h"

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "fft.h"
#include "get_bits.h"
#include "internal.h"
#include "vorbis.h"
#include "vorbisdsp.h"
#include "xiph.h"

#define V_NB_BITS 8
#define V_NB_BITS2 11
#define V_MAX_VLCS (1 << 16)
#define V_MAX_PARTITIONS (1 << 20)

typedef struct vorbis_codebook {
    uint8_t      dimensions;
    uint8_t      lookup_type;
    uint8_t      maxdepth;
    VLC          vlc;
    float       *codevectors;
    unsigned int nb_bits;
} vorbis_codebook;

typedef union  vorbis_floor_u  vorbis_floor_data;
typedef struct vorbis_floor0_s vorbis_floor0;
typedef struct vorbis_floor1_s vorbis_floor1;
struct vorbis_context_s;
typedef
int (* vorbis_floor_decode_func)
    (struct vorbis_context_s *, vorbis_floor_data *, float *);
typedef struct vorbis_floor {
    uint8_t floor_type;
    vorbis_floor_decode_func decode;
    union vorbis_floor_u {
        struct vorbis_floor0_s {
            uint8_t       order;
            uint16_t      rate;
            uint16_t      bark_map_size;
            int32_t      *map[2];
            uint32_t      map_size[2];
            uint8_t       amplitude_bits;
            uint8_t       amplitude_offset;
            uint8_t       num_books;
            uint8_t      *book_list;
            float        *lsp;
        } t0;
        struct vorbis_floor1_s {
            uint8_t       partitions;
            uint8_t       partition_class[32];
            uint8_t       class_dimensions[16];
            uint8_t       class_subclasses[16];
            uint8_t       class_masterbook[16];
            int16_t       subclass_books[16][8];
            uint8_t       multiplier;
            uint16_t      x_list_dim;
            vorbis_floor1_entry *list;
        } t1;
    } data;
} vorbis_floor;

typedef struct vorbis_residue {
    uint16_t      type;
    uint32_t      begin;
    uint32_t      end;
    unsigned      partition_size;
    uint8_t       classifications;
    uint8_t       classbook;
    int16_t       books[64][8];
    uint8_t       maxpass;
    uint16_t      ptns_to_read;
    uint8_t      *classifs;
} vorbis_residue;

typedef struct vorbis_mapping {
    uint8_t       submaps;
    uint16_t      coupling_steps;
    uint8_t      *magnitude;
    uint8_t      *angle;
    uint8_t      *mux;
    uint8_t       submap_floor[16];
    uint8_t       submap_residue[16];
} vorbis_mapping;

typedef struct vorbis_mode {
    uint8_t       blockflag;
    uint16_t      windowtype;
    uint16_t      transformtype;
    uint8_t       mapping;
} vorbis_mode;

typedef struct vorbis_context_s {
    AVCodecContext *avctx;
    GetBitContext gb;
    VorbisDSPContext dsp;
    AVFloatDSPContext *fdsp;

    FFTContext mdct[2];
    uint8_t       first_frame;
    uint32_t      version;
    uint8_t       audio_channels;
    uint32_t      audio_samplerate;
    uint32_t      bitrate_maximum;
    uint32_t      bitrate_nominal;
    uint32_t      bitrate_minimum;
    uint32_t      blocksize[2];
    const float  *win[2];
    uint16_t      codebook_count;
    vorbis_codebook *codebooks;
    uint8_t       floor_count;
    vorbis_floor *floors;
    uint8_t       residue_count;
    vorbis_residue *residues;
    uint8_t       mapping_count;
    vorbis_mapping *mappings;
    uint8_t       mode_count;
    vorbis_mode  *modes;
    uint8_t       mode_number; // mode number for the current packet
    int8_t       previous_window;
    float        *channel_residues;
    float        *saved;
} vorbis_context;

/* Helper functions */

#define BARK(x) \
    (13.1f * atan(0.00074f * (x)) + 2.24f * atan(1.85e-8f * (x) * (x)) + 1e-4f * (x))

static const char idx_err_str[] = "Index value %d out of range (0 - %d) for %s at %s:%i\n";
#define VALIDATE_INDEX(idx, limit) \
    if (idx >= limit) {\
        av_log(vc->avctx, AV_LOG_ERROR,\
               idx_err_str,\
               (int)(idx), (int)(limit - 1), #idx, __FILE__, __LINE__);\
        return AVERROR_INVALIDDATA;\
    }
#define GET_VALIDATED_INDEX(idx, bits, limit) \
    {\
        idx = get_bits(gb, bits);\
        VALIDATE_INDEX(idx, limit)\
    }

{
}


// Free all allocated memory -----------------------------------------

{




        }

            } else {
            }
        }

        }

// Parse setup header -------------------------------------------------

// Process codebooks part

{



        ret = AVERROR(ENOMEM);
        goto error;
    }



            av_log(vc->avctx, AV_LOG_ERROR,
                   " %u. Codebook setup data corrupt.\n", cb);
            ret = AVERROR_INVALIDDATA;
            goto error;
        }

            av_log(vc->avctx, AV_LOG_ERROR,
                   " %u. Codebook's dimension is invalid (%d).\n",
                   cb, codebook_setup->dimensions);
            ret = AVERROR_INVALIDDATA;
            goto error;
        }
            av_log(vc->avctx, AV_LOG_ERROR,
                   " %u. Codebook has too many entries (%u).\n",
                   cb, entries);
            ret = AVERROR_INVALIDDATA;
            goto error;
        }


                codebook_setup->dimensions, entries);



                ff_dlog(NULL, " sparse \n");

                used_entries = 0;
                    } else
                }
            } else {
                ff_dlog(NULL, " not sparse \n");

            }
        } else {







            }
                av_log(vc->avctx, AV_LOG_ERROR, " More codelengths than codes in codebook. \n");
                ret = AVERROR_INVALIDDATA;
                goto error;
            }
        }


                codebook_setup->lookup_type ? "vq" : "no lookup");

// If the codebook is used for (inverse) VQ, calculate codevectors.



                    codebook_lookup_values);
                    codebook_delta_value, codebook_minimum_value);


                        (float)codebook_multiplicands[i] * codebook_delta_value + codebook_minimum_value);
            }

// Weed out unused vlcs and build codevector vector
                               sizeof(*codebook_setup->codevectors));
                    ret = AVERROR(ENOMEM);
                    goto error;
                }
            } else
                codebook_setup->codevectors = NULL;


                    float last = 0.0;
                    unsigned lookup_offset = i;

                    ff_dlog(vc->avctx, "Lookup offset %u ,", i);

                    }

                        ff_dlog(vc->avctx, " %f ",
                                codebook_setup->codevectors[j * dim + k]);

                }
            }
                av_log(vc->avctx, AV_LOG_ERROR, "Bug in codevector vector building code. \n");
                ret = AVERROR_INVALIDDATA;
                goto error;
            }
            entries = used_entries;
            av_log(vc->avctx, AV_LOG_ERROR, "Codebook lookup type not supported. \n");
            ret = AVERROR_INVALIDDATA;
            goto error;
        }

// Initialize VLC table
            av_log(vc->avctx, AV_LOG_ERROR, " Invalid code lengths while generating vlcs. \n");
            ret = AVERROR_INVALIDDATA;
            goto error;
        }

            codebook_setup->nb_bits = V_NB_BITS2;
        else


                            entries, tmp_vlc_bits, sizeof(*tmp_vlc_bits),
                            sizeof(*tmp_vlc_bits), tmp_vlc_codes,
                            sizeof(*tmp_vlc_codes), sizeof(*tmp_vlc_codes),
                            INIT_VLC_LE))) {
            av_log(vc->avctx, AV_LOG_ERROR, " Error generating vlc tables. \n");
            goto error;
        }
    }


// Error:
error:
    av_free(tmp_vlc_bits);
    av_free(tmp_vlc_codes);
    av_free(codebook_multiplicands);
    return ret;
}

// Process time domain transforms part (unused in Vorbis I)

{


                vorbis_time_count, vorbis_tdtransform);

            av_log(vc->avctx, AV_LOG_ERROR, "Vorbis time domain transform data nonzero. \n");
            return AVERROR_INVALIDDATA;
        }
    }
    return 0;
}

// Process floors part

static int vorbis_floor0_decode(vorbis_context *vc,
                                vorbis_floor_data *vfu, float *vec);
static int create_map(vorbis_context *vc, unsigned floor_number);
static int vorbis_floor1_decode(vorbis_context *vc,
                                vorbis_floor_data *vfu, float *vec);
{


        return AVERROR(ENOMEM);







                    i, floor_setup->data.t1.partitions);

                    maximum_class = floor_setup->data.t1.partition_class[j];

                        i, j, floor_setup->data.t1.partition_class[j]);

            }

            ff_dlog(NULL, " maximum class %d \n", maximum_class);


                        floor_setup->data.t1.class_dimensions[j],
                        floor_setup->data.t1.class_subclasses[j]);


                    ff_dlog(NULL, "   masterbook: %d \n", floor_setup->data.t1.class_masterbook[j]);
                }


                }
            }



                                                   sizeof(*floor_setup->data.t1.list));
                return AVERROR(ENOMEM);

                av_log(vc->avctx, AV_LOG_ERROR,
                       "A rangebits value of 0 is not compliant with the Vorbis I specification.\n");
                return AVERROR_INVALIDDATA;
            }
                av_log(vc->avctx, AV_LOG_ERROR,
                       "Floor value is too large for blocksize: %u (%"PRIu32")\n",
                       rangemax, vc->blocksize[1] / 2);
                return AVERROR_INVALIDDATA;
            }


                            floor_setup->data.t1.list[floor1_values].x);
                }
            }

// Precalculate order of x coordinates - needed for decode
                                            floor_setup->data.t1.list,
                return AVERROR_INVALIDDATA;
            }


                av_log(vc->avctx, AV_LOG_ERROR, "Floor 0 order is 0.\n");
                return AVERROR_INVALIDDATA;
            }
                av_log(vc->avctx, AV_LOG_ERROR, "Floor 0 rate is 0.\n");
                return AVERROR_INVALIDDATA;
            }
                av_log(vc->avctx, AV_LOG_ERROR,
                       "Floor 0 bark map size is 0.\n");
                return AVERROR_INVALIDDATA;
            }

            /* allocate mem for booklist */
                return AVERROR(ENOMEM);
            /* read book indexes */
            {
                int idx;
                unsigned book_idx;
                        max_codebook_dim = vc->codebooks[book_idx].dimensions;
                }
            }

                return ret;

            /* codebook dim is for padding if codebook dim doesn't *
             * divide order+1 then we need to read more data       */
                                sizeof(*floor_setup->data.t0.lsp));
                return AVERROR(ENOMEM);

            /* debug output parsed headers */
            ff_dlog(NULL, "floor0 order: %u\n", floor_setup->data.t0.order);
            ff_dlog(NULL, "floor0 rate: %u\n", floor_setup->data.t0.rate);
            ff_dlog(NULL, "floor0 bark map size: %u\n",
                    floor_setup->data.t0.bark_map_size);
            ff_dlog(NULL, "floor0 amplitude bits: %u\n",
                    floor_setup->data.t0.amplitude_bits);
            ff_dlog(NULL, "floor0 amplitude offset: %u\n",
                    floor_setup->data.t0.amplitude_offset);
            ff_dlog(NULL, "floor0 number of books: %u\n",
                    floor_setup->data.t0.num_books);
            ff_dlog(NULL, "floor0 book list pointer: %p\n",
                    floor_setup->data.t0.book_list);
            {
                int idx;
                for (idx = 0; idx < floor_setup->data.t0.num_books; ++idx) {
                    ff_dlog(NULL, "  Book %d: %u\n", idx + 1,
                            floor_setup->data.t0.book_list[idx]);
                }
            }
        } else {
            av_log(vc->avctx, AV_LOG_ERROR, "Invalid floor type!\n");
            return AVERROR_INVALIDDATA;
        }
    }
    return 0;
}

// Process residues part

{

        return AVERROR(ENOMEM);

    ff_dlog(NULL, " There are %d residues. \n", vc->residue_count);




        /* Validations to prevent a buffer overflow later. */
            av_log(vc->avctx, AV_LOG_ERROR,
                   "partition out of bounds: type, begin, end, size, blocksize: %"PRIu16", %"PRIu32", %"PRIu32", %u, %"PRIu32"\n",
                   res_setup->type, res_setup->begin, res_setup->end,
                   res_setup->partition_size, vc->blocksize[1] / 2);
            return AVERROR_INVALIDDATA;
        }


            (res_setup->end - res_setup->begin) / res_setup->partition_size;
                                        sizeof(*res_setup->classifs));
            return AVERROR(ENOMEM);

        ff_dlog(NULL, "    begin %"PRIu32" end %"PRIu32" part.size %u classif.s %"PRIu8" classbook %"PRIu8"\n",
                res_setup->begin, res_setup->end, res_setup->partition_size,
                res_setup->classifications, res_setup->classbook);


        }


                            j, k, res_setup->books[j][k]);

                } else {
                }
            }
        }
    }
    return 0;
}

// Process mappings part

{

        return AVERROR(ENOMEM);

    ff_dlog(NULL, " There are %d mappings. \n", vc->mapping_count);


            av_log(vc->avctx, AV_LOG_ERROR, "Other mappings than type 0 are not compliant with the Vorbis I specification. \n");
            return AVERROR_INVALIDDATA;
        }
        } else {
        }

                av_log(vc->avctx, AV_LOG_ERROR,
                       "Square polar channel mapping with less than two channels is not compliant with the Vorbis I specification.\n");
                return AVERROR_INVALIDDATA;
            }
                                                       sizeof(*mapping_setup->magnitude));
                                                       sizeof(*mapping_setup->angle));
                return AVERROR(ENOMEM);

            }
        } else {
        }

                i, mapping_setup->coupling_steps);

            av_log(vc->avctx, AV_LOG_ERROR, "%u. mapping setup data invalid.\n", i);
            return AVERROR_INVALIDDATA; // following spec.
        }

                                            sizeof(*mapping_setup->mux));
                return AVERROR(ENOMEM);

        }


                    mapping_setup->submap_floor[j],
                    mapping_setup->submap_residue[j]);
        }
    }
    return 0;
}

// Process modes part

{

            return AVERROR(ENOMEM);


                map[idx] = vf->bark_map_size - 1;
        }
    }

    for (idx = 0; idx <= n; ++idx) {
        ff_dlog(NULL, "floor0 map: map at pos %d is %"PRId32"\n", idx, map[idx]);
    }

    return 0;
}

{

        return AVERROR(ENOMEM);

    ff_dlog(NULL, " There are %d modes.\n", vc->mode_count);



                i, mode_setup->blockflag, mode_setup->windowtype,
                mode_setup->transformtype, mode_setup->mapping);
    }
    return 0;
}

// Process the whole setup header using the functions above

{

        av_log(vc->avctx, AV_LOG_ERROR, " Vorbis setup header packet corrupt (no vorbis signature). \n");
        return AVERROR_INVALIDDATA;
    }

        av_log(vc->avctx, AV_LOG_ERROR, " Vorbis setup header packet corrupt (codebooks). \n");
        return ret;
    }
        av_log(vc->avctx, AV_LOG_ERROR, " Vorbis setup header packet corrupt (time domain transforms). \n");
        return ret;
    }
        av_log(vc->avctx, AV_LOG_ERROR, " Vorbis setup header packet corrupt (floors). \n");
        return ret;
    }
        av_log(vc->avctx, AV_LOG_ERROR, " Vorbis setup header packet corrupt (residues). \n");
        return ret;
    }
        av_log(vc->avctx, AV_LOG_ERROR, " Vorbis setup header packet corrupt (mappings). \n");
        return ret;
    }
        av_log(vc->avctx, AV_LOG_ERROR, " Vorbis setup header packet corrupt (modes). \n");
        return ret;
    }
        av_log(vc->avctx, AV_LOG_ERROR, " Vorbis setup header packet corrupt (framing flag). \n");
        return AVERROR_INVALIDDATA; // framing flag bit unset error
    }

    return 0;
}

// Process the identification header

{

        av_log(vc->avctx, AV_LOG_ERROR, " Vorbis id header packet corrupt (no vorbis signature). \n");
        return AVERROR_INVALIDDATA;
    }

        av_log(vc->avctx, AV_LOG_ERROR, "Invalid number of channels\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(vc->avctx, AV_LOG_ERROR, "Invalid samplerate\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(vc->avctx, AV_LOG_ERROR, " Vorbis id header packet corrupt (illegal blocksize). \n");
        return AVERROR_INVALIDDATA;
    }

        av_log(vc->avctx, AV_LOG_ERROR, " Vorbis id header packet corrupt (framing flag not set). \n");
        return AVERROR_INVALIDDATA;
    }

        return AVERROR(ENOMEM);


        return AVERROR(ENOMEM);

    ff_dlog(NULL, " vorbis version %"PRIu32" \n audio_channels %"PRIu8" \n audio_samplerate %"PRIu32" \n bitrate_max %"PRIu32" \n bitrate_nom %"PRIu32" \n bitrate_min %"PRIu32" \n blk_0 %"PRIu32" blk_1 %"PRIu32" \n ",
            vc->version, vc->audio_channels, vc->audio_samplerate, vc->bitrate_maximum, vc->bitrate_nominal, vc->bitrate_minimum, vc->blocksize[0], vc->blocksize[1]);

/*
    BLK = vc->blocksize[0];
    for (i = 0; i < BLK / 2; ++i) {
        vc->win[0][i] = sin(0.5*3.14159265358*(sin(((float)i + 0.5) / (float)BLK*3.14159265358))*(sin(((float)i + 0.5) / (float)BLK*3.14159265358)));
    }
*/

    return 0;
}

// Process the extradata using the functions above (identification header, setup header)

{



        av_log(avctx, AV_LOG_ERROR, "Extradata missing.\n");
        return AVERROR_INVALIDDATA;
    }

        av_log(avctx, AV_LOG_ERROR, "Extradata corrupt.\n");
        return ret;
    }

        av_log(avctx, AV_LOG_ERROR, "First header is not the id header.\n");
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "Id header corrupt.\n");
        vorbis_free(vc);
        return ret;
    }

        av_log(avctx, AV_LOG_ERROR, "Third header is not the setup header.\n");
        vorbis_free(vc);
        return AVERROR_INVALIDDATA;
    }
        av_log(avctx, AV_LOG_ERROR, "Setup header corrupt.\n");
        vorbis_free(vc);
        return ret;
    }

        avctx->channel_layout = 0;
    else


}

// Decode audiopackets -------------------------------------------------

// Read and decode floor

                                vorbis_floor_data *vfu, float *vec)
{

        return 1;


            av_log(vc->avctx, AV_LOG_ERROR, "floor0 dec: booknumber too high!\n");
            book_idx =  0;
        }
        /* Invalid codebook! */
            return AVERROR_INVALIDDATA;


            /* read temp vector */
                               codebook.nb_bits, codebook.maxdepth);
                return AVERROR_INVALIDDATA;
            /* copy each vector component and add last to it */

        }
        /* DEBUG: output lsp coeffs */
        {
            int idx;
                ff_dlog(NULL, "floor0 dec: coeff at %d is %f\n", idx, lsp[idx]);
        }

        /* synthesize floor output vector */
        {


            ff_dlog(NULL, "floor0 synth: map_size = %"PRIu32"; m = %d; wstep = %f\n",
                    vf->map_size[blockflag], order, wstep);

            i = 0;

                /* similar part for the q and p products */
                }
                } else { // odd order

                    /* final step and square */
                }

                    return AVERROR_INVALIDDATA;

                /* calculate linear floor value */

                /* fill vector */
            }
        }
    } else {
        /* this channel is unused */
        return 1;
    }

    ff_dlog(NULL, " Floor0 decoded\n");

    return 0;
}

                                vorbis_floor_data *vfu, float *vec)
{


        return 1;

// Read values (or differences) for the floor's points







                    book, cbits, cval, get_bits_count(gb));

                    return AVERROR_INVALIDDATA;
            } else {
            }

                    vf->list[offset+j].x, floor1_Y[offset+j]);
        }
    }

// Amplitude calculation from the differences



        } else {
        } // render_point end

        } else {
        }
                } else {
                }
            } else {
                } else {
                }
            }
        } else {
        }

                vf->list[i].x, floor1_Y_final[i], val);
    }

// Curve synth - connect the calculated dots and convert from dB scale FIXME optimize ?



}

                                           vorbis_residue *vr,
                                           uint8_t *do_not_decode,
                                           unsigned ch_used,
                                           int partition_count,
                                           int ptns_to_read
                                          )
{



                av_log(vc->avctx, AV_LOG_ERROR,
                       "Invalid vlc code decoding %d channel.", j);
                return AVERROR_INVALIDDATA;
            }

                for (i = partition_count + c_p_c - 1; i >= partition_count; i--) {
                    if (i < ptns_to_read)
                        vr->classifs[p + i] = 0;
                }
            } else {

            }
            }
        }
    }
    return 0;
}
// Read and decode residue

                                                           vorbis_residue *vr,
                                                           unsigned ch,
                                                           uint8_t *do_not_decode,
                                                           float *vec,
                                                           unsigned vlen,
                                                           unsigned ch_left,
                                                           int vr_type)
{

            return 0;
    } else {
    }

        if (max_output <= ch_left * vlen + vr->partition_size*ch_used/ch) {
            ptns_to_read--;
            libvorbis_bug = 1;
        } else {
            av_log(vc->avctx, AV_LOG_ERROR, "Insufficient output buffer\n");
            return AVERROR_INVALIDDATA;
        }
    }



                if (ret < 0)
                    return ret;
            }




                                }
                                        return coffs;

                                                pass, voffs, vec[voffs], codebook.codevectors[coffs+l], coffs);
                                    }
                                }

                                            return coffs;
                                    }
                                            return coffs;
                                    }
                                } else
                                        return coffs;

                                                pass, voffset / ch + (voffs % ch) * vlen,
                                                vec[voffset / ch + (voffs % ch) * vlen],
                                                codebook.codevectors[coffs + l], coffs, l);
                                    }
                                }


                                        return coffs;

                                                pass, voffs_div + voffs_mod * vlen,
                                                vec[voffs_div + voffs_mod * vlen],
                                                codebook.codevectors[coffs + l], coffs, l);

                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
            for (j = 0; j < ch_used; ++j) {
                if (!do_not_decode[j]) {
                    get_vlc2(&vc->gb, vc->codebooks[vr->classbook].vlc.table,
                                vc->codebooks[vr->classbook].nb_bits, 3);
                }
            }
        }
    }
    return 0;
}

                                        unsigned ch,
                                        uint8_t *do_not_decode,
                                        float *vec, unsigned vlen,
                                        unsigned ch_left)
{
    else {
        av_log(vc->avctx, AV_LOG_ERROR, " Invalid residue type while residue decode?! \n");
        return AVERROR_INVALIDDATA;
    }
}

void ff_vorbis_inverse_coupling(float *mag, float *ang, intptr_t blocksize)
{
    int i;
    for (i = 0;  i < blocksize;  i++) {
        if (mag[i] > 0.0) {
            if (ang[i] > 0.0) {
                ang[i] = mag[i] - ang[i];
            } else {
                float temp = ang[i];
                ang[i]     = mag[i];
                mag[i]    += temp;
            }
        } else {
            if (ang[i] > 0.0) {
                ang[i] += mag[i];
            } else {
                float temp = ang[i];
                ang[i]     = mag[i];
                mag[i]    -= temp;
            }
        }
    }
}

// Decode the audio packet using the functions above

{

        av_log(vc->avctx, AV_LOG_ERROR, "Not a Vorbis I audio packet.\n");
        return AVERROR_INVALIDDATA; // packet type not audio
    }

        mode_number = 0;
    } else {
    }

            vc->modes[mode_number].mapping, vc->modes[mode_number].blockflag);

        previous_window = 0;


// Decode floor

        } else {
        }


            av_log(vc->avctx, AV_LOG_ERROR, "Invalid codebook in vorbis_floor_decode.\n");
            return AVERROR_INVALIDDATA;
        }
    }

// Nonzero vector propagate

        }
    }

// Decode residue

        vorbis_residue *residue;
        unsigned ch = 0;
        int ret;

                } else {
                }
            }
        }
            av_log(vc->avctx, AV_LOG_ERROR, "Too many channels in vorbis_floor_decode.\n");
            return AVERROR_INVALIDDATA;
        }
                return ret;
        }

    }

        return AVERROR_INVALIDDATA;

// Inverse coupling


    }

// Dotproduct, MDCT


    }

// Overlap/add, save data for next overlapping


        } else {
        }
    }

}

// Return the decoded audio packet through the standard api

                               int *got_frame_ptr, AVPacket *avpkt)
{


        if ((ret = init_get_bits8(gb, buf + 1, buf_size - 1)) < 0)
            return ret;

        vorbis_free(vc);
        if ((ret = vorbis_parse_id_hdr(vc))) {
            av_log(avctx, AV_LOG_ERROR, "Id header corrupt.\n");
            vorbis_free(vc);
            return ret;
        }

        if (vc->audio_channels > 8)
            avctx->channel_layout = 0;
        else
            avctx->channel_layout = ff_vorbis_channel_layouts[vc->audio_channels - 1];

        avctx->channels    = vc->audio_channels;
        avctx->sample_rate = vc->audio_samplerate;
        return buf_size;
    }

        av_log(avctx, AV_LOG_DEBUG, "Ignoring comment header\n");
        return buf_size;
    }

        if ((ret = init_get_bits8(gb, buf + 1, buf_size - 1)) < 0)
            return ret;

        if ((ret = vorbis_parse_setup_hdr(vc))) {
            av_log(avctx, AV_LOG_ERROR, "Setup header corrupt.\n");
            vorbis_free(vc);
            return ret;
        }
        return buf_size;
    }

        av_log(avctx, AV_LOG_ERROR, "Data packet before valid headers\n");
        return AVERROR_INVALIDDATA;
    }

    /* get output buffer */
        return ret;

        for (i = 0; i < vc->audio_channels; i++)
            channel_ptrs[i] = (float *)frame->extended_data[i];
    } else {
        }
    }

        return ret;

        return len;

    }

            get_bits_count(gb) / 8, get_bits_count(gb) % 8, len);


}

// Close decoder

{


}

static av_cold void vorbis_decode_flush(AVCodecContext *avctx)
{
    vorbis_context *vc = avctx->priv_data;

    if (vc->saved) {
        memset(vc->saved, 0, (vc->blocksize[1] / 4) * vc->audio_channels *
                             sizeof(*vc->saved));
    }
    vc->previous_window = -1;
    vc->first_frame = 0;
}

AVCodec ff_vorbis_decoder = {
    .name            = "vorbis",
    .long_name       = NULL_IF_CONFIG_SMALL("Vorbis"),
    .type            = AVMEDIA_TYPE_AUDIO,
    .id              = AV_CODEC_ID_VORBIS,
    .priv_data_size  = sizeof(vorbis_context),
    .init            = vorbis_decode_init,
    .close           = vorbis_decode_close,
    .decode          = vorbis_decode_frame,
    .flush           = vorbis_decode_flush,
    .capabilities    = AV_CODEC_CAP_DR1,
    .caps_internal   = FF_CODEC_CAP_INIT_CLEANUP,
    .channel_layouts = ff_vorbis_channel_layouts,
    .sample_fmts     = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
                                                       AV_SAMPLE_FMT_NONE },
};
