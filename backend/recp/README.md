# recp

A simple utility for setting up Linux environments based on recipe files.

## Features

- Operations:
	- `CREATE_DIR path [path...]`    - Create directories.
	- `CREATE_FILE path [path...]`   - Create empty files.
	- `MOVE source destination`      - Move/rename files.
	- `COPY source destination`      - Copy files.
	- `DELETE path [path...]`        - Delete files/directories
	- `EXEC command [args...]`       - Execute shell commands.
	- `SCRIPT ... END_SCRIPT`        - Run Bash/Python/other scripts.
	- `BACKGROUND command [args...]` - Execute shell commands in background.
	- `DEFINE name value`            - Define a variable (substitute value with %name%).

## Build

```bash
$ make
$ ./recp -h
```
