/*
** 2000-05-29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Driver template for the LEMON parser generator.
**
** The "lemon" program processes an LALR(1) input grammar file, then uses
** this template to construct a parser.  The "lemon" program inserts text
** at each "%%" line.  Also, any "P-a-r-s-e" identifer prefix (without the
** interstitial "-" characters) contained in this template is changed into
** the value of the %name directive from the grammar.  Otherwise, the content
** of this template is copied straight through into the generate parser
** source file.
**
** The following is the concatenation of all %include directives from the
** input grammar file:
*/
#include <stdio.h>
/************ Begin %include sections from the grammar ************************/
#line 48 "parse.y"

#include "sqliteInt.h"

/*
** Disable all error recovery processing in the parser push-down
** automaton.
*/
#define YYNOERRORRECOVERY 1

/*
** Make yytestcase() the same as testcase()
*/
#define yytestcase(X) testcase(X)

/*
** Indicate that sqlite3ParserFree() will never be called with a null
** pointer.
*/
#define YYPARSEFREENEVERNULL 1

/*
** Alternative datatype for the argument to the malloc() routine passed
** into sqlite3ParserAlloc().  The default is size_t.
*/
#define YYMALLOCARGTYPE  u64

/*
** An instance of this structure holds information about the
** LIMIT clause of a SELECT statement.
*/
struct LimitVal {
  Expr *pLimit;    /* The LIMIT expression.  NULL if there is no limit */
  Expr *pOffset;   /* The OFFSET expression.  NULL if there is none */
};

/*
** An instance of this structure is used to store the LIKE,
** GLOB, NOT LIKE, and NOT GLOB operators.
*/
struct LikeOp {
  Token eOperator;  /* "like" or "glob" or "regexp" */
  int bNot;         /* True if the NOT keyword is present */
};

/*
** An instance of the following structure describes the event of a
** TRIGGER.  "a" is the event type, one of TK_UPDATE, TK_INSERT,
** TK_DELETE, or TK_INSTEAD.  If the event is of the form
**
**      UPDATE ON (a,b,c)
**
** Then the "b" IdList records the list "a,b,c".
*/
struct TrigEvent { int a; IdList * b; };

/*
** An instance of this structure holds the ATTACH key and the key type.
*/
struct AttachKey { int type;  Token key; };

/*
** Disable lookaside memory allocation for objects that might be
** shared across database connections.
*/
static void disableLookaside(Parse *pParse){
  pParse->disableLookaside++;
  pParse->db->lookaside.bDisable++;
}

#line 416 "parse.y"

  /*
  ** For a compound SELECT statement, make sure p->pPrior->pNext==p for
  ** all elements in the list.  And make sure list length does not exceed
  ** SQLITE_LIMIT_COMPOUND_SELECT.
  */
  static void parserDoubleLinkSelect(Parse *pParse, Select *p){
    if( p->pPrior ){
      Select *pNext = 0, *pLoop;
      int mxSelect, cnt = 0;
      for(pLoop=p; pLoop; pNext=pLoop, pLoop=pLoop->pPrior, cnt++){
        pLoop->pNext = pNext;
        pLoop->selFlags |= SF_Compound;
      }
      if( (p->selFlags & SF_MultiValue)==0 && 
        (mxSelect = pParse->db->aLimit[SQLITE_LIMIT_COMPOUND_SELECT])>0 &&
        cnt>mxSelect
      ){
        sqlite3ErrorMsg(pParse, "too many terms in compound SELECT");
      }
    }
  }
#line 833 "parse.y"

  /* This is a utility routine used to set the ExprSpan.zStart and
  ** ExprSpan.zEnd values of pOut so that the span covers the complete
  ** range of text beginning with pStart and going to the end of pEnd.
  */
  static void spanSet(ExprSpan *pOut, Token *pStart, Token *pEnd){
    pOut->zStart = pStart->z;
    pOut->zEnd = &pEnd->z[pEnd->n];
  }

  /* Construct a new Expr object from a single identifier.  Use the
  ** new Expr to populate pOut.  Set the span of pOut to be the identifier
  ** that created the expression.
  */
  static void spanExpr(ExprSpan *pOut, Parse *pParse, int op, Token t){
    pOut->pExpr = sqlite3PExpr(pParse, op, 0, 0, &t);
    pOut->zStart = t.z;
    pOut->zEnd = &t.z[t.n];
  }
#line 925 "parse.y"

  /* This routine constructs a binary expression node out of two ExprSpan
  ** objects and uses the result to populate a new ExprSpan object.
  */
  static void spanBinaryExpr(
    Parse *pParse,      /* The parsing context.  Errors accumulate here */
    int op,             /* The binary operation */
    ExprSpan *pLeft,    /* The left operand, and output */
    ExprSpan *pRight    /* The right operand */
  ){
    pLeft->pExpr = sqlite3PExpr(pParse, op, pLeft->pExpr, pRight->pExpr, 0);
    pLeft->zEnd = pRight->zEnd;
  }

  /* If doNot is true, then add a TK_NOT Expr-node wrapper around the
  ** outside of *ppExpr.
  */
  static void exprNot(Parse *pParse, int doNot, ExprSpan *pSpan){
    if( doNot ){
      pSpan->pExpr = sqlite3PExpr(pParse, TK_NOT, pSpan->pExpr, 0, 0);
    }
  }
#line 984 "parse.y"

  /* Construct an expression node for a unary postfix operator
  */
  static void spanUnaryPostfix(
    Parse *pParse,         /* Parsing context to record errors */
    int op,                /* The operator */
    ExprSpan *pOperand,    /* The operand, and output */
    Token *pPostOp         /* The operand token for setting the span */
  ){
    pOperand->pExpr = sqlite3PExpr(pParse, op, pOperand->pExpr, 0, 0);
    pOperand->zEnd = &pPostOp->z[pPostOp->n];
  }                           
#line 1001 "parse.y"

  /* A routine to convert a binary TK_IS or TK_ISNOT expression into a
  ** unary TK_ISNULL or TK_NOTNULL expression. */
  static void binaryToUnaryIfNull(Parse *pParse, Expr *pY, Expr *pA, int op){
    sqlite3 *db = pParse->db;
    if( pA && pY && pY->op==TK_NULL ){
      pA->op = (u8)op;
      sqlite3ExprDelete(db, pA->pRight);
      pA->pRight = 0;
    }
  }
#line 1029 "parse.y"

  /* Construct an expression node for a unary prefix operator
  */
  static void spanUnaryPrefix(
    ExprSpan *pOut,        /* Write the new expression node here */
    Parse *pParse,         /* Parsing context to record errors */
    int op,                /* The operator */
    ExprSpan *pOperand,    /* The operand */
    Token *pPreOp         /* The operand token for setting the span */
  ){
    pOut->zStart = pPreOp->z;
    pOut->pExpr = sqlite3PExpr(pParse, op, pOperand->pExpr, 0, 0);
    pOut->zEnd = pOperand->zEnd;
  }
#line 1241 "parse.y"

  /* Add a single new term to an ExprList that is used to store a
  ** list of identifiers.  Report an error if the ID list contains
  ** a COLLATE clause or an ASC or DESC keyword, except ignore the
  ** error while parsing a legacy schema.
  */
  static ExprList *parserAddExprIdListTerm(
    Parse *pParse,
    ExprList *pPrior,
    Token *pIdToken,
    int hasCollate,
    int sortOrder
  ){
    ExprList *p = sqlite3ExprListAppend(pParse, pPrior, 0);
    if( (hasCollate || sortOrder!=SQLITE_SO_UNDEFINED)
        && pParse->db->init.busy==0
    ){
      sqlite3ErrorMsg(pParse, "syntax error after column name \"%.*s\"",
                         pIdToken->n, pIdToken->z);
    }
    sqlite3ExprListSetName(pParse, p, pIdToken, 1);
    return p;
  }
#line 228 "parse.c"
/**************** End of %include directives **********************************/
/* These constants specify the various numeric values for terminal symbols
** in a format understandable to "makeheaders".  This section is blank unless
** "lemon" is run with the "-m" command-line option.
***************** Begin makeheaders token definitions *************************/
/**************** End makeheaders token definitions ***************************/

/* The next sections is a series of control #defines.
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used to store the integer codes
**                       that represent terminal and non-terminal symbols.
**                       "unsigned char" is used if there are fewer than
**                       256 symbols.  Larger types otherwise.
**    YYNOCODE           is a number of type YYCODETYPE that is not used for
**                       any terminal or nonterminal symbol.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       (also known as: "terminal symbols") have fall-back
**                       values which should be used if the original symbol
**                       would not parse.  This permits keywords to sometimes
**                       be used as identifiers, for example.
**    YYACTIONTYPE       is the data type used for "action codes" - numbers
**                       that indicate what to do in response to the next
**                       token.
**    sqlite3ParserTOKENTYPE     is the data type used for minor type for terminal
**                       symbols.  Background: A "minor type" is a semantic
**                       value associated with a terminal or non-terminal
**                       symbols.  For example, for an "ID" terminal symbol,
**                       the minor type might be the name of the identifier.
**                       Each non-terminal can have a different minor type.
**                       Terminal symbols all have the same minor type, though.
**                       This macros defines the minor type for terminal 
**                       symbols.
**    YYMINORTYPE        is the data type used for all minor types.
**                       This is typically a union of many types, one of
**                       which is sqlite3ParserTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    sqlite3ParserARG_SDECL     A static variable declaration for the %extra_argument
**    sqlite3ParserARG_PDECL     A parameter declaration for the %extra_argument
**    sqlite3ParserARG_STORE     Code to store %extra_argument into yypParser
**    sqlite3ParserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YY_MAX_SHIFT       Maximum value for shift actions
**    YY_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    YY_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    YY_MIN_REDUCE      Maximum value for reduce actions
**    YY_ERROR_ACTION    The yy_action[] code for syntax error
**    YY_ACCEPT_ACTION   The yy_action[] code for accept
**    YY_NO_ACTION       The yy_action[] code for no-op
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define YYCODETYPE unsigned char
#define YYNOCODE 252
#define YYACTIONTYPE unsigned short int
#define YYWILDCARD 96
#define sqlite3ParserTOKENTYPE Token
typedef union {
  int yyinit;
  sqlite3ParserTOKENTYPE yy0;
  Expr* yy72;
  TriggerStep* yy145;
  ExprList* yy148;
  SrcList* yy185;
  ExprSpan yy190;
  int yy194;
  Select* yy243;
  IdList* yy254;
  With* yy285;
  struct TrigEvent yy332;
  struct LimitVal yy354;
  struct LikeOp yy392;
  struct {int value; int mask;} yy497;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define sqlite3ParserARG_SDECL Parse *pParse;
#define sqlite3ParserARG_PDECL ,Parse *pParse
#define sqlite3ParserARG_FETCH Parse *pParse = yypParser->pParse
#define sqlite3ParserARG_STORE yypParser->pParse = pParse
#define YYFALLBACK 1
#define YYNSTATE             443
#define YYNRULE              328
#define YY_MAX_SHIFT         442
#define YY_MIN_SHIFTREDUCE   653
#define YY_MAX_SHIFTREDUCE   980
#define YY_MIN_REDUCE        981
#define YY_MAX_REDUCE        1308
#define YY_ERROR_ACTION      1309
#define YY_ACCEPT_ACTION     1310
#define YY_NO_ACTION         1311
/************* End control #defines *******************************************/

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N <= YY_MAX_SHIFT             Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   N between YY_MIN_SHIFTREDUCE       Shift to an arbitrary state then
**     and YY_MAX_SHIFTREDUCE           reduce by rule N-YY_MIN_SHIFTREDUCE.
**
**   N between YY_MIN_REDUCE            Reduce by rule N-YY_MIN_REDUCE
**     and YY_MAX_REDUCE

