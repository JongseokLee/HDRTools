###
###     Makefile for HDRTools
###
###             generated for UNIX/LINUX/MAC OS environments
###             by A. M. Tourapis
###             Copyright: Apple Inc
###             Created: July 17, 2014
###

SUBDIRS := common projects/HDRConvert projects/HDRVQM projects/HDRConvScaler projects/HDRMetrics projects/ChromaConvert projects/HDRMontage projects/GamutTest

### include debug information: 1=yes, 0=no
DBG?= 0
### include MMX optimization : 1=yes, 0=no
MMX?= 0
### Generate 32 bit executable : 1=yes, 0=no
M32?= 0
### include O level optimization : 0-3
OPT?= 3
### Static Compilation
STC?= 0

#check for LLVM and silence warnings accordingly
LLVM = $(shell $(CC) --version | grep LLVM)
ifneq ($(LLVM),)
        CFLAGS+=-Qunused-arguments
else
        CFLAGS+=-Wno-unused-but-set-variable
endif

export DBG
export STC 
export M32
export MMX

.PHONY: default all distclean clean tags depend $(SUBDIRS)

default: all

all: | $(SUBDIRS) 

$(SUBDIRS):
	$(MAKE) -C $@

clean depend:
	@echo "Cleaning dependencies"
	@for i in $(SUBDIRS); do make -C $$i $@; done

tags:
	@echo "Update tag table at top directory"
	@ctags -R .
	@for i in $(SUBDIRS); do make -C $$i $@; done

distclean: clean
	@echo "Cleaning all"
	@rm -f tags
	@rm -f lib/*
	@for i in $(SUBDIRS); do make -C $$i $@; done



