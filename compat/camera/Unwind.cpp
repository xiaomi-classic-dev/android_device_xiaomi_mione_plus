/*
 * Copyright (C) 2020 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <unwind.h>

// The legacy camera blobs use this entry point for diagnostic stack dumps.
// Android 10's libc binds the ARM personality to libdl_android's trap.
// Report that a backtrace cannot be collected, without replacing exception
// unwinding or the personality routine for any other process.
extern "C" _Unwind_Reason_Code __gnu_Unwind_Backtrace(
        _Unwind_Control_Block*, _Unwind_Trace_Fn, void*)
{
    return _URC_FAILURE;
}
