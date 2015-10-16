
/******************************************************************************
 *  C++ code file : general.cpp
 *  Project       : Pazcal Compiler
 *  Written by    : Lolos Konstantinos, Podimata Charikleia
 *                  (lolos.kostis@gmail.com, charapod@gmail.com)
 *  Date          : 2014-2015
 *  Description   : General use functions and tools
 *  Initial Code  : Nikolaos S. Papaspyrou (nickie@softlab.ntua.gr)
 ******************************************************************************/


/* ---------------------------------------------------------------------
   ---------------------------- Header files ---------------------------
   --------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "general.h"
#include "symbol.h"
#include "error.h"
#include "stdio.h"
#include <list>
#include <string>
#include <map>
using namespace std;

typedef RepReal (*REAL_OP)(RepReal, RepReal);
typedef int (*INT_OP)(int, int);
typedef bool (*COMP_OP)(RepReal, RepReal);
typedef bool (*COMP_BOOL)(bool, bool);

/* ---------------------------------------------------------------------
   -------------------- Memory management functions --------------------
   --------------------------------------------------------------------- */

void * newAlloc(size_t size)
{
    void * result = malloc(size);
   
    if (result == NULL)
        fatal("\rOut of memory");
    return result;
}

void deleteAlloc(void * p)
{
    // we use garbage collection
}

const char *getFileName(const char *filename)
{
    const char *s, *ret;
    ret = filename;
    for (s = filename; *s; s++)
        if (*s == '/')
            ret = s;
    return ret;
}

/* ---------------------------------------------------------------------
   ----------------------- General variables ---------------------------
   --------------------------------------------------------------------- */

char *trueConst, *falseConst;
char *zeroConst, *oneConst, *minusOneConst;
char *floatDecimals, *printLength;
char *new_line_const, *space_const;
unsigned int program_offset;
map<string, SymbolEntry *> lib_functions;

const char *filename;
bool ENABLE_OPT;
bool ENABLE_FINAL;
bool PRINT_IMM;
bool PRINT_DBG;
FILE *asm_out;
FILE *imm_out;
vector<quad> quads;

/* ---------------------------------------------------------------------
   -------------------------- Helper functions -------------------------
   --------------------------------------------------------------------- */

const char *print(const char* s);
void        initiate_constants();
void        initiate_library_functions();
void        library_func(const char *lib_name, const char *pazcal_name, Type retType, Type t1, Type t2, Type t3);
const char *addressOf(const char * place);
Type        funcResult(char * func);
Type        typePtr(Type type);
Type        typeArr(Type type);
const char *dereference(const char *place);
void        printQuad(quad q, int i);
char *      replace_extension(const char *filename, const char *ext);
void        config_imm();
void        config_final();
void        config_debug();
void        config_default();
REAL_OP     get_real_op(const char *op);
INT_OP      get_int_op(const char *op);
COMP_OP     get_comp_op(const char *op);
COMP_BOOL   get_comp_bool(const char *op);

/* Returns the number of the next quad */
int nextQuad()
{
    return quads.size() ;
}

/* Generates and stores the given quad */
void genQuad(const char *op, const char *x, const char *y, const char *z)
{
    SymbolEntry *xe = lookupEntry(x, LOOKUP_ALL_SCOPES, false);
    SymbolEntry *ye = lookupEntry(y, LOOKUP_ALL_SCOPES, false);
    SymbolEntry *ze = lookupEntry(z, LOOKUP_ALL_SCOPES, false);

    quad *q = (quad *) malloc(sizeof(*q));
    q->op = strdup(op);
    q->x  = strdup(x);
    q->y  = strdup(y);
    q->z  = strdup(z);
    q->xe = xe;
    q->ye = ye;
    q->ze = ze;
    quads.push_back(*q);
}

/* Generates and stores the given quad for parameters special */
void genQuad(const char *op, const char *x, const char *y, const char *z, SymbolEntry *par)
{
    SymbolEntry * xe = lookupEntry(x, LOOKUP_ALL_SCOPES, false);
    SymbolEntry * ye = lookupEntry(y, LOOKUP_ALL_SCOPES, false);

    quad *q = (quad *) malloc(sizeof(*q));
    q->op = strdup(op);
    q->x  = strdup(x);
    q->y  = strdup(y);
    q->z  = strdup(z);
    q->xe = xe;
    q->ye = ye;
    q->ze = par;
    quads.push_back(*q);
}

/* Returns the address of the given place */
const char *addressOf(const char *place)
{
    if (*place != '[') 
        internal("Problem addressOf");

    return strndup(place + 1,strlen(place+1)); 
}

/* Creates a list of quad tags that contains only x */
list<int> *makelist(int x)
{
    list<int> *l = new list<int>();
    l->push_back(x);
    return l;
}

/* Function to backpatch a quad tag into the list l */
void backpatch(list<int> *l, int quadNum)
{
    list<int>::iterator it;
    char *temp;
    for (it = l->begin(); it != l->end(); it++) {
        if (strcmp(quads[*it].z, "*")) {
            printQuads();
            internal("Tried to backpatch value %i in line %i, was %s before", quadNum, *it, quads[*it].z);
        }
        temp = (char *) malloc(10);
        sprintf(temp, "%d", quadNum);
        quads[*it].z = temp;
    }
}

