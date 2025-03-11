Horizon Project
===============

`Horizon Project` offers a suite of libraries for gaming
applications. Using these libraries, developers can create polished
games quickly with exceptional portability. Programs built with
Horizon's libraries run seamlessly across multiple operating systems
and devices - including Linux, Windows, macOS, iOS, Android, and Web
browsers. Our goal is to create a lightweight yet powerful framework
that both beginners and experienced developers can easily use.

## Linguine: A Fast, Modern, Compact Scripting Language

Linguine is a fast, compact scripting language for application
integration. With its C-like syntax, it includes modern features like
iterators and dictionaries, making it both beginner-friendly and
powerful. The language includes a Just-in-Time compiler that generates
native code for Intel, AMD, and Arm architectures, with interpreter
fallback for other platforms.

```
cd linguine
cd build
cd linux
make
./linguine ../../hello.ls     # JIT, a faster method
./linguine -i ../../hello.ls  # Interpreter, a safe mode method
```

## Momo: A Modern, Ultra-Portable Gaming API

`Momo` is the foundation framework of Horizon Project, provides a
modern, efficient C API that abstracts away platform-specific details
while maintaining excellent performance. The architecture follows a
component-based design, enabling modularity and portability.

## SpriteKit: A 3D Game Library

Horizon provides `SpriteKit` - a comprehensive C++ and Linguine
toolkit designed specifically for 2D game development. This versatile
component offers a rich set of features and optimized rendering
capabilities that streamline the game creation process.

## WorldKit: A Scene-Based 3D Framework

A scene-based 3D framework built on Momo, written for C++ and Linguine.

## NovelKit: A Visual Novel Framework

A visual novel engine on SpriteKit and WorldKit, written for Linguine.
