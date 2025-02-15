HORIZON_SRCS = \
	src/main.c

LINGUINE_SRCS = \
	src/linguine/parser.tab.c \
	src/linguine/lexer.yy.c \
	src/linguine/ast.c \
	src/linguine/hir.c \
	src/linguine/lir.c \
	src/linguine/runtime.c

LIN_SRCS = \
	src/linguine/lin.c

LINC_SRCS = \
	src/linguine/linc.c

MEDIAKIT_SRCS =

SRCS = $(HORIZON_SRCS) $(LINGUINE_SRCS) $(MEDIAKIT_SRCS)

CPPFLAGS = -Isrc
CFLAGS = -O0 -g3
LDFLAGS = -lefence

all: horizon lin linc

##
## horizon
##

horizon: $(SRCS)
	$(CC) -o horizon $(CPPFLAGS) $(CFLAGS) $(SRCS)

##
## linguine
##

lin: $(LIN_SRCS)  $(LINGUINE_SRCS)
	$(CC) -o lin $(CPPFLAGS) $(CFLAGS) $(LIN_SRCS) $(LINGUINE_SRCS)

linc: $(LINC_SRCS) $(LINGUINE_SRCS)
	$(CC) -o linc $(CPPFLAGS) $(CFLAGS) $(LINC_SRCS) $(LINGUINE_SRCS)

../src/linguine/parser.tab.c: ../src/linguine/parser.y
	bison -Wcounterexamples -d -p ast_yy -o ../src/linguine/parser.tab.c ../src/linguine/parser.y

../src/linguine/lexer.yy.c: ../src/linguine/lexer.l
	flex -o ../src/linguine/lexer.yy.c --prefix=ast_yy ../src/linguine/lexer.l

##
## Phony
##

clean:
	rm -rf horizon lin linc
