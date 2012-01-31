(define (caar obj) (car (car obj)))

(define (cadr obj) (car (cdr obj)))

(define (cdar obj) (cdr (car obj)))

(define (cddr obj) (cdr (cdr obj)))

(define (caaar obj) (car (car (car obj))))

(define (caadr obj) (car (car (cdr obj))))

(define (cadar obj) (car (cdr (car obj))))

(define (caddr obj) (car (cdr (cdr obj))))

(define (cdaar obj) (cdr (car (car obj))))

(define (cdadr obj) (cdr (car (cdr obj))))

(define (cddar obj) (cdr (cdr (car obj))))

(define (cdddr obj) (cdr (cdr (cdr obj))))

(define (caaaar obj) (car (car (car (car obj)))))

(define (caaadr obj) (car (car (car (cdr obj)))))

(define (caadar obj) (car (car (cdr (car obj)))))

(define (caaddr obj) (car (car (cdr (cdr obj)))))

(define (cadaar obj) (car (cdr (car (car obj)))))

(define (cadadr obj) (car (cdr (car (cdr obj)))))

(define (caddar obj) (car (cdr (cdr (car obj)))))

(define (cadddr obj) (car (cdr (cdr (cdr obj)))))

(define (cdaaar obj) (cdr (car (car (car obj)))))

(define (cdaadr obj) (cdr (car (car (cdr obj)))))

(define (cdadar obj) (cdr (car (cdr (car obj)))))

(define (cdaddr obj) (cdr (car (cdr (cdr obj)))))

(define (cddaar obj) (cdr (cdr (car (car obj)))))

(define (cddadr obj) (cdr (cdr (car (cdr obj)))))

(define (cdddar obj) (cdr (cdr (cdr (car obj)))))

(define (cddddr obj) (cdr (cdr (cdr (cdr obj)))))

(define (equal? obj1 obj2)
  (if (and (pair? obj1) (pair? obj2))
    (and (equal? (car obj1) (car obj2))
         (equal? (cdr obj1) (cdr obj2)))
    (eqv? obj1 obj2)))

(define (zero? z)
  (= z 0))

(define (positive? x)
  (>= x 0))

(define (negative? x)
  (< x 0))

(define (odd? n)
  (= (modulo n 2) 1))

(define (even? n)
  (= (modulo n 2) 0))

(define (max x y . zz)
  (let ((x (if (> x y) x y)))
    (if (null? zz)
      x
      (max x (car lst) (cdr lst)))))

(define (min x y . lst)
  (let ((x (if (< x y) x y)))
    (if (null? lst)
      x
      (min x (car lst) (cdr lst)))))

(define (abs x)
  (if (< x 0) (- x) x))

;(define (gcd . nn)
;  (if (null? nn)
;    0
;    ...))

;(define (lcm . nn)
;  (if (null? nn)
;    1
;    ...))

;(define (rationalize x y) ...)

(define (not obj)
  (if obj #f #t))

(define (boolean? obj)
  (or (eq? obj #f) (eq? obj #t)))

(define (null? obj)
  (eq? obj '()))

(define (list? obj)
  (if (null? obj) #t (and (pair? obj) (list? (cdr obj)))))

(define (list . objs)
  objs)

(define (length lst)
  (assert (list? lst)) ; TODO: proper error checking
  (if (null? lst) 0 (if (null? (cdr lst)) 1 (+ 1 (length (cdr lst))))))

;(define (append . lst) ...)

;(define (reverse lst)
;  (assert (list? lst))
;  (if (null? lst) lst
;    (if (null? (cdr lst)) lst
;      (cons (car (reverse (cdr lst))) (cons (car lst) '())))))

(define (list-tail lst k)
  (if (zero? k) lst (list-tail (cdr lst) (- k 1))))

(define (list-ref lst k)
  (car (list-tail lst k)))

(define (memq obj lst)
  (assert (list? lst))
  (if (null? lst)
    #f
    (if (eq? obj (car lst))
      lst
      (memq obj (cdr lst)))))

(define (memv obj lst)
  (assert (list? lst))
  (if (null? lst) 
    #f 
    (if (eqv? obj (car lst)) 
      lst 
      (memv obj (cdr lst)))))

(define (member obj lst)
  (assert (list? lst))
  (if (null? lst) 
    #f 
    (if (equal? obj (car lst)) 
      lst 
      (member obj (cdr lst)))))

(define (assq obj lst)
  (if (null? lst) #f
    (if (pair? (car lst))
      (if (eq? obj (caar lst))
        (car lst)
        (assq obj (cdr lst)))
      (error 'assq "alist must be list of pairs"))))

(define (assv obj lst)
  (if (null? lst) #f
    (if (pair? (car lst))
      (if (eqv? obj (caar lst))
        (car lst)
        (assv obj (cdr lst)))
      (error 'assv "alist must be list of pairs"))))

(define (assoc obj lst)
  (if (null? lst) #f
    (if (pair? (car lst))
      (if (equal? obj (caar lst))
        (car lst)
        (assoc obj (cdr lst)))
      (error 'assoc "alist must be list of pairs"))))

;(define (string . chars) ...)

(define (vector . objs)
  (list->vector objs));

(define (vector->list vec)
  (define (iter k)
    (if (= k (vector-length vec))
      '()
      (cons (vector-ref vec k) (iter (+ k 1)))))
  (iter 0))

(define (list->vector lst)
  (let ((vec (make-vector (length lst))))
    (define (iter lst k)
      (if (< k (vector-length vec))
        (begin (vector-set! vec k (car lst))
               (iter (cdr lst) (+ k 1)))
        vec))
    (iter lst 0)))

(define (vector-fill! vec fill)
  (let ((len (vector-length vec)))
    (define (iter k)
      (if (< k len)
        (begin (vector-set! vec k fill)
               (iter (+ k 1)))
        vec))
    (iter 0)))

(define (map proc . lists)
  (define (cars lists)
    (if (null? lists)
      lists
      (cons (caar lists) (cars (cdr lists)))))
  (define (cdrs lists)
    (if (null? lists)
      lists
      (cons (cdar lists) (cdrs (cdr lists)))))
  (if (or (null? lists) (null? (car lists)))
    '()
    (cons (apply proc (cars lists))
          (apply map (cons proc (cdrs lists))))))

(display (let ((count 0))
  (map (lambda (ignored)
         (set! count (+ count 1))
         count)
       '(a b))))

;(define (for-each proc . lists) lists)

;(let ((v (make-vector 5)))
;  (for-each (lambda (i)
;              (vector-set! v i (* i i)))
;            '(0 1 2 3 4))
;  v)

;(define (force promise) ...)
