# TODO
A list of things that i want to do, just so i keep focused and don't drift off on random ideas

## Higher priority
- Use simple POD like types for asset data
    - Design the POD types for models, meshes, materials, fonts and atlases
- Rewrite the asset loaders to use expected\<T\> and the POD types
- Use the OpenGL texture formats properly
- Provide a way to load assets using an arena or some other allocator
- Finish rewriting the camera class
- Revisit the system to upload drawing commands to the render context.
    - Also add support for uniform buffers

## Medium priotity
- Fix the circular dependency between the window class and the render context class somehow.
- Rewrite the transform graph system.
    - Maybe design some kind of scene system
- Fix the messy code for expected\<T\>
    - Maybe optional\<T\> too
- Reduce source files size?
- Provide some interface for assets to interact with the render context with less boilerplate
    - Maybe something like the font rendering system in BGFX?
    - Or maybe some wrapper class for each common asset type (models, atlas, fonts) like the old
    way i was doing things.
- Skeletal animation system
- Sound system
- 2D & 3D physics systems
- Lighting system
- More demos :p

## Low priority
- Some kind of assset management system
    - Could rewrite the asset pool? Maybe using the fixed size arena or something similar
    - Maybe add some virtual filesystem
- GLM replacement
- Make our render context own the OpenGL context, instead of implicitly using the GLFW one
- Vulkan renderer
- Add an option to use other windowing systems instead of GLFW
    - SDL2 and native X11 window
    - Maybe win32 and Wayland
- Multiple windows support
- Some kind of serialization system
- Lua bindings?
- Multithreading support?
- Windows suppor
    - DirectX renderer maybe?
- Switch support
    - deko3d renderer too?
    - Could add some support for using Borealis
- Emscripten support
