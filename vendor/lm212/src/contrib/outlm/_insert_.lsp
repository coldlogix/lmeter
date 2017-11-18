;
;	This is for OUTLM to handle INSERTs & MINSERTs 
;			
;				Created	04/21/93 by Paul Bunyk
;
(setq	*workers*	
	(cons	(cons    "INSERT"      'HereIsInsert)
		*workers*
	)
)	
(load	"_SKIP_")
(defun	HereIsInsert	(	ent / 	blkname name blkename skip
					inspnt inssx inssy insrot 
					insnr insnc inscs insrs x y mx my
			)
	;	Probably I must skip this block?
	(setq	noskip	T)
	(setq		blkname	(GetA BLOCKNAME ent))
	(foreach	name	SkippedNames
		(if	(=	name	blkname)
			(progn	(princ	"Skipped block: ")
				(princ	name)
				(terpri)
				(setq	noskip	nil)
			)
		)
	)
	(if	noskip
		(progn
			;	Save the insertion info
			(setq   blkename
				(GetA NAMEFROMTBL (tblsearch "BLOCK" blkname ))
			)
			(setq	inspnt	(GetA	POINT_1		ent)
				inssx	(GetA	SX		ent)
				inssy	(GetA	SY		ent)
				insrot	(GetA	ROTANGLE	ent)
				insnr	(GetA	NROWS		ent)
				insnc	(GetA	NCOLOMNS	ent)
				insrs	(GetA	ROWSTEP		ent)
				inscs	(GetA	COLSTEP		ent)
			)
			(setq	x	(car	inspnt))
			(setq	y	(cadr	inspnt))
			;	Change nr, nc: the are 0 for INSERT.
			(if	(=	insnr	0)	(setq	insnr	1))
			(if	(=	insnc	0)	(setq	insnc	1))
			;       Push ename & GTM
			(setq   NameStack       (cons ename     NameStack))
			(setq   GTMStack      	(cons GTM     	GTMStack))
			(setq	mx	0)
			(repeat	insnc
				(setq	my	0)
				(repeat	insnr
					(setq   ename   blkename)
					(setq	GTM	(car	GTMStack))
					(setq	GTM	(BuildGTM x y inssx inssy insrot mx my))
					(while	ename
						(setq   ent	(entget	ename))
						(setq   worker  (assoc  (GetA ENTTYPE   ent)    *workers*))
						;	(princ	(GetA ENTTYPE   ent))	(terpri)
						(if     worker  
							(apply  (cdr   worker) (list    ent))
							(NoWorker	ent)
						)
						(setq	ename	(entnext ename))
					)
					(setq	my	(+	my	insrs))
				)
				(setq	mx	(+	mx	inscs))
			)
			;	Pop ename & GTM
			(setq   ename           (car    NameStack))
			(setq   NameStack       (cdr    NameStack))
			(setq   GTM           	(car    GTMStack))
			(setq   GTMStack	(cdr    GTMStack))
		)
	)
)
;	Multiply GTM by matrix of scaling, then minsert movement, 
;	then rotation, then movement - it IS thi right sequence 
;	(as far as I know).
(defun	BuildGTM	(dx dy sx sy fi mx my)
	(3x3*	(list	(list	(* sx (Cos fi))		(* sx (Sin fi))	0)
			(list	(-(* sy (Sin fi)))	(* sy (Cos fi))	0)
			(list	(+ dx (* mx (Cos fi))
				   (- (* my (Sin fi)))
				)
				(+ dy (* my (Cos fi))
				      (* mx (Sin fi))
				)
				1
			)
		)
		GTM
	)
)
;	Return the product of two 3x3 matrixes.
(defun	3x3*	(a b /	a11 a12 a13 a21 a22 a23 a31 a32 a33
			b11 b12 b13 b21 b22 b23 b31 b32 b33 row
		)
	(setq	row	(car	a))
	(setq	a11	(car	row)
		a12	(cadr	row)
		a13	(caddr	row)
	)
	(setq	row	(cadr	a))
	(setq	a21	(car	row)	
		a22	(cadr	row)	
		a23	(caddr	row)
	)	
	(setq	row	(caddr	a))
	(setq	a31	(car	row)
		a32	(cadr	row)	
		a33	(caddr	row)
	)	
	(setq	row	(car	b))
	(setq	b11	(car	row)
		b12	(cadr	row)	
		b13	(caddr	row)
	)	
	(setq	row	(cadr	b))
	(setq	b21	(car	row)	
		b22	(cadr	row)
		b23	(caddr	row)
	)	
	(setq	row	(caddr	b))
	(setq	b31	(car	row)
		b32	(cadr	row)	
		b33	(caddr	row)
	)	
	(list	(list	(+	(* a11 b11)(* a12 b21)(* a13 b31))
			(+	(* a11 b12)(* a12 b22)(* a13 b32))
			(+	(* a11 b13)(* a12 b23)(* a13 b33))
		)
		(list	(+	(* a21 b11)(* a22 b21)(* a23 b31))
			(+	(* a21 b12)(* a22 b22)(* a23 b32))
			(+	(* a21 b13)(* a22 b23)(* a23 b33))
		)
		(list	(+	(* a31 b11)(* a32 b21)(* a33 b31))
			(+	(* a31 b12)(* a32 b22)(* a33 b32))
			(+	(* a31 b13)(* a32 b23)(* a33 b33))
		)
	)
)
