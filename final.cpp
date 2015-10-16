
/******************************************************************************
 *  C++ code file : final.cpp
 *  Project       : Pazcal Compiler
 *  Written by    : Lolos Konstantinos, Podimata Charikleia
 *                  (lolos.kostis@gmail.com, charapod@gmail.com)
 *  Date          : 2014-2015
 *  Description   : Everything related to final code generation
 ******************************************************************************/


#include <stdlib.h>
#include <string.h>
#include "general.h"
#include "symbol.h"
#include "error.h"
#include "stdio.h"
#include "final.h"
#include <list>
#include <string>

using namespace std;

/* functions */    
void            translateQuad(quad);
void            op_plus(quad);
void            op_minus(quad);
void            op_mult(quad);
void            op_div(quad);
void            op_mod(quad);
void            op_equals(quad);
void            op_not(quad); //unary operator
void            op_not_equals(quad);
void            op_gt(quad);
void            op_lt(quad);
void            op_ge(quad);
void            op_le(quad);
void            q_assign(quad);
void            q_unit(quad);
void            q_endu(quad);
void            q_jump(quad);
void            q_par(quad);
void            q_call(quad);
void            q_ret(quad);
void            q_retv(quad);
void            q_array(quad);
void            reg_to_var(const char *reg, SymbolEntry *var);
void            reg_to_var(const char *reg, KIND kind, int offset);
void            var_to_reg(SymbolEntry *var, const char *reg);
int             offset(SymbolEntry *var);
const char *    ptr_type(int kind); 
const char *    mov_op(int kind);
void            preamble(vector<quad> quads);
void            allocate_constants(vector<quad> quads);
void            allocate_globals(vector<quad> quads);
void            init_globals(vector<quad> quads);
void            bool_to_reg(SymbolEntry *var, const char * reg);
void            reg_to_bool(const char * reg, SymbolEntry *var);
void            fstack_to_var(SymbolEntry *var);
void            fstack_to_var(KIND kind, int offset, bool by_ref);
void            push_real_to_fstack(SymbolEntry *real);
void            push_int_to_fstack(SymbolEntry *var);
void            push_char_to_fstack(SymbolEntry *var);
void            alloc_data(SymbolEntry *var);
void            alloc_global(SymbolEntry *var);
void            push_from_fstack();
const char *    quad_to_s(quad q);
void            translate_comparison(quad q);

/* global variables */
Type curRetType;
int real_counter = 0;
int str_counter  = 0;
int byte_counter = 0;
int int_counter  = 0;

/* Generates the final code from the given list of quads */
void generateFinalCode(vector<quad> quads)
{
    unsigned int i;
    preamble(quads);
    bool inFunction = false;
    for (i = 0; i < quads.size(); ++i) {
        if (equal(quads[i].op, "unit"))
            inFunction = true;
        if (inFunction) {
            fprintf(asm_out, "__L%d: # %s\n", i, quad_to_s(quads[i]));
            translateQuad(quads[i]);
        }
        if (equal(quads[i].op, "endu"))
            inFunction = false;
    }
}

/* String representation for quads for final code comments */
const char *quad_to_s(quad q)
{
    int len = strlen(q.op) + strlen(q.x) + strlen(q.y) + strlen(q.z) + 10;
    char *s = (char *) newAlloc(len);
    sprintf(s, "%s, %s, %s, %s", q.op, q.x, q.y, q.z);
    return s;
}

/* Creates the first part of the assembly code.
   Allocates all needed memory and calls the main function */
void preamble(vector<quad> quads)
{
    fprintf(asm_out, ".section .data\n");
    allocate_constants(quads);
    allocate_globals(quads);
    fprintf(asm_out, ".section .text\n");
    fprintf(asm_out, ".globl   _start\n");
    fprintf(asm_out, "\n");
    fprintf(asm_out, "_start: \n");
    fprintf(asm_out, "\tmovl\t%%esp, %%ebp\n");
    init_globals(quads);
    fprintf(asm_out, "\n");
    fprintf(asm_out, "\tpushl\t%%ebp\n");
    fprintf(asm_out, "\tcall\t__main\n");
    fprintf(asm_out, "\n");
    fprintf(asm_out, "\taddl\t$4, %%esp\n");
    fprintf(asm_out, "\tmovl\t$1, %%eax\n");
    fprintf(asm_out, "\tmovl\t$0, %%ebx\n");
    fprintf(asm_out, "\tint\t$0x80\n");
    fprintf(asm_out, "\n");
}

/* Allocates memory for all constants in the code */
void allocate_constants(vector<quad> quads)
{
    for (auto q : quads) {
        alloc_data(q.xe);
        alloc_data(q.ye);
        alloc_data(q.ze);
    }
}

/* Allocates memory for the given valiable if it is a constnant 
   and has not yet been allocated.                              */
void alloc_data(SymbolEntry *var) 
{
    if (!var || var->final_code_name)
        return;

    if (isConst(var) && is_real_type(var)) {
        char * name = (char *) newAlloc(20);
        sprintf(name, "__fl%d", real_counter++);
        fprintf(asm_out, "%s: .tfloat %Lf\n", name, get_real_val(var)); 
        var->final_code_name = name; 
    } 
    else if (isConst(var) && isArray(var)) { //string literal
        char * name = (char *) newAlloc(20);
        sprintf(name, "__str%d", str_counter++);
        fprintf(asm_out, "%s: .string \"%s\"\n", name, get_str_val(var)); 
        var->final_code_name = name;        
    }
}

/* Allocates memory for all global variables */
void allocate_globals(vector<quad> quads)
{
    for (auto q : quads) {
        alloc_global(q.xe);
        alloc_global(q.ye);
        alloc_global(q.ze);
    }
}

/* Allocates memory for the given variable if it is a global
   and has not yet been allocated.                              */
