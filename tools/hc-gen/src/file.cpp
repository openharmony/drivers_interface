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

#include "file.h"
#include <climits>
#include <cstdlib>
#include "types.h"

using namespace OHOS::Hardware::Util;

std::string File::AbsPath(const std::string &path)
{
    char realPath[PATH_MAX];
#ifdef MINGW32
    char *p = _fullpath(realPath, path.data(), PATH_MAX);
    if (p != nullptr && access(p, F_OK) != 0) {
        p = nullptr;
    }
#else
    char *p = realpath(path.data(), realPath);
#endif
    return p == nullptr ? "" : p;
}

std::string File::StripSuffix(std::string path)
{
    auto sepPos = path.rfind(OS_SEPARATOR);
    auto dotPos = path.rfind('.');
    if (sepPos == std::string::npos || dotPos > sepPos) {
        return path.substr(0, dotPos);
    } else {
        return path;
    }
}

std::string File::GetDir(std::string path)
{
    auto separatorPos = path.rfind(OS_SEPARATOR);
    if (separatorPos == std::string::npos) {
        return path;
    }
    return path.substr(0, separatorPos + 1);
}

std::string File::FileNameBase(const std::string &path)
{
    auto sepPos = path.rfind(OS_SEPARATOR);
    auto dotPos = path.rfind('.');

    if (sepPos == std::string::npos) {
        sepPos = 0;
    } else {
        sepPos++;
    }

    if (dotPos == std::string::npos || dotPos < sepPos) {
        dotPos = path.size();
    }

    auto len = path.size() - 1;
    if (dotPos != std::string::npos && dotPos > sepPos) {
        len = dotPos - sepPos;
    }
    return path.substr(sepPos, len);
}
