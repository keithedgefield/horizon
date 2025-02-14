/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         ast_yyparse
#define yylex           ast_yylex
#define yyerror         ast_yyerror
#define yydebug         ast_yydebug
#define yynerrs         ast_yynerrs
#define yylval          ast_yylval
#define yychar          ast_yychar
#define yylloc          ast_yylloc

/* First part of user prologue.  */
#line 1 "../src/linguine/parser.y"

/*
 * Horizon
 * Copyright (C) 2025, The Horizon Authors. All rights reserved.
 */
#include "ast.h"

#include <stdio.h>
#include <string.h>

#undef DEBUG
#ifdef DEBUG
static void print_debug(const char *s);
#define debug(s) print_debug(s)
#else
#define debug(s)
#endif

extern int ast_error_line;
extern int ast_error_column;

int ast_yylex(void *);
void ast_yyerror(void *, char *s);
#line 26 "../src/linguine/parser.y"

#include "stdio.h"
extern void ast_yyerror(void *scanner, char *s);

#line 108 "../src/linguine/parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_TOKEN_SYMBOL = 3,               /* TOKEN_SYMBOL  */
  YYSYMBOL_TOKEN_STR = 4,                  /* TOKEN_STR  */
  YYSYMBOL_TOKEN_INT = 5,                  /* TOKEN_INT  */
  YYSYMBOL_TOKEN_FLOAT = 6,                /* TOKEN_FLOAT  */
  YYSYMBOL_TOKEN_FUNC = 7,                 /* TOKEN_FUNC  */
  YYSYMBOL_TOKEN_PLUS = 8,                 /* TOKEN_PLUS  */
  YYSYMBOL_TOKEN_MINUS = 9,                /* TOKEN_MINUS  */
  YYSYMBOL_TOKEN_MUL = 10,                 /* TOKEN_MUL  */
  YYSYMBOL_TOKEN_DIV = 11,                 /* TOKEN_DIV  */
  YYSYMBOL_TOKEN_MOD = 12,                 /* TOKEN_MOD  */
  YYSYMBOL_TOKEN_ASSIGN = 13,              /* TOKEN_ASSIGN  */
  YYSYMBOL_TOKEN_LPAR = 14,                /* TOKEN_LPAR  */
  YYSYMBOL_TOKEN_RPAR = 15,                /* TOKEN_RPAR  */
  YYSYMBOL_TOKEN_LBLK = 16,                /* TOKEN_LBLK  */
  YYSYMBOL_TOKEN_RBLK = 17,                /* TOKEN_RBLK  */
  YYSYMBOL_TOKEN_LARR = 18,                /* TOKEN_LARR  */
  YYSYMBOL_TOKEN_RARR = 19,                /* TOKEN_RARR  */
  YYSYMBOL_TOKEN_SEMICOLON = 20,           /* TOKEN_SEMICOLON  */
  YYSYMBOL_TOKEN_DOT = 21,                 /* TOKEN_DOT  */
  YYSYMBOL_TOKEN_ARROW = 22,               /* TOKEN_ARROW  */
  YYSYMBOL_TOKEN_COMMA = 23,               /* TOKEN_COMMA  */
  YYSYMBOL_TOKEN_IF = 24,                  /* TOKEN_IF  */
  YYSYMBOL_TOKEN_ELSE = 25,                /* TOKEN_ELSE  */
  YYSYMBOL_TOKEN_WHILE = 26,               /* TOKEN_WHILE  */
  YYSYMBOL_TOKEN_FOR = 27,                 /* TOKEN_FOR  */
  YYSYMBOL_TOKEN_IN = 28,                  /* TOKEN_IN  */
  YYSYMBOL_TOKEN_DOTDOT = 29,              /* TOKEN_DOTDOT  */
  YYSYMBOL_TOKEN_GT = 30,                  /* TOKEN_GT  */
  YYSYMBOL_TOKEN_GTE = 31,                 /* TOKEN_GTE  */
  YYSYMBOL_TOKEN_LT = 32,                  /* TOKEN_LT  */
  YYSYMBOL_TOKEN_LTE = 33,                 /* TOKEN_LTE  */
  YYSYMBOL_TOKEN_EQ = 34,                  /* TOKEN_EQ  */
  YYSYMBOL_TOKEN_NEQ = 35,                 /* TOKEN_NEQ  */
  YYSYMBOL_TOKEN_RETURN = 36,              /* TOKEN_RETURN  */
  YYSYMBOL_TOKEN_BREAK = 37,               /* TOKEN_BREAK  */
  YYSYMBOL_TOKEN_CONTINUE = 38,            /* TOKEN_CONTINUE  */
  YYSYMBOL_TOKEN_AND = 39,                 /* TOKEN_AND  */
  YYSYMBOL_TOKEN_OR = 40,                  /* TOKEN_OR  */
  YYSYMBOL_YYACCEPT = 41,                  /* $accept  */
  YYSYMBOL_func_list = 42,                 /* func_list  */
  YYSYMBOL_func = 43,                      /* func  */
  YYSYMBOL_param_list = 44,                /* param_list  */
  YYSYMBOL_stmt_list = 45,                 /* stmt_list  */
  YYSYMBOL_stmt = 46,                      /* stmt  */
  YYSYMBOL_expr_stmt = 47,                 /* expr_stmt  */
  YYSYMBOL_assign_stmt = 48,               /* assign_stmt  */
  YYSYMBOL_if_stmt = 49,                   /* if_stmt  */
  YYSYMBOL_elif_stmt = 50,                 /* elif_stmt  */
  YYSYMBOL_else_stmt = 51,                 /* else_stmt  */
  YYSYMBOL_while_stmt = 52,                /* while_stmt  */
  YYSYMBOL_for_stmt = 53,                  /* for_stmt  */
  YYSYMBOL_return_stmt = 54,               /* return_stmt  */
  YYSYMBOL_break_stmt = 55,                /* break_stmt  */
  YYSYMBOL_continue_stmt = 56,             /* continue_stmt  */
  YYSYMBOL_expr = 57,                      /* expr  */
  YYSYMBOL_arg_list = 58,                  /* arg_list  */
  YYSYMBOL_term = 59                       /* term  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1046

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  19
/* YYNRULES -- Number of rules.  */
#define YYNRULES  70
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  159

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   295


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   107,   107,   112,   118,   123,   128,   133,   139,   144,
     150,   155,   161,   167,   173,   179,   185,   191,   197,   203,
     209,   215,   222,   228,   234,   239,   245,   250,   256,   261,
     267,   272,   278,   283,   288,   293,   298,   303,   309,   315,
     321,   327,   332,   337,   342,   347,   352,   357,   362,   367,
     372,   377,   382,   387,   392,   397,   402,   407,   412,   417,
     422,   427,   432,   438,   443,   449,   454,   459,   464,   469,
     474
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "TOKEN_SYMBOL",
  "TOKEN_STR", "TOKEN_INT", "TOKEN_FLOAT", "TOKEN_FUNC", "TOKEN_PLUS",
  "TOKEN_MINUS", "TOKEN_MUL", "TOKEN_DIV", "TOKEN_MOD", "TOKEN_ASSIGN",
  "TOKEN_LPAR", "TOKEN_RPAR", "TOKEN_LBLK", "TOKEN_RBLK", "TOKEN_LARR",
  "TOKEN_RARR", "TOKEN_SEMICOLON", "TOKEN_DOT", "TOKEN_ARROW",
  "TOKEN_COMMA", "TOKEN_IF", "TOKEN_ELSE", "TOKEN_WHILE", "TOKEN_FOR",
  "TOKEN_IN", "TOKEN_DOTDOT", "TOKEN_GT", "TOKEN_GTE", "TOKEN_LT",
  "TOKEN_LTE", "TOKEN_EQ", "TOKEN_NEQ", "TOKEN_RETURN", "TOKEN_BREAK",
  "TOKEN_CONTINUE", "TOKEN_AND", "TOKEN_OR", "$accept", "func_list",
  "func", "param_list", "stmt_list", "stmt", "expr_stmt", "assign_stmt",
  "if_stmt", "elif_stmt", "else_stmt", "while_stmt", "for_stmt",
  "return_stmt", "break_stmt", "continue_stmt", "expr", "arg_list", "term", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-36)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
       1,    22,     4,   -36,    12,   -36,   -36,     2,   -36,    -6,
       7,    15,    11,    25,   -36,   -36,   -36,   -36,    73,    73,
      27,   -36,    24,    31,    59,    52,    70,    73,    77,    81,
     130,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,
     -36,   -36,   551,   -36,   155,   -36,    -9,   617,   -36,   -36,
      73,   180,    88,    73,   103,   650,   -36,   -36,   -36,   -36,
      73,    73,    73,    73,    73,    73,  1005,    73,   -36,   106,
     116,    73,    73,    73,    73,    73,    73,    73,    73,   -36,
     205,   -36,   683,   -36,   230,    73,   716,    57,   -36,    83,
      -9,   101,    43,   -15,   749,   -36,   914,    75,   782,   -36,
     107,   566,   141,   967,   995,   191,    26,   961,   929,   -36,
     104,   -36,   815,   109,   124,    73,   -36,   -36,    73,   -36,
    1028,   255,   112,   280,   110,   584,   914,   -36,    84,   -36,
     305,   330,   -36,   355,    73,   113,    73,   -36,   -36,   -36,
     380,   -36,   848,   405,   881,   -36,   115,   -36,   430,   121,
     455,   -36,   480,   -36,   505,   -36,   530,   -36,   -36
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     2,     0,     1,     3,     0,     8,     0,
       0,     0,     0,     0,    68,    67,    65,    66,     0,     0,
       0,     7,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    10,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,     0,    41,     0,     9,    55,     0,    70,    69,
       0,     0,     0,     0,     0,     0,    39,    40,     6,    11,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     5,
       0,    56,     0,    29,     0,     0,     0,     0,    38,    50,
      51,    52,    53,    54,     0,    60,    63,     0,     0,    58,
       0,    46,    47,    44,    45,    48,    49,    43,    42,     4,
       0,    28,     0,     0,     0,     0,    23,    59,     0,    57,
       0,     0,     0,     0,     0,     0,    64,    62,     0,    25,
       0,     0,    31,     0,     0,     0,     0,    61,    24,    27,
       0,    30,     0,     0,     0,    26,     0,    35,     0,     0,
       0,    34,     0,    33,     0,    37,     0,    32,    36
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -36,   -36,   138,   -36,   -35,   -30,   -36,   -36,   -36,   -36,
     -36,   -36,   -36,   -36,   -36,   -36,    -4,    21,   -36
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     2,     3,    10,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    97,    43
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      59,    62,    63,    64,     5,     8,    69,    70,     1,    80,
      11,     1,    69,    70,    46,    47,    84,     9,    14,    15,
      16,    17,    12,    55,    18,     4,     7,    44,    45,    19,
      13,    20,    21,    22,    60,    61,    62,    63,    64,    23,
      24,    25,    26,    49,    48,    50,    82,    69,    70,    86,
      59,    27,    28,    29,    59,    64,    89,    90,    91,    92,
      93,    94,    96,    98,    69,    70,    53,   101,   102,   103,
     104,   105,   106,   107,   108,    51,    14,    15,    16,    17,
     114,   112,    18,    52,    54,   115,   130,    19,   133,    20,
     117,    22,    61,    62,    63,    64,   140,    56,   118,   137,
      59,    57,    85,    59,    69,    70,    87,   118,   148,    99,
      59,   125,    63,    64,   126,   154,    96,   156,    59,   100,
     121,   120,    69,    70,    59,   123,    59,   124,   131,   143,
     142,   150,   144,    14,    15,    16,    17,   152,   134,    18,
       6,   128,     0,     0,    19,     0,    20,    58,    22,    60,
      61,    62,    63,    64,    23,    24,    25,    26,    14,    15,
      16,    17,    69,    70,    18,     0,    27,    28,    29,    19,
       0,    20,    79,    22,     0,    75,    76,     0,     0,    23,
      24,    25,    26,    14,    15,    16,    17,     0,     0,    18,
       0,    27,    28,    29,    19,     0,    20,    83,    22,    60,
      61,    62,    63,    64,    23,    24,    25,    26,    14,    15,
      16,    17,    69,    70,    18,     0,    27,    28,    29,    19,
       0,    20,   109,    22,     0,     0,    76,     0,     0,    23,
      24,    25,    26,    14,    15,    16,    17,     0,     0,    18,
       0,    27,    28,    29,    19,     0,    20,   111,    22,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    14,    15,
      16,    17,     0,     0,    18,     0,    27,    28,    29,    19,
       0,    20,   129,    22,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    14,    15,    16,    17,     0,     0,    18,
       0,    27,    28,    29,    19,     0,    20,   132,    22,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    14,    15,
      16,    17,     0,     0,    18,     0,    27,    28,    29,    19,
       0,    20,   138,    22,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    14,    15,    16,    17,     0,     0,    18,
       0,    27,    28,    29,    19,     0,    20,   139,    22,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    14,    15,
      16,    17,     0,     0,    18,     0,    27,    28,    29,    19,
       0,    20,   141,    22,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    14,    15,    16,    17,     0,     0,    18,
       0,    27,    28,    29,    19,     0,    20,   145,    22,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    14,    15,
      16,    17,     0,     0,    18,     0,    27,    28,    29,    19,
       0,    20,   147,    22,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    14,    15,    16,    17,     0,     0,    18,
       0,    27,    28,    29,    19,     0,    20,   151,    22,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    14,    15,
      16,    17,     0,     0,    18,     0,    27,    28,    29,    19,
       0,    20,   153,    22,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    14,    15,    16,    17,     0,     0,    18,
       0,    27,    28,    29,    19,     0,    20,   155,    22,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    14,    15,
      16,    17,     0,     0,    18,     0,    27,    28,    29,    19,
       0,    20,   157,    22,     0,     0,     0,     0,     0,    23,
      24,    25,    26,    14,    15,    16,    17,     0,     0,    18,
       0,    27,    28,    29,    19,     0,    20,   158,    22,     0,
       0,     0,     0,     0,    23,    24,    25,    26,     0,    60,
      61,    62,    63,    64,    65,    66,    27,    28,    29,    67,
       0,    68,    69,    70,    60,    61,    62,    63,    64,     0,
       0,    71,    72,    73,    74,    75,    76,    69,    70,     0,
      77,    78,    60,    61,    62,    63,    64,    72,    66,   135,
      75,    76,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,   136,    71,    72,    73,    74,    75,    76,
       0,     0,     0,    77,    78,    60,    61,    62,    63,    64,
       0,    66,    81,     0,     0,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,    71,    72,    73,
      74,    75,    76,     0,     0,     0,    77,    78,    60,    61,
      62,    63,    64,     0,    66,     0,     0,     0,    67,     0,
      88,    69,    70,     0,     0,     0,     0,     0,     0,     0,
      71,    72,    73,    74,    75,    76,     0,     0,     0,    77,
      78,    60,    61,    62,    63,    64,     0,    66,   110,     0,
       0,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,    71,    72,    73,    74,    75,    76,     0,
       0,     0,    77,    78,    60,    61,    62,    63,    64,     0,
      66,   113,     0,     0,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,    71,    72,    73,    74,
      75,    76,     0,     0,     0,    77,    78,    60,    61,    62,
      63,    64,     0,    66,     0,     0,     0,    67,     0,   116,
      69,    70,     0,     0,     0,     0,     0,     0,     0,    71,
      72,    73,    74,    75,    76,     0,     0,     0,    77,    78,
      60,    61,    62,    63,    64,     0,    66,     0,     0,     0,
      67,   119,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,    71,    72,    73,    74,    75,    76,     0,     0,
       0,    77,    78,    60,    61,    62,    63,    64,     0,    66,
     122,     0,     0,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,    71,    72,    73,    74,    75,
      76,     0,     0,     0,    77,    78,    60,    61,    62,    63,
      64,     0,    66,   146,     0,     0,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,    71,    72,
      73,    74,    75,    76,     0,     0,     0,    77,    78,    60,
      61,    62,    63,    64,     0,    66,   149,     0,     0,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,    71,    72,    73,    74,    75,    76,     0,     0,     0,
      77,    78,    60,    61,    62,    63,    64,     0,    66,     0,
       0,     0,    67,     0,     0,    69,    70,    60,    61,    62,
      63,    64,     0,     0,    71,    72,    73,    74,    75,    76,
      69,    70,     0,    77,    78,     0,     0,     0,     0,    71,
      72,    73,    74,    75,    76,     0,     0,     0,    77,    60,
      61,    62,    63,    64,     0,    60,    61,    62,    63,    64,
       0,     0,    69,    70,     0,     0,     0,     0,    69,    70,
       0,    71,    72,    73,    74,    75,    76,    71,    72,     0,
      74,    75,    76,    60,    61,    62,    63,    64,    14,    15,
      16,    17,     0,     0,    18,     0,    69,    70,     0,    19,
      95,    20,     0,    22,     0,    71,    72,     0,     0,    75,
      76,    14,    15,    16,    17,     0,     0,    18,     0,     0,
       0,     0,    19,   127,    20,     0,    22
};

