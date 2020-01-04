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

#include "d2gfx_resize_window_on_resolution_change.hpp"

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

} //namespace

void SGD2FMMB_D2GFX_ResizeWindowOnResolutionChange(
    HWND hWnd,
    HWND hWndInsertAfter,
    int X,
    int Y,
    int cx,
    int cy,
    UINT uFlags
) {
  // Determine if the window should be resized.
  const std::tuple resolution_width_and_height = GetResolution();

  const int resolution_width = std::get<0>(resolution_width_and_height);
  const int resolution_height = std::get<1>(resolution_width_and_height);

  const HWND window_handle = d2::d2gfx::GetWindowHandle();

  RECT window_rect;
  GetClientRect(window_handle, &window_rect);

  const int window_width = window_rect.right - window_rect.left;
  const int window_height = window_rect.bottom - window_rect.top;

  if (window_width == resolution_width
      && window_height == resolution_height) {
    SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
  }
}

} // namespace sgd2fmmb::patches
