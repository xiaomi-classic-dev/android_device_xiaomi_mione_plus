/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __QCAMERA_INTF_H__
#define __QCAMERA_INTF_H__

#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>
#include <media/msm_camera.h>
#include <linux/videodev2.h>
#include <linux/msm_ion.h>
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#define CACHED ION_FLAG_CACHED
#define UNCACHED 0
typedef struct { float min_fps; float max_fps; } cam_sensor_fps_range_t;

#define PAD_TO_WORD(a)               (((a)+3)&~3)
#define PAD_TO_2K(a)                 (((a)+2047)&~2047)
#define PAD_TO_4K(a)                 (((a)+4095)&~4095)
#define PAD_TO_8K(a)                 (((a)+8191)&~8191)

#define CEILING32(X) (((X) + 0x0001F) & 0xFFFFFFE0)
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define CEILING4(X)  (((X) + 0x0003) & 0xFFFC)
#define CEILING2(X)  (((X) + 0x0001) & 0xFFFE)

/* No sharpness for default */
#define CAMERA_MIN_SHARPNESS   0
#define CAMERA_DEF_SHARPNESS   10
#define CAMERA_MAX_SHARPNESS   30
#define CAMERA_SHARPNESS_STEP  5

/* No saturation for default */
#define CAMERA_MIN_SATURATION  0
#define CAMERA_DEF_SATURATION  5
#define CAMERA_MAX_SATURATION  10
#define CAMERA_SATURATION_STEP 1

#define CAMERA_MIN_BRIGHTNESS  0
#define CAMERA_DEF_BRIGHTNESS  3
#define CAMERA_MAX_BRIGHTNESS  6
#define CAMERA_BRIGHTNESS_STEP 1

#define CAMERA_MIN_CONTRAST    0
#define CAMERA_DEF_CONTRAST    5
#define CAMERA_MAX_CONTRAST    10
#define CAMERA_CONTRAST_STEP   1

/* No hue for default. */
#define CAMERA_MIN_HUE         0
#define CAMERA_DEF_HUE         0
#define CAMERA_MAX_HUE         300
#define CAMERA_HUE_STEP        60

#define CAMERA_MIN_ZOOM  0
#define CAMERA_DEF_ZOOM  0
#define CAMERA_MAX_ZOOM  0x31
#define CAMERA_ZOOM_STEP 0x3

#define ANDROID_FB0 "/dev/graphics/fb0"
#define LE_FB0 "/dev/fb0"

// Events common to encoder and decoder
#define JPEG_EVENT_DONE      0
#define JPEG_EVENT_WARNING   1
#define JPEG_EVENT_ERROR     2
#define JPEG_EVENT_ABORTED   3
// Events specific to encoder
#define JPEG_EVENT_THUMBNAIL_DROPPED 4


#define MAX_ROI 2
#define MAX_NUM_PARM 5
#define MAX_NUM_OPS 2
#define VIDEO_MAX_PLANES 8
#define MAX_SNAPSHOT_BUFFERS 5
#define MAX_EXP_BRACKETING_LENGTH 32


/* Exif Tag ID */
typedef uint32_t exif_tag_id_t;

/* Exif Info (opaque definition) */
struct exif_info_t;
typedef struct exif_info_t * exif_info_obj_t;

typedef enum {
  BACK_CAMERA,
  FRONT_CAMERA,
}cam_position_t;

typedef enum {
  CAM_CTRL_FAILED,        /* Failure in doing operation */
  CAM_CTRL_SUCCESS,       /* Operation Succeded */
  CAM_CTRL_INVALID_PARM,  /* Inavlid parameter provided */
  CAM_CTRL_NOT_SUPPORTED, /* Parameter/operation not supported */
  CAM_CTRL_ACCEPTED,      /* Parameter accepted */
  CAM_CTRL_MAX,
} cam_ctrl_status_t;

typedef enum {
  CAMERA_YUV_420_NV12,
  CAMERA_YUV_420_NV21,
  CAMERA_YUV_420_NV21_ADRENO,
  CAMERA_BAYER_SBGGR10,
  CAMERA_RDI,
  CAMERA_YUV_420_YV12,
  CAMERA_YUV_422_NV16,
  CAMERA_YUV_422_NV61
} cam_format_t;

typedef enum {
  CAMERA_PAD_NONE,
  CAMERA_PAD_TO_WORD,   /*2 bytes*/
  CAMERA_PAD_TO_LONG_WORD, /*4 bytes*/
  CAMERA_PAD_TO_8, /*8 bytes*/
  CAMERA_PAD_TO_16, /*16 bytes*/

  CAMERA_PAD_TO_1K, /*1k bytes*/
  CAMERA_PAD_TO_2K, /*2k bytes*/
  CAMERA_PAD_TO_4K,
  CAMERA_PAD_TO_8K
} cam_pad_format_t;

typedef struct {
  int ext_mode;   /* preview, main, thumbnail, video, raw, etc */
  int frame_idx;  /* frame index */
  int fd;         /* origin fd */
  uint32_t size;
} mm_camera_frame_map_type;

typedef struct {
  int ext_mode;   /* preview, main, thumbnail, video, raw, etc */
  int frame_idx;  /* frame index */
} mm_camera_frame_unmap_type;

typedef enum {
  CAM_SOCK_MSG_TYPE_FD_MAPPING,
  CAM_SOCK_MSG_TYPE_FD_UNMAPPING,
  CAM_SOCK_MSG_TYPE_WDN_START,
  CAM_SOCK_MSG_TYPE_HIST_MAPPING,
  CAM_SOCK_MSG_TYPE_HIST_UNMAPPING,
  CAM_SOCK_MSG_TYPE_MAX
}mm_camera_socket_msg_type;

#define MM_MAX_WDN_NUM 2
typedef struct {
  unsigned long cookie;
  int num_frames;
  int ext_mode[MM_MAX_WDN_NUM];
  int frame_idx[MM_MAX_WDN_NUM];
} mm_camera_wdn_start_type;

typedef struct {
  mm_camera_socket_msg_type msg_type;
  union {
    mm_camera_frame_map_type frame_fd_map;
    mm_camera_frame_unmap_type frame_fd_unmap;
    mm_camera_wdn_start_type wdn_start;
  } payload;
} cam_sock_packet_t;


typedef enum {
  CAM_VIDEO_FRAME,
  CAM_SNAPSHOT_FRAME,
  CAM_PREVIEW_FRAME,
}cam_frame_type_t;


typedef enum {
  CAMERA_MODE_2D = (1<<0),
  CAMERA_MODE_3D = (1<<1),
  CAMERA_NONZSL_MODE = (1<<2),
  CAMERA_ZSL_MODE = (1<<3),
  CAMERA_MODE_MAX = CAMERA_ZSL_MODE,
} camera_mode_t;


typedef struct {
  int  modes_supported;
  int8_t camera_id;
  cam_position_t position;
  uint32_t sensor_mount_angle;
}qcamera_info_t;

typedef struct {
  camera_mode_t mode;
  int8_t camera_id;
  camera_mode_t cammode;
}config_params_t;

typedef struct {
  uint32_t len;
  uint32_t y_offset;
  uint32_t cbcr_offset;
} cam_sp_len_offset_t;

typedef struct{
  uint32_t len;
  uint32_t offset;
} cam_mp_len_offset_t;

typedef struct {
  int num_planes;
  union {
    cam_sp_len_offset_t sp;
    cam_mp_len_offset_t mp[8];
  };
  uint32_t frame_len;
} cam_frame_len_offset_t;