**   N == YY_ERROR_ACTION               A syntax error has occurred.
**
**   N == YY_ACCEPT_ACTION              The parser accepts its input.
**
**   N == YY_NO_ACTION                  No such action.  Denotes unused
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
**
*********** Begin parsing tables **********************************************/
#define YY_ACTTAB_COUNT (1507)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   317,  814,  341,  808,    5,  195,  195,  802,   93,   94,
 /*    10 */    84,  823,  823,  835,  838,  827,  827,   91,   91,   92,
 /*    20 */    92,   92,   92,  293,   90,   90,   90,   90,   89,   89,
 /*    30 */    88,   88,   88,   87,  341,  317,  958,  958,  807,  807,
 /*    40 */   807,  928,  344,   93,   94,   84,  823,  823,  835,  838,
 /*    50 */   827,  827,   91,   91,   92,   92,   92,   92,  328,   90,
 /*    60 */    90,   90,   90,   89,   89,   88,   88,   88,   87,  341,
 /*    70 */    89,   89,   88,   88,   88,   87,  341,  776,  958,  958,
 /*    80 */   317,   88,   88,   88,   87,  341,  777,   69,   93,   94,
 /*    90 */    84,  823,  823,  835,  838,  827,  827,   91,   91,   92,
 /*   100 */    92,   92,   92,  437,   90,   90,   90,   90,   89,   89,
 /*   110 */    88,   88,   88,   87,  341, 1310,  147,  147,    2,  317,
 /*   120 */    76,   25,   74,   49,   49,   87,  341,   93,   94,   84,
 /*   130 */   823,  823,  835,  838,  827,  827,   91,   91,   92,   92,
 /*   140 */    92,   92,   95,   90,   90,   90,   90,   89,   89,   88,
 /*   150 */    88,   88,   87,  341,  939,  939,  317,  260,  415,  400,
 /*   160 */   398,   58,  737,  737,   93,   94,   84,  823,  823,  835,
 /*   170 */   838,  827,  827,   91,   91,   92,   92,   92,   92,   57,
 /*   180 */    90,   90,   90,   90,   89,   89,   88,   88,   88,   87,
 /*   190 */   341,  317, 1253,  928,  344,  269,  940,  941,  242,   93,
 /*   200 */    94,   84,  823,  823,  835,  838,  827,  827,   91,   91,
 /*   210 */    92,   92,   92,   92,  293,   90,   90,   90,   90,   89,
 /*   220 */    89,   88,   88,   88,   87,  341,  317,  919, 1303,  793,
 /*   230 */   691, 1303,  724,  724,   93,   94,   84,  823,  823,  835,
 /*   240 */   838,  827,  827,   91,   91,   92,   92,   92,   92,  337,
 /*   250 */    90,   90,   90,   90,   89,   89,   88,   88,   88,   87,
 /*   260 */   341,  317,  114,  919, 1304,  684,  395, 1304,  124,   93,
 /*   270 */    94,   84,  823,  823,  835,  838,  827,  827,   91,   91,
 /*   280 */    92,   92,   92,   92,  683,   90,   90,   90,   90,   89,
 /*   290 */    89,   88,   88,   88,   87,  341,  317,   86,   83,  169,
 /*   300 */   801,  917,  234,  399,   93,   94,   84,  823,  823,  835,
 /*   310 */   838,  827,  827,   91,   91,   92,   92,   92,   92,  686,
 /*   320 */    90,   90,   90,   90,   89,   89,   88,   88,   88,   87,
 /*   330 */   341,  317,  436,  742,   86,   83,  169,  917,  741,   93,
 /*   340 */    94,   84,  823,  823,  835,  838,  827,  827,   91,   91,
 /*   350 */    92,   92,   92,   92,  902,   90,   90,   90,   90,   89,
 /*   360 */    89,   88,   88,   88,   87,  341,  317,  321,  434,  434,
 /*   370 */   434,    1,  722,  722,   93,   94,   84,  823,  823,  835,
 /*   380 */   838,  827,  827,   91,   91,   92,   92,   92,   92,  190,
 /*   390 */    90,   90,   90,   90,   89,   89,   88,   88,   88,   87,
 /*   400 */   341,  317,  685,  292,  939,  939,  150,  977,  310,   93,
 /*   410 */    94,   84,  823,  823,  835,  838,  827,  827,   91,   91,
 /*   420 */    92,   92,   92,   92,  437,   90,   90,   90,   90,   89,
 /*   430 */    89,   88,   88,   88,   87,  341,  926,    2,  372,  719,
 /*   440 */   698,  369,  950,  317,   49,   49,  940,  941,  719,  177,
 /*   450 */    72,   93,   94,   84,  823,  823,  835,  838,  827,  827,
 /*   460 */    91,   91,   92,   92,   92,   92,  322,   90,   90,   90,
 /*   470 */    90,   89,   89,   88,   88,   88,   87,  341,  317,  415,
 /*   480 */   405,  824,  824,  836,  839,   75,   93,   82,   84,  823,
 /*   490 */   823,  835,  838,  827,  827,   91,   91,   92,   92,   92,
 /*   500 */    92,  430,   90,   90,   90,   90,   89,   89,   88,   88,
 /*   510 */    88,   87,  341,  317,  340,  340,  340,  658,  659,  660,
 /*   520 */   333,  288,   94,   84,  823,  823,  835,  838,  827,  827,
 /*   530 */    91,   91,   92,   92,   92,   92,  437,   90,   90,   90,
 /*   540 */    90,   89,   89,   88,   88,   88,   87,  341,  317,  882,
 /*   550 */   882,  375,  828,   66,  330,  409,   49,   49,   84,  823,
 /*   560 */   823,  835,  838,  827,  827,   91,   91,   92,   92,   92,
 /*   570 */    92,  351,   90,   90,   90,   90,   89,   89,   88,   88,
 /*   580 */    88,   87,  341,   80,  432,  742,    3, 1180,  351,  350,
 /*   590 */   741,  334,  796,  939,  939,  761,   80,  432,  278,    3,
 /*   600 */   204,  161,  279,  393,  274,  392,  191,  362,  437,  277,
 /*   610 */   745,   77,   78,  272,  800,  254,  355,  243,   79,  342,
 /*   620 */   342,   86,   83,  169,   77,   78,  234,  399,   49,   49,
 /*   630 */   435,   79,  342,  342,  437,  940,  941,  186,  442,  655,
 /*   640 */   390,  387,  386,  435,  235,  213,  108,  421,  761,  351,
 /*   650 */   437,  385,  167,  732,   10,   10,  124,  124,  671,  814,
 /*   660 */   421,  439,  438,  415,  414,  802,  362,  168,  327,  124,
 /*   670 */    49,   49,  814,  219,  439,  438,  800,  186,  802,  326,
 /*   680 */   390,  387,  386,  437, 1248, 1248,   23,  939,  939,   80,
 /*   690 */   432,  385,    3,  761,  416,  876,  807,  807,  807,  809,
 /*   700 */    19,  290,  149,   49,   49,  415,  396,  260,  910,  807,
 /*   710 */   807,  807,  809,   19,  312,  237,  145,   77,   78,  746,
 /*   720 */   168,  702,  437,  149,   79,  342,  342,  114,  358,  940,
 /*   730 */   941,  302,  223,  397,  345,  313,  435,  260,  415,  417,
 /*   740 */   858,  374,   31,   31,   80,  432,  761,    3,  348,   92,
 /*   750 */    92,   92,   92,  421,   90,   90,   90,   90,   89,   89,
 /*   760 */    88,   88,   88,   87,  341,  814,  114,  439,  438,  796,
 /*   770 */   367,  802,   77,   78,  701,  796,  124, 1187,  220,   79,
 /*   780 */   342,  342,  124,  747,  734,  939,  939,  775,  404,  939,
 /*   790 */   939,  435,  254,  360,  253,  402,  895,  346,  254,  360,
 /*   800 */   253,  774,  807,  807,  807,  809,   19,  800,  421,   90,
 /*   810 */    90,   90,   90,   89,   89,   88,   88,   88,   87,  341,
 /*   820 */   814,  114,  439,  438,  939,  939,  802,  940,  941,  114,
 /*   830 */   437,  940,  941,   86,   83,  169,  192,  166,  309,  979,
 /*   840 */    70,  432,  700,    3,  382,  870,  238,   86,   83,  169,
 /*   850 */    10,   10,  361,  406,  763,  190,  222,  807,  807,  807,
 /*   860 */   809,   19,  870,  872,  329,   24,  940,  941,   77,   78,
 /*   870 */   359,  437,  335,  260,  218,   79,  342,  342,  437,  307,
 /*   880 */   306,  305,  207,  303,  339,  338,  668,  435,  339,  338,
 /*   890 */   407,   10,   10,  762,  216,  216,  939,  939,   49,   49,
 /*   900 */   437,  260,   97,  241,  421,  225,  402,  189,  188,  187,
 /*   910 */   309,  918,  980,  149,  221,  898,  814,  868,  439,  438,
 /*   920 */    10,   10,  802,  870,  915,  316,  898,  163,  162,  171,
 /*   930 */   249,  240,  322,  410,  412,  687,  687,  272,  940,  941,
 /*   940 */   239,  965,  901,  437,  226,  403,  226,  437,  963,  367,
 /*   950 */   964,  173,  248,  807,  807,  807,  809,   19,  174,  367,
 /*   960 */   899,  124,  172,   48,   48,    9,    9,   35,   35,  966,
 /*   970 */   966,  899,  363,  966,  966,  814,  900,  808,  725,  939,
 /*   980 */   939,  802,  895,  318,  980,  324,  125,  900,  726,  420,
 /*   990 */    92,   92,   92,   92,   85,   90,   90,   90,   90,   89,
 /*  1000 */    89,   88,   88,   88,   87,  341,  216,  216,  437,  946,
 /*  1010 */   349,  292,  807,  807,  807,  114,  291,  693,  402,  705,
 /*  1020 */   890,  940,  941,  437,  245,  889,  247,  437,   36,   36,
 /*  1030 */   437,  353,  391,  437,  260,  252,  260,  437,  361,  437,
 /*  1040 */   706,  437,  370,   12,   12,  224,  437,   27,   27,  437,
 /*  1050 */    37,   37,  437,   38,   38,  752,  368,   39,   39,   28,
 /*  1060 */    28,   29,   29,  215,  166,  331,   40,   40,  437,   41,
 /*  1070 */    41,  437,   42,   42,  437,  866,  246,  731,  437,  879,
 /*  1080 */   437,  256,  437,  878,  437,  267,  437,  261,   11,   11,
 /*  1090 */   437,   43,   43,  437,   99,   99,  437,  373,   44,   44,
 /*  1100 */    45,   45,   32,   32,   46,   46,   47,   47,  437,  426,
 /*  1110 */    33,   33,  776,  116,  116,  437,  117,  117,  437,  124,
 /*  1120 */   437,  777,  437,  260,  437,  957,  437,  352,  118,  118,
 /*  1130 */   437,  195,  437,  111,  437,   53,   53,  264,   34,   34,
 /*  1140 */   100,  100,   50,   50,  101,  101,  102,  102,  437,  260,
 /*  1150 */    98,   98,  115,  115,  113,  113,  437,  262,  437,  265,
 /*  1160 */   437,  943,  958,  437,  727,  437,  681,  437,  106,  106,
 /*  1170 */    68,  437,  893,  730,  437,  365,  105,  105,  103,  103,
 /*  1180 */   104,  104,  217,   52,   52,   54,   54,   51,   51,  694,
 /*  1190 */   259,   26,   26,  266,   30,   30,  677,  323,  433,  323,
 /*  1200 */   674,  423,  427,  943,  958,  114,  114,  431,  681,  865,
 /*  1210 */  1277,  233,  366,  714,  112,   20,  154,  704,  703,  810,
 /*  1220 */   914,   55,  159,  311,  798,  255,  383,  194,   68,  200,
 /*  1230 */    21,  694,  268,  114,  114,  114,  270,  711,  712,   68,
 /*  1240 */   114,  739,  770,  715,   71,  194,  861,  875,  875,  200,
 /*  1250 */   696,  865,  874,  874,  679,  699,  273,  110,  229,  419,
 /*  1260 */   768,  810,  799,  378,  748,  759,  418,  210,  294,  281,
 /*  1270 */   295,  806,  283,  682,  676,  665,  664,  666,  933,  151,
 /*  1280 */   285,    7, 1267,  308,  251,  790,  354,  244,  892,  364,
 /*  1290 */   287,  422,  300,  164,  160,  936,  974,  127,  197,  137,
 /*  1300 */   909,  907,  971,  388,  276,  863,  862,   56,  698,  325,
 /*  1310 */   148,   59,  122,   66,  356,  381,  357,  176,  152,   62,
 /*  1320 */   371,  130,  877,  181,  377,  760,  211,  182,  132,  133,
 /*  1330 */   134,  135,  258,  146,  140,  795,  787,  263,  183,  379,
 /*  1340 */   667,  394,  184,  332,  894,  314,  718,  717,  857,  716,
 /*  1350 */   696,  315,  709,  690,   65,  196,    6,  408,  289,  708,
 /*  1360 */   275,  689,  688,  948,  756,  757,  280,  282,  425,  755,
 /*  1370 */   284,  336,   73,   67,  754,  429,  411,   96,  286,  413,
 /*  1380 */   205,  934,  673,   22,  209,  440,  119,  120,  109,  206,
 /*  1390 */   208,  441,  662,  661,  656,  843,  654,  343,  158,  236,
 /*  1400 */   170,  347,  107,  227,  121,  738,  873,  298,  296,  297,
 /*  1410 */   299,  871,  794,  128,  129,  728,  230,  131,  175,  250,
 /*  1420 */   888,  136,  138,  231,  232,  139,   60,   61,  891,  178,
 /*  1430 */   179,  887,    8,   13,  180,  257,  880,  968,  194,  141,
 /*  1440 */   142,  376,  153,  670,  380,  185,  143,  277,   63,  384,
 /*  1450 */    14,  707,  271,   15,  389,   64,  319,  320,  126,  228,
 /*  1460 */   813,  812,  841,  736,  123,   16,  401,  740,    4,  769,
 /*  1470 */   165,  212,  214,  193,  144,  764,   71,   68,   17,   18,
 /*  1480 */   856,  842,  840,  897,  845,  896,  199,  198,  923,  155,
 /*  1490 */   424,  929,  924,  156,  201,  202,  428,  844,  157,  203,
 /*  1500 */   811,  680,   81, 1269, 1268,  301,  304,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    19,   95,   53,   97,   22,   24,   24,  101,   27,   28,
 /*    10 */    29,   30,   31,   32,   33,   34,   35,   36,   37,   38,
 /*    20 */    39,   40,   41,  152,   43,   44,   45,   46,   47,   48,
 /*    30 */    49,   50,   51,   52,   53,   19,   55,   55,  132,  133,
 /*    40 */   134,    1,    2,   27,   28,   29,   30,   31,   32,   33,
 /*    50 */    34,   35,   36,   37,   38,   39,   40,   41,  187,   43,
 /*    60 */    44,   45,   46,   47,   48,   49,   50,   51,   52,   53,
 /*    70 */    47,   48,   49,   50,   51,   52,   53,   61,   97,   97,
 /*    80 */    19,   49,   50,   51,   52,   53,   70,   26,   27,   28,
 /*    90 */    29,   30,   31,   32,   33,   34,   35,   36,   37,   38,
 /*   100 */    39,   40,   41,  152,   43,   44,   45,   46,   47,   48,
 /*   110 */    49,   50,   51,   52,   53,  144,  145,  146,  147,   19,
 /*   120 */   137,   22,  139,  172,  173,   52,   53,   27,   28,   29,
 /*   130 */    30,   31,   32,   33,   34,   35,   36,   37,   38,   39,
 /*   140 */    40,   41,   81,   43,   44,   45,   46,   47,   48,   49,
 /*   150 */    50,   51,   52,   53,   55,   56,   19,  152,  207,  208,
 /*   160 */   115,   24,  117,  118,   27,   28,   29,   30,   31,   32,
 /*   170 */    33,   34,   35,   36,   37,   38,   39,   40,   41,   79,
 /*   180 */    43,   44,   45,   46,   47,   48,   49,   50,   51,   52,
 /*   190 */    53,   19,    0,    1,    2,   23,   97,   98,  193,   27,
 /*   200 */    28,   29,   30,   31,   32,   33,   34,   35,   36,   37,
 /*   210 */    38,   39,   40,   41,  152,   43,   44,   45,   46,   47,
 /*   220 */    48,   49,   50,   51,   52,   53,   19,   22,   23,  163,
 /*   230 */    23,   26,  190,  191,   27,   28,   29,   30,   31,   32,
 /*   240 */    33,   34,   35,   36,   37,   38,   39,   40,   41,  187,
 /*   250 */    43,   44,   45,   46,   47,   48,   49,   50,   51,   52,
 /*   260 */    53,   19,  196,   22,   23,   23,   49,   26,   92,   27,
 /*   270 */    28,   29,   30,   31,   32,   33,   34,   35,   36,   37,
 /*   280 */    38,   39,   40,   41,  172,   43,   44,   45,   46,   47,
 /*   290 */    48,   49,   50,   51,   52,   53,   19,  221,  222,  223,
 /*   300 */    23,   96,  119,  120,   27,   28,   29,   30,   31,   32,
 /*   310 */    33,   34,   35,   36,   37,   38,   39,   40,   41,  172,
 /*   320 */    43,   44,   45,   46,   47,   48,   49,   50,   51,   52,
 /*   330 */    53,   19,  152,  116,  221,  222,  223,   96,  121,   27,
 /*   340 */    28,   29,   30,   31,   32,   33,   34,   35,   36,   37,
 /*   350 */    38,   39,   40,   41,  241,   43,   44,   45,   46,   47,
 /*   360 */    48,   49,   50,   51,   52,   53,   19,  157,  168,  169,
 /*   370 */   170,   22,  190,  191,   27,   28,   29,   30,   31,   32,
 /*   380 */    33,   34,   35,   36,   37,   38,   39,   40,   41,   30,
 /*   390 */    43,   44,   45,   46,   47,   48,   49,   50,   51,   52,
 /*   400 */    53,   19,  172,  152,   55,   56,   24,  247,  248,   27,
 /*   410 */    28,   29,   30,   31,   32,   33,   34,   35,   36,   37,
 /*   420 */    38,   39,   40,   41,  152,   43,   44,   45,   46,   47,
 /*   430 */    48,   49,   50,   51,   52,   53,  146,  147,  228,  179,
 /*   440 */   180,  231,  185,   19,  172,  173,   97,   98,  188,   26,
 /*   450 */   138,   27,   28,   29,   30,   31,   32,   33,   34,   35,
 /*   460 */    36,   37,   38,   39,   40,   41,  107,   43,   44,   45,
 /*   470 */    46,   47,   48,   49,   50,   51,   52,   53,   19,  207,
 /*   480 */   208,   30,   31,   32,   33,  138,   27,   28,   29,   30,
 /*   490 */    31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
 /*   500 */    41,  250,   43,   44,   45,   46,   47,   48,   49,   50,
 /*   510 */    51,   52,   53,   19,  168,  169,  170,    7,    8,    9,
 /*   520 */    19,  152,   28,   29,   30,   31,   32,   33,   34,   35,
 /*   530 */    36,   37,   38,   39,   40,   41,  152,   43,   44,   45,
 /*   540 */    46,   47,   48,   49,   50,   51,   52,   53,   19,  108,
 /*   550 */   109,  110,  101,  130,   53,  152,  172,  173,   29,   30,
 /*   560 */    31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
 /*   570 */    41,  152,   43,   44,   45,   46,   47,   48,   49,   50,
 /*   580 */    51,   52,   53,   19,   20,  116,   22,   23,  169,  170,
 /*   590 */   121,  207,   85,   55,   56,   26,   19,   20,  101,   22,
 /*   600 */    99,  100,  101,  102,  103,  104,  105,  152,  152,  112,
 /*   610 */   210,   47,   48,  112,  152,  108,  109,  110,   54,   55,
 /*   620 */    56,  221,  222,  223,   47,   48,  119,  120,  172,  173,
 /*   630 */    66,   54,   55,   56,  152,   97,   98,   99,  148,  149,
 /*   640 */   102,  103,  104,   66,  154,   23,  156,   83,   26,  230,
 /*   650 */   152,  113,  152,  163,  172,  173,   92,   92,   21,   95,
 /*   660 */    83,   97,   98,  207,  208,  101,  152,   98,  186,   92,
 /*   670 */   172,  173,   95,  218,   97,   98,  152,   99,  101,  217,
 /*   680 */   102,  103,  104,  152,  119,  120,  196,   55,   56,   19,
 /*   690 */    20,  113,   22,  124,  163,   11,  132,  133,  134,  135,
 /*   700 */   136,  152,  152,  172,  173,  207,  208,  152,  152,  132,
 /*   710 */   133,  134,  135,  136,  164,  152,   84,   47,   48,   49,
 /*   720 */    98,  181,  152,  152,   54,   55,   56,  196,   91,   97,
 /*   730 */    98,  160,  218,  163,  244,  164,   66,  152,  207,  208,
 /*   740 */   103,  217,  172,  173,   19,   20,  124,   22,  193,   38,
 /*   750 */    39,   40,   41,   83,   43,   44,   45,   46,   47,   48,
 /*   760 */    49,   50,   51,   52,   53,   95,  196,   97,   98,   85,
 /*   770 */   152,  101,   47,   48,  181,   85,   92,  140,  193,   54,
 /*   780 */    55,   56,   92,   49,  195,   55,   56,  175,  163,   55,
 /*   790 */    56,   66,  108,  109,  110,  206,  163,  242,  108,  109,
 /*   800 */   110,  175,  132,  133,  134,  135,  136,  152,   83,   43,
 /*   810 */    44,   45,   46,   47,   48,   49,   50,   51,   52,   53,
 /*   820 */    95,  196,   97,   98,   55,   56,  101,   97,   98,  196,
 /*   830 */   152,   97,   98,  221,  222,  223,  211,  212,   22,   23,
 /*   840 */    19,   20,  181,   22,   19,  152,  152,  221,  222,  223,
 /*   850 */   172,  173,  219,   19,  124,   30,  238,  132,  133,  134,
 /*   860 */   135,  136,  169,  170,  186,  232,   97,   98,   47,   48,
 /*   870 */   237,  152,  217,  152,    5,   54,   55,   56,  152,   10,
 /*   880 */    11,   12,   13,   14,   47,   48,   17,   66,   47,   48,
 /*   890 */    56,  172,  173,  124,  194,  195,   55,   56,  172,  173,
 /*   900 */   152,  152,   22,  152,   83,  186,  206,  108,  109,  110,
 /*   910 */    22,   23,   96,  152,  193,   12,   95,  152,   97,   98,
 /*   920 */   172,  173,  101,  230,  152,  164,   12,   47,   48,   60,
 /*   930 */   152,   62,  107,  207,  186,   55,   56,  112,   97,   98,
 /*   940 */    71,  100,  193,  152,  183,  152,  185,  152,  107,  152,
 /*   950 */   109,   82,   16,  132,  133,  134,  135,  136,   89,  152,
 /*   960 */    57,   92,   93,  172,  173,  172,  173,  172,  173,  132,
 /*   970 */   133,   57,  152,  132,  133,   95,   73,   97,   75,   55,
 /*   980 */    56,  101,  163,  114,   96,  245,  246,   73,   85,   75,
 /*   990 */    38,   39,   40,   41,   42,   43,   44,   45,   46,   47,
 /*  1000 */    48,   49,   50,   51,   52,   53,  194,  195,  152,  171,
 /*  1010 */   141,  152,  132,  133,  134,  196,  225,  179,  206,   65,
 /*  1020 */   152,   97,   98,  152,   88,  152,   90,  152,  172,  173,
 /*  1030 */   152,  219,   78,  152,  152,  238,  152,  152,  219,  152,
 /*  1040 */    86,  152,  152,  172,  173,  238,  152,  172,  173,  152,
 /*  1050 */   172,  173,  152,  172,  173,  213,  237,  172,  173,  172,
 /*  1060 */   173,  172,  173,  211,  212,  111,  172,  173,  152,  172,
 /*  1070 */   173,  152,  172,  173,  152,  193,  140,  193,  152,   59,
 /*  1080 */   152,  152,  152,   63,  152,   16,  152,  152,  172,  173,
 /*  1090 */   152,  172,  173,  152,  172,  173,  152,   77,  172,  173,
 /*  1100 */   172,  173,  172,  173,  172,  173,  172,  173,  152,  250,
 /*  1110 */   172,  173,   61,  172,  173,  152,  172,  173,  152,   92,
 /*  1120 */   152,   70,  152,  152,  152,   26,  152,  100,  172,  173,
 /*  1130 */   152,   24,  152,   22,  152,  172,  173,  152,  172,  173,
 /*  1140 */   172,  173,  172,  173,  172,  173,  172,  173,  152,  152,
 /*  1150 */   172,  173,  172,  173,  172,  173,  152,   88,  152,   90,
 /*  1160 */   152,   55,   55,  152,  193,  152,   55,  152,  172,  173,
 /*  1170 */    26,  152,  163,  163,  152,   19,  172,  173,  172,  173,
 /*  1180 */   172,  173,   22,  172,  173,  172,  173,  172,  173,   55,
 /*  1190 */   193,  172,  173,  152,  172,  173,  166,  167,  166,  167,
 /*  1200 */   163,  163,  163,   97,   97,  196,  196,  163,   97,   55,
 /*  1210 */    23,  199,   56,   26,   22,   22,   24,  100,  101,   55,
 /*  1220 */    23,  209,  123,   26,   23,   23,   23,   26,   26,   26,
 /*  1230 */    37,   97,  152,  196,  196,  196,   23,    7,    8,   26,
 /*  1240 */   196,   23,   23,  152,   26,   26,   23,  132,  133,   26,
 /*  1250 */   106,   97,  132,  133,   23,  152,  152,   26,  210,  191,
 /*  1260 */   152,   97,  152,  234,  152,  152,  152,  233,  152,  210,
 /*  1270 */   152,  152,  210,  152,  152,  152,  152,  152,  152,  197,
 /*  1280 */   210,  198,  122,  150,  239,  201,  214,  214,  201,  239,
 /*  1290 */   214,  227,  200,  184,  198,  155,   67,  243,  122,   22,
 /*  1300 */   159,  159,   69,  176,  175,  175,  175,  240,  180,  159,
 /*  1310 */   220,  240,   27,  130,   18,   18,  159,  158,  220,  137,
 /*  1320 */   159,  189,  236,  158,   74,  159,  159,  158,  192,  192,
 /*  1330 */   192,  192,  235,   22,  189,  189,  201,  159,  158,  177,
 /*  1340 */   159,  107,  158,   76,  201,  177,  174,  174,  201,  174,
 /*  1350 */   106,  177,  182,  174,  107,  159,   22,  125,  159,  182,
 /*  1360 */   174,  176,  174,  174,  216,  216,  215,  215,  177,  216,
 /*  1370 */   215,   53,  137,  128,  216,  177,  127,  129,  215,  126,
 /*  1380 */    25,   13,  162,   26,    6,  161,  165,  165,  178,  153,
 /*  1390 */   153,  151,  151,  151,  151,  224,    4,    3,   22,  142,
 /*  1400 */    15,   94,   16,  178,  165,  205,   23,  202,  204,  203,
 /*  1410 */   201,   23,  120,  131,  111,   20,  226,  123,  125,   16,
 /*  1420 */     1,  123,  131,  229,  229,  111,   37,   37,   56,   64,
 /*  1430 */   122,    1,    5,   22,  107,  140,   80,   87,   26,   80,
 /*  1440 */   107,   72,   24,   20,   19,  105,   22,  112,   22,   79,
 /*  1450 */    22,   58,   23,   22,   79,   22,  249,  249,  246,   79,
 /*  1460 */    23,   23,   23,  116,   68,   22,   26,   23,   22,   56,
 /*  1470 */   122,   23,   23,   64,   22,  124,   26,   26,   64,   64,
 /*  1480 */    23,   23,   23,   23,   11,   23,   22,   26,   23,   22,
 /*  1490 */    24,    1,   23,   22,   26,  122,   24,   23,   22,  122,
 /*  1500 */    23,   23,   22,  122,  122,   23,   15,
};
#define YY_SHIFT_USE_DFLT (-95)
#define YY_SHIFT_COUNT (442)
#define YY_SHIFT_MIN   (-94)
#define YY_SHIFT_MAX   (1491)
static const short yy_shift_ofst[] = {
 /*     0 */    40,  564,  869,  577,  725,  725,  725,  725,  690,  -19,
 /*    10 */    16,   16,  100,  725,  725,  725,  725,  725,  725,  725,
 /*    20 */   841,  841,  538,  507,  684,  565,   61,  137,  172,  207,
 /*    30 */   242,  277,  312,  347,  382,  424,  424,  424,  424,  424,
 /*    40 */   424,  424,  424,  424,  424,  424,  424,  424,  424,  424,
 /*    50 */   459,  424,  494,  529,  529,  670,  725,  725,  725,  725,
 /*    60 */   725,  725,  725,  725,  725,  725,  725,  725,  725,  725,
 /*    70 */   725,  725,  725,  725,  725,  725,  725,  725,  725,  725,
 /*    80 */   725,  725,  725,  725,  821,  725,  725,  725,  725,  725,
 /*    90 */   725,  725,  725,  725,  725,  725,  725,  725,  952,  711,
 /*   100 */   711,  711,  711,  711,  766,   23,   32,  924,  637,  825,
 /*   110 */   837,  837,  924,   73,  183,  -51,  -95,  -95,  -95,  501,
 /*   120 */   501,  501,  903,  903,  632,  205,  241,  924,  924,  924,
 /*   130 */   924,  924,  924,  924,  924,  924,  924,  924,  924,  924,
 /*   140 */   924,  924,  924,  924,  924,  924,  924,  192, 1027, 1106,
 /*   150 */  1106,  183,  176,  176,  176,  176,  176,  176,  -95,  -95,
 /*   160 */   -95,  880,  -94,  -94,  578,  734,   99,  730,  769,  349,
 /*   170 */   924,  924,  924,  924,  924,  924,  924,  924,  924,  924,
 /*   180 */   924,  924,  924,  924,  924,  924,  924,  954,  954,  954,
 /*   190 */   924,  924,  622,  924,  924,  924,  -18,  924,  924,  914,
 /*   200 */   924,  924,  924,  924,  924,  924,  924,  924,  924,  924,
 /*   210 */   441, 1020, 1107, 1107, 1107,  569,   45,  217,  510,  423,
 /*   220 */   834,  834, 1156,  423, 1156, 1144, 1187,  359, 1051,  834,
 /*   230 */   -17, 1051, 1051, 1099,  469, 1192, 1229, 1176, 1176, 1233,
 /*   240 */  1233, 1176, 1277, 1285, 1183, 1296, 1296, 1296, 1296, 1176,
 /*   250 */  1297, 1183, 1277, 1285, 1285, 1183, 1176, 1297, 1182, 1250,
 /*   260 */  1176, 1176, 1297, 1311, 1176, 1297, 1176, 1297, 1311, 1234,
 /*   270 */  1234, 1234, 1267, 1311, 1234, 1244, 1234, 1267, 1234, 1234,
 /*   280 */  1232, 1247, 1232, 1247, 1232, 1247, 1232, 1247, 1176, 1334,
 /*   290 */  1176, 1235, 1311, 1318, 1318, 1311, 1248, 1253, 1245, 1249,
 /*   300 */  1183, 1355, 1357, 1368, 1368, 1378, 1378, 1378, 1378,  -95,
 /*   310 */   -95,  -95,  -95,  -95,  -95,  -95,  -95,  451,  936,  816,
 /*   320 */   888, 1069,  799, 1111, 1197, 1193, 1201, 1202, 1203, 1213,
 /*   330 */  1134, 1117, 1230,  497, 1218, 1219, 1154, 1223, 1115, 1120,
 /*   340 */  1231, 1164, 1160, 1392, 1394, 1376, 1257, 1385, 1307, 1386,
 /*   350 */  1383, 1388, 1292, 1282, 1303, 1294, 1395, 1293, 1403, 1419,
 /*   360 */  1298, 1291, 1389, 1390, 1314, 1372, 1365, 1308, 1430, 1427,
 /*   370 */  1411, 1327, 1295, 1356, 1412, 1359, 1350, 1369, 1333, 1418,
 /*   380 */  1423, 1425, 1335, 1340, 1424, 1370, 1426, 1428, 1429, 1431,
 /*   390 */  1375, 1393, 1433, 1380, 1396, 1437, 1438, 1439, 1347, 1443,
 /*   400 */  1444, 1446, 1440, 1348, 1448, 1449, 1413, 1409, 1452, 1351,
 /*   410 */  1450, 1414, 1451, 1415, 1457, 1450, 1458, 1459, 1460, 1461,
 /*   420 */  1462, 1464, 1473, 1465, 1467, 1466, 1468, 1469, 1471, 1472,
 /*   430 */  1468, 1474, 1476, 1477, 1478, 1480, 1373, 1377, 1381, 1382,
 /*   440 */  1482, 1491, 1490,
};
#define YY_REDUCE_USE_DFLT (-130)
#define YY_REDUCE_COUNT (316)
#define YY_REDUCE_MIN   (-129)
#define YY_REDUCE_MAX   (1243)
static const short yy_reduce_ofst[] = {
 /*     0 */   -29,  531,  490,  570,  -49,  272,  456,  498,  633,  400,
 /*    10 */   612,  626,  113,  482,  678,  719,  384,  726,  748,  791,
 /*    20 */   419,  693,  761,  812,  819,  625,   76,   76,   76,   76,
 /*    30 */    76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
 /*    40 */    76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
 /*    50 */    76,   76,   76,   76,   76,  793,  795,  856,  871,  875,
 /*    60 */   878,  881,  885,  887,  889,  894,  897,  900,  916,  919,
 /*    70 */   922,  926,  928,  930,  932,  934,  938,  941,  944,  956,
 /*    80 */   963,  966,  968,  970,  972,  974,  978,  980,  982,  996,
 /*    90 */  1004, 1006, 1008, 1011, 1013, 1015, 1019, 1022,   76,   76,
 /*   100 */    76,   76,   76,   76,   76,   76,   76,  555,  210,  260,
 /*   110 */   200,  346,  571,   76,  700,   76,   76,   76,   76,  838,
 /*   120 */   838,  838,   42,  182,  251,  160,  160,  550,    5,  455,
 /*   130 */   585,  721,  749,  882,  884,  971,  618,  462,  797,  514,
 /*   140 */   807,  524,  997, -129,  655,  859,   62,  290,   66, 1030,
 /*   150 */  1032,  589, 1009, 1010, 1037, 1038, 1039, 1044,  740,  852,
 /*   160 */  1012,  112,  147,  230,  257,  180,  369,  403,  500,  549,
 /*   170 */   556,  563,  694,  751,  765,  772,  778,  820,  868,  873,
 /*   180 */   890,  929,  935,  985, 1041, 1080, 1091,  540,  593,  661,
 /*   190 */  1103, 1104,  842, 1108, 1110, 1112, 1048, 1113, 1114, 1068,
 /*   200 */  1116, 1118, 1119,  180, 1121, 1122, 1123, 1124, 1125, 1126,
 /*   210 */  1029, 1034, 1059, 1062, 1070,  842, 1082, 1083, 1133, 1084,
 /*   220 */  1072, 1073, 1045, 1087, 1050, 1127, 1109, 1128, 1129, 1076,
 /*   230 */  1064, 1130, 1131, 1092, 1096, 1140, 1054, 1141, 1142, 1067,
 /*   240 */  1071, 1150, 1090, 1132, 1135, 1136, 1137, 1138, 1139, 1157,
 /*   250 */  1159, 1143, 1098, 1145, 1146, 1147, 1161, 1165, 1086, 1097,
 /*   260 */  1166, 1167, 1169, 1162, 1178, 1180, 1181, 1184, 1168, 1172,
 /*   270 */  1173, 1175, 1170, 1174, 1179, 1185, 1186, 1177, 1188, 1189,
 /*   280 */  1148, 1151, 1149, 1152, 1153, 1155, 1158, 1163, 1196, 1171,
 /*   290 */  1199, 1190, 1191, 1194, 1195, 1198, 1200, 1204, 1206, 1205,
 /*   300 */  1209, 1220, 1224, 1236, 1237, 1240, 1241, 1242, 1243, 1207,
 /*   310 */  1208, 1212, 1221, 1222, 1210, 1225, 1239,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */  1258, 1248, 1248, 1248, 1180, 1180, 1180, 1180, 1248, 1077,
 /*    10 */  1106, 1106, 1232, 1309, 1309, 1309, 1309, 1309, 1309, 1179,
 /*    20 */  1309, 1309, 1309, 1309, 1248, 1081, 1112, 1309, 1309, 1309,
 /*    30 */  1309, 1309, 1309, 1309, 1309, 1231, 1233, 1120, 1119, 1214,
 /*    40 */  1093, 1117, 1110, 1114, 1181, 1175, 1176, 1174, 1178, 1182,
 /*    50 */  1309, 1113, 1144, 1159, 1143, 1309, 1309, 1309, 1309, 1309,
 /*    60 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309,
 /*    70 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309,
 /*    80 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309,
 /*    90 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1153, 1158,
 /*   100 */  1165, 1157, 1154, 1146, 1145, 1147, 1148, 1309, 1000, 1048,
 /*   110 */  1309, 1309, 1309, 1149, 1309, 1150, 1162, 1161, 1160, 1239,
 /*   120 */  1266, 1265, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309,
 /*   130 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309,
 /*   140 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1258, 1248, 1006,
 /*   150 */  1006, 1309, 1248, 1248, 1248, 1248, 1248, 1248, 1244, 1081,
 /*   160 */  1072, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309,
 /*   170 */  1309, 1236, 1234, 1309, 1195, 1309, 1309, 1309, 1309, 1309,
 /*   180 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309,
 /*   190 */  1309, 1309, 1309, 1309, 1309, 1309, 1077, 1309, 1309, 1309,
 /*   200 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1260,
 /*   210 */  1309, 1209, 1077, 1077, 1077, 1079, 1061, 1071,  985, 1116,
 /*   220 */  1095, 1095, 1298, 1116, 1298, 1023, 1280, 1020, 1106, 1095,
 /*   230 */  1177, 1106, 1106, 1078, 1071, 1309, 1301, 1086, 1086, 1300,
 /*   240 */  1300, 1086, 1125, 1051, 1116, 1057, 1057, 1057, 1057, 1086,
 /*   250 */   997, 1116, 1125, 1051, 1051, 1116, 1086,  997, 1213, 1295,
 /*   260 */  1086, 1086,  997, 1188, 1086,  997, 1086,  997, 1188, 1049,
 /*   270 */  1049, 1049, 1038, 1188, 1049, 1023, 1049, 1038, 1049, 1049,
 /*   280 */  1099, 1094, 1099, 1094, 1099, 1094, 1099, 1094, 1086, 1183,
 /*   290 */  1086, 1309, 1188, 1192, 1192, 1188, 1111, 1100, 1109, 1107,
 /*   300 */  1116, 1003, 1041, 1263, 1263, 1259, 1259, 1259, 1259, 1306,
 /*   310 */  1306, 1244, 1275, 1275, 1025, 1025, 1275, 1309, 1309, 1309,
 /*   320 */  1309, 1309, 1309, 1270, 1309, 1197, 1309, 1309, 1309, 1309,
 /*   330 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309,
 /*   340 */  1309, 1309, 1131, 1309,  981, 1241, 1309, 1309, 1240, 1309,
 /*   350 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309,
 /*   360 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1297, 1309, 1309,
 /*   370 */  1309, 1309, 1309, 1309, 1212, 1211, 1309, 1309, 1309, 1309,
 /*   380 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309,
 /*   390 */  1309, 1309, 1309, 1309, 1309, 1309, 1309, 1309, 1063, 1309,
 /*   400 */  1309, 1309, 1284, 1309, 1309, 1309, 1309, 1309, 1309, 1309,
 /*   410 */  1108, 1309, 1101, 1309, 1309, 1288, 1309, 1309, 1309, 1309,
 /*   420 */  1309, 1309, 1309, 1309, 1309, 1309, 1250, 1309, 1309, 1309,
 /*   430 */  1249, 1309, 1309, 1309, 1309, 1309, 1133, 1309, 1132, 1136,
 /*   440 */  1309,  991, 1309,
};
/********** End of lemon-generated parsing tables *****************************/

