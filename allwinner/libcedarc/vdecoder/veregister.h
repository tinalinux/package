
/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : veregister.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/

#ifndef LIBVE_REGISTER_H
#define LIBVE_REGISTER_H

#include "adapter.h"
//#include "secureMemoryAdapter.h"
#include "log.h"
#include "typedef.h"

static const size_addr REG_GROUP_OFFSET_ARR[] = {0, 0x100, 0x200, 0x300, 0x400, 0x500, 0xe00};
static const char* REG_GROUP_NAME[] = {"Top Level",
                                  "Mpeg 1/2/4",
                                  "H264/AVS",
                                  "VC-1",
                                  "RV",
                                  "H265",
                                  "JPEG"};

typedef enum VE_REGISTER_GROUP
{
    REG_GROUP_VETOP             = 0,
    REG_GROUP_MPEG_DECODER      = 1,
    REG_GROUP_H264_DECODER      = 2,
    REG_GROUP_VC1_DECODER       = 3,
    REG_GROUP_RV_DECODER        = 4,
    REG_GROUP_H265_DECODER      = 5,
    REG_GROUP_JPEG_DECODER      = 6,
    REG_GROUP_AVS_DECODER        = 2
}ve_register_group_e;


//* 0x08
typedef struct VETOP_REG_CODEC_COUNTER
{
    volatile unsigned int counter                   :31;
    volatile unsigned int enable                    :1;
}vetop_reg_codec_cntr_t;


//* 0x0c
typedef struct VETOP_REG_CODEC_OVERTIME
{
    volatile unsigned int overtime_value            :31;
    volatile unsigned int reserved0                 :1;
}vetop_reg_codec_overtime_t;


//* 0x10
typedef struct VETOP_REG_MMC_REQUEST_WNUM
{
    volatile unsigned int wnum                      :24;
    volatile unsigned int reserved0                 :8;
}vetop_reg_mmc_req_wnum_t;


//* 0x14
typedef struct VETOP_REG_CACHE_REQ_WNUM
{
    volatile unsigned int wnum                      :24;
    volatile unsigned int reserved0                 :8;
}vetop_reg_cache_req_wnum_t;


//* 0x18 register is reserved.


//* 0x1c
typedef struct VETOP_REG_STATUS
{
    volatile unsigned int timeout_intr              :1;

    //the following 7 bits are for 1619, 1620, 1623, ...
    volatile unsigned int isp_intr                  :1;
    //* in 1633, this bit is not used.
    volatile unsigned int ave_intr                  :1;
    //* in 1633, this bit is not used.
    volatile unsigned int mpe_intr                  :1;
    //* in 1633, this bit is ave_intr.
    volatile unsigned int rv_intr                   :1;
    volatile unsigned int vc1_intr                  :1;
    volatile unsigned int avc_intr                  :1;
    volatile unsigned int mpg_intr                  :1;

    volatile unsigned int timeout_enable            :1;

    //* the following 7 bits are for 1619.
    volatile unsigned int graphic_intr              :1;
    //* in 1633, this bit is not used.
    volatile unsigned int graphic_intr_enable       :1;
    //* in 1633, this bit is not used.
    volatile unsigned int reserved1                 :5;

    volatile unsigned int mem_sync_idle             :1;
    //* this bit will be 1 when mem sync idle.

    volatile unsigned int reserved0                 :11;
    //* the following 2 bits are for 1619.
    volatile unsigned int graphic_cmd_queue_busy    :1;
    //* in 1633, this bit is not used.
    volatile unsigned int graphic_busy              :1;
    //* in 1633, this bit is not used.

    //* the following 1 bit is for 1619, 1620, 1623, ...
    volatile unsigned int maf_busy                  :1;
    //* in 1633, this bit is not used.

    volatile unsigned int esp_mode                  :1;
    //* in 1633, this bit is not used.
}vetop_reg_status_t;


