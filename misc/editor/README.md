# Editor support for Solomon

Since Solomon provides plain preprocessor macros, editors do not highlight them by default. The files here make the `SOLOMON_*` macros look like directives. See the top-level READMEs for details.

- `vim/after/syntax/{c,cpp,fortran}.vim`: copy (or symlink) into `~/.vim/after/syntax/` (neovim: `~/.config/nvim/after/syntax/`)
- `emacs/solomon-highlight.el`: `(load "...")` it from your `init.el`
- `vscode/solomon-highlight/`: copy (or symlink) the folder into `~/.vscode/extensions/` and restart VS Code (a build-free local injection grammar; nothing is published to the marketplace)
- `../../.clang-format` at the repository root registers the Solomon directive macros in `StatementMacros`/`AttributeMacros` as a reference setting, so that clang-format does not mangle directive lines; copy the relevant entries into your own `.clang-format`
