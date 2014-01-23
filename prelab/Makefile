
# $a  lhs of :
# $^  rhs of :
# $<  first item in dependency list
CC=gcc

a.out: exec.c
	$(CC) -o $@ $^

.PHONY: clean delete
clean:
	rm dummy* -rf
	rm *.out -f
delete:
	rm dummy* -rf