/* Returns the type of the Nth argument of the function */
Type paramType(char *name, int n)
{
    SymbolEntry * entry = lookupEntry(name, LOOKUP_ALL_SCOPES, false);
    if (!entry) { 
        internal("%s function entry not found", name);
        return typeVoid;
    }
    if (entry->entryType != ENTRY_FUNCTION) {
        internal("%s entry found but was not a function", name);
        return typeVoid;
    }
    SymbolEntry * temp = entry->u.eFunction.firstArgument;
    while (temp != entry->u.eFunction.lastArgument) {
        if (n-- == 1) return temp->u.eParameter.type;
        temp = temp->u.eParameter.next;
    }
    if (n == 1) 
        return temp->u.eParameter.type;
    else 
        internal("Invalid number of arguments.");
    return typeVoid;
}

/* Returns the return type of the function */
Type funcResult(char * func)
{
    SymbolEntry * entry = lookupEntry(func, LOOKUP_ALL_SCOPES, false);
    if (!entry) { 
        internal("%s function entry not found", func);
        return typeVoid; /* default error return value */
    }
    if (entry->entryType != ENTRY_FUNCTION) {
        internal("%s entry found but was not a function", func);
        return typeVoid;
    }
    Type type = entry->u.eFunction.resultType;
    if (type->kind == TYPE_VOID) 
        internal("%s is a procedure and NOT a function!", func);
    return type;
}

/* Returns the type referenced by the given type (pointer) */
Type typePtr(Type type)
{
    if (type->kind != TYPE_POINTER) {
        internal("Argument type not of TYPE_POINTER type.");
        return typeVoid;
    }
    return type->refType;
}

/* Returns the type of the elements of the given array type */
Type typeArr(Type type)
{
    if (type->kind != TYPE_ARRAY && type->kind != TYPE_IARRAY) {
        internal("Argument type not of Array type.");
        return typeVoid;
    }
    return type->refType;
}

/* Prints all the intermediate code that has been generated so far */
void printQuads()
{
    unsigned int i;
    for (i = 0; i < quads.size(); ++i) {
        printQuad(quads[i], i);
    }
}

/* Prints a quad */
void printQuad(quad q, int i)
{
    fprintf(imm_out, "%d: %s, %s, %s, %s", i, q.op, print(q.x), print(q.y), print(q.z));
    if (PRINT_DBG)
        fprintf(imm_out, "  \t(%s, %s, %s)\n", get_type(q.xe), get_type(q.ye), get_type(q.ze));
    else
        fprintf(imm_out, "\n");
}

/* Returns the printable version of the given variable */
const char *print(const char* s)
{
    SymbolEntry *se = lookupEntry(s, LOOKUP_ALL_SCOPES, false);
    if (se && (entry_type(se) == ENTRY_TEMPORARY) && (se->u.eTemporary.isReference)) {
        char *temp = (char *) newAlloc(strlen(s)+3);
        sprintf(temp, "[%s]", s);
        return temp;
    }
    return s;
}

/* Prints lists */
void printList(list<int> *l,const char * s)
{
    list<int>::iterator i;
    printf("%s:",s);
    for (i = l->begin(); i != l->end(); ++i)
        printf(" %d ", *i);
    printf("\n");
}

/* Creates intermediate code for expr evaluation */
char *generatePlace(list<int> *TRUE, list<int> *FALSE) 
{
    SymbolEntry *entry = newTemporary(typeBoolean);
    backpatch(TRUE, nextQuad());
    
    genQuad(":=", trueConst, "-", entry->u.eTemporary.name);   
    genQuad("jump", "-", "-", to_string(nextQuad() + 2).c_str());
    backpatch(FALSE, nextQuad());
    genQuad(":=", falseConst, "-", entry->u.eTemporary.name);
    return entry->u.eTemporary.name;
}

/* Returns the type of a symbol table entry in string format */
const char *get_type(SymbolEntry *e)
{
    if (!e)
        return "-";

    Type type;
    switch (e->entryType) {
        case ENTRY_VARIABLE:
            type = e->u.eVariable.type;
            break;
        case ENTRY_CONSTANT:
            type = e->u.eConstant.type;
            break;
        case ENTRY_FUNCTION:
            type = e->u.eFunction.resultType;
            break;
        case ENTRY_PARAMETER:
            type = e->u.eParameter.type;
            break;
        case ENTRY_TEMPORARY:
            type = e->u.eTemporary.type;
            break;
        default:
            internal("quad symbol entry was of unknown type!");
            return "";
    }

    switch (type->kind) {
        case TYPE_VOID:
            return "void";
        case TYPE_INTEGER:
            return "int";
        case TYPE_BOOLEAN:
            return "bool";
        case TYPE_CHAR:
            return "char";
        case TYPE_REAL:
            return "real";
        case TYPE_ARRAY:
            return "arr";
        case TYPE_IARRAY:
            return "iarr";
        case TYPE_POINTER:
            return "ptr";
        default:
            internal("quad symbol entry was of unknown type!");
            return "";
    }
}

