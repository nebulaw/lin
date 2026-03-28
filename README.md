# lin

Lin is just mnaawh. Lin means everything whatever you'd like to. It might be Leverage Insufficient Napping or whatever really. But remember, it's just a tool to help you look over your project and how it goes.

## Build

You need CMake 3.10+ and OpenSSL.

```sh
mkdir build && cd build
cmake ..
make
```
The binary will be compiled to `build/lin`.

## Quick Start

Initialize `lin`, add some files, and create your first baseline snapshot.

```sh
lin init
lin add src/*.c src/*.h
lin checkpoint -m "initial baseline"
```

For the complete manual, command reference, and a full workflow example, see **[USAGE.md](USAGE.md)**.

## Ignoring files

By default, `lin` ignores common artifacts (`.git`, `node_modules`, `build/`, `*.o`, etc.). 

If you need to ignore specific project files, create a `.linignore` file at your project root using standard glob syntax:

```text
*.log
tmp/
!important.log
```

## Data Storage

All data is stored locally in `.lin/`. The manifest is plain text, while historical checkpoints use an optimized binary format (`LINF`/`LINC` headers) to keep operations fast as your project history grows. You can run `lin fsck` at any time to verify data integrity.

## License

MIT