typedef struct {
  uint32_t parm[MAX_NUM_PARM];
  uint32_t ops[MAX_NUM_OPS];
  uint8_t yuv_output;
  uint8_t jpeg_capture;
  uint32_t max_pict_width;
  uint32_t max_pict_height;
  uint32_t max_preview_width;
  uint32_t max_preview_height;
  uint32_t max_video_width;
  uint32_t max_video_height;
  uint32_t effect;
  camera_mode_t modes;
  uint8_t preview_format;
  uint32_t preview_sizes_cnt;
  uint32_t thumb_sizes_cnt;
  uint32_t video_sizes_cnt;
  uint32_t hfr_sizes_cnt;
  uint8_t vfe_output_enable;
  uint8_t hfr_frame_skip;
  uint32_t default_preview_width;
  uint32_t default_preview_height;
  uint32_t bestshot_reconfigure;
}cam_prop_t;

typedef struct {
  uint16_t video_width;         /* Video width seen by VFE could be different than orig. Ex. DIS */
  uint16_t video_height;        /* Video height seen by VFE */
  uint16_t picture_width;       /* Picture width seen by VFE */
  uint16_t picture_height;      /* Picture height seen by VFE */
  uint16_t display_width;       /* width of display */
  uint16_t display_height;      /* height of display */
  uint16_t orig_video_width;    /* original video width received */
  uint16_t orig_video_height;   /* original video height received */
  uint16_t orig_picture_dx;     /* original picture width received */
  uint16_t orig_picture_dy;     /* original picture height received */
  uint16_t ui_thumbnail_height; /* Just like orig_picture_dx */
  uint16_t ui_thumbnail_width;  /* Just like orig_picture_dy */
  uint16_t thumbnail_height;
  uint16_t thumbnail_width;
  uint16_t orig_picture_width;
  uint16_t orig_picture_height;
  uint16_t orig_thumb_width;
  uint16_t orig_thumb_height;
  uint16_t raw_picture_height;
  uint16_t raw_picture_width;
  uint16_t rdi0_height;
  uint16_t rdi0_width;
  uint16_t rdi1_height;
  uint16_t rdi1_width;
  uint32_t hjr_xtra_buff_for_bayer_filtering;
  cam_format_t    prev_format;
  cam_format_t    enc_format;
  cam_format_t    thumb_format;
  cam_format_t    main_img_format;
  cam_format_t    rdi0_format;
  cam_format_t    rdi1_format;
  cam_pad_format_t prev_padding_format;
  cam_pad_format_t enc_padding_format;
  cam_pad_format_t thumb_padding_format;
  cam_pad_format_t main_padding_format;
  uint16_t display_luma_width;
  uint16_t display_luma_height;
  uint16_t display_chroma_width;
  uint16_t display_chroma_height;
  uint16_t video_luma_width;
  uint16_t video_luma_height;
  uint16_t video_chroma_width;
  uint16_t video_chroma_height;
  uint16_t thumbnail_luma_width;
  uint16_t thumbnail_luma_height;
  uint16_t thumbnail_chroma_width;
  uint16_t thumbnail_chroma_height;
  uint16_t main_img_luma_width;
  uint16_t main_img_luma_height;
  uint16_t main_img_chroma_width;
  uint16_t main_img_chroma_height;
  int rotation;
  cam_frame_len_offset_t display_frame_offset;
  cam_frame_len_offset_t video_frame_offset;
  cam_frame_len_offset_t picture_frame_offset;
  cam_frame_len_offset_t thumb_frame_offset;
  uint32_t channel_interface_mask;
} cam_ctrl_dimension_t;

/* Add enumenrations at the bottom but before MM_CAMERA_PARM_MAX */
typedef enum {
    MM_CAMERA_PARM_PICT_SIZE,
    MM_CAMERA_PARM_ZOOM_RATIO,
    MM_CAMERA_PARM_HISTOGRAM,
    MM_CAMERA_PARM_DIMENSION,
    MM_CAMERA_PARM_FPS,
    MM_CAMERA_PARM_FPS_MODE, /*5*/
    MM_CAMERA_PARM_EFFECT,
    MM_CAMERA_PARM_EXPOSURE_COMPENSATION,
    MM_CAMERA_PARM_EXPOSURE,
    MM_CAMERA_PARM_SHARPNESS,
    MM_CAMERA_PARM_CONTRAST, /*10*/
    MM_CAMERA_PARM_SATURATION,
    MM_CAMERA_PARM_BRIGHTNESS,
    MM_CAMERA_PARM_WHITE_BALANCE,
    MM_CAMERA_PARM_LED_MODE,
    MM_CAMERA_PARM_ANTIBANDING, /*15*/
    MM_CAMERA_PARM_ROLLOFF,
    MM_CAMERA_PARM_CONTINUOUS_AF,
    MM_CAMERA_PARM_FOCUS_RECT,
    MM_CAMERA_PARM_AEC_ROI,
    MM_CAMERA_PARM_AF_ROI, /*20*/
    MM_CAMERA_PARM_HJR,
    MM_CAMERA_PARM_ISO,
    MM_CAMERA_PARM_BL_DETECTION,
    MM_CAMERA_PARM_SNOW_DETECTION,
    MM_CAMERA_PARM_BESTSHOT_MODE, /*25*/
    MM_CAMERA_PARM_ZOOM,
    MM_CAMERA_PARM_VIDEO_DIS,
    MM_CAMERA_PARM_VIDEO_ROT,
    MM_CAMERA_PARM_SCE_FACTOR,
    MM_CAMERA_PARM_FD, /*30*/
    MM_CAMERA_PARM_MODE,
    /* 2nd 32 bits */
    MM_CAMERA_PARM_3D_FRAME_FORMAT,
    MM_CAMERA_PARM_CAMERA_ID,
    MM_CAMERA_PARM_CAMERA_INFO,
    MM_CAMERA_PARM_PREVIEW_SIZE, /*35*/
    MM_CAMERA_PARM_QUERY_FALSH4SNAP,
    MM_CAMERA_PARM_FOCUS_DISTANCES,
    MM_CAMERA_PARM_BUFFER_INFO,
    MM_CAMERA_PARM_JPEG_ROTATION,
    MM_CAMERA_PARM_JPEG_MAINIMG_QUALITY, /* 40 */
    MM_CAMERA_PARM_JPEG_THUMB_QUALITY,
    MM_CAMERA_PARM_ZSL_ENABLE,
    MM_CAMERA_PARM_FOCAL_LENGTH,
    MM_CAMERA_PARM_HORIZONTAL_VIEW_ANGLE,
    MM_CAMERA_PARM_VERTICAL_VIEW_ANGLE, /* 45 */
    MM_CAMERA_PARM_MCE,
    MM_CAMERA_PARM_RESET_LENS_TO_INFINITY,
    MM_CAMERA_PARM_SNAPSHOTDATA,
    MM_CAMERA_PARM_HFR,
    MM_CAMERA_PARM_REDEYE_REDUCTION, /* 50 */
    MM_CAMERA_PARM_WAVELET_DENOISE,
    MM_CAMERA_PARM_3D_DISPLAY_DISTANCE,
    MM_CAMERA_PARM_3D_VIEW_ANGLE,
    MM_CAMERA_PARM_PREVIEW_FORMAT,
    MM_CAMERA_PARM_RDI_FORMAT,
    MM_CAMERA_PARM_HFR_SIZE, /* 55 */
    MM_CAMERA_PARM_3D_EFFECT,
    MM_CAMERA_PARM_3D_MANUAL_CONV_RANGE,
    MM_CAMERA_PARM_3D_MANUAL_CONV_VALUE,
    MM_CAMERA_PARM_ENABLE_3D_MANUAL_CONVERGENCE,
    /* These are new parameters defined here */
    MM_CAMERA_PARM_CH_IMAGE_FMT, /* 60 */       // mm_camera_ch_image_fmt_parm_t
    MM_CAMERA_PARM_OP_MODE,             // camera state, sub state also
    MM_CAMERA_PARM_SHARPNESS_CAP,       //
    MM_CAMERA_PARM_SNAPSHOT_BURST_NUM,  // num shots per snapshot action
    MM_CAMERA_PARM_LIVESHOT_MAIN,       // enable/disable full size live shot
    MM_CAMERA_PARM_MAXZOOM, /* 65 */
    MM_CAMERA_PARM_LUMA_ADAPTATION,     // enable/disable
    MM_CAMERA_PARM_HDR,
    MM_CAMERA_PARM_CROP,
    MM_CAMERA_PARM_MAX_PICTURE_SIZE,
    MM_CAMERA_PARM_MAX_PREVIEW_SIZE, /* 70 */
    MM_CAMERA_PARM_ASD_ENABLE,
    MM_CAMERA_PARM_RECORDING_HINT,
    MM_CAMERA_PARM_CAF_ENABLE,
    MM_CAMERA_PARM_FULL_LIVESHOT,
    MM_CAMERA_PARM_DIS_ENABLE, /* 75 */
    MM_CAMERA_PARM_AEC_LOCK,
    MM_CAMERA_PARM_AWB_LOCK,
    MM_CAMERA_PARM_AF_MTR_AREA,
    MM_CAMERA_PARM_AEC_MTR_AREA,
    MM_CAMERA_PARM_LOW_POWER_MODE,
    MM_CAMERA_PARM_MAX_HFR_MODE, /* 80 */
    MM_CAMERA_PARM_MAX_VIDEO_SIZE,
    MM_CAMERA_PARM_DEF_PREVIEW_SIZES,
    MM_CAMERA_PARM_DEF_VIDEO_SIZES,
    MM_CAMERA_PARM_DEF_THUMB_SIZES,
    MM_CAMERA_PARM_DEF_HFR_SIZES,
    MM_CAMERA_PARM_PREVIEW_SIZES_CNT,
    MM_CAMERA_PARM_VIDEO_SIZES_CNT,
    MM_CAMERA_PARM_THUMB_SIZES_CNT,
    MM_CAMERA_PARM_HFR_SIZES_CNT,
    MM_CAMERA_PARM_GRALLOC_USAGE,
    MM_CAMERA_PARM_VFE_OUTPUT_ENABLE, //to check whether both oputputs are
    MM_CAMERA_PARM_DEFAULT_PREVIEW_WIDTH,
    MM_CAMERA_PARM_DEFAULT_PREVIEW_HEIGHT,
    MM_CAMERA_PARM_FOCUS_MODE,
    MM_CAMERA_PARM_HFR_FRAME_SKIP,
    MM_CAMERA_PARM_CH_INTERFACE,
    //or single output enabled to differentiate 7x27a with others
    MM_CAMERA_PARM_BESTSHOT_RECONFIGURE,
    MM_CAMERA_PARM_MAX_NUM_FACES_DECT,
    MM_CAMERA_PARM_FPS_RANGE,
    MM_CAMERA_PARM_FACIAL_FEATURE_INFO,
    MM_CAMERA_PARM_MAX
} mm_camera_parm_type_t;