/* General initializations fuction */
void initiate()
{
    openScope();
    initiate_constants();
    initiate_library_functions();
}

/* Initiates all the global variables for constants */
void initiate_constants()
{
    /* create trueConst, falseConst */
    VALUE v;
    v.vBoolean = false; 
    SymbolEntry *entry = newTemporary(typeBoolean, &v);
    falseConst = entry->u.eTemporary.name;
    v.vBoolean = true;
    entry = newTemporary(typeBoolean, &v);
    trueConst = entry->u.eTemporary.name;

    /* create oneConst */
    v.vInteger = 1;
    entry = newTemporary(typeInteger, &v);
    oneConst = entry->u.eTemporary.name;
    
    /* create zeroConst */
    v.vInteger = 0;
    entry = newTemporary(typeInteger, &v);
    zeroConst = entry->u.eTemporary.name;

    /* create minusOneConst */
    v.vInteger = -1; 
    entry = newTemporary(typeInteger, &v);
    minusOneConst = entry->u.eTemporary.name;

    /* create default num of float decimals */
    v.vInteger = 6;
    entry = newTemporary(typeInteger, &v);
    floatDecimals = entry->u.eTemporary.name;

    /* create default print lenght */
    v.vInteger = 1;
    entry = newTemporary(typeInteger, &v);
    printLength = entry->u.eTemporary.name;

    /* create new constant for space */
    v.vString = " ";
    entry = newTemporary(typeString, &v);
    space_const = entry->u.eTemporary.name;
    
    /* create new constant for new line */
    v.vString = "\\n";
    entry = newTemporary(typeString, &v);
    new_line_const = entry->u.eTemporary.name;
}

/* Adds all the library functions to the symbol table */
void initiate_library_functions()
{
    /* Library Functions for input/output */
    library_func("_putchar",      "putchar",      typeVoid,    typeChar,    NULL,        NULL);
    library_func("_puts",         "puts",         typeVoid,    typeString,  NULL,        NULL);
    library_func("_WRITE_INT",    "WRITE_INT",    typeVoid,    typeInteger, typeInteger, NULL);
    library_func("_WRITE_REAL2",  "WRITE_REAL",   typeVoid,    typeReal,    typeInteger, typeInteger);
    library_func("_WRITE_BOOL",   "WRITE_BOOL",   typeVoid,    typeBoolean, typeInteger, NULL);
    library_func("_WRITE_CHAR",   "WRITE_CHAR",   typeVoid,    typeChar,    typeInteger, NULL);
    library_func("_WRITE_STRING", "WRITE_STRING", typeVoid,    typeString,  typeInteger, NULL);
    
    library_func("_READ_INT2",    "READ_INT",     typeInteger, NULL,        NULL,        NULL);
    library_func("_READ_BOOL2",   "READ_BOOL",    typeBoolean, NULL,        NULL,        NULL);
    library_func("_getchar2",     "getchar",      typeInteger, NULL,        NULL,        NULL);
    library_func("_READ_REAL2",   "READ_REAL",    typeReal,    NULL,        NULL,        NULL);
    library_func("_READ_STRING",  "READ_STRING",  typeVoid,    typeInteger, typeString,  NULL);

    /* Mathematical Functions */
    library_func("_sin2",         "sin",          typeReal,    typeReal,    NULL,        NULL);
    library_func("_abs",          "abs",          typeInteger, typeInteger, NULL,        NULL);
    library_func("_fabs",         "fabs",         typeReal,    typeReal,    NULL,        NULL);
    library_func("_sqrt",         "sqrt",         typeReal,    typeReal,    NULL,        NULL);
    library_func("_cos",          "cos",          typeReal,    typeReal,    NULL,        NULL);
    library_func("_tan",          "tan",          typeReal,    typeReal,    NULL,        NULL);
    library_func("_arctan",       "arctan",       typeReal,    typeReal,    NULL,        NULL);
    library_func("_exp2",         "exp",          typeReal,    typeReal,    NULL,        NULL);
    library_func("_ln2",          "ln",           typeReal,    typeReal,    NULL,        NULL);
    library_func("_pi",           "pi",           typeReal,    NULL,        NULL,        NULL);

    /* Conversion Functions */
    library_func("_trunc2",       "trunc",        typeReal,    typeReal,    NULL,        NULL);
    library_func("_round2",       "round",        typeReal,    typeReal,    NULL,        NULL);
    library_func("_TRUNC2",       "TRUNC",        typeInteger, typeReal,    NULL,        NULL);
    library_func("_ROUND2",       "ROUND",        typeInteger, typeReal,    NULL,        NULL);
    
    /* String Manipulation Functions */
    library_func("_strlen",       "strlen",       typeInteger, typeString,  NULL,        NULL);
    library_func("_strcmp",       "strcmp",       typeInteger, typeString,  typeString,  NULL);
    library_func("_strcpy",       "strcpy",       typeVoid,    typeString,  typeString,  NULL);
    library_func("_strcat",       "strcat",       typeVoid,    typeString,  typeString,  NULL);
}

