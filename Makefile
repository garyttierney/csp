MAKEFLAGS+=--no-builtin-rules
CFLAGS=-I./libsecsp/include
YACC=bison
LEX=flex

ifeq ($(DEBUG),1)
	CFLAGS += -g3 -O0 -gdwarf-2 -fno-strict-aliasing -Wall -Wno-error -Wshadow
	LDFLAGS += -g
endif

LIBSECSP_GEN_SRC=libsecsp/src/lexer.yy.c \
				 libsecsp/src/parser.tab.c
LIBSECSP_GEN_HDR=$(LIBSECSP_GEN_SRC:.c=.h)

LIBSECSP_SRC=libsecsp/src/ast.c \
			 libsecsp/src/parser.c
LIBSECSP_OBJ=$(LIBSECSP_GEN_SRC:.c=.o)
LIBSECSP_OBJ+=$(LIBSECSP_SRC:.c=.o)

all: libsecsp.so secspc
clean: clean-libsecsp clean-secspc

$(LIBSECSP_OBJ): CFLAGS+=-fPIC -fvisibility=hidden
libsecsp.so: $(LIBSECSP_OBJ)
	$(CC) -shared -fPIC -fvisibility=hidden -Wl,-soname,$@ -o $@ $(LIBSECSP_OBJ) -lc

# Make sure the parser header is generated before the
# lexer is compiled

libsecsp/src/lexer.yy.o: libsecsp/src/parser.tab.h

clean-libsecsp:
	@$(RM) $(LIBSECSP_OBJ) $(LIBSECSP_GEN_SRC) $(LIBSECSP_GEN_HDR)
	@$(RM) libsecsp.so

SECSPC_SRC=compiler/src/secspc.c
SECSPC_OBJ=$(SECSPC_SRC:.c=.o)

$(SECSPC_OBJ): CFLAGS+=
secspc: $(SECSPC_OBJ)
	$(CC) $(CFLAGS) -o $@ $< -L. -lsecsp

clean-secspc:
	@$(RM) $(SECSPC_OBJ)
	@$(RM) secspc

%.tab.h: %.tab.c
	touch $@

%.tab.c: %.y
	$(YACC) -l -o $@ --defines="$(@:.c=.h)" $<

%.yy.h: %.yy.c
	touch $@

%.yy.c: %.l
	$(LEX) -o $@ --header-file="$(@:.c=.h)" $<

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
