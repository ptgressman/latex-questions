//
//  words.h
//  
//
//  Created by Philip Gressman on 12/14/18.
//

#ifndef treesroutines_h
#define treesroutines_h

#include "trees.h"
#include "integers.h"
#include "data.h"
#include "treeparser.h"
// -std=c++11

/* Because these functions hijack shortcut_finalize, make sure you don't accidentally hand over control to the wrong finalize function */

bool has_two_simple_results(DataTree *tree)
{
    if (tree->total_children() != 2) {return false;}
    if (!tree->chld_frst->has_result_tree()) {return false;}
    if (!tree->chld_frst->get_result_tree()->is_terminal()) {return false;}
    //if (tree->chld_frst->get_result_tree()->get_tag()->get_type()!="string") {return false;}
    if (!tree->chld_frst->sibl_next->has_result_tree()) {return false;}
    if (!tree->chld_frst->sibl_next->get_result_tree()->is_terminal()) {return false;}
    //if (tree->chld_frst->sibl_next->get_result_tree()->get_tag()->get_type()!="string") {return false;}
    return true;
}

bool finalize_rfill(DataTree *tree)
{
    string first, second;
    if (!has_two_simple_results(tree)) {
        tree->warn("ERROR: Invalid <rfill> specification\n");
        return false;
    }
    first = tree->first_child()->get_result_tree()->get_data()->value();
    second = tree->first_child()->sibl_next->get_result_tree()->get_data()->value();
    
    if (first.length() <= second.length()) {
        first = second;
    } else {
        first = first.replace(first.length()-second.length(),second.length(),second);
    }
    tree->reset_result_tree()->get_data()->copy(Data(first));
    tree->get_result_tree()->reset_tag_type("string");
    tree->shortcut_finalize = &finalize_rfill;
    return true;
}

bool finalize_lfill(DataTree *tree)
{
    string first, second;
    if (!has_two_simple_results(tree)) {
        tree->warn("ERROR: Invalid <lfill> specification\n");
        return false;
    }
    first = tree->first_child()->get_result_tree()->get_data()->value();
    second = tree->first_child()->sibl_next->get_result_tree()->get_data()->value();
    
    if (first.length() <= second.length()) {
        first = second;
    } else {
        first = first.replace(0,second.length(),second);
    }
    tree->reset_result_tree()->get_data()->copy(Data(first));
    tree->get_result_tree()->reset_tag_type("string");
    tree->shortcut_finalize = &finalize_lfill;
    return true;
}

bool finalize_project(DataTree *tree)
{
    DataTree *theresults;
    if (tree->total_children() != 2) {tree->warn("ERROR: <project> must have two children: object , index.\n"); return false;}
    if (!tree->chld_frst->has_result_tree()) {tree->warn("ERROR: <project> children must produce results. \n"); return false;}
    if (!tree->chld_frst->sibl_next->has_result_tree()) {tree->warn("ERROR: <project> children must produce results. \n"); return false;}
    if (!tree->chld_frst->sibl_next->get_result_tree()->is_terminal()) {tree->warn("ERROR: <project> must have two children: object , index.\n"); return false;}
    if (!tree->chld_frst->sibl_next->get_result_tree()->get_data()->is_number()) {tree->warn("ERROR: <project> must have two children: object , index.\n"); return false;}
    int whichone = tree->chld_frst->sibl_next->get_result_tree()->get_data()->to_Number();
    
    theresults = tree->chld_frst->get_result_tree();
    while((theresults->get_tag()->get_type()=="all")&&(theresults->total_children()==1)&&(!theresults->chld_frst->is_terminal())) {theresults = theresults->chld_frst;}
    
    if (theresults->total_children() <= whichone) {
        tree->warn("ERROR: <project> first object doesn't have enough subresults.\n"); return false;
    }
    tree->reset_result_tree();
    tree->get_result_tree()->clone(theresults->child(whichone));
    tree->shortcut_finalize = &finalize_project;
    return true;
    
    
}

bool finalize_cfill(DataTree *tree)
{
    string first, second;
    int howmany = 0;
    if (!has_two_simple_results(tree)) {
        tree->warn("ERROR: Invalid <cfill> specification\n");
        return false;
    }
    first = tree->first_child()->get_result_tree()->get_data()->value();
    second = tree->first_child()->sibl_next->get_result_tree()->get_data()->value();
    
    if (first.length() <= second.length()) {
        first = second;
    } else {
        howmany = (first.length() - second.length()) / 2;
        first = first.replace(howmany,second.length(),second);
    }
    tree->reset_result_tree()->get_data()->copy(Data(first));
    tree->get_result_tree()->reset_tag_type("string");
    tree->shortcut_finalize = &finalize_cfill;
    return true;
}

bool finalize_str_before(DataTree *tree)
{
    string first, second;
    int posn;
    if (!has_two_simple_results(tree)) {
        tree->warn("ERROR: Invalid <stringbefore> specification\n");
        return false;
    }
    first = tree->first_child()->get_result_tree()->get_data()->value();
    second = tree->first_child()->sibl_next->get_result_tree()->get_data()->value();
    
    posn = first.find(second);
    if (posn < first.length()) {first = first.substr(0,posn);}

    tree->reset_result_tree()->get_data()->copy(Data(first));
    tree->get_result_tree()->reset_tag_type("string");
    tree->shortcut_finalize = &finalize_str_before;
    return true;
}
bool finalize_str_after(DataTree *tree)
{
    string first, second;
    int posn;
    if (!has_two_simple_results(tree)) {
        tree->warn("ERROR: Invalid <stringafter> specification\n");
        return false;
    }
    first = tree->first_child()->get_result_tree()->get_data()->value();
    second = tree->first_child()->sibl_next->get_result_tree()->get_data()->value();
    
    posn = first.find(second);
    if (posn < first.length()) {first = first.substr(posn+second.length(),first.length()-posn-second.length());} else {first = "";}
    
    tree->reset_result_tree()->get_data()->copy(Data(first));
    tree->get_result_tree()->reset_tag_type("string");
    tree->shortcut_finalize = &finalize_str_after;
    return true;
}