//* 0x20
typedef struct VETOP_REG_DRAM_READ_COUNTER
{
    volatile unsigned int counter                   :31;
    volatile unsigned int enable                    :1;
}vetop_reg_dram_read_cntr_t;


//* 0x24
typedef struct VETOP_REG_DRAM_WRITE_COUNTER
{
    volatile unsigned int counter                   :31;
    volatile unsigned int enable                    :1;
}vetop_reg_dram_write_cntr_t;


//* for 1618, 0x28 - 0x7c registers are reserved.
//* for 1620 and 1619, 0x28 - 0x2c registers are reserved.
// *0x28
typedef struct VETOP_REG_ANAGLYPH_CONTROL
{
     volatile unsigned int anaglyph_src_mode    : 2;
     //* 00: source mode; 01: top/down or over/under; 10: line by line; 11: separate.
     volatile unsigned int reserved2            : 1;
     volatile unsigned int irec_is_right        : 1;
     //* 0: left, 1: right.
     volatile unsigned int anaglyph_proc_mode   : 3;
     //* 000: read/bule, 001: red/green, 010: red cyan,
     //*011: color, 100: half color, 101: optimized, 110: yello/blue, 111: reserved.
     volatile unsigned int reserved1            : 1;
     volatile unsigned int yuv2rgb_mode         : 2;
     //* 00: YCC, 01: BT601, 10: BT709.
     volatile unsigned int reserved0            : 21;
     volatile unsigned int anaglyph_out_enable  : 1;
     //* 0: disable, 1: enable.
}vetop_reg_anaglyph_cntr_t;

//* 0x30 //* register not used in 1633.
typedef struct VETOP_REG_MAF_CONTROL
{
    volatile unsigned int still_to_motion_threshold  :4;
    volatile unsigned int motion_to_still_threshold  :4;
    volatile unsigned int average_shifter_y          :4;
    volatile unsigned int average_shifter_c          :4;
    volatile unsigned int reserved0                  :14;
    volatile unsigned int maf_enable                 :1;
    volatile unsigned int new_frame_bgn              :1;
}vetop_reg_maf_ctrl_t;


//* 0x34 //* register not used in 1633.
typedef struct VETOP_REG_MAF_THRESHOLD_CLIP
{
    volatile unsigned int min_clip_value_y           :8;
    volatile unsigned int max_clip_value_y           :8;
    volatile unsigned int min_clip_value_c           :8;
    volatile unsigned int max_clip_value_c           :8;
}vetop_reg_maf_th_clip_t;


//* 0x38 //* register not used in 1633.
typedef struct VETOP_REG_MAF_REF1_FB_LUMA_ADDR
{
    volatile unsigned int addr;
}vetop_reg_maf_ref1_luma_addr_t;


//* 0x3c //* register not used in 1633.
typedef struct VETOP_REG_MAF_REF1_FB_CHROMA_ADDR
{
    volatile unsigned int addr;
}vetop_reg_maf_ref1_chroma_addr_t;


//* 0x40 //* register not used in 1633.
typedef struct VETOP_REG_CURRENT_FRAME_MAF_ADDR
{
    volatile unsigned int addr;
}vetop_reg_cur_frame_maf_addr_t;


//* 0x44 //* register not used in 1633.
typedef struct VETOP_REG_REF1_FRAME_MAF_ADDR
{
    volatile unsigned int addr;
}vetop_reg_ref1_frame_maf_addr_t;


//* 0x48 //* register not used in 1633.
typedef struct VETOP_REG_REF2_FRAME_MAF_ADDR
{
    volatile unsigned int addr;
}vetop_reg_ref2_frame_maf_addr_t;


//* 0x4c //* register not used in 1633.
typedef struct VETOP_REG_MAF_MAX_GROUP_DIFFERENCE
{
    volatile unsigned int top_y_max_group_diff       :8;
    volatile unsigned int top_c_max_group_diff       :8;
    volatile unsigned int bottom_y_max_group_diff    :8;
    volatile unsigned int bottom_c_max_group_diff    :8;
}vetop_reg_maf_max_group_diff_t;


