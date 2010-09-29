(list
  ;; basic math functions are all binary
  (+ 1 1) (- 6 2) (* 2 3) (/ 32 4)

  ;; eq tests reference equality
  (eq 'foo 'bar) (eq 'baz 'baz)

  ;; basic list functions
  (cons 1 2) (cons 1 '(2 3)) (car (cons 1 2)) (cdr (cons 1 2))

  ;; a symbol can have both a value and a function-value
  (do
    (set-value 'ho 2)
    (set-function 'ho (function '*))
    (ho ho ho))

  ;; Sup, dawg? I heard you like functions...
  (apply (fn (x) x) '(:foo))
  (apply (function (fn (y) y)) '(:bar))
  (apply (function '+) '(1 2))

  ;; macros for defining things, such as macros and functions
  (do
    (defmacro foo (x y) (bar x y))
    (defn bar (x y) (+ x y))
    (foo 6 7)))

RESULT

(2 4 6 8 nil true (1 . 2) (1 2 3) 1 2 4 :foo :bar 3 13)