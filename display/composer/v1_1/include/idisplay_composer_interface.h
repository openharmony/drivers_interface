/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_HDI_DISPLAY_V1_1_IDISPLAY_COMPOSER_INTERFACE_H
#define OHOS_HDI_DISPLAY_V1_1_IDISPLAY_COMPOSER_INTERFACE_H

#include "v1_0/include/idisplay_composer_interface.h"
#include "v1_1/display_composer_type.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_1 {

class IDisplayComposerInterface : public V1_0::IDisplayComposerInterface {
public:
    /**
     * @brief Obtains all interfaces of IDisplayComposerInterface.
     *
     * @return Returns <b>IDisplayComposerInterface*</b> if the operation is successful;
     * returns an null point otherwise.
     * @since 4.0
     * @version 1.1
     */
    static IDisplayComposerInterface* Get();

    /**
     * @brief Registers the callback to be invoked when it's ready to change framerate.
     *
     * @param cb Indicates the callback
     * @param data Data used by cb
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 4.1
     * @version 1.1
     */
    virtual int32_t RegSeamlessChangeCallback(SeamlessChangeCallback cb, void* data) = 0;

    /**
     * @brief Obtains the display modes supported by a display device.
     *
     * @param devId Indicates the ID of the display device.
     * @param modes Indicates the vector of the information about all modes supported by the display device,
     * including all supported resolutions, refresh rates and groupId. Each mode has an ID, which will be used when
     * the mode is set or obtained. For details, see {@link DisplayModeInfoExt}.
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 4.1
     * @version 1.1
     */
    virtual int32_t GetDisplaySupportedModesExt(unsigned int devId, std::vector<DisplayModeInfoExt>& modes) = 0;

    /**
     * @brief Sets the display mode of a display device.
     *
     * @param devId Indicates the ID of the display device.
     * @param modeId Indicates the ID of the display mode. The device is switched to the display mode specified by
     * this parameter in this interface.
     * @param cb Indicates the callback to be invoked when mode is change.
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 4.1
     * @version 1.1
     */
    virtual int32_t SetDisplayModeAsync(uint32_t devId, uint32_t modeId, ModeCallback cb) = 0;

    /**
     * @brief Get the current vblank period.
     * @param devId Indicates the ID of the display device.
     * @param period Indicates the vblank period(ns).
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 4.1
     * @version 1.1
     */
    virtual int32_t GetDisplayVBlankPeriod(uint32_t devId, uint64_t &period) = 0;
};
} // V1_1
} // Composer
} // Display
} // HDI
} // OHOS
#endif // OHOS_HDI_DISPLAY_V1_1_IDISPLAY_COMPOSER_INTERFACE_H