static const yytype_int16 yycheck[] =
{
      30,    10,    11,    12,     0,     3,    21,    22,     7,    44,
      16,     7,    21,    22,    18,    19,    51,    15,     3,     4,
       5,     6,    15,    27,     9,     3,    14,    16,     3,    14,
      23,    16,    17,    18,     8,     9,    10,    11,    12,    24,
      25,    26,    27,    19,    17,    14,    50,    21,    22,    53,
      80,    36,    37,    38,    84,    12,    60,    61,    62,    63,
      64,    65,    66,    67,    21,    22,    14,    71,    72,    73,
      74,    75,    76,    77,    78,    16,     3,     4,     5,     6,
      23,    85,     9,    24,    14,    28,   121,    14,   123,    16,
      15,    18,     9,    10,    11,    12,   131,    20,    23,    15,
     130,    20,    14,   133,    21,    22,     3,    23,   143,     3,
     140,   115,    11,    12,   118,   150,   120,   152,   148,     3,
      16,    14,    21,    22,   154,    16,   156,     3,    16,    16,
     134,    16,   136,     3,     4,     5,     6,    16,    28,     9,
       2,   120,    -1,    -1,    14,    -1,    16,    17,    18,     8,
       9,    10,    11,    12,    24,    25,    26,    27,     3,     4,
       5,     6,    21,    22,     9,    -1,    36,    37,    38,    14,
      -1,    16,    17,    18,    -1,    34,    35,    -1,    -1,    24,
      25,    26,    27,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    36,    37,    38,    14,    -1,    16,    17,    18,     8,
       9,    10,    11,    12,    24,    25,    26,    27,     3,     4,
       5,     6,    21,    22,     9,    -1,    36,    37,    38,    14,
      -1,    16,    17,    18,    -1,    -1,    35,    -1,    -1,    24,
      25,    26,    27,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    36,    37,    38,    14,    -1,    16,    17,    18,    -1,
      -1,    -1,    -1,    -1,    24,    25,    26,    27,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    36,    37,    38,    14,
      -1,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,    24,
      25,    26,    27,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    36,    37,    38,    14,    -1,    16,    17,    18,    -1,
      -1,    -1,    -1,    -1,    24,    25,    26,    27,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    36,    37,    38,    14,
      -1,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,    24,
      25,    26,    27,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    36,    37,    38,    14,    -1,    16,    17,    18,    -1,
      -1,    -1,    -1,    -1,    24,    25,    26,    27,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    36,    37,    38,    14,
      -1,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,    24,
      25,    26,    27,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    36,    37,    38,    14,    -1,    16,    17,    18,    -1,
      -1,    -1,    -1,    -1,    24,    25,    26,    27,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    36,    37,    38,    14,
      -1,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,    24,
      25,    26,    27,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    36,    37,    38,    14,    -1,    16,    17,    18,    -1,
      -1,    -1,    -1,    -1,    24,    25,    26,    27,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    36,    37,    38,    14,
      -1,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,    24,
      25,    26,    27,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    36,    37,    38,    14,    -1,    16,    17,    18,    -1,
      -1,    -1,    -1,    -1,    24,    25,    26,    27,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    36,    37,    38,    14,
      -1,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,    24,
      25,    26,    27,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    36,    37,    38,    14,    -1,    16,    17,    18,    -1,
      -1,    -1,    -1,    -1,    24,    25,    26,    27,    -1,     8,
       9,    10,    11,    12,    13,    14,    36,    37,    38,    18,
      -1,    20,    21,    22,     8,     9,    10,    11,    12,    -1,
      -1,    30,    31,    32,    33,    34,    35,    21,    22,    -1,
      39,    40,     8,     9,    10,    11,    12,    31,    14,    15,
      34,    35,    18,    -1,    -1,    21,    22,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    30,    31,    32,    33,    34,    35,
      -1,    -1,    -1,    39,    40,     8,     9,    10,    11,    12,
      -1,    14,    15,    -1,    -1,    18,    -1,    -1,    21,    22,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    32,
      33,    34,    35,    -1,    -1,    -1,    39,    40,     8,     9,
      10,    11,    12,    -1,    14,    -1,    -1,    -1,    18,    -1,
      20,    21,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    32,    33,    34,    35,    -1,    -1,    -1,    39,
      40,     8,     9,    10,    11,    12,    -1,    14,    15,    -1,
      -1,    18,    -1,    -1,    21,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    32,    33,    34,    35,    -1,
      -1,    -1,    39,    40,     8,     9,    10,    11,    12,    -1,
      14,    15,    -1,    -1,    18,    -1,    -1,    21,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    32,    33,
      34,    35,    -1,    -1,    -1,    39,    40,     8,     9,    10,
      11,    12,    -1,    14,    -1,    -1,    -1,    18,    -1,    20,
      21,    22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    32,    33,    34,    35,    -1,    -1,    -1,    39,    40,
       8,     9,    10,    11,    12,    -1,    14,    -1,    -1,    -1,
      18,    19,    -1,    21,    22,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    32,    33,    34,    35,    -1,    -1,
      -1,    39,    40,     8,     9,    10,    11,    12,    -1,    14,
      15,    -1,    -1,    18,    -1,    -1,    21,    22,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    32,    33,    34,
      35,    -1,    -1,    -1,    39,    40,     8,     9,    10,    11,
      12,    -1,    14,    15,    -1,    -1,    18,    -1,    -1,    21,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      32,    33,    34,    35,    -1,    -1,    -1,    39,    40,     8,
       9,    10,    11,    12,    -1,    14,    15,    -1,    -1,    18,
      -1,    -1,    21,    22,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    32,    33,    34,    35,    -1,    -1,    -1,
      39,    40,     8,     9,    10,    11,    12,    -1,    14,    -1,
      -1,    -1,    18,    -1,    -1,    21,    22,     8,     9,    10,
      11,    12,    -1,    -1,    30,    31,    32,    33,    34,    35,
      21,    22,    -1,    39,    40,    -1,    -1,    -1,    -1,    30,
      31,    32,    33,    34,    35,    -1,    -1,    -1,    39,     8,
       9,    10,    11,    12,    -1,     8,     9,    10,    11,    12,
      -1,    -1,    21,    22,    -1,    -1,    -1,    -1,    21,    22,
      -1,    30,    31,    32,    33,    34,    35,    30,    31,    -1,
      33,    34,    35,     8,     9,    10,    11,    12,     3,     4,
       5,     6,    -1,    -1,     9,    -1,    21,    22,    -1,    14,
      15,    16,    -1,    18,    -1,    30,    31,    -1,    -1,    34,
      35,     3,     4,     5,     6,    -1,    -1,     9,    -1,    -1,
      -1,    -1,    14,    15,    16,    -1,    18
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     7,    42,    43,     3,     0,    43,    14,     3,    15,
      44,    16,    15,    23,     3,     4,     5,     6,     9,    14,
      16,    17,    18,    24,    25,    26,    27,    36,    37,    38,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    59,    16,     3,    57,    57,    17,    19,
      14,    16,    24,    14,    14,    57,    20,    20,    17,    46,
       8,     9,    10,    11,    12,    13,    14,    18,    20,    21,
      22,    30,    31,    32,    33,    34,    35,    39,    40,    17,
      45,    15,    57,    17,    45,    14,    57,     3,    20,    57,
      57,    57,    57,    57,    57,    15,    57,    58,    57,     3,
       3,    57,    57,    57,    57,    57,    57,    57,    57,    17,
      15,    17,    57,    15,    23,    28,    20,    15,    23,    19,
      14,    16,    15,    16,     3,    57,    57,    15,    58,    17,
      45,    16,    17,    45,    28,    15,    29,    15,    17,    17,
      45,    17,    57,    16,    57,    17,    15,    17,    45,    15,
      16,    17,    16,    17,    45,    17,    45,    17,    17
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    41,    42,    42,    43,    43,    43,    43,    44,    44,
      45,    45,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    47,    48,    49,    49,    50,    50,    51,    51,
      52,    52,    53,    53,    53,    53,    53,    53,    54,    55,
      56,    57,    57,    57,    57,    57,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    57,    57,
      57,    57,    57,    58,    58,    59,    59,    59,    59,    59,
      59
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     8,     7,     7,     6,     1,     3,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     4,     7,     6,     8,     7,     4,     3,
       7,     6,    11,    10,     9,     8,    11,    10,     3,     2,
       2,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     4,     3,     4,
       3,     6,     5,     1,     3,     1,     1,     1,     1,     2,
       2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (scanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (scanner);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, scanner);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, void *scanner)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, void *scanner)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */


