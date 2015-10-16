
/******************************************************************************
 *  C++ code file : opt.cpp
 *  Project       : Pazcal Compiler
 *  Written by    : Lolos Konstantinos, Podimata Charikleia
 *                  (lolos.kostis@gmail.com, charapod@gmail.com)
 *  Date          : 2014-2015
 *  Description   : Intermediate code optimizations
 ******************************************************************************/

#include "opt.h"

extern vector<quad> quads;

using namespace std;

/* Function declarations */
int             find_first_no_op();
void            remove_quad(unsigned int i);
bool            is_quad_num(const char *name);
unsigned int    quad_num(const char *name);
void            decr_quad_num(int i);
bool            is_jump(int i);
bool            is_comparison(int i);
void            simplify_jump(int i);
void            reverse_comparison(int i);
bool            jump_simplification();
bool            remove_inconsequential_jumps();
bool            redirect_chained_jumps();
bool            dead_code_elimination();
bool            eliminate_dead_code(unsigned int i, int *reachable);
void            mark_reachable(unsigned int i, int *reachable);
bool            eliminate_unreachable(unsigned int i, int *reachable);
bool            evaluate_comparison(unsigned int i);
void            redirect_jump(int i, int target);
void            make_no_op(int i);
bool            produces_result(unsigned int i);
bool            rev_copy_propagation();
bool            is_also_operand(unsigned int i);
bool            appears_elsewhere(unsigned int i);
bool            appears_elsewhere_but_next(unsigned int i);
bool            is_jumped_upon(unsigned int i);
bool            is_assignment(unsigned int i);
bool            is_cast(unsigned int i);
bool            inconsequential_code_elimination();
bool            algebraic_transformations();
bool            is_zero_val(SymbolEntry *e);
bool            algebraic_plus(unsigned int i);
bool            algebraic_minus(unsigned int i);
bool            algebraic_mult(unsigned int i);
bool            algebraic_div(unsigned int i);
bool            algebraic_mod(unsigned int i);
bool            is_one_val(SymbolEntry *e);
bool            remove_self_assignment();

/* Performs all the intermediate code optimizations */
void optimize()
{
    bool did_opt = true;
    while (did_opt) {
        did_opt = false;
        did_opt |= jump_simplification();
        did_opt |= remove_inconsequential_jumps();
        did_opt |= redirect_chained_jumps();
        did_opt |= dead_code_elimination();
        did_opt |= rev_copy_propagation();
        did_opt |= inconsequential_code_elimination();
        did_opt |= algebraic_transformations();
        did_opt |= remove_self_assignment();
        remove_no_ops();
    }
}

/* Removes all the no_op instructions from the intermediate code */
void remove_no_ops()
{
    int i = find_first_no_op();
    while (i != -1) {
        remove_quad(i);
        i = find_first_no_op();
    }
}

/* Remove quads in the form : :=, x, -, x */
bool remove_self_assignment()
{
    bool did_opt = false;
    unsigned int i;
    for (i = 0; i < quads.size(); i++)
        if (equal(quads[i].op, ":=") && (quads[i].xe == quads[i].ze)) {
            make_no_op(i);
            did_opt = true;
        }

    return did_opt;
}

/* Algebraic transformations */
bool algebraic_transformations()
{
    bool did_opt = false;
    unsigned int i;

    for (i = 0; i < quads.size(); i++) {
        const char *op = quads[i].op;
        if      (equal(op, "+")) did_opt |= algebraic_plus(i);
        else if (equal(op, "-")) did_opt |= algebraic_minus(i);
        else if (equal(op, "*")) did_opt |= algebraic_mult(i); 
        else if (equal(op, "/")) did_opt |= algebraic_div(i); 
        else if (equal(op, "%")) did_opt |= algebraic_mod(i);
    }

    return did_opt;
}

