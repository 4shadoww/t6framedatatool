/*
  Copyright (C) 2025 Noa-Emil Nissinen

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.    If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef GUI_CONSTANTS_HPP
#define GUI_CONSTANTS_HPP

#define WINDOW_NAME "T6FDTOOL"
#define STAT_LINES 4
#define FONT_SIZE 25
#define TEXT_MARGIN 5
#define LINE_HEIGHT (FONT_SIZE + TEXT_MARGIN)
#define MAX_WIDTH 1000
#define MAX_HEIGTH (STAT_LINES * LINE_HEIGHT)
#define ANALYSER_START_INTERVAL 1

// UI
#define NO_STARTUP_FRAMES "Startup Frames --- (No game hooked)"
#define NO_FRAME_ADVANTAGE "Frame Advantage ---"
#define NO_STATUS "Status ---"
#define NO_DISTANCE "Distance ---"

#define BORDER_MARGIN_X 25
#define BORDER_MARGIN_Y 15

#define STARTUP_FRAMES "Startup Frames %d"
#define FRAME_ADVANTAGE "Frame Advantage %d"
#define FRAME_ADVANTAGE_KD "Frame Advantage KD"
#define STATUS "Status %s"
#define DISTANCE "Distance %.2f"

// RPSC3 Window properties
#define RPSC3_CLASS "rpcs3"
#define RPSC3_NAME "fps:"

#endif
