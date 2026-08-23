" Solomon: highlight offloading macros as preprocessor directives
" install: copy (or symlink) misc/editor/vim/after/syntax/ into ~/.vim/after/syntax/
"          (neovim: ~/.config/nvim/after/syntax/)
syntax match solomonDirective /\<\(SOLOMON\|PRAGMA_ACC\|PRAGMA_OMP\|ACC_CLAUSE\|OMP_CLAUSE\|OMP_TARGET\)_[A-Z0-9_]\+\>/
syntax match solomonDirective /\<OMP_PASS_LIST\>/
highlight default link solomonDirective PreProc
