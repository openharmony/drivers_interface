/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef CAMERA_METADATA_OPERATOR_H
#define CAMERA_METADATA_OPERATOR_H

#include <cstdio>
#include <cstdint>
#include <string>

#include "camera_device_ability_items.h"

/** Versioning information */
#define CURRENT_CAMERA_METADATA_VERSION 1

#define ITEM_ALIGNMENT ((size_t) 4)

#define DATA_ALIGNMENT ((size_t) 8)

#define METADATA_ALIGNMENT ((size_t) 4)

#define INDEX_COUNTER 2
#define WRAP_LENGTH 4
#define ENTRY_DATA_SIZE 4

// data type
enum {
    // uint8_t
    META_TYPE_BYTE = 0,
    // int32_t
    META_TYPE_INT32 = 1,
    // uint32_t
    META_TYPE_UINT32 = 2,
    // float
    META_TYPE_FLOAT = 3,
    // int64_t
    META_TYPE_INT64 = 4,
    // double
    META_TYPE_DOUBLE = 5,
    // A 64-bit fraction (camera_metadata_rational_t)
    META_TYPE_RATIONAL = 6,
    // Number of data type
    META_NUM_TYPES
};

const static char *OHOS_CAMERA_METADATA_TYPE[META_NUM_TYPES] = {
    [META_TYPE_BYTE] = "byte",
    [META_TYPE_INT32] = "int32",
    [META_TYPE_UINT32] = "uint32",
    [META_TYPE_FLOAT] = "float",
    [META_TYPE_INT64] = "int64",
    [META_TYPE_DOUBLE] = "double",
    [META_TYPE_RATIONAL] = "rational"
};

typedef struct camera_rational {
    int32_t numerator;
    int32_t denominator;
} camera_rational_t;

// common metadata header
typedef struct common_metadata_header {
    uint32_t version;
    uint32_t size;
    uint32_t item_count;
    uint32_t item_capacity;
    uint32_t items_start; // Offset from common_metadata_header
    uint32_t data_count;
    uint32_t data_capacity;
    uint32_t data_start; // Offset from common_metadata_header
} common_metadata_header_t;

typedef struct camera_metadata_item_entry {
    uint32_t item;
    uint32_t data_type;
    uint32_t count;
    union {
        uint32_t offset;
        uint8_t  value[4];
    } data;
} camera_metadata_item_entry_t;

typedef struct camera_metadata_item {
    uint32_t index;
    uint32_t item;
    uint32_t data_type;
    uint32_t count;
    union {
        uint8_t  *u8;
        int32_t  *i32;
        uint32_t *ui32;
        float    *f;
        int64_t  *i64;
        double   *d;
        camera_rational_t *r;
    } data;
} camera_metadata_item_t;

typedef struct item_info {
    const char *item_name;
    uint8_t item_type;
    int32_t dataCnt;
} item_info_t;

/* camera_metadata_section */
typedef enum camera_metadata_sec {
    OHOS_SECTION_CAMERA_PROPERTIES,
    OHOS_SECTION_CAMERA_SENSOR,
    OHOS_SECTION_CAMERA_SENSOR_INFO,
    OHOS_SECTION_CAMERA_STATISTICS,
    OHOS_SECTION_CAMERA_CONTROL,
    OHOS_SECTION_DEVICE_EXPOSURE,
    OHOS_SECTION_DEVICE_FOCUS,
    OHOS_SECTION_DEVICE_WHITE,
    OHOS_SECTION_DEVICE_FLASH,
    OHOS_SECTION_DEVICE_ZOOM,
    OHOS_SECTION_STREAM_ABILITY,
    OHOS_SECTION_STREAM_JPEG,
    OHOS_SECTION_STREAM_VIDEO,
    OHOS_SECTION_CAMERA_EFFECT,
    OHOS_SECTION_CAMERA_SECURE,
    OHOS_SECTION_CAMERA_XMAGE,
    OHOS_SECTION_STREAM_CONTROL,
    OHOS_SECTION_LIGHT_STATUS,
    OHOS_SECTION_COUNT,
} camera_metadata_sec_t;