/* User initialization code.  */
#line 101 "../src/linguine/parser.y"
{
	ast_yylloc.last_line = yylloc.first_line = 0;
	ast_yylloc.last_column = yylloc.first_column = 0;
}

#line 1345 "../src/linguine/parser.tab.c"

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* func_list: func  */
#line 108 "../src/linguine/parser.y"
                {
			(yyval.func_list) = ast_accept_func_list(NULL, (yyvsp[0].func));
			debug("func_list: class");
		}
#line 1561 "../src/linguine/parser.tab.c"
    break;

  case 3: /* func_list: func_list func  */
#line 113 "../src/linguine/parser.y"
                {
			(yyval.func_list) = ast_accept_func_list((yyvsp[-1].func_list), (yyvsp[0].func));
			debug("func_list: func_list func");
		}
#line 1570 "../src/linguine/parser.tab.c"
    break;

  case 4: /* func: TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR param_list TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 119 "../src/linguine/parser.y"
                {
			(yyval.func) = ast_accept_func((yyvsp[-6].sval), (yyvsp[-4].param_list), (yyvsp[-1].stmt_list));
			debug("func: func name(param_list) { stmt_list }");
		}
#line 1579 "../src/linguine/parser.tab.c"
    break;

  case 5: /* func: TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR param_list TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 124 "../src/linguine/parser.y"
                {
			(yyval.func) = ast_accept_func((yyvsp[-5].sval), (yyvsp[-3].param_list), NULL);
			debug("func: func name(param_list) { empty }");
		}
#line 1588 "../src/linguine/parser.tab.c"
    break;

  case 6: /* func: TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 129 "../src/linguine/parser.y"
                {
			(yyval.func) = ast_accept_func((yyvsp[-5].sval), NULL, (yyvsp[-1].stmt_list));
			debug("func: func name() { stmt_list }");
		}
#line 1597 "../src/linguine/parser.tab.c"
    break;

  case 7: /* func: TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 134 "../src/linguine/parser.y"
                {
			(yyval.func) = ast_accept_func((yyvsp[-4].sval), NULL, NULL);
			debug("func: func name() { empty }");
		}
#line 1606 "../src/linguine/parser.tab.c"
    break;

  case 8: /* param_list: TOKEN_SYMBOL  */
