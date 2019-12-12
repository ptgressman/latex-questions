//
//  words.h
//  
//
//  Created by Philip Gressman on 12/14/18.
//

#ifndef trees_h
#define trees_h

#include "iohandler.h"
#include "integers.h"
#include "data.h"
// -std=c++11



class Tag {
public:
    Tag(string conten="") {name = ""; content = ""; mytag = conten; validated = is_valid(); }
    bool set_tag(string content) {mytag = content; return is_valid();}
    string get_type() {return type;}
    string get_err_msg() {return err_msg;}
    string open_tag_written(string altname="") {
        string output = "<" + type;
        if (altname != "") {output = output + " name=\"" + altname + "\"";}
        else if (name != "") {output = output + " name=\"" + name + "\"";}
        if (content != "") {output = output + " content=\"" + content + "\"";}
        output = output + myattr + ">";
        return output;
    }
    string close_tag_written() {return "</" + type + ">";}
    string clopen_tag_written() {return "<" + type + myattr + "/>";}
    void add_attribute(string attr, string val) {myattr = " " + attr + "=\"" + val + "\"" + myattr;}
    bool is_valid();
    bool is_open() {return open;}
    bool is_close() {return close;}
    bool is_clopen() {return (open && close);}
    bool reset_tag_type(string tagtype) {type = tagtype; myattr = ""; open = true; close = false; name = ""; content=""; return true;}
    string get_attribute(string attr);
    string mytag;
    string myattr;
    string err_msg;
    string type;
    bool validated;
    bool open, close;
    string name, content;
};

string Tag::get_attribute(string attr) {
    int posn,posm;
    if (attr == "name") {return name;} if (attr == "content") {return content;}
    posn = myattr.find(attr);
    if ((posn < 0)||(posn >= myattr.length())) return "";
    posn = myattr.find("\"",posn);
    if ((posn < 0)||(posn >= myattr.length())) return "";
    posm = myattr.find("\"",posn+1);
    if ((posm < 0)||(posm >= myattr.length())) return "";

    return myattr.substr(posn+1,posm-posn-1);
}