/* The next table maps tokens (terminal symbols) into fallback tokens.  
** If a construct like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
**
** This feature can be used, for example, to cause some keywords in a language
** to revert to identifiers if they keyword does not apply in the context where
** it appears.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
    0,  /*          $ => nothing */
    0,  /*       SEMI => nothing */
   55,  /*    EXPLAIN => ID */
   55,  /*      QUERY => ID */
   55,  /*       PLAN => ID */
   55,  /*      BEGIN => ID */
    0,  /* TRANSACTION => nothing */
   55,  /*   DEFERRED => ID */
   55,  /*  IMMEDIATE => ID */
   55,  /*  EXCLUSIVE => ID */
    0,  /*     COMMIT => nothing */
   55,  /*        END => ID */
   55,  /*   ROLLBACK => ID */
   55,  /*  SAVEPOINT => ID */
   55,  /*    RELEASE => ID */
    0,  /*         TO => nothing */
    0,  /*      TABLE => nothing */
    0,  /*     CREATE => nothing */
   55,  /*         IF => ID */
    0,  /*        NOT => nothing */
    0,  /*     EXISTS => nothing */
   55,  /*       TEMP => ID */
    0,  /*         LP => nothing */
    0,  /*         RP => nothing */
    0,  /*         AS => nothing */
   55,  /*    WITHOUT => ID */
    0,  /*      COMMA => nothing */
    0,  /*         OR => nothing */
    0,  /*        AND => nothing */
    0,  /*         IS => nothing */
   55,  /*      MATCH => ID */
   55,  /*    LIKE_KW => ID */
    0,  /*    BETWEEN => nothing */
    0,  /*         IN => nothing */
    0,  /*     ISNULL => nothing */
    0,  /*    NOTNULL => nothing */
    0,  /*         NE => nothing */
    0,  /*         EQ => nothing */
    0,  /*         GT => nothing */
    0,  /*         LE => nothing */
    0,  /*         LT => nothing */
    0,  /*         GE => nothing */
    0,  /*     ESCAPE => nothing */
    0,  /*     BITAND => nothing */
    0,  /*      BITOR => nothing */
    0,  /*     LSHIFT => nothing */
    0,  /*     RSHIFT => nothing */
    0,  /*       PLUS => nothing */
    0,  /*      MINUS => nothing */
    0,  /*       STAR => nothing */
    0,  /*      SLASH => nothing */
    0,  /*        REM => nothing */
    0,  /*     CONCAT => nothing */
    0,  /*    COLLATE => nothing */
    0,  /*     BITNOT => nothing */
    0,  /*         ID => nothing */
    0,  /*    INDEXED => nothing */
   55,  /*      ABORT => ID */
   55,  /*     ACTION => ID */
   55,  /*      AFTER => ID */
   55,  /*    ANALYZE => ID */
   55,  /*        ASC => ID */
   55,  /*     ATTACH => ID */
   55,  /*     BEFORE => ID */
   55,  /*         BY => ID */
   55,  /*    CASCADE => ID */
   55,  /*       CAST => ID */
   55,  /*   COLUMNKW => ID */
   55,  /*   CONFLICT => ID */
   55,  /*   DATABASE => ID */
   55,  /*       DESC => ID */
   55,  /*     DETACH => ID */
   55,  /*       EACH => ID */
   55,  /*       FAIL => ID */
   55,  /*        FOR => ID */
   55,  /*     IGNORE => ID */
   55,  /*  INITIALLY => ID */
   55,  /*    INSTEAD => ID */
   55,  /*         NO => ID */
   55,  /*        KEY => ID */
   55,  /*         OF => ID */
   55,  /*     OFFSET => ID */
   55,  /*     PRAGMA => ID */
   55,  /*      RAISE => ID */
   55,  /*  RECURSIVE => ID */
   55,  /*    REPLACE => ID */
   55,  /*   RESTRICT => ID */
   55,  /*        ROW => ID */
   55,  /*    TRIGGER => ID */
   55,  /*     VACUUM => ID */
   55,  /*       VIEW => ID */
   55,  /*    VIRTUAL => ID */
   55,  /*       WITH => ID */
   55,  /*    REINDEX => ID */
   55,  /*     RENAME => ID */
   55,  /*   CTIME_KW => ID */
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
**
** After the "shift" half of a SHIFTREDUCE action, the stateno field
** actually contains the reduce action for the second half of the
** SHIFTREDUCE.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number, or reduce action in SHIFTREDUCE */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  yyStackEntry *yytos;          /* Pointer to top element of the stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyhwm;                    /* High-water mark of the stack */
#endif
#ifndef YYNOERRORRECOVERY
  int yyerrcnt;                 /* Shifts left before out of the error */
