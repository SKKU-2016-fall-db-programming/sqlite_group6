/* Automatically generated.  Do not edit */
/* See the tool/mkopcodeh.tcl script for details */
#define OP_Savepoint       0
#define OP_AutoCommit      1
#define OP_Transaction     2
#define OP_SorterNext      3
#define OP_PrevIfOpen      4
#define OP_NextIfOpen      5
#define OP_Prev            6
#define OP_Next            7
#define OP_Checkpoint      8
#define OP_JournalMode     9
#define OP_Vacuum         10
#define OP_VFilter        11 /* synopsis: iplan=r[P3] zplan='P4'           */
#define OP_VUpdate        12 /* synopsis: data=r[P3@P2]                    */
#define OP_Goto           13
#define OP_Gosub          14
#define OP_InitCoroutine  15
#define OP_Yield          16
#define OP_MustBeInt      17
#define OP_Jump           18
#define OP_Not            19 /* same as TK_NOT, synopsis: r[P2]= !r[P1]    */
#define OP_Once           20
#define OP_If             21
#define OP_IfNot          22
#define OP_SeekLT         23 /* synopsis: key=r[P3@P4]                     */
#define OP_SeekLE         24 /* synopsis: key=r[P3@P4]                     */
#define OP_SeekGE         25 /* synopsis: key=r[P3@P4]                     */
#define OP_SeekGT         26 /* synopsis: key=r[P3@P4]                     */
#define OP_Or             27 /* same as TK_OR, synopsis: r[P3]=(r[P1] || r[P2]) */
#define OP_And            28 /* same as TK_AND, synopsis: r[P3]=(r[P1] && r[P2]) */
#define OP_NoConflict     29 /* synopsis: key=r[P3@P4]                     */
#define OP_NotFound       30 /* synopsis: key=r[P3@P4]                     */
#define OP_Found          31 /* synopsis: key=r[P3@P4]                     */
#define OP_SeekRowid      32 /* synopsis: intkey=r[P3]                     */
#define OP_NotExists      33 /* synopsis: intkey=r[P3]                     */
#define OP_IsNull         34 /* same as TK_ISNULL, synopsis: if r[P1]==NULL goto P2 */
#define OP_NotNull        35 /* same as TK_NOTNULL, synopsis: if r[P1]!=NULL goto P2 */
#define OP_Ne             36 /* same as TK_NE, synopsis: if r[P1]!=r[P3] goto P2 */
#define OP_Eq             37 /* same as TK_EQ, synopsis: if r[P1]==r[P3] goto P2 */
#define OP_Gt             38 /* same as TK_GT, synopsis: if r[P1]>r[P3] goto P2 */
#define OP_Le             39 /* same as TK_LE, synopsis: if r[P1]<=r[P3] goto P2 */
#define OP_Lt             40 /* same as TK_LT, synopsis: if r[P1]<r[P3] goto P2 */
#define OP_Ge             41 /* same as TK_GE, synopsis: if r[P1]>=r[P3] goto P2 */
#define OP_Last           42
#define OP_BitAnd         43 /* same as TK_BITAND, synopsis: r[P3]=r[P1]&r[P2] */
#define OP_BitOr          44 /* same as TK_BITOR, synopsis: r[P3]=r[P1]|r[P2] */
#define OP_ShiftLeft      45 /* same as TK_LSHIFT, synopsis: r[P3]=r[P2]<<r[P1] */
#define OP_ShiftRight     46 /* same as TK_RSHIFT, synopsis: r[P3]=r[P2]>>r[P1] */
#define OP_Add            47 /* same as TK_PLUS, synopsis: r[P3]=r[P1]+r[P2] */
#define OP_Subtract       48 /* same as TK_MINUS, synopsis: r[P3]=r[P2]-r[P1] */
#define OP_Multiply       49 /* same as TK_STAR, synopsis: r[P3]=r[P1]*r[P2] */
#define OP_Divide         50 /* same as TK_SLASH, synopsis: r[P3]=r[P2]/r[P1] */
#define OP_Remainder      51 /* same as TK_REM, synopsis: r[P3]=r[P2]%r[P1] */
#define OP_Concat         52 /* same as TK_CONCAT, synopsis: r[P3]=r[P2]+r[P1] */
#define OP_SorterSort     53
#define OP_BitNot         54 /* same as TK_BITNOT, synopsis: r[P1]= ~r[P1] */
#define OP_Sort           55
#define OP_Rewind         56
#define OP_IdxLE          57 /* synopsis: key=r[P3@P4]                     */
#define OP_IdxGT          58 /* synopsis: key=r[P3@P4]                     */
#define OP_IdxLT          59 /* synopsis: key=r[P3@P4]                     */
#define OP_IdxGE          60 /* synopsis: key=r[P3@P4]                     */
#define OP_RowSetRead     61 /* synopsis: r[P3]=rowset(P1)                 */
#define OP_RowSetTest     62 /* synopsis: if r[P3] in rowset(P1) goto P2   */
#define OP_Program        63
#define OP_FkIfZero       64 /* synopsis: if fkctr[P1]==0 goto P2          */
#define OP_IfPos          65 /* synopsis: if r[P1]>0 then r[P1]-=P3, goto P2 */
#define OP_IfNotZero      66 /* synopsis: if r[P1]!=0 then r[P1]-=P3, goto P2 */
#define OP_DecrJumpZero   67 /* synopsis: if (--r[P1])==0 goto P2          */
#define OP_IncrVacuum     68
#define OP_VNext          69
#define OP_Init           70 /* synopsis: Start at P2                      */
#define OP_Return         71
#define OP_EndCoroutine   72
#define OP_HaltIfNull     73 /* synopsis: if r[P3]=null halt               */
#define OP_Halt           74
#define OP_Integer        75 /* synopsis: r[P2]=P1                         */
#define OP_Int64          76 /* synopsis: r[P2]=P4                         */
#define OP_String         77 /* synopsis: r[P2]='P4' (len=P1)              */
#define OP_Null           78 /* synopsis: r[P2..P3]=NULL                   */
#define OP_SoftNull       79 /* synopsis: r[P1]=NULL                       */
#define OP_Blob           80 /* synopsis: r[P2]=P4 (len=P1)                */
#define OP_Variable       81 /* synopsis: r[P2]=parameter(P1,P4)           */
#define OP_Move           82 /* synopsis: r[P2@P3]=r[P1@P3]                */
#define OP_Copy           83 /* synopsis: r[P2@P3+1]=r[P1@P3+1]            */
#define OP_SCopy          84 /* synopsis: r[P2]=r[P1]                      */
#define OP_IntCopy        85 /* synopsis: r[P2]=r[P1]                      */
#define OP_ResultRow      86 /* synopsis: output=r[P1@P2]                  */
#define OP_CollSeq        87
#define OP_Function0      88 /* synopsis: r[P3]=func(r[P2@P5])             */
#define OP_Function       89 /* synopsis: r[P3]=func(r[P2@P5])             */
#define OP_AddImm         90 /* synopsis: r[P1]=r[P1]+P2                   */
#define OP_RealAffinity   91
#define OP_Cast           92 /* synopsis: affinity(r[P1])                  */
#define OP_Permutation    93
#define OP_Compare        94 /* synopsis: r[P1@P3] <-> r[P2@P3]            */
#define OP_Column         95 /* synopsis: r[P3]=PX                         */
#define OP_Affinity       96 /* synopsis: affinity(r[P1@P2])               */
#define OP_String8        97 /* same as TK_STRING, synopsis: r[P2]='P4'    */
#define OP_MakeRecord     98 /* synopsis: r[P3]=mkrec(r[P1@P2])            */
#define OP_Count          99 /* synopsis: r[P2]=count()                    */
#define OP_ReadCookie    100
#define OP_SetCookie     101
#define OP_ReopenIdx     102 /* synopsis: root=P2 iDb=P3                   */
#define OP_OpenRead      103 /* synopsis: root=P2 iDb=P3                   */
#define OP_OpenWrite     104 /* synopsis: root=P2 iDb=P3                   */
#define OP_OpenAutoindex 105 /* synopsis: nColumn=P2                       */
#define OP_OpenEphemeral 106 /* synopsis: nColumn=P2                       */
#define OP_SorterOpen    107
#define OP_SequenceTest  108 /* synopsis: if( cursor[P1].ctr++ ) pc = P2   */
#define OP_OpenPseudo    109 /* synopsis: P3 columns in r[P2]              */
#define OP_Close         110
#define OP_ColumnsUsed   111
#define OP_Sequence      112 /* synopsis: r[P2]=cursor[P1].ctr++           */
#define OP_NewRowid      113 /* synopsis: r[P2]=rowid                      */
#define OP_Insert        114 /* synopsis: intkey=r[P3] data=r[P2]          */
#define OP_InsertInt     115 /* synopsis: intkey=P3 data=r[P2]             */
#define OP_Delete        116
#define OP_ResetCount    117
#define OP_SorterCompare 118 /* synopsis: if key(P1)!=trim(r[P3],P4) goto P2 */
#define OP_SorterData    119 /* synopsis: r[P2]=data                       */
#define OP_RowKey        120 /* synopsis: r[P2]=key                        */
#define OP_RowData       121 /* synopsis: r[P2]=data                       */
#define OP_Rowid         122 /* synopsis: r[P2]=rowid                      */
#define OP_NullRow       123
#define OP_SorterInsert  124
#define OP_IdxInsert     125 /* synopsis: key=r[P2]                        */
#define OP_IdxDelete     126 /* synopsis: key=r[P2@P3]                     */
#define OP_Seek          127 /* synopsis: Move P3 to P1.rowid              */
#define OP_IdxRowid      128 /* synopsis: r[P2]=rowid                      */
#define OP_Destroy       129
#define OP_Clear         130
#define OP_ResetSorter   131
#define OP_CreateIndex   132 /* synopsis: r[P2]=root iDb=P1                */
#define OP_Real          133 /* same as TK_FLOAT, synopsis: r[P2]=P4       */
#define OP_CreateTable   134 /* synopsis: r[P2]=root iDb=P1                */
#define OP_ParseSchema   135
#define OP_LoadAnalysis  136
#define OP_DropTable     137
#define OP_DropIndex     138
#define OP_DropTrigger   139
#define OP_IntegrityCk   140
#define OP_RowSetAdd     141 /* synopsis: rowset(P1)=r[P2]                 */
#define OP_Param         142
#define OP_FkCounter     143 /* synopsis: fkctr[P1]+=P2                    */
#define OP_MemMax        144 /* synopsis: r[P1]=max(r[P1],r[P2])           */
#define OP_OffsetLimit   145 /* synopsis: if r[P1]>0 then r[P2]=r[P1]+max(0,r[P3]) else r[P2]=(-1) */
#define OP_AggStep0      146 /* synopsis: accum=r[P3] step(r[P2@P5])       */
#define OP_AggStep       147 /* synopsis: accum=r[P3] step(r[P2@P5])       */
#define OP_AggFinal      148 /* synopsis: accum=r[P1] N=P2                 */
#define OP_Expire        149
#define OP_TableLock     150 /* synopsis: iDb=P1 root=P2 write=P3          */
#define OP_VBegin        151
#define OP_VCreate       152
#define OP_VDestroy      153
#define OP_VOpen         154
#define OP_VColumn       155 /* synopsis: r[P3]=vcolumn(P2)                */
#define OP_VRename       156
#define OP_Pagecount     157
#define OP_MaxPgcnt      158
#define OP_CursorHint    159
#define OP_Noop          160
#define OP_Explain       161