typedef enum {
  STREAM_IMAGE,
  STREAM_RAW,
  STREAM_IMAGE_AND_RAW,
  STREAM_RAW_AND_RAW,
  STREAM_MAX,
} mm_camera_channel_stream_info_t;

typedef enum {
  CAMERA_SET_PARM_DISPLAY_INFO,
  CAMERA_SET_PARM_DIMENSION,

  CAMERA_SET_PARM_ZOOM,
  CAMERA_SET_PARM_SENSOR_POSITION,
  CAMERA_SET_PARM_FOCUS_RECT,
  CAMERA_SET_PARM_LUMA_ADAPTATION,
  CAMERA_SET_PARM_CONTRAST,
  CAMERA_SET_PARM_BRIGHTNESS,
  CAMERA_SET_PARM_EXPOSURE_COMPENSATION,
  CAMERA_SET_PARM_SHARPNESS,
  CAMERA_SET_PARM_HUE,  /* 10 */
  CAMERA_SET_PARM_SATURATION,
  CAMERA_SET_PARM_EXPOSURE,
  CAMERA_SET_PARM_AUTO_FOCUS,
  CAMERA_SET_PARM_WB,
  CAMERA_SET_PARM_EFFECT,
  CAMERA_SET_PARM_FPS,
  CAMERA_SET_PARM_FLASH,
  CAMERA_SET_PARM_NIGHTSHOT_MODE,
  CAMERA_SET_PARM_REFLECT,
  CAMERA_SET_PARM_PREVIEW_MODE,  /* 20 */
  CAMERA_SET_PARM_ANTIBANDING,
  CAMERA_SET_PARM_RED_EYE_REDUCTION,
  CAMERA_SET_PARM_FOCUS_STEP,
  CAMERA_SET_PARM_EXPOSURE_METERING,
  CAMERA_SET_PARM_AUTO_EXPOSURE_MODE,
  CAMERA_SET_PARM_ISO,
  CAMERA_SET_PARM_BESTSHOT_MODE,
  CAMERA_SET_PARM_ENCODE_ROTATION,

  CAMERA_SET_PARM_PREVIEW_FPS,
  CAMERA_SET_PARM_AF_MODE,  /* 30 */
  CAMERA_SET_PARM_HISTOGRAM,
  CAMERA_SET_PARM_FLASH_STATE,
  CAMERA_SET_PARM_FRAME_TIMESTAMP,
  CAMERA_SET_PARM_STROBE_FLASH,
  CAMERA_SET_PARM_FPS_LIST,
  CAMERA_SET_PARM_HJR,
  CAMERA_SET_PARM_ROLLOFF,

  CAMERA_STOP_PREVIEW,
  CAMERA_START_PREVIEW,
  CAMERA_START_SNAPSHOT, /* 40 */
  CAMERA_START_RAW_SNAPSHOT,
  CAMERA_STOP_SNAPSHOT,
  CAMERA_EXIT,
  CAMERA_ENABLE_BSM,
  CAMERA_DISABLE_BSM,
  CAMERA_GET_PARM_ZOOM,
  CAMERA_GET_PARM_MAXZOOM,
  CAMERA_GET_PARM_ZOOMRATIOS,
  CAMERA_GET_PARM_AF_SHARPNESS,
  CAMERA_SET_PARM_LED_MODE, /* 50 */
  CAMERA_SET_MOTION_ISO,
  CAMERA_AUTO_FOCUS_CANCEL,
  CAMERA_GET_PARM_FOCUS_STEP,
  CAMERA_ENABLE_AFD,
  CAMERA_PREPARE_SNAPSHOT,
  CAMERA_SET_FPS_MODE,
  CAMERA_START_VIDEO,
  CAMERA_STOP_VIDEO,
  CAMERA_START_RECORDING,
  CAMERA_STOP_RECORDING, /* 60 */
  CAMERA_SET_VIDEO_DIS_PARAMS,
  CAMERA_SET_VIDEO_ROT_PARAMS,
  CAMERA_SET_PARM_AEC_ROI,
  CAMERA_SET_CAF,
  CAMERA_SET_PARM_BL_DETECTION_ENABLE,
  CAMERA_SET_PARM_SNOW_DETECTION_ENABLE,
  CAMERA_SET_PARM_STROBE_FLASH_MODE,
  CAMERA_SET_PARM_AF_ROI,
  CAMERA_START_LIVESHOT,
  CAMERA_SET_SCE_FACTOR, /* 70 */
  CAMERA_GET_CAPABILITIES,
  CAMERA_GET_PARM_DIMENSION,
  CAMERA_GET_PARM_LED_MODE,
  CAMERA_SET_PARM_FD,
  CAMERA_GET_PARM_3D_FRAME_FORMAT,
  CAMERA_QUERY_FLASH_FOR_SNAPSHOT,
  CAMERA_GET_PARM_FOCUS_DISTANCES,
  CAMERA_START_ZSL,
  CAMERA_STOP_ZSL,
  CAMERA_ENABLE_ZSL, /* 80 */
  CAMERA_GET_PARM_FOCAL_LENGTH,
  CAMERA_GET_PARM_HORIZONTAL_VIEW_ANGLE,
  CAMERA_GET_PARM_VERTICAL_VIEW_ANGLE,
  CAMERA_SET_PARM_WAVELET_DENOISE,
  CAMERA_SET_PARM_MCE,
  CAMERA_ENABLE_STEREO_CAM,
  CAMERA_SET_PARM_RESET_LENS_TO_INFINITY,
  CAMERA_GET_PARM_SNAPSHOTDATA,
  CAMERA_SET_PARM_HFR,
  CAMERA_SET_REDEYE_REDUCTION, /* 90 */
  CAMERA_SET_PARM_3D_DISPLAY_DISTANCE,
  CAMERA_SET_PARM_3D_VIEW_ANGLE,
  CAMERA_SET_PARM_3D_EFFECT,
  CAMERA_SET_PARM_PREVIEW_FORMAT,
  CAMERA_GET_PARM_3D_DISPLAY_DISTANCE, /* 95 */
  CAMERA_GET_PARM_3D_VIEW_ANGLE,
  CAMERA_GET_PARM_3D_EFFECT,
  CAMERA_GET_PARM_3D_MANUAL_CONV_RANGE,
  CAMERA_SET_PARM_3D_MANUAL_CONV_VALUE,
  CAMERA_ENABLE_3D_MANUAL_CONVERGENCE, /* 100 */
  CAMERA_SET_PARM_HDR,
  CAMERA_SET_ASD_ENABLE,
  CAMERA_POSTPROC_ABORT,
  CAMERA_SET_AEC_MTR_AREA,
  CAMERA_SET_AEC_LOCK,       /*105*/
  CAMERA_SET_AWB_LOCK,
  CAMERA_SET_RECORDING_HINT,
  CAMERA_SET_PARM_CAF,
  CAMERA_SET_FULL_LIVESHOT,
  CAMERA_SET_DIS_ENABLE,  /*110*/
  CAMERA_GET_PARM_MAX_HFR_MODE,
  CAMERA_SET_LOW_POWER_MODE,
  CAMERA_GET_PARM_DEF_PREVIEW_SIZES,
  CAMERA_GET_PARM_DEF_VIDEO_SIZES,
  CAMERA_GET_PARM_DEF_THUMB_SIZES, /*115*/
  CAMERA_GET_PARM_DEF_HFR_SIZES,
  CAMERA_GET_PARM_MAX_LIVESHOT_SIZE,
  CAMERA_GET_PARM_FPS_RANGE,
  CAMERA_SET_3A_CONVERGENCE,
  CAMERA_SET_PREVIEW_HFR, /*120*/
  CAMERA_GET_MAX_DIMENSION,
  CAMERA_GET_MAX_NUM_FACES_DECT,
  CAMERA_SET_CHANNEL_STREAM,
  CAMERA_GET_CHANNEL_STREAM,
  CAMERA_GET_FACIAL_FEATURE_INFO,
  CAMERA_CTRL_PARM_MAX
} cam_ctrl_type;

