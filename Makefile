-include nemu/Makefile.git

default:
	@echo "Please run 'make' under any subprojects to compile."
 
clean:
	-$(MAKE) -C nemu clean
	-$(MAKE) -C nexus-am clean
	-$(MAKE) -C nanos-lite clean
	-$(MAKE) -C navy-apps clean

submit: clean
	git gc
	cd .. && tar cj $(shell basename `pwd`) > $(STU_ID).tar.bz2

.PHONY: default clean submit

CURRENET_COUNT = $(shell find ./nemu/ -name "*.[ch]" | xargs cat | grep -Ev "^$$" | wc -l)
ADD_COUNT = $(shell expr $(CURRENET_COUNT) - 2817)
count:
	@echo "$(CURRENET_COUNT) lines of nemu code in total currently."
	@echo "Already add $(ADD_COUNT) lines."
