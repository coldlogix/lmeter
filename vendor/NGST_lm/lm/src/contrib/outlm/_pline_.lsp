;
;	This is for OUTLM to handle thin & wide POLYLINEs.
;
;	Special THANKS to Dima Zinoviev - without his geometrical ideas 
;	it would be impossible to write the wide polylines support.
;			
;				Created	04/21/93 by Paul Bunyk
;
(setq	*workers*
	(cons	(cons    "POLYLINE"      'HereIsPolyline)
		*workers*
	)
)	
(defun  HereIsPolyline     (ent)
	(setq	Lay	(GetA LAYERNAME ent))
	(setq	Param	(GetAD THICKNESS ent 0))
	(if	(/=     (GetA   STARTWIDTH      ent) 0.0)
		(HereIsWidePline ent)
		(HereIsThinPline ent)
	)
)
(defun	HereIsWidePline	(	ent / 	pntlist points SWs EWs ups lows 
					i a b ax ay bx by sw ew d vx vy
			)
	(if  	(/=     (logand	(GetA   FLAGS   ent)	1)	0)
		(*error*	"Wide PLINE cann't be CLOSED")
	)
	(setq	Lay	(GetA	LAYERNAME	ent))
	(setq	Ite	(GetA	THICKNESS	ent))
	;	Get all VERTEXes with their Starting and Ending widthes
	(setq	points	nil)
	(setq	SWs	nil)
	(setq	EWs	nil)
	(setq	ename	(entnext ename))
	(setq   ent	(entget	ename))
	(while  (/=     (GetA   ENTTYPE ent)    "SEQEND")
		(setq	points	(cons	(GetA	POINT_1		ent)	points))
		(setq	SWs	(cons	(/	(GetA	STARTWIDTH	ent)	2)	SWs))
		(setq	EWs	(cons	(/	(GetA	ENDWIDTH	ent)	2)	EWs))
		(setq	ename	(entnext ename))
		(setq   ent	(entget	ename))
	)
	(setq	points	(reverse	points))
	(setq	SWs	(reverse	SWs))
	(setq	EWs	(reverse	EWs))
	;	Build two pointlists - for upper and lower boundaries 
	(setq	ups	nil)
	(setq	lows	nil)
	(setq	i	0)
	(repeat	(1-	(length	points))
		(setq	a	(nth	i	points))
		(setq	sw	(nth	i	SWs))
		(setq	ew	(nth	i	EWs))
		(setq	i	(1+	i))
		(setq	b	(nth	i	points))
		(setq	ax	(car	a)
			ay	(cadr	a)
			bx	(car	b)
			by	(cadr	b)
		)
		;	d=|ab|
		(setq	d	(distance	(list ax ay) (list bx by)))
		;	v is a unit vector orthogonal to ab
		(setq	vx	(/	(-	ay	by)	d))
		(setq	vy	(/	(-	bx	ax)	d))
		(setq	ups	
			(append	ups
				(list	(list	(list	(+ ax (* sw vx))
							(+ ay (* sw vy))
						)
						(list	(+ bx (* ew vx))
							(+ by (* ew vy))
						)
					)
				)
			)
		)
		(setq	lows	
			(append	lows
				(list	(list	(list	(+ ax (- (* sw vx)))
							(+ ay (- (* sw vy)))
						)
						(list	(+ bx (- (* ew vx)))
							(+ by (- (* ew vy)))
						)
					)	
				)
				
			)
		)
	)
	(Output	(append	(Intersect	ups)
			(reverse	(Intersect	lows))
			(list	(car	(car	ups)))
		)
	)
)
(defun	HereIsThinPline	(ent / pnt1 isclosed pntlist)
	(setq   isclosed        (/=     (logand (GetA   FLAGS   ent) 1)0))
	;	Get next VERTEX
	(setq	ename	(entnext ename))	
	(setq   ent	(entget	ename))
	(setq   pnt1    (GetA   POINT_1 ent))
	(while  (/=     (GetA   ENTTYPE ent)    "SEQEND")
		(setq   pntlist (cons   (GetA   POINT_1 ent) pntlist))
		(setq	ename	(entnext ename))	
		(setq   ent	(entget	ename))
	)
	(if     isclosed        (setq   pntlist (cons  pnt1 pntlist)))
	(Output pntlist)
)
(defun	Intersect	(plist / outplist i a b ip)
	(setq	i	1)
	(setq	b		(car	plist))
	(setq	outplist	(list	(car	b)))
	(repeat	(1-	(length	plist))
		(setq	a	b)
		(setq	b	(nth	i	plist))
		(setq	i	(1+	i))
		;	a & b are lines, try to intersect them
		(if	(setq	ip	
				(inters	(car a) (cadr a) (car b) (cadr b) nil)
			)
			(setq	outplist	
				(append	outplist	(list ip))
			)
			(setq	outplist
				(append	outplist	(list (cadr a) (car b)))	
			)
		)
	)
	(setq	outplist	(append	outplist	(list	(cadr b))))
	outplist
)