#line 140 "../src/linguine/parser.y"
                {
			(yyval.param_list) = ast_accept_param_list(NULL, (yyvsp[0].sval));
			debug("param_list: type symbol");
		}
#line 1615 "../src/linguine/parser.tab.c"
    break;

  case 9: /* param_list: param_list TOKEN_COMMA TOKEN_SYMBOL  */
#line 145 "../src/linguine/parser.y"
                {
			(yyval.param_list) = ast_accept_param_list((yyvsp[-2].param_list), (yyvsp[0].sval));
			debug("param_list: param_list type symbol");
		}
#line 1624 "../src/linguine/parser.tab.c"
    break;

  case 10: /* stmt_list: stmt  */
#line 151 "../src/linguine/parser.y"
                {
			(yyval.stmt_list) = ast_accept_stmt_list(NULL, (yyvsp[0].stmt));
			debug("stmt_list: stmt");
		}
#line 1633 "../src/linguine/parser.tab.c"
    break;

  case 11: /* stmt_list: stmt_list stmt  */
#line 156 "../src/linguine/parser.y"
                {
			(yyval.stmt_list) = ast_accept_stmt_list((yyvsp[-1].stmt_list), (yyvsp[0].stmt));
			debug("stmt_list: stmt_list stmt");
		}
#line 1642 "../src/linguine/parser.tab.c"
    break;

  case 12: /* stmt: expr_stmt  */
