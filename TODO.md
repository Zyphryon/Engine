**Audio**: Callbacks (OnMusicStart, OnMusicEnd, OnXXXStart, OnXXXEnd)

**Audio**: Effects (Fade, Reverb, ...)

~~**Audio**: Handle OnPause and OnResume of Application.~~

**Input**: Action, Binding and Setting serialization.

**Graphic**: Implement Read Buffer and Read Texture.

**Graphic**: Implement High Level Pass system.

**Graphic**: Move out render stuff from RHI module (Mapping, Font, Renderer)

**Graphic**: Descriptor Set (for binding to different shaders, instead of all)

~~**Math**: Implement Matrix4x4::Inverse for general matrices using SIMD.~~

~~**Collision**: Basic shape implementation.~~

**Collision**: Add OBB, Pill and Convex shapes for 2D.

**Collision**: Add 3D shapes.

**Collision**: Add Spatial Grid.

**Network**: Implement it from scratch with proper abstraction.

~~**Scene**: Builder Pattern For Query/System.~~

~~**Scene**: Abstraction of Pair, Query, System.~~

~~**Scene**: Feature rich Tag system (Name, Category, Etc).~~

**Core**: Clean core and remove external dependencies (Own STL).

**Dependencies**: Own implementation for asset loading (PNG, DXT, MP3, WAV, TOML, GLTF)

**Dependencies**: Remove FAudio with own implementation of 3D using SDL_Mixer

**Core**: Implement Job System using Fibers.

**Core**: Implement a well designed handle system.