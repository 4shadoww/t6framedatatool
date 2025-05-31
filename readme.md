Tekken 6 frame data tool
========================

Tekken 6 frame data overlay for RPCS3.

[![Tekken 6 frame data tool](t6framedata.png)](https://www.youtube.com/watch?v=juCl0Y5Rw4U)

Compiling
---------

Dependencies:
- C and C++ compiler (Tested with GCC)
- CMake
- Make
- Ninja

For X11 GUI (optional):
- X11 library
- Adobe Courier font family

Make targets:

```bash
# Release build
make release

# Clean
make clean

# Debug build
make debug

# Run tests
make run_test
```

Missing features
----------------

- Active frames

Known bugs
----------

- Frame data of moves which do damage at multiple points of the move are not calculated correctly.
  For example Alisa Destroy Form.

- Minor GUI flickering and text unreadable on white backgrounds caused by the current way of rendering text. The GUI should probably be rewritten in GLFW.