#endif
  sqlite3ParserARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
  yyStackEntry yystk0;          /* First stack entry */
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
void sqlite3ParserTrace(FILE *TraceFILE, char *zTracePrompt){
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
  "$",             "SEMI",          "EXPLAIN",       "QUERY",       
  "PLAN",          "BEGIN",         "TRANSACTION",   "DEFERRED",    
  "IMMEDIATE",     "EXCLUSIVE",     "COMMIT",        "END",         
  "ROLLBACK",      "SAVEPOINT",     "RELEASE",       "TO",          
  "TABLE",         "CREATE",        "IF",            "NOT",         
  "EXISTS",        "TEMP",          "LP",            "RP",          
  "AS",            "WITHOUT",       "COMMA",         "OR",          
  "AND",           "IS",            "MATCH",         "LIKE_KW",     
  "BETWEEN",       "IN",            "ISNULL",        "NOTNULL",     
  "NE",            "EQ",            "GT",            "LE",          
  "LT",            "GE",            "ESCAPE",        "BITAND",      
  "BITOR",         "LSHIFT",        "RSHIFT",        "PLUS",        
  "MINUS",         "STAR",          "SLASH",         "REM",         
  "CONCAT",        "COLLATE",       "BITNOT",        "ID",          
  "INDEXED",       "ABORT",         "ACTION",        "AFTER",       
  "ANALYZE",       "ASC",           "ATTACH",        "BEFORE",      
  "BY",            "CASCADE",       "CAST",          "COLUMNKW",    
  "CONFLICT",      "DATABASE",      "DESC",          "DETACH",      
  "EACH",          "FAIL",          "FOR",           "IGNORE",      
  "INITIALLY",     "INSTEAD",       "NO",            "KEY",         
  "OF",            "OFFSET",        "PRAGMA",        "RAISE",       
  "RECURSIVE",     "REPLACE",       "RESTRICT",      "ROW",         
  "TRIGGER",       "VACUUM",        "VIEW",          "VIRTUAL",     
  "WITH",          "REINDEX",       "RENAME",        "CTIME_KW",    
  "ANY",           "STRING",        "JOIN_KW",       "CONSTRAINT",  
  "DEFAULT",       "NULL",          "PRIMARY",       "UNIQUE",      
  "CHECK",         "REFERENCES",    "AUTOINCR",      "ON",          
  "INSERT",        "DELETE",        "UPDATE",        "SET",         
  "DEFERRABLE",    "FOREIGN",       "DROP",          "UNION",       
  "ALL",           "EXCEPT",        "INTERSECT",     "SELECT",      
  "VALUES",        "DISTINCT",      "DOT",           "FROM",        
  "JOIN",          "USING",         "ORDER",         "GROUP",       
  "HAVING",        "LIMIT",         "WHERE",         "INTO",        
  "INTEGER",       "FLOAT",         "BLOB",          "VARIABLE",    
  "CASE",          "WHEN",          "THEN",          "ELSE",        
  "INDEX",         "ALTER",         "ADD",           "error",       
  "input",         "cmdlist",       "ecmd",          "explain",     
  "cmdx",          "cmd",           "transtype",     "trans_opt",   
  "nm",            "savepoint_opt",  "create_table",  "create_table_args",
  "createkw",      "temp",          "ifnotexists",   "dbnm",        
  "columnlist",    "conslist_opt",  "table_options",  "select",      
  "columnname",    "carglist",      "typetoken",     "typename",    
  "signed",        "plus_num",      "minus_num",     "ccons",       
  "term",          "expr",          "onconf",        "sortorder",   
  "autoinc",       "eidlist_opt",   "refargs",       "defer_subclause",
  "refarg",        "refact",        "init_deferred_pred_opt",  "conslist",    
  "tconscomma",    "tcons",         "sortlist",      "eidlist",     
  "defer_subclause_opt",  "orconf",        "resolvetype",   "raisetype",   
  "ifexists",      "fullname",      "selectnowith",  "oneselect",   
  "with",          "multiselect_op",  "distinct",      "selcollist",  
  "from",          "where_opt",     "groupby_opt",   "having_opt",  
  "orderby_opt",   "limit_opt",     "values",        "nexprlist",   
  "exprlist",      "sclp",          "as",            "seltablist",  
  "stl_prefix",    "joinop",        "indexed_opt",   "on_opt",      
  "using_opt",     "idlist",        "setlist",       "insert_cmd",  
  "idlist_opt",    "likeop",        "between_op",    "in_op",       
  "paren_exprlist",  "case_operand",  "case_exprlist",  "case_else",   
  "uniqueflag",    "collate",       "nmnum",         "trigger_decl",
  "trigger_cmd_list",  "trigger_time",  "trigger_event",  "foreach_clause",
  "when_clause",   "trigger_cmd",   "trnm",          "tridxby",     
  "database_kw_opt",  "key_opt",       "add_column_fullname",  "kwcolumn_opt",
  "create_vtab",   "vtabarglist",   "vtabarg",       "vtabargtoken",
  "lp",            "anylist",       "wqlist",      
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "explain ::= EXPLAIN",
 /*   1 */ "explain ::= EXPLAIN QUERY PLAN",
 /*   2 */ "cmdx ::= cmd",
 /*   3 */ "cmd ::= BEGIN transtype trans_opt",
 /*   4 */ "transtype ::=",
 /*   5 */ "transtype ::= DEFERRED",
 /*   6 */ "transtype ::= IMMEDIATE",
 /*   7 */ "transtype ::= EXCLUSIVE",
 /*   8 */ "cmd ::= COMMIT trans_opt",
 /*   9 */ "cmd ::= END trans_opt",
 /*  10 */ "cmd ::= ROLLBACK trans_opt",
 /*  11 */ "cmd ::= SAVEPOINT nm",
 /*  12 */ "cmd ::= RELEASE savepoint_opt nm",
 /*  13 */ "cmd ::= ROLLBACK trans_opt TO savepoint_opt nm",
 /*  14 */ "create_table ::= createkw temp TABLE ifnotexists nm dbnm",
 /*  15 */ "createkw ::= CREATE",
 /*  16 */ "ifnotexists ::=",
 /*  17 */ "ifnotexists ::= IF NOT EXISTS",
 /*  18 */ "temp ::= TEMP",
 /*  19 */ "temp ::=",
 /*  20 */ "create_table_args ::= LP columnlist conslist_opt RP table_options",
 /*  21 */ "create_table_args ::= AS select",
 /*  22 */ "table_options ::=",
 /*  23 */ "table_options ::= WITHOUT nm",
 /*  24 */ "columnname ::= nm typetoken",
 /*  25 */ "typetoken ::=",
 /*  26 */ "typetoken ::= typename LP signed RP",
 /*  27 */ "typetoken ::= typename LP signed COMMA signed RP",
 /*  28 */ "typename ::= typename ID|STRING",
 /*  29 */ "ccons ::= CONSTRAINT nm",
 /*  30 */ "ccons ::= DEFAULT term",
 /*  31 */ "ccons ::= DEFAULT LP expr RP",
 /*  32 */ "ccons ::= DEFAULT PLUS term",
 /*  33 */ "ccons ::= DEFAULT MINUS term",
 /*  34 */ "ccons ::= DEFAULT ID|INDEXED",
 /*  35 */ "ccons ::= NOT NULL onconf",
 /*  36 */ "ccons ::= PRIMARY KEY sortorder onconf autoinc",
 /*  37 */ "ccons ::= UNIQUE onconf",
 /*  38 */ "ccons ::= CHECK LP expr RP",
 /*  39 */ "ccons ::= REFERENCES nm eidlist_opt refargs",
 /*  40 */ "ccons ::= defer_subclause",
 /*  41 */ "ccons ::= COLLATE ID|STRING",
 /*  42 */ "autoinc ::=",
 /*  43 */ "autoinc ::= AUTOINCR",
 /*  44 */ "refargs ::=",
 /*  45 */ "refargs ::= refargs refarg",
 /*  46 */ "refarg ::= MATCH nm",
 /*  47 */ "refarg ::= ON INSERT refact",
 /*  48 */ "refarg ::= ON DELETE refact",
 /*  49 */ "refarg ::= ON UPDATE refact",
 /*  50 */ "refact ::= SET NULL",
 /*  51 */ "refact ::= SET DEFAULT",
 /*  52 */ "refact ::= CASCADE",
 /*  53 */ "refact ::= RESTRICT",
 /*  54 */ "refact ::= NO ACTION",
 /*  55 */ "defer_subclause ::= NOT DEFERRABLE init_deferred_pred_opt",
 /*  56 */ "defer_subclause ::= DEFERRABLE init_deferred_pred_opt",
 /*  57 */ "init_deferred_pred_opt ::=",
 /*  58 */ "init_deferred_pred_opt ::= INITIALLY DEFERRED",
 /*  59 */ "init_deferred_pred_opt ::= INITIALLY IMMEDIATE",
 /*  60 */ "conslist_opt ::=",
 /*  61 */ "tconscomma ::= COMMA",
 /*  62 */ "tcons ::= CONSTRAINT nm",
 /*  63 */ "tcons ::= PRIMARY KEY LP sortlist autoinc RP onconf",
 /*  64 */ "tcons ::= UNIQUE LP sortlist RP onconf",
 /*  65 */ "tcons ::= CHECK LP expr RP onconf",
 /*  66 */ "tcons ::= FOREIGN KEY LP eidlist RP REFERENCES nm eidlist_opt refargs defer_subclause_opt",
 /*  67 */ "defer_subclause_opt ::=",
 /*  68 */ "onconf ::=",
 /*  69 */ "onconf ::= ON CONFLICT resolvetype",
 /*  70 */ "orconf ::=",
 /*  71 */ "orconf ::= OR resolvetype",
 /*  72 */ "resolvetype ::= IGNORE",
 /*  73 */ "resolvetype ::= REPLACE",
 /*  74 */ "cmd ::= DROP TABLE ifexists fullname",
 /*  75 */ "ifexists ::= IF EXISTS",
 /*  76 */ "ifexists ::=",
 /*  77 */ "cmd ::= createkw temp VIEW ifnotexists nm dbnm eidlist_opt AS select",
 /*  78 */ "cmd ::= DROP VIEW ifexists fullname",
 /*  79 */ "cmd ::= select",
 /*  80 */ "select ::= with selectnowith",
 /*  81 */ "selectnowith ::= selectnowith multiselect_op oneselect",
 /*  82 */ "multiselect_op ::= UNION",
 /*  83 */ "multiselect_op ::= UNION ALL",
 /*  84 */ "multiselect_op ::= EXCEPT|INTERSECT",
 /*  85 */ "oneselect ::= SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt",
 /*  86 */ "values ::= VALUES LP nexprlist RP",
 /*  87 */ "values ::= values COMMA LP exprlist RP",
 /*  88 */ "distinct ::= DISTINCT",
 /*  89 */ "distinct ::= ALL",
 /*  90 */ "distinct ::=",
 /*  91 */ "sclp ::=",
 /*  92 */ "selcollist ::= sclp expr as",
 /*  93 */ "selcollist ::= sclp STAR",
 /*  94 */ "selcollist ::= sclp nm DOT STAR",
 /*  95 */ "as ::= AS nm",
 /*  96 */ "as ::=",
 /*  97 */ "from ::=",
 /*  98 */ "from ::= FROM seltablist",
 /*  99 */ "stl_prefix ::= seltablist joinop",
 /* 100 */ "stl_prefix ::=",
 /* 101 */ "seltablist ::= stl_prefix nm dbnm as indexed_opt on_opt using_opt",
 /* 102 */ "seltablist ::= stl_prefix nm dbnm LP exprlist RP as on_opt using_opt",
 /* 103 */ "seltablist ::= stl_prefix LP select RP as on_opt using_opt",
 /* 104 */ "seltablist ::= stl_prefix LP seltablist RP as on_opt using_opt",
 /* 105 */ "dbnm ::=",
 /* 106 */ "dbnm ::= DOT nm",
 /* 107 */ "fullname ::= nm dbnm",
 /* 108 */ "joinop ::= COMMA|JOIN",
 /* 109 */ "joinop ::= JOIN_KW JOIN",
 /* 110 */ "joinop ::= JOIN_KW nm JOIN",
 /* 111 */ "joinop ::= JOIN_KW nm nm JOIN",
 /* 112 */ "on_opt ::= ON expr",
 /* 113 */ "on_opt ::=",
 /* 114 */ "indexed_opt ::=",
 /* 115 */ "indexed_opt ::= INDEXED BY nm",
 /* 116 */ "indexed_opt ::= NOT INDEXED",
 /* 117 */ "using_opt ::= USING LP idlist RP",
 /* 118 */ "using_opt ::=",
 /* 119 */ "orderby_opt ::=",
 /* 120 */ "orderby_opt ::= ORDER BY sortlist",
 /* 121 */ "sortlist ::= sortlist COMMA expr sortorder",
 /* 122 */ "sortlist ::= expr sortorder",
 /* 123 */ "sortorder ::= ASC",
 /* 124 */ "sortorder ::= DESC",
 /* 125 */ "sortorder ::=",
 /* 126 */ "groupby_opt ::=",
 /* 127 */ "groupby_opt ::= GROUP BY nexprlist",
 /* 128 */ "having_opt ::=",
 /* 129 */ "having_opt ::= HAVING expr",
 /* 130 */ "limit_opt ::=",
 /* 131 */ "limit_opt ::= LIMIT expr",
 /* 132 */ "limit_opt ::= LIMIT expr OFFSET expr",
 /* 133 */ "limit_opt ::= LIMIT expr COMMA expr",
 /* 134 */ "cmd ::= with DELETE FROM fullname indexed_opt where_opt",
 /* 135 */ "where_opt ::=",
 /* 136 */ "where_opt ::= WHERE expr",
 /* 137 */ "cmd ::= with UPDATE orconf fullname indexed_opt SET setlist where_opt",
 /* 138 */ "setlist ::= setlist COMMA nm EQ expr",
 /* 139 */ "setlist ::= nm EQ expr",
 /* 140 */ "cmd ::= with insert_cmd INTO fullname idlist_opt select",
 /* 141 */ "cmd ::= with insert_cmd INTO fullname idlist_opt DEFAULT VALUES",
 /* 142 */ "insert_cmd ::= INSERT orconf",
 /* 143 */ "insert_cmd ::= REPLACE",
 /* 144 */ "idlist_opt ::=",
 /* 145 */ "idlist_opt ::= LP idlist RP",
 /* 146 */ "idlist ::= idlist COMMA nm",
 /* 147 */ "idlist ::= nm",
 /* 148 */ "expr ::= LP expr RP",
 /* 149 */ "term ::= NULL",
 /* 150 */ "expr ::= ID|INDEXED",
 /* 151 */ "expr ::= JOIN_KW",
 /* 152 */ "expr ::= nm DOT nm",
 /* 153 */ "expr ::= nm DOT nm DOT nm",
 /* 154 */ "term ::= INTEGER|FLOAT|BLOB",
 /* 155 */ "term ::= STRING",
 /* 156 */ "expr ::= VARIABLE",
 /* 157 */ "expr ::= expr COLLATE ID|STRING",
 /* 158 */ "expr ::= CAST LP expr AS typetoken RP",
 /* 159 */ "expr ::= ID|INDEXED LP distinct exprlist RP",
 /* 160 */ "expr ::= ID|INDEXED LP STAR RP",
 /* 161 */ "term ::= CTIME_KW",
 /* 162 */ "expr ::= expr AND expr",
 /* 163 */ "expr ::= expr OR expr",
 /* 164 */ "expr ::= expr LT|GT|GE|LE expr",
 /* 165 */ "expr ::= expr EQ|NE expr",
 /* 166 */ "expr ::= expr BITAND|BITOR|LSHIFT|RSHIFT expr",
 /* 167 */ "expr ::= expr PLUS|MINUS expr",
 /* 168 */ "expr ::= expr STAR|SLASH|REM expr",
 /* 169 */ "expr ::= expr CONCAT expr",
 /* 170 */ "likeop ::= LIKE_KW|MATCH",
 /* 171 */ "likeop ::= NOT LIKE_KW|MATCH",
 /* 172 */ "expr ::= expr likeop expr",
 /* 173 */ "expr ::= expr likeop expr ESCAPE expr",
 /* 174 */ "expr ::= expr ISNULL|NOTNULL",
 /* 175 */ "expr ::= expr NOT NULL",
 /* 176 */ "expr ::= expr IS expr",
 /* 177 */ "expr ::= expr IS NOT expr",
 /* 178 */ "expr ::= NOT expr",
 /* 179 */ "expr ::= BITNOT expr",
 /* 180 */ "expr ::= MINUS expr",
 /* 181 */ "expr ::= PLUS expr",
 /* 182 */ "between_op ::= BETWEEN",
 /* 183 */ "between_op ::= NOT BETWEEN",
 /* 184 */ "expr ::= expr between_op expr AND expr",
 /* 185 */ "in_op ::= IN",
 /* 186 */ "in_op ::= NOT IN",
 /* 187 */ "expr ::= expr in_op LP exprlist RP",
 /* 188 */ "expr ::= LP select RP",
 /* 189 */ "expr ::= expr in_op LP select RP",
 /* 190 */ "expr ::= expr in_op nm dbnm paren_exprlist",
 /* 191 */ "expr ::= EXISTS LP select RP",
 /* 192 */ "expr ::= CASE case_operand case_exprlist case_else END",
 /* 193 */ "case_exprlist ::= case_exprlist WHEN expr THEN expr",
 /* 194 */ "case_exprlist ::= WHEN expr THEN expr",
 /* 195 */ "case_else ::= ELSE expr",
 /* 196 */ "case_else ::=",
 /* 197 */ "case_operand ::= expr",
 /* 198 */ "case_operand ::=",
 /* 199 */ "exprlist ::=",
 /* 200 */ "nexprlist ::= nexprlist COMMA expr",
 /* 201 */ "nexprlist ::= expr",
 /* 202 */ "paren_exprlist ::=",
 /* 203 */ "paren_exprlist ::= LP exprlist RP",
 /* 204 */ "cmd ::= createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP sortlist RP where_opt",
 /* 205 */ "uniqueflag ::= UNIQUE",
 /* 206 */ "uniqueflag ::=",
 /* 207 */ "eidlist_opt ::=",
 /* 208 */ "eidlist_opt ::= LP eidlist RP",
 /* 209 */ "eidlist ::= eidlist COMMA nm collate sortorder",
 /* 210 */ "eidlist ::= nm collate sortorder",
 /* 211 */ "collate ::=",
 /* 212 */ "collate ::= COLLATE ID|STRING",
 /* 213 */ "cmd ::= DROP INDEX ifexists fullname",
 /* 214 */ "cmd ::= VACUUM",
 /* 215 */ "cmd ::= VACUUM nm",
 /* 216 */ "cmd ::= PRAGMA nm dbnm",
 /* 217 */ "cmd ::= PRAGMA nm dbnm EQ nmnum",
 /* 218 */ "cmd ::= PRAGMA nm dbnm LP nmnum RP",
 /* 219 */ "cmd ::= PRAGMA nm dbnm EQ minus_num",
 /* 220 */ "cmd ::= PRAGMA nm dbnm LP minus_num RP",
 /* 221 */ "plus_num ::= PLUS INTEGER|FLOAT",
 /* 222 */ "minus_num ::= MINUS INTEGER|FLOAT",
 /* 223 */ "cmd ::= createkw trigger_decl BEGIN trigger_cmd_list END",
 /* 224 */ "trigger_decl ::= temp TRIGGER ifnotexists nm dbnm trigger_time trigger_event ON fullname foreach_clause when_clause",
 /* 225 */ "trigger_time ::= BEFORE",
 /* 226 */ "trigger_time ::= AFTER",
 /* 227 */ "trigger_time ::= INSTEAD OF",
 /* 228 */ "trigger_time ::=",
 /* 229 */ "trigger_event ::= DELETE|INSERT",
 /* 230 */ "trigger_event ::= UPDATE",
 /* 231 */ "trigger_event ::= UPDATE OF idlist",
 /* 232 */ "when_clause ::=",
 /* 233 */ "when_clause ::= WHEN expr",
 /* 234 */ "trigger_cmd_list ::= trigger_cmd_list trigger_cmd SEMI",
 /* 235 */ "trigger_cmd_list ::= trigger_cmd SEMI",
 /* 236 */ "trnm ::= nm DOT nm",
 /* 237 */ "tridxby ::= INDEXED BY nm",
 /* 238 */ "tridxby ::= NOT INDEXED",
 /* 239 */ "trigger_cmd ::= UPDATE orconf trnm tridxby SET setlist where_opt",
 /* 240 */ "trigger_cmd ::= insert_cmd INTO trnm idlist_opt select",
 /* 241 */ "trigger_cmd ::= DELETE FROM trnm tridxby where_opt",
 /* 242 */ "trigger_cmd ::= select",
 /* 243 */ "expr ::= RAISE LP IGNORE RP",
 /* 244 */ "expr ::= RAISE LP raisetype COMMA nm RP",
 /* 245 */ "raisetype ::= ROLLBACK",
 /* 246 */ "raisetype ::= ABORT",
 /* 247 */ "raisetype ::= FAIL",
 /* 248 */ "cmd ::= DROP TRIGGER ifexists fullname",
 /* 249 */ "cmd ::= ATTACH database_kw_opt expr AS expr key_opt",
 /* 250 */ "cmd ::= DETACH database_kw_opt expr",
 /* 251 */ "key_opt ::=",
 /* 252 */ "key_opt ::= KEY expr",
 /* 253 */ "cmd ::= REINDEX",
 /* 254 */ "cmd ::= REINDEX nm dbnm",
 /* 255 */ "cmd ::= ANALYZE",
 /* 256 */ "cmd ::= ANALYZE nm dbnm",
 /* 257 */ "cmd ::= ALTER TABLE fullname RENAME TO nm",
 /* 258 */ "cmd ::= ALTER TABLE add_column_fullname ADD kwcolumn_opt columnname carglist",
 /* 259 */ "add_column_fullname ::= fullname",
 /* 260 */ "cmd ::= create_vtab",
 /* 261 */ "cmd ::= create_vtab LP vtabarglist RP",
 /* 262 */ "create_vtab ::= createkw VIRTUAL TABLE ifnotexists nm dbnm USING nm",
 /* 263 */ "vtabarg ::=",
 /* 264 */ "vtabargtoken ::= ANY",
 /* 265 */ "vtabargtoken ::= lp anylist RP",
 /* 266 */ "lp ::= LP",
 /* 267 */ "with ::=",
 /* 268 */ "with ::= WITH wqlist",
 /* 269 */ "with ::= WITH RECURSIVE wqlist",
 /* 270 */ "wqlist ::= nm eidlist_opt AS LP select RP",
 /* 271 */ "wqlist ::= wqlist COMMA nm eidlist_opt AS LP select RP",
 /* 272 */ "input ::= cmdlist",
 /* 273 */ "cmdlist ::= cmdlist ecmd",
 /* 274 */ "cmdlist ::= ecmd",
 /* 275 */ "ecmd ::= SEMI",
 /* 276 */ "ecmd ::= explain cmdx SEMI",
 /* 277 */ "explain ::=",
 /* 278 */ "trans_opt ::=",
 /* 279 */ "trans_opt ::= TRANSACTION",
 /* 280 */ "trans_opt ::= TRANSACTION nm",
 /* 281 */ "savepoint_opt ::= SAVEPOINT",
 /* 282 */ "savepoint_opt ::=",
 /* 283 */ "cmd ::= create_table create_table_args",
 /* 284 */ "columnlist ::= columnlist COMMA columnname carglist",
 /* 285 */ "columnlist ::= columnname carglist",
 /* 286 */ "nm ::= ID|INDEXED",
 /* 287 */ "nm ::= STRING",
 /* 288 */ "nm ::= JOIN_KW",
 /* 289 */ "typetoken ::= typename",
 /* 290 */ "typename ::= ID|STRING",
 /* 291 */ "signed ::= plus_num",
 /* 292 */ "signed ::= minus_num",
 /* 293 */ "carglist ::= carglist ccons",
 /* 294 */ "carglist ::=",
 /* 295 */ "ccons ::= NULL onconf",
 /* 296 */ "conslist_opt ::= COMMA conslist",
 /* 297 */ "conslist ::= conslist tconscomma tcons",
 /* 298 */ "conslist ::= tcons",
 /* 299 */ "tconscomma ::=",
 /* 300 */ "defer_subclause_opt ::= defer_subclause",
 /* 301 */ "resolvetype ::= raisetype",
 /* 302 */ "selectnowith ::= oneselect",
 /* 303 */ "oneselect ::= values",
 /* 304 */ "sclp ::= selcollist COMMA",
 /* 305 */ "as ::= ID|STRING",
 /* 306 */ "expr ::= term",
 /* 307 */ "exprlist ::= nexprlist",
 /* 308 */ "nmnum ::= plus_num",
 /* 309 */ "nmnum ::= nm",
 /* 310 */ "nmnum ::= ON",
 /* 311 */ "nmnum ::= DELETE",
 /* 312 */ "nmnum ::= DEFAULT",
 /* 313 */ "plus_num ::= INTEGER|FLOAT",
 /* 314 */ "foreach_clause ::=",
 /* 315 */ "foreach_clause ::= FOR EACH ROW",
 /* 316 */ "trnm ::= nm",
 /* 317 */ "tridxby ::=",
 /* 318 */ "database_kw_opt ::= DATABASE",
 /* 319 */ "database_kw_opt ::=",
 /* 320 */ "kwcolumn_opt ::=",
 /* 321 */ "kwcolumn_opt ::= COLUMNKW",
 /* 322 */ "vtabarglist ::= vtabarg",
 /* 323 */ "vtabarglist ::= vtabarglist COMMA vtabarg",
 /* 324 */ "vtabarg ::= vtabarg vtabargtoken",
 /* 325 */ "anylist ::=",
 /* 326 */ "anylist ::= anylist LP anylist RP",
 /* 327 */ "anylist ::= anylist ANY",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.  Return the number
** of errors.  Return 0 on success.
*/
static int yyGrowStack(yyParser *p){
  int newSize;
  int idx;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  idx = p->yytos ? (int)(p->yytos - p->yystack) : 0;
  if( p->yystack==&p->yystk0 ){
    pNew = malloc(newSize*sizeof(pNew[0]));
    if( pNew ) pNew[0] = p->yystk0;
  }else{
    pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  }
  if( pNew ){
    p->yystack = pNew;
    p->yytos = &p->yystack[idx];
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows from %d to %d entries.\n",
              yyTracePrompt, p->yystksz, newSize);
    }
#endif
    p->yystksz = newSize;
  }
  return pNew==0; 
}
#endif

/* Datatype of the argument to the memory allocated passed as the
** second argument to sqlite3ParserAlloc() below.  This can be changed by
** putting an appropriate #define in the %include section of the input
** grammar.
*/
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
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
** to sqlite3Parser and sqlite3ParserFree.
*/
void *sqlite3ParserAlloc(void *(*mallocProc)(YYMALLOCARGTYPE)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (YYMALLOCARGTYPE)sizeof(yyParser) );
  if( pParser ){
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyhwm = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yytos = NULL;
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    if( yyGrowStack(pParser) ){
      pParser->yystack = &pParser->yystk0;
      pParser->yystksz = 1;
    }
#endif
#ifndef YYNOERRORRECOVERY
    pParser->yyerrcnt = -1;
#endif
    pParser->yytos = pParser->yystack;
    pParser->yystack[0].stateno = 0;
    pParser->yystack[0].major = 0;
  }
  return pParser;
}

/* The following function deletes the "minor type" or semantic value
** associated with a symbol.  The symbol can be either a terminal
** or nonterminal. "yymajor" is the symbol code, and "yypminor" is
** a pointer to the value to be deleted.  The code used to do the 
** deletions is derived from the %destructor and/or %token_destructor
** directives of the input grammar.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  sqlite3ParserARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are *not* used
    ** inside the C code.
    */
/********* Begin destructor definitions ***************************************/
    case 163: /* select */
    case 194: /* selectnowith */
    case 195: /* oneselect */
    case 206: /* values */
{
#line 410 "parse.y"
sqlite3SelectDelete(pParse->db, (yypminor->yy243));
#line 1550 "parse.c"
}
      break;
    case 172: /* term */
    case 173: /* expr */
{
#line 831 "parse.y"
sqlite3ExprDelete(pParse->db, (yypminor->yy190).pExpr);
#line 1558 "parse.c"
}
      break;
    case 177: /* eidlist_opt */
    case 186: /* sortlist */
    case 187: /* eidlist */
    case 199: /* selcollist */
    case 202: /* groupby_opt */
    case 204: /* orderby_opt */
    case 207: /* nexprlist */
    case 208: /* exprlist */
    case 209: /* sclp */
    case 218: /* setlist */
    case 224: /* paren_exprlist */
    case 226: /* case_exprlist */
{
#line 1239 "parse.y"
sqlite3ExprListDelete(pParse->db, (yypminor->yy148));
#line 1576 "parse.c"
}
      break;
    case 193: /* fullname */
    case 200: /* from */
    case 211: /* seltablist */
    case 212: /* stl_prefix */
{
#line 642 "parse.y"
sqlite3SrcListDelete(pParse->db, (yypminor->yy185));
#line 1586 "parse.c"
}
      break;
    case 196: /* with */
    case 250: /* wqlist */
{
#line 1516 "parse.y"
sqlite3WithDelete(pParse->db, (yypminor->yy285));
#line 1594 "parse.c"
}
      break;
    case 201: /* where_opt */
    case 203: /* having_opt */
    case 215: /* on_opt */
    case 225: /* case_operand */
    case 227: /* case_else */
    case 236: /* when_clause */
    case 241: /* key_opt */
{
#line 758 "parse.y"
sqlite3ExprDelete(pParse->db, (yypminor->yy72));
#line 1607 "parse.c"
}
      break;
    case 216: /* using_opt */
    case 217: /* idlist */
    case 220: /* idlist_opt */
{
#line 676 "parse.y"
sqlite3IdListDelete(pParse->db, (yypminor->yy254));
#line 1616 "parse.c"
}
      break;
    case 232: /* trigger_cmd_list */
    case 237: /* trigger_cmd */
{
#line 1353 "parse.y"
sqlite3DeleteTriggerStep(pParse->db, (yypminor->yy145));
#line 1624 "parse.c"
}
      break;
    case 234: /* trigger_event */
{
#line 1339 "parse.y"
sqlite3IdListDelete(pParse->db, (yypminor->yy332).b);
#line 1631 "parse.c"
}
      break;
/********* End destructor definitions *****************************************/
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
*/
static void yy_pop_parser_stack(yyParser *pParser){
  yyStackEntry *yytos;
  assert( pParser->yytos!=0 );
  assert( pParser->yytos > pParser->yystack );
  yytos = pParser->yytos--;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yy_destructor(pParser, yytos->major, &yytos->minor);
}

/* 
** Deallocate and destroy a parser.  Destructors are called for
** all stack elements before shutting the parser down.
**
** If the YYPARSEFREENEVERNULL macro exists (for example because it
** is defined in a %include section of the input grammar) then it is
** assumed that the input pointer is never NULL.
*/
void sqlite3ParserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
#ifndef YYPARSEFREENEVERNULL
  if( pParser==0 ) return;