#line 162 "../src/linguine/parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			ast_accept_stmt((yyvsp[0].stmt), ast_yylloc.first_line + 1);
			debug("stmt: expr_stmt");
		}
#line 1652 "../src/linguine/parser.tab.c"
    break;

  case 13: /* stmt: assign_stmt  */
#line 168 "../src/linguine/parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			ast_accept_stmt((yyvsp[0].stmt), ast_yylloc.first_line + 1);
			debug("stmt: assign_stmt");
		}
#line 1662 "../src/linguine/parser.tab.c"
    break;

  case 14: /* stmt: if_stmt  */
#line 174 "../src/linguine/parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			ast_accept_stmt((yyvsp[0].stmt), ast_yylloc.first_line + 1);
			debug("stmt: if_stmt");
		}
#line 1672 "../src/linguine/parser.tab.c"
    break;

  case 15: /* stmt: elif_stmt  */
#line 180 "../src/linguine/parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			ast_accept_stmt((yyvsp[0].stmt), ast_yylloc.first_line + 1);
			debug("stmt: elif_stmt");
		}
#line 1682 "../src/linguine/parser.tab.c"
    break;

  case 16: /* stmt: else_stmt  */
#line 186 "../src/linguine/parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			ast_accept_stmt((yyvsp[0].stmt), ast_yylloc.first_line + 1);
			debug("stmt: else_stmt");
		}
