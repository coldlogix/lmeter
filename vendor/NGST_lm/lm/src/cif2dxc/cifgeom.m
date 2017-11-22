A=	{	{	a00,	a01,	a02	},
		{	a10,	a11,	a12	},
		{	a20,	a21,	a22	}
	}
T=	{	{	1,	0,	0	},
		{	0,	1,	0	},
		{	a,	b,	1	}
	}
MX=	{	{	-1,	0,	0	},
		{	0,	1,	0	},
		{	0,	0,	1	}
	}
MY=	{	{	1,	0,	0	},
		{	0,	-1,	0	},
		{	0,	0,	1	}
	}

R=	{	{	a/d,	b/d,	0	},
		{	-b/d,	a/d,	0	},
		{	0,	0,	1	}
	}

outTeX[s_,e_]:=	(	TextForm[s]>>>"out.tex";
			TeXForm[MatrixForm[e]]>>>"out.tex"
		)

(*
outTeX["A.T-A=",Simplify[A.T-A]]
outTeX["A.MX-A=",Simplify[A.MX-A]]
outTeX["A.MY-A=",Simplify[A.MY-A]]
outTeX["A.R-A=",Simplify[A.R-A]]
*)