/* Properties such as "out2" or "jump" that are specified in
** comments following the "case" for each opcode in the vdbe.c
** are encoded into bitvectors as follows:
*/
#define OPFLG_JUMP        0x01  /* jump:  P2 holds jmp target */
#define OPFLG_IN1         0x02  /* in1:   P1 is an input */
#define OPFLG_IN2         0x04  /* in2:   P2 is an input */
#define OPFLG_IN3         0x08  /* in3:   P3 is an input */
#define OPFLG_OUT2        0x10  /* out2:  P2 is an output */
#define OPFLG_OUT3        0x20  /* out3:  P3 is an output */
#define OPFLG_INITIALIZER {\
/*   0 */ 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,\
/*   8 */ 0x00, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01,\
/*  16 */ 0x03, 0x03, 0x01, 0x12, 0x01, 0x03, 0x03, 0x09,\
/*  24 */ 0x09, 0x09, 0x09, 0x26, 0x26, 0x09, 0x09, 0x09,\
/*  32 */ 0x09, 0x09, 0x03, 0x03, 0x0b, 0x0b, 0x0b, 0x0b,\
/*  40 */ 0x0b, 0x0b, 0x01, 0x26, 0x26, 0x26, 0x26, 0x26,\
/*  48 */ 0x26, 0x26, 0x26, 0x26, 0x26, 0x01, 0x12, 0x01,\
/*  56 */ 0x01, 0x01, 0x01, 0x01, 0x01, 0x23, 0x0b, 0x01,\
/*  64 */ 0x01, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x02,\
/*  72 */ 0x02, 0x08, 0x00, 0x10, 0x10, 0x10, 0x10, 0x00,\
/*  80 */ 0x10, 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00,\
/*  88 */ 0x00, 0x00, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00,\
/*  96 */ 0x00, 0x10, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00,\
/* 104 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
/* 112 */ 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
/* 120 */ 0x00, 0x00, 0x10, 0x00, 0x04, 0x04, 0x00, 0x00,\
/* 128 */ 0x10, 0x10, 0x00, 0x00, 0x10, 0x10, 0x10, 0x00,\
/* 136 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x10, 0x00,\
/* 144 */ 0x04, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
/* 152 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00,\
/* 160 */ 0x00, 0x00,}

/* The sqlite3P2Values() routine is able to run faster if it knows
** the value of the largest JUMP opcode.  The smaller the maximum
** JUMP opcode the better, so the mkopcodeh.tcl script that
** generated this include file strives to group all JUMP opcodes
** together near the beginning of the list.
*/
#define SQLITE_MX_JUMP_OPCODE  70  /* Maximum JUMP opcode */
