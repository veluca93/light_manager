# Tools
LATEXMK = latexmk
RM = rm -f

# Targets
all: doc
doc: pdf
pdf: $(DOCNAME).pdf

# Rules
%.pdf: %.tex
		$(LATEXMK) -shell-escape -pdf -M -MP -MF $*.d $*

mostlyclean:
		$(LATEXMK) -silent -c
		$(RM) *.bbl
		$(RM) -r _minted-$(DOCNAME) 
		$(RM) -r $(DOCNAME).snm $(DOCNAME).nav $(DOCNAME).pre $(DOCNAME).vrb 

clean: mostlyclean
		$(LATEXMK) -silent -C
		$(RM) *.run.xml *.synctex.gz
		$(RM) *.d

.PHONY: all clean doc mostlyclean pdf

# Include auto-generated dependencies
-include *.d
