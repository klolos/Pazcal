
/******************************************************************************
 *  C++ code file : general.cpp
 *  Project       : Pazcal Compiler
 *  Written by    : Lolos Konstantinos, Podimata Charikleia
 *                  (lolos.kostis@gmail.com, charapod@gmail.com)
 *  Date          : 2014-2015
 *  Description   : General use functions and tools, header file
 *  Initial Code  : Nikolaos S. Papaspyrou (nickie@softlab.ntua.gr)
 ******************************************************************************/

#ifndef __GENERAL_H__
#define __GENERAL_H__
#include "error.h"
#include "symbol.h"
#include <string>
#include <vector>
#include <list>
#include <map>
using namespace std;

typedef struct quad {
    const char *op;  /* name of operator */ 
    const char *x;   /* 1st operand */
    const char *y;   /* 2nd operand */
    const char *z;   /* 3rd operand */
    SymbolEntry *xe; /* Symbol Table Entries */
    SymbolEntry *ye; 
    SymbolEntry *ze;
} quad;

extern int lineno;
extern const char * filename;
extern bool ENABLE_OPT;
extern bool ENABLE_FINAL;
extern bool PRINT_IMM;
extern bool PRINT_DBG;
extern FILE *asm_out;
extern FILE *imm_out;
extern FILE *dbg_out;
extern char *trueConst, *falseConst, *zeroConst, *oneConst, *minusOneConst, *floatDecimals, *printLength;
extern char *new_line_const, *space_const;
extern unsigned int program_offset;
extern map<string, SymbolEntry *> lib_functions;
void yyerror(const char * msg);

void *      newAlloc    (size_t);
void        deleteAlloc (void *);
const char *getFileName(const char *filename);


void        initiate();
int         max(int a, int b);
bool        equal(const char *s1, const char *s2);

void        backpatch(list<int> *l, int quadNum);
void        merge(list<int> *l1, list<int> *l2);
int         nextQuad();
void        genQuad(const char *op, const char * x, const char * y, const char * z);
void        genQuad(const char *op, const char *x, const char *y, const char *z, SymbolEntry *par);
void        printQuads();
bool        can_be_assigned_to(Type t1, Type t2);
bool        assignable_var_def(Type t1, Type t2);
char *      generatePlace(list<int> *TRUE, list<int> *FALSE);
Type        paramType(const char * name, int n);
const char *paramMode(const char * id, int N);
void        parse_arguments(int argc, char **argv);
list<int>   *makelist(int x);
VALUE       eval_bin_expr(VALUE v1, VALUE v2, KIND k1, KIND k2, const char *op);
bool        eval_comp_expr(VALUE v1, VALUE v2, KIND k1, KIND k2, const char *op);

bool        equal_types(Type t1, Type t2);
KIND        var_kind(SymbolEntry *var);
Type        var_type(SymbolEntry *var);
EntryType   entry_type(SymbolEntry *var);
Type        calc_type(Type t1, Type t2);
bool        isConst(SymbolEntry *var);
bool        isArrayElement(SymbolEntry *var);
bool        isTemp(SymbolEntry *var);
bool        isGlobal(SymbolEntry *var);
bool        is_real_type(SymbolEntry *e);
bool        is_real_type(Type type);
bool        is_numeric_type(Type t);
bool        is_numeric_kind(KIND kind);
bool        is_integer_type(Type t);
bool        is_integer_kind(KIND kind);
bool        is_numeric(SymbolEntry *e);
bool        is_by_ref(SymbolEntry *e);
bool        isArray(Type t);
bool        isArray(KIND k);
bool        isArray(SymbolEntry *var);

int         int_val(SymbolEntry *var);
char        char_val(SymbolEntry *var);
bool        bool_val(SymbolEntry *var);
RepReal     real_val(SymbolEntry *var);
VALUE       get_val(SymbolEntry *var);
RepReal     get_real_val(SymbolEntry *real);
const char *get_str_val(SymbolEntry *str);
const char *get_type(SymbolEntry *e);
int         calc_param_size(SymbolEntry *e);
void        call_write(Type t, const char *p1, const char * p2, const char *p3);
void        write_space();
void        write_new_line();
void        printList(list<int> *l,const char *s);

#endif


