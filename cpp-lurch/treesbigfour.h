//
//  words.h
//  
//
//  Created by Philip Gressman on 12/14/18.
//

#ifndef treesbigfour_h
#define treesbigfour_h
#include "trees.h"
#include "treesroutines.h"
#include <random>
// -std=c++11

int tree_randseed = 0;
mt19937 rand_gen;

bool DataTree::running_initialize() {
    /* What to do on the very first encounter of a tag; before entering its contents */
    /* Obvious things would include loading content, etc. */
    DataTree *child = chld_frst;

    if ((myname != "")&&(mytag.get_attribute("name")!=myname)) {myname = mytag.get_attribute("name");} // RECENT: 2/8/18 Did this because variables inside <block> weren't getting unnamed

    /* For variable substitution purposes, it's important to clear the result tree unless you're
     totally certain that the result doesn't change */
    if ((chld_frst != this)||((mytag.get_type()!="number")&&(mytag.get_type()!="string"))) {delete_result_tree();}
    
    if (mytag.get_type() == "dostring") {if (!initialize_dostring(this)) {return false;} child = chld_frst;}
    if (mytag.get_type() == "parse") {if (!initialize_parse(this)) {return false;}
        exe_inside_frst = this; exe_inside_last = this; return true;
    }
    else if (mytag.get_type() == "dosubroutine") {if (!initialize_dosubroutine(this)) {return false;} child = chld_frst;}
    else if (mytag.get_type() == "subroutine") {return initialize_subroutine(this);}
    
    
    if ((mytag.get_type() == "pick")&&(mytag.is_clopen())) {
        if(!initialize_clopen_pick(this)) {return false;}
    }
    

    if (chld_frst==this) {return true;}
    
    if (mytag.get_type() != "pick") { /* A typical DOALL type tag in which all children are executed */
        while(child != this) {
            child->exe_before = child->sibl_prev; child->exe_after = child->sibl_next;
            exe_inside_frst = chld_frst; exe_inside_last = chld_last;
            child = child->sibl_next;
        }
    }
    if (mytag.get_type() == "pick") { /*    A typical PICKONE type tag */
        // This is where you would shuffle things if they're randomized.
        int j,totkids = total_children();
        if ((totkids > 1) && (mytag.get_attribute("shuffle") == "true")) {
            uniform_int_distribution<int> picker(0,totkids-1);
            for(int i =0; i < totkids; i++) {
                //j = rand() % totkids;
                j = picker(rand_gen);
                swap_children(i,j);
            }
        }
        if (chld_frst != this) {
            chld_frst->exe_before = this; chld_frst->exe_after = this;
            exe_inside_frst = chld_frst; exe_inside_last = chld_frst;
        }
    }
    if (mytag.get_type()=="define") {myname = "";}
    return true;
}

bool DataTree::running_increment() {
    /* Executing content at some point has failed. Can you adapt before giving up? */
    
    if (mytag.get_type() == "pick") {
        DataTree *chld_next = exe_inside_frst;
        if (chld_next == this) {return false;}
        chld_next = chld_next->sibl_next;
        if (chld_next == this) { return false;}
        chld_next->exe_before = this; chld_next->exe_after = this;
        exe_inside_frst = chld_next; exe_inside_last = chld_next;
        return true;
    }
    
    if (mytag.get_type() == "whilegood") {
        if (exe_inside_frst == this) {return false;}
        exe_inside_frst = this; exe_inside_last = this; return true;
    }
    
    if (mytag.get_type() == "whilebad") {
        if (exe_inside_frst == this) {return false;}
        return true;
    }
    
    
    return false;
}

