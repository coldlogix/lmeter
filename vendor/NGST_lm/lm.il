;;; -*-lisp-*-

;;; ****************************************************************
;;;
;;; Cadence to LMeter interface 
;;; Paul.Bunyk@trw.com Oct. 2001
;;; 
;;; Partially based on code by A.Rylyakov and S.Polonsky,
;;; SUNY at Stony Brook RSFQ Laboratory 1995-1998
;;;
;;; ****************************************************************

(setq  lmTerminalLayer "term")  
(setq  lmTextLayer "text")
(setq  lmDefaultDir "/home/pbunyk/lm")

(info "Loading lm.il\n")
(info 
  (strcat "\n*********************************************************\n"
	  "*\tSUNY at Stony Brook RSFQ Laboratory\n"
	  "*\tLayout extraction for L-Meter\n"    
	  "*\tCopyright (c) 1995- by A.Rylyakov, S.Polonsky and P.Bunyk\n"
	  "*\n"
	  "*\tModified by P.Bunyk @ TRW SCO, Oct. 2001\n"
	  "*********************************************************\n\n"))

;;;  create  a pulldown menu to run LMeter commands
(setq LMmenu
       (hiCreatePulldownMenu
	'LMmenu
	"LMeter" 
	(list
	 (hiCreateMenuItem
	  ?name 'initializeItem
	  ?itemText "Initialize LMeter"
	  ?callback "lmInit()")
	 (hiCreateMenuItem
	  ?name 'lmeterItem
	  ?itemText "Export to LMeter"
	  ?callback "lmExport()")
	 (hiCreateMenuItem
	  ?name 'lmeterRunItem
	  ?itemText "Run LMeter"
	  ?callback "lmProcStart()")
	 (hiCreateMenuItem
	  ?name 'lmterminalItem
	  ?itemText "Draw Terminal"
	  ?callback "lmDrawTerminal()")
	 (hiCreateMenuItem
	  ?name 'lmJJterminalsItem
	  ?itemText "Draw JJ Terminals"
	  ?callback "lmMakeJJTerminals()")
	 (hiCreateMenuItem
	  ?name 'lmBackannotateItem
	  ?itemText "Update Inductance values"
	  ?callback "lmBackannotate()")
	 )))

