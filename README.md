# lin

A codebase analytics engine. Tracks file metadata, line counts, growth trends, language composition, and project progress over time.

lin does **not** store file contents. It records only metadata: line counts, file sizes, SHA1 hashes (for change detection), and timestamps. Think of it as a fitness tracker for your codebase.

## Build

Requires CMake 3.10+ and OpenSSL.

```sh
mkdir build && cd build
cmake ..
make
```

The binary is at `build/lin`.

## Quick Start

```sh
lin init
lin add src/*.c src/*.h
lin checkpoint -m "initial baseline"
```

That's it. You now have a snapshot of your codebase metrics.

## Commands

### `lin init`

Create a `.lin/` directory in the current project. A `default` group is created automatically.

```
$ lin init
```

Run this once at the root of your project. All lin data lives inside `.lin/`.

### `lin add <file>...` (alias: `a`)

Track files for analytics. Each file is scanned in a single pass for line count, size, and SHA1 hash.

```
$ lin add main.c utils.c lib/*.c
3 added, 0 updated, 0 unchanged in 'default'
```

If a file is already tracked and its content changed, the manifest entry is updated automatically. Files matching ignore patterns are skipped.

**Options:**
- `-g, --group <name>` -- add to a specific group
- `-v, --verbose` -- show per-file details (added/updated/unchanged/ignored)

### `lin remove <file>...` (alias: `rm`)

Stop tracking files. Removes them from the group manifest.

```
$ lin remove old_module.c
removed 1 file(s) from 'default'
```

**Options:**
- `-g, --group <name>` -- remove from a specific group
- `-v, --verbose` -- show per-file details

### `lin group <subcommand>`

Manage groups. Groups are independent collections of tracked files, each with their own manifest and checkpoint history.

```
$ lin group create backend
created group 'backend'

$ lin group list
  default               12 files  3847 lines  3 checkpoints
  backend                0 files     0 lines  0 checkpoints

$ lin group remove backend
removed group 'backend'
```

**Subcommands:**
- `create` / `mk` `<name>` -- create a new group
- `remove` / `rm` `<name>` -- delete a group (cannot remove `default`)
- `list` / `ls` -- list all groups with stats

Group names: alphanumeric, hyphens, underscores. Max 24 characters.

### `lin checkpoint` (alias: `cp`)

Snapshot the current state of all tracked files in the group. Creates a binary checkpoint with line counts, sizes, and hashes. Shows delta from the previous checkpoint when available.

```
$ lin checkpoint -m "after refactor"
checkpoint #2 "after refactor"
  files: 12
  lines: 4102 (+255)
  size:  98304 bytes
```

**Options:**
- `-g, --group <name>` -- checkpoint a specific group
- `-m, --message <text>` -- attach a message to the checkpoint
- `-v, --verbose` -- show per-file line counts

### `lin status` (alias: `st`)

Show changes to tracked files since they were last added or updated. Compares current file hashes against the manifest.

```
$ lin status
  modified:  src/parser.c (340 -> 385 lines, +45)
  missing:   src/old.c

group 'default': 12 tracked
  1 modified
  1 missing
  10 unchanged
  lines: 3847 tracked, 3892 current (+45)
```

**Options:**
- `-g, --group <name>` -- check a specific group
- `-v, --verbose` -- show every file (including unchanged)

### `lin log`

Show chronological checkpoint history with timestamps, file counts, line counts, and deltas between checkpoints.

```
$ lin log
#1       2026-03-10 14:22:01     12 files     3847 lines  "initial baseline"
#2       2026-03-11 09:15:33     12 files     4102 lines  +255  "after refactor"
#3       2026-03-12 16:40:08     14 files     4891 lines  +789  "new module"
```

**Options:**
- `-g, --group <name>` -- log for a specific group
- `-v, --verbose` -- show per-file breakdown per checkpoint

### `lin stats`

Show project analytics: group summary, extension/language breakdown by line count and file count, and growth trend between checkpoints.

