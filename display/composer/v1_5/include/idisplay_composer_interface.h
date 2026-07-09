/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_HDI_DISPLAY_V1_5_IDISPLAY_COMPOSER_INTERFACE_H
#define OHOS_HDI_DISPLAY_V1_5_IDISPLAY_COMPOSER_INTERFACE_H

#include "v1_4/include/idisplay_composer_interface.h"
#include "v1_5/display_composer_type.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_5 {

class IDisplayComposerInterface : public V1_4::IDisplayComposerInterface {
public:
    /**
     * @brief Obtains all interfaces of IDisplayComposerInterface.
     *
     * @return Returns <b>IDisplayComposerInterface*</b> if the operation is successful;
     * returns an null point otherwise.
     * @since 7.0
     * @version 1.0
     */
    static IDisplayComposerInterface* Get(bool needSMQ = true);

    /**
     * @brief Gets the current and maximum values of a VCP code for a physical display device.
     *
     * This interface retrieves the status of a specific Virtual Control Panel (VCP)
     * code for the specified display device, such as brightness or contrast.
     *
     * @param devId Indicates the ID of the display device.
     * @param vcpCode Indicates the VESA VCP code to be queried.
     * @param currentValue Receives the current value of the VCP feature.
     * @param maximumValue Receives the maximum supported value of the VCP feature.
     * @param replyErrorCode Indicates the error code from the underlying DDC/CI protocol,
     * populated only when the function returns 0.
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 7.0
     * @version 1.0
     */
    virtual int32_t GetDisplayVCPFeature(uint32_t devId, uint8_t vcpCode,
        uint16_t& currentValue, uint16_t& maximumValue, int32_t& replyErrorCode) = 0;
    
    /**
     * @brief Sets the value of a VCP feature for a physical display device.
     *
     * This interface sends a command to adjust a specific Virtual Control Panel (VCP)
     * code for the specified display device, such as setting the brightness level.
     *
     * @param devId Indicates the ID of the display device.
     * @param vcpCode Indicates the VESA VCP code to be set.
     * @param currentValue Indicates the new value to be set for the VCP feature.
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 7.0
     * @version 1.0
     */
    virtual int32_t SetDisplayVCPFeature(uint32_t devId, uint8_t vcpCode, uint16_t currentValue) = 0;

    /**
     * @brief GetLayerColor.
     *
     * @param devId Indicates the ID of the display device.
     * @param layerId Indicates the ID of the layer.
     * @param color Indicates the solid filled color of the layer.
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 7.0
     * @version 1.0
     */
    virtual int32_t GetLayerColor(uint32_t devId, uint32_t layerId, LayerColor &color) = 0;
};
} // V1_5
} // Composer
} // Display
} // HDI
} // OHOS
#endif // OHOS_HDI_DISPLAY_V1_5_IDISPLAY_COMPOSER_INTERFACE_H
