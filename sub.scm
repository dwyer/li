; What follows are a list of features that are described in R5RS as ``library
; procedures'' meaning they are redundant and can be implemented with primitive
; procedures. Some library procedures that I consider essential have been 
; implemented as primitives. 

; Works similar to assert. I'm thinking about throwing out type-checking for
; arithmatic procedures and letting the passing of non-numbers to them be
; unspecified. It could lead to some interesting hacking but I haven't thought
; too much about the security implications.
(define (check condition who msg . args)
  (if (not condition)
    (apply error (cons who (cons msg args)))))

(define (zero? z)
  (check (number? z) 'zero? "not a number" z)
  (= z 0))

(define (positive? x)
  (check (number? x) 'positive? "not a number" x)
  (>= x 0))

(define (negative? x)
  (check (number? x) 'negative? "not a number" x)
  (< x 0))

(define (odd? n)
  (check (integer? n) 'odd? "not a number" n)
  (not (= (remainder n 2) 0)))

(define (even? n)
  (check (integer? n) 'even? "not a number" n)
  (= (remainder n 2) 0))

(define (max x . args)
  (if (null? args)
    x
    (let ((y (car args)))
      (if (and (number? x) (number? y))
        (apply max (cons (if (> x y) x y) (cdr args)))
        (error 'max "arguments must be real numbers" x args)))))

(define (min x . args)
  (if (null? args)
    x
    (let ((y (car args)))
      (if (and (number? x) (number? y))
        (apply min (cons (if (< x y) x y) (cdr args)))
        (error 'min "arguments must be real numbers" x args)))))

(define (abs x)
  (check (number? x) 'abs "not a number" x)
  (if (< x 0)
    (- x)
    x))

(define (gcd . args)
  (define (euclid a b)
    (check (integer? a) 'gcd "not an integer" a)
    (check (integer? b) 'gcd "not an integer" b)
    (if (= b 0)
      a
      (euclid b (remainder a b))))
  (cond ((null? args) 0)
        ((null? (cdr args)) (abs (car args)))
        (else
          (apply gcd (cons (euclid (car args) (cadr args))
                           (cddr args))))))

(define (lcm . args)
  (define (algorithm a b)
    (/ (abs (* a b)) (gcd a b)))
  (cond ((null? args) 1)
        ((null? (cdr args)) (abs (car args)))
        (else
          (apply lcm (cons (algorithm (car args) (cadr args))
                           (cddr args))))))

(define (reverse lst)
  (cond ((null? lst) '())
        ((not (pair? lst)) (error 'reverse "arg must be a list" lst))
        ((null? (cdr lst)) lst)
        (else (append (reverse (cdr lst)) (list (car lst))))))

; Taken from R5RS
(define (list-tail lst k)
  (if (= k 0)
    lst
    (list-tail (cdr lst) (- k 1))))

(define (list-ref lst k)
  (car (list-tail lst k)))

(define (__member name pred obj lst)
  (define (iter next)
    (cond ((null? next) #f)
          ((not (pair? next))
           (error name "arg2 must be a list" lst))
          ((pred obj (car next)) next)
          (else (iter (cdr next)))))
  (iter lst))

(define (memq obj lst)
  (__member 'memq eq? obj lst))

(define (memv obj lst)
  (__member 'memv eqv? obj lst))

(define (member obj lst)
  (__member 'member equal? obj lst))

(define (__assoc name pred obj lst)
  (define (iter next)
    (cond ((null? next) #f)
          ((not (and (pair? next) (pair? (car next))))
           (error name "arg2 must be a list of pairs" lst))
          ((pred obj (caar next)) (car next))
          (else (iter (cdr next)))))
  (iter lst))

(define (assq obj lst)
  (__assoc 'assq eq? obj lst))

(define (assv obj lst)
  (__assoc 'assv eqv? obj lst))

(define (assoc obj lst)
  (__assoc 'assoc equal? obj lst))

(define (make-vector k . args)
  (if (and (not (null? args)) (not (null? (cdr args))))
    (error 'make-vector "too many arguments"))
  (if (or (not (integer? k)) (< k 0))
    (error 'make-vector "arg1 must be a positive integer" k))
  (define (make-list k fill)
    (if (= k 0)
      '()
      (cons fill (make-list (- k 1) fill))))
  (list->vector (make-list k (if (null? args) '() (car args)))))

(define (vector->list vec)
  (if (not (vector? vec))
    (error 'vector->list "arg must be a vector" vec))
  (define (iter k)
    (if (= k (vector-length vec))
      '()
      (cons (vector-ref vec k) (iter (+ k 1)))))
  (iter 0))

(define (list->vector lst)
  (if (not (list? lst))
    (error 'list->vector "arg must be a list" lst))
  (apply vector lst))

(define (vector-fill! vec fill)
  (if (not (vector? vec))
    (error 'vector-fill! "arg1 must be a vector" vec))
  (let ((len (vector-length vec)))
    (define (iter k)
      (if (< k len)
        (begin (vector-set! vec k fill)
               (iter (+ k 1)))
        vec))
    (iter 0)))

(define (map proc . args)
  (if (not (procedure? proc))
    (error 'map "arg1 must be a procedure" proc))
  (define (cars args)
    (if (null? args)
      args
      (cons (caar args) (cars (cdr args)))))
  (define (cdrs args)
    (if (null? args)
      args
      (cons (cdar args) (cdrs (cdr args)))))
  (if (or (null? args) (null? (car args)))
    '()
    (cons (apply proc (cars args))
          (apply map (cons proc (cdrs args))))))

; Is there a difference?
(define for-each map)
