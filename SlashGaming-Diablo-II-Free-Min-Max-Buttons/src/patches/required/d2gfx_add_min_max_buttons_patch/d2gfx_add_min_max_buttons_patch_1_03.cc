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

#include "d2gfx_add_min_max_buttons_patch_1_03.hpp"

#include <array>

#include "../../../asm_x86_macro.h"
#include "../../../helper/window_style.hpp"

namespace sgd2fmmb::patches {

std::vector<mapi::GamePatch>
Make_D2GFX_AddMinMaxButtonsPatch_1_03() {
  std::vector<mapi::GamePatch> patches;

  // Set up the window style buffer.
  DWORD window_style = GetWindowStyle();
  std::array<std::uint8_t, sizeof(window_style)> window_style_buffer = {
      window_style & 0xFF,
      (window_style >> (8 * 1)) & 0xFF,
      (window_style >> (8 * 2)) & 0xFF,
      (window_style >> (8 * 3)) & 0xFF,
  };

  // Create window with the min and max buttons.
  mapi::GameAddress game_address_01 = mapi::GameAddress::FromOffset(
      mapi::DefaultLibrary::kD2GFX,
      0x5983 + 3
  );

  patches.push_back(
      mapi::GamePatch::MakeGameBufferPatch(
          std::move(game_address_01),
          window_style_buffer.cbegin(),
          window_style_buffer.cend()
      )
  );

  // When resolution is modified, modify window rect adjustment to include
  // the window style.
  mapi::GameAddress game_address_02 = mapi::GameAddress::FromOffset(
      mapi::DefaultLibrary::kD2GFX,
      0x6128 + 1
  );

  patches.push_back(
      mapi::GamePatch::MakeGameBufferPatch(
          std::move(game_address_02),
          window_style_buffer.cbegin(),
          window_style_buffer.cend()
      )
  );

  return patches;
}

} // namespace sgd2fmmb::patches
