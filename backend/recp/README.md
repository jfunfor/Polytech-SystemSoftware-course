# recp

A simple utility written in C for configuring a virtual machine.

### Recipes

- Commands supported:

	- `CREATE <dir>` – Create directory.
	- `MOVE <source> <destination> `– Move file.
	- `EXEC <command>` – Execute shell command.

- Comments supported: Lines starting with `#` are ignored.
- Whitespace agnostic.

### Build & Usage

```bash
$ gcc -o recp recp.c
$ sudo ./recp <id>
```

### TODO

Implement searching for the recipe in task directory using valid patterns.
