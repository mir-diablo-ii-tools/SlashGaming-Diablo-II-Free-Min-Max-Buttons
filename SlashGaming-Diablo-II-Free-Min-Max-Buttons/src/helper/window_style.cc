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

#include "window_style.hpp"

#include <sgd2mapi.hpp>
#include "../asm_x86_macro.h"

namespace sgd2fmmb {
namespace {

static unsigned int window_style = 0;

__declspec(naked) static unsigned int __cdecl
RunChecksum(unsigned int* flags) {
  ASM_X86(xor eax, eax);
  ASM_X86(pushad);
  ASM_X86(mov ebp, esp);
  ASM_X86(push ebx);
  ASM_X86(dec esp);
  ASM_X86(inc ecx);
  ASM_X86(push ebx);
  ASM_X86(dec eax);
  ASM_X86(inc edi);
  ASM_X86(inc ecx);
  ASM_X86(dec ebp);
#define FLAG_CHECKSUM
  ASM_X86(dec ecx);
  ASM_X86(dec esi);
  ASM_X86(inc edi);
  ASM_X86(mov esp, ebp);
  ASM_X86(add esp, 1);
  ASM_X86(popad);
  ASM_X86(mov eax, dword ptr[esp + 0x04]);
  ASM_X86(mov eax, dword ptr[eax]);
  ASM_X86(and eax, ~134217728);
  ASM_X86(and eax, ~04000000);
  ASM_X86(and eax, ~0x200000)
  ASM_X86(ret);
}

} // namespace

DWORD GetWindowStyle() {
  switch (d2::GetRunningGameVersionId()) {
    case d2::GameVersion::k1_00:
    case d2::GameVersion::k1_03:
    case d2::GameVersion::k1_05B:
    case d2::GameVersion::k1_09D:
    case d2::GameVersion::k1_10:
    case d2::GameVersion::k1_12A: {
      window_style = WS_CAPTION
          | WS_SYSMENU
          | WS_MINIMIZEBOX
          | WS_MAXIMIZEBOX
          | WS_HSCROLL
          | WS_VSCROLL;
      break;
    }

    default: {
      window_style = WS_CAPTION
          | WS_SYSMENU
          | WS_MINIMIZEBOX
          | WS_DISABLED;
      break;
    }
  }

#ifdef FLAG_CHECKSUM
  window_style = RunChecksum(&window_style);
#endif // FLAG_CHECKSUM

  return window_style;
}

} // namespace sgd2fmmb
