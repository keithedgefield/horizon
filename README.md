Horizon
=======

```
Dear my friend,

This is the starting point to me and to you!

Regards,
Keith
```

## Overview

* `Horizon` (code name) is a scripting infrastructure for games and multimedia arts.
* `Horizon` works on various platforms including PC, Mac, smartphones, tablets, and gaming consoles.
* `Horizon` is a free software and is intended to be used by indie developers.

## Scripting Language

* Works made with `Horizon` are written in `Linguine` (code name), our project's scripting language.
* I focus on easiness of the scripting language because I have a belief that talents to make fun games are different than one to write complex code.
* For speed-sensitive codes, users may write C functions and may call them from scripts seamlessly.
* All scripts may be compiled into a binary by our `release mode`, and thus, it runs at a native speed, wow!
* All scripts may be transpiled to C source codes that use Unity or Unreal Engine.
    * This is because OSS projects cannot generate binaries by using proprietary SDKs of game console vendors.

## Porting Layer

* We use an approach to have `Hardware Abstraction Layer` (HAL) on the very bottom of our code in order to realize a portability.
* HAL is called `MediaKit` (code name).

## System Structure

```
    +-------------------------+
    |         User App        |
    +-------------------------+
    |         GameKits        |
    +-------------------------+
	|    Linguine Bindings    |
    +------------+------------+
    |  Linguine  |  MediaKit  |
    -------------+------------+
```

Let me explain it from the bottom...

* We have two base layers: `Linguine` and `MediaKit`.
    * `Linguine` and `MediaKit` are comletely independent software written in C.
* We have an upper layer called `Linguine Bindings` on top of the two base layers.
    * `Linguine Bindings` enables programmers to call `MediaKit` C functions in `Linguine` codes.
* We have an upper layer called `GameKits`.
    * `GameKits` consist of some building blocks for games.
	* A game building block is intended for making a game of a specific gaming genre.
    * For example, we may have `NovelKit` and `3D RPG Kit`.
    * Users may write a `GameKit`. 
* `User App` will be made ontop of `GameKits`.