bool Tag::is_valid() {
    int pos = 0,getstuff;
    int thingno = 0;
    int slashthing = -1,closething = -1, namething = -1;
    int attributes=0, equalsigns=0, values=0;
    string current_attr = "";
    err_msg = "";
    string grabbed;
    string wholetag = "";
    myattr = "";
    
    
    while((pos < mytag.length())&& isspace(mytag[pos])) {pos++;}
    if (pos == mytag.length()) {err_msg = "Empty tag."; return false;}
    
    if (mytag[pos] != '<') {err_msg = "Tag must begin with <"; return false;}
    wholetag = "<";
    getstuff = mytag.find("<!--");
    if ((getstuff >= 0)&&(getstuff <= mytag.length())&&(getstuff != pos)) {err_msg = "Errant <!--."; return false;}
    if (getstuff==pos) {
        getstuff = mytag.find("-->");
        if ((getstuff < 0)||(getstuff >= mytag.length())) {err_msg = "Missing -->."; return false;}
        pos = getstuff + 3;
        while((pos < mytag.length())&& isspace(mytag[pos])) {pos++;}
        if (pos != mytag.length()) {err_msg = "Unexpected characters after -->"; return false;}
        type = "XML Comment";
        return true;
    }
    
    getstuff = mytag.find("<?");
    if ((getstuff >= 0)&&(getstuff <= mytag.length())&&(getstuff != pos)) {err_msg = "Errant <?."; return false;}
    if (getstuff==pos) {
        getstuff = mytag.find("?>");
        if ((getstuff < 0)||(getstuff >= mytag.length())) {err_msg = "Missing ?>."; return false;}
        pos = getstuff + 2;
        while((pos < mytag.length())&& isspace(mytag[pos])) {pos++;}
        if (pos != mytag.length()) {err_msg = "Unexpected characters after ?>"; return false;}
        type = "XML Parser Command";
        return true;
    }
    
    
    thingno++; // thing 0 must be the opening bracket;
    pos++;
    
    while(pos < mytag.length()) {
        while((pos < mytag.length())&& isspace(mytag[pos])) {pos++;} // Ignore whitespace;
        
        if (mytag[pos] == '/') {
            wholetag = wholetag + "/";
            if (slashthing == -1) { slashthing = thingno;} else {err_msg = "Too many slashes /."; return false;}
            /* Important: record *where* slash appears but don't count it as a thing */
            pos++;
        } else if (mytag[pos]=='=') {
            wholetag = wholetag + "=";
            if ((current_attr != "name")&&(current_attr != "content")) {myattr = myattr + "=";}
            if (thingno % 3 != 0) {err_msg = "Unexpected equality."; return false;}
            thingno++; pos++; equalsigns++;
        } else if (mytag[pos]=='"') {
            wholetag = wholetag + "\""; grabbed = "";
            if ((thingno % 3 != 1)||(thingno < 4)) {err_msg = "Unexpected quote \"."; return false;}
            thingno++; pos++; while((pos < mytag.length())&&(mytag[pos] != '"')) {grabbed = grabbed + mytag[pos]; pos++;}
            if (pos == mytag.length()) {err_msg = "Missing closing quote \"."; return false;}
            values++;
            pos++;
            wholetag = wholetag + grabbed + "\"";
            if ((current_attr != "name")&&(current_attr != "content")) {myattr = myattr + "\"" + grabbed + "\"";} else if (current_attr == "name") {name = grabbed;} else if (current_attr == "content") {content = grabbed;}
        } else if (mytag[pos]=='>') {
            wholetag = wholetag + ">";
            if (closething < 0) {closething = thingno;} else {err_msg = "Too many closing >."; return false;}
            thingno++; pos++;
        } else if (((mytag[pos] >= '0')&&(mytag[pos]<='9'))||((mytag[pos] >= 'A')&&(mytag[pos]<='Z'))||((mytag[pos] >= 'a')&&(mytag[pos]<='z'))) {
            grabbed = "";
            while (((mytag[pos] >= '0')&&(mytag[pos]<='9'))||((mytag[pos] >= 'A')&&(mytag[pos]<='Z'))||((mytag[pos] >= 'a')&&(mytag[pos]<='z'))) {
                if ((mytag[pos] >= 'A')&&(mytag[pos] <='Z')) {grabbed += (char)(mytag[pos]+'a'-'A');} else {grabbed += mytag[pos];}
                pos++;
            }
            if (namething < 0) {wholetag = wholetag + grabbed;} else {
                wholetag = wholetag + " " + grabbed;
                current_attr = grabbed;
                if ((current_attr != "name")&&(current_attr != "content")) {
                    myattr = myattr + " " + grabbed;
                }
            }
            if (namething < 0) {namething = thingno;
                /*starting = pos; */ type = grabbed;
                if (namething != 1) {err_msg = "Name must be first nontrivial thing."; return false;}
            } else if (thingno % 3 != 2) { err_msg = "Unexpected attribute."; return false;} else if (thingno %3 ==2) {attributes++;}
            thingno++;
        } else {err_msg = "Unexpected objects in tag."; return false;}
    }
    
    if (closething <= 0) {err_msg = "Missing >."; return false;}
    if ((slashthing != -1)&&(slashthing != 1)&&(slashthing != closething)) {err_msg = "Errant slash /."; return false;}
    if ((attributes != equalsigns)||(equalsigns != values)) {err_msg =  "All attributes must have values."; return false;}
    mytag = wholetag;
    if (slashthing == 1) {open = false; close = true;}
    else if (slashthing > 1) {open = true; close = true;} else {open = true; close = false;}
    
    return true;
}


// "this" is supposed to be the first and last thing you see when stepping through children along sibling lines.