void alloc_global(SymbolEntry *var) 
{
    if (!var || var->final_code_name || !var->isGlobal)
        return;
    
    int c;
    char *name;
    RepReal r;
    KIND k = var_kind(var);
    switch (k) {
        case TYPE_BOOLEAN:
            c = 0;
            if (isConst(var))
                c = bool_val(var) ? 1 : 0;

            name = (char *) newAlloc(20);
            sprintf(name, "__byte%d", byte_counter++);
            var->final_code_name = name;        
            fprintf(asm_out, "%s: .byte %d\n", name, c); 
            break;
        case TYPE_CHAR:
            c = 0;
            if (isConst(var))
                c = char_val(var);

            name = (char *) newAlloc(20);
            sprintf(name, "__byte%d", byte_counter++);
            var->final_code_name = name;        
            fprintf(asm_out, "%s: .byte %d\n", name, c); 
            break;
        case TYPE_INTEGER:
            c = 0;
            if (isConst(var))
                c = int_val(var);

            name = (char *) newAlloc(20);
            sprintf(name, "__int%d", int_counter++);
            var->final_code_name = name;        
            fprintf(asm_out, "%s: .long %d\n", name, c); 
            break;
        case TYPE_ARRAY:
            name = (char *) newAlloc(20);
            sprintf(name, "__byte%d", byte_counter++);
            var->final_code_name = name;        
            fprintf(asm_out, "%s:\n", name); 
            c = sizeOfType(var_type(var));
            fprintf(asm_out, ".rept\t%d\n", c);
            fprintf(asm_out, ".byte\t0\n");
            fprintf(asm_out, ".endr\n");
            break;
        case TYPE_IARRAY:
            internal("alloc global called with IARRAY type");
            break;
        case TYPE_REAL:
            r = 0;
            if (isConst(var))
                r = get_real_val(var);

            name = (char *) newAlloc(20);
            sprintf(name, "__fl%d", real_counter++);
            var->final_code_name = name;        
            fprintf(asm_out, "%s: .tfloat %Lf\n", name, r); 
            break;
        default:
            internal("alloc_global called with incompatible var type");
    }
}

/* Generates the code that does the global variable initialization.
   This code goes in the preamble and gets executed before calling the main function. */
void init_globals(vector<quad> quads)
{
    unsigned int i;
    bool inFunction = false;
    for (i = 0; i < quads.size(); ++i) {
        if (equal(quads[i].op, "unit"))
            inFunction = true;
        if (!inFunction) {
            fprintf(asm_out, "__L%d: # %s\n", i, quad_to_s(quads[i]));
            translateQuad(quads[i]);
        }
        if (equal(quads[i].op, "endu"))
            inFunction = false;
    }
}

/* Translates a quad to final code */
void translateQuad(quad q)
{
    if      (equal(q.op, "+"))     op_plus(q);
    else if (equal(q.op, "-"))     op_minus(q);
    else if (equal(q.op, "*"))     op_mult(q);
    else if (equal(q.op, "/"))     op_div(q);
    else if (equal(q.op, "%"))     op_mod(q);
    else if (equal(q.op, "=="))    op_equals(q);
    else if (equal(q.op, "not"))   op_not(q);
    else if (equal(q.op, "!="))    op_not_equals(q);
    else if (equal(q.op, ">"))     op_gt(q);
    else if (equal(q.op, "<"))     op_lt(q);
    else if (equal(q.op, ">="))    op_ge(q);
    else if (equal(q.op, "<="))    op_le(q);
    else if (equal(q.op, ":="))    q_assign(q);
    else if (equal(q.op, "unit"))  q_unit(q);
    else if (equal(q.op, "endu"))  q_endu(q);
    else if (equal(q.op, "jump"))  q_jump(q);
    else if (equal(q.op, "par"))   q_par(q);
    else if (equal(q.op, "call"))  q_call(q);
    else if (equal(q.op, "ret"))   q_ret(q);
    else if (equal(q.op, "retv"))  q_retv(q);
    else if (equal(q.op, "array")) q_array(q);
    else if (equal(q.op, "no_op")); // do nothing
    else internal("Unknown quad op: %s", q.op);
}