//* for 1620, 0x50 - 0x58 registers are reserved.
//* 0x50
typedef struct VETOP_REG_DEBLK_INTRAPRED_BUF_CTRL
{
    volatile unsigned int deblk_buf_ctrl             :2;
    //* deblocking above buffer's control bits.
    //* 00: all data is stored in internal SRAM,
    //* 01: the data of left 1280 pixels of picture is stored in internal SRAM.
    //* 10: all data is stored in DRAM.
    volatile unsigned int intrapred_buf_ctrl         :2;
    //* intra prediction above buffer's control bits.
    volatile unsigned int reserved0                  :28;
}vetop_reg_deblk_intrapred_buf_ctrl_t;


//* 0x54
typedef struct VETOP_REG_DEBLK_DRAM_BUF_ADDR
{
    volatile unsigned int addr;
}vetop_reg_deblk_dram_buf_addr_t;


//* 0x58
typedef struct VETOP_REG_INTRAPRED_DRAM_BUF_ADDR
{
    volatile unsigned int addr;
}vetop_reg_intrapred_dram_buf_addr_t;


//* registers from 0x5c to 0xcc are not used in 1633.
//* 0x5c //* register not used int 1633.
typedef struct VETOP_REG_ARGB_COMMAND_QUEUE_START
{
    volatile unsigned int cmd_num                    :6;
    volatile unsigned int reserved0                  :25;
    volatile unsigned int multi_cmd_start            :1;
}vetop_reg_argb_cmd_queue_start_t;


//* 0x60 //* register not used int 1633.
typedef struct VETOP_REG_ARGB_SOURCE1_BLOCK_ADDR
{
    volatile unsigned int addr;
}vetop_reg_argb_src1_blk_addr_t;


//* 0x64 //* register not used int 1633.
typedef struct VETOP_REG_ARGB_SOURCE2_BLOCK_ADDR
{
    volatile unsigned int addr;
}vetop_reg_argb_src2_blk_addr_t;


//* 0x68 //* register not used int 1633.
typedef struct VETOP_REG_ARGB_TARGET_BLOCK_ADDR
{
    volatile unsigned int addr;
}vetop_reg_argb_target_blk_addr_t;


//* 0x6c //* register not used int 1633.
typedef struct VETOP_REG_ARGB_SRC_PIXEL_STRIDE
{
    volatile unsigned int stride_src1                :16;
    volatile unsigned int stride_src2                :16;
}vetop_reg_argb_scr_pix_stride_t;

//* the following register 70~78 is redefined for 1639.
typedef struct VETOP_REG_CCI400_CTRL0
{
    volatile unsigned int dec_qcmd                    :18;
    volatile unsigned int reserved0                    :5;
    volatile unsigned int dec_qvld                    :1;
    volatile unsigned int gtb_swon                    :8;
}vetop_reg_cci400_ctrl0;

typedef struct VETOP_REG_CCI400_CTRL1
{
    volatile unsigned int enc_qcmd                    :18;
    volatile unsigned int reserved0                    :5;
    volatile unsigned int enc_qvld                    :1;
    volatile unsigned int gtb_swoff                    :8;
}vetop_reg_cci400_ctrl1;

typedef struct VETOP_REG_CCI400_CTRL2
{
    volatile unsigned int mbus_rnbr                    :16;
    volatile unsigned int mbus_wait                    :8;
    volatile unsigned int dec_wdram_end                :1;
    volatile unsigned int dec_wdram_clr                :1;
    volatile unsigned int enc_wdram_end                :1;
    volatile unsigned int enc_wdram_clr                :1;
    volatile unsigned int reserved0                    :4;
}vetop_reg_cci400_ctrl2;


//* 0x70 //* register not used int 1633.
typedef struct VETOP_REG_ARGB_TARGET_PIXEL_STRIDE
{
    volatile unsigned int stride_target              :16;
    volatile unsigned int alpha_global               :8;
    volatile unsigned int reserved0                  :8;
}vetop_reg_argb_target_pix_stride_t;