#line 1692 "../src/linguine/parser.tab.c"
    break;

  case 17: /* stmt: while_stmt  */
#line 192 "../src/linguine/parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			ast_accept_stmt((yyvsp[0].stmt), ast_yylloc.first_line + 1);
			debug("stmt: while_stmt");
		}
#line 1702 "../src/linguine/parser.tab.c"
    break;

  case 18: /* stmt: for_stmt  */
#line 198 "../src/linguine/parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			ast_accept_stmt((yyvsp[0].stmt), ast_yylloc.first_line + 1);
			debug("stmt: for_stmt");
		}
#line 1712 "../src/linguine/parser.tab.c"
    break;

  case 19: /* stmt: return_stmt  */
#line 204 "../src/linguine/parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			ast_accept_stmt((yyvsp[0].stmt), ast_yylloc.first_line + 1);
			debug("stmt: return_stmt");
		}
#line 1722 "../src/linguine/parser.tab.c"
    break;

  case 20: /* stmt: break_stmt  */
#line 210 "../src/linguine/parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			ast_accept_stmt((yyvsp[0].stmt), ast_yylloc.first_line + 1);
			debug("stmt: break_stmt");
		}
#line 1732 "../src/linguine/parser.tab.c"
    break;

  case 21: /* stmt: continue_stmt  */
#line 216 "../src/linguine/parser.y"
                {
			(yyval.stmt) = (yyvsp[0].stmt);
			ast_accept_stmt((yyvsp[0].stmt), ast_yylloc.first_line + 1);
			debug("stmt: continue_stmt");
		}
#line 1742 "../src/linguine/parser.tab.c"
    break;

  case 22: /* expr_stmt: expr TOKEN_SEMICOLON  */
#line 223 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_expr_stmt((yyvsp[-1].expr));
			debug("expr_stmt");
		}
#line 1751 "../src/linguine/parser.tab.c"
    break;

  case 23: /* assign_stmt: expr TOKEN_ASSIGN expr TOKEN_SEMICOLON  */
#line 229 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_assign_stmt((yyvsp[-3].expr), (yyvsp[-1].expr));
			debug("assign_stmt");
		}
#line 1760 "../src/linguine/parser.tab.c"
    break;

  case 24: /* if_stmt: TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 235 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_if_stmt((yyvsp[-4].expr), (yyvsp[-1].stmt_list));
			debug("if_stmt: stmt_list");
		}
#line 1769 "../src/linguine/parser.tab.c"
    break;

  case 25: /* if_stmt: TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 240 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_if_stmt((yyvsp[-3].expr), NULL);
			debug("if_stmt: empty");
		}
#line 1778 "../src/linguine/parser.tab.c"
    break;

  case 26: /* elif_stmt: TOKEN_ELSE TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 246 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_elif_stmt((yyvsp[-4].expr), (yyvsp[-1].stmt_list));
			debug("elif_stmt: stmt_list");
		}
#line 1787 "../src/linguine/parser.tab.c"
    break;

  case 27: /* elif_stmt: TOKEN_ELSE TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 251 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_elif_stmt((yyvsp[-3].expr), NULL);
			debug("elif_stmt: empty");
		}
#line 1796 "../src/linguine/parser.tab.c"
    break;

  case 28: /* else_stmt: TOKEN_ELSE TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 257 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_else_stmt((yyvsp[-1].stmt_list));
			debug("else_stmt: stmt_list");
		}
#line 1805 "../src/linguine/parser.tab.c"
    break;

  case 29: /* else_stmt: TOKEN_ELSE TOKEN_LBLK TOKEN_RBLK  */
#line 262 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_else_stmt(NULL);
			debug("else_stmt: empty");
		}