DataTree *match_ordereds(DataTree *shorter, DataTree *longer)
{
    if ((shorter->get_tag()->get_type()!="ordered")||(longer->get_tag()->get_type() != "ordered")) {return longer;}
    DataTree *shortkids, *longkids;
    shortkids = shorter->chld_frst;
    longkids = longer->chld_frst;
    while((shortkids!=shorter)&&(longkids != longer)) {
        if (!shortkids->test_value_and_tag_equality(longkids)) {return longer;}
        shortkids = shortkids->sibl_next;
        longkids = longkids->sibl_next;
    }
    if ((shortkids != shorter)||(longkids == longer)) {return longer;}
    return longkids;
}

bool finalize_funceval(DataTree *tree) {
    tree->shortcut_finalize = &finalize_funceval;
    DataTree *resultptr = tree->link_child_results();
    DataTree *firstone,*abscissa;
    if (resultptr == tree) {return true; /* No results found */}
    bool state,continues, found=false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}
    while(continues) {
        if (!found) {found = true; firstone = resultptr;}
        else {
            abscissa = match_ordereds(firstone,resultptr);
            if (abscissa != resultptr) {
                tree->unlink_child_results();
                tree->reset_result_tree();
                tree->get_result_tree()->reset_tag_type("all");
                while(abscissa!=resultptr) {
                    tree->get_result_tree()->clone_as_child(abscissa);
                    abscissa = abscissa->sibl_next;
                }
                return true;
            }
            
        }
        
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    /*if (tree->get_tag()->get_attribute("silent") != "true")*/
    tree->warn("ERROR: point is not in domain of function.\n");
    return false;
}

bool finalize_funcdel(DataTree *tree) {
    tree->shortcut_finalize = &finalize_funcdel;
    DataTree *resultptr = tree->link_child_results();
    DataTree *firstone,*abscissa;
    if (resultptr == tree) {return true; /* No results found */}
    bool state,continues, found=false;
    bool founddom = false;
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}
    while(continues) {
        if (!found) {found = true; firstone = resultptr;}
        else {
            abscissa = match_ordereds(firstone,resultptr);
            if (abscissa == resultptr) {
                if (!founddom) {founddom = true; tree->reset_result_tree(); tree->get_result_tree()->reset_tag_type("all");}
                tree->get_result_tree()->clone_as_child(resultptr);
            }
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    return true;
}


bool initialize_clopen_pick(DataTree *tree)
{
    tree->cleanup();
    if (tree->sibl_prev == tree->parent) {
        tree->warn("ERROR: Nothing comes before <pick/>"); return false;
    }
    if (!tree->sibl_prev->has_result_tree()) {
        tree->warn("ERROR: No prior result to pick from."); return false;
    }
    //if (tree->sibl_prev->get_result_tree()->get_tag()->get_type()=="all") {
        DataTree *ender=tree->sibl_prev->get_result_tree();
        DataTree *child=ender->chld_frst;
        while(child != ender) {
            tree->clone_as_child(child);
            child = child->sibl_next;
        }
        tree->sibl_prev->delete_result_tree();
    //}
    return true;
}

bool finalize_showtree(DataTree *tree) {
    tree->shortcut_finalize = &finalize_showtree;
    DataTree *goesup = tree;
    while(!goesup->is_root()) {goesup = goesup->parent;}
    tree->reset_result_tree()->get_data()->copy(Data(goesup->XMLFMT()));
    tree->get_result_tree()->reset_tag_type("string");
    return true;
}

bool finalize_all(DataTree *tree) {
    tree->shortcut_finalize = &finalize_all;
    DataTree *resultptr = tree->link_child_results();
    if (resultptr == tree) {return true; /* No results found */}
    bool state,continues, found=false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}
    DataTree *mainresult = tree->reset_result_tree();
    while(continues) {
        found = true;
        mainresult->clone_as_child(resultptr);
        resultptr = resultptr->bunch_step(state,continues);
    }
    if (found) {mainresult->reset_tag_type("all");} else {tree->delete_result_tree();}
    tree->unlink_child_results();
    return true;
}

bool finalize_ordered(DataTree *tree) {
    bool result = finalize_all(tree);
    tree->shortcut_finalize = &finalize_ordered; // Important to do this **after** since finalize_all will mess it up
    if (result && tree->has_result_tree()) {
        tree->get_result_tree()->reset_tag_type("ordered");
    }
    return result;
}

bool finalize_findreplace(DataTree *tree) {
    tree->shortcut_finalize = &finalize_findreplace;
    DataTree *resultptr = tree->link_child_results();
    DataTree *starttree;
    bool startstate,startcont;
    if (resultptr == tree) {return true; /* No results found */}
    bool state,continues;
    bool has_changed = true;
    string tomanip = "";
    string needle, replacer;
    int wherefound;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}
    
    if ((resultptr->get_tag()->get_type()!="string")&&(resultptr->get_tag()->get_type()!="number")) {
        tree->warn("ERROR: First object inside find replace must be string or number type.\n");
        tree->unlink_child_results(); return false;
    }
    tomanip = resultptr->get_data()->value();
    starttree = resultptr->bunch_step(state,continues);
    startstate = state; startcont = continues;
    
    while(has_changed) {
        has_changed = false;
        resultptr = starttree; state = startstate; continues = startcont;
        while(continues) {
            if ((resultptr->get_tag()->get_type()!="ordered")||(resultptr->first_child()==resultptr)||(resultptr->first_child()->sibl_next!=resultptr->chld_last)) {
                tree->warn("ERROR: Only binary <ordered> objects can follow after main object in find replace.\n"); tree->unlink_child_results(); return false;
            }
            needle = resultptr->chld_frst->get_data()->value();
            replacer = resultptr->chld_frst->sibl_next->get_data()->value();
            
            wherefound = tomanip.find(needle);
            if ((wherefound >= 0)&&(wherefound < tomanip.length())) {
                has_changed = true; continues = false;
                tomanip.replace(wherefound,needle.length(),replacer);
            }
            
            if (!has_changed) {resultptr = resultptr->bunch_step(state,continues);}
        }
    }
    tree->unlink_child_results();
    tree->reset_result_tree()->get_data()->copy(Data(tomanip));
    tree->get_result_tree()->reset_tag_type("string");
    return true;
}