#endif
  while( pParser->yytos>pParser->yystack ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  if( pParser->yystack!=&pParser->yystk0 ) free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int sqlite3ParserStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyhwm;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
*/
static unsigned int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yytos->stateno;
 
  if( stateno>=YY_MIN_REDUCE ) return stateno;
  assert( stateno <= YY_SHIFT_COUNT );
  do{
    i = yy_shift_ofst[stateno];
    if( i==YY_SHIFT_USE_DFLT ) return yy_default[stateno];
    assert( iLookAhead!=YYNOCODE );
    i += iLookAhead;
    if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
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
          assert( yyFallback[iFallback]==0 ); /* Fallback loop must terminate */
          iLookAhead = iFallback;
          continue;
        }
#endif
#ifdef YYWILDCARD
        {
          int j = i - iLookAhead + YYWILDCARD;
          if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
            j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
            j<YY_ACTTAB_COUNT &&
#endif
            yy_lookahead[j]==YYWILDCARD
          ){
#ifndef NDEBUG
            if( yyTraceFILE ){
              fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
                 yyTracePrompt, yyTokenName[iLookAhead],
                 yyTokenName[YYWILDCARD]);
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
  }while(1);
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser){
   sqlite3ParserARG_FETCH;
   yypParser->yytos--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
/******** Begin %stack_overflow code ******************************************/
#line 37 "parse.y"

  sqlite3ErrorMsg(pParse, "parser stack overflow");
#line 1808 "parse.c"
/******** End %stack_overflow code ********************************************/
   sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Print tracing information for a SHIFT action
*/
#ifndef NDEBUG
static void yyTraceShift(yyParser *yypParser, int yyNewState){
  if( yyTraceFILE ){
    if( yyNewState<YYNSTATE ){
      fprintf(yyTraceFILE,"%sShift '%s', go to state %d\n",
         yyTracePrompt,yyTokenName[yypParser->yytos->major],
         yyNewState);
    }else{
      fprintf(yyTraceFILE,"%sShift '%s'\n",
         yyTracePrompt,yyTokenName[yypParser->yytos->major]);
    }
  }
}
#else
# define yyTraceShift(X,Y)
#endif

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  sqlite3ParserTOKENTYPE yyMinor        /* The minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yytos++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
    yypParser->yyhwm++;
    assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack) );
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yytos>=&yypParser->yystack[YYSTACKDEPTH] ){
    yyStackOverflow(yypParser);
    return;
  }
#else
  if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz] ){
    if( yyGrowStack(yypParser) ){
      yyStackOverflow(yypParser);
      return;
    }
  }
#endif
  if( yyNewState > YY_MAX_SHIFT ){
    yyNewState += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
  }
  yytos = yypParser->yytos;
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor.yy0 = yyMinor;
  yyTraceShift(yypParser, yyNewState);
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 147, 1 },
  { 147, 3 },
  { 148, 1 },
  { 149, 3 },
  { 150, 0 },
  { 150, 1 },
  { 150, 1 },
  { 150, 1 },
  { 149, 2 },
  { 149, 2 },
  { 149, 2 },
  { 149, 2 },
  { 149, 3 },
  { 149, 5 },
  { 154, 6 },
  { 156, 1 },
  { 158, 0 },
  { 158, 3 },
  { 157, 1 },
  { 157, 0 },
  { 155, 5 },
  { 155, 2 },
  { 162, 0 },
  { 162, 2 },
  { 164, 2 },
  { 166, 0 },
  { 166, 4 },
  { 166, 6 },
  { 167, 2 },
  { 171, 2 },
  { 171, 2 },
  { 171, 4 },
  { 171, 3 },
  { 171, 3 },
  { 171, 2 },
  { 171, 3 },
  { 171, 5 },
  { 171, 2 },
  { 171, 4 },
  { 171, 4 },
  { 171, 1 },
  { 171, 2 },
  { 176, 0 },
  { 176, 1 },
  { 178, 0 },
  { 178, 2 },
  { 180, 2 },
  { 180, 3 },
  { 180, 3 },
  { 180, 3 },
  { 181, 2 },
  { 181, 2 },
  { 181, 1 },
  { 181, 1 },
  { 181, 2 },
  { 179, 3 },
  { 179, 2 },
  { 182, 0 },
  { 182, 2 },
  { 182, 2 },
  { 161, 0 },
  { 184, 1 },
  { 185, 2 },
  { 185, 7 },
  { 185, 5 },
  { 185, 5 },
  { 185, 10 },
  { 188, 0 },
  { 174, 0 },
  { 174, 3 },
  { 189, 0 },
  { 189, 2 },
  { 190, 1 },
  { 190, 1 },
  { 149, 4 },
  { 192, 2 },
  { 192, 0 },
  { 149, 9 },
  { 149, 4 },
  { 149, 1 },
  { 163, 2 },
  { 194, 3 },
  { 197, 1 },
  { 197, 2 },
  { 197, 1 },
  { 195, 9 },
  { 206, 4 },
  { 206, 5 },
  { 198, 1 },
  { 198, 1 },
  { 198, 0 },
  { 209, 0 },
  { 199, 3 },
  { 199, 2 },
  { 199, 4 },
  { 210, 2 },
  { 210, 0 },
  { 200, 0 },
  { 200, 2 },
  { 212, 2 },
  { 212, 0 },
  { 211, 7 },
  { 211, 9 },
  { 211, 7 },
  { 211, 7 },
  { 159, 0 },
  { 159, 2 },
  { 193, 2 },
  { 213, 1 },
  { 213, 2 },
  { 213, 3 },
  { 213, 4 },
  { 215, 2 },
  { 215, 0 },
  { 214, 0 },
  { 214, 3 },
  { 214, 2 },
  { 216, 4 },
  { 216, 0 },
  { 204, 0 },
  { 204, 3 },
  { 186, 4 },
  { 186, 2 },
  { 175, 1 },
  { 175, 1 },
  { 175, 0 },
  { 202, 0 },
  { 202, 3 },
  { 203, 0 },
  { 203, 2 },
  { 205, 0 },
  { 205, 2 },
  { 205, 4 },
  { 205, 4 },
  { 149, 6 },
  { 201, 0 },
  { 201, 2 },
  { 149, 8 },
  { 218, 5 },
  { 218, 3 },
  { 149, 6 },
  { 149, 7 },
  { 219, 2 },
  { 219, 1 },
  { 220, 0 },
  { 220, 3 },
  { 217, 3 },
  { 217, 1 },
  { 173, 3 },
  { 172, 1 },
  { 173, 1 },
  { 173, 1 },
  { 173, 3 },
  { 173, 5 },
  { 172, 1 },
  { 172, 1 },
  { 173, 1 },
  { 173, 3 },
  { 173, 6 },
  { 173, 5 },
  { 173, 4 },
  { 172, 1 },
  { 173, 3 },
  { 173, 3 },
  { 173, 3 },
  { 173, 3 },
  { 173, 3 },
  { 173, 3 },
  { 173, 3 },
  { 173, 3 },
  { 221, 1 },
  { 221, 2 },
  { 173, 3 },
  { 173, 5 },
  { 173, 2 },
  { 173, 3 },
  { 173, 3 },
  { 173, 4 },
  { 173, 2 },
  { 173, 2 },
  { 173, 2 },
  { 173, 2 },
  { 222, 1 },
  { 222, 2 },
  { 173, 5 },
  { 223, 1 },
  { 223, 2 },
  { 173, 5 },
  { 173, 3 },
  { 173, 5 },
  { 173, 5 },
  { 173, 4 },
  { 173, 5 },
  { 226, 5 },
  { 226, 4 },
  { 227, 2 },
  { 227, 0 },
  { 225, 1 },
  { 225, 0 },
  { 208, 0 },
  { 207, 3 },
  { 207, 1 },
  { 224, 0 },
  { 224, 3 },
  { 149, 12 },
  { 228, 1 },
  { 228, 0 },
  { 177, 0 },
  { 177, 3 },
  { 187, 5 },
  { 187, 3 },
  { 229, 0 },
  { 229, 2 },
  { 149, 4 },
  { 149, 1 },
  { 149, 2 },
  { 149, 3 },
  { 149, 5 },
  { 149, 6 },
  { 149, 5 },
  { 149, 6 },
  { 169, 2 },
  { 170, 2 },
  { 149, 5 },
  { 231, 11 },
  { 233, 1 },
  { 233, 1 },
  { 233, 2 },
  { 233, 0 },
  { 234, 1 },
  { 234, 1 },
  { 234, 3 },
  { 236, 0 },
  { 236, 2 },
  { 232, 3 },
  { 232, 2 },
  { 238, 3 },
  { 239, 3 },
  { 239, 2 },
  { 237, 7 },
  { 237, 5 },
  { 237, 5 },
  { 237, 1 },
  { 173, 4 },
  { 173, 6 },
  { 191, 1 },
  { 191, 1 },
  { 191, 1 },
  { 149, 4 },
  { 149, 6 },
  { 149, 3 },
  { 241, 0 },
  { 241, 2 },
  { 149, 1 },
  { 149, 3 },
  { 149, 1 },
  { 149, 3 },
  { 149, 6 },
  { 149, 7 },
  { 242, 1 },
  { 149, 1 },
  { 149, 4 },
  { 244, 8 },
  { 246, 0 },
  { 247, 1 },
  { 247, 3 },
  { 248, 1 },
  { 196, 0 },
  { 196, 2 },
  { 196, 3 },
  { 250, 6 },
  { 250, 8 },
  { 144, 1 },
  { 145, 2 },
  { 145, 1 },
  { 146, 1 },
  { 146, 3 },
  { 147, 0 },
  { 151, 0 },
  { 151, 1 },
  { 151, 2 },
  { 153, 1 },
  { 153, 0 },
  { 149, 2 },
  { 160, 4 },
  { 160, 2 },
  { 152, 1 },
  { 152, 1 },
  { 152, 1 },
  { 166, 1 },
  { 167, 1 },
  { 168, 1 },
  { 168, 1 },
  { 165, 2 },
  { 165, 0 },
  { 171, 2 },
  { 161, 2 },
  { 183, 3 },
  { 183, 1 },
  { 184, 0 },
  { 188, 1 },
  { 190, 1 },
  { 194, 1 },
  { 195, 1 },
  { 209, 2 },
  { 210, 1 },
  { 173, 1 },
  { 208, 1 },
  { 230, 1 },
  { 230, 1 },
  { 230, 1 },
  { 230, 1 },
  { 230, 1 },
  { 169, 1 },
  { 235, 0 },
  { 235, 3 },
  { 238, 1 },
  { 239, 0 },
  { 240, 1 },
  { 240, 0 },
  { 243, 0 },
  { 243, 1 },
  { 245, 1 },
  { 245, 3 },
  { 246, 2 },
  { 249, 0 },
  { 249, 4 },
  { 249, 2 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  unsigned int yyruleno        /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  sqlite3ParserARG_FETCH;
  yymsp = yypParser->yytos;
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    fprintf(yyTraceFILE, "%sReduce [%s], go to state %d.\n", yyTracePrompt,
      yyRuleName[yyruleno], yymsp[-yysize].stateno);
  }
#endif /* NDEBUG */

  /* Check that the stack is large enough to grow by a single entry
  ** if the RHS of the rule is empty.  This ensures that there is room
  ** enough on the stack to push the LHS value */
  if( yyRuleInfo[yyruleno].nrhs==0 ){
#ifdef YYTRACKMAXSTACKDEPTH
    if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
      yypParser->yyhwm++;
      assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack));
    }
#endif
#if YYSTACKDEPTH>0 
    if( yypParser->yytos>=&yypParser->yystack[YYSTACKDEPTH-1] ){
      yyStackOverflow(yypParser);
      return;
    }
#else
    if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz-1] ){
      if( yyGrowStack(yypParser) ){
        yyStackOverflow(yypParser);
        return;
      }
      yymsp = yypParser->yytos;
    }
#endif
  }

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
/********** Begin reduce actions **********************************************/
        YYMINORTYPE yylhsminor;
      case 0: /* explain ::= EXPLAIN */
#line 127 "parse.y"
{ pParse->explain = 1; }
#line 2274 "parse.c"
        break;
      case 1: /* explain ::= EXPLAIN QUERY PLAN */
#line 128 "parse.y"
{ pParse->explain = 2; }
#line 2279 "parse.c"
        break;
      case 2: /* cmdx ::= cmd */
#line 130 "parse.y"
{ sqlite3FinishCoding(pParse); }
#line 2284 "parse.c"
        break;
      case 3: /* cmd ::= BEGIN transtype trans_opt */
#line 135 "parse.y"
{sqlite3BeginTransaction(pParse, yymsp[-1].minor.yy194);}
#line 2289 "parse.c"
        break;
      case 4: /* transtype ::= */
#line 140 "parse.y"
{yymsp[1].minor.yy194 = TK_DEFERRED;}
#line 2294 "parse.c"
        break;
      case 5: /* transtype ::= DEFERRED */
      case 6: /* transtype ::= IMMEDIATE */ yytestcase(yyruleno==6);
      case 7: /* transtype ::= EXCLUSIVE */ yytestcase(yyruleno==7);
#line 141 "parse.y"
{yymsp[0].minor.yy194 = yymsp[0].major; /*A-overwrites-X*/}
#line 2301 "parse.c"
        break;
      case 8: /* cmd ::= COMMIT trans_opt */
      case 9: /* cmd ::= END trans_opt */ yytestcase(yyruleno==9);
#line 144 "parse.y"
{sqlite3CommitTransaction(pParse);}
#line 2307 "parse.c"
        break;
      case 10: /* cmd ::= ROLLBACK trans_opt */
#line 146 "parse.y"
{sqlite3RollbackTransaction(pParse);}
#line 2312 "parse.c"
        break;
      case 11: /* cmd ::= SAVEPOINT nm */
#line 150 "parse.y"
{
  sqlite3Savepoint(pParse, SAVEPOINT_BEGIN, &yymsp[0].minor.yy0);
}
#line 2319 "parse.c"
        break;
      case 12: /* cmd ::= RELEASE savepoint_opt nm */
#line 153 "parse.y"
{
  sqlite3Savepoint(pParse, SAVEPOINT_RELEASE, &yymsp[0].minor.yy0);
}
#line 2326 "parse.c"
        break;
      case 13: /* cmd ::= ROLLBACK trans_opt TO savepoint_opt nm */
#line 156 "parse.y"
{
  sqlite3Savepoint(pParse, SAVEPOINT_ROLLBACK, &yymsp[0].minor.yy0);
}
#line 2333 "parse.c"
        break;
      case 14: /* create_table ::= createkw temp TABLE ifnotexists nm dbnm */
#line 163 "parse.y"
{
   sqlite3StartTable(pParse,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0,yymsp[-4].minor.yy194,0,0,yymsp[-2].minor.yy194);
}
#line 2340 "parse.c"
        break;
      case 15: /* createkw ::= CREATE */
#line 166 "parse.y"
{disableLookaside(pParse);}
#line 2345 "parse.c"
        break;
      case 16: /* ifnotexists ::= */
      case 19: /* temp ::= */ yytestcase(yyruleno==19);
      case 22: /* table_options ::= */ yytestcase(yyruleno==22);
      case 42: /* autoinc ::= */ yytestcase(yyruleno==42);
      case 57: /* init_deferred_pred_opt ::= */ yytestcase(yyruleno==57);
      case 67: /* defer_subclause_opt ::= */ yytestcase(yyruleno==67);
      case 76: /* ifexists ::= */ yytestcase(yyruleno==76);
      case 90: /* distinct ::= */ yytestcase(yyruleno==90);
      case 211: /* collate ::= */ yytestcase(yyruleno==211);
#line 169 "parse.y"
{yymsp[1].minor.yy194 = 0;}
#line 2358 "parse.c"
        break;
      case 17: /* ifnotexists ::= IF NOT EXISTS */
#line 170 "parse.y"
{yymsp[-2].minor.yy194 = 1;}
#line 2363 "parse.c"
        break;
      case 18: /* temp ::= TEMP */
      case 43: /* autoinc ::= AUTOINCR */ yytestcase(yyruleno==43);
#line 173 "parse.y"
{yymsp[0].minor.yy194 = 1;}
#line 2369 "parse.c"
        break;
      case 20: /* create_table_args ::= LP columnlist conslist_opt RP table_options */
#line 176 "parse.y"
{
  sqlite3EndTable(pParse,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0,yymsp[0].minor.yy194,0);
}
#line 2376 "parse.c"
        break;
      case 21: /* create_table_args ::= AS select */
#line 179 "parse.y"
{
  sqlite3EndTable(pParse,0,0,0,yymsp[0].minor.yy243);
  sqlite3SelectDelete(pParse->db, yymsp[0].minor.yy243);
}
#line 2384 "parse.c"
        break;
      case 23: /* table_options ::= WITHOUT nm */
#line 185 "parse.y"
{
  if( yymsp[0].minor.yy0.n==5 && sqlite3_strnicmp(yymsp[0].minor.yy0.z,"rowid",5)==0 ){
    yymsp[-1].minor.yy194 = TF_WithoutRowid | TF_NoVisibleRowid;
  }else{
    yymsp[-1].minor.yy194 = 0;
    sqlite3ErrorMsg(pParse, "unknown table option: %.*s", yymsp[0].minor.yy0.n, yymsp[0].minor.yy0.z);
  }
}
#line 2396 "parse.c"
        break;
      case 24: /* columnname ::= nm typetoken */
#line 195 "parse.y"
{sqlite3AddColumn(pParse,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0);}
#line 2401 "parse.c"
        break;
      case 25: /* typetoken ::= */
      case 60: /* conslist_opt ::= */ yytestcase(yyruleno==60);
      case 96: /* as ::= */ yytestcase(yyruleno==96);
#line 260 "parse.y"
{yymsp[1].minor.yy0.n = 0; yymsp[1].minor.yy0.z = 0;}
#line 2408 "parse.c"
        break;
      case 26: /* typetoken ::= typename LP signed RP */
#line 262 "parse.y"
{
  yymsp[-3].minor.yy0.n = (int)(&yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-3].minor.yy0.z);
}
#line 2415 "parse.c"
        break;
      case 27: /* typetoken ::= typename LP signed COMMA signed RP */
#line 265 "parse.y"
{
  yymsp[-5].minor.yy0.n = (int)(&yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-5].minor.yy0.z);
}
#line 2422 "parse.c"
        break;
      case 28: /* typename ::= typename ID|STRING */
#line 270 "parse.y"
{yymsp[-1].minor.yy0.n=yymsp[0].minor.yy0.n+(int)(yymsp[0].minor.yy0.z-yymsp[-1].minor.yy0.z);}
#line 2427 "parse.c"
        break;
      case 29: /* ccons ::= CONSTRAINT nm */
      case 62: /* tcons ::= CONSTRAINT nm */ yytestcase(yyruleno==62);
#line 279 "parse.y"
{pParse->constraintName = yymsp[0].minor.yy0;}
#line 2433 "parse.c"
        break;
      case 30: /* ccons ::= DEFAULT term */
      case 32: /* ccons ::= DEFAULT PLUS term */ yytestcase(yyruleno==32);
#line 280 "parse.y"
{sqlite3AddDefaultValue(pParse,&yymsp[0].minor.yy190);}
#line 2439 "parse.c"
        break;
      case 31: /* ccons ::= DEFAULT LP expr RP */
#line 281 "parse.y"
{sqlite3AddDefaultValue(pParse,&yymsp[-1].minor.yy190);}
#line 2444 "parse.c"
        break;
      case 33: /* ccons ::= DEFAULT MINUS term */
#line 283 "parse.y"
{
  ExprSpan v;
  v.pExpr = sqlite3PExpr(pParse, TK_UMINUS, yymsp[0].minor.yy190.pExpr, 0, 0);
  v.zStart = yymsp[-1].minor.yy0.z;
  v.zEnd = yymsp[0].minor.yy190.zEnd;
  sqlite3AddDefaultValue(pParse,&v);
}
#line 2455 "parse.c"
        break;
      case 34: /* ccons ::= DEFAULT ID|INDEXED */
#line 290 "parse.y"
{
  ExprSpan v;
  spanExpr(&v, pParse, TK_STRING, yymsp[0].minor.yy0);
  sqlite3AddDefaultValue(pParse,&v);
}
#line 2464 "parse.c"
        break;
      case 35: /* ccons ::= NOT NULL onconf */
#line 300 "parse.y"
{sqlite3AddNotNull(pParse, yymsp[0].minor.yy194);}
#line 2469 "parse.c"
        break;
      case 36: /* ccons ::= PRIMARY KEY sortorder onconf autoinc */
#line 302 "parse.y"
{sqlite3AddPrimaryKey(pParse,0,yymsp[-1].minor.yy194,yymsp[0].minor.yy194,yymsp[-2].minor.yy194);}
#line 2474 "parse.c"
        break;
      case 37: /* ccons ::= UNIQUE onconf */
#line 303 "parse.y"
{sqlite3CreateIndex(pParse,0,0,0,0,yymsp[0].minor.yy194,0,0,0,0,
                                   SQLITE_IDXTYPE_UNIQUE);}
#line 2480 "parse.c"
        break;
      case 38: /* ccons ::= CHECK LP expr RP */
#line 305 "parse.y"
{sqlite3AddCheckConstraint(pParse,yymsp[-1].minor.yy190.pExpr);}
#line 2485 "parse.c"
        break;
      case 39: /* ccons ::= REFERENCES nm eidlist_opt refargs */
#line 307 "parse.y"
{sqlite3CreateForeignKey(pParse,0,&yymsp[-2].minor.yy0,yymsp[-1].minor.yy148,yymsp[0].minor.yy194);}
#line 2490 "parse.c"
        break;
      case 40: /* ccons ::= defer_subclause */
#line 308 "parse.y"
{sqlite3DeferForeignKey(pParse,yymsp[0].minor.yy194);}
#line 2495 "parse.c"
        break;
      case 41: /* ccons ::= COLLATE ID|STRING */
#line 309 "parse.y"
{sqlite3AddCollateType(pParse, &yymsp[0].minor.yy0);}
#line 2500 "parse.c"
        break;
      case 44: /* refargs ::= */
#line 322 "parse.y"
{ yymsp[1].minor.yy194 = OE_None*0x0101; /* EV: R-19803-45884 */}
#line 2505 "parse.c"
        break;
      case 45: /* refargs ::= refargs refarg */
#line 323 "parse.y"
{ yymsp[-1].minor.yy194 = (yymsp[-1].minor.yy194 & ~yymsp[0].minor.yy497.mask) | yymsp[0].minor.yy497.value; }
#line 2510 "parse.c"
        break;
      case 46: /* refarg ::= MATCH nm */
#line 325 "parse.y"
{ yymsp[-1].minor.yy497.value = 0;     yymsp[-1].minor.yy497.mask = 0x000000; }
#line 2515 "parse.c"
        break;
      case 47: /* refarg ::= ON INSERT refact */
#line 326 "parse.y"
{ yymsp[-2].minor.yy497.value = 0;     yymsp[-2].minor.yy497.mask = 0x000000; }
#line 2520 "parse.c"
        break;
      case 48: /* refarg ::= ON DELETE refact */
#line 327 "parse.y"
{ yymsp[-2].minor.yy497.value = yymsp[0].minor.yy194;     yymsp[-2].minor.yy497.mask = 0x0000ff; }
#line 2525 "parse.c"
        break;
      case 49: /* refarg ::= ON UPDATE refact */
#line 328 "parse.y"
{ yymsp[-2].minor.yy497.value = yymsp[0].minor.yy194<<8;  yymsp[-2].minor.yy497.mask = 0x00ff00; }
#line 2530 "parse.c"
        break;
      case 50: /* refact ::= SET NULL */
#line 330 "parse.y"
{ yymsp[-1].minor.yy194 = OE_SetNull;  /* EV: R-33326-45252 */}
#line 2535 "parse.c"
        break;
      case 51: /* refact ::= SET DEFAULT */
#line 331 "parse.y"
{ yymsp[-1].minor.yy194 = OE_SetDflt;  /* EV: R-33326-45252 */}
#line 2540 "parse.c"
        break;
      case 52: /* refact ::= CASCADE */
#line 332 "parse.y"
{ yymsp[0].minor.yy194 = OE_Cascade;  /* EV: R-33326-45252 */}
#line 2545 "parse.c"
        break;
      case 53: /* refact ::= RESTRICT */
#line 333 "parse.y"
{ yymsp[0].minor.yy194 = OE_Restrict; /* EV: R-33326-45252 */}
#line 2550 "parse.c"
        break;
      case 54: /* refact ::= NO ACTION */
#line 334 "parse.y"
{ yymsp[-1].minor.yy194 = OE_None;     /* EV: R-33326-45252 */}
#line 2555 "parse.c"
        break;
      case 55: /* defer_subclause ::= NOT DEFERRABLE init_deferred_pred_opt */
#line 336 "parse.y"
{yymsp[-2].minor.yy194 = 0;}
#line 2560 "parse.c"
        break;
      case 56: /* defer_subclause ::= DEFERRABLE init_deferred_pred_opt */
      case 71: /* orconf ::= OR resolvetype */ yytestcase(yyruleno==71);
      case 142: /* insert_cmd ::= INSERT orconf */ yytestcase(yyruleno==142);
#line 337 "parse.y"
{yymsp[-1].minor.yy194 = yymsp[0].minor.yy194;}
#line 2567 "parse.c"
        break;
      case 58: /* init_deferred_pred_opt ::= INITIALLY DEFERRED */
      case 75: /* ifexists ::= IF EXISTS */ yytestcase(yyruleno==75);
      case 183: /* between_op ::= NOT BETWEEN */ yytestcase(yyruleno==183);
      case 186: /* in_op ::= NOT IN */ yytestcase(yyruleno==186);
      case 212: /* collate ::= COLLATE ID|STRING */ yytestcase(yyruleno==212);
#line 340 "parse.y"
{yymsp[-1].minor.yy194 = 1;}
#line 2576 "parse.c"
        break;
      case 59: /* init_deferred_pred_opt ::= INITIALLY IMMEDIATE */
#line 341 "parse.y"
{yymsp[-1].minor.yy194 = 0;}
#line 2581 "parse.c"
        break;
      case 61: /* tconscomma ::= COMMA */
#line 347 "parse.y"
{pParse->constraintName.n = 0;}
#line 2586 "parse.c"
        break;
      case 63: /* tcons ::= PRIMARY KEY LP sortlist autoinc RP onconf */
#line 351 "parse.y"
{sqlite3AddPrimaryKey(pParse,yymsp[-3].minor.yy148,yymsp[0].minor.yy194,yymsp[-2].minor.yy194,0);}
#line 2591 "parse.c"
        break;
      case 64: /* tcons ::= UNIQUE LP sortlist RP onconf */
#line 353 "parse.y"
{sqlite3CreateIndex(pParse,0,0,0,yymsp[-2].minor.yy148,yymsp[0].minor.yy194,0,0,0,0,
                                       SQLITE_IDXTYPE_UNIQUE);}
#line 2597 "parse.c"
        break;
      case 65: /* tcons ::= CHECK LP expr RP onconf */
#line 356 "parse.y"
{sqlite3AddCheckConstraint(pParse,yymsp[-2].minor.yy190.pExpr);}
#line 2602 "parse.c"
        break;
      case 66: /* tcons ::= FOREIGN KEY LP eidlist RP REFERENCES nm eidlist_opt refargs defer_subclause_opt */
#line 358 "parse.y"
{
    sqlite3CreateForeignKey(pParse, yymsp[-6].minor.yy148, &yymsp[-3].minor.yy0, yymsp[-2].minor.yy148, yymsp[-1].minor.yy194);
    sqlite3DeferForeignKey(pParse, yymsp[0].minor.yy194);
}
#line 2610 "parse.c"
        break;
      case 68: /* onconf ::= */
      case 70: /* orconf ::= */ yytestcase(yyruleno==70);