class DataTree {
public:
    DataTree(string nameit="") {parent = this; sibl_prev = this; sibl_next = this; chld_frst = this; chld_last = this; exe_before = this; exe_after = this; exe_inside_frst = this; exe_inside_last = this; myname = nameit; mytag.set_tag(""); calculation_result = this; result_allocated = false; warning_issued = false; shortcut_finalize = NULL; }
    bool is_root() {return (parent == this); }
    bool is_terminal() {return (chld_frst == this);}
    bool has_one_child() {return ((chld_frst == chld_last)&&(chld_frst != this));}
    bool is_my_ancestor(DataTree *candidate) {
        DataTree *lineage = this;
        while (lineage != lineage->parent) {
            if (candidate == lineage) {return true;}
            lineage = lineage->parent;
        }
        if (candidate == lineage) {return true;}
        return false;
    }
    DataTree *first_child() {return chld_frst;}
    DataTree *next_sibling() {return sibl_next;}
    DataTree *add_child(string nameit="") {
        DataTree *child;
        try {
            child = new DataTree(nameit);
        } catch (std::bad_alloc& ba) {
            cout << "Allocation Failed.\n"; exit(-1);
        }
        DataTree *old_last_child = chld_last;
        if (chld_last != this) {
            chld_last = child;
            child->sibl_prev = old_last_child; old_last_child->sibl_next = child;
            child->parent = this; child->sibl_next = this;
        } else {
            chld_frst = child; chld_last = child;
            child->sibl_prev = this; child->parent = this; child->sibl_next = this;
        }
        child->world = world;
        return child;
    }
    bool remove_child(DataTree *child) {
        if ((child->parent != this)||(child == this)) {return false;}
        if (child == chld_frst) {chld_frst = child->sibl_next;}
        if (child == chld_last) {chld_last = child->sibl_prev;}
        if (child->sibl_next != this) {child->sibl_next->sibl_prev = child->sibl_prev;}
        if (child->sibl_prev != this) {child->sibl_prev->sibl_next = child->sibl_next;}
        child->cleanup();  delete child;
        return true;
    }
    bool remove_subtree(DataTree *subtree) {
        if (subtree ==  this) {return false;}
        if (!subtree->is_my_ancestor(this)) {return false;}
        DataTree *theparent = subtree->parent;
        bool result = theparent->remove_child(subtree);
        return result;
    }
    void clone_payload(DataTree *original) {
        myname = original->myname; mytag = original->mytag;
        mydata = original->mydata;
    }
    DataTree *clone_as_child(DataTree *original) {
        if (this->is_my_ancestor(original)) return this;
        DataTree *child = this->add_child();
        DataTree *ochild = original->chld_frst;
        while(ochild != original) {
            child->clone_as_child(ochild);
            ochild = ochild->sibl_next;
        }
        child->clone_payload(original);
        return child;
    }
    void clone(DataTree *original) {
        DataTree *ochild = original->chld_frst;
        while(ochild != original) {clone_as_child(ochild); ochild = ochild->sibl_next;}
        clone_payload(original);
    }
    DataTree *child(int i) {
        DataTree *whichchild = chld_frst;
        if ((i < 0)||(chld_frst==this)) {return this;}
        while(i>0) {
            whichchild = whichchild->sibl_next; i--;
            if (whichchild == this) {return this;}
        }
        return whichchild;
    }
    int total_children() {
        int ret = 0;
        DataTree *kid = chld_frst;
        while(kid != this) {ret++; kid = kid->sibl_next;}
        return ret;
    }
    void swap_children(int i, int j) {
        if (i == j) return;
        DataTree *kid = chld_frst, *firstkid = this, *secondkid = this;
        DataTree *firstprev, *secondprev, *firstnext, *secondnext;
        int counter = 0;
        while(kid != this) {
            if ((counter == i)||(counter==j)) {
                if (firstkid == this) {firstkid = kid;}
                else if (secondkid == this) {secondkid = kid; kid = chld_last;}
            }
            counter++; kid = kid->sibl_next;
        }
        if ((firstkid == this)||(secondkid == this)) {return;}
        firstprev = firstkid->sibl_prev; firstnext = firstkid->sibl_next;
        secondprev = secondkid->sibl_prev; secondnext = secondkid->sibl_next;
        if (firstnext == secondkid) {
            firstnext = firstkid; secondprev = secondkid;
        } else {
            firstnext->sibl_prev = secondkid;
            secondprev->sibl_next = firstkid;
        }
        firstkid->sibl_prev = secondprev; firstkid->sibl_next = secondnext;
        secondkid->sibl_prev = firstprev; secondkid->sibl_next = firstnext;
        if (chld_frst == firstkid) {
            chld_frst = secondkid;
        } else {
            firstprev->sibl_next = secondkid;
        }
        if (chld_last == secondkid) {
            chld_last = firstkid;
        } else {
            secondnext->sibl_prev = firstkid;
        }
    }
    DataTree *child(string nameof) {
        DataTree *whichchild = chld_last;
        if (chld_last==this) {return this;}
        while(whichchild != this) {
            if (whichchild->myname == nameof) {return whichchild;}
            whichchild = whichchild->sibl_prev;
        }
        return whichchild;
    }