/* Addition quad */
void op_plus(quad q)
{
    KIND x_type = var_kind(q.xe);
    KIND y_type = var_kind(q.ye);
    if (x_type != TYPE_REAL && y_type != TYPE_REAL) {
        var_to_reg(q.ye, "%ebx");
        var_to_reg(q.xe, "%eax");
        fprintf(asm_out, "\taddl\t%%ebx, %%eax\n");
        reg_to_var("%eax", q.ze);
    } 
    else if (x_type == TYPE_INTEGER && y_type == TYPE_REAL)    {
        push_int_to_fstack(q.xe);
        push_real_to_fstack(q.ye);
        fprintf(asm_out, "\tfaddp\t%%st(1), %%st(0)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_CHAR && y_type == TYPE_REAL)    {
        push_char_to_fstack(q.xe);
        push_real_to_fstack(q.ye);
        fprintf(asm_out, "\tfaddp\t%%st(1), %%st(0)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_REAL && y_type == TYPE_INTEGER) {
        push_real_to_fstack(q.xe);
        push_int_to_fstack(q.ye);
        fprintf(asm_out, "\tfaddp\t%%st(1), %%st(0)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_REAL && y_type == TYPE_CHAR)    {
        push_real_to_fstack(q.xe);
        push_char_to_fstack(q.ye);
        fprintf(asm_out, "\tfaddp\t%%st(1), %%st(0)\n");
        fstack_to_var(q.ze);
    }   
    else if (x_type == TYPE_REAL && y_type == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfaddp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }
}

/* Minus quad */
void op_minus(quad q)
{
    KIND x_type = var_kind(q.xe);
    KIND y_type = var_kind(q.ye);
    if (x_type != TYPE_REAL && y_type != TYPE_REAL) {
        var_to_reg(q.xe, "%eax");
        var_to_reg(q.ye, "%ebx");
        fprintf(asm_out, "\tsubl\t%%ebx, %%eax\n");
        reg_to_var("%eax", q.ze);
    } 
    else if (x_type == TYPE_INTEGER && y_type == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_int_to_fstack(q.xe);
        fprintf(asm_out, "\tfsubp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_CHAR && y_type == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_char_to_fstack(q.xe);
        fprintf(asm_out, "\tfsubp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_REAL && y_type == TYPE_INTEGER) {
        push_int_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfsubp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_REAL && y_type == TYPE_CHAR)    {
        push_char_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfsubp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }   
    else if (x_type == TYPE_REAL && y_type == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfsubp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }

}

/* Multiplication quad */
void op_mult(quad q)
{
    KIND x_type = var_kind(q.xe);
    KIND y_type = var_kind(q.ye);
    if (x_type != TYPE_REAL && y_type != TYPE_REAL) {
        var_to_reg(q.xe, "%eax");
        var_to_reg(q.ye, "%ebx");
        fprintf(asm_out, "\tmul\t\t%%ebx\n");
        reg_to_var("%eax", q.ze);
    } 
    else if (x_type == TYPE_INTEGER && y_type == TYPE_REAL)    {
        push_int_to_fstack(q.xe);
        push_real_to_fstack(q.ye);
        fprintf(asm_out, "\tfmulp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_CHAR && y_type == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_char_to_fstack(q.xe);
        fprintf(asm_out, "\tfmulp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_REAL && y_type == TYPE_INTEGER) {
        push_int_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfmulp\t%%st, %%st(1)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_REAL && y_type == TYPE_CHAR)    {
        push_char_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfmulp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }   
    else if (x_type == TYPE_REAL && y_type == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfmulp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }

}

/* Division quad */
void op_div(quad q)
{
    KIND x_type = var_kind(q.xe);
    KIND y_type = var_kind(q.ye);
    if (x_type != TYPE_REAL && y_type != TYPE_REAL) {
        fprintf(asm_out, "\tmovl\t$0, %%edx\n");
        var_to_reg(q.xe, "%eax");
        var_to_reg(q.ye, "%ebx");
        fprintf(asm_out, "\tidiv\t%%ebx\n");
        reg_to_var("%eax", q.ze);
    } 
    else if (x_type == TYPE_INTEGER && y_type == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_int_to_fstack(q.xe);
        fprintf(asm_out, "\tfdivp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_CHAR    && y_type == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_char_to_fstack(q.xe);
        fprintf(asm_out, "\tfdivp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_REAL    && y_type == TYPE_INTEGER) {
        push_int_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfdivp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_REAL    && y_type == TYPE_CHAR)    {
        push_char_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfdivp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }   
    else if (x_type == TYPE_REAL    && y_type == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfdivp\t%%st(1)\n");
        fstack_to_var(q.ze);
    }
}

/* Modulo quad */
void op_mod(quad q)
{
    KIND x_type = var_kind(q.xe);
    KIND y_type = var_kind(q.ye);
    if (x_type != TYPE_REAL && y_type != TYPE_REAL) {
        fprintf(asm_out, "\tmovl\t$0, %%edx\n");
        var_to_reg(q.xe, "%eax");
        var_to_reg(q.ye, "%ebx");
        fprintf(asm_out, "\tidiv\t%%ebx\n");
        reg_to_var("%edx", q.ze);  // in x86, dx:ax has the result for division
    }
    else 
        internal("Mod applied to non integer values."); 
}

/* Final code for unary operator "not" or "!".
   Operand q.xe is always a boolean.             */
void op_not(quad q)
{
    bool_to_reg(q.xe, "%al");
    fprintf(asm_out, "\tcmpb\t$0, %%al\n");
    fprintf(asm_out, "\tsete\t%%al\n");
    reg_to_var("%al", q.ze);

    /*if (isGlobal(q.xe)) {
        bool_to_reg(q.xe, "%al");
        fprintf(asm_out, "\tcmpb\t$0, %%al\n");
        fprintf(asm_out, "\tsete\t%%al\n");
    }
    else {
        fprintf(asm_out, "\tcmpb\t$0, %d(%%ebp)\n", offset(q.xe));
        fprintf(asm_out, "\tsete\t%%al\n");
    }
    reg_to_var("%al", q.ze);*/
}

/* Equals comparison quad */
void op_equals(quad q)
{
    translate_comparison(q);
    if (q.ze) { // it is assigning the value to a temp variable
        fprintf(asm_out, "\tcmove\t%%edx, %%ecx\n");
        reg_to_var("%cl", q.ze);
    }
    else { // it jumps to the quad number q.z
        fprintf(asm_out, "\tje\t__L%s\n", q.z);  
    }
}

/* Not equals comparison quad */
void op_not_equals(quad q)
{
    translate_comparison(q);
    if (q.ze) { // it is assigning the value to a temp variable
        fprintf(asm_out, "\tcmovne\t%%edx, %%ecx\n");
        reg_to_var("%cl", q.ze);
    }
    else { // it jumps to the quad number q.z
        fprintf(asm_out, "\tjne\t__L%s\n", q.z);  
    }
}

/* Greater than quad */
void op_gt(quad q)
{
    translate_comparison(q);
    if (is_real_type(q.xe) || is_real_type(q.ye)) {
        if (q.ze) { // it is assigning the value to a temp variable
            fprintf(asm_out, "\tcmovnbe\t%%edx, %%ecx\n");
            reg_to_var("%cl", q.ze);
        }
        else { // it jumps to the quad number q.z
            fprintf(asm_out, "\tjg\t__L%s\n", q.z);  
        }
    }
    else {
        if (q.ze) { // it is assigning the value to a temp variable
            fprintf(asm_out, "\tcmovg\t%%edx, %%ecx\n");
            reg_to_var("%cl", q.ze);
        }
        else { // it jumps to the quad number q.z
            fprintf(asm_out, "\tjg\t__L%s\n", q.z);  
        }
    }
}

/* Less than quad */
void op_lt(quad q)
{
    translate_comparison(q);
    if (is_real_type(q.xe) || is_real_type(q.ye)) {
        if (q.ze) { // it is assigning the value to a temp variable
            fprintf(asm_out, "\tcmovb\t%%edx, %%ecx\n");
            reg_to_var("%cl", q.ze);
        }
        else { // it jumps to the quad number q.z
            fprintf(asm_out, "\tjl\t__L%s\n", q.z);  
        }
    }
    else {
        if (q.ze) { // it is assigning the value to a temp variable
            fprintf(asm_out, "\tcmovl\t%%edx, %%ecx\n");
            reg_to_var("%cl", q.ze);
        }
        else { // it jumps to the quad number q.z
            fprintf(asm_out, "\tjl\t__L%s\n", q.z);  
        }
    }
}

/* Greater or equal quad */
void op_ge(quad q)
{
    translate_comparison(q);
    if (is_real_type(q.xe) || is_real_type(q.ye)) {
        if (q.ze) { // it is assigning the value to a temp variable
            fprintf(asm_out, "\tcmovnb\t%%edx, %%ecx\n");
            reg_to_var("%cl", q.ze);
        }
        else { // it jumps to the quad number q.z
            fprintf(asm_out, "\tjge\t__L%s\n", q.z);  
        }
    }
    else {
        if (q.ze) { // it is assigning the value to a temp variable
            fprintf(asm_out, "\tcmovge\t%%edx, %%ecx\n");
            reg_to_var("%cl", q.ze);
        }
        else { // it jumps to the quad number q.z
            fprintf(asm_out, "\tjge\t__L%s\n", q.z);  
        }
    }
}

/* Less or equal quad */
void op_le(quad q) 
{
    translate_comparison(q);
    if (is_real_type(q.xe) || is_real_type(q.ye)) {
        if (q.ze) { // it is assigning the value to a temp variable
            fprintf(asm_out, "\tcmovbe\t%%edx, %%ecx\n");
            reg_to_var("%cl", q.ze);
        }
        else { // it jumps to the quad number q.z
            fprintf(asm_out, "\tjle\t__L%s\n", q.z);  
        }
    }
    else {
        if (q.ze) { // it is assigning the value to a temp variable
            fprintf(asm_out, "\tcmovle\t%%edx, %%ecx\n");
            reg_to_var("%cl", q.ze);
        }
        else { // it jumps to the quad number q.z
            fprintf(asm_out, "\tjle\t__L%s\n", q.z);  
        }
    }
}

/* Assignment quad */
void q_assign(quad q)
{
    if (isConst(q.ze)) 
        internal("Assigned to constant.");
    
    KIND x_type = var_kind(q.xe);
    KIND z_type = var_kind(q.ze);
    
    if (x_type == TYPE_BOOLEAN && z_type == TYPE_BOOLEAN) {
        bool_to_reg(q.xe, "%al");
        reg_to_bool("%al", q.ze);
    }
    else if (x_type != TYPE_REAL && z_type != TYPE_REAL) {
        var_to_reg(q.xe, "%eax");
        if ((z_type == TYPE_CHAR) || (z_type == TYPE_BOOLEAN))
            reg_to_var("%al", q.ze);
        else 
            reg_to_var("%eax", q.ze);
    } 
    else if (x_type == TYPE_INTEGER && z_type == TYPE_REAL) {
        push_int_to_fstack(q.xe); 
        fstack_to_var(q.ze);
    }
    else if (x_type == TYPE_CHAR && z_type == TYPE_REAL) {
        push_char_to_fstack(q.xe); 
        fstack_to_var(q.ze);
    }
    else if ((x_type == TYPE_REAL) && 
             (z_type == TYPE_INTEGER || z_type == TYPE_CHAR || z_type == TYPE_REAL))   {
        push_real_to_fstack(q.xe);
        fstack_to_var(q.ze);
    }
}

/* Unit quad (start of a function) */
void q_unit(quad q)
{
    unsigned int negOffset;

    if (q.xe == NULL) {
        fprintf(asm_out, "__main:\n");
        fprintf(asm_out, "\tpushl\t%%ebp\n");
        fprintf(asm_out, "\tmovl\t%%esp, %%ebp\n");
        negOffset = program_offset; // this is the main function
    } else {                      
        curRetType = q.xe->u.eFunction.resultType; 
        negOffset = q.xe->u.eFunction.negOffset; // any other function 
        fprintf(asm_out, "%s:\n", q.x);
        fprintf(asm_out, "\tpushl\t%%ebp\n");
        fprintf(asm_out, "\tmovl\t%%esp, %%ebp\n");
    }
    fprintf(asm_out, "\tsubl\t$%d, %%esp\n", -negOffset);
}

/* Endu quad (end of a function) */
void q_endu(quad q)
{
    fprintf(asm_out, "\tmovl\t%%ebp, %%esp\n");
    fprintf(asm_out, "\tpopl\t%%ebp\n");
    fprintf(asm_out, "\tret\n\n");
    curRetType = NULL;
}

/* Jump quads */
void q_jump(quad q)
{
    fprintf(asm_out, "\tjmp __L%s\n", q.z);
}

/* Parameter quad (parameter passing for function calls) */
void q_par(quad q)
{
    int off = offset(q.xe);
    KIND real_kind    = var_kind(q.xe);
    KIND typical_kind = var_kind(q.ze);
    
    if (equal(q.y, "RET")) {
        fprintf(asm_out, "\tmovl\t%%ebp, %%eax\n");
        fprintf(asm_out, "\taddl\t$%d, %%eax\n", off);
        fprintf(asm_out, "\tpushl\t%%eax\n");
    }
    else if (equal(q.y, "V")) {  // pass by value
        if (real_kind == TYPE_BOOLEAN && typical_kind == TYPE_BOOLEAN) {
            var_to_reg(q.xe, "%eax");
            fprintf(asm_out, "\tpushl\t%%eax\n");
        }
        else if (real_kind == TYPE_CHAR) {
            if (typical_kind == TYPE_CHAR) {
                var_to_reg(q.xe, "%eax");
                fprintf(asm_out, "\tpushl\t%%eax\n");
            }
            else if (typical_kind == TYPE_INTEGER) {
                var_to_reg(q.xe, "%eax");
                fprintf(asm_out, "\tpushl\t%%eax\n");
            }
            else if (typical_kind == TYPE_REAL) {
                push_char_to_fstack(q.xe);
                push_from_fstack();
            }
            else
                internal("char type passed to illegal typical parameter type");
        } 
        else if (real_kind == TYPE_INTEGER) {
            if (typical_kind == TYPE_CHAR) {
                var_to_reg(q.xe, "%eax");
                fprintf(asm_out, "\tpushb\t%%al\n");
            }
            else if (typical_kind == TYPE_INTEGER) {
                var_to_reg(q.xe, "%eax");
                fprintf(asm_out, "\tpushl\t%%eax\n");
            }
            else if (typical_kind == TYPE_REAL) {
                push_int_to_fstack(q.xe);
                push_from_fstack();
            }
            else
                internal("int type passed to illegal typical parameter type");
        }
        else if (real_kind == TYPE_REAL) {
            if (typical_kind != TYPE_REAL)
                internal("real type can only be passed to real typical parameters");
        
            push_real_to_fstack(q.xe);
            push_from_fstack();
        }
        else
            internal("parameter passed by value with incompatible type");
    }
    else if (equal(q.y, "R")) {  // pass by reference
        if (isConst(q.xe) || isGlobal(q.xe)) {
            fprintf(asm_out, "\tmovl\t$%s, %%eax\n", q.xe->final_code_name);
            fprintf(asm_out, "\tpushl\t%%eax\n");
        }        
        else {
            fprintf(asm_out, "\tmovl\t%%ebp, %%eax\n");
            fprintf(asm_out, "\taddl\t$%d, %%eax\n", off);  // eax holds the address of the variable
            if (is_by_ref(q.xe))
                fprintf(asm_out, "\tpushl\t(%%eax)\n");     // if was already passed by reference then dereference it
            else                                            // so that the actual address is passed instead of a pointer to its pointer
                fprintf(asm_out, "\tpushl\t%%eax\n");
        }
    }
}

/* Call quad (calls a function) */
void q_call(quad q)
{
    int extra_space = 4; // space for access link and possibly return value address
    if (q.ze->u.eFunction.resultType->kind != TYPE_VOID) // if z has a return type
        extra_space += 4;                                // make room for it in the stack
    fprintf(asm_out, "\tpushl\t%%ebp\n");
    fprintf(asm_out, "\tcall\t%s\n", q.z);
    fprintf(asm_out, "\taddl\t$%d, %%esp\n", extra_space + calc_param_size(q.ze));
}

/* Return quad (returns from a procedure) */
void q_ret(quad q)
{
    fprintf(asm_out, "\tmovl\t%%ebp, %%esp\n");
    fprintf(asm_out, "\tpopl\t%%ebp\n");
    fprintf(asm_out, "\tret\n");
}

/* Returns value quad (returns from a function) */
void q_retv(quad q)
{
    if (!curRetType) 
        internal("Current return type of function is null");
    
    KIND ret_kind       = var_kind(q.xe);
    KIND func_kind      = curRetType->kind;
    int ret_addr_offset = 12;

    if (ret_kind == TYPE_BOOLEAN && func_kind == TYPE_BOOLEAN) {
        bool_to_reg(q.xe, "%al");
        reg_to_var("%al", func_kind, ret_addr_offset);
    }
    else if (ret_kind != TYPE_REAL && func_kind != TYPE_REAL) {
        var_to_reg(q.xe, "%eax");
        reg_to_var("%eax", func_kind, ret_addr_offset);
    } 
    else if (ret_kind == TYPE_INTEGER && func_kind == TYPE_REAL) {
        push_int_to_fstack(q.xe); 
        fstack_to_var(func_kind, ret_addr_offset, true);
    }
    else if (ret_kind == TYPE_CHAR && func_kind == TYPE_REAL) {
        push_char_to_fstack(q.xe); 
        fstack_to_var(func_kind, ret_addr_offset, true);
    }
    else if ((ret_kind == TYPE_REAL) && 
             (func_kind == TYPE_INTEGER || func_kind == TYPE_CHAR || func_kind == TYPE_REAL))   {
        push_real_to_fstack(q.xe);
        fstack_to_var(func_kind, ret_addr_offset, true);
    }
    else 
        internal("retv called with unknown parameter type");

    fprintf(asm_out, "\tmovl\t%%ebp, %%esp\n");
    fprintf(asm_out, "\tpopl\t%%ebp\n");
    fprintf(asm_out, "\tret\n");
}

/* Array quad */
void q_array(quad q)
{
    Type refType;
    EntryType eType = entry_type(q.xe);
    switch (eType) {
        case ENTRY_VARIABLE:
            refType = q.xe->u.eVariable.type->refType;
            break;
        case ENTRY_PARAMETER:
            refType = q.xe->u.eParameter.type->refType;
            break;
        case ENTRY_TEMPORARY:
            refType = q.xe->u.eTemporary.type->refType;
            break;
        default:
            internal("q_array called with invalid entry type");
    }
    int ref_size = sizeOfType(refType);
    int x_offset = offset(q.xe);
    int z_offset = offset(q.ze);
    var_to_reg(q.ye, "%eax");                                   // eax holds the index value
    fprintf(asm_out, "\tmovl\t$%d, %%ecx\n", ref_size);         // ecx holds the ref_size
    fprintf(asm_out, "\timull\t%%ecx\n");                       // eax holds the distance from the first element
    if (isGlobal(q.xe)) {
        fprintf(asm_out, "\tmovl\t$%s, %%ecx\n", q.xe->final_code_name);
    }
    else {
        fprintf(asm_out, "\tmovl\t%%ebp, %%ecx\n");
        fprintf(asm_out, "\taddl\t$%d, %%ecx\n", x_offset);     // ecx holds the address of the array variable
    }
    if (is_by_ref(q.xe))
        fprintf(asm_out, "\taddl\t(%%ecx), %%eax\n");           // eax holds the address of the element
    else
        fprintf(asm_out, "\taddl\t%%ecx, %%eax\n");               
        
    fprintf(asm_out, "\tmovl\t%%eax, %d(%%ebp)\n", z_offset);   // store the address in the z temporary variable
}

/* Moves a variable to a register */
void var_to_reg(SymbolEntry *var, const char *reg)
{
    int off = offset(var);
    KIND kind = var_kind(var);
    const char *op = mov_op(kind);
    
    if (isGlobal(var)) {
        if (kind == TYPE_INTEGER) {
            if (isConst(var))
                fprintf(asm_out, "\tmovl\t$%d, %s\n", int_val(var), reg); //use immediate instr
            else
                fprintf(asm_out, "\tmovl\t%s, %s\n", var->final_code_name, reg);
        }
        else if (kind == TYPE_CHAR || kind == TYPE_BOOLEAN) {
            if (isConst(var)) 
                fprintf(asm_out, "\tmovl\t$%d, %s\n", char_val(var), reg); //use immediate instr
            else {
                fprintf(asm_out, "\tmovl\t$0, %s\n", reg); // zero the register
                fprintf(asm_out, "\tmovsbl\t%s, %s\n", var->final_code_name, reg);
            }
        }
    }
    else if (is_by_ref(var)) {
        fprintf(asm_out, "\tmovl\t%d(%%ebp), %%esi\n", off);
        if (kind == TYPE_INTEGER) 
            fprintf(asm_out, "\tmovl\t0(%%esi), %s\n", reg);
        else if (kind == TYPE_CHAR || kind == TYPE_BOOLEAN) 
            fprintf(asm_out, "\tmovsbl\t0(%%esi), %s\n", reg);
        else
            internal("var_to_reg called with invalid var type");
    }
    else if (kind == TYPE_INTEGER) {
        if (isConst(var))
            fprintf(asm_out, "\tmovl\t$%d, %s\n", int_val(var), reg); //use immediate instr
        else
            fprintf(asm_out, "\t%s\t%d(%%ebp), %s\n", op, off, reg);
    }
    else if (kind == TYPE_CHAR || kind == TYPE_BOOLEAN) {
        if (isConst(var)) 
            fprintf(asm_out, "\tmovl\t$%d, %s\n", char_val(var), reg); //use immediate instr
        else {
            fprintf(asm_out, "\tmovl\t$0, %s\n", reg); // zero the register
            fprintf(asm_out, "\tmovsbl\t%d(%%ebp), %s\n", off, reg);
        }
    }
    else 
        internal("var_to_reg called with invalid var type");
}

/* Moves a boolean variable to a register */
void bool_to_reg(SymbolEntry *var, const char *reg)
{
    if (var_kind(var) != TYPE_BOOLEAN) 
        internal("Reg_to_bool called with non boolean var");

    int off = offset(var);
    const char *op = mov_op(TYPE_BOOLEAN);
    
    if (isGlobal(var)) {
        if (isConst(var)) {
            int val = bool_val(var) ? 1 : 0; 
            fprintf(asm_out, "\tmovb\t$%d, %s\n", val, reg);
        }
        else
            fprintf(asm_out, "\t%s\t%s, %s\n", op, var->final_code_name, reg);
    }
    else if (is_by_ref(var)) {
        fprintf(asm_out, "\tmovl\t%d(%%ebp), %%esi\n", off);
        fprintf(asm_out, "\tmovb\t0(%%esi), %s\n", reg);
    }
    else {
        if (isConst(var)) {
            int val = bool_val(var) ? 1 : 0; 
            fprintf(asm_out, "\tmovb\t$%d, %s\n", val, reg);
        }
        else
            fprintf(asm_out, "\t%s\t%d(%%ebp), %s\n", op, off, reg);
    }
}

/* Moves a value from a register to a variable */
void reg_to_var(const char *reg, SymbolEntry *var)
{
    int off = offset(var);
    KIND kind = var_kind(var);
    const char *op = mov_op(kind);

    if (isGlobal(var)) {
        fprintf(asm_out, "\t%s\t%s, %s\n", op, reg, var->final_code_name);
    }
    else if (is_by_ref(var)) {
        fprintf(asm_out, "\tmovl\t%d(%%ebp), %%esi\n", off);
        fprintf(asm_out, "\t%s\t%s, 0(%%esi)\n", op, reg);
    }
    else {
        fprintf(asm_out, "\t%s\t%s, %d(%%ebp)\n", op, reg, off);
    }
}

/* Moves a value of the given type from a register to the given offset in the stack */
void reg_to_var(const char *reg, KIND kind, int offset)
{
    const char *op = mov_op(kind);
    fprintf(asm_out, "\tmovl\t%d(%%ebp), %%esi\n", offset);
    fprintf(asm_out, "\t%s\t%s, 0(%%esi)\n", op, reg);
}

/* Moves a value from a register to the given boolean variable */
void reg_to_bool(const char *reg, SymbolEntry *var)
{
    if (var_kind(var) != TYPE_BOOLEAN) 
        internal("Reg_to_bool called with non boolean var");

    int off = offset(var);
    const char *op = mov_op(TYPE_BOOLEAN);

    if (isGlobal(var)) {
        fprintf(asm_out, "\t%s\t%s, %s\n", op, reg, var->final_code_name);
    }
    else if (is_by_ref(var)) {
        fprintf(asm_out, "\tmovl\t%d(%%ebp), %%esi\n", off);
        fprintf(asm_out, "\t%s\t%s, 0(%%esi)\n", op, reg);
    }
    else {
        fprintf(asm_out, "\t%s\t%s, %d(%%ebp)\n", op, reg, off);
    }
}

/* Pushes a 10 byte float to the x87 floating point stack */
void push_real_to_fstack(SymbolEntry *real)
{
    if (var_kind(real) != TYPE_REAL) 
        internal("Push_real_to_fstack called with non real var");
     
    if (isConst(real) || isGlobal(real)) 
        fprintf(asm_out, "\tfldt\t%s\n", real->final_code_name);
    else if (is_by_ref(real)) {
        fprintf(asm_out, "\tmovl\t%d(%%ebp), %%esi\n", offset(real));  // move its location to esi
        fprintf(asm_out, "\tfldt\t(%%esi)\n");                         // move from the stack to the fp stack
    }
    else 
        fprintf(asm_out, "\tfldt\t%d(%%ebp)\n", offset(real));   
}

/* Pushes an integet to the x87 floating point stack */
void push_int_to_fstack(SymbolEntry *var)
{
    if (var_kind(var) != TYPE_INTEGER) 
        internal("Push_int_to_fstack called with non int var");

    if (isGlobal(var)) {
        fprintf(asm_out, "\tmovl\t%s, -4(%%esp)\n", var->final_code_name); // move to stack
        fprintf(asm_out, "\tfildl\t-4(%%esp)\n");                          // move to fpstack
    }
    else if (isConst(var)) {  // cannot be a parameter
        fprintf(asm_out, "\tmovl\t$%d, -4(%%esp)\n", int_val(var));    // move to stack
        fprintf(asm_out, "\tfildl\t-4(%%esp)\n");                      // move to fpstack
    }
    else {
        if (is_by_ref(var)) {
            fprintf(asm_out, "\tmovl\t%d(%%ebp), %%esi\n", offset(var));
            fprintf(asm_out, "\tfildl\t(%%esi)\n");
        }
        else {
            fprintf(asm_out, "\tfildl\t%d(%%ebp)\n", offset(var));
        }
    }
}

/* Pushes a character to the x87 floating point stack */
void push_char_to_fstack(SymbolEntry *var)
{
    if (var_kind(var) != TYPE_CHAR) 
        internal("Push_char_to_fstack called with non char var");
    
    if (isGlobal(var)) {
        fprintf(asm_out, "\tmovl\t$0, -4(%%esp)\n");
        fprintf(asm_out, "\tmovsbl\t%s, -4(%%esp)\n", var->final_code_name); // move to stack
        fprintf(asm_out, "\tfildl\t-4(%%esp)\n");                            // move to fpstack
    }
    else if (isConst(var)) {  // constants cannot be passed by reference
        fprintf(asm_out, "\tmovl\t$%d, -4(%%esp)\n", char_val(var));         // move to stack
        fprintf(asm_out, "\tfildl\t-4(%%esp)\n");                            // move to fpstack
    }
    else { 
        if (is_by_ref(var)) {
            fprintf(asm_out, "\tmovl\t%d(%%ebp), %%esi\n", offset(var));     // move its location to esi
            fprintf(asm_out, "\tmovsbl\t(%%esi), %%ecx\n");                  // move to ecx as an integer
            fprintf(asm_out, "\tmovl\t%%ecx, -4(%%esp)\n");                  // move it to the stack
            fprintf(asm_out, "\tfildl\t-4(%%esp)\n");                        // move from the stack to the fp stack
        }
        else {
            fprintf(asm_out, "\tmovsbl\t%d(%%ebp), %%ecx\n", offset(var));   // move to ecx as an integer
            fprintf(asm_out, "\tmovl\t%%ecx, -4(%%esp)\n");                  // move it to the stack
            fprintf(asm_out, "\tfildl\t-4(%%esp)\n");                        // move from the stack to the fp stack
        }
    }
}

/* Moves a floating point number from the x87 floating point stack to a variable */
void fstack_to_var(SymbolEntry *var)
{
    if (isConst(var))
        internal("Assigned to constant.");
    
    int type = var_kind(var);
    
    if (isGlobal(var)) {
        if (type == TYPE_REAL) 
            fprintf(asm_out, "\tfstpt\t%s\n", var->final_code_name);
        else if (type == TYPE_INTEGER)
            fprintf(asm_out, "\tfistl\t%s\n", var->final_code_name);
        else if (type == TYPE_CHAR) {
            fprintf(asm_out, "\tfistl\t-4(%%esp)\n");                   // move from the fp regs to the stack
            fprintf(asm_out, "\tmovl\t0(%%esp), %%ecx\n");              // move from the stack to ecx
            fprintf(asm_out, "\tmovb\t%%cl, %s", var->final_code_name); // move from ecx to var location
        }
        else
            internal("fstack_to_var called with invalid type parameter.");
    }
    else if (is_by_ref(var)) {
        fprintf(asm_out, "\tmovl\t%d(%%ebp), %%esi\n", offset(var)); // esi holds the var address
        if (type == TYPE_REAL) 
            fprintf(asm_out, "\tfstpt\t(%%esi)\n");
        else if (type == TYPE_INTEGER)
            fprintf(asm_out, "\tfistl\t(%%esi)\n");
        else if (type == TYPE_CHAR) {
            fprintf(asm_out, "\tsub\t$4, %%esp\n");             // increase the stack
            fprintf(asm_out, "\tfistl\t0(%%esp)\n");            // move from the fp regs to the stack
            fprintf(asm_out, "\tmovl\t0(%%esp), %%ecx\n");      // move from the stack to ecx
            fprintf(asm_out, "\tmovb\t%%cl, (%%esi)");          // move from ecx to var location
            fprintf(asm_out, "\tadd\t$4, %%esp\n");             // decrease the stack
        }
        else
            internal("fstack_to_var called with invalid type parameter.");
    }
    else {
        if (type == TYPE_REAL) 
            fprintf(asm_out, "\tfstpt\t%d(%%ebp)\n", offset(var));
        else if (type == TYPE_INTEGER)
            fprintf(asm_out, "\tfistl\t%d(%%ebp)\n", offset(var));
        else if (type == TYPE_CHAR) {
            fprintf(asm_out, "\tsub\t$4, %%esp\n");                     // increase the stack
            fprintf(asm_out, "\tfistl\t0(%%esp)\n");                    // move from the fp regs to the stack
            fprintf(asm_out, "\tmovl\t0(%%esp), %%ecx\n");              // move from the stack to ecx
            fprintf(asm_out, "\tmovb\t%%cl, %d(%%ebp)", offset(var));   // move from ecx to var location
        }
        else
            internal("fstack_to_var called with invalid type parameter.");
    }
}

/* Moves a floating point number from the x87 floating point to the given offset in the stack */
void fstack_to_var(KIND kind, int offset, bool by_ref)
{
    if (by_ref) {
        fprintf(asm_out, "\tmovl\t%d(%%ebp), %%esi\n", offset);
        if (kind == TYPE_REAL) 
            fprintf(asm_out, "\tfstpt\t(%%esi)\n");
        else if (kind == TYPE_INTEGER)
            fprintf(asm_out, "\tfistl\t(%%esi)\n");
        else if (kind == TYPE_CHAR) {
            fprintf(asm_out, "\tsub\t$4, %%esp\n");             // increase the stack
            fprintf(asm_out, "\tfistl\t0(%%esp)\n");            // move from the fp regs to the stack
            fprintf(asm_out, "\tmovl\t0(%%esp), %%ecx\n");      // move from the stack to ecx
            fprintf(asm_out, "\tmovb\t%%cl, (%%esi)");          // move from ecx to var location
            fprintf(asm_out, "\tadd\t$4, %%esp\n");             // decrease the stack
        }
        else
            internal("fstack_to_var called with invalid type parameter.");
    }
    else {
        if (kind == TYPE_REAL) 
            fprintf(asm_out, "\tfstpt\t%d(%%ebp)\n", offset);
        else if (kind == TYPE_INTEGER)
            fprintf(asm_out, "\tfistl\t%d(%%ebp)\n", offset);
        else if (kind == TYPE_CHAR) {
            fprintf(asm_out, "\tsub\t$4, %%esp\n");                     // increase the stack
            fprintf(asm_out, "\tfistl\t0(%%esp)\n");                    // move from the fp regs to the stack
            fprintf(asm_out, "\tmovl\t0(%%esp), %%ecx\n");              // move from the stack to ecx
            fprintf(asm_out, "\tmovb\t%%cl, %d(%%ebp)", offset);        // move from ecx to var location
        }
        else
            internal("fstack_to_var called with invalid type parameter.");
    }
}

/* Move operator for the given variable kind */
const char *mov_op(int kind)
{
    switch (kind) {
        case TYPE_INTEGER:
            return "movl";
        case TYPE_BOOLEAN: 
            return "movb";
        case TYPE_CHAR:
            return "movb";
        default:
            internal("mov_op type called with invalid type");
            return "ERROR";
    }
}

/* The offset of the given variable from the base pointer */
int offset(SymbolEntry *var)
{
    switch (var->entryType) {
        case ENTRY_VARIABLE:
            return var->u.eVariable.offset;
        case ENTRY_PARAMETER:
            return var->u.eParameter.offset;
        case ENTRY_TEMPORARY:
            return var->u.eTemporary.offset;
        case ENTRY_CONSTANT:
            return 0; // dummy return value, constants will be implemented with immediate instructions
        default:
            internal("Offset called with non variable type.");
            return 0;
    }
}

/* Generates final code for a comparison quad */
void translate_comparison(quad q)
{
    fprintf(asm_out, "\tmovl\t$0, %%ecx\n");
    fprintf(asm_out, "\tmovl\t$1, %%edx\n");
    
    KIND x_kind = var_kind(q.xe);
    KIND y_kind = var_kind(q.ye);
    if (x_kind != TYPE_REAL && y_kind != TYPE_REAL) {
        var_to_reg(q.ye, "%ebx");
        var_to_reg(q.xe, "%eax");
        fprintf(asm_out, "\tcmpl\t%%ebx, %%eax\n");
    }
    else if (x_kind == TYPE_INTEGER && y_kind == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_int_to_fstack(q.xe); 
        fprintf(asm_out, "\tfucomip\n");
        fprintf(asm_out, "\tfstp\t%%st(0)\n");
    }
    else if (x_kind == TYPE_CHAR && y_kind == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_char_to_fstack(q.xe);
        fprintf(asm_out, "\tfucomip\n");
        fprintf(asm_out, "\tfstp\t%%st(0)\n");
    }
    else if (x_kind == TYPE_REAL && y_kind == TYPE_INTEGER) {
        push_int_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfucomip\n");
        fprintf(asm_out, "\tfstp\t%%st(0)\n");
    }
    else if (x_kind == TYPE_REAL && y_kind == TYPE_CHAR)    {
        push_char_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfucomip\n");
        fprintf(asm_out, "\tfstp\t%%st(0)\n");
    }   
    else if (x_kind == TYPE_REAL && y_kind == TYPE_REAL)    {
        push_real_to_fstack(q.ye);
        push_real_to_fstack(q.xe);
        fprintf(asm_out, "\tfucomip\n");
        fprintf(asm_out, "\tfstp\t%%st(0)\n");
    }
    else 
        internal("translate_comparison called with invalid operand types");
}


/* Pushes the real number that is in the top of the fstack to the stack */
void push_from_fstack()
{
    fprintf(asm_out, "\tsubl\t$10, %%esp\n");
    fprintf(asm_out, "\tfstpt\t0(%%esp)\n");
}

