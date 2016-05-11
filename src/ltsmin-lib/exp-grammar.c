/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 1 "exp-grammar.lemon"

#include <hre/config.h>
#include <assert.h>
#include <stdlib.h>

#include <hre-io/stream.h>
#include <hre/user.h>
#include <hre/stringindex.h>
#include <util-lib/chunk_support.h>
#include <ltsmin-lib/exp-parse-env.h>
#include <ltsmin-lib/exp-syntax.h>
#include <ltsmin-lib/exp-grammar.h>
#line 21 "exp-grammar.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    expParseTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is expParseTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    expParseARG_SDECL     A static variable declaration for the %extra_argument
**    expParseARG_PDECL     A parameter declaration for the %extra_argument
**    expParseARG_STORE     Code to store %extra_argument into yypParser
**    expParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 32
#define YYACTIONTYPE unsigned char
#define expParseTOKENTYPE  int 
typedef union {
  int yyinit;
  expParseTOKENTYPE yy0;
  char** yy1;
  sync_action_number_t yy28;
  char* yy29;
  list_t yy38;
  exp_model_t yy39;
  transition_t yy47;
  int yy63;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define expParseARG_SDECL  exp_parse_env_t env ;
#define expParseARG_PDECL , exp_parse_env_t env 
#define expParseARG_FETCH  exp_parse_env_t env  = yypParser->env 
#define expParseARG_STORE yypParser->env  = env 
#define YYNSTATE 62
#define YYNRULE 25
#define YYERRORSYMBOL 19
#define YYERRSYMDT yy63
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    20,    6,   47,   16,   48,   22,   60,   18,   59,   56,
 /*    10 */    46,   47,   88,   19,    6,   47,   29,   49,   22,   13,
 /*    20 */    47,   43,   49,   46,   15,   47,    3,   49,    1,   17,
 /*    30 */    58,   54,   18,   59,   57,   37,   12,    2,   37,   21,
 /*    40 */    61,   31,   34,   10,   39,    4,   50,   40,   62,    9,
 /*    50 */    63,    5,    7,   42,   23,   41,    8,    7,   25,   38,
 /*    60 */     7,   24,   26,   89,   27,   28,   11,   30,   44,   89,
 /*    70 */    14,   32,   35,   33,   36,   52,   51,   45,   53,   89,
 /*    80 */    89,   55,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     1,    2,   19,   24,   21,    6,   27,   28,   29,   29,
 /*    10 */    11,   19,   20,   21,    2,   19,    7,   21,    6,   23,
 /*    20 */    19,   30,   21,   11,   23,   19,    3,   21,    5,   23,
 /*    30 */    12,   27,   28,   29,   16,   12,   22,    3,   12,    4,
 /*    40 */    26,   11,   12,    9,    4,    3,   26,    4,    0,   25,
 /*    50 */     0,    9,   17,    2,    7,   14,   15,   17,    9,   13,
 /*    60 */    17,    8,    8,   31,    9,    8,   10,    8,   10,   31,
 /*    70 */     9,    9,    9,    8,    8,    2,    8,   10,    2,   31,
 /*    80 */    31,   12,
};
#define YY_SHIFT_USE_DFLT (-2)
#define YY_SHIFT_MAX 41
static const signed char yy_shift_ofst[] = {
 /*     0 */    -1,   18,   12,   12,   12,   18,   23,   12,   18,    9,
 /*    10 */    26,   -2,   34,   35,   30,   40,   42,   43,   41,   48,
 /*    20 */    50,   51,   47,   53,   49,   54,   55,   57,   56,   59,
 /*    30 */    61,   62,   65,   58,   63,   66,   67,   46,   68,   73,
 /*    40 */    76,   69,
};
#define YY_REDUCE_USE_DFLT (-22)
#define YY_REDUCE_MAX 11
static const signed char yy_reduce_ofst[] = {
 /*     0 */    -8,  -21,   -4,    1,    6,    4,   14,  -17,  -20,   -9,
 /*    10 */    20,   24,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */    87,   87,   87,   87,   87,   87,   87,   87,   87,   67,
 /*    10 */    87,   84,   87,   87,   87,   87,   87,   87,   87,   87,
 /*    20 */    87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
 /*    30 */    87,   87,   87,   87,   87,   87,   87,   73,   87,   87,
 /*    40 */    87,   87,   64,   83,   85,   86,   68,   69,   81,   82,
 /*    50 */    70,   72,   65,   66,   74,   76,   77,   79,   80,   78,
 /*    60 */    75,   71,
};
#define YY_SZ_ACTTAB (int)(sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  expParseARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void expParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "ERROR",         "PAR",           "IN",          
  "END",           "USING",         "DES",           "LPAR",        
  "NUMBER",        "COMMA",         "RPAR",          "STRING",      
  "ACTION",        "HASH",          "ARROW",         "STAR",        
  "UNDERSCORE",    "BARS",          "USER",          "error",       
  "input",         "exp_spec",      "sync_actions",  "proc_section",
  "sync_vectors",  "trans_list",    "sync_action",   "sync_rule",   
  "actions",       "action",        "trans",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "input ::= exp_spec",
 /*   1 */ "input ::= ERROR",
 /*   2 */ "exp_spec ::= PAR sync_actions IN proc_section END PAR",
 /*   3 */ "exp_spec ::= PAR IN proc_section END PAR",
 /*   4 */ "exp_spec ::= PAR USING sync_vectors IN proc_section END PAR",
 /*   5 */ "exp_spec ::= DES LPAR NUMBER COMMA NUMBER COMMA NUMBER RPAR trans_list",
 /*   6 */ "exp_spec ::= STRING",
 /*   7 */ "exp_spec ::= error",
 /*   8 */ "sync_actions ::= sync_actions COMMA sync_action",
 /*   9 */ "sync_actions ::= sync_action",
 /*  10 */ "sync_action ::= ACTION HASH NUMBER",
 /*  11 */ "sync_action ::= ACTION",
 /*  12 */ "sync_vectors ::= sync_vectors COMMA sync_rule",
 /*  13 */ "sync_vectors ::= sync_rule",
 /*  14 */ "sync_rule ::= actions ARROW ACTION",
 /*  15 */ "actions ::= actions STAR action",
 /*  16 */ "actions ::= action",
 /*  17 */ "action ::= UNDERSCORE",
 /*  18 */ "action ::= ACTION",
 /*  19 */ "proc_section ::= proc_section BARS exp_spec",
 /*  20 */ "proc_section ::= exp_spec",
 /*  21 */ "trans_list ::= trans_list trans",
 /*  22 */ "trans_list ::=",
 /*  23 */ "trans ::= LPAR NUMBER COMMA STRING COMMA NUMBER RPAR",
 /*  24 */ "trans ::= LPAR NUMBER COMMA ACTION COMMA NUMBER RPAR",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to expParse and expParseFree.