typedef enum {
  CAMERA_ERROR_NO_MEMORY,
  CAMERA_ERROR_EFS_FAIL,                /* Low-level operation failed */
  CAMERA_ERROR_EFS_FILE_OPEN,           /* File already opened */
  CAMERA_ERROR_EFS_FILE_NOT_OPEN,       /* File not opened */
  CAMERA_ERROR_EFS_FILE_ALREADY_EXISTS, /* File already exists */
  CAMERA_ERROR_EFS_NONEXISTENT_DIR,     /* User directory doesn't exist */
  CAMERA_ERROR_EFS_NONEXISTENT_FILE,    /* User directory doesn't exist */
  CAMERA_ERROR_EFS_BAD_FILE_NAME,       /* Client specified invalid file/directory name*/
  CAMERA_ERROR_EFS_BAD_FILE_HANDLE,     /* Client specified invalid file/directory name*/
  CAMERA_ERROR_EFS_SPACE_EXHAUSTED,     /* Out of file system space */
  CAMERA_ERROR_EFS_OPEN_TABLE_FULL,     /* Out of open-file table slots                */
  CAMERA_ERROR_EFS_OTHER_ERROR,         /* Other error                                 */
  CAMERA_ERROR_CONFIG,
  CAMERA_ERROR_EXIF_ENCODE,
  CAMERA_ERROR_VIDEO_ENGINE,
  CAMERA_ERROR_IPL,
  CAMERA_ERROR_INVALID_FORMAT,
  CAMERA_ERROR_TIMEOUT,
  CAMERA_ERROR_ESD,
  CAMERA_ERROR_MAX
} camera_error_type;

#if defined CAMERA_ANTIBANDING_OFF
#undef CAMERA_ANTIBANDING_OFF
#endif

#if defined CAMERA_ANTIBANDING_60HZ
#undef CAMERA_ANTIBANDING_60HZ
#endif

#if defined CAMERA_ANTIBANDING_50HZ
#undef CAMERA_ANTIBANDING_50HZ
#endif

#if defined CAMERA_ANTIBANDING_AUTO
#undef CAMERA_ANTIBANDING_AUTO
#endif

typedef enum {
  CAMERA_ANTIBANDING_OFF,
  CAMERA_ANTIBANDING_60HZ,
  CAMERA_ANTIBANDING_50HZ,
  CAMERA_ANTIBANDING_AUTO,
  CAMERA_ANTIBANDING_AUTO_50HZ,
  CAMERA_ANTIBANDING_AUTO_60HZ,
  CAMERA_MAX_ANTIBANDING,
} camera_antibanding_type;

/* Enum Type for different ISO Mode supported */
typedef enum {
  CAMERA_ISO_AUTO = 0,
  CAMERA_ISO_DEBLUR,
  CAMERA_ISO_100,
  CAMERA_ISO_200,
  CAMERA_ISO_400,
  CAMERA_ISO_800,
  CAMERA_ISO_1600,
  CAMERA_ISO_MAX
} camera_iso_mode_type;

typedef enum {
  MM_CAMERA_FACIAL_FEATURE_FD, // facial detection
  MM_CAMERA_FACIAL_FEATURE_MAX
} camera_facial_features;

typedef enum {
  AEC_ROI_OFF,
  AEC_ROI_ON
} aec_roi_ctrl_t;

