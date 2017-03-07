all:
	xelatex -shell-escape socket.tex
clean:
	@rm -r _minted-socket *.log *.aux *.log *.nav *.out *.pdf *.snm *.toc *.vrb