```
$ lin stats
group: default
  created:     2026-03-10 14:20:00
  updated:     2026-03-12 16:40:08
  files:       14
  lines:       4891
  checkpoints: 3

by extension:
  .c             8 files     3200 lines
  .h             6 files     1691 lines

growth trend:
  #1   -> #2     +255 lines
  #2   -> #3     +789 lines
  average: +522 lines/checkpoint
```

**Options:**
- `-g, --group <name>` -- stats for a specific group

### `lin diff <id1> <id2>`

Compare two checkpoints. Shows added, removed, and modified files with line and size deltas.

```
$ lin diff 1 3
diff #1 -> #3

  added:    src/module.c (450 lines)
  added:    src/module.h (82 lines)
  modified: src/parser.c (+45 lines)

summary:
  2 added, 0 removed, 1 modified
  lines: 3847 -> 4891 (+1044)
  size:  85200 -> 112400 (+27200 bytes)
```

**Options:**
- `-g, --group <name>` -- diff checkpoints in a specific group

### `lin fsck`

Verify data integrity across all groups (or a specific group). Validates binary headers, manifest parsing, and checkpoint files.

```
$ lin fsck
checking 'default'...
  info: ok (magic valid, version 1)
  manifest: 14 entries
  checkpoints: 3 found
    #1: ok (12 files, 3847 lines)
    #2: ok (12 files, 4102 lines)
    #3: ok (14 files, 4891 lines)

all checks passed
```

**Options:**
- `-g, --group <name>` -- check only a specific group

### `lin help [command]` (alias: `h`)

Show general help or per-command help.

```
$ lin help checkpoint
```

### `lin --version` / `lin -V`

```
$ lin --version
lin 0.2.0
```

## Global Options

These can appear before or after the command:

| Flag | Description |
|------|-------------|
| `-g, --group <name>` | Operate on a specific group (default: `default`) |
| `-m, --message <text>` | Attach message to checkpoint |
| `-v, --verbose` | Detailed output |
| `-V, --version` | Print version and exit |
| `-H, --help` | Print help and exit |

## `.linignore`

Place a `.linignore` file at your project root to control which files `lin add` skips. Syntax:

```
# comments start with #
*.log
tmp/
secret.key

# negation: re-include a previously ignored pattern
!important.log
```

Patterns use glob matching (`fnmatch`). A pattern like `node_modules` matches the directory name in any position in the path.

**Built-in defaults** (always active):

| Category | Patterns |
|----------|----------|
| Version control | `.git`, `.lin`, `.svn`, `.hg` |
| Build artifacts | `build`, `dist`, `target`, `bin`, `obj` |
| Dependencies | `node_modules`, `vendor`, `__pycache__`, `.venv`, `venv` |
| Compiled objects | `*.o`, `*.so`, `*.dylib`, `*.a`, `*.dll`, `*.exe`, `*.class`, `*.pyc` |
| Binary media | `*.jpg`, `*.jpeg`, `*.png`, `*.gif`, `*.ico`, `*.pdf`, `*.zip`, `*.tar`, `*.gz`, `*.bz2` |
| IDE/editor | `.idea`, `.vscode`, `*.swp`, `*.swo`, `.DS_Store` |

## Full Workflow Example

Track a C project's growth over a week-long development cycle.

### Day 1: Project setup

```sh
$ cd ~/projects/webapp

$ lin init
initialized .lin

$ lin add src/main.c src/server.c src/router.c include/server.h include/router.h
5 added, 0 updated, 0 unchanged in 'default'

$ lin checkpoint -m "project skeleton"
checkpoint #1 "project skeleton"
  files: 5
  lines: 342
  size:  8120 bytes
```

### Day 2: Add database layer

```sh
$ lin add src/db.c src/models.c include/db.h include/models.h
4 added, 0 updated, 0 unchanged in 'default'

$ lin status -v
  unchanged: src/main.c (45 lines)
  unchanged: src/server.c (120 lines)
  modified:  src/router.c (88 -> 134 lines, +46)
  unchanged: include/server.h (22 lines)
  unchanged: include/router.h (18 lines)

group 'default': 5 tracked
  1 modified
  4 unchanged

$ lin add src/router.c
0 added, 1 updated, 0 unchanged in 'default'

$ lin checkpoint -m "database layer"
checkpoint #2 "database layer"
  files: 9 (+4)
  lines: 890 (+548)
  size:  21400 bytes
```

