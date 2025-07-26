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

#ifndef OHOS_HDI_DISPLAY_V1_3_IDISPLAY_COMPOSER_INTERFACE_H
#define OHOS_HDI_DISPLAY_V1_3_IDISPLAY_COMPOSER_INTERFACE_H

#include "v1_2/include/idisplay_composer_interface.h"
#include "v1_3/display_composer_type.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_3 {

class IDisplayComposerInterface : public V1_2::IDisplayComposerInterface {
public:
    /**
     * @brief Obtains all interfaces of IDisplayComposerInterface.
     *
     * @return Returns <b>IDisplayComposerInterface*</b> if the operation is successful;
     * returns an null point otherwise.
     * @since 6.0
     * @version 1.0
     */
    static IDisplayComposerInterface* Get(bool needSMQ = true);

    /**
     * @brief Registers the callback to be invoked when the environment changes.
     *
     * @param cb Indicates the instance used to notify the graphics service of an environment change event occurred.
     * @param data Data used by cb
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 6.0
     * @version 1.0
     */
    virtual int32_t RegHwcEventCallback(HwcEventCallback cb, void *data) = 0;
};
} // V1_3
} // Composer
} // Display
} // HDI
} // OHOS
#endif // OHOS_HDI_DISPLAY_V1_3_IDISPLAY_COMPOSER_INTERFACE_H
