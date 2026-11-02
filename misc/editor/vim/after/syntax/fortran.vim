" Solomon: highlight offloading macros as preprocessor directives
" install: copy (or symlink) the files {c,cpp,fortran}.vim of misc/editor/vim/after/syntax/ into ~/.vim/after/syntax/
"          (neovim: ~/.config/nvim/after/syntax/)
" the macro names are case sensitive (syntax/fortran.vim ignores case)
syntax case match
syntax match solomonDirective /\<\(SOLOMON\|PRAGMA_ACC\|PRAGMA_OMP\|ACC_CLAUSE\|OMP_CLAUSE\|OMP_TARGET\)_[A-Z0-9_]\+\>/
syntax match solomonDirective /\<\(ACC\|OMP\)_PASS_LIST\>/
syntax case ignore
highlight default link solomonDirective PreProc