    void cleanup() {
        DataTree *tchild, *child = chld_frst;
        while (child != this) {child->cleanup(); tchild = child->sibl_next; delete child;  child = tchild;}
        chld_frst = this; chld_last = this;
        //if (result_allocated) {
        if (calculation_result != this) {
            calculation_result->cleanup(); delete calculation_result; result_allocated = false;
        }
        calculation_result = this;
    }
    bool validate() {
        if ((chld_frst->sibl_prev != this)&&(chld_frst !=this)) {return false;}
        if ((chld_last->sibl_next != this)&&(chld_last !=this)) {return false;}
        DataTree *thekids = chld_frst;
        while(thekids != this) {if (!thekids->validate()) {return false;} thekids = thekids->sibl_next; }
        return true;
    }
    
    
    bool execute_all() {
        DataTree *executionptr = this;
        bool state = true; // Start at the openers
        bool prev_result = true; // Assume that the earlier results have all been successful
        do {
            if (state && prev_result) {prev_result = executionptr->running_initialize();}
            else if (state && !prev_result) {
                prev_result = executionptr->running_increment();
                if ((!prev_result)&&(executionptr==this)) {return false;}
            }
            else if (!state & prev_result) {
                prev_result = executionptr->running_finalize();
                if (prev_result&&(executionptr==this)) {return true;}
            }
            else {prev_result = executionptr->running_reenter();}
            if (prev_result) {executionptr = executionptr->exe_step_forwards(state);} else {
                executionptr = executionptr->exe_step_backwards(state);
            }
        } while (true);
    }
    
    
    bool running_initialize();
    bool running_increment();
    bool running_finalize();
    bool running_reenter();
    
    DataTree *link_child_results()
    {
        DataTree *chldptr = this->chld_frst;
        DataTree *firstresult=this, *prevresult, *nextresult;
        bool found = false;
        while (chldptr != this) {
            if (chldptr->has_result_tree()) {

                nextresult = chldptr->get_result_tree();
                if (!found) {found = true; firstresult = nextresult; prevresult = firstresult;}
                else { prevresult->sibl_next = nextresult;
                    nextresult->sibl_prev = prevresult;
                    prevresult = nextresult;
                }
            }
            
            chldptr = chldptr->sibl_next;
        }
        return firstresult;
    }
    void unlink_child_results()
    {
        DataTree *childnode = this->chld_frst;
        DataTree *resultnode;
        while (childnode != this) {
            if (childnode->has_result_tree()) {
                resultnode = childnode->get_result_tree();
                resultnode->sibl_prev = resultnode; resultnode->sibl_next = resultnode;
            }
            childnode = childnode->sibl_next;
        }
    }
    DataTree *bunch_start(bool &state, bool &continues) {state = true; continues = true; if (this->get_tag()->get_type()=="all") {return bunch_step(state,continues);} else {return this;}}
    DataTree *bunch_step(bool &state, bool &continues) {
        DataTree *next_one = this;
        /* continues = false means that all viable trees have *already* been passed out */
        if ((this->sibl_next == this)&&(!state)) {continues = false; return this;}
        while(continues) {
            if (next_one->get_tag()->get_type()=="all") {
                next_one = next_one->tree_step(true,state,continues);
            } else {next_one = next_one->tree_step(false,state,continues);}
            if (state && (next_one->get_tag()->get_type()!="all")) {return next_one;}
            // if (next_one->sibl_next == next_one) { state = false; continues = false; return next_one;}
        }
        return next_one;
    }
    DataTree *tree_step(bool allow_children, bool &state, bool &continues)
    { /* state = true is incoming from outside; state = false is upcoming from inside; */
        if (state) {
            if ((chld_frst != this)&&(allow_children)) {return chld_frst;}
            if (sibl_next != parent) {return sibl_next;}
            if (parent == this) {continues = false;}
            state = false; return parent;
        } else {
            if (sibl_next != parent) {state = true; return sibl_next;}
            if (parent == this) {continues = false;}
            return parent;
        }
    }
    
