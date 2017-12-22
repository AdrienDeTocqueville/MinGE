# MinGE

Experimental 3D game engine

## Dependencies
* [SFML GCC 4.9.2 TDM (SJLJ) - 32-bit](https://www.sfml-dev.org/download/sfml/2.4.2/index-fr.php)
* [GLEW](http://glew.sourceforge.net/index.html)
* [GLM](https://github.com/g-truc/glm/releases/tag/0.9.9-a2)

## Build options

### Link libraries
* opengl32
* glu32

*release
-lsfml-audio-s
-lopenal32
-lflac
-lvorbisenc
-lvorbisfile
-lvorbis
-logg
-lsfml-graphics-s
-lfreetype
-ljpeg
-lsfml-window-s
-lopengl32
-lgdi32
-lsfml-network-s
-lws2_32
-lsfml-system-s
-lwinmm

### Defines
GLEW_STATIC
SFML_STATIC