/* Algebraic transformations for multiplication */
bool algebraic_mult(unsigned int i)
{
    bool did_opt = false;
    // *, a, 0, b => :=, 0, -, b
    if (is_zero_val(quads[i].ye) && !is_real_type(quads[i].xe) && !is_real_type(quads[i].ye)) {
        quads[i].op = ":=";
        quads[i].x  = quads[i].y;
        quads[i].xe = quads[i].ye;
        quads[i].y  = "-";
        quads[i].ye = NULL;
        did_opt     = true;
    }
    // *, 0, a, b => :=, 0, -, b
    else if (is_zero_val(quads[i].xe) && !is_real_type(quads[i].xe) && !is_real_type(quads[i].ye)) {
        quads[i].op = ":=";
        quads[i].y  = "-";
        quads[i].ye = NULL;
        did_opt     = true;
    }
    // *, a, 1, b => :=, a, -, b
    else if (is_one_val(quads[i].ye)) {
        quads[i].op = ":=";
        quads[i].y  = "-";
        quads[i].ye = NULL;
        did_opt     = true;
    }
    // *, 1, a, b => :=, a, -, b
    else if (is_one_val(quads[i].xe)) {
        quads[i].op = ":=";
        quads[i].x  = quads[i].y;
        quads[i].xe = quads[i].ye;
        quads[i].y  = "-";
        quads[i].ye = NULL;
        did_opt     = true;
    }
    return did_opt;
}

/* Algebraic transformations for division */
bool algebraic_div(unsigned int i)
{
    bool did_opt = false;
    // /, a, 1, b => :=, a, -, b
    if (is_one_val(quads[i].ye)) {
        quads[i].op = ":=";
        quads[i].y  = "-";
        quads[i].ye = NULL;
        did_opt     = true;
    }
    return did_opt;
}

/* Algebraic transformations for subtraction */
bool algebraic_minus(unsigned int i)
{
    bool did_opt = false;
    // -, a, 0, b => :=, a, -, b
    if (is_zero_val(quads[i].ye)) {
        quads[i].op = ":=";
        quads[i].y  = "-";
        quads[i].ye = NULL;
        did_opt     = true;
    }
    return did_opt; 
}

/* Algebraic transformations for addition */
bool algebraic_plus(unsigned int i)
{
    bool did_opt = false;
    // +, a, 0, b => :=, a, -, b
    if (is_zero_val(quads[i].ye)) {
        quads[i].op = ":=";
        quads[i].y  = "-";
        quads[i].ye = NULL;
        did_opt     = true;
    }
    // +, 0, a, b => :=, a, -, b
    else if (is_zero_val(quads[i].xe)) {
        quads[i].op = ":=";
        quads[i].x  = quads[i].y;
        quads[i].xe = quads[i].ye;
        quads[i].y  = "-";
        quads[i].ye = NULL;
        did_opt     = true;
    }
    return did_opt;

}

/* Algebraic transformations for modulo */
bool algebraic_mod(unsigned int i)
{
    bool did_opt = false;
    // %, a, 1, b => :=, a, -, b
    if (is_one_val(quads[i].ye)) {
        quads[i].op = ":=";
        quads[i].y  = "-";
        quads[i].ye = NULL;
        did_opt     = true;
    }
    return did_opt;
}

/* Returns true if the given symbol entry is a constant value equal to 0 */
bool is_zero_val(SymbolEntry *e)
{
    if (!e || !isConst(e))
        return false;

    KIND kind = var_kind(e);
    switch (kind) {
        case TYPE_CHAR:
            return !char_val(e);
        case TYPE_INTEGER:
            return !int_val(e);
        case TYPE_REAL:
            return !real_val(e);
        default:
            internal("is_zero_val called with invalid kind variable");
            return false;
    }
}

/* Returns true if the given symbol entry is a constant value equal to 1 */
bool is_one_val(SymbolEntry *e)
{
    if (!e || !isConst(e))
        return false;

    KIND kind = var_kind(e);
    switch (kind) {
        case TYPE_CHAR:
            return char_val(e) == 1;
        case TYPE_INTEGER:
            return int_val(e) == 1;
        case TYPE_REAL:
            return real_val(e) == 1;
        default:
            internal("is_one_val called with invalid kind variable");
            return false;
    }
}

