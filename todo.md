# TODO
A list of things that i want to do, just so i keep focused and don't drift off on random ideas

## Higher priority
- Make an abstraction to render assets (at least models, sprites, fonts)
    - Maybe an abstraction to render simple shapes too?
- Finish rewriting the camera class
- Revisit the system to upload drawing commands to the render context.
    - Also add support for uniform buffers & SSBOs

## Medium priotity
- Rewrite the transform graph system.
    - Maybe design some kind of scene system
- Fix the messy code for expected\<T\>
    - Maybe optional\<T\> too
- Reduce source files size?
- Import more material data from models
- Sound system
- 2D & 3D physics systems
- Lighting system
- More demos :p

## Low priority
- Some kind of assset management system
    - Could rewrite the asset pool? Maybe using the fixed size arena or something similar
    - Maybe add some virtual filesystem
- Provide a way to load assets using an arena or some other allocator
- GLM replacement
- Vulkan renderer
- Add an option to use other windowing systems instead of GLFW
    - SDL2 and native X11 window
    - Maybe win32 and Wayland
- Multiple windows support
- Some kind of serialization system
- Lua bindings?
    - or expose a C interface for free FFI?
- Multithreading support?
- Windows suppor
    - DirectX renderer maybe?
- Switch support
    - deko3d renderer too?
    - Could add some support for using Borealis
- Emscripten support
