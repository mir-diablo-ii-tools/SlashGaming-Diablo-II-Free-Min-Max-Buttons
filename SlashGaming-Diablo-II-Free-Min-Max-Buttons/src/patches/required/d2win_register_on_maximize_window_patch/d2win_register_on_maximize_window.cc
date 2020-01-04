/**
 * SlashGaming Diablo II Free Min Max Buttons
 * Copyright (C) 2019-2020  Mir Drualga
 *
 * This file is part of SlashGaming Diablo II Free Min Max Buttons.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Additional permissions under GNU Affero General Public License version 3
 *  section 7
 *
 *  If you modify this Program, or any covered work, by linking or combining
 *  it with Diablo II (or a modified version of that game and its
 *  libraries), containing parts covered by the terms of Blizzard End User
 *  License Agreement, the licensors of this Program grant you additional
 *  permission to convey the resulting work. This additional permission is
 *  also extended to any combination of expansions, mods, and remasters of
 *  the game.
 *
 *  If you modify this Program, or any covered work, by linking or combining
 *  it with any Graphics Device Interface (GDI), DirectDraw, Direct3D,
 *  Glide, OpenGL, or Rave wrapper (or modified versions of those
 *  libraries), containing parts not covered by a compatible license, the
 *  licensors of this Program grant you additional permission to convey the
 *  resulting work.
 *
 *  If you modify this Program, or any covered work, by linking or combining
 *  it with any library (or a modified version of that library) that links
 *  to Diablo II (or a modified version of that game and its libraries),
 *  containing parts not covered by a compatible license, the licensors of
 *  this Program grant you additional permission to convey the resulting
 *  work.
 */

#include "d2win_register_on_maximize_window.hpp"

#include <windows.h>
#include <array>
#include <map>

#include "../../../asm_x86_macro.h"

namespace sgd2fmmb::patches {
namespace {

static std::tuple<int, int> GetResolution() {
  switch (d2::d2gfx::GetVideoMode()) {
    case d2::VideoMode::kGDI: {
      return std::make_tuple(
          d2::d2gdi::GetBitBlockWidth(),
          d2::d2gdi::GetBitBlockHeight()
      );
    }

    case d2::VideoMode::kDirectDraw: {
      return std::make_tuple(
          d2::d2ddraw::GetDisplayWidth(),
          d2::d2ddraw::GetDisplayHeight()
      );
    }

    case d2::VideoMode::kGlide: {
      return std::make_tuple(
          d2::d2glide::GetDisplayWidth(),
          d2::d2glide::GetDisplayHeight()
      );
    }

    case d2::VideoMode::kDirect3D: {
      return std::make_tuple(
          d2::d2direct3d::GetDisplayWidth(),
          d2::d2direct3d::GetDisplayHeight()
      );
    }
  }
}

static std::tuple<int, int> DetermineClosestCommonAspectRatio(
    double unknown_aspect_ratio
) {
  constexpr std::array common_aspect_ratios = {
      std::make_tuple(1, 1),
      std::make_tuple(3, 2),
      std::make_tuple(4, 3),
      std::make_tuple(5, 3),
      std::make_tuple(5, 4),
      std::make_tuple(16, 9),
      std::make_tuple(16, 10), // aka 8:5
      std::make_tuple(17, 9),
      std::make_tuple(21, 9),
      std::make_tuple(21, 10),
  };

  std::map<float, std::tuple<int, int>> diff_to_ratios;

  for (std::size_t i = 0; i < common_aspect_ratios.size(); i += 1) {
    float width = std::get<0>(common_aspect_ratios.at(i));
    float height = std::get<1>(common_aspect_ratios.at(i));

    diff_to_ratios.insert_or_assign(
        std::abs(unknown_aspect_ratio - (width / height)),
        common_aspect_ratios.at(i)
    );
  }

  return diff_to_ratios.cbegin()->second;
}

static bool IsFixedAspectRatioFromRegistry() {
  // Read the key.
  HKEY key_handle;

  LSTATUS open_key_status = RegOpenKeyExW(
      HKEY_CURRENT_USER,
      L"SOFTWARE\\Blizzard Entertainment\\Diablo II",
      0,
      KEY_QUERY_VALUE,
      &key_handle
  );

  if (open_key_status != ERROR_SUCCESS) {
    open_key_status = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Blizzard Entertainment\\Diablo II",
        0,
        KEY_QUERY_VALUE,
        &key_handle
    );

    if (open_key_status != ERROR_SUCCESS) {
      return false;
    }
  }

  // Get the video mode value from the registry.
  DWORD reg_is_fixed_aspect_ratio = 0;
  DWORD data_type = 0;
  DWORD data_size = sizeof(reg_is_fixed_aspect_ratio);

  LSTATUS query_value_status = RegQueryValueExW(
      key_handle,
      L"Fixed Aspect Ratio",
      nullptr,
      &data_type,
      reinterpret_cast<LPBYTE>(&reg_is_fixed_aspect_ratio),
      &data_size
  );

