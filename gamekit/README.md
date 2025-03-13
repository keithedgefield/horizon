Momo
====

GameKit is the lowest layer of our framework, designed to abstract
platform differences and provide common interfaces for the upper layer
code. The Momo library includes a few functional components covering
file operations, storage, image manipulation, font rendering, audio
processing, graphics rendering, sound mixing, input handling, and
system features. The codebase is modularized to support multiple
platforms: Linux, Windows, macOS, iOS, Android, and Web browsers.

## Layers

GameKit acts as a HAL (hardware abstraction layer) for our engine. It
absorbs differences of platforms to make the upper layer code single
and common.

```
+------------------------------------------------+
| Upper Layer Code (Platform-Independent)        |
+---------+-----------+--------------------------+
| Win HAL | Linux HAL | (Platform-Dependent) ... |
+---------+-----------+--------------------------+
```

## Callbacks

HAL provides startup routines such as `main()`, and thus, for the
upper layer code, the following callbacks form the skeleton of the
application.

| Callback Function | Details |
| --- | --- |
| `on_hal_init_render()` | This callback must return the window title and the size. |
| `on_hal_ready()`  | This callback may prepare the rendering objects. |
| `on_hal_frame()`  | This callback must do a frame rendering. |

## Components

Momo is composed of 10 distinct functional components that form the core of its architecture. Each component serves a specific purpose in the framework, from handling basic file operations to complex graphics rendering. The implementation of these essential components is managed through a variety of platform-dependent modules, ensuring optimal performance across different operating systems and environments.

|Component |Description                   |
|----------|------------------------------|
|file_     |Asset access.                 |
|stor_     |Save data access.             |
|image_    |Image manipulation.           |
|font_     |Font drawing.                 |
|wave_     |Audio decoding.               |
|render_   |Graphics rendering.           |
|mixer_    |Audio playback.               |
|input_    |Key and gamepad input.        |
|sys_      |System features.              |
|shader_   |Shader parser.                |

## The modules of the implementation

Each component in our framework is implemented by a platform-specific
C module to adopt across operating systems. The table below maps modules
to their supported platforms, showing our cross-platform
implementation strategy and helping developers identify which modules
handle specific functionality.

|Module     |Description                 |Linux  |Windows|macOS  |iOS    |Android|Web    |
|-----------|----------------------------|-------|-------|-------|-------|-------|-------|
|stdfile    |file_ for standard C.       |v      |v      |v      |v      |       |v      |
|ndkfile    |file_ for Android API.      |       |       |       |       |v      |       |
|stdstor    |stor_ for standard C.       |v      |v      |v      |v      |       |v      |
|ndkstor    |stor_ for Android API.      |       |       |       |       |v      |       |
|stdimage   |image_ for standartd C.     |v      |v      |v      |v      |v      |v      |
|stdfont    |font_ for standard C.       |v      |v      |v      |v      |v      |v      |
|glrender   |render_ for OpenGL.         |v      |       |       |       |v      |v      |
|vkrender   |render_ for Vulkan.         |       |       |       |       |       |       |
|dx11render |render_ for DirectX 11.     |       |v      |       |       |       |       |
|dx12render |render_ for DirectX 12.     |       |v      |       |       |       |       |
|metalrender|render_ for Metal.          |       |       |v      |v      |       |       |
|alsamixer  |mixer_ for ALSA.            |v      |       |       |       |       |       |
|dsmixer    |mixer_ for DirectSound 5.   |       |v      |       |       |       |       |
|aumixer    |mixer_ for AudioUnit.       |       |       |v      |v      |       |       |
|almixer    |mixer_ for OpenAL.          |       |       |       |       |v      |v      |
|linuxmain  |sys_ for Linux.             |v      |       |       |       |       |       |
|winmain    |sys_ for Windows.           |       |v      |       |       |       |       |
|nsmain     |sys_ for macOS.             |       |       |v      |       |       |       |
|uimain     |sys_ for iOS.               |       |       |       |v      |       |       |
|ndkmain    |sys_ for NDK.               |       |       |       |       |v      |       |
|emmain     |sys_ for Emscripten.        |       |       |       |       |       |v      |
|shaderv1   |shader_ (version 1).        |v      |v      |v      |v      |v      |v      |
