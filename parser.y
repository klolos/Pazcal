%{

/******************************************************************************
 *  File          : parser.y
 *  Project       : Pazcal Compiler
 *  Written by    : Lolos Konstantinos, Podimata Charikleia
 *                  (lolos.kostis@gmail.com, charapod@gmail.com)
 *  Date          : 2014-2015
 *  Description   : Parser definition to be used by bison
 ******************************************************************************/

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <string.h>
#include "general.h"
#include "error.h"
#include "symbol.h"
#include "final.h"
#include "opt.h"
#include "gc/include/gc.h"
#include <list>
using namespace std;

enum WRITE_TYPE {WRITE, WRITELN, WRITESP, WRITESPLN};

/* external functions and variables */
extern int lineno;
extern int yylex(void);
extern vector<quad> quads;

/* global variables used by the parser for semantics checking */
list<char*> n_list;
list<const char*> p_list;
list<Type> t_list;
list<const char*> sw_list;
list<array_decl> a_list;
list<arg_decl> arg_list; 
Type currRetType;
int program = 0;
unsigned int currentOffset;
unsigned int funcOffset;
unsigned int maxFuncOffset;
WRITE_TYPE write_type;

%}
 
%union {
    char *n;        /* for string literals                  */
    char c;         /* for char literals -single characters */
    int i;          /* for integer literals                 */
    long double r;  /* for real literals                    */
    /* Typed types that have a type */
    struct { 
        Type type;
    } t;
    /* function calls */
    struct { 
        Type type;
        const char *place;
    } call;
    /* ALL expressions */
    struct {
        Type type;
        bool isConst;
        VALUE value;
        const char *place;
        list<int> *TRUE;
        list<int> *FALSE;
        list<const char *> *places;
        int i;
    } e;
    /* lvalues */
    struct {
        Type type;
        char * name;
        bool isConst;
        VALUE value;
        const char * place;
    } lv;
    /* functions */
    struct {
        char *name;
        Type type;
        bool byRef;
        SymbolEntry *entry;
    } f;
    /* statements */
    struct {
        int i; 
        list<int> *NEXT;
        list<int> *BREAK;
        list<int> *CONT;
    } s;
    type_node *t_n; 
    /* range */
    struct {
        const char *expr1_place;
        const char *expr2_place;
        const char *step_place;
        bool isUp;
    } rng;
    /* two integers! */
    struct {
        int i1;
        int i2;
    } ii;
    /* FOR */
    struct {
        int i;
        const char * c;
    } forr;
}

%token T_bool          "bool"
%token T_and           "and"
%token T_break         "break"
%token T_case          "case"
%token T_char          "char"
%token T_const         "const"
%token T_cont          "continue"
%token T_default       "default"
%token T_do            "do"
%token T_downto        "DOWNTO"
%token T_else          "else"
%token T_false         "false"
%token T_for           "FOR"
%token T_form          "FORM"
%token T_func          "FUNC"
%token T_if            "if"
%token T_int           "int"
%token T_mod           "MOD"
%token T_next          "NEXT"
%token T_not           "not"
%token T_or            "or"
%token T_proc          "PROC"
%token T_program       "PROGRAM"
%token T_real          "REAL"
%token T_return        "return"
%token T_step          "STEP"
%token T_switch        "switch"
%token T_to            "TO"
%token T_true          "true"
%token T_while         "while"
%token T_write         "WRITE"
%token T_writeln       "WRITELN"
%token T_writesp       "WRITESP"
%token T_writespln     "WRITESPLN"

%token T_eqeq          "=="
%token T_noteq         "!="
%token T_geq           ">="
%token T_leq           "<="
%token T_andand        "&&"
%token T_oror          "||"
%token T_plusplus      "++"
%token T_minmin        "--"
%token T_pluseq        "+="
%token T_mineq         "-="
%token T_muleq         "*="
%token T_diveq         "/="
%token T_modeq         "%="

%token<n> T_identifier
%token<i> T_int_lit
%token<r> T_real_lit
%token<c> T_char_lit
%token<n> T_string_lit

%nonassoc "then"
%nonassoc "else"
%left     "||" "or"
%left     "&&" "and"
%left     "==" "!="
%left     '>' '<' ">=" "<="
%left     '+' '-'
%left     '*' '/' '%' "MOD" /* operators infix */
%left     UNOP              /* unary operators (prefix) */

%type<t>    type c_def c_def_list v_init_list init_a_lst func_type c_expr_list
%type<call> call
%type<n>    ID
%type<e>    expr un_expr bin_expr const_expr init_value expr_list opt_c_expr switch_case 
%type<e>    case_list cases T_case switch_list "&&" "and" "||" "or" 
%type<lv>   l_value var_init
%type<c>    assign
%type<f>    formal routine_hd
%type<t_n>  call_args c_arg_list
%type<s>    stmt block block_body switch_body clause stmt_list default_case else_clause "else"
%type<i>    "do" "if"
%type<forr> "FOR"
%type<ii>   "while"
%type<rng>  range

%%

module: 
    decl_list
    {
        if (program != 1) 
            error("? Where is the program???");
    }
;

decl_list:
    /* nothing */
    | declaration decl_list 
;

declaration: 
    const_def 
    | var_def
    | routine
    | program 
;

const_def: 
    "const" type ID '=' const_expr c_def_list ';'
    {
        SymbolEntry *e;
        switch ($5.type->kind) {
            case TYPE_INTEGER:
                e = newConstant($3,$2.type,$5.value.vInteger);
                break;
            case TYPE_BOOLEAN:
                e = newConstant($3,$2.type,$5.value.vBoolean);
                break;
            case TYPE_CHAR:
                e = newConstant($3,$2.type,$5.value.vChar);
                break;
            case TYPE_REAL:
                e = newConstant($3,$2.type,$5.value.vReal);
                break;
            default: 
                internal("Unknown constant type."); 
        }    
        if ((($6.type->kind != TYPE_VOID) && ($2.type->kind != $6.type->kind)) ||
            ($2.type->kind != $5.type->kind))
            error("Declaration and value types do not match.");

        e->isGlobal = !currentScope->nestingLevel;
    }
;

ID: 
    T_identifier 
    { 
        $$ = $1; 
    }
;

c_def_list: 
    /* nothing */ 
    {
        $$.type = typeVoid;
    }
    | c_def c_def_list
    {
        if (($2.type->kind != TYPE_VOID) && ($1.type->kind != $2.type->kind)) {
            $$.type = typeVoid;
            error("Declaration and value types do not match.");
        } else
            $$.type = $1.type;
    }
;
            
c_def: 
    ',' ID '=' const_expr
    {
        SymbolEntry *e;
        $$.type = $4.type;
        switch ($4.type->kind) {
            case TYPE_INTEGER:
                e = newConstant($2,$4.type,$4.value.vInteger);
                break;
            case TYPE_BOOLEAN:
                e = newConstant($2,$4.type,$4.value.vBoolean);
                break;
            case TYPE_CHAR:
                e = newConstant($2,$4.type,$4.value.vChar);
                break;
            case TYPE_REAL:
                e = newConstant($2,$4.type,$4.value.vReal);
                break;
            default: 
                internal("Unknown constant type."); 
        }    
        e->isGlobal = !currentScope->nestingLevel;
    }
;