typedef enum {
  AEC_ROI_BY_INDEX,
  AEC_ROI_BY_COORDINATE,
} aec_roi_type_t;

typedef struct {
  uint32_t x;
  uint32_t y;
} cam_coordinate_type_t;

/*
 * Define DRAW_RECTANGLES to draw rectangles on screen. Just for test purpose.
 */
//#define DRAW_RECTANGLES

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t dx;
  uint16_t dy;
} roi_t;

typedef struct {
  aec_roi_ctrl_t aec_roi_enable;
  aec_roi_type_t aec_roi_type;
  union {
    cam_coordinate_type_t coordinate;
    uint32_t aec_roi_idx;
  } aec_roi_position;
} cam_set_aec_roi_t;

typedef struct {
  uint32_t frm_id;
  uint8_t num_roi;
  roi_t roi[MAX_ROI];
  uint8_t is_multiwindow;
} roi_info_t;

/* Exif Tag Data Type */
typedef enum
{
    EXIF_BYTE      = 1,
    EXIF_ASCII     = 2,
    EXIF_SHORT     = 3,
    EXIF_LONG      = 4,
    EXIF_RATIONAL  = 5,
    EXIF_UNDEFINED = 7,
    EXIF_SLONG     = 9,
    EXIF_SRATIONAL = 10
} exif_tag_type_t;


/* Exif Rational Data Type */
typedef struct
{
    uint32_t  num;    // Numerator
    uint32_t  denom;  // Denominator

} rat_t;

/* Exif Signed Rational Data Type */
typedef struct
{
    int32_t  num;    // Numerator
    int32_t  denom;  // Denominator

} srat_t;

typedef struct
{
  exif_tag_type_t type;
  uint8_t copy;
  uint32_t count;
  union
  {
    char      *_ascii;
    uint8_t   *_bytes;
    uint8_t    _byte;
    uint16_t  *_shorts;
    uint16_t   _short;
    uint32_t  *_longs;
    uint32_t   _long;
    rat_t     *_rats;
    rat_t      _rat;
    uint8_t   *_undefined;
    int32_t   *_slongs;
    int32_t    _slong;
    srat_t    *_srats;
    srat_t     _srat;
  } data;
} exif_tag_entry_t;

typedef struct {
    uint32_t      tag_id;
    exif_tag_entry_t  tag_entry;
} exif_tags_info_t;


typedef enum {
 HDR_BRACKETING_OFF,
 HDR_MODE,
 EXP_BRACKETING_MODE
 } hdr_mode;

typedef struct {
  hdr_mode mode;
  uint32_t hdr_enable;
  uint32_t total_frames;
  uint32_t total_hal_frames;
  char values[MAX_EXP_BRACKETING_LENGTH];  /* user defined values */
} exp_bracketing_t;
typedef struct {
  roi_t      mtr_area[MAX_ROI];
  uint32_t   num_area;
  int        weight[MAX_ROI];
} aec_mtr_area_t;

typedef struct {
  int denoise_enable;
  int process_plates;
} denoise_param_t;

#ifndef HAVE_CAMERA_SIZE_TYPE
  #define HAVE_CAMERA_SIZE_TYPE
struct camera_size_type {
  int width;
  int height;
};
#endif

typedef struct {
  uint32_t yoffset;
  uint32_t cbcr_offset;
  uint32_t size;
  struct camera_size_type resolution;
}cam_buf_info_t;

typedef struct {
  int x;
  int y;
}cam_point_t;

typedef struct {
  /* AF parameters */
  uint8_t focus_position;
  /* AEC parameters */
  uint32_t line_count;
  uint8_t luma_target;
  /* AWB parameters */
  int32_t r_gain;
  int32_t b_gain;
  int32_t g_gain;
  uint8_t exposure_mode;
  uint8_t exposure_program;
  float exposure_time;
  uint32_t iso_speed;
} snapshotData_info_t;


typedef enum {
  CAMERA_HFR_MODE_OFF = 1,
  CAMERA_HFR_MODE_60FPS,
  CAMERA_HFR_MODE_90FPS,
  CAMERA_HFR_MODE_120FPS,
  CAMERA_HFR_MODE_150FPS,
} camera_hfr_mode_t;

/* frame Q*/
struct fifo_node
{
  struct fifo_node *next;
  void *f;
};

struct fifo_queue
{
  int num_of_frames;
  struct fifo_node *front;
  struct fifo_node *back;
  pthread_mutex_t mut;
  pthread_cond_t wait;
  char* name;
};

typedef struct {
  uint32_t buf_len;
  uint8_t num;
  uint8_t pmem_type;
  uint32_t vaddr[8];
} mm_camera_histo_mem_info_t;

typedef enum {
  MM_CAMERA_CTRL_EVT_ZOOM_DONE,
  MM_CAMERA_CTRL_EVT_AUTO_FOCUS_DONE,
  MM_CAMERA_CTRL_EVT_PREP_SNAPSHOT,
  MM_CAMERA_CTRL_EVT_SNAPSHOT_CONFIG_DONE,
  MM_CAMERA_CTRL_EVT_WDN_DONE, // wavelet denoise done
  MM_CAMERA_CTRL_EVT_ERROR,
  MM_CAMERA_CTRL_EVT_MAX
}mm_camera_ctrl_event_type_t;

typedef struct {
  mm_camera_ctrl_event_type_t evt;
  cam_ctrl_status_t status;
  unsigned long cookie;
} mm_camera_ctrl_event_t;

typedef enum {
  MM_CAMERA_CH_EVT_STREAMING_ON,
  MM_CAMERA_CH_EVT_STREAMING_OFF,
  MM_CAMERA_CH_EVT_STREAMING_ERR,
  MM_CAMERA_CH_EVT_DATA_DELIVERY_DONE,
  MM_CAMERA_CH_EVT_DATA_REQUEST_MORE,
  MM_CAMERA_CH_EVT_MAX
}mm_camera_ch_event_type_t;

typedef struct {
  uint32_t ch;
  mm_camera_ch_event_type_t evt;
} mm_camera_ch_event_t;

typedef struct {
  uint32_t index;
  /* TBD: need more fields for histo stats? */
} mm_camera_stats_histo_t;

typedef struct  {
  uint32_t event_id;
  union {
    mm_camera_stats_histo_t    stats_histo;
  } e;
} mm_camera_stats_event_t;

typedef enum {
  FD_ROI_TYPE_HEADER,
  FD_ROI_TYPE_DATA
} fd_roi_type_t;

typedef struct {
  int fd_mode;
  int num_fd;
} fd_set_parm_t;

typedef struct {
  uint32_t frame_id;
  int16_t num_face_detected;
} fd_roi_header_type;

struct fd_rect_t {
  uint16_t x;
  uint16_t y;
  uint16_t dx;
  uint16_t dy;
};

typedef struct {
  struct fd_rect_t face_boundary;
  uint16_t left_eye_center[2];
  uint16_t right_eye_center[2];
  uint16_t mouth_center[2];
  uint8_t smile_degree;  //0 -100
  uint8_t smile_confidence;  //
  uint8_t blink_detected;  // 0 or 1
  uint8_t is_face_recognised;  // 0 or 1
  int8_t gaze_angle;  // -90 -45 0 45 90 for head left to rigth tilt
  int8_t updown_dir;  // -90 to 90
  int8_t leftright_dir;  //-90 to 90
  int8_t roll_dir;  // -90 to 90
  int8_t left_right_gaze;  // -50 to 50
  int8_t top_bottom_gaze;  // -50 to 50
  uint8_t left_blink;  // 0 - 100
  uint8_t right_blink;  // 0 - 100
  int8_t id;  // unique id for face tracking within view unless view changes
  int8_t score;  // score of confidence( 0 -100)
} fd_face_type;