### Day 3: Organize with groups

```sh
$ lin group create frontend
created group 'frontend'

$ lin -g frontend add static/app.js static/style.css templates/index.html
3 added, 0 updated, 0 unchanged in 'frontend'

$ lin -g frontend checkpoint -m "initial frontend"
checkpoint #1 "initial frontend"
  files: 3
  lines: 210
  size:  5800 bytes

$ lin group list
  default               9 files   890 lines  2 checkpoints
  frontend              3 files   210 lines  1 checkpoints
```

### Day 4: Heavy development

```sh
$ lin checkpoint -m "auth + middleware"
checkpoint #3 "auth + middleware"
  files: 9
  lines: 1456 (+566)
  size:  35200 bytes

$ lin log
#1       2026-03-10 09:00:12      5 files      342 lines  "project skeleton"
#2       2026-03-11 17:30:45      9 files      890 lines  +548  "database layer"
#3       2026-03-13 14:22:08      9 files     1456 lines  +566  "auth + middleware"
```

### Day 5: Analytics review

```sh
$ lin stats
group: default
  created:     2026-03-10 09:00:00
  updated:     2026-03-13 14:22:08
  files:       9
  lines:       1456
  checkpoints: 3

by extension:
  .c             5 files     1150 lines
  .h             4 files      306 lines

growth trend:
  #1   -> #2     +548 lines
  #2   -> #3     +566 lines
  average: +557 lines/checkpoint

$ lin diff 1 3
diff #1 -> #3

  added:    src/db.c (280 lines)
  added:    src/models.c (245 lines)
  added:    include/db.h (42 lines)
  added:    include/models.h (38 lines)
  modified: src/main.c (+30 lines)
  modified: src/server.c (+95 lines)
  modified: src/router.c (+46 lines)
  modified: include/server.h (+12 lines)
  modified: include/router.h (+8 lines)

summary:
  4 added, 0 removed, 5 modified
  lines: 342 -> 1456 (+1114)
  size:  8120 -> 35200 (+27080 bytes)
```

### Day 6: Cleanup and verify

```sh
$ lin remove src/models.c
removed 1 file(s) from 'default'

$ lin add src/orm.c
1 added, 0 updated, 0 unchanged in 'default'

$ lin checkpoint -m "replaced models with orm"
checkpoint #4 "replaced models with orm"
  files: 9
  lines: 1580 (+124)
  size:  38400 bytes

$ lin fsck
checking 'default'...
  info: ok (magic valid, version 1)
  manifest: 9 entries
  checkpoints: 4 found
    #1: ok (5 files, 342 lines)
    #2: ok (9 files, 890 lines)
    #3: ok (9 files, 1456 lines)
    #4: ok (9 files, 1580 lines)

checking 'frontend'...
  info: ok (magic valid, version 1)
  manifest: 3 entries
  checkpoints: 1 found
    #1: ok (3 files, 210 lines)

all checks passed
```

## Data Storage

All data lives in `.lin/` at the project root:

```
.lin/
  default/
    manifest          # tracked file metadata (text)
    info              # group summary (binary, LINF header)
    checkpoints/
      000001.cp       # snapshot (binary, LINC header)
      000002.cp
  frontend/
    manifest
    info
    checkpoints/
      000001.cp
```

- **manifest** -- text format with `#lin-manifest-v1` header, tab-separated entries
- **info** -- binary with 8-byte header (magic `LINF`, version, flags), timestamps, counters
- **checkpoints** -- binary with 8-byte header (magic `LINC`), per-file entries with SHA1, line count, size

All binary writes are atomic (write to temp, fsync, rename). Advisory file locking prevents concurrent corruption.

## License

MIT