/* Adds an entry to the symbol table for the given library function */
void library_func(const char *lib_name, const char *pazcal_name, Type retType, Type t1, Type t2, Type t3)
{
    SymbolEntry *f = newFunction(lib_name);
    openScope();
    if (t1) newParameter("__x", t1, isArray(t1) ? PASS_BY_REFERENCE : PASS_BY_VALUE, f);
    if (t2) newParameter("__y", t2, isArray(t2) ? PASS_BY_REFERENCE : PASS_BY_VALUE, f);
    if (t3) newParameter("__z", t3, isArray(t3) ? PASS_BY_REFERENCE : PASS_BY_VALUE, f);
    endFunctionHeader(f, retType);
    closeScope();
    lib_functions[pazcal_name] = f;
}

/* Returns the mode the Nth parameter of the given function is passed with */
const char *paramMode(const char *id, int N)
{
    int i;
    SymbolEntry *x, *entry = lookupEntry(id, LOOKUP_ALL_SCOPES, false);
    if ((!entry) || (entry->entryType != ENTRY_FUNCTION) || !(entry->u.eFunction.firstArgument)) 
        internal("paramMode called with invalid fuction id");
    x = entry->u.eFunction.firstArgument;
    for (i=0;i<N-1;i++) {
        if (x == entry->u.eFunction.lastArgument) 
            internal("paramMode called with out of bounds argument number");
        x = x->u.eParameter.next;
    }
    if (x->u.eParameter.mode == PASS_BY_VALUE)
        return "V";
    return "R";
}

/* Merges the elements of the second list into the first list */
void merge(list<int> *l1, list<int> *l2)
{
    (*(l1)).merge(*(l2));
}

/* Generates the intermediate code that prints a space character */
void write_space()
{
    call_write(typeString, space_const, printLength, NULL);
}

/* Generates the intermediate code that prints a new line */
void write_new_line()
{
    call_write(typeString, new_line_const, printLength, NULL);
}

/* Generates intermediate code for WRITES */
void call_write(Type t, const char *printed, const char *length, const char *num_decimals)
{
    SymbolEntry *write_func, *par;
    switch (t->kind) { 
        case TYPE_VOID:
            error("unprintable type");
            break;
        case TYPE_INTEGER:
            write_func = lookupEntry("_WRITE_INT", LOOKUP_ALL_SCOPES, false);
            par = write_func->u.eFunction.firstArgument;
            genQuad("par", printed, "V", "-", par);
            par = par->u.eParameter.next;
            genQuad("par", length, "V", "-", par);
            genQuad("call","-","-","_WRITE_INT");
            break;
        case TYPE_BOOLEAN:
            write_func = lookupEntry("_WRITE_BOOL", LOOKUP_ALL_SCOPES, false);
            par = write_func->u.eFunction.firstArgument;
            genQuad("par", printed, "V", "-", par);
            par = par->u.eParameter.next;
            genQuad("par", length, "V", "-", par);
            genQuad("call","-","-","_WRITE_BOOL");
            break;
        case TYPE_CHAR:
            write_func = lookupEntry("_WRITE_CHAR", LOOKUP_ALL_SCOPES, false);
            par = write_func->u.eFunction.firstArgument;
            genQuad("par", printed, "V", "-", par);
            par = par->u.eParameter.next;
            genQuad("par", length, "V", "-", par);
            genQuad("call","-","-","_WRITE_CHAR");
            break;
        case TYPE_REAL:
            write_func = lookupEntry("_WRITE_REAL2", LOOKUP_ALL_SCOPES, false);
            par = write_func->u.eFunction.firstArgument;
            genQuad("par", printed, "V", "-", par);
            par = par->u.eParameter.next;
            genQuad("par", length, "V", "-", par);
            par = par->u.eParameter.next;
            genQuad("par", num_decimals, "V", "-", par);
            genQuad("call","-","-","_WRITE_REAL2");
            break;
        case TYPE_ARRAY:
        case TYPE_IARRAY:
            if (t->refType->kind != TYPE_CHAR)
                error("cannot print arrays");
            write_func = lookupEntry("_WRITE_STRING", LOOKUP_ALL_SCOPES, false);
            par = write_func->u.eFunction.firstArgument;
            genQuad("par", printed, "R", "-", par);
            par = par->u.eParameter.next;
            genQuad("par", length, "V", "-", par);
            genQuad("call","-","-","_WRITE_STRING");
            break;
        default:
            internal("call_write: unknown parameter type");
    }
}

/* returns the type of the result of a bin_expr between two arithmetic types
   assumes the given types are reals, ints or chars */
Type calc_type(Type t1, Type t2)
{
    int k1 = t1->kind, k2 = t2->kind;
    if (k1 == TYPE_REAL || k2 == TYPE_REAL)
        return typeReal;
    if (k1 == TYPE_INTEGER || k2 == TYPE_INTEGER)
        return typeInteger;
    return typeChar;
}