  if (query_value_status == ERROR_FILE_NOT_FOUND) {
    reg_is_fixed_aspect_ratio = 1;
  }

  RegCloseKey(key_handle);

  return reg_is_fixed_aspect_ratio != 0;
}

static bool IsFixedAspectRatioFromCmdLine() {
  int argc;

  const wchar_t* cmd_line = GetCommandLineW();
  wchar_t** argv = CommandLineToArgvW(cmd_line, &argc);

  return std::find(argv, &argv[argc], L"-nofixaspect") == &argv[argc];
}

static bool IsFixedAspectRatio() {
  return IsFixedAspectRatioFromCmdLine()
      && IsFixedAspectRatioFromRegistry();
}

static bool ResizeWindowToOriginal() {
  if (!d2::d2gfx::GetIsWindowedMode()) {
    return true;
  }

  HWND window_handle = d2::d2gfx::GetWindowHandle();

  // Get window rect, width, and height.
  RECT window_rect;
  GetClientRect(window_handle, &window_rect);

  const int window_width = window_rect.right - window_rect.left;
  const int window_height = window_rect.bottom - window_rect.top;

  // Get resolution width and height.
  const std::tuple resolution_width_and_height = GetResolution();
  const int resolution_width = std::get<0>(resolution_width_and_height);
  const int resolution_height = std::get<1>(resolution_width_and_height);

  if (window_width > resolution_width
      || window_height > resolution_height) {
    return true;
  }

  // Get work area rect, width, and height.
  RECT work_area;
  SystemParametersInfoW(SPI_GETWORKAREA, 0,&work_area, 0);

  const int work_area_width = work_area.right - work_area.left;
  const int work_area_height = work_area.bottom - work_area.top;

  // Change window rect and position.
  RECT window_new_rect;
  window_new_rect.left = 0;
  window_new_rect.top = 0;
  window_new_rect.right = resolution_width;
  window_new_rect.bottom = resolution_height;
  AdjustWindowRectEx(
      &window_new_rect,
      WS_CAPTION | WS_SYSMENU | WS_GROUP | WS_MAXIMIZEBOX,
      0,
      WS_THICKFRAME
  );

  int window_new_left = ((work_area_width - window_width) / 2)
      + work_area.left;
  int window_new_top = ((work_area_height - window_height) / 2)
      + work_area.top;
  int window_new_width = window_new_rect.right - window_new_rect.left;
  int window_new_height = window_new_rect.bottom - window_new_rect.top;

  if (window_new_left < 0) {
    window_new_left = 0;
  }

  if (window_new_top < 0) {
    window_new_top = 0;
  }

  SetWindowPos(
      window_handle,
      HWND_NOTOPMOST,
      window_new_left,
      window_new_top,
      window_new_width,
      window_new_height,
      SWP_NOACTIVATE | SWP_NOZORDER
  );

  return true;
}

static void ResizeWindowToMaximize(
    HWND window_handle,
    Message* message,
    int resolution_width,
    int resolution_height,
    int monitor_width,
    int monitor_height,
    MONITORINFO* monitor_info
) {
  WINDOWPLACEMENT window_placement;
  window_placement.length = sizeof(window_placement);
  GetWindowPlacement(window_handle, &window_placement);

  if (window_placement.showCmd == SW_MAXIMIZE) {
    message->unknown_0x18 = 1;
    message->return_value = 0;

    return;
  }

  // Calculate the new width and height of the window.
  float aspect_ratio = static_cast<float>(resolution_width)
      / static_cast<float>(resolution_height);

  std::tuple closest_aspect_ratio = DetermineClosestCommonAspectRatio(
      aspect_ratio
  );

  aspect_ratio = static_cast<float>(std::get<0>(closest_aspect_ratio))
      / std::get<1>(closest_aspect_ratio);

  int window_new_right;
  int window_new_bottom;

  if (monitor_width >= monitor_height) {
    window_new_right = static_cast<int>(
        std::round(monitor_height * aspect_ratio)
    );
    window_new_bottom = monitor_height;
  } else {
    window_new_right = monitor_width;
    window_new_bottom = static_cast<int>(
        std::round(monitor_width / aspect_ratio)
    );
  }

  window_new_right -= static_cast<int>(std::round(aspect_ratio * 9));
  window_new_bottom -= 9;

  int window_new_left = ((monitor_width - window_new_right) / 2)
      + monitor_info->rcWork.left;
  int window_new_top = ((monitor_height - window_new_bottom) / 2)
      + monitor_info->rcWork.top;

  if (window_new_top < 0) {
    window_new_top = 0;
  }

  if (window_new_left < 0) {
    window_new_left = 0;
  }

  SetWindowPos(
      window_handle,
      HWND_NOTOPMOST,
      window_new_left,
      window_new_top,
      window_new_right,
      window_new_bottom,
      SWP_NOACTIVATE | SWP_NOZORDER
  );
}

static void __stdcall Storm_RegisterCommandEvent(
    HWND window_handle,
    std::int32_t message_id,
    void (*callback)(Message*)
) {
  static std::intptr_t func_ptr =
      mapi::GameAddress::FromOrdinal(mapi::DefaultLibrary::kStorm, 417)
          .raw_address();

  reinterpret_cast<decltype(&Storm_RegisterCommandEvent)>(func_ptr)(
      window_handle,
      message_id,
      callback
  );
}

static void __stdcall Storm_UnregisterCommandEvent(
    HWND window_handle,
    std::int32_t message_id,
    void (*callback)(Message*)
) {
  static std::intptr_t func_ptr =
      mapi::GameAddress::FromOrdinal(mapi::DefaultLibrary::kStorm, 515)
          .raw_address();

  reinterpret_cast<decltype(&Storm_UnregisterCommandEvent)>(func_ptr)(
      window_handle,
      message_id,
      callback
  );
}

extern "C" static void SGD2FMMB_D2Win_ResizeWindow(
    Message* message
) {
  WPARAM system_command_type = message->wParam & 0xFFF0;

  if (system_command_type == SC_TASKLIST
      || system_command_type == SC_KEYMENU) {
    message->unknown_0x18 = 1;
    message->return_value = 0;
    return;
  }

  if (system_command_type == SC_MOVE) {
    if (!d2::d2gfx::GetIsWindowedMode()) {
      message->unknown_0x18 = 1;
      message->return_value = 0;
    }
    return;
  }

  if (system_command_type != SC_MAXIMIZE
      || !d2::d2gfx::GetIsWindowedMode()) {
    return;
  }

  // Get the monitor info from the monitor the window is in.
  HWND window_handle = d2::d2gfx::GetWindowHandle();
  HMONITOR monitor_handle = MonitorFromWindow(
      window_handle,
      MONITOR_DEFAULTTONEAREST
  );

  MONITORINFO monitor_info;
  monitor_info.cbSize = sizeof(monitor_info);
  GetMonitorInfoW(monitor_handle, &monitor_info);

  // Get window rect.
  RECT window_rect;
  GetClientRect(window_handle, &window_rect);

  const std::tuple<int, int> resolution_width_and_height = GetResolution();

  // Get desktop window rect.
  HWND desktop_window_handle = GetDesktopWindow();

  RECT desktop_rect;
  GetClientRect(desktop_window_handle, &desktop_rect);

  const int window_width = window_rect.right - window_rect.left;
  const int window_height = window_rect.bottom - window_rect.top;

  const int monitor_width = monitor_info.rcWork.right
      - monitor_info.rcWork.left;
  const int monitor_height = monitor_info.rcWork.bottom
      - monitor_info.rcWork.top;

  const int desktop_width = desktop_rect.right - desktop_rect.left;
  const int desktop_height = desktop_rect.bottom - desktop_rect.top;

  const int resolution_width = std::get<0>(resolution_width_and_height);
  const int resolution_height = std::get<1>(resolution_width_and_height);

  if (window_width == resolution_width
      && window_height == resolution_height
      && resolution_width < desktop_width
      && resolution_height < desktop_height) {
    if (!IsFixedAspectRatio()) {
      return;
    }

    ResizeWindowToMaximize(
        window_handle,
        message,
        resolution_width,
        resolution_height,
        monitor_width,
        monitor_height,
        &monitor_info
    );
  } else if (resolution_width < monitor_info.rcWork.right
      && resolution_height < monitor_info.rcWork.bottom) {
    SetWindowPos(
        window_handle,
        HWND_NOTOPMOST,
        0,
        0,
        resolution_width,
        resolution_height,
        SWP_NOACTIVATE | SWP_NOZORDER
    );

    ResizeWindowToOriginal();
  }

  message->unknown_0x18 = 1;
  message->return_value = 0;
}

static __declspec(naked) void __cdecl ResizeWindow_Shim(
    Message* message
) {
  ASM_X86(push ebp);
  ASM_X86(mov ebp, esp);

  ASM_X86(push eax);
  ASM_X86(push ecx);
  ASM_X86(push edx);

  ASM_X86(push dword ptr [ebp + 8]);
  ASM_X86(call ASM_X86_FUNC(SGD2FMMB_D2Win_ResizeWindow));
  ASM_X86(add esp, 4);

  ASM_X86(pop edx);
  ASM_X86(pop ecx);
  ASM_X86(pop eax);

  ASM_X86(leave);
  ASM_X86(ret 4);
}

} // namespace

void SGD2FMMB_D2Win_RegisterResizeEvent() {
  Storm_RegisterCommandEvent(
      d2::d2gfx::GetWindowHandle(),
      WM_SYSCOMMAND,
      &ResizeWindow_Shim
  );
}

void SGD2FMMB_D2Win_UnregisterResizeEvent() {
  Storm_UnregisterCommandEvent(
      d2::d2gfx::GetWindowHandle(),
      WM_SYSCOMMAND,
      &ResizeWindow_Shim
  );
}

} // namespace sgd2fmmb::patches
