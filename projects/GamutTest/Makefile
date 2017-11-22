###
###     Makefile for GamutTest project
###
###             generated for UNIX/LINUX/Mac environments
###             by A. M. Tourapis
###



NAME = GamutTest

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

DEPEND= dependencies

BINDIR= ../../bin
INCDIR= inc
SRCDIR= src
OBJDIR= obj
LIBDIR= ../../lib

#ADDSRCDIR= ../../common/src
ADDINCDIR= ../../common/inc


ifeq ($(M32),1)
CC=     $(shell which g++) -m32
else
CC=     $(shell which g++) 
endif

ifeq ($(STC),1)
ifeq ($(DBG),1)  ### Do not use static compilation for Debug mode
STC=0
STATIC=
else
STATIC= -static
endif
else
STATIC= 
endif


LIBS    =   -lm $(STATIC)
AFLAGS  =  
ifeq ($OS), Windows_NT)
  CFLAGS += -ffloat-store
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Darwin)
  else
    CFLAGS += -ffloat-store    
  endif
endif
CFLAGS +=  -D JM_PSNR -fno-strict-aliasing -fsigned-char -msse2 -mfpmath=sse  $(STATIC) -I$(LIBDIR)
#CFLAGS +=  -D JM_PSNR -ffloat-store -fno-strict-aliasing -fsigned-char $(STATIC)
FLAGS=  $(CFLAGS) -Wall -I$(INCDIR) -I$(ADDINCDIR) -D __USE_LARGEFILE64 -D _FILE_OFFSET_BITS=64 
#FLAGS=  -ffloat-store -Wall -I$(INCDIR) -I$(ADDINCDIR) -D __USE_LARGEFILE64 -D _FILE_OFFSET_BITS=64

OPT_FLAG = -O$(OPT)
ifeq ($(DBG),1)
SUFFIX= .dbg
FLAGS+= -g
LDFLAGS += $(LIBDIR)/HDRLib.a.dbg
ifeq ($(MMX),1)
SUFFIX= .dmmx
FLAGS+=  $(OPT_FLAG) -D USEMMX  
#FLAGS+= -O3 -march=pentium4 -fomit-frame-pointer
endif
else
LDFLAGS += $(LIBDIR)/HDRLib.a
SUFFIX=
ifeq ($(MMX),1)
SUFFIX= .mmx
AFLAGS+=  -march=athlon64
FLAGS+=  $(AFLAGS) $(OPT_FLAG) -D USEMMX  
#FLAGS+= -O3 -march=pentium4 -fomit-frame-pointer
endif
FLAGS+= $(OPT_FLAG) -fomit-frame-pointer
endif

OBJSUF= .o$(SUFFIX)

SRC=    $(wildcard $(SRCDIR)/*.cpp) 
ADDSRC= $(wildcard $(ADDSRCDIR)/*.cpp)
#OBJ=    $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o$(SUFFIX))  $(ADDSRC:$(ADDSRCDIR)/%.cpp=$(OBJDIR)/%.o$(SUFFIX)) 
OBJ=    $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o$(SUFFIX))  
BIN=    $(BINDIR)/$(NAME)$(SUFFIX)

.PHONY: default distclean clean tags depend

default: messages objdir_mk depend bin tags

messages:
ifeq ($(M32),1)
	@echo 'Compiling with M32 support...'
endif
ifeq ($(DBG),1)
	@echo 'Compiling with Debug support...'
	@echo 'Note static compilation not supported in this mode.'
endif
ifeq ($(STC),1)
	@echo 'Compiling with -static support...'
endif
ifeq ($(MMX),1)
	@echo 'Compiling with MMX support...'
endif

dependencies:
	@echo "" >dependencies

clean:
	@echo remove all objects
	@rm -rf $(OBJDIR)

distclean: clean
	@rm -f $(DEPEND) tags
	@rm -f $(BIN)

tags:
	@echo update tag table
	@ctags -w  inc/*.H src/*.cpp

bin:    $(OBJ)
	@echo
	@echo 'creating binary "$(BIN)"'
	@$(CC) $(FLAGS) -o $(BIN) $(OBJ) $(LDFLAGS) $(LIBS)
	@echo '... done'
	@echo

depend:
	@echo
	@echo 'checking dependencies'
	@$(SHELL) -ec '$(CC) $(FLAGS) -MM -I$(INCDIR) -I$(ADDINCDIR) $(SRC) $(ADDSRC)  \
         | sed '\''s@\(.*\)\.o[ :]@$(OBJDIR)/\1.o$(SUFFIX):@g'\''               \
         >$(DEPEND)'
	@echo

$(OBJDIR)/%.o$(SUFFIX): $(SRCDIR)/%.cpp
	@echo 'compiling object file "$@" ...'
	@$(CC) -c -o $@ $(FLAGS) $<

$(OBJDIR)/%.o$(SUFFIX): $(ADDSRCDIR)/%.cpp
	@echo 'compiling object file "$@" ...'
	@$(CC) -c -o $@ $(FLAGS) $<

objdir_mk:
	@echo 'Creating $(OBJDIR) ...'
	@mkdir -p $(OBJDIR)

-include $(DEPEND)

