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

#ifndef OHOS_HDI_DISPLAY_V1_4_IDISPLAY_COMPOSER_INTERFACE_H
#define OHOS_HDI_DISPLAY_V1_4_IDISPLAY_COMPOSER_INTERFACE_H

#include "v1_3/include/idisplay_composer_interface.h"
#include "v1_4/display_composer_type.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_4 {

class IDisplayComposerInterface : public V1_3::IDisplayComposerInterface {
public:
    /**
     * @brief Obtains all interfaces of IDisplayComposerInterface.
     *
     * @return Returns <b>IDisplayComposerInterface*</b> if the operation is successful;
     * returns an null point otherwise.
     * @since 6.1
     * @version 1.0
     */
    static IDisplayComposerInterface* Get(bool needSMQ = true);

    /**
     * @brief Obtains the power status of the panel.
     *
     * @param devId Indicates the ID of the display device.
     * @param status Indicates the pointer to the power status of a panel. The status is written by this interface.
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 6.1
     * @version 1.0
     */
    virtual int32_t GetPanelPowerStatus(uint32_t devId, PanelPowerStatus &status) = 0;

    /**
     * @brief Obtains the display connection type.
     *
     * @param devId Indicates the ID of the display device.
     * @param outType Indicates the display connection type. The status is written by this interface.
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 6.1
     * @version 1.0
     */
    virtual int32_t GetDisplayConnectionType(uint32_t devId, DisplayConnectionType& outType) = 0;

    /**
     * @brief client layer buffer format and dataspace
     *
     * @param devId Indicates the ID of the display device.
     * @param pixelFormat is format of client layer.
     * @param dataspace is dataspace of client layer.
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 6.1
     * @version 1.0
     */
    virtual int32_t GetDisplayClientTargetProperty(uint32_t devId, int32_t& pixelFormat, int32_t& dataspace) = 0;

    /**
    * @brief set the colorgamut for a dispalydevice
    *
    * @param devId Indicates the ID of the display device.
    * @param gamut Indicates the colorgamut to set.
    *
    * @return Returns <b>0</b> if the operation is successful; returns an error code defined
    * in {@link DispErrCode} otherwise.
    * @since 6.1
    * @version 1.0
    */
    virtual int32_t SetDisplayColorGamut(uint32_t devId, ColorGamut gamut) = 0;
};
} // V1_4
} // Composer
} // Display
} // HDI
} // OHOS
#endif // OHOS_HDI_DISPLAY_V1_4_IDISPLAY_COMPOSER_INTERFACE_H