#line 1814 "../src/linguine/parser.tab.c"
    break;

  case 30: /* while_stmt: TOKEN_WHILE TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 268 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_while_stmt((yyvsp[-4].expr), (yyvsp[-1].stmt_list));
			debug("while_stmt: stmt_list");
		}
#line 1823 "../src/linguine/parser.tab.c"
    break;

  case 31: /* while_stmt: TOKEN_WHILE TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 273 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_while_stmt((yyvsp[-3].expr), NULL);
			debug("while_stmt: empty");
		}
#line 1832 "../src/linguine/parser.tab.c"
    break;

  case 32: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_COMMA TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 279 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_for_kv_stmt((yyvsp[-8].sval), (yyvsp[-6].sval), (yyvsp[-4].expr), (yyvsp[-1].stmt_list));
			debug("for_stmt: for(k, v in array) { stmt_list }");
		}
#line 1841 "../src/linguine/parser.tab.c"
    break;

  case 33: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_COMMA TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 284 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_for_kv_stmt((yyvsp[-7].sval), (yyvsp[-5].sval), (yyvsp[-3].expr), NULL);
			debug("for_stmt: for(k, v in array) { empty }");
		}
#line 1850 "../src/linguine/parser.tab.c"
    break;

  case 34: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 289 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_for_v_stmt((yyvsp[-6].sval), (yyvsp[-4].expr), (yyvsp[-1].stmt_list));
			debug("for_stmt: for(v in array) { stmt_list }");
		}
#line 1859 "../src/linguine/parser.tab.c"
    break;

  case 35: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 294 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_for_v_stmt((yyvsp[-5].sval), (yyvsp[-3].expr), NULL);
			debug("for_stmt: for(v in array) { empty }");
		}
#line 1868 "../src/linguine/parser.tab.c"
    break;

  case 36: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_DOTDOT expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK  */
#line 299 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_for_range_stmt((yyvsp[-8].sval), (yyvsp[-6].expr), (yyvsp[-4].expr), (yyvsp[-1].stmt_list));
			debug("for_stmt: for(i in x..y) { stmt_list }");
		}
#line 1877 "../src/linguine/parser.tab.c"
    break;

  case 37: /* for_stmt: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_DOTDOT expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK  */
#line 304 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_for_range_stmt((yyvsp[-7].sval), (yyvsp[-5].expr), (yyvsp[-3].expr), NULL);
			debug("for_stmt: for(i in x..y) { empty}");
		}
#line 1886 "../src/linguine/parser.tab.c"
    break;

  case 38: /* return_stmt: TOKEN_RETURN expr TOKEN_SEMICOLON  */
#line 310 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_return_stmt((yyvsp[-1].expr));
			debug("rerurn_stmt:");
		}
#line 1895 "../src/linguine/parser.tab.c"
    break;

  case 39: /* break_stmt: TOKEN_BREAK TOKEN_SEMICOLON  */
#line 316 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_break_stmt();
			debug("break_stmt:");
		}
#line 1904 "../src/linguine/parser.tab.c"
    break;

  case 40: /* continue_stmt: TOKEN_CONTINUE TOKEN_SEMICOLON  */
#line 322 "../src/linguine/parser.y"
                {
			(yyval.stmt) = ast_accept_continue_stmt();
			debug("continue_stmt");
		}
#line 1913 "../src/linguine/parser.tab.c"
    break;

  case 41: /* expr: term  */
#line 328 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_term_expr((yyvsp[0].term));
			debug("expr: term");
		}
#line 1922 "../src/linguine/parser.tab.c"
    break;

  case 42: /* expr: expr TOKEN_OR expr  */
#line 333 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_or_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr or expr");
		}
#line 1931 "../src/linguine/parser.tab.c"
    break;

  case 43: /* expr: expr TOKEN_AND expr  */
#line 338 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_and_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr and expr");
		}
#line 1940 "../src/linguine/parser.tab.c"
    break;

  case 44: /* expr: expr TOKEN_LT expr  */
#line 343 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_lt_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr lt expr");
		}
#line 1949 "../src/linguine/parser.tab.c"
    break;

  case 45: /* expr: expr TOKEN_LTE expr  */
#line 348 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_lte_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr lte expr");
		}
#line 1958 "../src/linguine/parser.tab.c"
    break;

  case 46: /* expr: expr TOKEN_GT expr  */
#line 353 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_gt_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr gt expr");
		}
#line 1967 "../src/linguine/parser.tab.c"
    break;

  case 47: /* expr: expr TOKEN_GTE expr  */
#line 358 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_gte_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr gte expr");
		}
#line 1976 "../src/linguine/parser.tab.c"
    break;

  case 48: /* expr: expr TOKEN_EQ expr  */
#line 363 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_eq_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr eq expr");
		}
#line 1985 "../src/linguine/parser.tab.c"
    break;

  case 49: /* expr: expr TOKEN_NEQ expr  */
#line 368 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_neq_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr neq expr");
		}
#line 1994 "../src/linguine/parser.tab.c"
    break;

  case 50: /* expr: expr TOKEN_PLUS expr  */
#line 373 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_plus_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr plus expr");
		}
#line 2003 "../src/linguine/parser.tab.c"
    break;

  case 51: /* expr: expr TOKEN_MINUS expr  */
#line 378 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_minus_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr sub expr");
		}
#line 2012 "../src/linguine/parser.tab.c"
    break;

  case 52: /* expr: expr TOKEN_MUL expr  */
#line 383 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_mul_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr mul expr");
		}
#line 2021 "../src/linguine/parser.tab.c"
    break;

  case 53: /* expr: expr TOKEN_DIV expr  */
#line 388 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_div_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr div expr");
		}
#line 2030 "../src/linguine/parser.tab.c"
    break;

  case 54: /* expr: expr TOKEN_MOD expr  */