#line 372 "parse.y"
{yymsp[1].minor.yy194 = OE_Default;}
#line 2616 "parse.c"
        break;
      case 69: /* onconf ::= ON CONFLICT resolvetype */
#line 373 "parse.y"
{yymsp[-2].minor.yy194 = yymsp[0].minor.yy194;}
#line 2621 "parse.c"
        break;
      case 72: /* resolvetype ::= IGNORE */
#line 377 "parse.y"
{yymsp[0].minor.yy194 = OE_Ignore;}
#line 2626 "parse.c"
        break;
      case 73: /* resolvetype ::= REPLACE */
      case 143: /* insert_cmd ::= REPLACE */ yytestcase(yyruleno==143);
#line 378 "parse.y"
{yymsp[0].minor.yy194 = OE_Replace;}
#line 2632 "parse.c"
        break;
      case 74: /* cmd ::= DROP TABLE ifexists fullname */
#line 382 "parse.y"
{
  sqlite3DropTable(pParse, yymsp[0].minor.yy185, 0, yymsp[-1].minor.yy194);
}
#line 2639 "parse.c"
        break;
      case 77: /* cmd ::= createkw temp VIEW ifnotexists nm dbnm eidlist_opt AS select */
#line 393 "parse.y"
{
  sqlite3CreateView(pParse, &yymsp[-8].minor.yy0, &yymsp[-4].minor.yy0, &yymsp[-3].minor.yy0, yymsp[-2].minor.yy148, yymsp[0].minor.yy243, yymsp[-7].minor.yy194, yymsp[-5].minor.yy194);
}
#line 2646 "parse.c"
        break;
      case 78: /* cmd ::= DROP VIEW ifexists fullname */
#line 396 "parse.y"
{
  sqlite3DropTable(pParse, yymsp[0].minor.yy185, 1, yymsp[-1].minor.yy194);
}
#line 2653 "parse.c"
        break;
      case 79: /* cmd ::= select */
#line 403 "parse.y"
{
  SelectDest dest = {SRT_Output, 0, 0, 0, 0, 0};
  sqlite3Select(pParse, yymsp[0].minor.yy243, &dest);
  sqlite3SelectDelete(pParse->db, yymsp[0].minor.yy243);
}
#line 2662 "parse.c"
        break;
      case 80: /* select ::= with selectnowith */
#line 440 "parse.y"
{
  Select *p = yymsp[0].minor.yy243;
  if( p ){
    p->pWith = yymsp[-1].minor.yy285;
    parserDoubleLinkSelect(pParse, p);
  }else{
    sqlite3WithDelete(pParse->db, yymsp[-1].minor.yy285);
  }
  yymsp[-1].minor.yy243 = p; /*A-overwrites-W*/
}
#line 2676 "parse.c"
        break;
      case 81: /* selectnowith ::= selectnowith multiselect_op oneselect */
#line 453 "parse.y"
{
  Select *pRhs = yymsp[0].minor.yy243;
  Select *pLhs = yymsp[-2].minor.yy243;
  if( pRhs && pRhs->pPrior ){
    SrcList *pFrom;
    Token x;
    x.n = 0;
    parserDoubleLinkSelect(pParse, pRhs);
    pFrom = sqlite3SrcListAppendFromTerm(pParse,0,0,0,&x,pRhs,0,0);
    pRhs = sqlite3SelectNew(pParse,0,pFrom,0,0,0,0,0,0,0);
  }
  if( pRhs ){
    pRhs->op = (u8)yymsp[-1].minor.yy194;
    pRhs->pPrior = pLhs;
    if( ALWAYS(pLhs) ) pLhs->selFlags &= ~SF_MultiValue;
    pRhs->selFlags &= ~SF_MultiValue;
    if( yymsp[-1].minor.yy194!=TK_ALL ) pParse->hasCompound = 1;
  }else{
    sqlite3SelectDelete(pParse->db, pLhs);
  }
  yymsp[-2].minor.yy243 = pRhs;
}
#line 2702 "parse.c"
        break;
      case 82: /* multiselect_op ::= UNION */
      case 84: /* multiselect_op ::= EXCEPT|INTERSECT */ yytestcase(yyruleno==84);
#line 476 "parse.y"
{yymsp[0].minor.yy194 = yymsp[0].major; /*A-overwrites-OP*/}
#line 2708 "parse.c"
        break;
      case 83: /* multiselect_op ::= UNION ALL */
#line 477 "parse.y"
{yymsp[-1].minor.yy194 = TK_ALL;}
#line 2713 "parse.c"
        break;
      case 85: /* oneselect ::= SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt */
#line 481 "parse.y"
{
#if SELECTTRACE_ENABLED
  Token s = yymsp[-8].minor.yy0; /*A-overwrites-S*/
#endif
  yymsp[-8].minor.yy243 = sqlite3SelectNew(pParse,yymsp[-6].minor.yy148,yymsp[-5].minor.yy185,yymsp[-4].minor.yy72,yymsp[-3].minor.yy148,yymsp[-2].minor.yy72,yymsp[-1].minor.yy148,yymsp[-7].minor.yy194,yymsp[0].minor.yy354.pLimit,yymsp[0].minor.yy354.pOffset);
#if SELECTTRACE_ENABLED
  /* Populate the Select.zSelName[] string that is used to help with
  ** query planner debugging, to differentiate between multiple Select
  ** objects in a complex query.
  **
  ** If the SELECT keyword is immediately followed by a C-style comment
  ** then extract the first few alphanumeric characters from within that
  ** comment to be the zSelName value.  Otherwise, the label is #N where
  ** is an integer that is incremented with each SELECT statement seen.
  */
  if( yymsp[-8].minor.yy243!=0 ){
    const char *z = s.z+6;
    int i;
    sqlite3_snprintf(sizeof(yymsp[-8].minor.yy243->zSelName), yymsp[-8].minor.yy243->zSelName, "#%d",
                     ++pParse->nSelect);
    while( z[0]==' ' ) z++;
    if( z[0]=='/' && z[1]=='*' ){
      z += 2;
      while( z[0]==' ' ) z++;
      for(i=0; sqlite3Isalnum(z[i]); i++){}
      sqlite3_snprintf(sizeof(yymsp[-8].minor.yy243->zSelName), yymsp[-8].minor.yy243->zSelName, "%.*s", i, z);
    }
  }
#endif /* SELECTRACE_ENABLED */
}
#line 2747 "parse.c"
        break;
      case 86: /* values ::= VALUES LP nexprlist RP */
#line 515 "parse.y"
{
  yymsp[-3].minor.yy243 = sqlite3SelectNew(pParse,yymsp[-1].minor.yy148,0,0,0,0,0,SF_Values,0,0);
}
#line 2754 "parse.c"
        break;
      case 87: /* values ::= values COMMA LP exprlist RP */
#line 518 "parse.y"
{
  Select *pRight, *pLeft = yymsp[-4].minor.yy243;
  pRight = sqlite3SelectNew(pParse,yymsp[-1].minor.yy148,0,0,0,0,0,SF_Values|SF_MultiValue,0,0);
  if( ALWAYS(pLeft) ) pLeft->selFlags &= ~SF_MultiValue;
  if( pRight ){
    pRight->op = TK_ALL;
    pRight->pPrior = pLeft;
    yymsp[-4].minor.yy243 = pRight;
  }else{
    yymsp[-4].minor.yy243 = pLeft;
  }
}
#line 2770 "parse.c"
        break;
      case 88: /* distinct ::= DISTINCT */
#line 535 "parse.y"
{yymsp[0].minor.yy194 = SF_Distinct;}
#line 2775 "parse.c"
        break;
      case 89: /* distinct ::= ALL */
#line 536 "parse.y"
{yymsp[0].minor.yy194 = SF_All;}
#line 2780 "parse.c"
        break;
      case 91: /* sclp ::= */
      case 119: /* orderby_opt ::= */ yytestcase(yyruleno==119);
      case 126: /* groupby_opt ::= */ yytestcase(yyruleno==126);
      case 199: /* exprlist ::= */ yytestcase(yyruleno==199);
      case 202: /* paren_exprlist ::= */ yytestcase(yyruleno==202);
      case 207: /* eidlist_opt ::= */ yytestcase(yyruleno==207);
#line 549 "parse.y"
{yymsp[1].minor.yy148 = 0;}
#line 2790 "parse.c"
        break;
      case 92: /* selcollist ::= sclp expr as */
#line 550 "parse.y"
{
   yymsp[-2].minor.yy148 = sqlite3ExprListAppend(pParse, yymsp[-2].minor.yy148, yymsp[-1].minor.yy190.pExpr);
   if( yymsp[0].minor.yy0.n>0 ) sqlite3ExprListSetName(pParse, yymsp[-2].minor.yy148, &yymsp[0].minor.yy0, 1);
   sqlite3ExprListSetSpan(pParse,yymsp[-2].minor.yy148,&yymsp[-1].minor.yy190);
}
#line 2799 "parse.c"
        break;
      case 93: /* selcollist ::= sclp STAR */
#line 555 "parse.y"
{
  Expr *p = sqlite3Expr(pParse->db, TK_ASTERISK, 0);
  yymsp[-1].minor.yy148 = sqlite3ExprListAppend(pParse, yymsp[-1].minor.yy148, p);
}
#line 2807 "parse.c"
        break;
      case 94: /* selcollist ::= sclp nm DOT STAR */
#line 559 "parse.y"
{
  Expr *pRight = sqlite3PExpr(pParse, TK_ASTERISK, 0, 0, &yymsp[0].minor.yy0);
  Expr *pLeft = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *pDot = sqlite3PExpr(pParse, TK_DOT, pLeft, pRight, 0);
  yymsp[-3].minor.yy148 = sqlite3ExprListAppend(pParse,yymsp[-3].minor.yy148, pDot);
}
#line 2817 "parse.c"
        break;
      case 95: /* as ::= AS nm */
      case 106: /* dbnm ::= DOT nm */ yytestcase(yyruleno==106);
      case 221: /* plus_num ::= PLUS INTEGER|FLOAT */ yytestcase(yyruleno==221);
      case 222: /* minus_num ::= MINUS INTEGER|FLOAT */ yytestcase(yyruleno==222);
#line 570 "parse.y"
{yymsp[-1].minor.yy0 = yymsp[0].minor.yy0;}
#line 2825 "parse.c"
        break;
      case 97: /* from ::= */
#line 584 "parse.y"
{yymsp[1].minor.yy185 = sqlite3DbMallocZero(pParse->db, sizeof(*yymsp[1].minor.yy185));}
#line 2830 "parse.c"
        break;
      case 98: /* from ::= FROM seltablist */
#line 585 "parse.y"
{
  yymsp[-1].minor.yy185 = yymsp[0].minor.yy185;
  sqlite3SrcListShiftJoinType(yymsp[-1].minor.yy185);
}
#line 2838 "parse.c"
        break;
      case 99: /* stl_prefix ::= seltablist joinop */
#line 593 "parse.y"
{
   if( ALWAYS(yymsp[-1].minor.yy185 && yymsp[-1].minor.yy185->nSrc>0) ) yymsp[-1].minor.yy185->a[yymsp[-1].minor.yy185->nSrc-1].fg.jointype = (u8)yymsp[0].minor.yy194;
}
#line 2845 "parse.c"
        break;
      case 100: /* stl_prefix ::= */
#line 596 "parse.y"
{yymsp[1].minor.yy185 = 0;}
#line 2850 "parse.c"
        break;
      case 101: /* seltablist ::= stl_prefix nm dbnm as indexed_opt on_opt using_opt */
#line 598 "parse.y"
{
  yymsp[-6].minor.yy185 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy185,&yymsp[-5].minor.yy0,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,0,yymsp[-1].minor.yy72,yymsp[0].minor.yy254);
  sqlite3SrcListIndexedBy(pParse, yymsp[-6].minor.yy185, &yymsp[-2].minor.yy0);
}
#line 2858 "parse.c"
        break;
      case 102: /* seltablist ::= stl_prefix nm dbnm LP exprlist RP as on_opt using_opt */
#line 603 "parse.y"
{
  yymsp[-8].minor.yy185 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-8].minor.yy185,&yymsp[-7].minor.yy0,&yymsp[-6].minor.yy0,&yymsp[-2].minor.yy0,0,yymsp[-1].minor.yy72,yymsp[0].minor.yy254);
  sqlite3SrcListFuncArgs(pParse, yymsp[-8].minor.yy185, yymsp[-4].minor.yy148);
}
#line 2866 "parse.c"
        break;
      case 103: /* seltablist ::= stl_prefix LP select RP as on_opt using_opt */
#line 609 "parse.y"
{
    yymsp[-6].minor.yy185 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy185,0,0,&yymsp[-2].minor.yy0,yymsp[-4].minor.yy243,yymsp[-1].minor.yy72,yymsp[0].minor.yy254);
  }
#line 2873 "parse.c"
        break;
      case 104: /* seltablist ::= stl_prefix LP seltablist RP as on_opt using_opt */
#line 613 "parse.y"
{
    if( yymsp[-6].minor.yy185==0 && yymsp[-2].minor.yy0.n==0 && yymsp[-1].minor.yy72==0 && yymsp[0].minor.yy254==0 ){
      yymsp[-6].minor.yy185 = yymsp[-4].minor.yy185;
    }else if( yymsp[-4].minor.yy185->nSrc==1 ){
      yymsp[-6].minor.yy185 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy185,0,0,&yymsp[-2].minor.yy0,0,yymsp[-1].minor.yy72,yymsp[0].minor.yy254);
      if( yymsp[-6].minor.yy185 ){
        struct SrcList_item *pNew = &yymsp[-6].minor.yy185->a[yymsp[-6].minor.yy185->nSrc-1];
        struct SrcList_item *pOld = yymsp[-4].minor.yy185->a;
        pNew->zName = pOld->zName;
        pNew->zDatabase = pOld->zDatabase;
        pNew->pSelect = pOld->pSelect;
        pOld->zName = pOld->zDatabase = 0;
        pOld->pSelect = 0;
      }
      sqlite3SrcListDelete(pParse->db, yymsp[-4].minor.yy185);
    }else{
      Select *pSubquery;
      sqlite3SrcListShiftJoinType(yymsp[-4].minor.yy185);
      pSubquery = sqlite3SelectNew(pParse,0,yymsp[-4].minor.yy185,0,0,0,0,SF_NestedFrom,0,0);
      yymsp[-6].minor.yy185 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy185,0,0,&yymsp[-2].minor.yy0,pSubquery,yymsp[-1].minor.yy72,yymsp[0].minor.yy254);
    }
  }
#line 2899 "parse.c"
        break;
      case 105: /* dbnm ::= */
      case 114: /* indexed_opt ::= */ yytestcase(yyruleno==114);
#line 638 "parse.y"
{yymsp[1].minor.yy0.z=0; yymsp[1].minor.yy0.n=0;}
#line 2905 "parse.c"
        break;
      case 107: /* fullname ::= nm dbnm */
#line 644 "parse.y"
{yymsp[-1].minor.yy185 = sqlite3SrcListAppend(pParse->db,0,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0); /*A-overwrites-X*/}
#line 2910 "parse.c"
        break;
      case 108: /* joinop ::= COMMA|JOIN */
#line 647 "parse.y"
{ yymsp[0].minor.yy194 = JT_INNER; }
#line 2915 "parse.c"
        break;
      case 109: /* joinop ::= JOIN_KW JOIN */
#line 649 "parse.y"
{yymsp[-1].minor.yy194 = sqlite3JoinType(pParse,&yymsp[-1].minor.yy0,0,0);  /*X-overwrites-A*/}
#line 2920 "parse.c"
        break;
      case 110: /* joinop ::= JOIN_KW nm JOIN */
#line 651 "parse.y"
{yymsp[-2].minor.yy194 = sqlite3JoinType(pParse,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0,0); /*X-overwrites-A*/}
#line 2925 "parse.c"
        break;
      case 111: /* joinop ::= JOIN_KW nm nm JOIN */
#line 653 "parse.y"
{yymsp[-3].minor.yy194 = sqlite3JoinType(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0);/*X-overwrites-A*/}
#line 2930 "parse.c"
        break;
      case 112: /* on_opt ::= ON expr */
      case 129: /* having_opt ::= HAVING expr */ yytestcase(yyruleno==129);
      case 136: /* where_opt ::= WHERE expr */ yytestcase(yyruleno==136);
      case 195: /* case_else ::= ELSE expr */ yytestcase(yyruleno==195);
#line 657 "parse.y"
{yymsp[-1].minor.yy72 = yymsp[0].minor.yy190.pExpr;}
#line 2938 "parse.c"
        break;
      case 113: /* on_opt ::= */
      case 128: /* having_opt ::= */ yytestcase(yyruleno==128);
      case 135: /* where_opt ::= */ yytestcase(yyruleno==135);
      case 196: /* case_else ::= */ yytestcase(yyruleno==196);
      case 198: /* case_operand ::= */ yytestcase(yyruleno==198);
#line 658 "parse.y"
{yymsp[1].minor.yy72 = 0;}
#line 2947 "parse.c"
        break;
      case 115: /* indexed_opt ::= INDEXED BY nm */
#line 672 "parse.y"
{yymsp[-2].minor.yy0 = yymsp[0].minor.yy0;}
#line 2952 "parse.c"
        break;
      case 116: /* indexed_opt ::= NOT INDEXED */
#line 673 "parse.y"
{yymsp[-1].minor.yy0.z=0; yymsp[-1].minor.yy0.n=1;}
#line 2957 "parse.c"
        break;
      case 117: /* using_opt ::= USING LP idlist RP */
#line 677 "parse.y"
{yymsp[-3].minor.yy254 = yymsp[-1].minor.yy254;}
#line 2962 "parse.c"
        break;
      case 118: /* using_opt ::= */
      case 144: /* idlist_opt ::= */ yytestcase(yyruleno==144);
#line 678 "parse.y"
{yymsp[1].minor.yy254 = 0;}
#line 2968 "parse.c"
        break;
      case 120: /* orderby_opt ::= ORDER BY sortlist */
      case 127: /* groupby_opt ::= GROUP BY nexprlist */ yytestcase(yyruleno==127);
#line 692 "parse.y"
{yymsp[-2].minor.yy148 = yymsp[0].minor.yy148;}
#line 2974 "parse.c"
        break;
      case 121: /* sortlist ::= sortlist COMMA expr sortorder */
#line 693 "parse.y"
{
  yymsp[-3].minor.yy148 = sqlite3ExprListAppend(pParse,yymsp[-3].minor.yy148,yymsp[-1].minor.yy190.pExpr);
  sqlite3ExprListSetSortOrder(yymsp[-3].minor.yy148,yymsp[0].minor.yy194);
}
#line 2982 "parse.c"
        break;
      case 122: /* sortlist ::= expr sortorder */
#line 697 "parse.y"
{
  yymsp[-1].minor.yy148 = sqlite3ExprListAppend(pParse,0,yymsp[-1].minor.yy190.pExpr); /*A-overwrites-Y*/
  sqlite3ExprListSetSortOrder(yymsp[-1].minor.yy148,yymsp[0].minor.yy194);
}
#line 2990 "parse.c"
        break;
      case 123: /* sortorder ::= ASC */
#line 704 "parse.y"
{yymsp[0].minor.yy194 = SQLITE_SO_ASC;}
#line 2995 "parse.c"
        break;
      case 124: /* sortorder ::= DESC */
#line 705 "parse.y"
{yymsp[0].minor.yy194 = SQLITE_SO_DESC;}
#line 3000 "parse.c"
        break;
      case 125: /* sortorder ::= */
#line 706 "parse.y"
{yymsp[1].minor.yy194 = SQLITE_SO_UNDEFINED;}
#line 3005 "parse.c"
        break;
      case 130: /* limit_opt ::= */
#line 731 "parse.y"
{yymsp[1].minor.yy354.pLimit = 0; yymsp[1].minor.yy354.pOffset = 0;}
#line 3010 "parse.c"
        break;
      case 131: /* limit_opt ::= LIMIT expr */
#line 732 "parse.y"
{yymsp[-1].minor.yy354.pLimit = yymsp[0].minor.yy190.pExpr; yymsp[-1].minor.yy354.pOffset = 0;}
#line 3015 "parse.c"
        break;
      case 132: /* limit_opt ::= LIMIT expr OFFSET expr */
#line 734 "parse.y"
{yymsp[-3].minor.yy354.pLimit = yymsp[-2].minor.yy190.pExpr; yymsp[-3].minor.yy354.pOffset = yymsp[0].minor.yy190.pExpr;}
#line 3020 "parse.c"
        break;
      case 133: /* limit_opt ::= LIMIT expr COMMA expr */
#line 736 "parse.y"
{yymsp[-3].minor.yy354.pOffset = yymsp[-2].minor.yy190.pExpr; yymsp[-3].minor.yy354.pLimit = yymsp[0].minor.yy190.pExpr;}
#line 3025 "parse.c"
        break;
      case 134: /* cmd ::= with DELETE FROM fullname indexed_opt where_opt */
#line 750 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-5].minor.yy285, 1);
  sqlite3SrcListIndexedBy(pParse, yymsp[-2].minor.yy185, &yymsp[-1].minor.yy0);
  sqlite3DeleteFrom(pParse,yymsp[-2].minor.yy185,yymsp[0].minor.yy72);
}
#line 3034 "parse.c"
        break;
      case 137: /* cmd ::= with UPDATE orconf fullname indexed_opt SET setlist where_opt */
#line 777 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-7].minor.yy285, 1);
  sqlite3SrcListIndexedBy(pParse, yymsp[-4].minor.yy185, &yymsp[-3].minor.yy0);
  sqlite3ExprListCheckLength(pParse,yymsp[-1].minor.yy148,"set list"); 
  sqlite3Update(pParse,yymsp[-4].minor.yy185,yymsp[-1].minor.yy148,yymsp[0].minor.yy72,yymsp[-5].minor.yy194);
}
#line 3044 "parse.c"
        break;
      case 138: /* setlist ::= setlist COMMA nm EQ expr */
#line 788 "parse.y"
{
  yymsp[-4].minor.yy148 = sqlite3ExprListAppend(pParse, yymsp[-4].minor.yy148, yymsp[0].minor.yy190.pExpr);
  sqlite3ExprListSetName(pParse, yymsp[-4].minor.yy148, &yymsp[-2].minor.yy0, 1);
}
#line 3052 "parse.c"
        break;
      case 139: /* setlist ::= nm EQ expr */
#line 792 "parse.y"
{
  yylhsminor.yy148 = sqlite3ExprListAppend(pParse, 0, yymsp[0].minor.yy190.pExpr);
  sqlite3ExprListSetName(pParse, yylhsminor.yy148, &yymsp[-2].minor.yy0, 1);
}
#line 3060 "parse.c"
  yymsp[-2].minor.yy148 = yylhsminor.yy148;
        break;
      case 140: /* cmd ::= with insert_cmd INTO fullname idlist_opt select */
#line 799 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-5].minor.yy285, 1);
  sqlite3Insert(pParse, yymsp[-2].minor.yy185, yymsp[0].minor.yy243, yymsp[-1].minor.yy254, yymsp[-4].minor.yy194);
}
#line 3069 "parse.c"
        break;
      case 141: /* cmd ::= with insert_cmd INTO fullname idlist_opt DEFAULT VALUES */
