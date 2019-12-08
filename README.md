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

#### JSON parser
 * [json](https://github.com/nlohmann/json)


## Dependencies
* [SFML GCC 4.9.2 TDM (SJLJ) - 32-bit](https://www.sfml-dev.org/download/sfml/2.4.2/index-fr.php)
* [GLEW](http://glew.sourceforge.net/index.html)
* [GLM](https://github.com/g-truc/glm/releases/)

## Build on Linux

```bash
pacman -S sfml glew glm
make
```

## Build on Windows

Use codeblocks project file