//* 0x74 //* register not used int 1633.
typedef struct VETOP_REG_ARGB_BLOCK_SIZE
{
    volatile unsigned int blk_size_x                 :16;
    volatile unsigned int blk_size_y                 :16;
}vetop_reg_argb_blk_size_t;


//* 0x78 //* register not used int 1633.
typedef struct VETOP_REG_ARGB_VALUE_TO_FILL
{
    volatile unsigned int argb_value;
}vetop_reg_argb_value_to_fill_t;


//* 0x7c //* register not used int 1633.
typedef struct VETOP_REG_ARGB_BLOCK_CONTROL
{
    volatile unsigned int blk_ctrl               :2;
    volatile unsigned int alpha_mode             :2;
    volatile unsigned int bottom_line_first      :1;
    volatile unsigned int alpha_low_bits         :1;
    volatile unsigned int reserved0              :25;
    volatile unsigned int blk_start              :1;
}vetop_reg_argb_blk_ctrl_t;


//* 0x80 - 0x8c 4 registers. //* register not used int 1633.
typedef struct VETOP_REG_LUMA_HISTOGRAM_THRESHOLD
{
    volatile unsigned int threshold0                 :8;
    volatile unsigned int threshold1                 :8;
    volatile unsigned int threshold2                 :8;
    volatile unsigned int threshold3                 :8;
}vetop_reg_luma_hist_threshold_t;


//* 0x90 - 0xcc 16 registers.
typedef struct VETOP_REG_LUMA_HISTOGRAM_VALUE
{
    volatile unsigned int value                      :20;
    volatile unsigned int reserved0                  :12;
}vetop_reg_luma_hist_value_t;


//* for 1618, 1629, 1623, ..., 0xd0 - 0xf4 10 registers are reserved.
//* for 1619, 0xd0 - 0xdc 4 registers are reserved.
//* 0xd0.
typedef struct VETOP_REG_ANAGLYPH_RBUF_ADDR
{
    volatile unsigned int anaglyph_out_rbuf_addr:     32;
}vetop_reg_anaglyph_output_rbuf_addr_t;

//* 0xd4.
typedef struct VETOP_REG_ANAGLYPH_GBUF_ADDR
{
    volatile unsigned int anaglyph_out_gbuf_addr:     32;
}vetop_reg_anaglyph_output_gbuf_addr_t;

//* 0xd8
typedef struct VETOP_REG_ANAGLYPH_BBUF_ADDR
{
    volatile unsigned int anaglyph_out_bbuf_addr:     32;
}vetop_reg_anaglyph_output_bbuf_addr_t;

//* 0xe0 //* register not used in 1633.
typedef struct VETOP_REG_SRAM_OFFSET
{
    volatile unsigned int addr                       :12;
    volatile unsigned int reserved0                  :20;
}vetop_reg_sram_offset_t;


//* 0xe4 //* register not used in 1633.
typedef struct VETOP_REG_SRAM_DATA
{
    volatile unsigned int data;
}vetop_reg_sram_data_t;

//* 0xe8 for 1633
typedef struct VETOP_REG_CHROMA_BUF_LEN0
{
    volatile unsigned int sdrt_chroma_buf_len       :28;
    //volatile unsigned int reserved0                 :1;
    volatile unsigned int luma_align_mode            :1;
    //* 0: 16-bytes aligned, 1: 32-bytes aligned. for 1639
    volatile unsigned int chroma_align_mode         :1;
    //* 0: 16-bytes aligned, 1: 8-bytes aligned.
    volatile unsigned int output_data_format        :2;
    //* 00: 32x32 tile-based, 01: reserved, 10: YUV planer format, 11: YVU planer foramt.

    //* when planner format output, the line stride of each component is the actual pixel width.
}vetop_reg_chroma_buf_len0;

//* 0xec - 0xf4 4 registers are reserved.