*/
void *expParseAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  expParseARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    case 21: /* exp_spec */
{
#line 200 "exp-grammar.lemon"

    if((yypminor->yy39)->sync_rules)
        RTfree((yypminor->yy39)->sync_rules);
    if((yypminor->yy39)->processes)
        RTfree((yypminor->yy39)->processes);
    if((yypminor->yy39)->transitions)
        RTfree((yypminor->yy39)->transitions);

#line 420 "exp-grammar.c"
}
      break;
    case 22: /* sync_actions */
    case 23: /* proc_section */
    case 24: /* sync_vectors */
    case 25: /* trans_list */
    case 28: /* actions */
{
#line 210 "exp-grammar.lemon"
 expListFree((yypminor->yy38)); 
#line 431 "exp-grammar.c"
}
      break;
    case 26: /* sync_action */
{
#line 212 "exp-grammar.lemon"
 RTfree((yypminor->yy28)); 
#line 438 "exp-grammar.c"
}
      break;
    case 27: /* sync_rule */
{
#line 216 "exp-grammar.lemon"
 RTfree((yypminor->yy1)); 
#line 445 "exp-grammar.c"
}
      break;
    case 29: /* action */
{
#line 220 "exp-grammar.lemon"
 if((yypminor->yy29)) {RTfree((yypminor->yy29));} 
#line 452 "exp-grammar.c"
}
      break;
    case 30: /* trans */
{
#line 227 "exp-grammar.lemon"
 RTfree((yypminor->yy47)->label); RTfree((yypminor->yy47)); 
#line 459 "exp-grammar.c"
}
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from expParseAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void expParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int expParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_MAX || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( j>=0 && j<YY_SZ_ACTTAB && yy_lookahead[j]==YYWILDCARD ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_MAX ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_MAX );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_SZ_ACTTAB );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   expParseARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
#line 34 "exp-grammar.lemon"

    (void)yypMinor;
    Abort("stack overflow");
