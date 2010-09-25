(list
  ;; basic function
  (apply (fn () true) nil)

  ;; single parameter
  (apply (fn (x) x) (list (* 6 7)))

  ;; multiple parameters
  (apply (fn (x y) (eq y x)) (list (+ 1 2) 42))

  ;; curried function demonstrating lexical closure
  (apply (apply (fn (x) (fn (y) (+ x y))) '(5)) '(4))

  ;; & before the last formal parameter signifies it should get the
  ;; rest of the arguments as a list
  (apply (fn (x & ys) (list x ys)) '(1 2 3))

  ;; nil should be an acceptable value for an argument
  (apply (fn (x) x) '(nil)))

RESULT

(true 42 nil 9 (1 (2 3)) nil)