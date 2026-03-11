# lin

A fast, local codebase analytics engine. `lin` tracks file metadata, line counts, growth trends, language composition, and project progress over time.

It does not store your code or send data anywhere. It only tracks metadata: line counts, file sizes, SHA1 hashes, and timestamps. Think of it as a fitness tracker for your repo.

## Why use lin?

- **Privacy first.** It explicitly ignores code content and operates entirely offline.
- **Fast and safe.** Written in C. It uses atomic binary writes and advisory file locking to ensure your analytics data never gets corrupted.
- **Familiar workflow.** The CLI mirrors common `git` commands (`init`, `add`, `status`, `checkpoint`, `log`).
- **Logical isolation.** You can group files logically (e.g., `frontend` vs `backend`) to track architectural pieces independently.

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
