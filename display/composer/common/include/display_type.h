/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @addtogroup Display
 * @{
 *
 * @brief Defines driver functions of the display module.
 *
 * This module provides driver functions for the graphics subsystem, including graphics layer management,
 * device control, graphics hardware acceleration, display memory management, and callbacks.
 *
 * @since 1.0
 * @version 2.0
 */

/**
 * @file display_type.h
 *
 * @brief Declares the data types used by the display driver functions.
 *
 * @since 1.0
 * @version 2.0
 */

#ifndef DISPLAY_TYPE_H
#define DISPLAY_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Enumerates pixel formats.
 *
 */
typedef enum {
    PIXEL_FMT_CLUT8 = 0,                 /**< CLUT8 format */
    PIXEL_FMT_CLUT1,                     /**< CLUT1 format */
    PIXEL_FMT_CLUT4,                     /**< CLUT4 format */
    PIXEL_FMT_RGB_565,                   /**< RGB565 format */
    PIXEL_FMT_RGBA_5658,                 /**< RGBA5658 format */
    PIXEL_FMT_RGBX_4444,                 /**< RGBX4444 format */
    PIXEL_FMT_RGBA_4444,                 /**< RGBA4444 format */
    PIXEL_FMT_RGB_444,                   /**< RGB444 format */
    PIXEL_FMT_RGBX_5551,                 /**< RGBX5551 format */
    PIXEL_FMT_RGBA_5551,                 /**< RGBA5551 format */
    PIXEL_FMT_RGB_555,                   /**< RGB555 format */
    PIXEL_FMT_RGBX_8888,                 /**< RGBX8888 format */
    PIXEL_FMT_RGBA_8888,                 /**< RGBA8888 format */
    PIXEL_FMT_RGB_888,                   /**< RGB888 format */
    PIXEL_FMT_BGR_565,                   /**< BGR565 format */
    PIXEL_FMT_BGRX_4444,                 /**< BGRX4444 format */
    PIXEL_FMT_BGRA_4444,                 /**< BGRA4444 format */
    PIXEL_FMT_BGRX_5551,                 /**< BGRX5551 format */
    PIXEL_FMT_BGRA_5551,                 /**< BGRA5551 format */
    PIXEL_FMT_BGRX_8888,                 /**< BGRX8888 format */
    PIXEL_FMT_BGRA_8888,                 /**< BGRA8888 format */
    PIXEL_FMT_YUV_422_I,                 /**< YUV422 interleaved format */
    PIXEL_FMT_YCBCR_422_SP,              /**< YCBCR422 semi-planar format */
    PIXEL_FMT_YCRCB_422_SP,              /**< YCRCB422 semi-planar format */
    PIXEL_FMT_YCBCR_420_SP,              /**< YCBCR420 semi-planar format */
    PIXEL_FMT_YCRCB_420_SP,              /**< YCRCB420 semi-planar format */
    PIXEL_FMT_YCBCR_422_P,               /**< YCBCR422 planar format */
    PIXEL_FMT_YCRCB_422_P,               /**< YCRCB422 planar format */
    PIXEL_FMT_YCBCR_420_P,               /**< YCBCR420 planar format */
    PIXEL_FMT_YCRCB_420_P,               /**< YCRCB420 planar format */
    PIXEL_FMT_YUYV_422_PKG,              /**< YUYV422 packed format */
    PIXEL_FMT_UYVY_422_PKG,              /**< UYVY422 packed format */
    PIXEL_FMT_YVYU_422_PKG,              /**< YVYU422 packed format */
    PIXEL_FMT_VYUY_422_PKG,              /**< VYUY422 packed format */
    PIXEL_FMT_VENDER_MASK = 0X7FFF0000,  /**< vendor mask format */
    PIXEL_FMT_BUTT = 0X7FFFFFFF          /**< Invalid pixel format */
} PixelFormat;

#ifdef __cplusplus
}
#endif
#endif
/* @} */
