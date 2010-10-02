(list
  (if true 1 2)
  (if nil  1 2)
  (if true 1)
  (if nil 1))

RESULT

(1 2 1 nil)