/* Removes quads whose result is not used anywhere in the code */
bool inconsequential_code_elimination()
{
    bool did_opt = false;
    unsigned int i;
    for (i = 0; i < quads.size(); i++) 
        if (produces_result(i) && !is_by_ref(quads[i].ze) && !appears_elsewhere(i)) {
            make_no_op(i);
            did_opt = true;
        }

    return did_opt;
}

/* Removes intermediate results that get immediately assigned */
bool rev_copy_propagation()
{
    bool did_opt = false;
    unsigned int i;
    
    for (i = 0; i < quads.size() - 1; i++) {
        if ((!produces_result(i))           ||
            (quads[i].ze != quads[i+1].xe)  ||
            (is_also_operand(i))            ||
            (!is_assignment(i+1))           ||
            (is_cast(i) && is_cast(i+1))    ||
            (appears_elsewhere_but_next(i)) ||
            (is_jumped_upon(i+1))
           )
            continue;
            
        quads[i].z  = quads[i+1].z;
        quads[i].ze = quads[i+1].ze;
        make_no_op(i+1);
        did_opt = true;
    }

    return did_opt;
}

/* Returns true if the given quad num performs type casting */
bool is_cast(unsigned int i)
{
    if (is_assignment(i))
        return !equal_types(var_type(quads[i].xe), var_type(quads[i].ze));
    else {
        KIND x_kind = var_kind(quads[i].xe);
        KIND y_kind = var_kind(quads[i].ye);
        KIND z_kind = var_kind(quads[i].ze);
        if (x_kind == TYPE_CHAR && y_kind == TYPE_CHAR) {
            if (equal(quads[i].op, "%"))
                return z_kind != TYPE_INTEGER;
            else
                return z_kind != TYPE_CHAR;
        }
        if (x_kind == TYPE_CHAR && y_kind == TYPE_INTEGER)
            return z_kind != TYPE_INTEGER;
        if (x_kind == TYPE_CHAR && y_kind == TYPE_REAL)
            return z_kind != TYPE_REAL;
        if (x_kind == TYPE_INTEGER && y_kind == TYPE_CHAR)
            return z_kind != TYPE_INTEGER;
        if (x_kind == TYPE_INTEGER && y_kind == TYPE_INTEGER)
            return z_kind != TYPE_INTEGER;
        if (x_kind == TYPE_INTEGER && y_kind == TYPE_REAL)
            return z_kind != TYPE_REAL;
        if (x_kind == TYPE_REAL)
            return z_kind != TYPE_REAL;
        return false;
    }
}

/* Returns true if the given quad num is a value assignment */
bool is_assignment(unsigned int i)
{
    return equal(quads[i].op, ":=");
}

/* Returns true if the result of the quad is also an operand (for example :=, x, 1, x) */
bool is_also_operand(unsigned int i)
{
    return equal(quads[i].x, quads[i].z) || equal(quads[i].y, quads[i].z);
}

/* Returns true if the result of the quad is an operand in quads other than i */
bool appears_elsewhere(unsigned int i)
{
    SymbolEntry *ze = quads[i].ze;
    unsigned int j;
    
    for (j = 0; j < quads.size(); j++) {
        if (i == j)
            continue;
        if (quads[j].xe == ze || quads[j].ye == ze)
            return true;
    }
    return false;
}

/* Returns true if the result of the quad is an operand in quads other than i and i+1 */
bool appears_elsewhere_but_next(unsigned int i)
{
    SymbolEntry *ze = quads[i].ze;
    unsigned int j;
    
    for (j = 0; j < quads.size(); j++) {
        if (i == j || i+1 == j)
            continue;
        if (quads[j].xe == ze || quads[j].ye == ze)
            return true;
    }
    return false;
}

