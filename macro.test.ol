(set-function 'alpha (macro! (fn (x y) (list 'beta x y))))
(set-function 'beta (macro! (fn (x y) (list 'gamma x y))))
(set-function 'gamma (macro! (fn (x y) (list '* x y))))

(list
 (alpha 6 7)
 (macroexpand '(alpha 1 2))
 (macroexpand1 '(gamma 1 2))
 (macroexpand1 '(alpha 1 2)))

RESULT

(42 (* 1 2) (* 1 2) (beta 1 2))
