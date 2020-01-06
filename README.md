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

## Build

Install [premake 5](https://premake.github.io/) and download all dependencies.
Run premake to generate project file.

### Arch Linux
```bash
pacman -S sfml glew glm
premake5 gmake
```

### Windows

```bash
# Update libraries install dir ('LibsDir' and 'IncludeDir') in premake5.lua
premake5 vs2015
```