/* Returns true if there is a jump to the given quad num */
bool is_jumped_upon(unsigned int i)
{
    unsigned int j;
    for (j = 0; j < quads.size(); j++)
        if (is_jump(j) || (is_comparison(j) && is_quad_num(quads[j].z)))
            if (quad_num(quads[j].z) == i)
                return true;

    return false;
}

/* Returns true if the quad produces a result */
bool produces_result(unsigned int i)
{
    if (!quads[i].ze)
        return false;

    const char * op = quads[i].op;
    return (equal(op, "+")   ||
            equal(op, "-")   ||
            equal(op, "*")   ||
            equal(op, "/")   ||
            equal(op, "%")   ||
            equal(op, "==")  ||
            equal(op, "not") ||
            equal(op, "!=")  ||
            equal(op, ">")   ||
            equal(op, ">=")  ||
            equal(op, "<")   ||
            equal(op, "<=")  ||
            equal(op, ":=")
           );
}

/* Remove unreachable code */
bool dead_code_elimination()
{
    bool did_opt = false;
    int *reachable = (int *) newAlloc(quads.size() * sizeof(int));
    unsigned int i;
    for (i = 0; i < quads.size(); i++)
        reachable[i] = 0;

    for (i = 0; i < quads.size(); i++)
        if (equal(quads[i].op, "unit"))
            did_opt |= eliminate_dead_code(i, reachable);

    return did_opt;
}

/* Eliminates all unreachable code from the given quad to the end of the current function */
bool eliminate_dead_code(unsigned int i, int *reachable)
{
    mark_reachable(i+1, reachable);
    return eliminate_unreachable(i+1, reachable);
}

/* Marks all the quads that are reachable from the given quad (aka DFS) */
void mark_reachable(unsigned int i, int *reachable)
{
    if (equal(quads[i].op, "endu") || reachable[i])
        return;

    reachable[i] = 1;
    if (is_jump(i))
        mark_reachable(quad_num(quads[i].z), reachable);
    else if (is_comparison(i) && is_quad_num(quads[i].z)) {
        if (isConst(quads[i].xe) && isConst(quads[i].ye)) {
            VALUE vx = get_val(quads[i].xe);
            VALUE vy = get_val(quads[i].ye);
            KIND  kx = var_kind(quads[i].xe);
            KIND  ky = var_kind(quads[i].ye);
            bool will_jump = eval_comp_expr(vx, vy, kx, ky, quads[i].op);
            if (will_jump) {
                int target = quad_num(quads[i].z);
                quads[i].op = "jump";
                quads[i].x  = "-";
                quads[i].y  = "-";
                mark_reachable(target, reachable);
            }
            else {
                make_no_op(i);
                mark_reachable(i+1, reachable);
            }
        }
        else {
            int target = quad_num(quads[i].z);
            mark_reachable(target, reachable); 
            mark_reachable(i+1, reachable); 
        }
    }
    else if (!equal(quads[i].op, "ret") && !equal(quads[i].op, "retv"))
        mark_reachable(i+1, reachable);
}

/* Turns all code that has not been marked as reachable to no_op quads */
bool eliminate_unreachable(unsigned int i, int *reachable)
{
    bool did_opt = false;
    while (!equal(quads[i].op, "endu")) {
        if (!reachable[i]) {
            make_no_op(i);
            did_opt = true;
        }
        i++;
    }
    return did_opt;
}

/* Remove jumps to the next quad */
bool remove_inconsequential_jumps()
{
    bool did_opt = false;   
    for (unsigned int i = 0; i < quads.size(); i++) 
        if (is_jump(i) || (is_comparison(i) && is_quad_num(quads[i].z))) {
            unsigned int target = quad_num(quads[i].z);
            if (target == i+1) {
                make_no_op(i);
                did_opt = true;
            }
        }
    return did_opt;
}