/* returns TRUE is t1 is assignable to t2 */ 
bool assignable_var_def(Type t1, Type t2) 
{
    int k1 = t1->kind, k2 = t2->kind;
    if (k1 == TYPE_INTEGER) 
        return (k2 == TYPE_INTEGER || k2 == TYPE_CHAR || k2 == TYPE_REAL);
    else if (k1 == TYPE_CHAR)
        return (k2 == TYPE_INTEGER || k2 == TYPE_CHAR);
    else if (k1 == TYPE_REAL)
        return (k2 == TYPE_REAL);
    else if (k1 == TYPE_BOOLEAN)
        return (k2 == TYPE_BOOLEAN);
    else if (k1 == TYPE_VOID)
        return true;
    internal("Assignable called with invalid types.");

    return false;
}

/* Returns true if the variable is a constant */
bool isConst(SymbolEntry *var)
{
    return (var->entryType == ENTRY_CONSTANT) || 
           (var->entryType == ENTRY_TEMPORARY && var->u.eTemporary.isConst);
}

/* Returns true if the variable is global */
bool isGlobal(SymbolEntry *var)
{
    return var->isGlobal;
}

/* Returns true if the variable is temporary */
bool isTemp(SymbolEntry *var)
{
    return var->entryType == ENTRY_TEMPORARY;
}

/* Returns true if the given temporary is array element */
bool isArrayElement(SymbolEntry *var)
{
    return var->u.eTemporary.isArrayElement;
}