;;;  add LMeter menu to current window
(setq lmSavedMenuTrigger nil)
(defun  lmAddLMeterMenu (args)
  (if lmSavedMenuTrigger 
      (apply lmSavedMenuTrigger (list args)))
  winid = hiGetCurrentWindow()
  hiInsertBannerMenu(winid 'LMmenu 12)
  (if (null (boundp 'lmWorkingDir))
      (progn hiDisableMenuItem( LMmenu 'lmeterItem)
	     hiDisableMenuItem( LMmenu 'lmeterRunItem)))
  (if (equal geGetEditCellView()->viewName "schematic")
      (progn 
	hiDisableMenuItem( LMmenu 'lmterminalItem winid)
	hiDisableMenuItem( LMmenu 'lmJJterminalsItem winid)
	)))				

; register LMeterMenu:
(if deGetAppInfo("maskLayout")->userPostInstallTrigger != 'lmAddLMeterMenu
	       deRegUserTriggers("maskLayout" nil nil 'lmAddLMeterMenu))

(if deGetAppInfo("schematic")->userPostInstallTrigger != 'lmAddLMeterMenu
    ;; Can not do that twice:
    ;; deRegUserTriggers("schematic" nil nil 'lmAddLMeterMenu)
    (let (app menu post)
      (setq app deGetAppInfo("schematic")->userAppTrigger)
      (setq menu deGetAppInfo("schematic")->userMenuTrigger)
      (setq lmSavedMenuTrigger 
	    deGetAppInfo("schematic")->userPostInstallTrigger)
      (deUnRegUserTriggers "schematic")
      (deRegUserTriggers "schematic" app menu 'lmAddLMeterMenu)
      ))


;;; oklmInitCB - procedure that is called when the OK or Apply
;;; button on the lmWorkingDirForm is pressed
(procedure  oklmInitCB(form "r")
  lmWorkingDir = form->lmWorkingDirField->value
  (cond ( ! isDir(lmWorkingDir) 
	    (hiDisplayAppDBox
	     ?name 'lmWorkingDirNotFoundBox
	     ?dboxText
	     sprintf(result "%s directory not found; create it?" lmWorkingDir)
	     ?callback "createDir(lmWorkingDir)"
	     )))
  (cond ( ! isDir(lmWorkingDir)
	printf("%s not created!\n" lmWorkingDir)
	printf("check if the parent directory exists\n")
	))
  (cond ( (isDir lmWorkingDir)
	  (progn
	    (hiEnableMenuItem  LMmenu 'lmeterItem  ) ; hiGetCurrentWindow())
	    (hiEnableMenuItem  LMmenu 'lmeterRunItem ) ; hiGetCurrentWindow())
	    (lmPrepareDirectory lmWorkingDir)
	    ))))

;;;  lmInit is the procedure which creates lmWorkingDirForm 
(defun lmInit ()
  (setq lmHome       (getShellEnvVar "HOME"))	
  (setq lmWorkingDir (getWorkingDir))
  (cond ; here we change "~" to full path
   (lmWorkingDir == "~"
		 lmWorkingDir = lmHome)
   (t 
    lmWorkingDir = strcat(lmHome substring(lmWorkingDir 2 strlen(lmWorkingDir)-1)) ))
  cellid = geGetEditCellView()
  thisLibName = cellid->libName
  thisCellName = cellid->cellName
  (setq lmDefWorkingDir (sprintf nil "%s/%s.lm" 
				    lmWorkingDir 
				    thisCellName))
  (setq lmWorkingDirForm 
	(hiCreateAppForm
	 ?name 'lmWorkingDirForm
	 ?formTitle "L-Meter Initialization Form"
	 ?callback list('oklmInitCB)
	 ?fields (list
		  (list
		   (hiCreateStringField
		    ?name 'lmWorkingDirField
		    ?prompt "L-Meter Working Directory:"
		    ?value  lmDefWorkingDir
		    ?defValue lmDefWorkingDir
		    )
		   10:10  750:30  200
		   )
		  )
	 ?help ""
	 ?unmapAfterCB t
	 ?initialSize  800:250	))
  hiDisplayForm(lmWorkingDirForm)
  hiMoveInsBarToEnd(lmWorkingDirForm 'lmWorkingDirField)
  )					; ** procedure lmInit **

(defun lmMatchHead (s1 s2)
  (equal s1 (substring s2 1 (strlen s1))))

(defun lmExport ()
  thisViewName = geGetEditCellView()->viewName
  (cond 
   ( (lmMatchHead "sch" thisViewName)
     (lm2SchNetlist (strcat lmWorkingDir "/netlist.spi")))
   ( (lmMatchHead "lay" thisViewName)
     (lmLayoutExtract lmWorkingDir t))
   ( (lmMatchHead "lmeter" thisViewName)
     (lmLayoutExtract lmWorkingDir nil))
   ( t
     (error (strcat "LMeter does not know about cellview " thisViewName))) 
  ))

;;; ****************************************************************
;;;  
;;;  Terminal drawing on layout
;;;
;;; ****************************************************************

hiSetBindKey("Layout" "Alt<Key>t" "lmDrawTerminal()")

(setq lmUPValue "*")
(setq lmUPDefaultValue "*")
(setq lmDOWNValue "*")
(setq lmDOWNDefaultValue "*")
(setq lmlabelValue "P")
(setq lmlabelDefaultValue "P")
(setq lmLabelPosValue "left")
(setq lmLabelPosDefaultValue "right")

(setq lmTerminalLabelFont "roman")
(setq lmTerminalLabelSize 2)

;... lmDrawTerminal ........... Oct  7 94 sasha 11 

(procedure  lmDrawTerminal()

 cellid = geGetEditCellView()
 cancelEnterFun()	       
 geDeselectAllFig(cellid) 

 x1y1x2y2 = enterBox(
     ?prompts list("Point at the first corner of the terminal"
                   "Point at the opposite corner of the terminal" )
     ?alwaysMap t   )

 x1y1 = car(x1y1x2y2)      x1 = car(x1y1) y1 = car(cdr(x1y1))
 x2y2 = car(cdr(x1y1x2y2)) x2 = car(x2y2) y2 = car(cdr(x2y2))
 xtc  = (x1 + x2)/2      ytc=(y1+y2)/2  ; center of terminal
 thwx = abs(x1-x2)/2     thwy = abs(y1-y2)/2; terminal halfwidths 
 isAreaZerop = (x1 == x2 || y1 == y2)
 (cond
    ( isAreaZerop
    lmLine     = dbCreateLine(cellid lmTerminalLayer list(x1:y1 x2:y2))
    lmTerminal = lmLine )
    ( ! isAreaZerop
    lmRect =     dbCreateRect(cellid lmTerminalLayer list(x1:y1 x2:y2))
    lmTerminal = lmRect )	  
 ) ; ** cond **
  
 lmTerminalData()
) ; ** procedure  **

 xshift = 3.5    yshift = 3.5
;... oklmTerminalDataCB ....... Oct  7 94 sasha 11 

(procedure  oklmTerminalDataCB(form "r")
 dbCreateProp(lmTerminal "lmUP" "string" lmUPValue = form ->lmUPField->value )
 dbCreateProp(lmTerminal "lmDOWN" "string"
			lmDOWNValue = form->lmDOWNField->value )
 dbCreateProp(lmTerminal "label" "string"
			lmlabelValue = form->lmlabelField->value )
 lmLabelPosValue = form -> lmLabelPosField -> value
 (cond
    (lmLabelPosValue == "right"  labelcoords = list(xtc+thwx+xshift ytc))
    (lmLabelPosValue == "left"   labelcoords = list(xtc-thwx-xshift ytc))
    (lmLabelPosValue == "top"    labelcoords = list(xtc ytc+thwy+yshift))
    (lmLabelPosValue == "bottom" labelcoords = list(xtc ytc-thwy-yshift))
 );cond
 propid = car(setof(dummy lmTerminal~>prop dummy~>name == "label"))
 dbCreateTextDisplay(propid lmTerminal lmTextLayer t
  labelcoords "centerCenter" "R0"
  lmTerminalLabelFont lmTerminalLabelSize t nil t nil)
  
) ; ** procedure  **

(procedure  cancelTerminalDataCB(form "r")
 dbDeleteObject(lmTerminal)
	 )
			       
(defun  lmTerminalData ()

  xUP    = 10    yUP     = 10
  xDOWN  = 200   yDOWN   = 10
  xlabel = 10    ylabel  = 50
  xlpos  = 10    ylpos   = 100  
  width  = 81    height  = 20    promptWidth  = 80
  width2 = 300   height2 = 30    promptWidth2 = 70
  (setq lmTerminalDataForm 
	(hiCreateAppForm
	 ?name 'lmTerminalDataForm
	 ?formTitle "Terminal Properties Form"
	 ?callback (list 'oklmTerminalDataCB  'cancelTerminalDataCB )
	 ?fields (list
		  (list	 
		   (hiCreateCyclicField
		    ?name  'lmUPField
		    ?choices (list "*" 
				   "wirb" "wira" "triw" "gnde" "m1" "m2" "m3")
		    ?prompt "lmUP"
		    ?value  lmUPValue
		    ?defValue lmUPDefaultValue
		    )
		   xUP:yUP width:height promptWidth
		   )
		  (list	 		       
		   (hiCreateCyclicField
		    ?name  'lmDOWNField
		    ?choices (list "*" 
				   "gnde" "triw" "wira" "wirb" "m0" "m1" "m2")
		    ?prompt "lmDOWN"
		    ?value  lmDOWNValue
		    ?defValue lmDOWNDefaultValue
		    )
		   xDOWN:yDOWN width:height promptWidth
		   )
		  (list	 		       		       
		   (hiCreateStringField
		    ?name 'lmlabelField
		    ?prompt "label"
		    ?value  lmlabelValue
		    ?defValue lmlabelDefaultValue
		    )
		   xlabel:ylabel width2:height2 promptWidth2
		   )
		  (list	 		       
		   (hiCreateCyclicField
		    ?name  'lmLabelPosField
		    ?choices (list "right" "left" "top" "bottom")
		    ?prompt "Put label on"
		    ?value  lmLabelPosValue
		    ?defValue lmLabelPosDefaultValue
		    )
		   xlpos:ylpos width:height promptWidth
		   )
		  )
	 ?help ""
	 ?unmapAfterCB t
	 ))
  hiDisplayForm(lmTerminalDataForm)
  )					

;;; Make terminals around Josephson junction PCELLs.
;;; This is j230/j110 specific
(inScheme
 (defun lmMakeJJTerminals ()
   (letseq 
    ( (currCellView (geGetEditCellView))
      (instances currCellView->instances)
      (shapes currCellView->shapes)
      (jj230 (list "j230JJ_2"
		     "j230RSJ_2"))
      (jj110D (list "j110D_RSJ_v1"
		     "j110D_JJ_v1"))
      (jj110E (list "j110E_RSJ_v0"
		     "j110E_JJ_v0"))
      (jjCells (append jj230 (append jj110D jj110E)))
      )
    ;; Erase old terminals attached to JJs
    (foreach 
     s shapes
     (if (and (equal s->layerName "term")
	      s->parent
	      (member s->parent->cellName jjCells))
	 (dbDeleteObject s)))    
    ;; Add terminals for all JJ instances
    (foreach
	i instances
	(if (member i->cellName jjCells)
	    (letseq
	     ( (name i->name)
	       (origin i->xy)
	       (x0 (car origin))
	       (y0 (cadr origin))
	       (ic (lmPcellParamValue i "ic"))
	       (sioa_radius (cond
			     ((member i->cellName jj230)
			       (lmj230sioaRadius ic))
			     ((member i->cellName jj110E)
			       (lmj110EsioaRadius ic))
			     ((member i->cellName jj110D)
			       (lmj110DsioaRadius ic))
			     (T (error (strcat "Do not know how to calculate R for "  i->cellName)))))
	       (term (dbCreateRect currCellView "term" 
				   (list (list x0-sioa_radius y0-sioa_radius)
					 (list x0+sioa_radius y0+sioa_radius))))
	       (validJJname (member (substring name 1 1) (list "j" "J" "b" "B")))
	       (label (dbCreateLabel currCellView list("text" "drawing") 
				     (x0+sioa_radius:y0+sioa_radius) 
				     name "lowerLeft" "R0" "roman" 1.0))
	      )
	      (info (sprintf nil "Processing %s at %f,%f with Ic=%f, sioa radius=%f\n" 
			     name (car origin) (cadr origin) ic sioa_radius))
	      (if validJJname
		  (progn
		    (dbCreateProp term "lmUP" "string" "*")
		    (dbCreateProp term "lmDOWN" "string" "*")
		    (dbCreateProp term "label" "string" name)
		    ;; Erase and update auto-generated terminals if needed
		    ;; The terminal is attached as a child of JJ PCELL 
		    term->parent = i
		    label->parent = term
		    )
		(progn
		  (warn (sprintf nil "%s does not look like a JJ name, skipping\n" name))
		  (dbDeleteObject term)
		  (dbDeleteObject label)
		  ))
	      
	      ))))
   t))

;; modified PCELL parameters are properties, un-modified
;; are master's properties. Search these two places.
(defun lmPcellParamValue (cellid parname)
  (letseq
   ( (prop (dbSearchPropByName cellid parname)) 
     )
   (if prop
       prop->value
     (letseq
      ( (supermaster cellid->master->superMaster)
	(params  (dbSearchPropByName supermaster "parameters")->value)
	value)
      (foreach
       p params
       (if (equal p->name parname)
	   (setq value p->value)))
      value))))

;; code derived from the actual PCELLs to find (snapped to grid) 
;; SIOA radius given ic
(defun lmj230sioaRadius (ic)
  (letseq
   ( (pi 3.141592653589792)
     (grid 0.125)
     (j230Rule_H1  2.0)
     (j230Rule_H3d 1.5)
     (r sqrt((100*ic)/pi))
     )
   (round (cond ((greaterp r 4.0) r-j230Rule_H3d)
		((greaterp r 3.0) 2+0.5*(r-3))
		((greaterp r 1.75) 0.5*j230Rule_H1 + 0.8*(r-1.75))
		(t 0.5*j230Rule_H1))/grid)*grid))

(defun lmj110DsioaRadius (ic)
  (letseq
   ( (pi 3.141592653589792)
     (grid 0.250)
     (j110D_JC 4.0)
     (r sqrt((( (100.0/j110D_JC) * ic) / pi)))
     ;; JUNC (< 2.5 ?m) overlap of SIOA contact 
     ( j110DRule_G3a 0.375 )
     ;; JUNC (2.5 ?m to 4.0 ?m) overlap of SIOA contact
     ( j110DRule_G3b 0.5 )
     ;; JUNC (> 4.0 ?m) overlap of SIOA contact           
     ( j110DRule_G3c 1.0 )
     (j110DRule_G3
      (cond   ((greaterp r 2.0) j110DRule_G3c)
	      ((greaterp r 1.25) j110DRule_G3b)
	      (t j110DRule_G3a)))
     (sioaR (r-j110DRule_G3) )
     )
   (round sioaR/grid)*grid))

(defun lmj110EsioaRadius (ic)
  (letseq
   ( (pi 3.141592653589792)
     (grid 0.1)
     (j110E_JC 8.0)
     (r sqrt((( (100.0/j110E_JC) * ic) / pi)))

     (j110ERule_G3
      (cond	((r < 1.25) 0.1+ (r - 0.6)*0.4/0.65)
		((r < 2.0) 0.5 + (r-1.25)*2.0/3.0 )
		(else 1.0)))
     (sioaR (r-j110ERule_G3) )
     )
   (round sioaR/grid)*grid))


;;; ****************************************************************
;;;
;;; LMeter IPC
;;;
;;; ****************************************************************

;;; Prepare working directory for LMeter to run
(defun lmPrepareDirectory (lmWorkingDir)
  (lmWriteMakefile lmWorkingDir)
  (system (strcat "cp " lmTechFile " " lmWorkingDir "/lm.dfl")))

;;; write Makefile to run LMeter
(defun lmWriteMakefile (lmWorkingDir)
  (setq makefile (outfile (sprintf nil "%s/%s" lmWorkingDir "Makefile") "w"))
  (if (null (setq lmDir getShellEnvVar("LMETER"))) 
      (setq lmDir lmDefaultDir))
  (fprintf makefile
	   (strcat
	    "# Automatically created LMeter makefile\n"
	    "LMDIR = " lmDir "/bin/\n\n"
	    "all: lmeter.out netlist.spi term.name\n"
	    "\t$(LMDIR)lm2sch | grep -v LI | sort\n\n"
	    "lmeter.out: lmeter.dxc lmeter.term\n"
	    "\t$(LMDIR)lmeter lmeter.term < lmeter.dxc > lmeter.out "
	    "|| (rm lmeter.out && false)\n\n"
	    ))
  (close makefile))

;;; Handlers for LMeter IPC 

(defun lmProcTerm (cid exit)
  printf("LMeter finished with exit status = %L\n" exit) )

(setq lmWindow nil)

(defun lmProcStart ()
  (let (logfile)
    (setq logfile (strcat lmWorkingDir "/lmeter.log"))
    
    (if (null (windowp lmWindow))
	(progn 
	  (setq lmWindow (hiCreateWindow (list 100:100 600:800)
					 "text" "LMeter"))
	  (hiDisplayWindow lmWindow) ))

    (ipcBeginProcess (strcat "make -C " lmWorkingDir) 
		   ""
		   nil nil 'lmProcTerm
		   logfile)
    (hiSetViewfile lmWindow logfile)
    (hiSetWindowName lmWindow (strcat "LMeter @ " (car (last (parseString lmWorkingDir "/")))))
    (hiEnableTailViewfile lmWindow)
    )
)

(defun lmBackannotate ()
  (let (results
	instances 
	pnInds 
	spiceInds
	name value)
    (setq results (infile (strcat lmWorkingDir "/lm2sch.out")))
    (setq instances (geGetEditCellView)->instances)
    (setq pnInds (list "ind1" "ind2" "ind3" "ind3q"))
    (setq spiceInds (list "inductor" "lp"))
    (setq spiceJJ (list "rsj" "rsj1"))
    (while
	(fscanf results "%s %f" name value)
;      (printf "Searching for %s\n" name) 
      (foreach i instances
	       
	       (if (equal (upperCase name) (upperCase i->name))
		   (cond
		    ( (member i->cellName pnInds)
		      (printf "Updating %s: %f to %f\n"
			      i->name i->pnValue value)
		      i->pnValue=value)
		    ( (member i->cellName spiceInds)
		      (setq value (sprintf nil "%5.3fp" value))
		      (printf "Updating %s: %s to %s\n"
			      i->name i->l value)
			      i->l=value)))
	       (if (equal (upperCase name) (sprintf nil "LP%s" (upperCase i->name)))
		   (cond
		    ( (member i->cellName spiceJJ)
		      (setq value (sprintf nil "%5.3fp" value))
	              (printf "Updating Lp%s: %s to %s\n"
			      i->name i->lp value)
		      i->lp=value) ))
	       ))
    (close results))
    t)
    
    
    

;;; ****************************************************************
;;;
;;; Cadence to LMeter interface (Layout part)
;;; 
;;; ****************************************************************

;;;    Extract layout to DXC, etc. files for LMeter
(defun  lmLayoutExtract (lmWorkingDir fullP)	
  (setq cellid (geGetEditCellView))
  (if fullP (lmCopyAndClean cellid "lmeter") )
  (lmCreateLmeterInputData cellid "lmeter" lmWorkingDir)
  (writeTable (sprintf result "%s/%s" lmWorkingDir "lext.log") termInfo)
  (lmWriteTermInfo termInfo lmWorkingDir))

;;; Copy current view to 'lmeter' view and clean it up
(defun lmCopyAndClean (cellid workview)  
  thisLibName = cellid->libName
  thisCellName = cellid->cellName
  (info (sprintf nil "Creating lmeter view in %s:%s\n" 
		 thisLibName thisCellName))
  ;; Create lmeter view
  (geWriteCellView cellid thisLibName thisCellName workview)
  (setq workid (dbOpenCellView thisLibName thisCellName workview "" "a"))
  ;;  Flatten workview
  (foreach 
   inst workid~>instances 
   printf("%s of %s " inst~>objType inst~>cellName) 
   (println (leFlattenInst inst 20 t )))
  ;; clean-up workview
  (ivDRC 
   ?cell workid
   ?full t 
   ?set "lmeter?" 
   ?rulesFilePath "divaLMETER.rul")
  ;; and save it
  dbSave(workid)
  dbClose(workid)
)

;;; Write DXC file and accumulate termInfo information
(defun lmCreateLmeterInputData (cellid viewname lmWorkingDir)  
  thisLibName = cellid->libName
  thisCellName = cellid->cellName
  (info (sprintf nil "Writing DXC file for %s:%s\n" thisLibName thisCellName))

  (setq cellid (dbOpenCellView thisLibName thisCellName viewname "" "a"))
  ;; Convert shapes to polygons
  (foreach 
   shape cellid~>shapes
   shapeLayer = get_string(shape~>layerName)
   shapeType  = get_string(shape~>objType)
   (setq isTextLabelLinep
	 shapeLayer==lmTextLayer||shapeType=="label"||shapeType=="line")
   (cond ( ! isTextLabelLinep   leConvertShapeToPolygon(shape) )))

  ;; this table will store info on terminals as a list 
  ;; (termName termUp termDown)
  termInfo = makeTable("termInfo" 0)

  ;; Write to DXC file
  nterm=0
  (setq dxc (outfile sprintf(nil  "%s/%s" lmWorkingDir "lmeter.dxc") "w"))
  fprintf(dxc "$ENTITIES\n")
  (foreach 
   shape (setof dummy cellid~>shapes
		dummy~>objType=="polygon"||dummy~>objType=="line") 
   thisLayer=shape~>layerName
   (if (equal thisLayer lmTerminalLayer)
       (progn
	 nterm=nterm+1
	 elev=nterm
	 termInfo[nterm]=(list
			  (dbSearchPropByName shape "label")->value
			  (dbSearchPropByName shape "lmUP")->value
			  (dbSearchPropByName shape "lmDOWN")->value))
     (setq elev 0))
   ;; write polyline header
   fprintf(dxc "$POLYLINE\n%s\n%d\n" thisLayer elev)
   plist=shape~>points
   ;; write polyline points
   (foreach 
    point plist
    fprintf(dxc "%12.4f %12.4f\n" car(point) cadr(point)))
   (cond 
    (shape~>objType != "line"
		    fprintf(dxc "%12.4f %12.4f\n" car(car(plist)) cadr(car(plist))))))
    fprintf(dxc "$EOF\n")
    close(dxc)
    dbClose(cellid)
    ) ; ** procedure  **

;;; write termInfo for LMeter 
(procedure  lmWriteTermInfo(termInfo lmWorkingDir)
    nterm=length(termInfo)
    (info (sprintf nil "\nFound %d terminals\n" nterm))
    (for iterm 1 nterm
	 printf("%L: %L %L %L\n" iterm
		car(termInfo[iterm]) 
		nthelem(2 termInfo[iterm])
		nthelem(3 termInfo[iterm])))
    termfile=outfile(sprintf(result "%s/%s" lmWorkingDir "lmeter.term") "w")
    fprintf(termfile "nterm %d\n" nterm)
    fprintf(termfile "termname\t")
    (for iterm 1 nterm
	 fprintf(termfile "%s\t" nthelem(1 termInfo[iterm])))
    fprintf(termfile "\ntermtype\t")
    (for iterm 1 nterm
	 fprintf(termfile "I\t"))
    fprintf(termfile "\ntermup  \t")
    (for iterm 1 nterm
	 fprintf(termfile "%s\t" nthelem(2 termInfo[iterm])))
    fprintf(termfile "\ntermdown\t")
    (for iterm 1 nterm
      fprintf(termfile "%s\t" nthelem(3 termInfo[iterm])))
    fprintf(termfile "\n")
    close(termfile)
    lm2cirFile=outfile(sprintf(result "%s/%s" lmWorkingDir "term.name") "w")
    (for iterm 1 nterm
	 fprintf(lm2cirFile "%s %d\n" car(termInfo[iterm]) iterm))
    close(lm2cirFile)
)

;;; ****************************************************************
;;;
;;; Cadence to LMeter interface -- schematic netlister
;;; (c) Paul.Bunyk@trw.com 2001
;;;
;;; ****************************************************************

;;; Walk through one level of schematics and netlist all instances 
;;; (which are individual elements: junctions, inductors, etc., at this level)
;;; to a SPICE-like file.
;;;
;;; SOME ELEMENTS ARE IMPLICITLY TRANSFORMED:
;;;
;;; I/O terminals become Phase sources
;;;
;;; * A junction with one terminal grounded and another terminal having more 
;;; than one connection gets parasitic inductance in series.
;;; 
;;; * Resistors with a connection to the cell terminal (bias current sources)
;;; become inductances, others are ignored. 
;;;
;;; * Transformers are replaced by L1, L2 and M between L1 and L2.
;;;

(inScheme
(defun lm2SchNetlist (outFileName)
  (letseq
   ( (currCellView (geGetEditCellView))
     (instances currCellView->instances)
     (cellTermNames (currCellView->terminals~>name))
     (outFile (outfile outFileName "w"))
     (unusedNode (plus (length currCellView->nets) 0)) ; starting point for added nodes
     (nodeMap '( ("GND!" 0)))
     (maxNode 0) ; starting point for remapped nodes
     (mapNode (lambda (node) 
		(if (assoc node nodeMap) 
		    (cadr (assoc node nodeMap))
		  (progn 
		    (setq nodeMap (cons (list node (setq maxNode maxNode+1 )) nodeMap))
		    maxNode))))
     (paramNamesTRW '(;; Cells from TRW jjbasic + jjSymbolLib
		      ("junction_shunt" "a")
		      ("rsj" "a")
		      ("rsj1" "a")
		      ("junction" "a")
		      ("jjlp" "a")
		      ("inductor" "l")
		      ("lp" "l")
		      ("capacitor" "c")
		      ("resistor" "r")))
		   ;; Cells from SUNY rsfq.basic
     (paramNamesSUNY '(("ind1" "pnValue") 
		      ("ind2" "pnValue") 
		      ("ind3" "pnValue") 
		      ("ind3q" "pnValue") 
		      ("jj" "pnValue") 
		      ("isrc" "pnValue") 
		      ("isrcg" "pnValue"))) 
     (paramNames (append paramNamesTRW paramNamesSUNY))
     (info (sprintf nil (strcat 
			 "* LM2SCH netlisting of \"%s:%s:%s\" to \"%s\"\n"
			 "* started at %s\n")
		    currCellView->libName  
		    currCellView->cellName  
		    currCellView->viewName  
		    outFileName
		    (getCurrentTime)))
     (ioports nil)
     )
   (printf info)
   (fprintf outFile info)
   (foreach 
    inst instances
    (letseq
     ((cellName inst->cellName)
      (name inst->name)
      (prop inst->prop)
      (terms inst->instTerms)
      (net  inst->net)
      (paramName (assoc cellName paramNames)) )
     (cond
      ((member cellName (list "gnd" "GND" "GNDDW")) ; skip grounds
       t) 
      ((member cellName '("opin" "ipin" "iopin" "opinDW" "ipinDW" "iopinDW"))
       ;;; I/O pins are replaced with phase sources

       ;;; We want to drop I/O pins which share nodes with other
       ;;; terminals (junctions), Q.H.insists on this, I have finally
       ;;; agreed with him.

       ;;; Rule: if there is anything directly connected to the I/O
       ;;; pin which is not an inductor/transformer, DROP this pin
       ;;; (the other terminal will be generated for that element)
       ; (car geGetSelectedSet())~>net~>instTerms~>inst~>cellName
       (letseq
	( (cellNames inst->net~>instTerms~>inst~>cellName)
	  (inductors (list "ind1" "ind2" "ind3" "ind3q" "muind" ;; SUNY
			   "resistor" "inductor" "lp" "xfr" ;; jjSymbolLib
			   "res" "npn" "pnp" "ind")) ;; HBT stuff 
	  (doNetlist t)
	 )
	(foreach i cellNames
		 (if (not (member i inductors))
		     (setq doNetlist nil)))
	(if doNetlist
	    (fprintf outFile "P%s %d %d 0.0\n" 
		     net->name
		     (mapNode net->name) 
		     (mapNode "GND!")))
	))
      
      ((equal cellName "xfr") ; transformer
       (letseq 
	( (netNames (terms~>net)~>name)
	  (l1 (dbGetPropByName inst "l1")->value)
	  (l2 (dbGetPropByName inst "l2")->value)
	  (k (dbGetPropByName inst "k")->value)
	  )
	(fprintf outFile "L1%s %d %d %s\n" 
		 name 
		 (mapNode (nth 0 netNames))
		 (mapNode (nth 2 netNames)) l1)
	(fprintf outFile "L2%s %d %d %s\n" 
		 name 
		 (mapNode (nth 1 netNames)) 
		 (mapNode (nth 3 netNames)) l2)
	;; here k is coupling coefficient, not mutual inductance M:
	;; k = M/Sqrt(l1*l2). LMeter deals with M, so we convert it here.
	;; NB: scaling factors ('p' for pF) are not converted though.
	;; Move this conversion to lm2sch itself?
	(fprintf outFile "M%s L1%s L2%s %f\n" 
		 name name name (atof k)*sqrt((atof l1)*(atof l2)) )))       
      
      ((equal cellName "resistor")
       ;; check if it is a bias resistor, then replace with inductor, 
       ;; otherwise ignore.
       (letseq 
	( (netNames (terms~>net)~>name)
	  (t0 (nth 0 netNames))
	  (t1 (nth 1 netNames))
	  )
	(if (or (member t0 cellTermNames)
		(member t1 cellTermNames)) ; connected to the terminal, bias!
	    (fprintf outFile "LI%s %d %d 1.0\n" name (mapNode t0) (mapNode t1) ))))

      ((member cellName (list "junction_shunt" "rsj" "rsj1" "junction" "jjlp"))
       ;; Junction: check if it needs parasitic inductance
       (letseq 
	( (nets (terms~>net))
	  (n0 (cadr nets))
	  (n1 (car nets))
	  (nm0 (mapNode n0->name))
	  (nm1 (mapNode n1->name))
	  )
	(if (or (and (equal n0->name "GND!") n1->instTermCount > 2)
		(and (equal n1->name "GND!") n0->instTermCount > 2) 
		(and n1->instTermCount > 2   n0->instTermCount > 2) )
	    ;; add parasitic inductance
	    (letseq
	     ( (newNode (setq unusedNode unusedNode+1)) 
	       (lp (dbSearchPropByName inst "lp"))
	       )
	     (printf "Parasitic inductance added for %s\n" name)
	     ;; NB: parasitic inductance property of junction should go here
	     (fprintf outFile "Lp%s %d %d %s\n" name newNode nm1 (if lp lp->value "0.00"))
	     (setq nm1 newNode)))
	;; add junction itself
	(fprintf outFile "%s %d %d " name nm0 nm1)
	(if paramName
	    (foreach 
	     p prop
	     (if (equal p->name (cadr paramName))
		 (fprintf outFile "%s" p->value))))
	(fprintf outFile "\n")))
       
      (t ;;; this is the only case for SUNY symbols
       (fprintf outFile "%s " name)
       (foreach 
	term (reverse terms)
	(fprintf outFile "%d " (mapNode term->net->name)))
       ;; special case for SUNY 1-term grounded I-source
       (if (equal cellName "isrcg") (fprintf outFile "0 ")) 
       (if paramName
	   (foreach 
	    p prop
	    (if (equal p->name (cadr paramName))
		(fprintf outFile (if (floatp p->value) "%f" "%s") p->value))))
       (fprintf outFile "\n"))
      )
     )
    )
   (fprintf outFile ".END")
   (close outFile)
   )  
 t
)
)