/* Redirect jumps that lead to jumps */
bool redirect_chained_jumps()
{
    bool did_opt = false;   
    for (unsigned int i = 0; i < quads.size(); i++) {
        if (is_jump(i) || (is_comparison(i) && is_quad_num(quads[i].z))) {
            unsigned int target = quad_num(quads[i].z);
            while (is_jump(target) && target != i)
                target = quad_num(quads[target].z);
            
            if (target != i) // if it is not an infinite loop
                redirect_jump(i, target);
        }
    }
    return did_opt;
}

/* Redirects a jump to the given target */
void redirect_jump(int i, int target)
{
    char *c = (char *) newAlloc(20);
    sprintf(c, "%d", target);
    quads[i].z = c;
}

/* Turns the given quad to a no_op quad */
void make_no_op(int i)
{
    quads[i].op = "no_op";
    quads[i].x  = "-";
    quads[i].y  = "-";
    quads[i].z  = "-";
    quads[i].xe = NULL;
    quads[i].ye = NULL;
    quads[i].ze = NULL;
}

/* Turns comparisons that just avoid jumps to a single instuction.
   For example:
   3: ==, x, y, 5
   4: jump, -, -, 42
   is converted to:
   3: !=, x, y, 42
*/
bool jump_simplification()
{
    bool did_opt = false; 
    for (unsigned int i = 0; i < quads.size() - 1; i++) 
        if (is_comparison(i) && (is_quad_num(quads[i].z)) && (quad_num(quads[i].z) == i + 2)) 
            if (is_jump(i+1)) {
                simplify_jump(i);
                did_opt = true;
            }
    return did_opt;
}

/* Performs the jump simplification for quad i */
void simplify_jump(int i)
{
    reverse_comparison(i);
    quads[i].z = quads[i+1].z;
    make_no_op(i+1);
}

/* Returns true if quad i is a jump */
bool is_jump(int i)
{
    return equal(quads[i].op, "jump");
}

/* Returns true if quad i is a comparison */
bool is_comparison(int i)
{
    if (equal(quads[i].op, "=="))       return true;
    else if (equal(quads[i].op, "!="))  return true;    
    else if (equal(quads[i].op, ">"))   return true;    
    else if (equal(quads[i].op, "<"))   return true;    
    else if (equal(quads[i].op, ">="))  return true;    
    else if (equal(quads[i].op, "<="))  return true;    
    else return false;
}

/* Reverses the comparison operator for quad i */
void reverse_comparison(int i)
{   
    if (equal(quads[i].op, "=="))       quads[i].op = "!=";
    else if (equal(quads[i].op, "!="))  quads[i].op = "==";    
    else if (equal(quads[i].op, ">"))   quads[i].op = "<=";    
    else if (equal(quads[i].op, "<"))   quads[i].op = ">=";    
    else if (equal(quads[i].op, ">="))  quads[i].op = "<";    
    else if (equal(quads[i].op, "<="))  quads[i].op = ">";    
}

/* Returns the index of the first no_op instruction in the code, -1 if none exists */
int find_first_no_op()
{
    for (unsigned int i = 0; i < quads.size(); i++)
        if (equal(quads[i].op, "no_op"))
            return i;
    
    return -1;
}

/* Removes the given quad and adjusts the quad numbers accordingly */
void remove_quad(unsigned int index)
{
    quads.erase(quads.begin() + index);
    for (unsigned int i = 0; i < quads.size(); i++)
        if (is_quad_num(quads[i].z) && quad_num(quads[i].z) > index)
            decr_quad_num(i);
}

/* Returns true if the given string in a quad number.
   (Actually it just checks if it is an integer)        */
bool is_quad_num(const char *name)
{
    while (*name) {
        if (*name < '0' || *name > '9')
            return false;
        name++;
    }

    return true;
}

/* Returns the quad number this instruction points to */
unsigned int quad_num(const char *name)
{
    return atoi(name);
}

/* Decrements the quad number this instructin points to by 1 */
void decr_quad_num(int i)
{
    int old_index = quad_num(quads[i].z);
    char *c = (char *) newAlloc(20);
    sprintf(c, "%d", old_index - 1);
    quads[i].z = c;
}

