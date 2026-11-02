;;; solomon-highlight.el --- highlight Solomon offloading macros
;; install: add (load "/path/to/solomon/misc/editor/emacs/solomon-highlight.el") to your init.el
(defconst solomon-highlight-regexp
  "\\_<\\(?:\\(?:SOLOMON\\|PRAGMA_ACC\\|PRAGMA_OMP\\|ACC_CLAUSE\\|OMP_CLAUSE\\|OMP_TARGET\\)_[A-Z0-9_]+\\|\\(?:ACC\\|OMP\\)_PASS_LIST\\)\\_>"
  "Names of the Solomon offloading macros.")
(defun solomon-highlight-match (limit)
  "Search for a Solomon macro name before LIMIT, case sensitively (Fortran modes fold case)."
  (let ((case-fold-search nil))
    (re-search-forward solomon-highlight-regexp limit t)))
(defun solomon-highlight-setup ()
  "Highlight Solomon offloading macros as preprocessor directives."
  (font-lock-add-keywords nil
    '((solomon-highlight-match . font-lock-preprocessor-face))))
(dolist (hook '(c-mode-hook c++-mode-hook c-ts-mode-hook c++-ts-mode-hook
                fortran-mode-hook f90-mode-hook))
  (add-hook hook #'solomon-highlight-setup))
(provide 'solomon-highlight)
;;; solomon-highlight.el ends here