typedef struct {
  uint32_t frame_id;
  uint8_t idx;
  fd_face_type face;
} fd_roi_data_type;

struct fd_roi_t {
  fd_roi_type_t type;
  union {
    fd_roi_header_type hdr;
    fd_roi_data_type data;
  } d;
};

typedef struct  {
  uint32_t event_id;
  union {
    mm_camera_histo_mem_info_t histo_mem_info;
    struct fd_roi_t roi;
  } e;
} mm_camera_info_event_t;


typedef enum {
  MM_CAMERA_EVT_TYPE_CH,
  MM_CAMERA_EVT_TYPE_CTRL,
  MM_CAMERA_EVT_TYPE_STATS,
  MM_CAMERA_EVT_TYPE_INFO,
  MM_CAMERA_EVT_TYPE_MAX
} mm_camera_event_type_t;

/******************************************************************************
 * Function: exif_set_tag
 * Description: Inserts or modifies an Exif tag to the Exif Info object. Typical
 *              use is to call this function multiple times - to insert all the
 *              desired Exif Tags individually to the Exif Info object and
 *              then pass the info object to the Jpeg Encoder object so
 *              the inserted tags would be emitted as tags in the Exif header.
 * Input parameters:
 *   obj       - The Exif Info object where the tag would be inserted to or
 *               modified from.
 *   tag_id    - The Exif Tag ID of the tag to be inserted/modified.
 *   p_entry   - The pointer to the tag entry structure which contains the
 *               details of tag. The pointer can be set to NULL to un-do
 *               previous insertion for a certain tag.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int exif_set_tag(exif_info_obj_t    obj,
                 exif_tag_id_t      tag_id,
                 exif_tag_entry_t  *p_entry);


/* Public SDK declarations recovered from the CAF-derived CM10.1 tree.
 * The control enums and wire structures above remain the stock release ABI. */
/*
 * the APP event related defines
*/
typedef enum {
  MM_CAMERA_STATS_EVT_HISTO,
  MM_CAMERA_STATS_EVT_MAX
} mm_camera_stats_event_type_t;

typedef enum {
  MM_CAMERA_INFO_EVT_ROI,
  MM_CAMERA_INFO_FLASH_FRAME_IDX,
  MM_CAMERA_INFO_EVT_MAX
} mm_camera_info_event_type_t;

/* !!WARNING: PLAESE BE VERY CAREFUL!!
 * v4l2_event payload has a limitation of 64 bytes.
 * This makes that the whole mm_camera_event_t size
 * cannot go beyond 64 bytes.
 */
typedef struct {
  mm_camera_event_type_t event_type;
  /* the union size cannot go beyond 64 bytes. need RFC */
  union {
    mm_camera_ch_event_t ch;
    mm_camera_ctrl_event_t ctrl;
    mm_camera_stats_event_t stats;
    mm_camera_info_event_t info;
  } e;
} mm_camera_event_t;

/* Auto focus mode, used for CAMERA_PARM_AF_MODE */
typedef enum {
  AF_MODE_UNCHANGED = -1,
  AF_MODE_NORMAL    = 0,
  AF_MODE_MACRO,
  AF_MODE_AUTO,
  AF_MODE_CAF,
  AF_MODE_INFINITY,
  AF_MODE_MAX
} isp3a_af_mode_t;

typedef struct {
  uint32_t  in1_w;
  uint32_t  out1_w;
  uint32_t  in1_h;
  uint32_t  out1_h;
  uint32_t  in2_w;
  uint32_t  out2_w;
  uint32_t  in2_h;
  uint32_t  out2_h;
  uint8_t update_flag;
} common_crop_t;

typedef enum {
  LED_MODE_OFF,
  LED_MODE_AUTO,
  LED_MODE_ON,
  LED_MODE_TORCH,

  /*new mode above should be added above this line*/
  LED_MODE_MAX
} led_mode_t;

typedef struct {
  int is_checking_af_retry;
  int is_moving;
} af_actuator_status_t;

typedef struct{
  int aec_index_for_zsl;
  int zsl_flash_enable;
} aec_info_for_flash_t;

typedef struct video_dis_param_ctrl_t {
  uint32_t dis_enable;       /* DIS feature: 1 = enable, 0 = disable.
                               when enable, caller makes sure w/h are 10% more. */
  uint32_t video_rec_width;  /* video frame width for recording */
  uint32_t video_rec_height; /* video frame height for recording */
  uint32_t output_cbcr_offset;
} video_dis_param_ctrl_t;

typedef enum camera_rotation_type {
  ROT_NONE               = 0,
  ROT_CLOCKWISE_90       = 1,
  ROT_CLOCKWISE_180      = 6,
  ROT_CLOCKWISE_270      = 7,
} camera_rotation_type;

typedef struct video_rotation_param_ctrl_t {
  camera_rotation_type rotation; /* 0 degree = rot disable. */
} video_rotation_param_ctrl_t;

enum focus_distance_index{
  FOCUS_DISTANCE_NEAR_INDEX,  /* 0 */
  FOCUS_DISTANCE_OPTIMAL_INDEX,
  FOCUS_DISTANCE_FAR_INDEX,
  FOCUS_DISTANCE_MAX_INDEX
};

typedef struct {
  float focus_distance[FOCUS_DISTANCE_MAX_INDEX];
} focus_distances_info_t;

typedef enum msm_st_frame_packing cam_3d_frame_format_t;

typedef struct {
  cam_frame_type_t frame_type;
  cam_3d_frame_format_t format;
}camera_3d_frame_t;

typedef enum {
    CAMERA_BESTSHOT_OFF = 0,
    CAMERA_BESTSHOT_AUTO = 1,
    CAMERA_BESTSHOT_LANDSCAPE = 2,
    CAMERA_BESTSHOT_SNOW,
    CAMERA_BESTSHOT_BEACH,
    CAMERA_BESTSHOT_SUNSET,
    CAMERA_BESTSHOT_NIGHT,
    CAMERA_BESTSHOT_PORTRAIT,
    CAMERA_BESTSHOT_BACKLIGHT,
    CAMERA_BESTSHOT_SPORTS,
    CAMERA_BESTSHOT_ANTISHAKE,
    CAMERA_BESTSHOT_FLOWERS,
    CAMERA_BESTSHOT_CANDLELIGHT,
    CAMERA_BESTSHOT_FIREWORKS,
    CAMERA_BESTSHOT_PARTY,
    CAMERA_BESTSHOT_NIGHT_PORTRAIT,
    CAMERA_BESTSHOT_THEATRE,
    CAMERA_BESTSHOT_ACTION,
    CAMERA_BESTSHOT_AR,
    CAMERA_BESTSHOT_MAX
} camera_bestshot_mode_type;

typedef enum {
  AUTO = 1,
  SPOT,
  CENTER_WEIGHTED,
  AVERAGE
} cam_af_focusrect_t;

typedef enum {
  CAMERA_AEC_FRAME_AVERAGE,
  CAMERA_AEC_CENTER_WEIGHTED,
  CAMERA_AEC_SPOT_METERING,
  CAMERA_AEC_SMART_METERING,
  CAMERA_AEC_USER_METERING,
  CAMERA_AEC_MAX_MODES
} camera_auto_exposure_mode_type;