#line 804 "parse.y"
{
  sqlite3WithPush(pParse, yymsp[-6].minor.yy285, 1);
  sqlite3Insert(pParse, yymsp[-3].minor.yy185, 0, yymsp[-2].minor.yy254, yymsp[-5].minor.yy194);
}
#line 3077 "parse.c"
        break;
      case 145: /* idlist_opt ::= LP idlist RP */
#line 819 "parse.y"
{yymsp[-2].minor.yy254 = yymsp[-1].minor.yy254;}
#line 3082 "parse.c"
        break;
      case 146: /* idlist ::= idlist COMMA nm */
#line 821 "parse.y"
{yymsp[-2].minor.yy254 = sqlite3IdListAppend(pParse->db,yymsp[-2].minor.yy254,&yymsp[0].minor.yy0);}
#line 3087 "parse.c"
        break;
      case 147: /* idlist ::= nm */
#line 823 "parse.y"
{yymsp[0].minor.yy254 = sqlite3IdListAppend(pParse->db,0,&yymsp[0].minor.yy0); /*A-overwrites-Y*/}
#line 3092 "parse.c"
        break;
      case 148: /* expr ::= LP expr RP */
#line 856 "parse.y"
{spanSet(&yymsp[-2].minor.yy190,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0); /*A-overwrites-B*/  yymsp[-2].minor.yy190.pExpr = yymsp[-1].minor.yy190.pExpr;}
#line 3097 "parse.c"
        break;
      case 149: /* term ::= NULL */
      case 154: /* term ::= INTEGER|FLOAT|BLOB */ yytestcase(yyruleno==154);
      case 155: /* term ::= STRING */ yytestcase(yyruleno==155);
#line 857 "parse.y"
{spanExpr(&yymsp[0].minor.yy190,pParse,yymsp[0].major,yymsp[0].minor.yy0);/*A-overwrites-X*/}
#line 3104 "parse.c"
        break;
      case 150: /* expr ::= ID|INDEXED */
      case 151: /* expr ::= JOIN_KW */ yytestcase(yyruleno==151);
#line 858 "parse.y"
{spanExpr(&yymsp[0].minor.yy190,pParse,TK_ID,yymsp[0].minor.yy0); /*A-overwrites-X*/}
#line 3110 "parse.c"
        break;
      case 152: /* expr ::= nm DOT nm */
#line 860 "parse.y"
{
  Expr *temp1 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *temp2 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[0].minor.yy0);
  spanSet(&yymsp[-2].minor.yy190,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0); /*A-overwrites-X*/
  yymsp[-2].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_DOT, temp1, temp2, 0);
}
#line 3120 "parse.c"
        break;
      case 153: /* expr ::= nm DOT nm DOT nm */
#line 866 "parse.y"
{
  Expr *temp1 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-4].minor.yy0);
  Expr *temp2 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *temp3 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[0].minor.yy0);
  Expr *temp4 = sqlite3PExpr(pParse, TK_DOT, temp2, temp3, 0);
  spanSet(&yymsp[-4].minor.yy190,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0); /*A-overwrites-X*/
  yymsp[-4].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_DOT, temp1, temp4, 0);
}
#line 3132 "parse.c"
        break;
      case 156: /* expr ::= VARIABLE */
#line 876 "parse.y"
{
  if( !(yymsp[0].minor.yy0.z[0]=='#' && sqlite3Isdigit(yymsp[0].minor.yy0.z[1])) ){
    spanExpr(&yymsp[0].minor.yy190, pParse, TK_VARIABLE, yymsp[0].minor.yy0);
    sqlite3ExprAssignVarNumber(pParse, yymsp[0].minor.yy190.pExpr);
  }else{
    /* When doing a nested parse, one can include terms in an expression
    ** that look like this:   #1 #2 ...  These terms refer to registers
    ** in the virtual machine.  #N is the N-th register. */
    Token t = yymsp[0].minor.yy0; /*A-overwrites-X*/
    assert( t.n>=2 );
    spanSet(&yymsp[0].minor.yy190, &t, &t);
    if( pParse->nested==0 ){
      sqlite3ErrorMsg(pParse, "near \"%T\": syntax error", &t);
      yymsp[0].minor.yy190.pExpr = 0;
    }else{
      yymsp[0].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_REGISTER, 0, 0, &t);
      if( yymsp[0].minor.yy190.pExpr ) sqlite3GetInt32(&t.z[1], &yymsp[0].minor.yy190.pExpr->iTable);
    }
  }
}
#line 3156 "parse.c"
        break;
      case 157: /* expr ::= expr COLLATE ID|STRING */
#line 896 "parse.y"
{
  yymsp[-2].minor.yy190.pExpr = sqlite3ExprAddCollateToken(pParse, yymsp[-2].minor.yy190.pExpr, &yymsp[0].minor.yy0, 1);
  yymsp[-2].minor.yy190.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
#line 3164 "parse.c"
        break;
      case 158: /* expr ::= CAST LP expr AS typetoken RP */
#line 901 "parse.y"
{
  spanSet(&yymsp[-5].minor.yy190,&yymsp[-5].minor.yy0,&yymsp[0].minor.yy0); /*A-overwrites-X*/
  yymsp[-5].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_CAST, yymsp[-3].minor.yy190.pExpr, 0, &yymsp[-1].minor.yy0);
}
#line 3172 "parse.c"
        break;
      case 159: /* expr ::= ID|INDEXED LP distinct exprlist RP */
#line 906 "parse.y"
{
  if( yymsp[-1].minor.yy148 && yymsp[-1].minor.yy148->nExpr>pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG] ){
    sqlite3ErrorMsg(pParse, "too many arguments on function %T", &yymsp[-4].minor.yy0);
  }
  yylhsminor.yy190.pExpr = sqlite3ExprFunction(pParse, yymsp[-1].minor.yy148, &yymsp[-4].minor.yy0);
  spanSet(&yylhsminor.yy190,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0);
  if( yymsp[-2].minor.yy194==SF_Distinct && yylhsminor.yy190.pExpr ){
    yylhsminor.yy190.pExpr->flags |= EP_Distinct;
  }
}
#line 3186 "parse.c"
  yymsp[-4].minor.yy190 = yylhsminor.yy190;
        break;
      case 160: /* expr ::= ID|INDEXED LP STAR RP */
#line 916 "parse.y"
{
  yylhsminor.yy190.pExpr = sqlite3ExprFunction(pParse, 0, &yymsp[-3].minor.yy0);
  spanSet(&yylhsminor.yy190,&yymsp[-3].minor.yy0,&yymsp[0].minor.yy0);
}
#line 3195 "parse.c"
  yymsp[-3].minor.yy190 = yylhsminor.yy190;
        break;
      case 161: /* term ::= CTIME_KW */
#line 920 "parse.y"
{
  yylhsminor.yy190.pExpr = sqlite3ExprFunction(pParse, 0, &yymsp[0].minor.yy0);
  spanSet(&yylhsminor.yy190, &yymsp[0].minor.yy0, &yymsp[0].minor.yy0);
}
#line 3204 "parse.c"
  yymsp[0].minor.yy190 = yylhsminor.yy190;
        break;
      case 162: /* expr ::= expr AND expr */
      case 163: /* expr ::= expr OR expr */ yytestcase(yyruleno==163);
      case 164: /* expr ::= expr LT|GT|GE|LE expr */ yytestcase(yyruleno==164);
      case 165: /* expr ::= expr EQ|NE expr */ yytestcase(yyruleno==165);
      case 166: /* expr ::= expr BITAND|BITOR|LSHIFT|RSHIFT expr */ yytestcase(yyruleno==166);
      case 167: /* expr ::= expr PLUS|MINUS expr */ yytestcase(yyruleno==167);
      case 168: /* expr ::= expr STAR|SLASH|REM expr */ yytestcase(yyruleno==168);
      case 169: /* expr ::= expr CONCAT expr */ yytestcase(yyruleno==169);
#line 949 "parse.y"
{spanBinaryExpr(pParse,yymsp[-1].major,&yymsp[-2].minor.yy190,&yymsp[0].minor.yy190);}
#line 3217 "parse.c"
        break;
      case 170: /* likeop ::= LIKE_KW|MATCH */
#line 962 "parse.y"
{yymsp[0].minor.yy392.eOperator = yymsp[0].minor.yy0; yymsp[0].minor.yy392.bNot = 0;/*A-overwrites-X*/}
#line 3222 "parse.c"
        break;
      case 171: /* likeop ::= NOT LIKE_KW|MATCH */
#line 963 "parse.y"
{yymsp[-1].minor.yy392.eOperator = yymsp[0].minor.yy0; yymsp[-1].minor.yy392.bNot = 1;}
#line 3227 "parse.c"
        break;
      case 172: /* expr ::= expr likeop expr */
#line 964 "parse.y"
{
  ExprList *pList;
  pList = sqlite3ExprListAppend(pParse,0, yymsp[0].minor.yy190.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[-2].minor.yy190.pExpr);
  yymsp[-2].minor.yy190.pExpr = sqlite3ExprFunction(pParse, pList, &yymsp[-1].minor.yy392.eOperator);
  exprNot(pParse, yymsp[-1].minor.yy392.bNot, &yymsp[-2].minor.yy190);
  yymsp[-2].minor.yy190.zEnd = yymsp[0].minor.yy190.zEnd;
  if( yymsp[-2].minor.yy190.pExpr ) yymsp[-2].minor.yy190.pExpr->flags |= EP_InfixFunc;
}
#line 3240 "parse.c"
        break;
      case 173: /* expr ::= expr likeop expr ESCAPE expr */
#line 973 "parse.y"
{
  ExprList *pList;
  pList = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy190.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[-4].minor.yy190.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[0].minor.yy190.pExpr);
  yymsp[-4].minor.yy190.pExpr = sqlite3ExprFunction(pParse, pList, &yymsp[-3].minor.yy392.eOperator);
  exprNot(pParse, yymsp[-3].minor.yy392.bNot, &yymsp[-4].minor.yy190);
  yymsp[-4].minor.yy190.zEnd = yymsp[0].minor.yy190.zEnd;
  if( yymsp[-4].minor.yy190.pExpr ) yymsp[-4].minor.yy190.pExpr->flags |= EP_InfixFunc;
}
#line 3254 "parse.c"
        break;
      case 174: /* expr ::= expr ISNULL|NOTNULL */
#line 998 "parse.y"
{spanUnaryPostfix(pParse,yymsp[0].major,&yymsp[-1].minor.yy190,&yymsp[0].minor.yy0);}
#line 3259 "parse.c"
        break;
      case 175: /* expr ::= expr NOT NULL */
#line 999 "parse.y"
{spanUnaryPostfix(pParse,TK_NOTNULL,&yymsp[-2].minor.yy190,&yymsp[0].minor.yy0);}
#line 3264 "parse.c"
        break;
      case 176: /* expr ::= expr IS expr */
#line 1020 "parse.y"
{
  spanBinaryExpr(pParse,TK_IS,&yymsp[-2].minor.yy190,&yymsp[0].minor.yy190);
  binaryToUnaryIfNull(pParse, yymsp[0].minor.yy190.pExpr, yymsp[-2].minor.yy190.pExpr, TK_ISNULL);
}
#line 3272 "parse.c"
        break;
      case 177: /* expr ::= expr IS NOT expr */
#line 1024 "parse.y"
{
  spanBinaryExpr(pParse,TK_ISNOT,&yymsp[-3].minor.yy190,&yymsp[0].minor.yy190);
  binaryToUnaryIfNull(pParse, yymsp[0].minor.yy190.pExpr, yymsp[-3].minor.yy190.pExpr, TK_NOTNULL);
}
#line 3280 "parse.c"
        break;
      case 178: /* expr ::= NOT expr */
      case 179: /* expr ::= BITNOT expr */ yytestcase(yyruleno==179);
#line 1048 "parse.y"
{spanUnaryPrefix(&yymsp[-1].minor.yy190,pParse,yymsp[-1].major,&yymsp[0].minor.yy190,&yymsp[-1].minor.yy0);/*A-overwrites-B*/}
#line 3286 "parse.c"
        break;
      case 180: /* expr ::= MINUS expr */
#line 1052 "parse.y"
{spanUnaryPrefix(&yymsp[-1].minor.yy190,pParse,TK_UMINUS,&yymsp[0].minor.yy190,&yymsp[-1].minor.yy0);/*A-overwrites-B*/}
#line 3291 "parse.c"
        break;
      case 181: /* expr ::= PLUS expr */
#line 1054 "parse.y"
{spanUnaryPrefix(&yymsp[-1].minor.yy190,pParse,TK_UPLUS,&yymsp[0].minor.yy190,&yymsp[-1].minor.yy0);/*A-overwrites-B*/}
#line 3296 "parse.c"
        break;
      case 182: /* between_op ::= BETWEEN */
      case 185: /* in_op ::= IN */ yytestcase(yyruleno==185);
#line 1057 "parse.y"
{yymsp[0].minor.yy194 = 0;}
#line 3302 "parse.c"
        break;
      case 184: /* expr ::= expr between_op expr AND expr */
#line 1059 "parse.y"
{
  ExprList *pList = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy190.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[0].minor.yy190.pExpr);
  yymsp[-4].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_BETWEEN, yymsp[-4].minor.yy190.pExpr, 0, 0);
  if( yymsp[-4].minor.yy190.pExpr ){
    yymsp[-4].minor.yy190.pExpr->x.pList = pList;
  }else{
    sqlite3ExprListDelete(pParse->db, pList);
  } 
  exprNot(pParse, yymsp[-3].minor.yy194, &yymsp[-4].minor.yy190);
  yymsp[-4].minor.yy190.zEnd = yymsp[0].minor.yy190.zEnd;
}
#line 3318 "parse.c"
        break;
      case 187: /* expr ::= expr in_op LP exprlist RP */
#line 1075 "parse.y"
{
    if( yymsp[-1].minor.yy148==0 ){
      /* Expressions of the form
      **
      **      expr1 IN ()
      **      expr1 NOT IN ()
      **
      ** simplify to constants 0 (false) and 1 (true), respectively,
      ** regardless of the value of expr1.
      */
      sqlite3ExprDelete(pParse->db, yymsp[-4].minor.yy190.pExpr);
      yymsp[-4].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_INTEGER, 0, 0, &sqlite3IntTokens[yymsp[-3].minor.yy194]);
    }else if( yymsp[-1].minor.yy148->nExpr==1 ){
      /* Expressions of the form:
      **
      **      expr1 IN (?1)
      **      expr1 NOT IN (?2)
      **
      ** with exactly one value on the RHS can be simplified to something
      ** like this:
      **
      **      expr1 == ?1
      **      expr1 <> ?2
      **
      ** But, the RHS of the == or <> is marked with the EP_Generic flag
      ** so that it may not contribute to the computation of comparison
      ** affinity or the collating sequence to use for comparison.  Otherwise,
      ** the semantics would be subtly different from IN or NOT IN.
      */
      Expr *pRHS = yymsp[-1].minor.yy148->a[0].pExpr;
      yymsp[-1].minor.yy148->a[0].pExpr = 0;
      sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy148);
      /* pRHS cannot be NULL because a malloc error would have been detected
      ** before now and control would have never reached this point */
      if( ALWAYS(pRHS) ){
        pRHS->flags &= ~EP_Collate;
        pRHS->flags |= EP_Generic;
      }
      yymsp[-4].minor.yy190.pExpr = sqlite3PExpr(pParse, yymsp[-3].minor.yy194 ? TK_NE : TK_EQ, yymsp[-4].minor.yy190.pExpr, pRHS, 0);
    }else{
      yymsp[-4].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-4].minor.yy190.pExpr, 0, 0);
      if( yymsp[-4].minor.yy190.pExpr ){
        yymsp[-4].minor.yy190.pExpr->x.pList = yymsp[-1].minor.yy148;
        sqlite3ExprSetHeightAndFlags(pParse, yymsp[-4].minor.yy190.pExpr);
      }else{
        sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy148);
      }
      exprNot(pParse, yymsp[-3].minor.yy194, &yymsp[-4].minor.yy190);
    }
    yymsp[-4].minor.yy190.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 3373 "parse.c"
        break;
      case 188: /* expr ::= LP select RP */
#line 1126 "parse.y"
{
    spanSet(&yymsp[-2].minor.yy190,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0); /*A-overwrites-B*/
    yymsp[-2].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_SELECT, 0, 0, 0);
    sqlite3PExprAddSelect(pParse, yymsp[-2].minor.yy190.pExpr, yymsp[-1].minor.yy243);
  }
#line 3382 "parse.c"
        break;
      case 189: /* expr ::= expr in_op LP select RP */
#line 1131 "parse.y"
{
    yymsp[-4].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-4].minor.yy190.pExpr, 0, 0);
    sqlite3PExprAddSelect(pParse, yymsp[-4].minor.yy190.pExpr, yymsp[-1].minor.yy243);
    exprNot(pParse, yymsp[-3].minor.yy194, &yymsp[-4].minor.yy190);
    yymsp[-4].minor.yy190.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
#line 3392 "parse.c"
        break;
      case 190: /* expr ::= expr in_op nm dbnm paren_exprlist */
#line 1137 "parse.y"
{
    SrcList *pSrc = sqlite3SrcListAppend(pParse->db, 0,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0);
    Select *pSelect = sqlite3SelectNew(pParse, 0,pSrc,0,0,0,0,0,0,0);
    if( yymsp[0].minor.yy148 )  sqlite3SrcListFuncArgs(pParse, pSelect ? pSrc : 0, yymsp[0].minor.yy148);
    yymsp[-4].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-4].minor.yy190.pExpr, 0, 0);
    sqlite3PExprAddSelect(pParse, yymsp[-4].minor.yy190.pExpr, pSelect);
    exprNot(pParse, yymsp[-3].minor.yy194, &yymsp[-4].minor.yy190);
    yymsp[-4].minor.yy190.zEnd = yymsp[-1].minor.yy0.z ? &yymsp[-1].minor.yy0.z[yymsp[-1].minor.yy0.n] : &yymsp[-2].minor.yy0.z[yymsp[-2].minor.yy0.n];
  }
#line 3405 "parse.c"
        break;
      case 191: /* expr ::= EXISTS LP select RP */
#line 1146 "parse.y"
{
    Expr *p;
    spanSet(&yymsp[-3].minor.yy190,&yymsp[-3].minor.yy0,&yymsp[0].minor.yy0); /*A-overwrites-B*/
    p = yymsp[-3].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_EXISTS, 0, 0, 0);
    sqlite3PExprAddSelect(pParse, p, yymsp[-1].minor.yy243);
  }
#line 3415 "parse.c"
        break;
      case 192: /* expr ::= CASE case_operand case_exprlist case_else END */
#line 1155 "parse.y"
{
  spanSet(&yymsp[-4].minor.yy190,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0);  /*A-overwrites-C*/
  yymsp[-4].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_CASE, yymsp[-3].minor.yy72, 0, 0);
  if( yymsp[-4].minor.yy190.pExpr ){
    yymsp[-4].minor.yy190.pExpr->x.pList = yymsp[-1].minor.yy72 ? sqlite3ExprListAppend(pParse,yymsp[-2].minor.yy148,yymsp[-1].minor.yy72) : yymsp[-2].minor.yy148;
    sqlite3ExprSetHeightAndFlags(pParse, yymsp[-4].minor.yy190.pExpr);
  }else{
    sqlite3ExprListDelete(pParse->db, yymsp[-2].minor.yy148);
    sqlite3ExprDelete(pParse->db, yymsp[-1].minor.yy72);
  }
}
#line 3430 "parse.c"
        break;
      case 193: /* case_exprlist ::= case_exprlist WHEN expr THEN expr */
#line 1168 "parse.y"
{
  yymsp[-4].minor.yy148 = sqlite3ExprListAppend(pParse,yymsp[-4].minor.yy148, yymsp[-2].minor.yy190.pExpr);
  yymsp[-4].minor.yy148 = sqlite3ExprListAppend(pParse,yymsp[-4].minor.yy148, yymsp[0].minor.yy190.pExpr);
}
#line 3438 "parse.c"
        break;
      case 194: /* case_exprlist ::= WHEN expr THEN expr */
#line 1172 "parse.y"
{
  yymsp[-3].minor.yy148 = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy190.pExpr);
  yymsp[-3].minor.yy148 = sqlite3ExprListAppend(pParse,yymsp[-3].minor.yy148, yymsp[0].minor.yy190.pExpr);
}
#line 3446 "parse.c"
        break;
      case 197: /* case_operand ::= expr */
#line 1182 "parse.y"
{yymsp[0].minor.yy72 = yymsp[0].minor.yy190.pExpr; /*A-overwrites-X*/}
#line 3451 "parse.c"
        break;
      case 200: /* nexprlist ::= nexprlist COMMA expr */
#line 1193 "parse.y"
{yymsp[-2].minor.yy148 = sqlite3ExprListAppend(pParse,yymsp[-2].minor.yy148,yymsp[0].minor.yy190.pExpr);}
#line 3456 "parse.c"
        break;
      case 201: /* nexprlist ::= expr */
#line 1195 "parse.y"
{yymsp[0].minor.yy148 = sqlite3ExprListAppend(pParse,0,yymsp[0].minor.yy190.pExpr); /*A-overwrites-Y*/}
#line 3461 "parse.c"
        break;
      case 203: /* paren_exprlist ::= LP exprlist RP */
      case 208: /* eidlist_opt ::= LP eidlist RP */ yytestcase(yyruleno==208);
#line 1203 "parse.y"
{yymsp[-2].minor.yy148 = yymsp[-1].minor.yy148;}
#line 3467 "parse.c"
        break;
      case 204: /* cmd ::= createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP sortlist RP where_opt */
#line 1210 "parse.y"
{
  sqlite3CreateIndex(pParse, &yymsp[-7].minor.yy0, &yymsp[-6].minor.yy0, 
                     sqlite3SrcListAppend(pParse->db,0,&yymsp[-4].minor.yy0,0), yymsp[-2].minor.yy148, yymsp[-10].minor.yy194,
                      &yymsp[-11].minor.yy0, yymsp[0].minor.yy72, SQLITE_SO_ASC, yymsp[-8].minor.yy194, SQLITE_IDXTYPE_APPDEF);
}
#line 3476 "parse.c"
        break;
      case 205: /* uniqueflag ::= UNIQUE */
      case 246: /* raisetype ::= ABORT */ yytestcase(yyruleno==246);
#line 1217 "parse.y"
{yymsp[0].minor.yy194 = OE_Abort;}
#line 3482 "parse.c"
        break;
      case 206: /* uniqueflag ::= */
#line 1218 "parse.y"
{yymsp[1].minor.yy194 = OE_None;}
#line 3487 "parse.c"
        break;
      case 209: /* eidlist ::= eidlist COMMA nm collate sortorder */
#line 1268 "parse.y"
{
  yymsp[-4].minor.yy148 = parserAddExprIdListTerm(pParse, yymsp[-4].minor.yy148, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy194, yymsp[0].minor.yy194);
}
#line 3494 "parse.c"
        break;
      case 210: /* eidlist ::= nm collate sortorder */
