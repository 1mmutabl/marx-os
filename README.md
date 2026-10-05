# marx-os

A 32-bit operating system written fully from scratch.

> **Work in progress:** marx-os is currently **not usable as a general-purpose operating system**. A lot of things are still missing.

## Building from source

As of now, the only way to run marx-os is to build it from source, but it's really easy to do!

### Dependencies

- `make`
- `libg32`
- `qemu`
- `bear` *(optional, if you want to inspect/index the source code)*

### Building

If you use **Bash** or **Fish**, you can run the corresponding build script. The script will build the OS and automatically launch it.

If you don't use either shell, you can simply use `make`:

```sh
make clean
```
Not required if you're building for the first time.

```sh
make
```
Builds the operating system.

If you're going to inspect the source code, use:
```sh
bear -- make
```
This generates the compilation database used by tools such as language servers.

To run the operating system:
```sh
make run
```

You can also define variables when running it. The Bash build script does this for me because I use it with my old PC.

## Development

Most of the development is recorded live on [Twitch](https://twitch.tv/1mmutabl).

However, after October 3rd, I started getting really bored at school, so I've also been writing the OS off-camera using my alt account, **not-1mmutabl**.

So expect some changes to be made off-camera!

<sub>not-1mmutabl was here</sub>