typedef enum {
  FPS_MODE_AUTO,
  FPS_MODE_FIXED,
} fps_mode_t;

typedef struct {
  int32_t  buffer[256];       /* buffer to hold data */
  int32_t  max_value;
} camera_preview_histogram_info;

/* Clockwise */
typedef enum {
  CAMERA_ENCODING_ROTATE_0,
  CAMERA_ENCODING_ROTATE_90,
  CAMERA_ENCODING_ROTATE_180,
  CAMERA_ENCODING_ROTATE_270
} camera_encoding_rotate_t;

typedef enum {
  MOTION_ISO_OFF,
  MOTION_ISO_ON
} motion_iso_t;

typedef struct {
  struct msm_ctrl_cmd ctrlCmd;
  int fd;
  void (*af_cb)(int8_t );
  int8_t is_camafctrl_thread_join;
  isp3a_af_mode_t af_mode;
} cam_af_ctrl_t;

/* Display */
typedef struct {
    uint16_t user_input_display_width;
    uint16_t user_input_display_height;
} USER_INPUT_DISPLAY_T;

#if defined CAMERA_WB_AUTO
#undef CAMERA_WB_AUTO
#endif

#if defined CAMERA_WB_CUSTOM
#undef CAMERA_WB_CUSTOM
#endif

#if defined  CAMERA_WB_INCANDESCENT
#undef CAMERA_WB_INCANDESCENT
#endif

#if defined CAMERA_WB_FLUORESCENT
#undef CAMERA_WB_FLUORESCENT
#endif

#if defined CAMERA_WB_DAYLIGHT
#undef CAMERA_WB_DAYLIGHT
#endif

#if defined CAMERA_WB_CLOUDY_DAYLIGHT
#undef CAMERA_WB_CLOUDY_DAYLIGHT
#endif

#if defined CAMERA_WB_TWILIGHT
#undef CAMERA_WB_TWILIGHT
#endif

#if defined CAMERA_WB_SHADE
#undef CAMERA_WB_SHADE
#endif

typedef enum {
  CAMERA_WB_MIN_MINUS_1,
  CAMERA_WB_AUTO = 1,
  CAMERA_WB_CUSTOM,
  CAMERA_WB_INCANDESCENT,
  CAMERA_WB_FLUORESCENT,
  CAMERA_WB_DAYLIGHT,
  CAMERA_WB_CLOUDY_DAYLIGHT,
  CAMERA_WB_TWILIGHT,
  CAMERA_WB_SHADE,
  CAMERA_WB_OFF,
  CAMERA_WB_MAX_PLUS_1
} config3a_wb_t;


typedef uint32_t  jpeg_event_t;

// Possibly supported color formats
// Ordering handcrafted for efficient coding, alter with care!
typedef enum
{
    YCRCBLP_H2V2 = 0,
    YCBCRLP_H2V2 = 1,

    YCRCBLP_H2V1 = 2,
    YCBCRLP_H2V1 = 3,

    YCRCBLP_H1V2 = 4,
    YCBCRLP_H1V2 = 5,

    YCRCBLP_H1V1 = 6,
    YCBCRLP_H1V1 = 7,

    RGB565 = 8,
    RGB888 = 9,
    RGBa   = 10,

    JPEG_BITSTREAM_H2V2 = 12,
    JPEG_BITSTREAM_H2V1 = 14,
    JPEG_BITSTREAM_H1V2 = 16,
    JPEG_BITSTREAM_H1V1 = 18,

    JPEG_COLOR_FORMAT_MAX,

} jpeg_color_format_t;

/* EXIF header */
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
/* Enum defined to let compiler generate unique offset numbers for different
 * tags - ordering matters! NOT INTENDED to be used by any application. */