var_def: 
    type var_init v_init_list ';'
    {
        list<char*>::iterator names        = n_list.begin();
        list<const char*>::iterator places = p_list.begin();
        list<Type>::iterator types         = t_list.begin();
        list<array_decl>::iterator itt;
        Type type;
        SymbolEntry *e;

        // the names of the declared variables are stored in n_list
        // the types of the declared variables are stored in t_list

        // create the uninitialized variables in the symbol table
        for (; types != t_list.end(); names++, types++, places++) {
            e = newVariable(*names, $1.type);
            if ((*types)->kind != TYPE_VOID) 
                genQuad(":=", *places, "-", *names);
            if (!assignable_var_def(*types, $1.type))
                error("Declaration and initialization types are incompatible.");
            e->isGlobal = !currentScope->nestingLevel;
        }
        n_list.clear();
        t_list.clear();
        p_list.clear();

        // set the types of the declared variables
        for (itt = a_list.begin(); itt != a_list.end(); ++itt) {
            type = itt->type;
            while (type->refType->kind != TYPE_VOID) {
                type = type->refType;
            }
            type->refType = $1.type;
            e = newVariable(itt->name, itt->type);
            e->isGlobal = !currentScope->nestingLevel;
        }
        a_list.clear();
        
    }
;
            
v_init_list:
    /* nothing */ {}
    | ',' var_init v_init_list {}
;

var_init: 
    ID init_value
    {
        $$.type = $2.type;
        n_list.push_back($1);
        t_list.push_back($2.type);
        if ($2.type->kind != TYPE_VOID)
            p_list.push_back($2.place);
        else
            p_list.push_back("dummy");
    }
    | ID '[' const_expr ']' init_a_lst
    {
        int size;
        if (!$3.isConst) {
            error("Array size needs to be constant");
            size = 1;
        }
        else {
            if ($3.type->kind == TYPE_INTEGER)
                size = $3.value.vInteger;
            else if ($3.type->kind == TYPE_CHAR)
                size = $3.value.vChar;
            else { 
                size = 1;
                error("Array size must be an integer value"); 
            }
        }
        if (size <= 0) {
            error("Array size must be positive");
            size = 1;
        }

        $$.type = typeVoid;
        array_decl arr = {$1, typeArray(size, $5.type)};
        a_list.push_back(arr);
    }
;
            
init_value: 
    /* nothing */
    {
        $$.type = typeVoid;
    }
    | '=' expr
    {
        $$.type = $2.type;
        if ($2.isConst) {
            $$.value = $2.value;  
            $$.isConst = true;
        }
        $$.place = $2.place;
    }
;
            
init_a_lst  : 
    /* nothing */
    {
        $$.type = typeVoid;
    }
    | '[' const_expr ']' init_a_lst
    {
        int size;
        if (!$2.isConst) {
            error("Array size needs to be constant");
            size = 1;
        }
        else {
            if ($2.type->kind == TYPE_INTEGER)
                size = $2.value.vInteger;
            else if ($2.type->kind == TYPE_CHAR)
                size = $2.value.vChar;
            else {
                error("Array size must be an integer value"); 
                size = 1;
            }
        }
        if (size <= 0) {
            error("Array size must be positive");
            size = 1;
        }
        $$.type = typeArray(size, $4.type);
    }
;

routine_hd: 
    func_type ID '(' args ')'
    {
        list<arg_decl>::iterator it;
        SymbolEntry *p;
        PassMode pm;

        p = newFunction($2);
        forwardFunction(p);
        openScope();
        arg_list.reverse();

        /* add function parameters stored in arg_list */
        for (it = arg_list.begin(); it != arg_list.end(); ++it) {
            pm = it->byRef ? PASS_BY_REFERENCE : PASS_BY_VALUE;
            newParameter(it->name, it->type, pm, p);
        }
        arg_list.clear();
        endFunctionHeader(p, $1.type);
        currRetType = $1.type;

        $$.type = $1.type;
        $$.name = $2;
        $$.entry = p;
    }
;
            
func_type: 
    "PROC"
    {
        $$.type = typeVoid;
    }
    | "FUNC" type
    {
        $$.type = $2.type;
    }
;

args: 
    /* nothing */
    | type formal arg_list
    {
        Type argType = $2.type;
        arg_decl *arg = (arg_decl *) newAlloc(sizeof(* arg));
        arg->name = $2.name;
        if (argType->kind != TYPE_ARRAY && argType->kind != TYPE_IARRAY) 
            arg->type = $1.type;            /* if formal parameter is not an ARRAY */
        else {                              /* set type of last array to the declared type */
            while (argType->refType->kind == TYPE_ARRAY || argType->refType->kind == TYPE_IARRAY) {
                argType = argType->refType; 
            }
            argType->refType = $1.type; 
            arg->type = $2.type;
        }
        arg->byRef = $2.byRef;
        arg_list.push_back(*arg);
    }
;

arg_list: 
    /* nothing */
    | ',' type formal arg_list
    {
        Type argType = $3.type;
        arg_decl *arg = (arg_decl *) newAlloc(sizeof(* arg));
        arg->name = $3.name;
        if (argType->kind != TYPE_ARRAY && argType->kind != TYPE_IARRAY) 
            arg->type = $2.type;            /* if formal parameter is not an ARRAY */
        else {                              /* set type of last array to the declared type */
            while (argType->refType->kind == TYPE_ARRAY || argType->refType->kind == TYPE_IARRAY) {
                argType = argType->refType; 
            }
            argType->refType = $2.type; 
            arg->type = $3.type;
        }
        arg->byRef = $3.byRef;
        arg_list.push_back(*arg);
    }
;

formal: 
    ID
    {
        $$.name = $1;
        $$.byRef = false;
        $$.type = typeVoid;
    } 
    | '&' ID
    {
        $$.name = $2;
        $$.byRef = true;
        $$.type = typeVoid;
        
    }
    | ID '[' opt_c_expr ']' c_expr_list
    {
        $$.name = $1;
        $$.byRef = true;   /* Arrays ALWAYS by Reference */
        $$.type = typeVoid;
        if ($3.type->kind == TYPE_VOID) 
            $$.type = typeIArray($5.type); 
        else if ($3.type->kind != TYPE_INTEGER && $3.type->kind != TYPE_CHAR){
            error("Array dimensions need to be integers");
            $$.type = typeArray(1,$5.type);
        }
        else if ($3.type->kind == TYPE_CHAR) {
            if ($3.value.vChar <= 0) 
                error("Array dimensions need to be positive integers");
            $$.type = typeArray($3.value.vChar, $5.type);
        }
        else if ($3.type->kind == TYPE_INTEGER) {
            if ($3.value.vInteger <= 0)
                error("Array dimensions need to be positive integers");
            $$.type = typeArray($3.value.vInteger, $5.type);
        }
    }
;

const_expr: 
    expr 
    {
        $$ = $1;
        if (!$$.isConst) 
            error("Right value needs to be a constant expression.");
    }
;
            
opt_c_expr:
    /* nothing */
    {
        $$.type = typeVoid;
    }
    | const_expr
    {
        $$ = $1;
    }
;
            
c_expr_list: 
    /* nothing */
    {
        $$.type = typeVoid;
    }
    | '[' const_expr ']' c_expr_list
    {
        if ($2.type->kind != TYPE_INTEGER && $2.type->kind != TYPE_CHAR){
            error("Array dimensions need to be integers");
            $$.type = typeArray(1,$4.type);
        }
        else if ($2.type->kind == TYPE_CHAR) {
            if ($2.value.vChar <= 0) 
                error("Array dimensions need to be positive integers");
            $$.type = typeArray($2.value.vChar, $4.type);
        }
        else if ($2.type->kind == TYPE_INTEGER) {
            if ($2.value.vInteger <= 0)
                error("Array dimensions need to be positive integers");
            $$.type = typeArray($2.value.vInteger, $4.type);
        }
    }