    bool has_single_simple_child_result(Data& content,string type="") {
        DataTree *childresults = this->link_child_results();
        bool state,continues;
        
        childresults = childresults->bunch_start(state,continues);
        if (!continues) {
            this->unlink_child_results(); return false;
        }
        
        content.copy(*childresults->get_data());
        
        childresults = childresults->bunch_step(state,continues);
        this->unlink_child_results();
        
        if ((type!="")&&(childresults->get_tag()->get_type()!=type)) {return false;}
        
        if (continues) {return false;}
        return true;
        
    }

    DataTree *reset_result_tree() {
        if (result_allocated) {
            result_allocated = true;
            calculation_result->cleanup();
            return calculation_result;
        } else {
            if (calculation_result == this) {
                calculation_result = new DataTree();
            } else {calculation_result->cleanup();}
            calculation_result->set_tag("<result>");
            calculation_result->myname = this->myname;
            result_allocated = true;
            return calculation_result;
        }
    }
    DataTree *get_result_tree() {return calculation_result;}
    bool has_result_tree() {return (result_allocated&&(calculation_result != this));}
    DataTree *delete_result_tree() {
        if (result_allocated) {
            result_allocated = false;
            calculation_result->cleanup();
            //delete calculation_result;
            //calculation_result = this;
        }
        return this;
    }
    
    bool test_value_and_tag_equality(DataTree *comparator) {
        DataTree *childpointer,*otherchildpointer;
        if (!((*this->get_data())==(*comparator->get_data()))) {return false;}
        if (this->get_tag()->get_type() != comparator->get_tag()->get_type()) {return false;}
        childpointer = this->first_child(); otherchildpointer = comparator->first_child();
        while((childpointer!=this)&&(otherchildpointer!=comparator)) {
            if (!childpointer->test_value_and_tag_equality(otherchildpointer)) {return false;}
            childpointer = childpointer->next_sibling();
            otherchildpointer = otherchildpointer->next_sibling();
        }
        if ((childpointer!=this)||(otherchildpointer!=comparator)) return false;
        return true;
    }
    
