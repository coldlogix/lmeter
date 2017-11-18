;
;	This is file to collect various stuff usefull for 
;	AutoLISP programming - some "#defines", common used functions etc.
;
;					Created	04/21/93 by Paul Bunyk
;
;	Following are "#defines" to access different properties of ACAD 
;	entities by symbolic names.
;
;       Common for different entities:
(setq   ENTTYPE         0)
(setq   LAYERNAME       8)
(setq   LINETYPE        6)   
(setq   POINT_1         10)
(setq   POINT_2         11)
(setq   ELEVATION       38)
(setq   THICKNESS       39)
(setq   COLOR           62)
(setq   FLAGS           70)
(setq   OWNNAME         -1)
;       Fields for PLINE/VERTEX
(setq   STARTWIDTH      40)
(setq   ENDWIDTH        41)
;       Fields for INSERT
(setq   BLOCKNAME       2)
(setq   SX              41)
(setq   SY              42)
(setq   SZ              43)
(setq   ROTANGLE        50)
(setq   NCOLOMNS        70)
(setq   NROWS           71)
(setq   COLSTEP         44)
(setq   ROWSTEP         45)
(setq   NAMEFROMTBL     -2)

(defun  GetA    (Field EntList)	(cdr    (assoc  Field   EntList)))
(defun  GetAD   (Field EntList default / res ) 
	(if	(setq	res	(cdr    (assoc  Field   EntList)))
		res
		default
	)
)
(defun	Sign	(q)
	(cond	((<	q	0.0)	-1)
		((>	q	0.0)	+1)
		(T			 0)
	)
)
;	Snap must be defined elsewhere.
(defun	ToSnap	(q)	
	(*	(fix 	(+	(/	q	Snap)
				(* 	0.5 	(Sign	q))
			)
		)
		Snap
	)
)
(defun	*error*	(S)	(princ	S)	(terpri)	(quit)	)
;	Uncomment while debugging:
;	(setq	*error*	nil)

;	TWO FUNCTIONS TO WORK WITH SELECTION SETS
;	CONVERT SEL. SET TO LISTOF NAMES, SEL. SET IS DELETED
(defun	ss2list	(ss / l name)
	(repeat	(sslength ss)
		(setq	name	(ssname ss 0))
		(setq 	l	(cons	name	l))
		(ssdel	name	ss)
	)
	l
)
;	APPLY ANY FUNCTION f TO ALL ELEMENTS OF SEL. SET
(defun	ssmapcar (f ss)	(mapcar f (ss2list ss)))

