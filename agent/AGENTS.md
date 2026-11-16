# AGENTS.md

## Scope

`agent/` is a standalone C++17 command-line library manager. The sibling
`apprentissage/` and `analyse_image/` directories are separate Python/ML
notebooks and do not affect this build.

## Build and run

From `agent/`:

```bash
cmake -S . -B build
cmake --build build
./build/bibliotheque -d data
```

`-d <dir>` (or `--data-dir`) is required in practice: without it, the book and
user paths are empty strings and file loading/saving fails.

## Verification

There are no automated tests, linter, or CI. Build cleanly and exercise the CLI
manually. CMake enables `-Wall -Wextra -Wpedantic`; treat new warnings as
regressions.

## Gotchas

- `CMakeLists.txt` uses `file(GLOB *.cpp)`: re-run the configure step
  (`cmake -S . -B build`) after adding or removing a source file.
- `build/` is ignored; the executable is `build/bibliotheque`.
- Menu option `0` saves all books/users into the data directory. Running
  against `agent/data/` therefore modifies tracked fixtures; copy the directory
  for manual tests.
- Menu option `13` writes `books.txt.backup` and `users.txt.backup` beside the
  data files. Clean these up after testing.
- `.vscode/settings.json` at the repo root hardcodes a Windows CMake source
  path. Use the CLI commands above or override it locally.

## Data formats

- `books.txt`: `title|author|isbn|available(1/0)|borrowerId`
- `users.txt`: `name|userId|isbn,isbn,...`
- Files contain UTF-8 French accents; preserve the encoding.

## Code conventions

- Standard library only; no third-party C++ dependencies.
- `Library` owns `unique_ptr<Book>` / `unique_ptr<User>`; `FileManager` handles
  all persistence.
- User-facing output is French; keep messages in French and accents intact.
