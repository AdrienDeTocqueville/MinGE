# MinGE

Experimental data oriented 3D game engine.

## References

#### Renderer:
 * [Stateless, layered, multi-threaded rendering](https://blog.molecular-matters.com/2014/11/06/stateless-layered-multi-threaded-rendering-part-1/)
 * [Order your graphics draw calls around!](http://realtimecollisiondetection.net/blog/?p=86)
 * [bgfx](https://github.com/bkaradzic/bgfx)

#### Other
 * [JSON parser](https://github.com/nlohmann/json)
 * [Lock-free job system](https://blog.molecular-matters.com/tag/job-system/)


## Dependencies

* [SDL 2](https://www.libsdl.org/)
* [GLEW](http://glew.sourceforge.net/index.html)
* [GLM](https://github.com/g-truc/glm/releases/)

#### Included
* [microprofile](https://github.com/zeux/microprofile)
* [Dear ImGui](https://github.com/ocornut/imgui)
* [stb](https://github.com/nothings/stb)

## Build

Install [premake 5](https://premake.github.io/) and all the above dependencies.
You may want to modify libraries path in premake5.lua

To generate project files, run premake with the desired target:
```bash
# For unix makefiles
premake5 gmake

# For visual studio 2017
premake5 vs2017
```

### Installing dependencies on arch

```bash
pacman -S sdl2 glew glm
```