    int test_value_and_tag_order(DataTree *comparator) {
        DataTree *childpointer,*otherchildpointer;
        int result;
        if (this->get_tag()->get_type() < comparator->get_tag()->get_type()) {return -1;}
        if (this->get_tag()->get_type() > comparator->get_tag()->get_type()) { return 1;}
        if (this->is_terminal() && !comparator->is_terminal()) {return -1;}
        if ((!this->is_terminal()) && comparator->is_terminal()) {return 1;}
        if (this->is_terminal() && comparator->is_terminal()) {
            if (*this->get_data()==*comparator->get_data()) {return 0;}
            if (*this->get_data() <= *comparator->get_data()) {return -1;}
            return 1;
        }
        childpointer = this->first_child(); otherchildpointer = comparator->first_child();
        while((childpointer!=this)&&(otherchildpointer!=comparator)) {
            result = childpointer->test_value_and_tag_order(otherchildpointer);
            if (result != 0) {return result;}
            childpointer = childpointer->next_sibling();
            otherchildpointer = otherchildpointer->next_sibling();
        }
        if ((childpointer==this)&&(otherchildpointer!=comparator)) {return -1;}
        if ((childpointer!=this)&&(otherchildpointer==comparator)) {return 1;}
        return 0;
    }
    
    
    Data *get_data() {return &mydata;}
    void warn(string warning) {
        DataTree *ancestors = this;
        while(!ancestors->is_root()) {
            if (ancestors->warning_issued) {warning_issued = true;}
            ancestors = ancestors->parent;
        }
        if (!warning_issued && (mytag.get_attribute("silent") != "true")) {
            ancestors = this->parent;
            while(!ancestors->is_root()) {ancestors->warning_issued = true; ancestors = ancestors->parent;}
                cout << warning;
                cout << this->debug();
            warning_issued = true;
        }
    }
    bool reset_tag_type(string tagtype) {
        mytag.reset_tag_type(tagtype);
        return true;
    }
    bool set_tag(string tagdata) {
        if (!mytag.set_tag(tagdata)) {cout << "ERROR (Tag): " << mytag.get_err_msg() << "\n"; return false;}
        if (mytag.get_attribute("name") != "") {myname = mytag.get_attribute("name");} /* else if (myname != "") {mytag.add_attribute("name",myname);} */
        // string content = mytag.get_attribute("content");
        //if (content != "") {mydata.grab(content);} /* Really don't do this, I think. */
        return true;
    }
    Tag *get_tag() {return &mytag;}
    
    
    string namepath() {if (parent != this) {
        if (parent->myname != "") {return parent->fullname() + "\\";} else {return parent->namepath();}
    } return "\\";}
    string to_fullname(string locname) {if (locname[0] == '\\') {return locname;} if (locname == "") {return locname;} return (namepath()+locname);}
    string fullname() {return to_fullname(myname);}
    string allnames() {string output = ""; output = fullname() + "\n"; DataTree *children = chld_frst; while (children != this) {output += children->allnames(); children = children ->sibl_next;} return output;}
    string XMLFMT(int spaces = 0, string altname="") {
        DataTree *children;
        string output = "";
        string spacestring = "";
        string datastring = "";
        for(int i=0;i<spaces;i++) {spacestring = spacestring + " ";}
        children = chld_frst;
        if (mydata.is_null()) {datastring = "";} else if (mydata.is_string()) {datastring =
            "\"" + mydata.value() + "\"";} else {datastring = mydata.value();}
        if (children == this) {output = output + spacestring + mytag.open_tag_written(altname) + datastring + mytag.close_tag_written()  + "\n";} else {
         output = spacestring + mytag.open_tag_written(altname)  + datastring;
            output = output + "\n";
                while(children != this) {output = output + children->XMLFMT(spaces+2); children = children->sibl_next;}
                output = output + spacestring + mytag.close_tag_written()  + "\n";
        }
        return output;
    }
    string debug(bool state=true) {
        return XMLFMT();
    }
    string list_child_results() {
        string output="";
        output = "<all>\n";
        DataTree *children = this->chld_frst;
        while(children != this) {
            if (children->has_result_tree()) {
                output = output + children->get_result_tree()->XMLFMT(2,children->myname);
            }
            children = children->sibl_next;
        }
        output = output + "</all>\n";
        return output;
    }
    DataTree *parent;
    DataTree *sibl_prev, *sibl_next;
    DataTree *chld_frst, *chld_last;
    
    DataTree *exe_step_backwards(bool &state) {
        if ((!state)&&(this->exe_inside_last != this)) {return this->exe_inside_last;}
        if ((!state)&&(this->exe_inside_last == this)) {state = true; return this;}
        if (this->exe_before->exe_after == this) {state = false;}
        if (this->exe_before->exe_inside_frst==this) {state = true;}
        return this->exe_before;
    }
    DataTree *exe_step_forwards(bool &state) {
        if (state&&(this->exe_inside_frst!= this)) {return this->exe_inside_frst;}
        if (state&&(this->exe_inside_frst==this)) {state = false; return this;}
        if (this->exe_after->exe_before==this) {state = true;}
        if (this->exe_after->exe_inside_last == this) {state = false;}
        return this->exe_after;
    }
    
    void set_IOHandler(IOHandler *theone) {world = theone;}
    
    DataTree *exe_before, *exe_after;
    DataTree *exe_inside_frst, *exe_inside_last;
    bool exeresult, exe_state;
    bool warning_issued;
    /* Payload Stuff */
    string myname;
    Tag mytag;
    Data mydata;
    DataTree *calculation_result;
    bool result_allocated;
    IOHandler *world;
    bool (*shortcut_finalize)(DataTree *);
    /* End of Payload Stuff */
};



#endif /* data_h */
