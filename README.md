# MinGE

Experimental 3D game engine.

## References

General architecture is heavily inspired from Unity.

#### Renderer:
 * [Stateless, layered, multi-threaded rendering](https://blog.molecular-matters.com/2014/11/06/stateless-layered-multi-threaded-rendering-part-1/)
 * [Order your graphics draw calls around!](http://realtimecollisiondetection.net/blog/?p=86)
 * [bgfx](https://github.com/bkaradzic/bgfx)

#### Physic Engine
 * [bullet3](https://github.com/bulletphysics/bullet3)

#### Other
 * [JSON parser](https://github.com/nlohmann/json)
 * [Lock-free job system](https://blog.molecular-matters.com/tag/job-system/)
 * [microprofile](https://github.com/zeux/microprofile)


## Dependencies
* [SFML](https://www.sfml-dev.org/download/sfml/2.5.1/)
* [GLEW](http://glew.sourceforge.net/index.html)
* [GLM](https://github.com/g-truc/glm/releases/)
* [Ultralight](https://github.com/ultralight-ux/Ultralight/releases)

## Build

Install [premake 5](https://premake.github.io/) and all the above dependencies.
You may want to modify libraries path in premake5.lua

When ready, run premake with the desired target:
```bash
# For unix makefiles
premake5 gmake

# For visual studio 2017
premake5 vs2017
```

### Installing dependencies on arch

```bash
pacman -S sfml glew glm
```