typedef struct VendoTagInfo {
    uint32_t tagId;
    const char* tagName;
    uint8_t tagType;
} vendorTag_t;

/* Return codes */
#define CAM_META_FAILURE        (-1)
#define CAM_META_SUCCESS         0
#define CAM_META_INVALID_PARAM   2
#define CAM_META_ITEM_NOT_FOUND  3
#define CAM_META_ITEM_CAP_EXCEED 4
#define CAM_META_DATA_CAP_EXCEED 5

/* for shift opereration */
#define BITWISE_SHIFT_16 16

namespace OHOS::Camera {
// Allocate a new camera metadata buffer and return the metadata header
common_metadata_header_t *AllocateCameraMetadataBuffer(uint32_t item_capacity, uint32_t data_capacity);

// Is camera metadata item exist
bool IsCameraMetadataItemExist(const common_metadata_header_t *src, uint32_t item);

// Find camera metadata item and fill the found item
int FindCameraMetadataItem(const common_metadata_header_t *src, uint32_t item,
    camera_metadata_item_t *metadataItem);

// Find camera metadata item index if the item exists
int FindCameraMetadataItemIndex(const common_metadata_header_t *src, uint32_t item, uint32_t *index);

// Get camera metadata item name
const char *GetCameraMetadataItemName(uint32_t item);

// Update camera metadata item and fill the updated item
int UpdateCameraMetadataItem(common_metadata_header_t *dst, uint32_t item, const void *data, uint32_t dataCount,
    camera_metadata_item_t *updatedItem);

// Update camera metadata item by index and fill the updated item
int UpdateCameraMetadataItemByIndex(common_metadata_header_t *dst, uint32_t index, const void *data,
    uint32_t dataCount, camera_metadata_item_t *updated_item);

// Add camera metadata item
int AddCameraMetadataItem(common_metadata_header_t *dst, uint32_t item, const void *data, size_t dataCount);

// Delete camera metadata item
int DeleteCameraMetadataItem(common_metadata_header_t *dst, uint32_t item);

// Delete camera metadata item by index
int DeleteCameraMetadataItemByIndex(common_metadata_header_t *dst, uint32_t index);

// Free camera metadata buffer
void FreeCameraMetadataBuffer(common_metadata_header_t *dst);

std::string MetadataItemDump(const common_metadata_header_t *metadataHeader, uint32_t item);

std::string FormatCameraMetadataToString(const common_metadata_header_t *metadataHeader);

// Internal use
camera_metadata_item_entry_t *GetMetadataItems(const common_metadata_header_t *metadataHeader);
uint8_t *GetMetadataData(const common_metadata_header_t *metadataHeader);
int GetCameraMetadataItem(const common_metadata_header_t *src, uint32_t index, camera_metadata_item_t *item);
uint32_t GetCameraMetadataItemCount(const common_metadata_header_t *metadata_header);
uint32_t GetCameraMetadataItemCapacity(const common_metadata_header_t *metadata_header);
uint32_t GetCameraMetadataDataSize(const common_metadata_header_t *metadata_header);
int32_t CopyCameraMetadataItems(common_metadata_header_t *newMetadata,
    const common_metadata_header_t *oldMetadata);
int32_t CalculateCameraMetadataItemDataSize(uint32_t type, size_t data_count);
int32_t GetCameraMetadataItemType(uint32_t item, uint32_t *data_type);
common_metadata_header_t *FillCameraMetadata(common_metadata_header_t *buffer, size_t memoryRequired,
    uint32_t itemCapacity, uint32_t dataCapacity);
int32_t GetMetadataSection(uint32_t itemSection, uint32_t *section);
int MetadataExpandItemMem(common_metadata_header_t *dst, camera_metadata_item_entry_t *item,
    size_t oldItemSize);
int32_t GetAllVendorTags(std::vector<vendorTag_t>& tagVec);
} // Camera
#endif // CAMERA_METADATA_OPERATOR_H
