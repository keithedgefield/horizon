CC=cc
LD=ld
AR=ar
YACC=bison
LEX=flex

CPPFLAGS=\
	-I../../src \
	-I./libroot/include

CFLAGS=\
	-O0 \
	-g3

LDFLAGS=\
	libroot/lib/libpng.a \
	libroot/lib/libjpeg.a \
	libroot/lib/libwebp.a \
	libroot/lib/libvorbis.a \
	libroot/lib/libogg.a \
	libroot/lib/libbrotlidec.a \
	libroot/lib/libbrotlicommon.a \
	libroot/lib/libbz2.a \
	libroot/lib/libz.a \
	-lX11 \
	-lasound \
	-lasound \
	-lX11 \
	-lXpm \
	-lGL \
	-lGLX \
	-lpthread \
	-lm

SRCS=\
	../../src/main.c \
	../../src/hal/linuxmain.c \
	../../src/hal/stdfile.c \
	../../src/hal/stdimage.c \
	../../src/hal/glrender.c \
	../../src/linguine/parser.tab.c \
	../../src/linguine/lexer.yy.c \
	../../src/linguine/ast.c \
	../../src/linguine/hir.c \
	../../src/linguine/lir.c \
	../../src/linguine/runtime.c

OBJS=\
	main.o \
	linuxmain.o \
	stdfile.o \
	stdimage.o \
	glrender.o \
	parser.tab.o \
	lexer.yy.o \
	ast.o \
	hir.o \
	lir.o \
	runtime.o

all: horizon

horizon: $(OBJS)
	$(CC) -o horizon $(CPPFLAGS) $(CFLAGS) $(OBJS) $(LDFLAGS)

libroot:
	@../../external/build-libs.sh "" $(CC) "-O2 -g0" $(AR)

main.o: ../../src/main.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

linuxmain.o: ../../src/hal/linuxmain.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

stdfile.o: ../../src/hal/stdfile.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

stdimage.o: ../../src/hal/stdimage.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

glrender.o: ../../src/hal/glrender.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

parser.tab.o: ../../src/linguine/parser.tab.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

lexer.yy.o: ../../src/linguine/lexer.yy.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

ast.o: ../../src/linguine/ast.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

hir.o: ../../src/linguine/hir.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

lir.o: ../../src/linguine/lir.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

runtime.o: ../../src/linguine/runtime.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

src/linguine/parser.tab.c: src/linguine/parser.y
	$(YACC) -Wcounterexamples -d -p ast_yy -o ../../src/linguine/parser.tab.c ../../src/linguine/parser.y

src/linguine/lexer.yy.c: src/linguine/lexer.l
	$(LEX) -o ../../src/linguine/lexer.yy.c --prefix=ast_yy ../../src/linguine/lexer.l

clean:
	rm -rf horizon $(OBJS)