#line 393 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_mod_expr((yyvsp[-2].expr), (yyvsp[0].expr));
			debug("expr: expr div expr");
		}
#line 2039 "../src/linguine/parser.tab.c"
    break;

  case 55: /* expr: TOKEN_MINUS expr  */
#line 398 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_neg_expr((yyvsp[0].expr));
			debug("expr: neg expr");
		}
#line 2048 "../src/linguine/parser.tab.c"
    break;

  case 56: /* expr: TOKEN_LPAR expr TOKEN_RPAR  */
#line 403 "../src/linguine/parser.y"
                {
			(yyval.expr) = (yyvsp[-1].expr);
			debug("expr: (expr)");
		}
#line 2057 "../src/linguine/parser.tab.c"
    break;

  case 57: /* expr: expr TOKEN_LARR expr TOKEN_RARR  */
#line 408 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_subscr_expr((yyvsp[-3].expr), (yyvsp[-1].expr));
			debug("expr: array[subscript]");
		}
#line 2066 "../src/linguine/parser.tab.c"
    break;

  case 58: /* expr: expr TOKEN_DOT TOKEN_SYMBOL  */
#line 413 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_dot_expr((yyvsp[-2].expr), (yyvsp[0].sval));
			debug("expr: expr.symbol");
		}
#line 2075 "../src/linguine/parser.tab.c"
    break;

  case 59: /* expr: expr TOKEN_LPAR arg_list TOKEN_RPAR  */
#line 418 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_call_expr((yyvsp[-3].expr), (yyvsp[-1].arg_list));
			debug("expr: call(param_list)");
		}
#line 2084 "../src/linguine/parser.tab.c"
    break;

  case 60: /* expr: expr TOKEN_LPAR TOKEN_RPAR  */
#line 423 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_call_expr((yyvsp[-2].expr), NULL);
			debug("expr: call()");
		}
#line 2093 "../src/linguine/parser.tab.c"
    break;

  case 61: /* expr: expr TOKEN_ARROW TOKEN_SYMBOL TOKEN_LPAR arg_list TOKEN_RPAR  */
#line 428 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_thiscall_expr((yyvsp[-5].expr), (yyvsp[-3].sval), (yyvsp[-1].arg_list));
			debug("expr: thiscall(param_list)");
		}
#line 2102 "../src/linguine/parser.tab.c"
    break;

  case 62: /* expr: expr TOKEN_ARROW TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR  */
#line 433 "../src/linguine/parser.y"
                {
			(yyval.expr) = ast_accept_thiscall_expr((yyvsp[-4].expr), (yyvsp[-2].sval), NULL);
			debug("expr: thiscall(param_list)");
		}
#line 2111 "../src/linguine/parser.tab.c"
    break;

  case 63: /* arg_list: expr  */
#line 439 "../src/linguine/parser.y"
                {
			(yyval.arg_list) = ast_accept_arg_list(NULL, (yyvsp[0].expr));
			debug("arg_list: expr");
		}
#line 2120 "../src/linguine/parser.tab.c"
    break;

  case 64: /* arg_list: arg_list TOKEN_COMMA expr  */
#line 444 "../src/linguine/parser.y"
                {
			(yyval.arg_list) = ast_accept_arg_list((yyvsp[-2].arg_list), (yyvsp[0].expr));
			debug("arg_list: arg_list param_list");
		}
#line 2129 "../src/linguine/parser.tab.c"
    break;

  case 65: /* term: TOKEN_INT  */
#line 450 "../src/linguine/parser.y"
                {
			(yyval.term) = ast_accept_int_term((yyvsp[0].ival));
			debug("term: int");
		}
#line 2138 "../src/linguine/parser.tab.c"
    break;

  case 66: /* term: TOKEN_FLOAT  */
#line 455 "../src/linguine/parser.y"
                {
			(yyval.term) = ast_accept_float_term((yyvsp[0].fval));
			debug("term: float");
		}
#line 2147 "../src/linguine/parser.tab.c"
    break;

  case 67: /* term: TOKEN_STR  */
#line 460 "../src/linguine/parser.y"
                {
			(yyval.term) = ast_accept_str_term((yyvsp[0].sval));
			debug("term: string");
		}
#line 2156 "../src/linguine/parser.tab.c"
    break;

  case 68: /* term: TOKEN_SYMBOL  */
#line 465 "../src/linguine/parser.y"
                {
			(yyval.term) = ast_accept_symbol_term((yyvsp[0].sval));
			debug("term: symbol");
		}
#line 2165 "../src/linguine/parser.tab.c"
    break;

  case 69: /* term: TOKEN_LARR TOKEN_RARR  */
#line 470 "../src/linguine/parser.y"
                {
			(yyval.term) = ast_accept_empty_array_term();
			debug("term: empty array symbol");
		}
#line 2174 "../src/linguine/parser.tab.c"
    break;

  case 70: /* term: TOKEN_LBLK TOKEN_RBLK  */
#line 475 "../src/linguine/parser.y"
                {
			(yyval.term) = ast_accept_empty_dict_term();
			debug("term: empty dict symbol");
		}
#line 2183 "../src/linguine/parser.tab.c"
    break;


#line 2187 "../src/linguine/parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (scanner, YY_("syntax error"));
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, scanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 480 "../src/linguine/parser.y"


#ifdef DEBUG
static void print_debug(const char *s)
{
	fprintf(stderr, "%s\n", s);
}
#endif

void ast_yyerror(void *scanner, char *s)
{
	extern int ast_error_line;
	extern int ast_error_column;
	extern char *ast_error_message;

	(void)scanner;
	(void)s;

	ast_error_line = ast_yylloc.last_line + 1;
	ast_error_column = ast_yylloc.last_column + 1;
	if (s != NULL)
		strcpy(ast_error_message, s);
	else
		strcpy(ast_error_message, "");
}
