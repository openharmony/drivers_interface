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

#ifndef HC_GEN_TYPES_H
#define HC_GEN_TYPES_H

#define ALIGN_SIZE        4
#define TAB               "    "
#define TAB_SIZE          4
#define OPCODE_BYTE_WIDTH 1

#define BYTE_SIZE  1
#define WORD_SIZE  2
#define DWORD_SIZE 4
#define QWORD_SIZE 8

#define UNIX_SEPARATOR '/'
#define WIN_SEPARATOR  '\\'

#ifdef OS_WIN
#define OS_SEPARATOR WIN_SEPARATOR
#else
#define OS_SEPARATOR UNIX_SEPARATOR
#endif

namespace OHOS {
namespace Hardware {
enum HcsErrorNo {
    NOERR = 0,     /* No error */
    EFAIL,         /* Process fail */
    EOOM,          /* Out of memory */
    EOPTION,       /* Option error */
    EREOPENF,      /* Reopen argument */
    EINVALF,       /* Invalid file */
    EINVALARG,     /* Invalid argument */
    EDECOMP,       /* Decompile error */
    EOUTPUT,       /* Output error */
    EASTWALKBREAK, /* Break ast walk */
};
} // namespace Hardware
} // namespace OHOS

#endif // HC_GEN_TYPES_H
