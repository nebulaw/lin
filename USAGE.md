# lin Usage Guide

`lin` operates on a simple cycle: track files, take snapshots, and analyze the data.

## Command Reference

### Tracking
* `lin init` - Initializes the `.lin/` repository and creates the `default` group. Run once at the project root.
* `lin add <file>...` - Starts tracking files or updates their metadata. Scans files in a single pass. 
* `lin remove <file>...` - Stops tracking files.
* `lin status` - Shows modified or missing files since your last `add` or `checkpoint`.

### Snapshots
* `lin checkpoint -m "message"` - Creates a permanent snapshot of the currently tracked files.
* `lin log` - Displays a chronological history of your checkpoints.
* `lin diff <id1> <id2>` - Compares two checkpoints and shows exact additions, removals, and modifications.

### Analytics & Groups
* `lin stats` - Displays group summaries, file extension breakdowns, and average growth trends.
* `lin group create <name>` - Creates a new group (e.g., to track `frontend` separately from `backend`).
* `lin group list` - Lists all existing groups.
* `lin -g <name> <command>` - Executes a command against a specific group rather than `default`.

## Example Workflow

Here is an example of what a typical development cycle looks like using `lin`.

```bash
# Initialize the repository
$ lin init
initialized .lin

# Add your core project files
$ lin add src/*.c include/*.h
5 added, 0 updated, 0 unchanged in 'default'

# Create a baseline checkpoint
$ lin checkpoint -m "project skeleton"
checkpoint #1 "project skeleton"
  files: 5
  lines: 342
  size:  8120 bytes

# After writing some new features, check your status
$ lin status -v
  unchanged: src/main.c (45 lines)
  modified:  src/router.c (88 -> 134 lines, +46)
  unchanged: include/server.h (22 lines)

# Update the tracker and add the new files you created
$ lin add src/router.c src/db.c include/db.h
2 added, 1 updated, 0 unchanged in 'default'

# Snapshot the new state
$ lin checkpoint -m "added database layer"
checkpoint #2 "added database layer"
  files: 7 (+2)
  lines: 890 (+548)
  size:  21400 bytes

# If you have separate modules, you can track them in their own group
$ lin group create frontend
created group 'frontend'

$ lin -g frontend add static/app.js static/style.css
2 added, 0 updated, 0 unchanged in 'frontend'

$ lin -g frontend checkpoint -m "initial ui"
checkpoint #1 "initial ui"
  files: 2
  lines: 210
  size:  5800 bytes

# Review your overall project metrics
$ lin stats
group: default
  created:     2026-03-10 09:00:00
  files:       7
  lines:       890
  checkpoints: 2

by extension:
  .c             4 files     650 lines
  .h             3 files     240 lines

growth trend:
  #1   -> #2     +548 lines
  average: +548 lines/checkpoint

# See specifically what changed between your first and second checkpoints
$ lin diff 1 2
diff #1 -> #2

  added:    src/db.c (280 lines)
  added:    include/db.h (42 lines)
  modified: src/router.c (+46 lines)

summary:
  2 added, 0 removed, 1 modified
  lines: 342 -> 890 (+548)
```
