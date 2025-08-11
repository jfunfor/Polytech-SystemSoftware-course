# recp

A simple utility written in C for setting up Linux environments based on recipe files.

## Features

- Operations:
	- `CREATE_DIR path [path...]` - Create directories.
	- `CREATE_FILE path [path...]` - Create empty files.
	- `MOVE source destination ` – Move/rename files.
	- `COPY source destination` - Copy files.
	- `DELETE path [path...]` - Delete files/directories
	- `EXEC command [args...]` - Execute shell commands.
	- `SCRIPT ... END_SCRIPT` - Run Bash/Python/other scripts.

- Comments: Lines starting with `#` are ignored.
- Logging: All operations are logged to `recp.log` with timestamps.
- Security:
	- Safe path handling;
	- Proper permissions;
	- Process isolation for command execution.

## Build & Usage

```bash
$ make
$ ./recp
```

### TODO

Implement searching for the recipe in task directory using valid patterns.