//* 0xec for 1639
typedef struct VETOP_REG_PRI_OUTPUT_FORMAT
{
       volatile unsigned int second_special_output_format        :3;
       //* 000: 32*32 tile-based, 001: 128*32 tile-based, 010: YU12, 011: YV12, 100: NV12, 101: NU12
       volatile unsigned int reserved0                            :1;
       volatile unsigned int primary_output_format                :3;
       //* 0: 16-bytes aligned, 1: 8-bytes aligned.
       volatile unsigned int reserved1                            :25;
}vetop_reg_pri_output_format;

//* 0xf8 //* register not used in 1633.
typedef struct VETOP_REG_DEBUG_CONTROL
{
    volatile unsigned int dbg_ctrl      :15;
    volatile unsigned int reserved0     :17;
}vetop_reg_dbg_ctrl_t;


//* 0xfc //* register not used in 1633.
typedef struct VETOP_REG_DEBUG_OUTPUT
{
    volatile unsigned int dbg_output    :32;
}vetop_reg_dbg_output_t;


// register is used for aw1667
typedef struct VETOP_REG_PRI_CHROMA_BUF_LEN
{
    volatile unsigned int pri_chroma_buf_len        :32;
}vetop_reg_pri_chroma_buf_len_t;

typedef struct VETOP_REG_PRI_BUF_LINE_STRIDE
{
    volatile unsigned int  pri_luma_line_stride      :16;
    volatile unsigned int  pri_chroma_line_stride    :16;
}vetop_reg_pri_buf_line_stride_t;

typedef struct VETOP_REG_SEC_BUF_LINE_STRIDE
{
    volatile unsigned int  sec_luma_line_stride      :16;
       volatile unsigned int  sec_chroma_line_stride    :16;
}vetop_reg_sec_buf_line_stride_t;