/* Parses the command line arguments */
void parse_arguments(int argc, char **argv)
{
    int c;
    int opt_i = 0, opt_o = 0, opt_f = 0, opt_d = 0;
    while ((c = getopt(argc, argv, "iofd")) != -1) {
        switch (c) {
        case 'i': opt_i = 1; break;
        case 'o': opt_o = 1; break;
        case 'f': opt_f = 1; break;
        case 'd': opt_d = 1; break;
        case '?': break;
        default : printf("?? getopt returned character code 0%o ??\n", c); 
                  exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        filename = argv[optind];
    } else if (!opt_i && !opt_f) {
        printf("Usage: %s [-oifd] <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // -ifo arguments are exclusive
    if ((opt_i && opt_d) || (opt_i && opt_f) || (opt_d && opt_f)) {
        printf("Error: -i -o -f flags are exclusive\n");
        exit(EXIT_FAILURE);
    }

    if (opt_i)      config_imm();
    else if (opt_f) config_final();
    else if (opt_d) config_debug();
    else            config_default();
        
    // enable optimizations?
    if (opt_o)      ENABLE_OPT = true;
    else            ENABLE_OPT = false;
}

/* -i flag: program from stdin, intermediate code to stdout */
void config_imm()
{
    ENABLE_FINAL = false;
    PRINT_IMM    = true;
    PRINT_DBG    = false;
    imm_out      = stdout;
    asm_out      = NULL;
}

/* -f flag: program from stdin, final code to stdout */
void config_final()
{
    ENABLE_FINAL = true;
    PRINT_IMM    = false;
    PRINT_DBG    = false;
    imm_out      = NULL;
    asm_out      = stdout;
}

/* -d flag: program from file, code and debug comments to stdout */
void config_debug()
{
    freopen(filename, "r", stdin);
    ENABLE_FINAL = true;
    PRINT_IMM    = true;
    PRINT_DBG    = true;
    imm_out      = stdout;
    asm_out      = stdout;
}

/* no flag: program from file, intermediate code to .imm, final code to .asm */
void config_default()
{
    ENABLE_FINAL = true;
    PRINT_IMM    = true;
    PRINT_DBG    = false;

    freopen(filename, "r", stdin);
    const char *file_imm = replace_extension(filename, ".imm");
    const char *file_asm = replace_extension(filename, ".asm");
    imm_out = fopen(file_imm, "w");
    if (!imm_out) {
        perror("open:");
        exit(EXIT_FAILURE);
    }
    asm_out = fopen(file_asm, "w");
    if (!asm_out) {
        perror("open:");
        exit(EXIT_FAILURE);
    }
}

/* Replaces the extension of a file if it is .pz, else it appends .pz */
char *replace_extension(const char *filename, const char *ext)
{
    int len     = strlen(filename);
    int ext_len = strlen(ext);
    char *name  = (char *) newAlloc(len + ext_len + 1);
    strcpy(name, filename);

    if (equal(filename + len - 3, ".pz"))
        strcpy(name + len - 3, ext);
    else 
        strcpy(name + len, ext);

    return name;
}

/* Returns true if the strings are equal */
inline bool equal(const char *s1, const char *s2)
{
    return !strcmp(s1, s2);
}

/* Returns true if the given type is a numeric type (int, char, real) */
bool is_numeric_type(Type t)
{
    return is_integer_type(t) || (t->kind == TYPE_REAL);
}

/* Returns true if the given symbol entry is of REAL type */
bool is_real_type(SymbolEntry *e)
{
    switch (e->entryType) {
        case ENTRY_VARIABLE:
            return is_real_type(e->u.eVariable.type);
        case ENTRY_CONSTANT:
            return is_real_type(e->u.eConstant.type);
        case ENTRY_FUNCTION:
            return is_real_type(e->u.eFunction.resultType);
        case ENTRY_PARAMETER:
            return is_real_type(e->u.eParameter.type);
        case ENTRY_TEMPORARY:
            return is_real_type(e->u.eTemporary.type);
        default:
            internal("unknown entryType");
            return false;
    }
}
bool is_real_type(Type type)
{
    return type->kind == TYPE_REAL;
}

/* Returns true if the given kind is a numeric type (int, char, real) */
bool is_numeric_kind(KIND kind)
{
    return is_integer_kind(kind) || (kind == TYPE_REAL);
}

/* Returns true if the given type is an integer type (int, char) */
bool is_integer_type(Type t)
{
    return (t->kind == TYPE_INTEGER) || (t->kind == TYPE_CHAR);
}

/* Returns true if the given type is an array or an iarray */
bool isArray(Type t)
{
    return (t->kind == TYPE_ARRAY) || (t->kind == TYPE_IARRAY);
}

/* Returns true if the given variable is an array or an iarray */
bool isArray(SymbolEntry *var)
{
    return isArray(var_kind(var));
}
bool isArray(KIND k)
{
    return (k == TYPE_ARRAY) || (k == TYPE_IARRAY);
}

/* Returns true if the given kind is a numeric type (int, char, real) */
bool is_integer_kind(KIND kind)
{
    return (kind == TYPE_INTEGER) || (kind == TYPE_CHAR);
}

/* Returns true if the given entry is a reference */
bool is_by_ref(SymbolEntry *e)
{
    KIND kind = var_kind(e);
    EntryType entryType = entry_type(e);
    if (entryType == ENTRY_PARAMETER) 
        return  (kind == TYPE_ARRAY) || (kind == TYPE_IARRAY) || e->u.eParameter.mode == PASS_BY_REFERENCE;
    else if (entryType == ENTRY_TEMPORARY) 
        return e->u.eTemporary.isReference;
    
    return false; 
}   

/* Returns the entry type of the given Symbol Entry */
EntryType entry_type(SymbolEntry *var)
{
    return var->entryType;
}

/* Returns the kind of the given variable */
KIND var_kind(SymbolEntry *var)
{
    if (!var)
        return TYPE_VOID;

    switch (var->entryType) {
        case ENTRY_VARIABLE:
            return var->u.eVariable.type->kind;
        case ENTRY_PARAMETER:
            return var->u.eParameter.type->kind;
        case ENTRY_CONSTANT:
            return var->u.eConstant.type->kind;
        case ENTRY_TEMPORARY:
            return var->u.eTemporary.type->kind;
        case ENTRY_FUNCTION:
            return var->u.eFunction.resultType->kind;
        default:
            internal("var_kind called with entry that has no type");
            return TYPE_VOID;
    }
}

/* Returns the type of the given variable */
Type var_type(SymbolEntry *var)
{
    if (!var)
        return typeVoid;

    switch (var->entryType) {
        case ENTRY_VARIABLE:
            return var->u.eVariable.type;
        case ENTRY_PARAMETER:
            return var->u.eParameter.type;
        case ENTRY_CONSTANT:
            return var->u.eConstant.type;
        case ENTRY_TEMPORARY:
            return var->u.eTemporary.type;
        case ENTRY_FUNCTION:
            return var->u.eFunction.resultType;
        default:
            internal("var_type called with entry that has no type");
            return typeVoid;
    }
}

/* Calculates the size of the parameters on the stack for a function call */
int calc_param_size(SymbolEntry *func)
{
    if (func->entryType != ENTRY_FUNCTION) 
        internal("calc_param_size called with non-function");
    
    SymbolEntry *first = func->u.eFunction.firstArgument;
    SymbolEntry *last  = func->u.eFunction.lastArgument;
    
    int count = 0;
    while (first != last) {
        if (is_by_ref(first))
            count += 4;
        else
            count += sizeOfType(first->u.eParameter.type);
        first  = first->u.eParameter.next;  
    }
    
    if (first) {
        if (is_by_ref(first)) 
            count += 4;
        else 
            count += sizeOfType(first->u.eParameter.type);
    }

    return count;
}

/* Returns true if the first type can be assigned to the second */
bool can_be_assigned_to(Type t1, Type t2)
{
    KIND k1 = t1->kind;
    KIND k2 = t2->kind;
    
    if (is_integer_type(t2))        // int and char
        return is_integer_type(t1);
    else if (is_numeric_type(t2))   // reals
        return is_numeric_type(t1);
    else if (k2 == TYPE_BOOLEAN)    // booleans
        return k1 == TYPE_BOOLEAN;
    else if (k2 == TYPE_POINTER)    // pointers
        return (k1 == TYPE_POINTER) && equal_types(t1->refType, t2->refType);
    else if (k2 == TYPE_ARRAY)      // arrays
        return isArray(t1) && (t1->size == t2->size) && equal_types(t1->refType, t2->refType);
    else if (k2 == TYPE_IARRAY)     // unknown size arrays
        return isArray(t1) && equal_types(t1->refType, t2->refType);

    return false;
}

/* Returns true if the two types are equal */
bool equal_types(Type t1, Type t2)
{
    KIND k1 = t1->kind;
    KIND k2 = t2->kind;

    switch (k1) {
        case TYPE_ARRAY:
            return (k2 == TYPE_ARRAY) && (t1->size == t2->size) && equal_types(t1->refType, t2->refType);
        case TYPE_IARRAY:
            return (k2 == TYPE_IARRAY) && equal_types(t1->refType, t2->refType);
        case TYPE_POINTER:
            return equal_types(t1->refType, t2->refType);
        default:
            return k1 == k2;
    }
    return false;
}

/* Returns the value of a real constant or const temporary */
RepReal get_real_val(SymbolEntry *real)
{
    if (!isConst(real))
        internal("get_real_val called with non constant symbol entry");

    if (entry_type(real) == ENTRY_CONSTANT)
        return real->u.eConstant.value.vReal;
    else if (entry_type(real) == ENTRY_TEMPORARY)
        return real->u.eTemporary.value.vReal;
    else {
        internal("get_real_val called with unknown constant entry type");
        return 0;
    }   
}

/* Return the whole "VALUE" struct */
VALUE get_val(SymbolEntry *val)
{
    if (!isConst(val))
        internal("get_val called with non constant symbol entry");

    if (entry_type(val) == ENTRY_CONSTANT)
        return val->u.eConstant.value;
    else if (entry_type(val) == ENTRY_TEMPORARY)
        return val->u.eTemporary.value;
    else {
        internal("get_val called with unknown constant entry type");
        VALUE v; //dummy value for return
        return v;
    }   
}

/* Returns the value of a string constant */
const char *get_str_val(SymbolEntry *str)
{
    if (!isConst(str))
        internal("get_str_val called with non constant symbol entry");

    if (entry_type(str) == ENTRY_CONSTANT)
        return str->u.eConstant.value.vString;
    else if (entry_type(str) == ENTRY_TEMPORARY)
        return str->u.eTemporary.value.vString;
    else {
        internal("get_str_val called with unknown constant entry type");
        return NULL;
    }   
}

/* Maximum of two integers */
int max(int a, int b)
{
    return a > b ? a : b;
}

/* Evaluates the result of the given binary operation */
VALUE eval_bin_expr(VALUE v1, VALUE v2, KIND k1, KIND k2, const char *op)
{
    VALUE res;
    RepReal (*real_op)(RepReal, RepReal) = get_real_op(op);
    int (*int_op)(int, int) = get_int_op(op);

    if (k1 == TYPE_INTEGER && k2 == TYPE_INTEGER)
        res.vInteger = int_op(v1.vInteger, v2.vInteger);

    else if (k1 == TYPE_INTEGER && k2 == TYPE_REAL)
        res.vReal = real_op(v1.vInteger, v2.vReal);

    else if (k1 == TYPE_INTEGER && k2 == TYPE_CHAR)
        res.vInteger = int_op(v1.vInteger, v2.vChar);

    else if (k1 == TYPE_REAL && k2 == TYPE_INTEGER)
        res.vReal = real_op(v1.vReal, v2.vInteger);

    else if (k1 == TYPE_REAL && k2 == TYPE_REAL)
        res.vReal = real_op(v1.vReal, v2.vReal);
    
    else if (k1 == TYPE_REAL && k2 == TYPE_CHAR)
        res.vReal = real_op(v1.vReal, v2.vChar);

    else if (k1 == TYPE_CHAR && k2 == TYPE_INTEGER) 
        res.vInteger = int_op(v1.vChar, v2.vInteger);

    else if (k1 == TYPE_CHAR && k2 == TYPE_REAL)
        res.vReal = real_op(v1.vChar, v2.vReal);

    else if (k1 == TYPE_CHAR && k2 == TYPE_CHAR)
        res.vInteger = int_op(v1.vChar, v2.vChar);

    return res;
}

/* Binary operations for floating point numbers */
RepReal op_add_real(RepReal x, RepReal y) { return x + y; }
RepReal op_sub_real(RepReal x, RepReal y) { return x - y; }
RepReal op_mul_real(RepReal x, RepReal y) { return x * y; }
RepReal op_div_real(RepReal x, RepReal y) 
{ 
    if (y == 0) error("Division by zero");
    return x / y; 
}

/* Returns the appropriate floating point number operator calculator */
REAL_OP get_real_op(const char *op)
{
    if (equal(op, "+"))      return op_add_real;
    else if (equal(op, "-")) return op_sub_real;
    else if (equal(op, "*")) return op_mul_real;
    else return op_div_real;
}

/* Binary operations for integers */
int op_add_int(int x, int y) { return x + y; }
int op_sub_int(int x, int y) { return x - y; }
int op_mul_int(int x, int y) { return x * y; }
int op_mod_int(int x, int y)
{ 
    if (y == 0) error("Modulo by zero");
    return x % y; 
}
int op_div_int(int x, int y) 
{ 
    if (y == 0) error("Division by zero");
    return x / y; 
}

/* Returns the appropriate integer operator calculator */
INT_OP get_int_op(const char *op)
{
    if (equal(op, "+"))      return op_add_int;
    else if (equal(op, "-")) return op_sub_int;
    else if (equal(op, "*")) return op_mul_int;
    else if (equal(op, "/")) return op_div_int;
    else return op_mod_int;
}

/* Evaluates the result of the given comparison */
bool eval_comp_expr(VALUE v1, VALUE v2, KIND k1, KIND k2, const char *op)
{
    bool (*compare)(RepReal, RepReal) = get_comp_op(op);
    bool (*compbool)(bool, bool) = get_comp_bool(op);

    if (k1 == TYPE_INTEGER && k2 == TYPE_INTEGER)
        return compare(v1.vInteger, v2.vInteger);

    else if (k1 == TYPE_INTEGER && k2 == TYPE_REAL)
        return compare(v1.vInteger, v2.vReal);

    else if (k1 == TYPE_INTEGER && k2 == TYPE_CHAR)
        return compare(v1.vInteger, v2.vChar);

    else if (k1 == TYPE_REAL && k2 == TYPE_INTEGER)
        return compare(v1.vReal, v2.vInteger);

    else if (k1 == TYPE_REAL && k2 == TYPE_REAL)
        return compare(v1.vReal, v2.vReal);
    
    else if (k1 == TYPE_REAL && k2 == TYPE_CHAR)
        return compare(v1.vReal, v2.vChar);

    else if (k1 == TYPE_CHAR && k2 == TYPE_INTEGER) 
        return compare(v1.vChar, v2.vInteger);

    else if (k1 == TYPE_CHAR && k2 == TYPE_REAL)
        return compare(v1.vChar, v2.vReal);

    else if (k1 == TYPE_CHAR && k2 == TYPE_CHAR)
        return compare(v1.vChar, v2.vChar);

    else if (k1 == TYPE_BOOLEAN && k2 == TYPE_BOOLEAN)
        return compbool(v1.vBoolean, v2.vBoolean);
    else 
        internal("eval_comp_expr called with invalid KIND kinds");

    return true;  //should never happen
}

/* Operations for comparisons between numbers */
bool op_equal_expr(RepReal x, RepReal y)     { return x == y; }
bool op_not_equal_expr(RepReal x, RepReal y) { return x != y; }
bool op_ge_expr(RepReal x, RepReal y)        { return x >= y; }
bool op_gt_expr(RepReal x, RepReal y)        { return x > y;  }
bool op_le_expr(RepReal x, RepReal y)        { return x <= y; }
bool op_lt_expr(RepReal x, RepReal y)        { return x < y;  }

/* Returns the appropriate number comparison calculator */
COMP_OP get_comp_op(const char *op)
{
    if (equal(op, "=="))      return op_equal_expr;
    else if (equal(op, "!=")) return op_not_equal_expr;
    else if (equal(op, ">=")) return op_ge_expr;
    else if (equal(op, ">"))  return op_gt_expr;
    else if (equal(op, "<=")) return op_le_expr;
    else return op_lt_expr;
}

/* Operations for comparisons between booleans */
bool op_equal_bool(bool x, bool y)      { return x == y; }
bool op_not_equal_bool(bool x, bool y)  { return x != y; }
bool op_ge_bool(bool x, bool y)         { return x >= y; }
bool op_gt_bool(bool x, bool y)         { return x > y;  }
bool op_le_bool(bool x, bool y)         { return x <= y; }
bool op_lt_bool(bool x, bool y)         { return x < y;  }

/* Returns the appropriate boolean comparison calculator */
COMP_BOOL get_comp_bool(const char *op)
{
    if (equal(op, "=="))      return op_equal_bool;
    else if (equal(op, "!=")) return op_not_equal_bool;
    else if (equal(op, ">=")) return op_ge_bool;
    else if (equal(op, ">"))  return op_gt_bool;
    else if (equal(op, "<=")) return op_le_bool;
    else return op_lt_bool;
}


/* Return the value of an integer constant. Assumes it isConst! */
int int_val(SymbolEntry *var)
{
    if (var->entryType == ENTRY_CONSTANT)
        return var->u.eConstant.value.vInteger;
    else
        return var->u.eTemporary.value.vInteger;
}

/* Return the value of a boolean constant. Assumes it isConst! */
bool bool_val(SymbolEntry *var)
{
    if (var->entryType == ENTRY_CONSTANT)
        return var->u.eConstant.value.vBoolean;
    else
        return var->u.eTemporary.value.vBoolean;
}

/* Return the value of an char constant. Assumes it isConst! */
char char_val(SymbolEntry *var)
{
    if (var->entryType == ENTRY_CONSTANT)
        return var->u.eConstant.value.vChar;
    else
        return var->u.eTemporary.value.vChar;
}

/* Return the value of an real constant. Assumes it isConst! */
RepReal real_val(SymbolEntry *var)
{
    if (var->entryType == ENTRY_CONSTANT)
        return var->u.eConstant.value.vReal;
    else
        return var->u.eTemporary.value.vReal;
}

