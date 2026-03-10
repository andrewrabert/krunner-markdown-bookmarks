# krunner-markdown-bookmarks

KRunner plugin for KDE Plasma 6 that opens web links from a user-configured Markdown file.

## Project structure

- `CMakeLists.txt` / `src/CMakeLists.txt` — CMake build system (KDE/ECM conventions)
- `src/runner/` — KRunner plugin (entry point: `markdownbookmarks.cpp`)
- `src/core/` — Core logic: `Bookmark` (model + relevance scoring), `FileReader` (markdown parser), `Config` (config keys/paths), `macros.h` (QSL helper)
- `src/config/` — KCModule settings UI: config form, `.ui` file, browse-for-file dialog

## Build

```sh
cmake -B build
cmake --build build
```

Requires: Qt 6.7+, KF6 (Runner, KCMUtils, KIO), ECM. Targets Plasma 6.

## Key details

- Language: C++ (Qt 6 / KDE Frameworks 6)
- License: GPL-3.0
- Trigger keyword: `bookmark` (regex in `markdownbookmarks.h:17`)
- Config stored in `krunnerrc` under `[Runners][markdownbookmarks]`
- Markdown parsing in `FileReader::parseMarkdownFile` — extracts `[title](url)` links from list items
- File watcher auto-reloads bookmarks when the markdown file changes
- Relevance matching is case-insensitive substring on bookmark name
- CPack packaging configured for Ubuntu (DEB), Fedora/openSUSE (RPM)
- AUR package: `plasma6-runners-markdown-bookmarks`
- Clang-format enforced via KDE git pre-commit hook

## Conventions

- KDE coding style with clang-format
- `#pragma once` in core headers, `#ifndef` guards in config/runner headers
- Based on [alex1701c/EmojiRunner](https://github.com/alex1701c/EmojiRunner) plugin structure