#line 1271 "parse.y"
{
  yymsp[-2].minor.yy148 = parserAddExprIdListTerm(pParse, 0, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy194, yymsp[0].minor.yy194); /*A-overwrites-Y*/
}
#line 3501 "parse.c"
        break;
      case 213: /* cmd ::= DROP INDEX ifexists fullname */
#line 1282 "parse.y"
{sqlite3DropIndex(pParse, yymsp[0].minor.yy185, yymsp[-1].minor.yy194);}
#line 3506 "parse.c"
        break;
      case 214: /* cmd ::= VACUUM */
      case 215: /* cmd ::= VACUUM nm */ yytestcase(yyruleno==215);
#line 1288 "parse.y"
{sqlite3Vacuum(pParse);}
#line 3512 "parse.c"
        break;
      case 216: /* cmd ::= PRAGMA nm dbnm */
#line 1296 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0,0,0);}
#line 3517 "parse.c"
        break;
      case 217: /* cmd ::= PRAGMA nm dbnm EQ nmnum */
#line 1297 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0,0);}
#line 3522 "parse.c"
        break;
      case 218: /* cmd ::= PRAGMA nm dbnm LP nmnum RP */
#line 1298 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,&yymsp[-1].minor.yy0,0);}
#line 3527 "parse.c"
        break;
      case 219: /* cmd ::= PRAGMA nm dbnm EQ minus_num */
#line 1300 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0,1);}
#line 3532 "parse.c"
        break;
      case 220: /* cmd ::= PRAGMA nm dbnm LP minus_num RP */
#line 1302 "parse.y"
{sqlite3Pragma(pParse,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,&yymsp[-1].minor.yy0,1);}
#line 3537 "parse.c"
        break;
      case 223: /* cmd ::= createkw trigger_decl BEGIN trigger_cmd_list END */
#line 1318 "parse.y"
{
  Token all;
  all.z = yymsp[-3].minor.yy0.z;
  all.n = (int)(yymsp[0].minor.yy0.z - yymsp[-3].minor.yy0.z) + yymsp[0].minor.yy0.n;
  sqlite3FinishTrigger(pParse, yymsp[-1].minor.yy145, &all);
}
#line 3547 "parse.c"
        break;
      case 224: /* trigger_decl ::= temp TRIGGER ifnotexists nm dbnm trigger_time trigger_event ON fullname foreach_clause when_clause */
#line 1327 "parse.y"
{
  sqlite3BeginTrigger(pParse, &yymsp[-7].minor.yy0, &yymsp[-6].minor.yy0, yymsp[-5].minor.yy194, yymsp[-4].minor.yy332.a, yymsp[-4].minor.yy332.b, yymsp[-2].minor.yy185, yymsp[0].minor.yy72, yymsp[-10].minor.yy194, yymsp[-8].minor.yy194);
  yymsp[-10].minor.yy0 = (yymsp[-6].minor.yy0.n==0?yymsp[-7].minor.yy0:yymsp[-6].minor.yy0); /*A-overwrites-T*/
}
#line 3555 "parse.c"
        break;
      case 225: /* trigger_time ::= BEFORE */
#line 1333 "parse.y"
{ yymsp[0].minor.yy194 = TK_BEFORE; }
#line 3560 "parse.c"
        break;
      case 226: /* trigger_time ::= AFTER */
#line 1334 "parse.y"
{ yymsp[0].minor.yy194 = TK_AFTER;  }
#line 3565 "parse.c"
        break;
      case 227: /* trigger_time ::= INSTEAD OF */
#line 1335 "parse.y"
{ yymsp[-1].minor.yy194 = TK_INSTEAD;}
#line 3570 "parse.c"
        break;
      case 228: /* trigger_time ::= */
#line 1336 "parse.y"
{ yymsp[1].minor.yy194 = TK_BEFORE; }
#line 3575 "parse.c"
        break;
      case 229: /* trigger_event ::= DELETE|INSERT */
      case 230: /* trigger_event ::= UPDATE */ yytestcase(yyruleno==230);
#line 1340 "parse.y"
{yymsp[0].minor.yy332.a = yymsp[0].major; /*A-overwrites-X*/ yymsp[0].minor.yy332.b = 0;}
#line 3581 "parse.c"
        break;
      case 231: /* trigger_event ::= UPDATE OF idlist */
#line 1342 "parse.y"
{yymsp[-2].minor.yy332.a = TK_UPDATE; yymsp[-2].minor.yy332.b = yymsp[0].minor.yy254;}
#line 3586 "parse.c"
        break;
      case 232: /* when_clause ::= */
      case 251: /* key_opt ::= */ yytestcase(yyruleno==251);
#line 1349 "parse.y"
{ yymsp[1].minor.yy72 = 0; }
#line 3592 "parse.c"
        break;
      case 233: /* when_clause ::= WHEN expr */
      case 252: /* key_opt ::= KEY expr */ yytestcase(yyruleno==252);
#line 1350 "parse.y"
{ yymsp[-1].minor.yy72 = yymsp[0].minor.yy190.pExpr; }
#line 3598 "parse.c"
        break;
      case 234: /* trigger_cmd_list ::= trigger_cmd_list trigger_cmd SEMI */
#line 1354 "parse.y"
{
  assert( yymsp[-2].minor.yy145!=0 );
  yymsp[-2].minor.yy145->pLast->pNext = yymsp[-1].minor.yy145;
  yymsp[-2].minor.yy145->pLast = yymsp[-1].minor.yy145;
}
#line 3607 "parse.c"
        break;
      case 235: /* trigger_cmd_list ::= trigger_cmd SEMI */
#line 1359 "parse.y"
{ 
  assert( yymsp[-1].minor.yy145!=0 );
  yymsp[-1].minor.yy145->pLast = yymsp[-1].minor.yy145;
}
#line 3615 "parse.c"
        break;
      case 236: /* trnm ::= nm DOT nm */
#line 1370 "parse.y"
{
  yymsp[-2].minor.yy0 = yymsp[0].minor.yy0;
  sqlite3ErrorMsg(pParse, 
        "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
        "statements within triggers");
}
#line 3625 "parse.c"
        break;
      case 237: /* tridxby ::= INDEXED BY nm */
#line 1382 "parse.y"
{
  sqlite3ErrorMsg(pParse,
        "the INDEXED BY clause is not allowed on UPDATE or DELETE statements "
        "within triggers");
}
#line 3634 "parse.c"
        break;
      case 238: /* tridxby ::= NOT INDEXED */
#line 1387 "parse.y"
{
  sqlite3ErrorMsg(pParse,
        "the NOT INDEXED clause is not allowed on UPDATE or DELETE statements "
        "within triggers");
}
#line 3643 "parse.c"
        break;
      case 239: /* trigger_cmd ::= UPDATE orconf trnm tridxby SET setlist where_opt */
#line 1400 "parse.y"
{yymsp[-6].minor.yy145 = sqlite3TriggerUpdateStep(pParse->db, &yymsp[-4].minor.yy0, yymsp[-1].minor.yy148, yymsp[0].minor.yy72, yymsp[-5].minor.yy194);}
#line 3648 "parse.c"
        break;
      case 240: /* trigger_cmd ::= insert_cmd INTO trnm idlist_opt select */
#line 1404 "parse.y"
{yymsp[-4].minor.yy145 = sqlite3TriggerInsertStep(pParse->db, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy254, yymsp[0].minor.yy243, yymsp[-4].minor.yy194);/*A-overwrites-R*/}
#line 3653 "parse.c"
        break;
      case 241: /* trigger_cmd ::= DELETE FROM trnm tridxby where_opt */
#line 1408 "parse.y"
{yymsp[-4].minor.yy145 = sqlite3TriggerDeleteStep(pParse->db, &yymsp[-2].minor.yy0, yymsp[0].minor.yy72);}
#line 3658 "parse.c"
        break;
      case 242: /* trigger_cmd ::= select */
#line 1412 "parse.y"
{yymsp[0].minor.yy145 = sqlite3TriggerSelectStep(pParse->db, yymsp[0].minor.yy243); /*A-overwrites-X*/}
#line 3663 "parse.c"
        break;
      case 243: /* expr ::= RAISE LP IGNORE RP */
#line 1415 "parse.y"
{
  spanSet(&yymsp[-3].minor.yy190,&yymsp[-3].minor.yy0,&yymsp[0].minor.yy0);  /*A-overwrites-X*/
  yymsp[-3].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_RAISE, 0, 0, 0); 
  if( yymsp[-3].minor.yy190.pExpr ){
    yymsp[-3].minor.yy190.pExpr->affinity = OE_Ignore;
  }
}
#line 3674 "parse.c"
        break;
      case 244: /* expr ::= RAISE LP raisetype COMMA nm RP */
#line 1422 "parse.y"
{
  spanSet(&yymsp[-5].minor.yy190,&yymsp[-5].minor.yy0,&yymsp[0].minor.yy0);  /*A-overwrites-X*/
  yymsp[-5].minor.yy190.pExpr = sqlite3PExpr(pParse, TK_RAISE, 0, 0, &yymsp[-1].minor.yy0); 
  if( yymsp[-5].minor.yy190.pExpr ) {
    yymsp[-5].minor.yy190.pExpr->affinity = (char)yymsp[-3].minor.yy194;
  }
}
#line 3685 "parse.c"
        break;
      case 245: /* raisetype ::= ROLLBACK */
#line 1432 "parse.y"
{yymsp[0].minor.yy194 = OE_Rollback;}
#line 3690 "parse.c"
        break;
      case 247: /* raisetype ::= FAIL */
#line 1434 "parse.y"
{yymsp[0].minor.yy194 = OE_Fail;}
#line 3695 "parse.c"
        break;
      case 248: /* cmd ::= DROP TRIGGER ifexists fullname */
#line 1439 "parse.y"
{
  sqlite3DropTrigger(pParse,yymsp[0].minor.yy185,yymsp[-1].minor.yy194);
}
#line 3702 "parse.c"
        break;
      case 249: /* cmd ::= ATTACH database_kw_opt expr AS expr key_opt */
#line 1446 "parse.y"
{
  sqlite3Attach(pParse, yymsp[-3].minor.yy190.pExpr, yymsp[-1].minor.yy190.pExpr, yymsp[0].minor.yy72);
}
#line 3709 "parse.c"
        break;
      case 250: /* cmd ::= DETACH database_kw_opt expr */
#line 1449 "parse.y"
{
  sqlite3Detach(pParse, yymsp[0].minor.yy190.pExpr);
}
#line 3716 "parse.c"
        break;
      case 253: /* cmd ::= REINDEX */
#line 1464 "parse.y"
{sqlite3Reindex(pParse, 0, 0);}
#line 3721 "parse.c"
        break;
      case 254: /* cmd ::= REINDEX nm dbnm */
#line 1465 "parse.y"
{sqlite3Reindex(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);}
#line 3726 "parse.c"
        break;
      case 255: /* cmd ::= ANALYZE */
#line 1470 "parse.y"
{sqlite3Analyze(pParse, 0, 0);}
#line 3731 "parse.c"
        break;
      case 256: /* cmd ::= ANALYZE nm dbnm */
#line 1471 "parse.y"
{sqlite3Analyze(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);}
#line 3736 "parse.c"
        break;
      case 257: /* cmd ::= ALTER TABLE fullname RENAME TO nm */
#line 1476 "parse.y"
{
  sqlite3AlterRenameTable(pParse,yymsp[-3].minor.yy185,&yymsp[0].minor.yy0);
}
#line 3743 "parse.c"
        break;
      case 258: /* cmd ::= ALTER TABLE add_column_fullname ADD kwcolumn_opt columnname carglist */
#line 1480 "parse.y"
{
  yymsp[-1].minor.yy0.n = (int)(pParse->sLastToken.z-yymsp[-1].minor.yy0.z) + pParse->sLastToken.n;
  sqlite3AlterFinishAddColumn(pParse, &yymsp[-1].minor.yy0);
}
#line 3751 "parse.c"
        break;
      case 259: /* add_column_fullname ::= fullname */
#line 1484 "parse.y"
{
  disableLookaside(pParse);
  sqlite3AlterBeginAddColumn(pParse, yymsp[0].minor.yy185);
}
#line 3759 "parse.c"
        break;
      case 260: /* cmd ::= create_vtab */
#line 1494 "parse.y"
{sqlite3VtabFinishParse(pParse,0);}
#line 3764 "parse.c"
        break;
      case 261: /* cmd ::= create_vtab LP vtabarglist RP */
#line 1495 "parse.y"
{sqlite3VtabFinishParse(pParse,&yymsp[0].minor.yy0);}
#line 3769 "parse.c"
        break;
      case 262: /* create_vtab ::= createkw VIRTUAL TABLE ifnotexists nm dbnm USING nm */
#line 1497 "parse.y"
{
    sqlite3VtabBeginParse(pParse, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0, yymsp[-4].minor.yy194);
}
#line 3776 "parse.c"
        break;
      case 263: /* vtabarg ::= */
#line 1502 "parse.y"
{sqlite3VtabArgInit(pParse);}
#line 3781 "parse.c"
        break;
      case 264: /* vtabargtoken ::= ANY */
      case 265: /* vtabargtoken ::= lp anylist RP */ yytestcase(yyruleno==265);
      case 266: /* lp ::= LP */ yytestcase(yyruleno==266);
#line 1504 "parse.y"
{sqlite3VtabArgExtend(pParse,&yymsp[0].minor.yy0);}
#line 3788 "parse.c"
        break;
      case 267: /* with ::= */
#line 1519 "parse.y"
{yymsp[1].minor.yy285 = 0;}
#line 3793 "parse.c"
        break;
      case 268: /* with ::= WITH wqlist */
#line 1521 "parse.y"
{ yymsp[-1].minor.yy285 = yymsp[0].minor.yy285; }
#line 3798 "parse.c"
        break;
      case 269: /* with ::= WITH RECURSIVE wqlist */
#line 1522 "parse.y"
{ yymsp[-2].minor.yy285 = yymsp[0].minor.yy285; }
#line 3803 "parse.c"
        break;
      case 270: /* wqlist ::= nm eidlist_opt AS LP select RP */
#line 1524 "parse.y"
{
  yymsp[-5].minor.yy285 = sqlite3WithAdd(pParse, 0, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy148, yymsp[-1].minor.yy243); /*A-overwrites-X*/
}
#line 3810 "parse.c"
        break;
      case 271: /* wqlist ::= wqlist COMMA nm eidlist_opt AS LP select RP */
#line 1527 "parse.y"
{
  yymsp[-7].minor.yy285 = sqlite3WithAdd(pParse, yymsp[-7].minor.yy285, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy148, yymsp[-1].minor.yy243);
}
#line 3817 "parse.c"
        break;
      default:
      /* (272) input ::= cmdlist */ yytestcase(yyruleno==272);
      /* (273) cmdlist ::= cmdlist ecmd */ yytestcase(yyruleno==273);
      /* (274) cmdlist ::= ecmd (OPTIMIZED OUT) */ assert(yyruleno!=274);
      /* (275) ecmd ::= SEMI */ yytestcase(yyruleno==275);
      /* (276) ecmd ::= explain cmdx SEMI */ yytestcase(yyruleno==276);
      /* (277) explain ::= */ yytestcase(yyruleno==277);
      /* (278) trans_opt ::= */ yytestcase(yyruleno==278);
      /* (279) trans_opt ::= TRANSACTION */ yytestcase(yyruleno==279);
      /* (280) trans_opt ::= TRANSACTION nm */ yytestcase(yyruleno==280);
      /* (281) savepoint_opt ::= SAVEPOINT */ yytestcase(yyruleno==281);
      /* (282) savepoint_opt ::= */ yytestcase(yyruleno==282);
      /* (283) cmd ::= create_table create_table_args */ yytestcase(yyruleno==283);
      /* (284) columnlist ::= columnlist COMMA columnname carglist */ yytestcase(yyruleno==284);
      /* (285) columnlist ::= columnname carglist */ yytestcase(yyruleno==285);
      /* (286) nm ::= ID|INDEXED */ yytestcase(yyruleno==286);
      /* (287) nm ::= STRING */ yytestcase(yyruleno==287);
      /* (288) nm ::= JOIN_KW */ yytestcase(yyruleno==288);
      /* (289) typetoken ::= typename */ yytestcase(yyruleno==289);
      /* (290) typename ::= ID|STRING */ yytestcase(yyruleno==290);
      /* (291) signed ::= plus_num (OPTIMIZED OUT) */ assert(yyruleno!=291);
      /* (292) signed ::= minus_num (OPTIMIZED OUT) */ assert(yyruleno!=292);
      /* (293) carglist ::= carglist ccons */ yytestcase(yyruleno==293);
      /* (294) carglist ::= */ yytestcase(yyruleno==294);
      /* (295) ccons ::= NULL onconf */ yytestcase(yyruleno==295);
      /* (296) conslist_opt ::= COMMA conslist */ yytestcase(yyruleno==296);
      /* (297) conslist ::= conslist tconscomma tcons */ yytestcase(yyruleno==297);
      /* (298) conslist ::= tcons (OPTIMIZED OUT) */ assert(yyruleno!=298);
      /* (299) tconscomma ::= */ yytestcase(yyruleno==299);
      /* (300) defer_subclause_opt ::= defer_subclause (OPTIMIZED OUT) */ assert(yyruleno!=300);
      /* (301) resolvetype ::= raisetype (OPTIMIZED OUT) */ assert(yyruleno!=301);
      /* (302) selectnowith ::= oneselect (OPTIMIZED OUT) */ assert(yyruleno!=302);
      /* (303) oneselect ::= values */ yytestcase(yyruleno==303);
      /* (304) sclp ::= selcollist COMMA */ yytestcase(yyruleno==304);
      /* (305) as ::= ID|STRING */ yytestcase(yyruleno==305);
      /* (306) expr ::= term (OPTIMIZED OUT) */ assert(yyruleno!=306);
      /* (307) exprlist ::= nexprlist */ yytestcase(yyruleno==307);
      /* (308) nmnum ::= plus_num (OPTIMIZED OUT) */ assert(yyruleno!=308);
      /* (309) nmnum ::= nm (OPTIMIZED OUT) */ assert(yyruleno!=309);
      /* (310) nmnum ::= ON */ yytestcase(yyruleno==310);
      /* (311) nmnum ::= DELETE */ yytestcase(yyruleno==311);
      /* (312) nmnum ::= DEFAULT */ yytestcase(yyruleno==312);
      /* (313) plus_num ::= INTEGER|FLOAT */ yytestcase(yyruleno==313);
      /* (314) foreach_clause ::= */ yytestcase(yyruleno==314);
      /* (315) foreach_clause ::= FOR EACH ROW */ yytestcase(yyruleno==315);
      /* (316) trnm ::= nm */ yytestcase(yyruleno==316);
      /* (317) tridxby ::= */ yytestcase(yyruleno==317);
      /* (318) database_kw_opt ::= DATABASE */ yytestcase(yyruleno==318);
      /* (319) database_kw_opt ::= */ yytestcase(yyruleno==319);
      /* (320) kwcolumn_opt ::= */ yytestcase(yyruleno==320);
      /* (321) kwcolumn_opt ::= COLUMNKW */ yytestcase(yyruleno==321);
      /* (322) vtabarglist ::= vtabarg */ yytestcase(yyruleno==322);
      /* (323) vtabarglist ::= vtabarglist COMMA vtabarg */ yytestcase(yyruleno==323);
      /* (324) vtabarg ::= vtabarg vtabargtoken */ yytestcase(yyruleno==324);
      /* (325) anylist ::= */ yytestcase(yyruleno==325);
      /* (326) anylist ::= anylist LP anylist RP */ yytestcase(yyruleno==326);
      /* (327) anylist ::= anylist ANY */ yytestcase(yyruleno==327);
        break;
/********** End reduce actions ************************************************/
  };
  assert( yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact <= YY_MAX_SHIFTREDUCE ){
    if( yyact>YY_MAX_SHIFT ){
      yyact += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
    }
    yymsp -= yysize-1;
    yypParser->yytos = yymsp;
    yymsp->stateno = (YYACTIONTYPE)yyact;
    yymsp->major = (YYCODETYPE)yygoto;
    yyTraceShift(yypParser, yyact);
  }else{
    assert( yyact == YY_ACCEPT_ACTION );
    yypParser->yytos -= yysize;
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  sqlite3ParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/
/************ End %parse_failure code *****************************************/
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  sqlite3ParserTOKENTYPE yyminor         /* The minor type of the error token */
){
  sqlite3ParserARG_FETCH;
#define TOKEN yyminor
/************ Begin %syntax_error code ****************************************/
#line 32 "parse.y"

  UNUSED_PARAMETER(yymajor);  /* Silence some compiler warnings */
  assert( TOKEN.z[0] );  /* The tokenizer always gives us a token */
  sqlite3ErrorMsg(pParse, "near \"%T\": syntax error", &TOKEN);
#line 3937 "parse.c"
/************ End %syntax_error code ******************************************/
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  sqlite3ParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
#ifndef YYNOERRORRECOVERY
  yypParser->yyerrcnt = -1;
#endif
  assert( yypParser->yytos==yypParser->yystack );
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
/*********** Begin %parse_accept code *****************************************/
/*********** End %parse_accept code *******************************************/
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "sqlite3ParserAlloc" which describes the current state of the parser.
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
void sqlite3Parser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  sqlite3ParserTOKENTYPE yyminor       /* The value for the token */
  sqlite3ParserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  unsigned int yyact;   /* The parser action. */
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  int yyendofinput;     /* True if we are at the end of input */
#endif
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  yypParser = (yyParser*)yyp;
  assert( yypParser->yytos!=0 );
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif
  sqlite3ParserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput '%s'\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact <= YY_MAX_SHIFTREDUCE ){
      yy_shift(yypParser,yyact,yymajor,yyminor);
#ifndef YYNOERRORRECOVERY
      yypParser->yyerrcnt--;
#endif
      yymajor = YYNOCODE;
    }else if( yyact <= YY_MAX_REDUCE ){
      yy_reduce(yypParser,yyact-YY_MIN_REDUCE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
      yyminorunion.yy0 = yyminor;
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
        yy_syntax_error(yypParser,yymajor,yyminor);
      }
      yymx = yypParser->yytos->major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor, &yyminorunion);
        yymajor = YYNOCODE;
      }else{
        while( yypParser->yytos >= &yypParser->yystack
            && yymx != YYERRORSYMBOL
            && (yyact = yy_find_reduce_action(
                        yypParser->yytos->stateno,
                        YYERRORSYMBOL)) >= YY_MIN_REDUCE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yytos < yypParser->yystack || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
          yypParser->yyerrcnt = -1;
#endif
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          yy_shift(yypParser,yyact,YYERRORSYMBOL,yyminor);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor, yyminor);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
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
        yy_syntax_error(yypParser,yymajor, yyminor);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
        yypParser->yyerrcnt = -1;
#endif
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yytos>yypParser->yystack );
#ifndef NDEBUG
  if( yyTraceFILE ){
    yyStackEntry *i;
    char cDiv = '[';
    fprintf(yyTraceFILE,"%sReturn. Stack=",yyTracePrompt);
    for(i=&yypParser->yystack[1]; i<=yypParser->yytos; i++){
      fprintf(yyTraceFILE,"%c%s", cDiv, yyTokenName[i->major]);
      cDiv = ' ';
    }
    fprintf(yyTraceFILE,"]\n");
  }
#endif
  return;
}