typedef enum
{
    // GPS IFD
    GPS_VERSION_ID = 0,
    GPS_LATITUDE_REF,
    GPS_LATITUDE,
    GPS_LONGITUDE_REF,
    GPS_LONGITUDE,
    GPS_ALTITUDE_REF,
    GPS_ALTITUDE,
    GPS_TIMESTAMP,
    GPS_SATELLITES,
    GPS_STATUS,
    GPS_MEASUREMODE,
    GPS_DOP,
    GPS_SPEED_REF,
    GPS_SPEED,
    GPS_TRACK_REF,
    GPS_TRACK,
    GPS_IMGDIRECTION_REF,
    GPS_IMGDIRECTION,
    GPS_MAPDATUM,
    GPS_DESTLATITUDE_REF,
    GPS_DESTLATITUDE,
    GPS_DESTLONGITUDE_REF,
    GPS_DESTLONGITUDE,
    GPS_DESTBEARING_REF,
    GPS_DESTBEARING,
    GPS_DESTDISTANCE_REF,
    GPS_DESTDISTANCE,
    GPS_PROCESSINGMETHOD,
    GPS_AREAINFORMATION,
    GPS_DATESTAMP,
    GPS_DIFFERENTIAL,

    // TIFF IFD
    NEW_SUBFILE_TYPE,
    SUBFILE_TYPE,
    IMAGE_WIDTH,
    IMAGE_LENGTH,
    BITS_PER_SAMPLE,
    COMPRESSION,
    PHOTOMETRIC_INTERPRETATION,
    THRESH_HOLDING,
    CELL_WIDTH,
    CELL_HEIGHT,
    FILL_ORDER,
    DOCUMENT_NAME,
    IMAGE_DESCRIPTION,
    MAKE,
    MODEL,
    STRIP_OFFSETS,
    ORIENTATION,
    SAMPLES_PER_PIXEL,
    ROWS_PER_STRIP,
    STRIP_BYTE_COUNTS,
    MIN_SAMPLE_VALUE,
    MAX_SAMPLE_VALUE,
    X_RESOLUTION,
    Y_RESOLUTION,
    PLANAR_CONFIGURATION,
    PAGE_NAME,
    X_POSITION,
    Y_POSITION,
    FREE_OFFSET,
    FREE_BYTE_COUNTS,
    GRAY_RESPONSE_UNIT,
    GRAY_RESPONSE_CURVE,
    T4_OPTION,
    T6_OPTION,
    RESOLUTION_UNIT,
    PAGE_NUMBER,
    TRANSFER_FUNCTION,
    SOFTWARE,
    DATE_TIME,
    ARTIST,
    HOST_COMPUTER,
    PREDICTOR,
    WHITE_POINT,
    PRIMARY_CHROMATICITIES,
    COLOR_MAP,
    HALFTONE_HINTS,
    TILE_WIDTH,
    TILE_LENGTH,
    TILE_OFFSET,
    TILE_BYTE_COUNTS,
    INK_SET,
    INK_NAMES,
    NUMBER_OF_INKS,
    DOT_RANGE,
    TARGET_PRINTER,
    EXTRA_SAMPLES,
    SAMPLE_FORMAT,
    TRANSFER_RANGE,
    JPEG_PROC,
    JPEG_INTERCHANGE_FORMAT,
    JPEG_INTERCHANGE_FORMAT_LENGTH,
    JPEG_RESTART_INTERVAL,
    JPEG_LOSSLESS_PREDICTORS,
    JPEG_POINT_TRANSFORMS,
    JPEG_Q_TABLES,
    JPEG_DC_TABLES,
    JPEG_AC_TABLES,
    YCBCR_COEFFICIENTS,
    YCBCR_SUB_SAMPLING,
    YCBCR_POSITIONING,
    REFERENCE_BLACK_WHITE,
    GAMMA,
    ICC_PROFILE_DESCRIPTOR,
    SRGB_RENDERING_INTENT,
    IMAGE_TITLE,
    COPYRIGHT,
    EXIF_IFD,
    ICC_PROFILE,
    GPS_IFD,


    // TIFF IFD (Thumbnail)
    TN_IMAGE_WIDTH,
    TN_IMAGE_LENGTH,
    TN_BITS_PER_SAMPLE,
    TN_COMPRESSION,
    TN_PHOTOMETRIC_INTERPRETATION,
    TN_IMAGE_DESCRIPTION,
    TN_MAKE,
    TN_MODEL,
    TN_STRIP_OFFSETS,
    TN_ORIENTATION,
    TN_SAMPLES_PER_PIXEL,
    TN_ROWS_PER_STRIP,
    TN_STRIP_BYTE_COUNTS,
    TN_X_RESOLUTION,
    TN_Y_RESOLUTION,
    TN_PLANAR_CONFIGURATION,
    TN_RESOLUTION_UNIT,
    TN_TRANSFER_FUNCTION,
    TN_SOFTWARE,
    TN_DATE_TIME,
    TN_ARTIST,
    TN_WHITE_POINT,
    TN_PRIMARY_CHROMATICITIES,
    TN_JPEGINTERCHANGE_FORMAT,
    TN_JPEGINTERCHANGE_FORMAT_L,
    TN_YCBCR_COEFFICIENTS,
    TN_YCBCR_SUB_SAMPLING,
    TN_YCBCR_POSITIONING,
    TN_REFERENCE_BLACK_WHITE,
    TN_COPYRIGHT,

    // EXIF IFD
    EXPOSURE_TIME,
    F_NUMBER,
    EXPOSURE_PROGRAM,
    SPECTRAL_SENSITIVITY,
    ISO_SPEED_RATING,
    OECF,
    EXIF_VERSION,
    EXIF_DATE_TIME_ORIGINAL,
    EXIF_DATE_TIME_DIGITIZED,
    EXIF_COMPONENTS_CONFIG,
    EXIF_COMPRESSED_BITS_PER_PIXEL,
    SHUTTER_SPEED,
    APERTURE,
    BRIGHTNESS,
    EXPOSURE_BIAS_VALUE,
    MAX_APERTURE,
    SUBJECT_DISTANCE,
    METERING_MODE,
    LIGHT_SOURCE,
    FLASH,
    FOCAL_LENGTH,
    SUBJECT_AREA,
    EXIF_MAKER_NOTE,
    EXIF_USER_COMMENT,
    SUBSEC_TIME,
    SUBSEC_TIME_ORIGINAL,
    SUBSEC_TIME_DIGITIZED,
    EXIF_FLASHPIX_VERSION,
    EXIF_COLOR_SPACE,
    EXIF_PIXEL_X_DIMENSION,
    EXIF_PIXEL_Y_DIMENSION,
    RELATED_SOUND_FILE,
    INTEROP,
    FLASH_ENERGY,
    SPATIAL_FREQ_RESPONSE,
    FOCAL_PLANE_X_RESOLUTION,
    FOCAL_PLANE_Y_RESOLUTION,
    FOCAL_PLANE_RESOLUTION_UNIT,
    SUBJECT_LOCATION,
    EXPOSURE_INDEX,
    SENSING_METHOD,
    FILE_SOURCE,
    SCENE_TYPE,
    CFA_PATTERN,
    CUSTOM_RENDERED,
    EXPOSURE_MODE,
    WHITE_BALANCE,
    DIGITAL_ZOOM_RATIO,
    FOCAL_LENGTH_35MM,
    SCENE_CAPTURE_TYPE,
    GAIN_CONTROL,
    CONTRAST,
    SATURATION,
    SHARPNESS,
    DEVICE_SETTINGS_DESCRIPTION,
    SUBJECT_DISTANCE_RANGE,
    IMAGE_UID,
    PIM,

    EXIF_TAG_MAX_OFFSET

} exif_tag_offset_t;

/* Below are the supported Tags (ID and structure for their data) */
#define CONSTRUCT_TAGID(offset,ID)   (offset << 16 | ID)
#define _ID_GPS_LATITUDE_REF                0x0001
#define EXIFTAGID_GPS_LATITUDE_REF          CONSTRUCT_TAGID(GPS_LATITUDE_REF, _ID_GPS_LATITUDE_REF)
#define _ID_GPS_LATITUDE                    0x0002
#define EXIFTAGID_GPS_LATITUDE              CONSTRUCT_TAGID(GPS_LATITUDE, _ID_GPS_LATITUDE)
#define _ID_GPS_LONGITUDE_REF               0x0003
#define EXIFTAGID_GPS_LONGITUDE_REF         CONSTRUCT_TAGID(GPS_LONGITUDE_REF, _ID_GPS_LONGITUDE_REF)
#define _ID_GPS_LONGITUDE                   0x0004
#define EXIFTAGID_GPS_LONGITUDE             CONSTRUCT_TAGID(GPS_LONGITUDE, _ID_GPS_LONGITUDE)
#define _ID_GPS_ALTITUDE_REF                0x0005
#define EXIFTAGID_GPS_ALTITUDE_REF          CONSTRUCT_TAGID(GPS_ALTITUDE_REF, _ID_GPS_ALTITUDE_REF)
#define _ID_GPS_ALTITUDE                    0x0006
#define EXIFTAGID_GPS_ALTITUDE              CONSTRUCT_TAGID(GPS_ALTITUDE, _ID_GPS_ALTITUDE)
#define _ID_GPS_TIMESTAMP                   0x0007
#define EXIFTAGID_GPS_TIMESTAMP             CONSTRUCT_TAGID(GPS_TIMESTAMP, _ID_GPS_TIMESTAMP)
#define _ID_GPS_PROCESSINGMETHOD            0x001b
#define EXIFTAGID_GPS_PROCESSINGMETHOD      CONSTRUCT_TAGID(GPS_PROCESSINGMETHOD, _ID_GPS_PROCESSINGMETHOD)
#define _ID_GPS_DATESTAMP                   0x001d
#define EXIFTAGID_GPS_DATESTAMP             CONSTRUCT_TAGID(GPS_DATESTAMP, _ID_GPS_DATESTAMP)
#define _ID_ORIENTATION                     0x0112
#define EXIFTAGID_ORIENTATION               CONSTRUCT_TAGID(ORIENTATION, _ID_ORIENTATION)
#define EXIFTAGTYPE_ORIENTATION             EXIF_SHORT
#define _ID_ISO_SPEED_RATING                0x8827
#define EXIFTAGID_ISO_SPEED_RATING          CONSTRUCT_TAGID(ISO_SPEED_RATING, _ID_ISO_SPEED_RATING)
#define _ID_EXIF_DATE_TIME_ORIGINAL          0x9003
#define EXIFTAGID_EXIF_DATE_TIME_ORIGINAL    CONSTRUCT_TAGID(EXIF_DATE_TIME_ORIGINAL, _ID_EXIF_DATE_TIME_ORIGINAL)
#define _ID_FOCAL_LENGTH                    0x920a
#define EXIFTAGID_FOCAL_LENGTH              CONSTRUCT_TAGID(FOCAL_LENGTH, _ID_FOCAL_LENGTH)

#endif /* __QCAMERA_INTF_H__ */
