<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY dbstyle SYSTEM "/usr/lib/sgml/stylesheets/nwalsh-modular/html/docbook.dsl" CDATA DSSSL>
]>
<style-sheet>
<style-specification id="html" use="docbook">
<style-specification-body>



;;#######################################################################
;;#                                                                     #
;;#    Dave's Custom DocBook Stylesheet by Dave Mason dcm@redhat.com    #
;;#            Based on Norman Walsh's Modular Stylesheets              #
;;#                                                                     #
;;#######################################################################
;;
;;#######################################################################
;;#                                                                     #
;;#    You can format your document using this stylesheet by running    #
;;#                                                                     #
;;#      $ jade -t sgml -ihtml -d white-papers-html.dsl file.sgml       #
;;#                                                                     #
;;#######################################################################

(declare-characteristic preserve-sdata?
          "UNREGISTERED::James Clark//Characteristic::preserve-sdata?"
          #f)

;;=========================
;;General Stuff
;;=========================

;;Should there be a link to the legalnotice?
(define %generate-legalnotice-link%
  #t)

;;Should Admon Graphics be used?
(define %admon-graphics%
  #f)

;;Where are those admon graphics?
(define %admon-graphics-path%
  "./stylesheet-images/")

;;What graphics extensions allowed?
(define %graphic-extensions% 
'("gif" "jpg" "jpeg" "tif" "tiff" "eps" "epsf" ))

;;What is the default extension for images?
(define %graphic-default-extension% "gif")

;;Use element ids as filenames?
(define %use-id-as-filename%
 #t)

;;=========================
;;Book Stuff
;;=========================

;;Do you want a TOC for Books?
(define %generate-book-toc% 
  #t)

;;What elements should have an LOT?
(define ($generate-book-lot-list$)
  (list (normalize "table")
  (normalize "example")
  (normalize "equation")))

;;Do you want a title page for your Book?
(define %generate-book-titlepage%
#t)

;;=========================
;;Part Stuff
;;=========================

;;Should parts have TOCs?
(define %generate-part-toc% 
  #t)

;;Should part TOCs be on their titlepages?
(define %generate-part-toc-on-titlepage%
  #t)

;;Do you want a title page for your part?
(define %generate-part-titlepage% 
  #t)

;;Should the Part intro be on the part title page?
(define %generate-partintro-on-titlepage%
 #t)

;;=========================
;;Article Stuff
;;=========================

;;Should Articles have a TOC?
(define %generate-article-toc% 
  #f)
;;=========================
;;Navigation
;;=========================

;;Should there be navigation at top?
(define %header-navigation%
 #t)

;;Should there be navigation at bottom?
(define %footer-navigation%
  #t)

;;Use tables to create the navigation?
(define %gentext-nav-use-tables%
 #t)

;;If tables are used for navigation, how wide should they be? 
(define %gentext-nav-tblwidth% 
"100%")

;;=========================
;;Tables and Lists
;;=========================

;;Should Variable lists be tables?
(define %always-format-variablelist-as-table%
 #f)

;;What is the length of the 'Term' in a variablelist?
(define %default-variablelist-termlength%
  20)

;;When true.. If the terms are shorter than the termlength above then
;;the variablelist will be formatted as a table.
(define %may-format-variablelist-as-table%
#f)

;; This overrides the tgroup definition (copied from 1.20, dbtable.dsl).
;; It changes the table background color, cell spacing and cell padding.
(element tgroup
  (let* ((wrapper   (parent (current-node)))
	 (frameattr (attribute-string (normalize "frame") wrapper))
	 (pgwide    (attribute-string (normalize "pgwide") wrapper))
	 (footnotes (select-elements (descendants (current-node)) 
				     (normalize "footnote")))
	 (border (if (equal? frameattr (normalize "none"))
		     '(("BORDER" "0"))
		     '(("BORDER" "1"))))
	 (bgcolor '(("BGCOLOR" "#E0E0E0")))
	 (width (if (equal? pgwide "1")
		    (list (list "WIDTH" ($table-width$)))
		    '()))
	 (head (select-elements (children (current-node)) (normalize "thead")))
	 (body (select-elements (children (current-node)) (normalize "tbody")))
	 (feet (select-elements (children (current-node)) (normalize "tfoot"))))
    (make element gi: "TABLE"
	  attributes: (append
		       border
		       width
		       bgcolor
		       '(("CELLSPACING" "0"))
		       '(("CELLPADDING" "4"))
		       (if %cals-table-class%
			   (list (list "CLASS" %cals-table-class%))
			   '()))
	  (process-node-list head)
	  (process-node-list body)
	  (process-node-list feet)
	  (make-table-endnotes))))

;;=========================
;;Labels
;;=========================

;;Enumerate Chapters?
(define %chapter-autolabel% 
 #f)

;;Enumerate Sections?
(define %section-autolabel%
 #f)

;;=========================
;;HTML Attributes
;;=========================

;;What attributes should be hung off of 'body'?
(define %body-attr%
 (list
   (list "BGCOLOR" "#FFFFFF")
   (list "TEXT" "#000000")
   (list "LINK" "#0000FF")
   (list "VLINK" "#840084")
   (list "ALINK" "#0000FF")))

;;Default extension for filenames?
(define %html-ext% 
  ".html")

;;=========================
;;Elements
;;=========================

;;Indent addresses?
(define %indent-address-lines%
  #f)

;;Indent Literal layouts?
(define %indent-literallayout-lines% 
  #f)

;;Indent Programlistings?
(define %indent-programlisting-lines%
  #f)

;;Number lines in Programlistings?
(define %number-programlisting-lines%
 #f)

;;Should verbatim items be 'shaded' with a table?
(define %shade-verbatim% 
 #t)

;;Define shade-verbatim attributes
(define ($shade-verbatim-attr$)
 (list
  (list "BORDER" "0")
  (list "BGCOLOR" "#E0E0E0")
  (list "WIDTH" ($table-width$))))

;;=================INLINES====================

(element application ($mono-seq$))
(element command ($bold-seq$))
(element filename ($mono-seq$))
(element function ($mono-seq$))
(element guibutton ($bold-seq$))
(element guiicon ($bold-seq$))
(element guilabel ($italic-seq$))
(element guimenu ($bold-seq$))
(element guimenuitem ($bold-seq$))
(element hardware ($bold-mono-seq$))
(element keycap ($bold-seq$))
(element literal ($mono-seq$))
(element parameter ($italic-mono-seq$))
(element prompt ($mono-seq$))
(element symbol ($charseq$))
(element emphasis ($italic-seq$))

;=============================================



;;=========================
;;Title Pages For Articles
;;=========================

(define (article-titlepage-recto-elements)
  (list (normalize "title")
	(normalize "subtitle")
	(normalize "corpauthor")
	(normalize "authorgroup")
	(normalize "author")
	(normalize "releaseinfo")
	(normalize "copyright")
	(normalize "pubdate")
	(normalize "revhistory")
	(normalize "abstract")))

(element abstract
    (make element gi: "DIV"
	  ($semiformal-object$)))

(element address 
    (make element gi: "DIV"
	  attributes: (list (list "CLASS" (gi)))
	  (with-mode titlepage-address-mode 
	    ($linespecific-display$ %indent-address-lines% %number-address-lines%))))


 (element affiliation
    (make element gi: "DIV"
	  attributes: (list (list "CLASS" (gi)))
	  (process-children)))


(element author
    (let ((author-name  (author-string))
	  (author-affil (select-elements (children (current-node)) 
					 (normalize "affiliation"))))
      (make sequence      
	(make element gi: "H3"
	      attributes: (list (list "CLASS" (gi)))
              (literal "by ")
	      (literal author-name))
	(process-node-list author-affil))))



</style-specification-body>
</style-specification>
<external-specification id="docbook" document="dbstyle">
</style-sheet>