#line 636 "exp-grammar.c"
   expParseARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 20, 1 },
  { 20, 1 },
  { 21, 6 },
  { 21, 5 },
  { 21, 7 },
  { 21, 9 },
  { 21, 1 },
  { 21, 1 },
  { 22, 3 },
  { 22, 1 },
  { 26, 3 },
  { 26, 1 },
  { 24, 3 },
  { 24, 1 },
  { 27, 3 },
  { 28, 3 },
  { 28, 1 },
  { 29, 1 },
  { 29, 1 },
  { 23, 3 },
  { 23, 1 },
  { 25, 2 },
  { 25, 0 },
  { 30, 7 },
  { 30, 7 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  expParseARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0: /* input ::= exp_spec */
#line 44 "exp-grammar.lemon"
{
    Warning(infoLong,"parsing finished");
    env->model = yymsp[0].minor.yy39;
    Warning(debug,"EXP model has %d processes", yymsp[0].minor.yy39->num_processes);
    Warning(debug,"EXP model has %d syncronization rules", yymsp[0].minor.yy39->num_sync_rules);
}
#line 780 "exp-grammar.c"
        break;
      case 1: /* input ::= ERROR */
#line 50 "exp-grammar.lemon"
{
    Abort("The error token is meant to give the lexer a way of passing the error.");
}
#line 787 "exp-grammar.c"
        break;
      case 2: /* exp_spec ::= PAR sync_actions IN proc_section END PAR */
#line 54 "exp-grammar.lemon"
{
    yygotominor.yy39 = RTmalloc(sizeof(struct exp_model_s));
    yygotominor.yy39->num_processes = expListLength(yymsp[-2].minor.yy38);
    yygotominor.yy39->processes = (struct exp_model_s*) expListToArray(yymsp[-2].minor.yy38, sizeof(struct exp_model_s));
    yygotominor.yy39->sync_rules = exp_sync_actions_to_rules(yymsp[-4].minor.yy38, yygotominor.yy39);
    exp_collect_action_labels(yygotominor.yy39);
    exp_collect_gates(yygotominor.yy39);
}
#line 799 "exp-grammar.c"
        break;
      case 3: /* exp_spec ::= PAR IN proc_section END PAR */
#line 63 "exp-grammar.lemon"
{
    yygotominor.yy39 = RTmalloc(sizeof(struct exp_model_s));
    yygotominor.yy39->num_processes = expListLength(yymsp[-2].minor.yy38);
    yygotominor.yy39->processes = (struct exp_model_s*) expListToArray(yymsp[-2].minor.yy38, sizeof(struct exp_model_s));
    yygotominor.yy39->sync_rules = RTmallocZero(sizeof(char**));
    yygotominor.yy39->num_sync_rules = 0;
    exp_collect_action_labels(yygotominor.yy39);
    exp_collect_gates(yygotominor.yy39);
}
#line 812 "exp-grammar.c"
        break;
      case 4: /* exp_spec ::= PAR USING sync_vectors IN proc_section END PAR */
#line 73 "exp-grammar.lemon"
{
    yygotominor.yy39 = RTmalloc(sizeof(struct exp_model_s));
    yygotominor.yy39->num_processes = expListLength(yymsp[-2].minor.yy38);
    yygotominor.yy39->processes = (struct exp_model_s*) expListToArray(yymsp[-2].minor.yy38, sizeof(struct exp_model_s));
    yygotominor.yy39->num_sync_rules = expListLength(yymsp[-4].minor.yy38);
    yygotominor.yy39->sync_rules = (char***) expListToArray(yymsp[-4].minor.yy38, sizeof(char**));
    exp_collect_action_labels(yygotominor.yy39);
    exp_collect_gates(yygotominor.yy39);
}
#line 825 "exp-grammar.c"
        break;
      case 5: /* exp_spec ::= DES LPAR NUMBER COMMA NUMBER COMMA NUMBER RPAR trans_list */
#line 82 "exp-grammar.lemon"
{
    exp_trans_t* transitions = RTmallocZero(sizeof(exp_trans_t) * yymsp[-2].minor.yy0);
    list_t current = yymsp[0].minor.yy38;
    string_index_t action_labels = SIcreate();
    while(current) {
        transition_t current_trans = current->item;
        if(transitions[current_trans->source] == NULL) {
            transitions[current_trans->source] = RTmallocZero(sizeof(struct exp_trans_s));
        }
        int match_found = 0;
        for(int i = 0; i < transitions[current_trans->source]->num; i++) {
            if(transitions[current_trans->source]->dest[i] == current_trans->destination) {
                // Extend the string
                char* appended_label = RTmalloc(sizeof(char) * strlen(transitions[current_trans->source]->label[i]) + 2 + strlen(current_trans->label));
                sprintf(appended_label, "%s\x1f%s", transitions[current_trans->source]->label[i], current_trans->label);
                if(strchr(transitions[current_trans->source]->label[i], '\x1f')) {
                    // This is an appended string, so it can be freed
                    RTfree(transitions[current_trans->source]->label[i]);
                }
                transitions[current_trans->source]->label[i] = appended_label;
                match_found = 1;
                break;
            }
        }
        if(!match_found) {
            transitions[current_trans->source]->label = RTrealloc(transitions[current_trans->source]->label, (transitions[current_trans->source]->num+1) * sizeof(char*));
            transitions[current_trans->source]->dest = RTrealloc(transitions[current_trans->source]->dest, (transitions[current_trans->source]->num+1) * sizeof(int));
            transitions[current_trans->source]->label[transitions[current_trans->source]->num] = current_trans->label;
            transitions[current_trans->source]->dest[transitions[current_trans->source]->num] = current_trans->destination;
            transitions[current_trans->source]->num++;
        }
        SIput(action_labels, current_trans->label);
        list_t prev = current->prev;
        RTfree(current);
        current = prev;
    }
    yygotominor.yy39 = RTmalloc(sizeof(struct exp_model_s));
    yygotominor.yy39->num_processes = 0;
    yygotominor.yy39->num_sync_rules = 0;
    yygotominor.yy39->action_labels = action_labels;
    yygotominor.yy39->process_states = yymsp[-2].minor.yy0;
    yygotominor.yy39->process_transitions = yymsp[-4].minor.yy0;
    yygotominor.yy39->initial_state = yymsp[-6].minor.yy0;
    yygotominor.yy39->transitions = transitions;
    exp_collect_gates(yygotominor.yy39);

}
#line 876 "exp-grammar.c"
        break;
      case 6: /* exp_spec ::= STRING */
#line 129 "exp-grammar.lemon"
{
    yygotominor.yy39 = exp_recur_parse_stream(env->scanner, SIget( env->strings, yymsp[0].minor.yy0) );
}
#line 883 "exp-grammar.c"
        break;
      case 7: /* exp_spec ::= error */
#line 133 "exp-grammar.lemon"
{
    HREprintf (error, "EXP syntax error.\n");
    HREabort(0);
}
#line 891 "exp-grammar.c"
        break;
      case 8: /* sync_actions ::= sync_actions COMMA sync_action */
#line 138 "exp-grammar.lemon"
{
    yygotominor.yy38 = expAddList(yymsp[-2].minor.yy38,yymsp[0].minor.yy28);
}
#line 898 "exp-grammar.c"
        break;
      case 9: /* sync_actions ::= sync_action */
#line 141 "exp-grammar.lemon"
{
    yygotominor.yy38 = expAddList(NULL, yymsp[0].minor.yy28);
}
#line 905 "exp-grammar.c"
        break;
      case 10: /* sync_action ::= ACTION HASH NUMBER */
#line 144 "exp-grammar.lemon"
{
    char* label = SIget(env->strings, yymsp[-2].minor.yy0);
    yygotominor.yy28 = RTmalloc(sizeof(struct sync_action_number_s));
    yygotominor.yy28->label = label;
    yygotominor.yy28->number = yymsp[0].minor.yy0;
}
#line 915 "exp-grammar.c"
        break;
      case 11: /* sync_action ::= ACTION */
#line 150 "exp-grammar.lemon"
{
    char* label = SIget(env->strings, yymsp[0].minor.yy0);
    yygotominor.yy28 = RTmalloc(sizeof(struct sync_action_number_s));
    yygotominor.yy28->label = label;
    yygotominor.yy28->number = 2;
}
#line 925 "exp-grammar.c"
        break;
      case 12: /* sync_vectors ::= sync_vectors COMMA sync_rule */
#line 157 "exp-grammar.lemon"
{
    char*** a = RTmalloc(sizeof(char**));
    *a = yymsp[0].minor.yy1;
    yygotominor.yy38 = expAddList(yymsp[-2].minor.yy38,a);
}
#line 934 "exp-grammar.c"
        break;
      case 13: /* sync_vectors ::= sync_rule */
#line 162 "exp-grammar.lemon"
{
    char*** a = RTmalloc(sizeof(char**));
    *a = yymsp[0].minor.yy1;
    yygotominor.yy38 = expAddList(NULL,a);
}
#line 943 "exp-grammar.c"
        break;
      case 14: /* sync_rule ::= actions ARROW ACTION */
#line 167 "exp-grammar.lemon"
{
    char** a = RTmalloc(sizeof(char*));
    *a = SIget(env->strings, yymsp[0].minor.yy0);
    list_t action_list = expAddList(yymsp[-2].minor.yy38,a);
    yygotominor.yy1 = (char**) expListToArray(action_list, sizeof(char*));
}
#line 953 "exp-grammar.c"
        break;
      case 15: /* actions ::= actions STAR action */
#line 173 "exp-grammar.lemon"
{ char** a = RTmalloc(sizeof(char*)); *a = yymsp[0].minor.yy29; yygotominor.yy38 = expAddList(yymsp[-2].minor.yy38,a); }
#line 958 "exp-grammar.c"
        break;
      case 16: /* actions ::= action */
#line 174 "exp-grammar.lemon"
{ char** a = RTmalloc(sizeof(char*)); *a = yymsp[0].minor.yy29; yygotominor.yy38 = expAddList(NULL,a); }
#line 963 "exp-grammar.c"
        break;
      case 17: /* action ::= UNDERSCORE */
#line 175 "exp-grammar.lemon"
{ yygotominor.yy29 = NULL; }
#line 968 "exp-grammar.c"
        break;
      case 18: /* action ::= ACTION */
#line 176 "exp-grammar.lemon"
{ yygotominor.yy29 = SIget(env->strings, yymsp[0].minor.yy0); }
#line 973 "exp-grammar.c"
        break;
      case 19: /* proc_section ::= proc_section BARS exp_spec */
#line 178 "exp-grammar.lemon"
{ yygotominor.yy38 = expAddList(yymsp[-2].minor.yy38,yymsp[0].minor.yy39); }
#line 978 "exp-grammar.c"
        break;
      case 20: /* proc_section ::= exp_spec */
#line 179 "exp-grammar.lemon"
{ yygotominor.yy38 = expAddList(NULL,yymsp[0].minor.yy39); }
#line 983 "exp-grammar.c"
        break;
      case 21: /* trans_list ::= trans_list trans */
#line 182 "exp-grammar.lemon"
{ yygotominor.yy38 = expAddList(yymsp[-1].minor.yy38,yymsp[0].minor.yy47); }
#line 988 "exp-grammar.c"
        break;
      case 22: /* trans_list ::= */
#line 183 "exp-grammar.lemon"
{ yygotominor.yy38 = NULL; }
#line 993 "exp-grammar.c"
        break;
      case 23: /* trans ::= LPAR NUMBER COMMA STRING COMMA NUMBER RPAR */
      case 24: /* trans ::= LPAR NUMBER COMMA ACTION COMMA NUMBER RPAR */
#line 184 "exp-grammar.lemon"
{
    transition_t trans = RTmalloc(sizeof(struct transition_s));
    trans->source = yymsp[-5].minor.yy0;
    trans->label = SIget(env->strings, yymsp[-3].minor.yy0);
    trans->destination = yymsp[-1].minor.yy0;
    yygotominor.yy47 = trans;
}
#line 1005 "exp-grammar.c"
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  expParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
#line 31 "exp-grammar.lemon"
 HREabort(0); 
#line 1052 "exp-grammar.c"
  expParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  expParseARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 23 "exp-grammar.lemon"

    (void)yymajor;(void)yyminor;
    if (env->lineno == 0) {
        HREmessage(error,"%s: syntax error near pos %d", env->filename, env->linepos+1);
    } else {
        HREmessage(error,"%s: syntax error near line %d, pos %d. Token: %s", env->filename, env->lineno+1,env->linepos+1, yyTokenName[TOKEN]);
    }
#line 1074 "exp-grammar.c"
  expParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  expParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
#line 32 "exp-grammar.lemon"
 Warning(infoLong,"success!"); 
#line 1095 "exp-grammar.c"
  expParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "expParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void expParse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  expParseTOKENTYPE yyminor       /* The value for the token */
  expParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  expParseARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
