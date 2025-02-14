HORIZON_SRCS = \
	../src/horizon/main.c

LINGUINE_SRCS = \
	../src/linguine/parser.tab.c \
	../src/linguine/lexer.yy.c \
	../src/linguine/ast.c \
	../src/linguine/hir.c \
	../src/linguine/lir.c \
	../src/linguine/runtime.c

MEDIAKIT_SRCS =

SRCS = $(HORIZON_SRCS) $(LINGUINE_SRCS) $(MEDIAKIT_SRCS)

CPPFLAGS = -I../src -I../src/linguine
CFLAGS = -O0 -g3
LDFLAGS = -lefence

all: horizon

##
## horizon
##

horizon: $(SRCS)
	$(CC) -o horizon $(CPPFLAGS) $(CFLAGS) $(SRCS)

##
## linguine
##

# bison
../src/linguine/parser.tab.c: ../src/linguine/parser.y
	bison -Wcounterexamples -d -p ast_yy -o ../src/linguine/parser.tab.c ../src/linguine/parser.y

# flex
../src/linguine/lexer.yy.c: ../src/linguine/lexer.l
	flex -o ../src/linguine/lexer.yy.c --prefix=ast_yy ../src/linguine/lexer.l
