;;; solomon-highlight.el --- highlight Solomon offloading macros
;; install: add (load "/path/to/solomon/misc/editor/emacs/solomon-highlight.el") to your init.el
(defun solomon-highlight-setup ()
  "Highlight Solomon offloading macros as preprocessor directives."
  (font-lock-add-keywords nil
    '(("\\_<\\(?:SOLOMON\\|PRAGMA_ACC\\|PRAGMA_OMP\\|ACC_CLAUSE\\|OMP_CLAUSE\\|OMP_TARGET\\)_[A-Z0-9_]+\\_>"
       . font-lock-preprocessor-face)
      ("\\_<OMP_PASS_LIST\\_>" . font-lock-preprocessor-face))))
(dolist (hook '(c-mode-hook c++-mode-hook c-ts-mode-hook c++-ts-mode-hook
                fortran-mode-hook f90-mode-hook))
  (add-hook hook #'solomon-highlight-setup))
(provide 'solomon-highlight)
;;; solomon-highlight.el ends here