bool finalize_sum(DataTree *tree) {
    tree->shortcut_finalize = &finalize_sum;
    DataTree *resultptr = tree->link_child_results();
    if (resultptr == tree) {return true; /*No child results found */}
    bool state,continues;
    Data d,e;
    bool found = false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}
    while(continues) {
        if ((resultptr->get_tag()->get_type() == "number")||(resultptr->get_tag()->get_type() =="string")) {
            e = *resultptr->get_data();
            if (!found) {found = true; d = e;} else {d = d + e;}
        } else {
            tree->unlink_child_results();
            //tree->delete_result_tree();
            tree->warn("ERROR: Only numbers and strings allowed in sum.\n");
            return false;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (found) {
        DataTree *myresult = tree->reset_result_tree();
        myresult->get_data()->copy(d);
        if (d.is_number()) {myresult->reset_tag_type("number");} else {myresult->reset_tag_type("string");}
    } else {/*tree->delete_result_tree();*/ tree->warn("WARNING: Nothing here to sum.\n");}
    return true;
}

bool finalize_prod(DataTree *tree) {
    tree->shortcut_finalize = &finalize_prod;
    DataTree *resultptr = tree->link_child_results();
    if (resultptr == tree) {return true; /*No child results found */}
    bool state,continues;
    Number e,n = Number(1);
    bool found = false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}

    while(continues) {
        if (resultptr->get_tag()->get_type() == "number") {
            e = resultptr->get_data()->to_Number();
            if (!found) {found = true; n = e;} else {n = n * e;}
        } else {
            tree->unlink_child_results();
            /*tree->delete_result_tree();*/
            tree->warn("ERROR: Only numbers allowed in product.\n");
            return false;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (found) {
        DataTree *mainresult = tree->reset_result_tree();
        mainresult->get_data()->copy(Data(n));
        mainresult->reset_tag_type("number");
    } else {/*tree->delete_result_tree();*/ tree->warn("WARNING: Nothing here in product.\n");}
    return true;
}


bool finalize_gcd(DataTree *tree) {
    tree->shortcut_finalize = &finalize_gcd;
    DataTree *resultptr = tree->link_child_results();
    if (resultptr == tree) {return true; /*No child results found */}
    bool state,continues;
    int e,n ;
    bool found = false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}
    while(continues) {
        if ((resultptr->get_tag()->get_type() == "number")&&(resultptr->get_data()->to_Number().is_integer())) {
            e = (int) resultptr->get_data()->to_Number();
            if (!found) {found = true; n = e;} else {n = gcd(e,n);}
        } else {
            tree->unlink_child_results();
            tree->warn("ERROR: Only integers allowed in gcd.\n");
            return false;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (found) {
        DataTree *mainresult = tree->reset_result_tree();
        mainresult->get_data()->copy(Data(n));
        mainresult->reset_tag_type("number");
    } else { tree->warn("WARNING: Nothing here in gcd.\n");}
    return true;
}

bool finalize_max(DataTree *tree) {
    tree->shortcut_finalize = &finalize_max;
    DataTree *resultptr = tree->link_child_results();
    if (resultptr == tree) {return true; /*No child results found */}
    bool state,continues;
    Number e,n ;
    bool found = false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}

    while(continues) {
        if (resultptr->get_tag()->get_type() == "number") {
            e = resultptr->get_data()->to_Number();
            if (!found) {found = true; n = e;} else {if (n < e) {n = e;}}
        } else {
            tree->unlink_child_results();
            /*tree->delete_result_tree();*/
            tree->warn("ERROR: Only numbers allowed in max.\n");
            return false;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (found) {
        DataTree *mainresult = tree->reset_result_tree();
        mainresult->get_data()->copy(Data(n));
        mainresult->reset_tag_type("number");
    } else {/*tree->delete_result_tree();*/ tree->warn("WARNING: Nothing here in maximum.\n");}
    return true;
}

bool finalize_score(DataTree *tree) {
    tree->shortcut_finalize = &finalize_score;
    DataTree *resultptr = tree->link_child_results();
    DataTree *restart = resultptr;
    if (resultptr == tree) {return true; /*No child results found */}
    bool state,continues;
    int commonden = 1;
    int den,num,numsum=0,numpos = 0,numneg = 0;
    bool found = false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}
    
    while(continues) {
        if (resultptr->get_tag()->get_type() == "number") {
            den = resultptr->get_data()->to_Number().denominator;
            if (den == 0) {tree->unlink_child_results(); tree->warn("ERROR: Zero denominator in <score>.\n"); return false;}
            if (!found) {found = true; commonden = den;} else {commonden = den * commonden / gcd(den,commonden);}
        } else {
            tree->unlink_child_results();
            /*tree->delete_result_tree();*/
            tree->warn("ERROR: Only numbers allowed in <score>.\n");
            return false;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    if (!found) { tree->warn("WARNING: Nothing here to score.\n"); return false;}
    resultptr = restart->bunch_start(state,continues);
    while(continues) {
        num = resultptr->get_data()->to_Number().numerator;
        den = resultptr->get_data()->to_Number().denominator;
        if (num > 0) {
            numpos = numpos + num * commonden / den;
        } else {
            numneg = numneg - num * commonden / den;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    numsum = numpos;
    if (numsum < numneg) {numsum = numneg;}
    if (numsum < commonden) {numsum = commonden;}
    tree->unlink_child_results();
    if (found) {
        DataTree *mainresult = tree->reset_result_tree();
        mainresult->get_data()->copy(Data(numsum));
        mainresult->reset_tag_type("number");
    } else {/*tree->delete_result_tree();*/ tree->warn("WARNING: Nothing here to score.\n");}
    return true;
}

bool finalize_min(DataTree *tree) {
    tree->shortcut_finalize = &finalize_min;
    DataTree *resultptr = tree->link_child_results();
    if (resultptr == tree) {return true; /*No child results found */}
    bool state,continues;
    Number e,n ;
    bool found = false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}

    while(continues) {
        if (resultptr->get_tag()->get_type() == "number") {
            e = resultptr->get_data()->to_Number();
            if (!found) {found = true; n = e;} else {if (n > e) {n = e;}}
        } else {
            tree->unlink_child_results();
            /*tree->delete_result_tree();*/
            tree->warn("ERROR: Only numbers allowed in min.\n");
            return false;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (found) {
        DataTree *mainresult = tree->reset_result_tree();
        mainresult->get_data()->copy(Data(n));
        mainresult->reset_tag_type("number");
    } else {/*tree->delete_result_tree();*/ tree->warn("WARNING: Nothing here in maximum.\n");}
    return true;
}



bool finalize_leq(DataTree *tree) {
    tree->shortcut_finalize = &finalize_leq;
    DataTree *resultptr = tree->link_child_results();
    if (resultptr == tree) {return true; /*No child results found */}
    bool state,continues;
    Number e,n ;
    bool found = false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}
    while(continues) {
        if (resultptr->get_tag()->get_type() == "number") {
            e = resultptr->get_data()->to_Number();
            if (!found) {found = true; n = e;} else {
                if (n > e) {
                    tree->unlink_child_results(); return false;
                } else {n = e;}
            }
        } else {
            tree->unlink_child_results();
            tree->warn("ERROR: Only numbers allowed in leq.\n");
            return false;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (!found) {tree->warn("WARNING: Nothing here in leq.\n");}
    return true;
}

bool finalize_geq(DataTree *tree) {
    tree->shortcut_finalize = &finalize_geq;
    DataTree *resultptr = tree->link_child_results();
    if (resultptr == tree) {return true; /*No child results found */}
    bool state,continues;
    Number e,n ;
    bool found = false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}
    while(continues) {
        if (resultptr->get_tag()->get_type() == "number") {
            e = resultptr->get_data()->to_Number();
            if (!found) {found = true; n = e;} else {
                if (n < e) {
                    tree->unlink_child_results(); return false;
                } else {n = e;}
            }
        } else {
            tree->unlink_child_results();
            tree->warn("ERROR: Only numbers allowed in geq.\n");
            return false;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (!found) {tree->warn("WARNING: Nothing here in geq.\n");}
    return true;
}



bool finalize_equal(DataTree *tree) {
    tree->shortcut_finalize = &finalize_equal;
    DataTree *resultptr = tree->link_child_results();
    if (resultptr == tree) {return true; /*No child results found */}
    bool state,continues;
    DataTree *first_result;
    bool found = false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}
    while(continues) {
        if (!found) {found = true; first_result = resultptr;} else {
            if (!first_result->test_value_and_tag_equality(resultptr)) {
                tree->unlink_child_results();
                return false;}
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (!found) {tree->warn("WARNING: Nothing here in equal.\n");}
    return true;
}

bool finalize_unequal(DataTree *tree) {
    tree->shortcut_finalize = &finalize_unequal;
    DataTree *resultptr = tree->link_child_results();
    DataTree *secondptr;
    if (resultptr == tree) {return true; /*No child results found */}
    bool state,continues;
    bool state2,continues2;
    DataTree *first_result;
    bool found = false;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); return true; /* It was bunches all the way down;*/}
    while(continues) {
        found = true;
        secondptr = resultptr; state2 = state; continues2 = continues;
        while (continues2) {
            if ((secondptr != resultptr)&&(resultptr->test_value_and_tag_equality(secondptr))) {
                tree->unlink_child_results();
                return false;
            }
            
            secondptr = secondptr->bunch_step(state2,continues2);
        }
        
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (!found) {tree->warn("WARNING: Nothing here in equal.\n");}
    return true;
}

bool finalize_numerator(DataTree *tree) {
    tree->shortcut_finalize = &finalize_numerator;
    Data e;
    Number N;
    if ((tree->has_single_simple_child_result(e))&&(e.is_number())) {
        N = e.to_Number();
        tree->reset_result_tree()->get_data()->copy(Data(N.numerator));
        tree->get_result_tree()->reset_tag_type("number");
        return true;
    }
    
    tree->delete_result_tree();
    tree->warn("ERROR: Invalid <numerator> specification.\n");
    return false;
}
bool finalize_denominator(DataTree *tree) {
    tree->shortcut_finalize = &finalize_denominator;
    Data e;
    Number N;
    if ((tree->has_single_simple_child_result(e))&&(e.is_number())) {
        N = e.to_Number();
        tree->reset_result_tree()->get_data()->copy(Data(N.denominator));
        tree->get_result_tree()->reset_tag_type("number");
        return true;
    }
    
    tree->delete_result_tree();
    tree->warn("ERROR: Invalid <denominator> specification.\n");
    return false;
}

bool finalize_abs(DataTree *tree) {
    tree->shortcut_finalize = &finalize_abs;
    Data e;
    Number N;
    if ((tree->has_single_simple_child_result(e))&&(e.is_number())) {
        N = e.to_Number();
        if (N < Number(0)) {N = N * Number(-1);}
        tree->reset_result_tree()->get_data()->copy(Data(N));
        tree->get_result_tree()->reset_tag_type("number");
        return true;
    }

    tree->delete_result_tree();
    tree->warn("ERROR: Invalid <abs> specification.\n");
    return false;
}

/*
bool finalize_scoreold(DataTree *tree) {
    tree->shortcut_finalize = &finalize_score;
    Data e;
    int totscore = 0;
    Number N;
    if ((tree->has_single_simple_child_result(e))&&(e.is_number())) {
        N = e.to_Number();
        totscore = complexity(N.numerator) + complexity(N.denominator);
        tree->reset_result_tree()->get_data()->copy(Data(totscore));
        tree->get_result_tree()->reset_tag_type("number");
        return true;
    }
    tree->delete_result_tree();
    tree->warn("ERROR: Invalid <score> specification.\n");
    return false;
} */

bool finalize_latex(DataTree *tree) {
    tree->shortcut_finalize = &finalize_latex;
    Data e;
    Number N;
    string output = "";
    if ((tree->has_single_simple_child_result(e))&&(e.is_number())) {
        N = e.to_Number();
        if (N.denominator == 1) {
            output = ::to_string(N.numerator);
        } else {
            if (N.numerator < 0) {
                output = "-\\frac{" + ::to_string(-N.numerator) + "}{" + ::to_string(N.denominator) + "}";
            } else {
                output = "\\frac{" + ::to_string(N.numerator) + "}{" + ::to_string(N.denominator) + "}";
            }
        }
        tree->reset_result_tree()->get_data()->copy(Data(output));
        tree->get_result_tree()->reset_tag_type("string");
        return true;
    }
    
    tree->delete_result_tree();
    tree->warn("ERROR: Invalid <latex> specification--should be a single number.\n");
    return false;
}


bool finalize_number(DataTree *tree) {
    tree->shortcut_finalize = &finalize_number;
    if ((tree->has_result_tree())&&(tree->is_terminal())) {return true;} /* The work was already done earlier and cannot possibly change now */
    
    string content = tree->get_tag()->get_attribute("content");
    
    if (content == "") {
        Data e;
        if (tree->has_single_simple_child_result(e)) {
            if (e.is_number()) {
                tree->reset_result_tree()->get_data()->copy(e);
                tree->get_result_tree()->reset_tag_type("number");
                return true;
            } else if (e.is_string()) { content = e.value(); }
        }
        if (content=="") {
            if (tree->get_data()->is_number()) {
                tree->reset_result_tree()->get_data()->copy(*tree->get_data());
                tree->get_result_tree()->reset_tag_type("number");
                return true;
            }
        }
    }
    if (content == "") {
        tree->warn("ERROR: Invalid <number> specification.\n"); return false;
    }
    Data test;
    test.grab(content);
    if (test.is_number()) {tree->reset_result_tree()->get_data()->copy(test); tree->get_result_tree()->reset_tag_type("number"); return true;}
    tree->delete_result_tree();
    tree->warn("ERROR: Invalid <number> specification.\n");
    return false;
}




bool finalize_string(DataTree *tree) {
    tree->shortcut_finalize = &finalize_string;
    bool specified = false;
    if ((tree->has_result_tree())&&(tree->is_terminal())) {return true;} /* The work was already done earlier and cannot possibly change now */
    
    string content = tree->get_tag()->get_attribute("content");
    
    if (content == "") {
        Data e;
        if (tree->has_single_simple_child_result(e)) {
            if (e.is_string()) {
                tree->reset_result_tree()->get_data()->copy(e);
                tree->get_result_tree()->reset_tag_type("string");
                return true;
            } else if (e.is_number()) { content = e.value(); specified = true; }
        }
        if (!specified) {
            if (tree->get_data()->is_string()) {content = tree->get_data()->value();}
        }
    }
    /*
    if (content == "") {
        tree->warn("ERROR: Invalid <string> specification.\n"); return false;
    } */

    tree->reset_result_tree()->get_data()->copy(Data(content));
    tree->get_result_tree()->reset_tag_type("string");
    return true;
}



bool finalize_variable(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_variable;
    string namefromtag = datapt->get_tag()->get_attribute("name");
    string parenttype = "";
    if (namefromtag == "") {
        Data e;
        if (datapt->has_single_simple_child_result(e)) {
            namefromtag = e.value();
        }
    }
    if (namefromtag == "") {
        datapt->warn("ERROR: Invalid <variable> specification.\n"); return false;
    }
    parenttype = datapt->parent->get_tag()->get_type();
    if ((parenttype=="define")||(parenttype=="defined")||(parenttype=="undefined")) {
        if (datapt->parent->first_child()==datapt) {
            DataTree *results = datapt->reset_result_tree();
            results->get_data()->copy(Data(namefromtag));
            datapt->myname = ""; // Important so that these things don't get picked up in search results.
            results->reset_tag_type("string"); return true;
        }
    }
    datapt->myname = namefromtag; // RECENTLY ADDED;
    namefromtag = datapt->to_fullname(namefromtag);
    DataTree *prevptr, *exeptr = datapt; bool state=false; bool prevstate;
    do {
        prevptr = exeptr; prevstate = state;
        exeptr = exeptr->exe_step_backwards(state);
        if ((exeptr->fullname()==namefromtag)&&(exeptr->has_result_tree())) {
            datapt->reset_result_tree()->clone(exeptr->get_result_tree());
            return true;
        }
    } while ((prevptr != exeptr)||(!prevstate));
    datapt->warn("INFO: Undefined <variable> " + namefromtag + "\n");
    if (datapt->parent->get_tag()->get_type() != "oracle") { return false;}
    return true;
}

bool finalize_oracle(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_oracle;
    string namefromtag;
    //namefromtag = datapt->to_fullname(namefromtag);
    DataTree *child = datapt->chld_frst;
    DataTree *prevptr, *exeptr;
    bool state; bool prevstate;
    
    while(child != datapt) {
        namefromtag = child->fullname();
        if (namefromtag == "") {
            datapt->warn("ERROR: Cannot oracle nameless objects.\n");
            return false;
        }
        state = false; exeptr = datapt;
        do {
            prevptr = exeptr; prevstate = state;
            exeptr = exeptr->exe_step_backwards(state);
            if ((exeptr != child)&&(exeptr->fullname()==namefromtag)&&(exeptr->parent->get_tag()->get_type()=="oracle")&&(exeptr->parent != datapt)) {
                // Should copy the opposite way from a variable.
                exeptr->reset_result_tree()->clone(child->get_result_tree());
                // prevstate = true; prevptr = exeptr; // This terminates the search.
                // NEW: Communicates back to *all* oracles
            }
        } while ((prevptr != exeptr)||(!prevstate));
        //datapt->warn("WARNING: Unknown oracle " + namefromtag +"\n"); return false;
        child = child->sibl_next;
    }
    return true;
}

bool finalize_sort(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_sort;
    if ((datapt->sibl_prev == datapt->parent)||(!datapt->sibl_prev->has_result_tree())) {
        datapt->warn("ERROR: No previous result to sort.\n"); return false;
    }
    datapt->reset_result_tree()->clone(datapt->sibl_prev->get_result_tree());
    datapt->sibl_prev->delete_result_tree();
    DataTree *child1, *child2;
    int i,j, total_children = datapt->get_result_tree()->total_children();
    int result;
    if (total_children < 2) {return true;}
    for(i=0; i < total_children; i++) {
        for(j=i+1; j < total_children;j++) {
            child1 = datapt->get_result_tree()->child(i);
            child2 = datapt->get_result_tree()->child(j);
            result = child1->test_value_and_tag_order(child2);
            
            
            if (result > 0) {
                datapt->get_result_tree()->swap_children(i,j);
                
            }
        }
    }
    return true;
}

bool finalize_defined(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_defined;
    string namefromtag = datapt->get_tag()->get_attribute("name");
    string parenttype = "";
    if (namefromtag == "") {
        Data e;
        if (datapt->has_single_simple_child_result(e)) {
            namefromtag = e.value();
        }
    }
    if (namefromtag == "") {
        datapt->warn("ERROR: Invalid <defined> specification.\n"); return false;
    }
    
    namefromtag = datapt->to_fullname(namefromtag);
    DataTree *prevptr, *exeptr = datapt; bool state=false; bool prevstate;
    do {
        prevptr = exeptr; prevstate = state;
        exeptr = exeptr->exe_step_backwards(state);
        if ((exeptr->fullname()==namefromtag)&&(exeptr->has_result_tree())) {
            return true;
        }
    } while ((prevptr != exeptr)||(!prevstate));
    return false;
}

bool finalize_undefined(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_undefined;
    string namefromtag = datapt->get_tag()->get_attribute("name");
    string parenttype = "";
    if (namefromtag == "") {
        Data e;
        if (datapt->has_single_simple_child_result(e)) {
            namefromtag = e.value();
        }
    }
    if (namefromtag == "") {
        datapt->warn("ERROR: Invalid <undefined> specification.\n"); return false;
    }
    
    namefromtag = datapt->to_fullname(namefromtag);
    DataTree *prevptr, *exeptr = datapt; bool state=false; bool prevstate;
    do {
        prevptr = exeptr; prevstate = state;
        exeptr = exeptr->exe_step_backwards(state);
        if ((exeptr->fullname()==namefromtag)&&(exeptr->has_result_tree())) {
            return false;
        }
    } while ((prevptr != exeptr)||(!prevstate));
    return true;
}

bool finalize_file(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_file;
    string namefromtag = datapt->get_tag()->get_attribute("name");
    if (namefromtag == "") {
        Data e;
        if (datapt->has_single_simple_child_result(e)) {
            namefromtag = e.value();
        }
    }
    if (namefromtag == "") {
        datapt->warn("ERROR: Invalid <file> specification.\n"); return false;
    }
    datapt->reset_result_tree()->get_data()->copy(Data(namefromtag));
    datapt->get_result_tree()->reset_tag_type("file");
    datapt->myname = ""; // Important to do so that it doesn't show in search results
    return true;
}

bool finalize_input(DataTree *datapt, IOHandler *theworld) {
    Data e;
    IORequest iorq;
    iorq.file="";
    iorq.make_get_request();
    if (datapt->has_single_simple_child_result(e,"file")) {
            iorq.file = e.value();
    } else {
        datapt->warn("ERROR: Invalid <input> specification.\n"); return false;
    }
    theworld->handle(iorq);
    datapt->reset_result_tree()->get_data()->copy(Data(iorq.message));
    datapt->get_result_tree()->reset_tag_type("string");
    return true;
}

bool finalize_ascii(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_ascii;
    int i;
    Data e;
    string result = "X";
    if ((datapt->has_single_simple_child_result(e))&&(e.is_number())&&(e.to_Number().is_integer())) {
        i = (int)e.to_Number();
        if ((i < 0)||(i>127)) {datapt->warn("ERROR: ASCII out of range.\n"); return false;}
        result[0] = (char)(i);
    } else {
        datapt->warn("ERROR: Invalid <ascii> specification.\n"); return false;
    }
    datapt->reset_result_tree()->get_data()->copy(Data(result));
    datapt->get_result_tree()->reset_tag_type("string");
    return true;
}

bool finalize_output(DataTree *datapt,IOHandler *world) {
    DataTree *resultptr = datapt->link_child_results();
    if (resultptr == datapt) {return true;}
    bool state,continues;
    IORequest iorq;
    iorq.file="";
    iorq.make_send_request();
    resultptr = resultptr->bunch_start(state,continues);
    while(continues) {
        if (resultptr->get_tag()->get_type() == "file") {
            iorq.file = resultptr->get_data()->value();
        } else {
            if (resultptr->is_terminal()) {
                iorq.message = resultptr->get_data()->value();
            } else {
                iorq.message = resultptr->debug();
            }
            if (iorq.file == "") {cout << iorq.message;} else {
                iorq.pending=true;
                world->handle(iorq);
            }
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    datapt->unlink_child_results();
    return true;
}

bool finalize_debug(DataTree *datapt) {
    /*
    DataTree *childptr = datapt->first_child();
    while(childptr != datapt) {
        if (childptr->has_result_tree()) {
            cout << childptr->get_result_tree()->debug();}
        else cout << "[[No DataTree]]\n";
        childptr = childptr->next_sibling();
    } */
    cout << datapt->list_child_results();
    bool result = finalize_all(datapt);
    datapt->shortcut_finalize = &finalize_debug; // finalize_all messes things up
    return result;
}

bool finalize_paren(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_paren;
    Data e;
        if (datapt->has_single_simple_child_result(e)) {
            datapt->reset_result_tree()->get_data()->copy(e);
            if (e.is_number()) {datapt->get_result_tree()->reset_tag_type("number");} else {datapt->get_result_tree()->reset_tag_type("string");}
            return true;
        }

    datapt->warn("ERROR: Invalid use of parentheses.\n"); return false;

    return true;
}

bool finalize_minus(DataTree *tree) {
    tree->shortcut_finalize = &finalize_minus;
    DataTree *resultptr = tree->link_child_results();
    DataTree *secondptr;
    if (resultptr == tree) {tree->warn("ERROR: Minus parsed without children.\n"); return false; /*No child results found */}
    bool state,continues;
    bool state2,continues2;
    DataTree *first_result;
    bool found = false,finished = false;
    Number first,second;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); tree->warn("ERROR: Minus parsed without children.\n"); return false; /* It was bunches all the way down;*/}
    while(continues) {
        if (!found) {
            if (resultptr->get_tag()->get_type()!="number") {
                tree->warn("ERROR: Only numbers allowed with minus.\n");
                tree->unlink_child_results();
                return false;
            }
            found = true; first = resultptr->get_data()->to_Number();
        } else {
            if (finished) {
                tree->warn("ERROR: Too many objects inside minus.\n");
                tree->unlink_child_results();
                return false;
            }
            if (resultptr->get_tag()->get_type()!="number") {
                tree->warn("ERROR: Only numbers allowed with minus.\n");
                tree->unlink_child_results();
                return false;
            }
            found = true; second = resultptr->get_data()->to_Number();
            finished = true;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (!found) {tree->warn("ERROR: Nothing here in minus.\n"); return false;}
    tree->reset_result_tree(); tree->get_result_tree()->reset_tag_type("number");
    if (!finished) {
        tree->get_result_tree()->get_data()->copy(Data(Number(-1)*first));
    } else {
        tree->get_result_tree()->get_data()->copy(Data(first-second));
    }
    return true;
}

bool finalize_divide(DataTree *tree) {
    tree->shortcut_finalize = &finalize_divide;
    DataTree *resultptr = tree->link_child_results();
    DataTree *secondptr;
    if (resultptr == tree) {tree->warn("ERROR: Divide parsed without children.\n"); return false; /*No child results found */}
    bool state,continues;
    bool state2,continues2;
    DataTree *first_result;
    bool found = false,finished = false;
    Number first,second;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); tree->warn("ERROR: Divide parsed without children.\n"); return false; /* It was bunches all the way down;*/}
    while(continues) {
        if (!found) {
            if (resultptr->get_tag()->get_type()!="number") {
                tree->warn("ERROR: Only numbers allowed with divide.\n");
                tree->unlink_child_results();
                return false;
            }
            found = true; first = resultptr->get_data()->to_Number();
        } else {
            if (finished) {
                tree->warn("ERROR: Too many objects inside divide.\n");
                tree->unlink_child_results();
                return false;
            }
            if (resultptr->get_tag()->get_type()!="number") {
                tree->warn("ERROR: Only numbers allowed with divide.\n");
                tree->unlink_child_results();
                return false;
            }
            found = true; second = resultptr->get_data()->to_Number();
            finished = true;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (!found) {tree->warn("ERROR: Nothing here in divide.\n"); return false;}

    if (!finished) {
        tree->warn("ERROR: Missing numerator or denominator in divide.\n"); return false;
    } else {
        tree->reset_result_tree();
        tree->get_result_tree()->get_data()->copy(Data(first/second));
        tree->get_result_tree()->reset_tag_type("number");
    }
    return true;
}

bool finalize_exponent(DataTree *tree) {
    tree->shortcut_finalize = &finalize_exponent;
    DataTree *resultptr = tree->link_child_results();
    DataTree *secondptr;
    if (resultptr == tree) {tree->warn("ERROR: Exponent parsed without children.\n"); return false; /*No child results found */}
    bool state,continues;
    bool state2,continues2;
    DataTree *first_result;
    bool found = false,finished = false;
    Number first,second;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); tree->warn("ERROR: Exponent parsed without children.\n"); return false; /* It was bunches all the way down;*/}
    while(continues) {
        if (!found) {
            if (resultptr->get_tag()->get_type()!="number") {
                tree->warn("ERROR: Only numbers allowed with exponent.\n");
                tree->unlink_child_results();
                return false;
            }
            found = true; first = resultptr->get_data()->to_Number();
        } else {
            if (finished) {
                tree->warn("ERROR: Too many objects inside exponent.\n");
                tree->unlink_child_results();
                return false;
            }
            if (resultptr->get_tag()->get_type()!="number") {
                tree->warn("ERROR: Only numbers allowed with exponent.\n");
                tree->unlink_child_results();
                return false;
            }
            found = true; second = resultptr->get_data()->to_Number();
            finished = true;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (!found) {tree->warn("ERROR: Nothing here in exponent.\n"); return false;}
    
    if (!finished) {
        tree->warn("ERROR: Missing numerator or denominator in exponent.\n"); return false;
    } else {
        tree->reset_result_tree();
        tree->get_result_tree()->get_data()->copy(Data(first^second));
        tree->get_result_tree()->reset_tag_type("number");
    }
    return true;
}

bool finalize_mod(DataTree *tree) {
    tree->shortcut_finalize = &finalize_mod;
    DataTree *resultptr = tree->link_child_results();
    DataTree *secondptr;
    if (resultptr == tree) {tree->warn("ERROR: Exponent parsed without children.\n"); return false; /*No child results found */}
    bool state,continues;
    bool state2,continues2;
    DataTree *first_result;
    bool found = false,finished = false;
    Number first,second;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); tree->warn("ERROR: Mod parsed without children.\n"); return false; /* It was bunches all the way down;*/}
    while(continues) {
        if (!found) {
            if (resultptr->get_tag()->get_type()!="number") {
                tree->warn("ERROR: Only numbers allowed with mod.\n");
                tree->unlink_child_results();
                return false;
            }
            found = true; first = resultptr->get_data()->to_Number();
        } else {
            if (finished) {
                tree->warn("ERROR: Too many objects inside mod.\n");
                tree->unlink_child_results();
                return false;
            }
            if (resultptr->get_tag()->get_type()!="number") {
                tree->warn("ERROR: Only numbers allowed with mod.\n");
                tree->unlink_child_results();
                return false;
            }
            found = true; second = resultptr->get_data()->to_Number();
            finished = true;
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    if (!found) {tree->warn("ERROR: Nothing here in mod.\n"); return false;}
    
    if (!finished) {
        tree->warn("ERROR: Missing pice in mod.\n"); return false;
    } else {
        tree->reset_result_tree();
        tree->get_result_tree()->get_data()->copy(Data(first%second));
        tree->get_result_tree()->reset_tag_type("number");
    }
    return true;
}

bool finalize_define(DataTree *tree) {
    tree->shortcut_finalize = &finalize_define;
    DataTree *resultptr = tree->link_child_results();
    DataTree *secondptr;
    if (resultptr == tree) {tree->warn("ERROR: Define parsed without children.\n"); return false; /*No child results found */}
    bool state,continues;
    bool state2,continues2;
    DataTree *first_result;
    bool found = false,finished = false, third=false;
    string first;
    DataTree *second;
    
    resultptr = resultptr->bunch_start(state,continues);
    if (!continues) {tree->unlink_child_results(); tree->warn("ERROR: Define parsed without children.\n"); return false; /* It was bunches all the way down;*/}
    while(continues) {
        if (!found) {
            if (resultptr->get_tag()->get_type()!="string") {
                tree->warn("ERROR: First result in define must be a string\n");
                tree->unlink_child_results();
                return false;
            }
            found = true; first = resultptr->get_data()->value();
        } else if (!finished) {
            found = true; second = resultptr;
            finished = true;
        } else {
            if (!third) {
                tree->reset_result_tree(); tree->get_result_tree()->reset_tag_type("all");
                tree->get_result_tree()->clone_as_child(second);
                third = true;
            }
            tree->get_result_tree()->clone_as_child(resultptr);
        }
        resultptr = resultptr->bunch_step(state,continues);
    }
    tree->unlink_child_results();
    
    if(third) {tree->myname = first; return true;}
    
    if (!found) {tree->warn("ERROR: Nothing here in definition.\n"); return false;}
    
    if (!finished) {
        tree->warn("ERROR: Null definition.\n"); return false;
    } else {
        tree->reset_result_tree()->clone(second);
        tree->myname = first;
    }
    return true;
}


bool initialize_parse(DataTree *datapt) {
    string whattodo = "";
    bool result;
    if ((datapt->get_tag()->get_attribute("static")=="true")&&(!datapt->is_terminal())) {datapt->sibl_prev->delete_result_tree(); return true;}
    datapt->cleanup();
    if ((datapt->sibl_prev == datapt->parent)||(!datapt->sibl_prev->has_result_tree())||(!(datapt->sibl_prev->get_result_tree()->get_tag()->get_type()=="string"))) {
        
        datapt->warn("ERROR: No previous command result string found for <parse>\n");
        return false;
    }
    whattodo = datapt->sibl_prev->get_result_tree()->get_data()->value();
    result = DataTreeFromXML(datapt->add_child(),whattodo);
    datapt->sibl_prev->delete_result_tree();
    return result;
}

bool finalize_parse(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_parse;
    datapt->reset_result_tree()->clone(datapt);
    datapt->get_result_tree()->reset_tag_type("all");
    return true;
}


bool initialize_dostring(DataTree *datapt) {
    string whattodo = "";
    bool result;
    if ((datapt->get_tag()->get_attribute("static")=="true")&&(!datapt->is_terminal())) {datapt->sibl_prev->delete_result_tree(); return true;}
    datapt->cleanup();
    if ((datapt->sibl_prev == datapt->parent)||(!datapt->sibl_prev->has_result_tree())||(!(datapt->sibl_prev->get_result_tree()->get_tag()->get_type()=="string"))) {
        
        datapt->warn("ERROR: No previous command result string found for <dostring>\n");
        return false;
    }
    whattodo = datapt->sibl_prev->get_result_tree()->get_data()->value();
    result = DataTreeFromXML(datapt->add_child(),whattodo);
    // 1/26 : RECENTLY CHANGED ABOVE FROM BELOW.
    /*
    DataTree *tester = new DataTree;
    result = DataTreeFromXML(tester,whattodo);
    if (result) {
        datapt->add_child()->clone(tester);
        tester->cleanup(); delete tester;
    } */
    datapt->sibl_prev->delete_result_tree();
    //cout << "MSSG: initialize_dostring:\n";
    //cout << datapt->first_child()->debug();
    return result;
}

bool finalize_dostring(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_dostring;
    DataTree *children = datapt->first_child();
    if (children == datapt) {return true;}
    datapt->reset_result_tree();
    datapt->get_result_tree()->reset_tag_type("all");
    while(children != datapt) {
        if (children->has_result_tree()) {
            datapt->get_result_tree()->clone_as_child(children->get_result_tree());
        }
        children = children->sibl_next;
    }
    if (datapt->get_result_tree()->is_terminal()) {datapt->delete_result_tree();}
    return true;
}

bool initialize_subroutine(DataTree *datapt) {
    DataTree *children = datapt->first_child();
    if (children==datapt) return true;
    datapt->reset_result_tree();
    while(children != datapt) {
        datapt->get_result_tree()->clone_as_child(children);
        children = children->sibl_next;
    }
    datapt->get_result_tree()->reset_tag_type("all");
    datapt->get_result_tree()->myname = "";
    datapt->exe_inside_frst = datapt; datapt->exe_inside_last = datapt;
    return true;
}

bool initialize_dosubroutine(DataTree *datapt) {
    bool result;
    if ((datapt->get_tag()->get_attribute("static")=="true")&&(!datapt->is_terminal())) {datapt->sibl_prev->delete_result_tree(); return true;}
    datapt->cleanup();
    if ((datapt->sibl_prev == datapt->parent)||(!datapt->sibl_prev->has_result_tree())) {
        
        datapt->warn("ERROR: No previous command result found for <dosubroutine>\n");
        return false;
    }
    datapt->clone_as_child(datapt->sibl_prev->get_result_tree());
    datapt->sibl_prev->delete_result_tree();
    return true;
}

bool finalize_dosubroutine(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_dosubroutine;
    if (datapt->first_child()->has_result_tree()) {
        datapt->reset_result_tree()->clone(datapt->first_child()->get_result_tree());
    }
    return true;
}

string to_python_list(DataTree *datapt) {
    DataTree *kid = datapt->first_child();
    string outstring = "[";
    string tempstring;
    if (kid == datapt) return kid->get_data()->to_common_string();
    while(kid != datapt) {
        if (!kid->is_terminal()) {
            tempstring = to_python_list(kid);
        } else {
            tempstring = kid->get_data()->to_common_string();
        }
        if (outstring != "[") outstring += ",  ";
        outstring += tempstring;
        kid = kid->sibl_next;
    }
    return outstring + "]";
}

bool finalize_catalog(DataTree *datapt) {
    datapt->shortcut_finalize = &finalize_catalog;
    if (datapt->get_tag()->get_attribute("python")!="true") {
        datapt->reset_result_tree()->get_data()->copy(Data(datapt->list_child_results()));
        datapt->get_result_tree()->reset_tag_type("string");
        return true;
    }
    
    DataTree *children = datapt->first_child();
    DataTree *result_child;
    if (children == datapt) {return true;}
    datapt->reset_result_tree();
    datapt->get_result_tree()->reset_tag_type("string");
    string listresults  = "{", thisresult = "";
    while(children != datapt) {
        if (children->has_result_tree()) {
            thisresult = to_python_list(children->get_result_tree());
            thisresult = '"' + children->get_tag()->get_attribute("name") + "\" : " + thisresult;
            if (listresults != "{") {listresults += ",  ";}
            listresults += thisresult;
 /*           if (children->get_result_tree()->is_terminal()) {
                thisresult =children->get_result_tree()->get_data()->to_common_string();
                thisresult = '"' + children->get_tag()->get_attribute("name") + "\" : " + thisresult;
                if (listresults != "{") {listresults += ",  ";}
                listresults += thisresult;
            } else {
                thisresult = "[";
                result_child = children->get_result_tree()->first_child();
                while (result_child != children->get_result_tree()) {
                    if (thisresult != "[") {thisresult += ", ";}
                    thisresult += result_child->get_data()->to_common_string();
                    result_child = result_child ->sibl_next;
                }
                thisresult = '"' + children->get_tag()->get_attribute("name") + "\" : " + thisresult + "]";
                if (listresults != "{") {listresults += ",  ";}
                listresults += thisresult;
            } */
        }
        children = children->sibl_next;
    }
    listresults += "}";
    datapt->reset_result_tree()->get_data()->copy(Data(listresults));
    datapt->get_result_tree()->reset_tag_type("string");
    
    return true;
}


    


#endif /* treesroutines_h */