;
            
routine: 
    routine_hd ';'
    {
        closeScope();
    }
    | routine_hd { genQuad("unit", $1.name, "-", "-"); } block
    {
        SymbolEntry *entry;
        
        entry = lookupEntry($1.name, LOOKUP_ALL_SCOPES,false);
        if (!entry || entry->entryType != ENTRY_FUNCTION) 
            internal("Routine_hd did not register to the function name");  
        else {
           if (!entry->u.eFunction.isForward) 
                error("Duplicate function definition"); 
        }
        $1.entry->u.eFunction.negOffset = currentScope->negOffset;
        closeScope();
        genQuad("endu", $1.name, "-", "-");
    }
;
            
type: 
    "int"    { $$.type = typeInteger; }
    | "bool" { $$.type = typeBoolean; }
    | "char" { $$.type = typeChar;    }
    | "REAL" { $$.type = typeReal;    }
;

            
program: 
    "PROGRAM" ID '(' ')' 
    { 
        genQuad("unit", $2, "-", "-"); 
        openScope();
        currRetType = typeVoid;
    } 
    block
    {
        program++;
        program_offset = currentOffset;
        genQuad("endu", $2, "-", "-");
        closeScope();
        currRetType = NULL;
    }
;

            
expr: 
    T_int_lit
    {
        $$.value.vInteger = $1;
        $$.isConst = true;
        $$.type = typeInteger;

        /* Intermediate Code */
        SymbolEntry *entry = newTemporary($$.type, &$$.value);
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | T_real_lit 
    {
        $$.isConst = true;
        $$.value.vReal = $1;
        $$.type = typeReal;

        /* Intermediate Code */
        SymbolEntry *entry = newTemporary($$.type, &$$.value);
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | T_char_lit 
    {   
        $$.isConst = true;
        $$.value.vChar = $1;
        $$.type = typeChar;

        /* Intermediate Code */
        SymbolEntry *entry = newTemporary($$.type, &$$.value);
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | T_string_lit 
    {
        $$.isConst = true;
        $$.value.vString = $1;
        $$.type = typeArray(strlen($1)+1,typeChar);

        /* Intermediate Code */
        SymbolEntry *entry = newTemporary($$.type, &$$.value);
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | "true"
    {
        $$.value.vBoolean = 1;
        $$.isConst = true;
        $$.type = typeBoolean;

        /* Intermediate Code */
        SymbolEntry *entry = newTemporary($$.type, &$$.value);
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | "false"
    {
        $$.value.vBoolean = 0;
        $$.isConst = true;
        $$.type = typeBoolean;

        /* Intermediate Code */
        SymbolEntry *entry = newTemporary($$.type, &$$.value);
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | '(' expr ')' 
    {
        $$ = $2;
        $$.TRUE  = $2.TRUE;
        $$.FALSE = $2.FALSE;
    }
    | l_value 
    {
        $$.type    = $1.type;
        $$.isConst = $1.isConst;
        $$.value   = $1.value;
        $$.place   = $1.place;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | call            
    {
        $$.type    = $1.type;
        $$.place   = $1.place;
        $$.isConst = false;
        $$.TRUE    = new list<int>();
        $$.FALSE   = new list<int>();
    }
    | un_expr
    {
        $$ = $1;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | bin_expr
    {
        $$ = $1;
        $$.TRUE  = $1.TRUE;
        $$.FALSE = $1.FALSE;
    }
;

l_value: 
    ID expr_list 
    {
        Type IDtype,exprType;
        exprType = $2.type;
        IDtype = typeInteger;  /* Default value to avoid typeVoid l_value */
        SymbolEntry * entry = lookupEntry($1,LOOKUP_ALL_SCOPES,true);
        if (!entry)
            error("unknown variable %s", $1);

        switch (entry->entryType) {
            case ENTRY_VARIABLE:
                IDtype = entry->u.eVariable.type;
                $$.isConst = false;
                break;
            case ENTRY_CONSTANT:
                IDtype = entry->u.eConstant.type;
                $$.isConst = true;
                $$.value = entry->u.eConstant.value;
                break;
            case ENTRY_FUNCTION:
                error("Invalid function call");    
                break;
            case ENTRY_PARAMETER: 
                IDtype = entry->u.eParameter.type;
                $$.isConst = false;
                break;
            case ENTRY_TEMPORARY:
                IDtype = entry->u.eTemporary.type;
                $$.isConst = entry->u.eTemporary.isConst;
                $$.value = entry->u.eTemporary.value;
                break;
        }
        
        list<const char *>::iterator it = (*$2.places).begin();
        const char * last_place = $1;
        while (exprType->kind == TYPE_IARRAY) {
            if (IDtype->kind != TYPE_ARRAY && IDtype->kind != TYPE_IARRAY) {
                error("Variable is not an array.");
                break;
            } 
            /* Intermediate Code */
            IDtype = IDtype->refType;
            SymbolEntry *newTemp = newTemporaryRef(IDtype);
            newTemp->u.eTemporary.isArrayElement = true;
            genQuad("array", last_place, *it, newTemp->id);
            it++;
            
            exprType = exprType->refType;
            last_place = newTemp->id;
        }
        $$.place = last_place; 
        $$.type = IDtype;

        if ($2.type->kind == TYPE_VOID)
            $$.place = $1;
    }
;
            
expr_list:
    /* nothing */
    {
        $$.type = typeVoid;
        $$.places = new list<const char *>();
    }
    | '[' expr ']' expr_list
    {
        if ($2.type->kind != TYPE_INTEGER && $2.type->kind != TYPE_CHAR) 
            error("Array pointer must be of integer type."); 
        $$.type = typeIArray($4.type);
        $$.places = $4.places;
        (*$$.places).push_front($2.place);
    }
;
            
un_expr: 
    '+' expr %prec UNOP 
    {
        if ($2.type->kind != TYPE_REAL && $2.type->kind != TYPE_INTEGER && $2.type->kind != TYPE_CHAR)
            error("Unary operator '+' only applies to numeric values");
        $$ = $2;

        /* Intermediate Code */
        $$.place = $2.place;
    }
    | '-' expr %prec UNOP
    {   
        $$ = $2;
        if ($2.type->kind == TYPE_REAL)
            $$.value.vReal *= -1;
        else if ($2.type->kind == TYPE_INTEGER)
            $$.value.vInteger *= -1;
        else if ($2.type->kind == TYPE_CHAR)
            $$.value.vChar *= -1;
        else
            error("Unary operator '-' only applies to numeric values");

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst)
            entry = newTemporary($$.type, &$$.value);
        else {
            entry = newTemporary($$.type);
            genQuad("-", zeroConst, $2.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
    }
    | '!' expr %prec UNOP
    {
        $$ = $2;
        $$.value.vBoolean = !$2.value.vBoolean;
        if ($2.type->kind != TYPE_BOOLEAN)
            error("Unary operator '!' only applies to boolean expressions");

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($2.isConst)
            entry = newTemporary($$.type, &($$.value));
        else {
            entry = newTemporary($$.type);
            genQuad("not", $2.place, "-", entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
    }
    | "not" expr %prec UNOP
    {
        $$ = $2;
        $$.value.vBoolean = !$2.value.vBoolean;
        if ($2.type->kind != TYPE_BOOLEAN)
            error("Unary operator '!' only applies to boolean expressions");

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst)
            entry = newTemporary($$.type, &$$.value);
        else {
            entry = newTemporary($$.type);
            genQuad("not", $2.place, " ", entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
    }
;

bin_expr: 
    expr '+' expr
    {       
        $$.isConst = false;
        if (is_numeric_type($1.type) && is_numeric_type($3.type)) {
            $$.type = calc_type($1.type, $3.type);
            if ($1.isConst && $3.isConst) {
                $$.isConst = true;
                $$.value = eval_bin_expr($1.value, $3.value, $1.type->kind, $3.type->kind, "+");
            }
        } else 
            error("Operator '+' applies only to numeric values.");
    
        // Intermediate Code
        SymbolEntry *entry;
        if ($$.isConst)
            entry = newTemporary($$.type, &$$.value);
        else {
            entry = newTemporary($$.type);
            genQuad("+", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr '-' expr
    {
        $$.isConst = false;
        if (is_numeric_type($1.type) && is_numeric_type($3.type)) {
            $$.type = calc_type($1.type, $3.type);
            if ($1.isConst && $3.isConst) {
                $$.isConst = true;
                $$.value = eval_bin_expr($1.value, $3.value, $1.type->kind, $3.type->kind, "-");
            }
        } else 
            error("Operator '-' applies only to numeric values.");

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst)
            entry = newTemporary($$.type, &$$.value);
        else {
            entry = newTemporary($$.type);
            genQuad("-", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr '*' expr
    {
        $$.isConst = false;
        if (is_numeric_type($1.type) && is_numeric_type($3.type)) {
            $$.type = calc_type($1.type, $3.type);
            if ($1.isConst && $3.isConst) {
                $$.isConst = true;
                $$.value = eval_bin_expr($1.value, $3.value, $1.type->kind, $3.type->kind, "*");
            }
        } else 
            error("Operator '*' applies only to numeric values.");

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst)
            entry = newTemporary($$.type, &$$.value);
        else {
            entry = newTemporary($$.type);
            genQuad("*", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr '/' expr
    {
        $$.isConst = false;
        if (is_numeric_type($1.type) && is_numeric_type($3.type)) {
            $$.type = calc_type($1.type, $3.type);
            if ($1.type->kind == TYPE_CHAR && $3.type->kind == TYPE_CHAR)
                $$.type = typeInteger;
            if ($1.isConst && $3.isConst) {
                $$.isConst = true;
                $$.value = eval_bin_expr($1.value, $3.value, $1.type->kind, $3.type->kind, "/");
                if ($1.type->kind == TYPE_CHAR && $3.type->kind == TYPE_CHAR)
                    $$.value.vInteger = $1.value.vChar / $3.value.vChar;
            }
        } else 
            error("Operator '/' applies only to numeric values.");

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst)
            entry = newTemporary($$.type, &$$.value);
        else {
            entry = newTemporary($$.type);
            genQuad("/", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr '%' expr
    {
        $$.isConst = false;
        $$.type = typeInteger;
        if (($1.type->kind != TYPE_INTEGER && $1.type->kind != TYPE_CHAR)  || 
            ($3.type->kind != TYPE_INTEGER && $3.type->kind != TYPE_CHAR)) 
            error("Operator '%%' applies only to numeric values.");
              
        if ($1.isConst && $3.isConst) {
            $$.isConst = true;
            $$.value = eval_bin_expr($1.value, $3.value, $1.type->kind, $3.type->kind, "%");
        }

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst)
            entry = newTemporary($$.type, &$$.value);
        else {
            entry = newTemporary($$.type);
            genQuad("%", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr "MOD" expr
    {
        $$.isConst = false;
        $$.type = typeInteger;
        if (($1.type->kind != TYPE_INTEGER && $1.type->kind != TYPE_CHAR)  || 
            ($3.type->kind != TYPE_INTEGER && $3.type->kind != TYPE_CHAR)) 
            error("Operator 'MOD' applies only to numeric values.");
              
        if ($1.isConst && $3.isConst) {
            $$.isConst = true;
            $$.value = eval_bin_expr($1.value, $3.value, $1.type->kind, $3.type->kind, "%");
        }

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst)
            entry = newTemporary($$.type, &$$.value);
        else {
            entry = newTemporary($$.type);
            genQuad("%", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr "==" expr 
    {
        $$.isConst = false;
        $$.type = typeBoolean;
        Type t1 = $1.type, t2 = $3.type;
        if (!is_numeric_type(t1) || !is_numeric_type(t2))
            error("== applied to non numeric values.");

        if ($1.isConst && $3.isConst) {
            $$.isConst = true;
            $$.value.vBoolean = eval_comp_expr($1.value, $3.value, $1.type->kind, $3.type->kind, "==");
        }

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst) {
            entry = newTemporary($$.type, &$$.value);
        }
        else {
            entry = newTemporary($$.type);
            genQuad("==", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr "!=" expr
    {
        $$.isConst = false;
        $$.type = typeBoolean;
        Type t1 = $1.type, t2 = $3.type;
        if (!is_numeric_type(t1) || !is_numeric_type(t2))
            error("!= applied to non numeric values.");
        
        if ($1.isConst && $3.isConst) {
            $$.isConst = true;
            $$.value.vBoolean = eval_comp_expr($1.value, $3.value, $1.type->kind, $3.type->kind, "!=");
        } 

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst) {
            entry = newTemporary($$.type, &$$.value);
        }
        else {
            entry = newTemporary($$.type);
            genQuad("!=", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr '<' expr
    {
        $$.isConst = false;
        $$.type = typeBoolean;
        Type t1 = $1.type, t2 = $3.type;
        if (!is_numeric_type(t1) || !is_numeric_type(t2))
            error("< applied to non numeric values.");
        
        if ($1.isConst && $3.isConst) {
            $$.isConst = true;
            $$.value.vBoolean = eval_comp_expr($1.value, $3.value, $1.type->kind, $3.type->kind, "<");
        } 

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst) {
            entry = newTemporary($$.type, &$$.value);
        }
        else {
            entry = newTemporary($$.type);
            genQuad("<", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr '>' expr
    {
        $$.isConst = false;
        $$.type = typeBoolean;
        Type t1 = $1.type, t2 = $3.type;
        if (!is_numeric_type(t1) || !is_numeric_type(t2))
            error("> applied to non numeric values.");
        
        if ($1.isConst && $3.isConst) {
            $$.isConst = true;
            $$.value.vBoolean = eval_comp_expr($1.value, $3.value, $1.type->kind, $3.type->kind, ">");
        } 

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst) {
            entry = newTemporary($$.type, &$$.value);
        }
        else {
            entry = newTemporary($$.type);
            genQuad(">", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr "<=" expr
    {
        $$.isConst = false;
        $$.type = typeBoolean;
        Type t1 = $1.type, t2 = $3.type;
        if (!is_numeric_type(t1) || !is_numeric_type(t2))
            error("<= applied to non numeric values.");
        
        if ($1.isConst && $3.isConst) {
            $$.isConst = true;
            $$.value.vBoolean = eval_comp_expr($1.value, $3.value, $1.type->kind, $3.type->kind, "<=");
        } 

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst) {
            entry = newTemporary($$.type, &$$.value);
        }
        else {
            entry = newTemporary($$.type);
            genQuad("<=", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr ">=" expr
    {
        $$.isConst = false;
        $$.type = typeBoolean;
        Type t1 = $1.type, t2 = $3.type;
        if (!is_numeric_type(t1) || !is_numeric_type(t2))
            error(">= applied to non numeric values.");
        
        if ($1.isConst && $3.isConst) {
            $$.isConst = true;
            $$.value.vBoolean = eval_comp_expr($1.value, $3.value, $1.type->kind, $3.type->kind, ">=");
        } 

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst) {
            entry = newTemporary($$.type, &$$.value);
        }
        else {
            entry = newTemporary($$.type);
            genQuad(">=", $1.place, $3.place, entry->u.eTemporary.name);
        }
        $$.place = entry->u.eTemporary.name;
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr "&&" 
    {
        SymbolEntry *entry;
        entry = newTemporary(typeBoolean);
        $2.place = entry->u.eTemporary.name;
        int n = nextQuad();
        genQuad("==", $1.place, trueConst, "*");
        genQuad(":=", falseConst, "-", $2.place); 
        $2.i = nextQuad();
        genQuad("jump", "-", "-", "*");
        backpatch(makelist(n), nextQuad());
    } 
    expr
    {
        $$.isConst = false;
        $$.type = typeBoolean;
        if ($1.type->kind == TYPE_BOOLEAN && $4.type->kind == TYPE_BOOLEAN) {
            if ($1.isConst && $4.isConst) {
                $$.isConst = true;
                $$.value.vBoolean = $1.value.vBoolean && $4.value.vBoolean;
            }
        } else
            error("&& applied to non boolean values.");

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst) {
            entry = newTemporary($$.type, &$$.value);
            $$.place = entry->u.eTemporary.name;
        } else {
            $$.place = $2.place;
            genQuad(":=", $4.place, "-", $$.place);
        }
        backpatch(makelist($2.i), nextQuad());
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr "and" 
    {
        SymbolEntry *entry;
        entry = newTemporary(typeBoolean);
        $2.place = entry->u.eTemporary.name;
        int n = nextQuad();
        genQuad("==", $1.place, trueConst, "*");
        genQuad(":=", falseConst, "-", $2.place); 
        $2.i = nextQuad();
        genQuad("jump", "-", "-", "*");
        backpatch(makelist(n), nextQuad());
    } 
    expr
    {
        $$.isConst = false;
        $$.type = typeBoolean;
        if ($1.type->kind == TYPE_BOOLEAN && $4.type->kind == TYPE_BOOLEAN) {
            if ($1.isConst && $4.isConst) {
                $$.isConst = true;
                $$.value.vBoolean = $1.value.vBoolean && $4.value.vBoolean;
            }
        } else
            error("\"and\" applied to non boolean values.");

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst) {
            entry = newTemporary($$.type, &$$.value);
            $$.place = entry->u.eTemporary.name;
        } else {
            $$.place = $2.place;
            genQuad(":=", $4.place, "-", $$.place);
        }
        backpatch(makelist($2.i), nextQuad());
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr "||" 
    {
        SymbolEntry *entry;
        entry = newTemporary(typeBoolean);
        $2.place = entry->u.eTemporary.name;
        int n = nextQuad();
        genQuad("==", $1.place, falseConst, "*");
        genQuad(":=", trueConst, "-", $2.place); 
        $2.i = nextQuad();
        genQuad("jump", "-", "-", "*");
        backpatch(makelist(n), nextQuad());
    } 
    expr
    {
        $$.isConst = false;
        $$.type = typeBoolean;
        if ($1.type->kind == TYPE_BOOLEAN && $4.type->kind == TYPE_BOOLEAN) {
            if ($1.isConst && $4.isConst) {
                $$.isConst = true;
                $$.value.vBoolean = $1.value.vBoolean && $4.value.vBoolean;
            }
        } else
            error("|| applied to non boolean values.");

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst) {
            entry = newTemporary($$.type, &$$.value);
            $$.place = entry->u.eTemporary.name;
        } else {
            $$.place = $2.place;
            genQuad(":=", $4.place, "-", $$.place);
        }
        backpatch(makelist($2.i), nextQuad());
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
    | expr "or" 
    {
        SymbolEntry *entry;
        entry = newTemporary(typeBoolean);
        $2.place = entry->u.eTemporary.name;
        int n = nextQuad();
        genQuad("==", $1.place, falseConst, "*");
        genQuad(":=", trueConst, "-", $2.place); 
        $2.i = nextQuad();
        genQuad("jump", "-", "-", "*");
        backpatch(makelist(n), nextQuad());
    } 
    expr
    {
        $$.isConst = false;
        $$.type = typeBoolean;
        if ($1.type->kind == TYPE_BOOLEAN && $4.type->kind == TYPE_BOOLEAN) {
            if ($1.isConst && $4.isConst) {
                $$.isConst = true;
                $$.value.vBoolean = $1.value.vBoolean && $4.value.vBoolean;
            }
        } else
            error("\"or\" applied to non boolean values.");

        /* Intermediate Code */
        SymbolEntry *entry;
        if ($$.isConst) {
            entry = newTemporary($$.type, &$$.value);
            $$.place = entry->u.eTemporary.name;
        } else {
            $$.place = $2.place;
            genQuad(":=", $4.place, "-", $$.place);
        }
        backpatch(makelist($2.i), nextQuad());
        $$.TRUE  = new list<int>();
        $$.FALSE = new list<int>();
    }
;
            
call: 
    ID '(' call_args ')'
    {
        const char *name = $1;
        type_node *real_types = $3;
        SymbolEntry *entry = lookupEntry(name, LOOKUP_ALL_SCOPES, false);

        // if it is a library function get its existing entry from the symbol table
        if (!entry) {
            string fun_name($1);
            map<string, SymbolEntry *>::iterator f = lib_functions.find(fun_name);
            if (f != lib_functions.end()) {
                entry = (*f).second;
                name = entry->id;
            }
        }
        if (!entry || (entry->entryType != ENTRY_FUNCTION))
            error("Unknown function call");

        // check the arguments one by one to see if the types are consistent
        $$.type = entry->u.eFunction.resultType;
        SymbolEntry *head = entry->u.eFunction.firstArgument; 
        SymbolEntry *tail = entry->u.eFunction.lastArgument;
        type_node *rt = real_types;
        while (head != tail) {
            if (rt->type->kind == TYPE_VOID)
                error("Wrong number of parameters");
            if (head->u.eParameter.mode == PASS_BY_REFERENCE && !isArray(rt->type)) {
                if (!equal_types(rt->type, head->u.eParameter.type))
                    error("Parameter passed by reference must be of equal type");    
                SymbolEntry *r = lookupEntry(rt->place, LOOKUP_ALL_SCOPES, true);
                if (isConst(r) || (isTemp(r) && !isArrayElement(r)))
                    error("Parameter passed by reference cannot be constant");
            }   
            else { 
                if (!can_be_assigned_to(rt->type, head->u.eParameter.type))
                    error("Invalid parameter type");
            }
            head = head->u.eParameter.next;
            rt = rt->next;
        }
        // reached the end of the typical parameters, check if the last one is ok (if it exists)
        if (head == NULL) { // there were no typical parameters
            if (rt->type->kind != TYPE_VOID)
                error("Wrong number of parameters"); 
        }
        else { // there is still one last typical parameter
            if ((rt->type->kind == TYPE_VOID) || (rt->next->type->kind != TYPE_VOID))
                error("Wrong number of parameters");
            if (head->u.eParameter.mode == PASS_BY_REFERENCE && !isArray(rt->type)) {
                if (!equal_types(rt->type, head->u.eParameter.type))
                    error("Parameter passed by reference must be of equal type");    
                SymbolEntry *r = lookupEntry(rt->place, LOOKUP_ALL_SCOPES, true);
                if (isConst(r) || (isTemp(r) && !isArrayElement(r)))
                    error("Parameter passed by reference cannot be constant");
            }   
            else { 
                if (!can_be_assigned_to(rt->type, head->u.eParameter.type))
                    error("Invalid parameter type");
            }
        }

        /* Intermediate Code */
        int n = 1;
        const char *pmode;
        // push all the parameters in the stack
        head = entry->u.eFunction.firstArgument; 
        while (real_types->type->kind != TYPE_VOID) {
            if (!real_types->place)
                internal("real parameter did not have a place");
            pmode = paramMode(name, n++);
            if (equal(pmode, "error"))
                error("invalid function call");
            genQuad("par", real_types->place, pmode, "-", head);
            real_types = real_types->next;
            head = head->u.eParameter.next;
        }
        // push the result address in the stack
        Type ret_type = entry->u.eFunction.resultType;
        if (ret_type->kind != TYPE_VOID) {
            SymbolEntry * ret_temp = newTemporary(ret_type);
            genQuad("par", ret_temp->id, "RET","-");
            $$.place = ret_temp->id;
        }
        genQuad("call", "-", "-", name);
    }
;
                        
call_args: 
    /*nothing*/
    {
        $$ = (type_node *)newAlloc(sizeof(type_node));
        $$->type = typeVoid;
        $$->next = NULL;
        $$->prev = NULL;
        $$->place = NULL;
    }
    | expr c_arg_list
    {
        // create the node for the first parameter
        $$ = (type_node *)newAlloc(sizeof(type_node));
        if ($1.type->kind == TYPE_VOID) {
            error("Invalid call parameters");
            $$->place = NULL;
        } else {
            $$->place = $1.place;
        }

        // connect the new node as the head of the list        
        $$->type = $1.type;
        $$->next = $2;
        $$->prev = NULL;
        $2->prev = $$;
    }
;            

c_arg_list: 
    /*nothing*/
    {
        $$ = (type_node *)newAlloc(sizeof(type_node));
        $$->type  = typeVoid;
        $$->next  = NULL;
        $$->prev  = NULL;
        $$->place = NULL;
    }
    | ',' expr c_arg_list
    {
        if ($2.type->kind == TYPE_VOID) 
            error("Invalid call parameters");
        $$ = (type_node *)newAlloc(sizeof(type_node));
        $$->type  = $2.type;
        $$->place = $2.place;
        $$->next  = $3;
        $$->prev  = NULL;
        $3->prev  = $$;
    }
;
                       
block: 
    open_brace block_body close_brace { $$ = $2; }
;
            
open_brace:  '{' { blockScope(currentScope->negOffset);                   };
close_brace: '}' { currentOffset = currentScope->negOffset; closeScope(); };

block_body: 
    /*nothing*/ 
    {
        $$.NEXT  = new list<int>();
        $$.CONT  = new list<int>();
        $$.BREAK = new list<int>();
    }
    | local_def block_body
    {
        $$.NEXT  = $2.NEXT;
        $$.CONT  = $2.CONT;
        $$.BREAK = $2.BREAK;
    }
    | stmt 
    {
        backpatch($1.NEXT, nextQuad());
    } 
    block_body 
    {
        merge($3.CONT, $1.CONT);
        merge($3.BREAK, $1.BREAK);
        $$.NEXT  = $3.NEXT;
        $$.CONT  = $3.CONT;
        $$.BREAK = $3.BREAK;
    }
;            
            
local_def: 
    const_def
    | var_def          
;

stmt: 
    ';' 
    { 
        $$.NEXT  = new list<int>();
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
    }
    | l_value assign expr ';'
    {
        const char *op;
        if ($1.type->kind == TYPE_POINTER || $3.type->kind == TYPE_POINTER ||
            $1.type->kind == TYPE_VOID    || $3.type->kind == TYPE_VOID) 
            error("Invalid assignment types.");
        else if ($1.type->kind == TYPE_ARRAY  || $3.type->kind == TYPE_ARRAY ||
                 $1.type->kind == TYPE_IARRAY || $3.type->kind == TYPE_IARRAY) 
            error("Cannot assign to/from array.");
        else if ($1.type->kind == TYPE_INTEGER && $3.type->kind == TYPE_REAL) 
            error("Cannot assign real to integer.");
        else if (($1.type->kind == TYPE_BOOLEAN  && $3.type->kind != TYPE_BOOLEAN) ||
                 ($1.type->kind != TYPE_BOOLEAN  && $3.type->kind == TYPE_BOOLEAN)) 
            error("Invalid assignment types.");
        else if ($1.type->kind == TYPE_CHAR && $3.type->kind != TYPE_CHAR && $3.type->kind != TYPE_INTEGER) 
            error("Cannot assign non character value to a character.");
        else {
            switch ($2) {
            case '=':
                op = ":=";
                break; 
            case '+':
                if ($1.type->kind == TYPE_BOOLEAN) 
                    error("Cannot add to a boolean value");
                op = "+";
                break;
            case '-':
                if ($1.type->kind == TYPE_BOOLEAN) 
                    error("Cannot subtract from a boolean value");
                op = "-";
                break;
            case '*':
                if ($1.type->kind == TYPE_BOOLEAN) 
                    error("Cannot multiply a boolean value");
                op = "*";
                break;
            case '/':
                if ($1.type->kind == TYPE_BOOLEAN) 
                    error("Cannot divide booleans");
                op = "/";
                break;
            case '%':
                if ($1.type->kind == TYPE_BOOLEAN) 
                    error("Cannot modulo booleans");
                if ($1.type->kind == TYPE_REAL || $3.type->kind == TYPE_REAL) 
                    error("Cannot modulo reals.");
                op = "%";
                break;
            default: 
                internal("Unknown assign operator!");
            }
        }

        /* Intermediate Code */
        if (!$3.place) /* create places if they don't exist */
            $3.place = generatePlace($3.TRUE, $3.FALSE);
        if (equal(op, ":="))
            genQuad(op, $3.place, "-", $1.place);
        else
            genQuad(op, $1.place, $3.place, $1.place);
        $$.NEXT  = new list<int>();
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
    }
    | l_value "++" ';'
    {
        if (!($1.type->kind == TYPE_INTEGER || $1.type->kind == TYPE_CHAR || $1.type->kind == TYPE_REAL))
            error("Operator '++' applied to non numeric values.");
        genQuad("+", $1.place, oneConst, $1.place);
        $$.NEXT  = new list<int>();
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
    }
    | l_value "--" ';'
    {    
        if (!($1.type->kind == TYPE_INTEGER || $1.type->kind == TYPE_CHAR || $1.type->kind == TYPE_REAL))
            error("Operator '++' applied to non numeric values.");
        genQuad("-", $1.place, oneConst, $1.place);
        $$.NEXT  = new list<int>();
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
    }
    | call ';' 
    {
        $$.NEXT  = new list<int>();
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
    }
    | "while" 
    {
        $1.i1 = nextQuad();
    } 
    '(' expr ')' 
    {
        $1.i2 = nextQuad();
        genQuad("==", $4.place, falseConst, "*");
    } 
    stmt 
    {
        if ($4.type->kind != TYPE_BOOLEAN) {
            error("'while' condition is not boolean.");
        } else {
            /* Intermediate Code */
            int while_start = $1.i1;
            int while_end   = $1.i2;
            genQuad("jump", "-", "-", to_string(while_start).c_str());
            backpatch($7.NEXT, while_start);
            backpatch($7.CONT, while_start);
            $$.BREAK = new list<int>();
            $$.CONT  = new list<int>();
            $$.NEXT  = makelist(while_end);
            merge($$.NEXT, $7.BREAK);
        }
    }
    | "FOR" '(' ID ',' range ')' 
    {
        SymbolEntry * entry = lookupEntry($3,LOOKUP_ALL_SCOPES, true);
        if (entry) { 
            if (entry->entryType != ENTRY_VARIABLE || entry->u.eVariable.type->kind != TYPE_INTEGER) 
                error("Index must be an integer variable.");
        }
        
        /* Intermediate Code */
        SymbolEntry * tempVar = newTemporary(entry->u.eVariable.type);
        const char * tempName = tempVar->id;
        genQuad(":=", $5.expr1_place, "-", tempName);
        $1.c = tempName;
        $1.i = nextQuad();
        genQuad(":=", tempName, "-", $3);
        if ($5.isUp) 
            genQuad("<=", tempName, $5.expr2_place, to_string($1.i + 3).c_str());
        else 
            genQuad(">=", tempName, $5.expr2_place, to_string($1.i + 3).c_str());
        $1.i = nextQuad();
        genQuad("jump", "-", "-", "*");
    } 
    stmt
    {
        /* Intermediate Code */
        const char * tempName = $1.c;
        backpatch($8.CONT, nextQuad());
        backpatch($8.NEXT, nextQuad());
        if ($5.isUp)
            genQuad("+", tempName, $5.step_place, tempName);
        else
            genQuad("-", tempName, $5.step_place, tempName);
        genQuad("jump", "-", "-", to_string($1.i - 2).c_str());
        
        $$.NEXT = makelist($1.i);
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
        merge($$.NEXT, $8.BREAK);
    }
    | "do" 
    { 
        $1 = nextQuad();
    } 
    stmt "while" '(' expr ')' ';'
    {
        if ($6.type->kind != TYPE_BOOLEAN) {
            error("'while' condition is not boolean.");
        } else {
            /* Intermediate Code */
            int do_start = $1;
            const char * expr_place = $6.place;
            genQuad("==", expr_place, trueConst, to_string(do_start).c_str());
            backpatch($3.CONT, do_start);
            backpatch($3.NEXT, do_start);
            $$.NEXT  = $3.BREAK;
            $$.BREAK = new list<int>();
            $$.CONT  = new list<int>();
        }
    }
    | "switch" '(' expr ')' 
    {
        if ($3.type->kind != TYPE_INTEGER && $3.type->kind != TYPE_CHAR)
            error("switch variable must be of integer type");
        sw_list.push_front($3.place);
    } 
    switch_body
    {
        $$.NEXT  = $6.NEXT;
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
        sw_list.pop_front();
    }
    | "break" ';'  
    { 
        $$.BREAK = makelist(nextQuad());
        $$.NEXT  = new list<int>();
        $$.CONT  = new list<int>();
        genQuad("jump", "-", "-", "*");
    }
    | "continue" ';' 
    { 
        $$.CONT  = makelist(nextQuad());
        $$.NEXT  = new list<int>();
        $$.BREAK = new list<int>();
        genQuad("jump", "-", "-", "*");
    }
    | "return" ';' { /* do nothing */ }
    {
        if (currRetType->kind != TYPE_VOID)
            error("'return' with no value, in function returning non-void");

        // Intermediate Code
        $$.NEXT  = new list<int>();
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
        genQuad("ret", "-", "-", "-"); 
    }
    | "return" expr ';'
    {
        if ($2.type->kind == TYPE_ARRAY || $2.type->kind == TYPE_IARRAY)
            error("Invalid return type");
        if (!can_be_assigned_to($2.type, currRetType))
            error("Invalid return type");

        //Intermediate Code 
        $$.NEXT  = new list<int>();
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
        genQuad("retv", $2.place, "-", "-");
    }
    | block  { /* do nothing */ }
    | write '(' ')' ';' 
    {   
        if ((write_type == WRITELN) || (write_type == WRITESPLN)) 
            write_new_line();

        $$.NEXT  = new list<int>();
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
    }
    | write '(' format format_list ')' ';' 
    {   
        if ((write_type == WRITELN) || (write_type == WRITESPLN)) 
            write_new_line();

        $$.NEXT  = new list<int>();
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
    }
    | "if" '(' expr ')'
    {
        if ($3.type->kind != TYPE_BOOLEAN) {
            error("'if' condition of not boolean type.");
        } else {
            $1 = nextQuad();
            genQuad("==", $3.place, falseConst, "*");
        }
    }  
    stmt
    {
        int if_expr_check = $1;
        $1 = nextQuad();
        genQuad("jump", "-", "-", "*");
        backpatch(makelist(if_expr_check), nextQuad());
    }
    else_clause
    {       
        /* Intermediate Code */
        if ($3.type->kind == TYPE_BOOLEAN) {
            $$.NEXT  = makelist($1);
            $$.BREAK = $8.BREAK;
            $$.CONT  = $8.CONT;
            merge($$.NEXT,  $8.NEXT);
            merge($$.NEXT,  $6.NEXT);
            merge($$.BREAK, $6.BREAK);
            merge($$.CONT,  $6.CONT);
        }
    }
; 

else_clause:
    /* nothing */ %prec "then"
    { 
        $$.NEXT   = new list<int>();
        $$.CONT   = new list<int>();
        $$.BREAK  = new list<int>();
    }       
    | "else" stmt %prec "else"
    {
        $$.NEXT  = $2.NEXT;
        $$.CONT  = $2.CONT;
        $$.BREAK = $2.BREAK;
    }
;


format_list: 
    /* nothing */
    | ',' 
    {
        if ((write_type == WRITESP) || (write_type == WRITESPLN)) 
            write_space();
    } 
    format format_list
;        

switch_body:
    open_brace switch_list close_brace
    {
        $$.NEXT  = $2.TRUE;
        merge($$.NEXT, $2.FALSE);
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
    }
    | open_brace switch_list
    {
        backpatch($2.FALSE, nextQuad()); // point to the default case
        genQuad("no_op", "-", "-", "-");
        genQuad("no_op", "-", "-", "-");
    }
    default_case close_brace
    {
        $$.BREAK = new list<int>();
        $$.CONT  = new list<int>();
        $$.NEXT  = $2.TRUE;
        merge($$.NEXT, $4.NEXT);
        merge($$.NEXT, $4.CONT);
        merge($$.NEXT, $4.BREAK);
    }
;

switch_list: 
    /* nothing */
    {
        $$.TRUE  = new list<int>();    // will point to the end of the switch statement
        $$.FALSE = new list<int>();    // will point to the next switch case
        $$.i     = 1;                  // list was empty
    }
    | switch_case
    {
        $1.i = nextQuad();
    }
    switch_list
    {
        $$.FALSE = new list<int>();    // this should be empty at start, but it isn't :O
        $$.TRUE  = new list<int>();    // same ^^
        $$.i     = 0;                  // next list was not empty
        $$.TRUE  = $1.TRUE;            // will point to the end of the switch statement
        merge($$.TRUE, $3.TRUE);

        if (!$3.i) {                   // check if it is not the last switch case (list not empty)
            backpatch($1.FALSE, $1.i); // there was another switch case, point to it
        }
        else
            $$.FALSE = $1.FALSE;       // it was the last switch, will point to the default or the end
        merge($$.FALSE, $3.FALSE);     // merge with the falses of the remaining list
    }
;

switch_case:
    case_list clause
    {
        $$.FALSE = $1.FALSE;           // will point to the next switch_case
        $$.TRUE  = $2.BREAK;           // will point to the end of the switch statement
        merge($$.FALSE, $2.NEXT);      // NEXT and CONTINUE clause lists must point to the next switch_case
        merge($$.FALSE, $2.CONT);
    }
;

case_list:
    "case" const_expr 
    {
        $1.i = nextQuad();
        genQuad("==", sw_list.front(), $2.place, "*");
    }
    ':' cases 
    {
        $$.TRUE  = new list<int>();
        $$.FALSE = makelist(nextQuad());
        genQuad("jump", "-", "-", "*");

        merge($5.FALSE, makelist($1.i));
        backpatch($5.FALSE, nextQuad());
    }
;
            
cases: 
    /* nothing */
    {
        $$.FALSE = new list<int>();
    }
    | "case" const_expr 
    {
        $1.i = nextQuad();
        genQuad("==", sw_list.front(), $2.place, "*");
    }
    ':' cases
    {
        $$.FALSE = makelist($1.i);
        merge($$.FALSE, $5.FALSE);
    }
;
 
default_case: 
    "default" ':' clause 
    {
        $$.NEXT  = $3.NEXT;
        $$.CONT  = $3.CONT;
        $$.BREAK = $3.BREAK;
        genQuad("no_op", "-", "-", "-");
        genQuad("no_op", "-", "-", "-");
    }
;

clause: 
    stmt_list "break" ';'
    {
        if ($1.BREAK->size()) // free breaks are not allowed in a switch statement
            error("Invalid break position");

        $$.NEXT  = new list<int>();
        $$.CONT  = new list<int>();
        $$.BREAK = makelist(nextQuad());
        merge($$.BREAK, $1.NEXT);
        merge($$.BREAK, $1.CONT);
        genQuad("jump", "-", "-", "*");
    }
    | stmt_list "NEXT" ';'
    {
        if ($1.BREAK->size()) // free breaks are not allowed in a switch statement
            error("Invalid break position");
        
        $$.NEXT  = $1.NEXT;
        $$.CONT  = $1.CONT;
        $$.BREAK = new list<int>();
        char *c = (char *) newAlloc(20);
        sprintf(c, "%d", nextQuad() + 3);
        genQuad("jump", "-", "-", c);
    }
;

stmt_list: 
    /* nothing */ 
    {
        $$.NEXT  = new list<int>();
        $$.CONT  = new list<int>();
        $$.BREAK = new list<int>();
        $$.i     = nextQuad();
    }
    | stmt_list stmt
    {
        $$.CONT  = $2.CONT;
        $$.NEXT  = $2.NEXT;
        $$.BREAK = $2.BREAK;
        merge($$.BREAK, $1.BREAK);
        backpatch($1.CONT, $1.i);
        backpatch($1.NEXT, $1.i);
        $$.i = nextQuad();      
    }
;

assign: 
    '='
     {
        $$ = '=';
     }
     | "+="
     {
        $$ = '+';
     }
     | "-="
     {
        $$ = '-';
     }
     | "*="
     {
        $$ = '*';
     }
     | "/="
     {
        $$ = '/';
     }
     | "%="
     {
        $$ = '%';
     }
;
            
range: 
    expr "TO" expr
    {
        if (($1.type->kind != TYPE_INTEGER && $1.type->kind != TYPE_CHAR) || 
            ($3.type->kind != TYPE_INTEGER && $3.type->kind != TYPE_CHAR))
             error("Range limits must be of integer type.");

        /* Intermediate Code */
        $$.expr1_place = $1.place;
        $$.expr2_place = $3.place;
        $$.step_place  = oneConst;
        $$.isUp = true;
    }
    | expr "DOWNTO" expr
    {
        if (($1.type->kind != TYPE_INTEGER && $1.type->kind != TYPE_CHAR) || 
            ($3.type->kind != TYPE_INTEGER && $3.type->kind != TYPE_CHAR))
             error("Range limits must be of integer type.");

        /* Intermediate Code */
        $$.expr1_place = $1.place;
        $$.expr2_place = $3.place;
        $$.step_place  = oneConst;
        $$.isUp = false;
    }
    | expr "TO" expr "STEP" expr
    {
        // check argument types
        if (($1.type->kind != TYPE_INTEGER && $1.type->kind != TYPE_CHAR) || 
            ($3.type->kind != TYPE_INTEGER && $3.type->kind != TYPE_CHAR) ||
            ($5.type->kind != TYPE_INTEGER && $5.type->kind != TYPE_CHAR))
             error("Range limits must be of integer type.");

        // check if step is positive
        if ($5.isConst) {
            if (($5.type->kind == TYPE_CHAR    && $5.value.vChar <= 0) ||
                ($5.type->kind == TYPE_INTEGER && $5.value.vInteger <= 0))
                error("Step must be positive.");
        }

        /* Intermediate Code */
        $$.expr1_place = $1.place;
        $$.expr2_place = $3.place;
        $$.step_place  = $5.place;
        $$.isUp = true;
    }
    | expr "DOWNTO" expr "STEP" expr
    {
        // check argument types
        if (($1.type->kind != TYPE_INTEGER && $1.type->kind != TYPE_CHAR) || 
            ($3.type->kind != TYPE_INTEGER && $3.type->kind != TYPE_CHAR) ||
            ($5.type->kind != TYPE_INTEGER && $5.type->kind != TYPE_CHAR))
             error("Range limits must be of integer type.");
        // check if step is positive
        if ($5.isConst) {
            if (($5.type->kind == TYPE_CHAR    && $5.value.vChar <= 0) ||
                ($5.type->kind == TYPE_INTEGER && $5.value.vInteger <= 0))
                error("Step must be positive.");
        }

        /* Intermediate Code */
        $$.expr1_place = $1.place;
        $$.expr2_place = $3.place;
        $$.step_place  = $5.place;
        $$.isUp = false;
    }
;

write: 
    "WRITE"         { write_type = WRITE;     }
    | "WRITELN"     { write_type = WRITELN;   }
    | "WRITESP"     { write_type = WRITESP;   }
    | "WRITESPLN"   { write_type = WRITESPLN; }
;
            
format: 
    expr 
    {
        call_write($1.type, $1.place, printLength, floatDecimals);
    }
    | "FORM" '(' expr ',' expr ')'
    {
        if ($5.type->kind != TYPE_INTEGER)
            error("print field type must be an integer");
        call_write($3.type, $3.place, $5.place, floatDecimals);
    }
    | "FORM" '(' expr ',' expr ',' expr ')'
    {
        if ($3.type->kind != TYPE_REAL)
            error("printable expression must be real");
        if ($5.type->kind != TYPE_INTEGER)
            error("print field type must be an integer");
        if ($7.type->kind != TYPE_INTEGER)
            error("number of decimals must be an integer");
        call_write($3.type, $3.place, $5.place, $7.place);
    }
;


%%

void yyerror (const char * msg)
{
    fprintf(stderr, "Line: %d PaZcal: %s  \n", lineno, msg);
    exit(1);
}

int main (int argc,char **argv)
{
    parse_arguments(argc, argv);

    initSymbolTable(10099); 
    initiate();
    int ret = yyparse();
    remove_no_ops();

    if (ENABLE_OPT)   optimize();
    if (PRINT_IMM)    printQuads();
    if (PRINT_DBG)    printf("----------------------------------------------------------\n");
    if (ENABLE_FINAL) generateFinalCode(quads);

    return ret;
}

