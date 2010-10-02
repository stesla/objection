'(42 "foo" bar 'baz (+ +1 -2 3 0xf 010) :keyword)

RESULT

(42 "foo" bar (quote baz) (+ 1 -2 3 15 8) :keyword)