bool DataTree::running_finalize() {
    /* All your internal content has executed successfully. How to you use it? */
    
    if (shortcut_finalize != NULL) {return (*shortcut_finalize)(this);}
    
    string type = mytag.get_type();
     if (type == "output") {return finalize_output(this,world);}
    else if (type == "input") {return finalize_input(this,world);}
    else if (type == "pick") {
        DataTree *chld_next = exe_inside_frst;
        if (chld_next->has_result_tree()) {this->reset_result_tree()->clone(chld_next->get_result_tree()); } else {delete_result_tree();} return true;
    }
    else if (type == "whilegood") {
        if (exe_inside_last == this) {return true;}
        return false;
    }
    else if (type == "number") {return finalize_number(this);}
    else if (type == "string") {return finalize_string(this);}
    else if (type == "min") {return finalize_min(this);}
    else if (type == "max") {return finalize_max(this);}
    else if (type == "prod") {return finalize_prod(this);}
    else if (type == "sum") {return finalize_sum(this);}
    else if (type == "equal") {return finalize_equal(this);}
    else if (type == "unequal") {return finalize_unequal(this);}
    else if (type == "leq") {return finalize_leq(this);}
    else if (type == "geq") {return finalize_geq(this);}
    else if (type == "debug") {return finalize_debug(this);}
    else if (type == "variable") {return finalize_variable(this);}
    else if (type == "all") {return finalize_all(this);}
    else if (type == "file") {return finalize_file(this);}
    else if (type == "paren") {return finalize_paren(this);}
    else if (type == "minus") {return finalize_minus(this);}
    else if (type == "divide") {return finalize_divide(this);}
    else if (type == "exponent") {return finalize_exponent(this);}
    else if (type == "mod") {return finalize_mod(this);}
    else if (type == "abs") {return finalize_abs(this);}
    else if (type == "define") {return finalize_define(this);}
    else if (type == "ascii") {return finalize_ascii(this);}
    else if (type == "defined") {return finalize_defined(this);}
    else if (type == "undefined") {return finalize_undefined(this);}
    else if (type == "dostring") {return finalize_dostring(this);}
    else if (type == "block") {return finalize_all(this);}
    else if (type == "subroutine") {return true;}
    else if (type == "dosubroutine") {return finalize_dosubroutine(this);}
    else if (type == "catalog") {return finalize_catalog(this);}
    else if (type == "oracle") {return finalize_oracle(this);}
    else if (type == "ordered") {return finalize_ordered(this);}
    else if (type == "findreplace") {return finalize_findreplace(this);}
    else if (type == "funceval") {return finalize_funceval(this);}
    else if (type == "funcdel") {return finalize_funcdel(this);}
    else if (type == "sort") {return finalize_sort(this);}
    else if (type == "parse") {return finalize_parse(this);}
    else if (type == "rfill") {return finalize_rfill(this);}
    else if (type == "lfill") {return finalize_lfill(this);}
    else if (type == "cfill") {return finalize_cfill(this);}
    else if (type == "stringbefore") {return finalize_str_before(this);}
    else if (type == "stringafter") {return finalize_str_after(this);}
    else if (type == "gcd") {return finalize_gcd(this);}
    else if (type == "latex") {return finalize_latex(this);}
    else if (type == "score") {return finalize_score(this);}
    else if (type == "showtree") {return finalize_showtree(this);}
    else if (type == "numerator") {return finalize_numerator(this);}
    else if (type == "denominator") {return finalize_denominator(this);}
    else if (type == "project") {return finalize_project(this);}

    
    return true;
}

/* You produced fine output which has seen failure at some later point. Can you do anything
 without looking back at your own contents? Probably not */

bool DataTree::running_reenter() {
    if (mytag.get_type()=="subroutine") {return false;}
    if ((chld_frst != this)||((mytag.get_type()!="number")&&(mytag.get_type()!="string"))) {delete_result_tree();}
    if ((mytag.get_type()=="block")||(mytag.get_type()=="whilebad")) {
        this->exe_inside_last = this; this->exe_inside_frst = this;
    }
    if (mytag.get_type()=="define") {myname="";}
    if ((myname != "")&&(mytag.get_attribute("name")!=myname)) {myname = mytag.get_attribute("name");} // 1/26 RECENT ADDED
    return false;
}


#endif /* data_h */