//* define VE top level register list.
typedef struct VETOP_REGISTER_LIST
{
    volatile vetop_reg_mode_sel_t           _00_mode_sel;
    vetop_reg_reset_t                       _04_reset;
    vetop_reg_codec_cntr_t                  _08_codec_cntr;
    vetop_reg_codec_overtime_t              _0c_overtime;
    vetop_reg_mmc_req_wnum_t                _10_mmc_req;
    vetop_reg_cache_req_wnum_t              _14_cache_req;
    unsigned int                            _18_reserved0;
    vetop_reg_status_t                      _1c_status;
    vetop_reg_dram_read_cntr_t              _20_dram_read_cntr;
    vetop_reg_dram_write_cntr_t             _24_dram_write_cntr;

    vetop_reg_anaglyph_cntr_t               _28_anaglyph_cntr;
    unsigned int                            _2c_reserved1;

    //* the following 8 registers are for 1619, 1629, 1623, ...
    vetop_reg_maf_ctrl_t                    _30_maf_ctrl;
    //* not used in 1633.
    vetop_reg_maf_th_clip_t                 _34_maf_th_clip;
    //* not used in 1633.
    vetop_reg_maf_ref1_luma_addr_t          _38_maf_ref1_luma_addr;
    //* not used in 1633.
    vetop_reg_maf_ref1_chroma_addr_t        _3c_maf_ref1_chroma_addr;
    //* not used in 1633.
    vetop_reg_cur_frame_maf_addr_t          _40_cur_maf_addr;
    //* not used in 1633.
    vetop_reg_ref1_frame_maf_addr_t         _44_ref1_maf_addr;
    //* not used in 1633.
    vetop_reg_ref2_frame_maf_addr_t         _48_ref2_maf_addr;
    //* not used in 1633.
    vetop_reg_maf_max_group_diff_t          _4c_maf_max_group_diff;
    //* not used in 1633.

    //* the following 12 registers are for 1619.
    vetop_reg_deblk_intrapred_buf_ctrl_t    _50_deblk_intrapred_buf_ctrl;
    vetop_reg_deblk_dram_buf_addr_t         _54_deblk_dram_buf;
    vetop_reg_intrapred_dram_buf_addr_t     _58_intrapred_dram_buf;
    vetop_reg_argb_cmd_queue_start_t        _5c_argb_cmd_queue_start;
    //* not used in 1633.
    vetop_reg_argb_src1_blk_addr_t          _60_argb_src1_blk_addr;
    //* not used in 1633.
    vetop_reg_argb_src2_blk_addr_t          _64_argb_src2_blk_addr;
    //* not used in 1633.
    vetop_reg_argb_target_blk_addr_t        _68_argb_target_blk_addr;
    //* not used in 1633.
    vetop_reg_argb_scr_pix_stride_t         _6c_argb_src_pix_stride;
    //* not used in 1633.
    vetop_reg_argb_target_pix_stride_t      _70_argb_target_pix_stride;
    //* not used in 1633.
    vetop_reg_argb_blk_size_t               _74_argb_blk_size;
    //* not used in 1633.
    vetop_reg_argb_value_to_fill_t          _78_argb_value_to_fill;
    //* not used in 1633.
    vetop_reg_argb_blk_ctrl_t               _7c_argb_blk_ctrl;
    //* not used in 1633.

    vetop_reg_luma_hist_threshold_t         _80_luma_hist_threshold[4];
    //* not used in 1633.
    vetop_reg_luma_hist_value_t             _90_luma_hist_value[12];
    //* not used in 1633.

    //For 1667
    unsigned int                             _c0_reserved;
    vetop_reg_pri_chroma_buf_len_t           _c4_pri_chroma_buf_len;
    vetop_reg_pri_buf_line_stride_t          _c8_pri_buf_line_stride;
    vetop_reg_sec_buf_line_stride_t          _cc_sec_buf_line_stride;


    vetop_reg_anaglyph_output_rbuf_addr_t   _d0_anaglyph_output_rbuf_addr;
    vetop_reg_anaglyph_output_gbuf_addr_t   _d4_anaglyph_output_gbuf_addr;
    vetop_reg_anaglyph_output_bbuf_addr_t   _d8_anaglyph_output_bbuf_addr;
    //For 1633
    unsigned int                            _dc_reserved[3];

    vetop_reg_chroma_buf_len0                _e8_chroma_buf_len0;

    //unsigned int                            _ec_reserved[3];
    vetop_reg_pri_output_format                _ec_pri_output_format;

    unsigned int                            _f0_reserved[2];
    vetop_reg_dbg_ctrl_t                    _f8_dbg_ctrl;
    //* not used in 1633.
    vetop_reg_dbg_output_t                  _fc_dbg_output;
    //* not used in 1633.

}vetop_reglist_t;

static __inline void* ve_get_reglist(ve_register_group_e reg_group_id)
{
    void*           reglist;
    size_addr    tmp;

    tmp = (size_addr)AdapterVeGetBaseAddress();

    tmp += REG_GROUP_OFFSET_ARR[reg_group_id];

    reglist = (void*)tmp;

    return reglist;
}

static __inline void ve_print_regs(ve_register_group_e reg_group_id,
    unsigned int offset, unsigned int count)
{
    unsigned int  i;
    size_addr  tmp;
    unsigned int* base;
    unsigned int nOffset = 0;

    nOffset = offset;

    tmp = (size_addr)AdapterVeGetBaseAddress();

    base = (unsigned int*)(tmp + REG_GROUP_OFFSET_ARR[reg_group_id]);

    logv("print register of %s, start offset = %d, count = %d", /
        REG_GROUP_NAME[reg_group_id], offset, count);
    for(i=0; i<count; i++)
    {
        logv("    reg[%2.2x] = %8.8x", offset + 4*i, base[(offset>>2) + i]);
    }

    logv("\n");
}

extern unsigned int gVeVersion;
static __inline unsigned int get_ve_version_id(void)
{
    return gVeVersion;
}

#define SetRegValue(REG, VALUE)     (*((u32*)&REG) = (VALUE))
#define GetRegValue(REG)            (*((u32*)&REG))

#endif
