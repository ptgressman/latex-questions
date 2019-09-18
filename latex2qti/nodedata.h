//
//  nodedata.h
//  
//
//  Created by Philip Gressman on 8/30/19.
//

#ifndef nodedata_h
#define nodedata_h
#include "latex.h"

class attributelist {
    public:
    attributelist() {name = ""; value = ""; first_attr = this; last_attr = this;}
    string to_string() {
        string result;
        attributelist *current;
        for(current = first_attr; current != this; current = current->last_attr) {
            result = result + " " + XMLescape(current->name) + "=\"" + XMLescape(current->value) + "\"";
        }
        return result;
    }
    void add_attribute(string setname, string setvalue) {
        attributelist *newguy = new attributelist;
        newguy->name = setname; newguy->value = setvalue;
        if (first_attr == this) {
            first_attr = newguy; last_attr = newguy;
            newguy->first_attr = this; newguy->last_attr = this;
            return;
        }
        newguy->last_attr = this;
        newguy->first_attr = last_attr;
        last_attr->last_attr = newguy;
        last_attr = newguy;
        return;
    }
    void destroy() {name = ""; value = "";}
    private:
    string name, value;
    attributelist *first_attr, *last_attr;
};

class treeobj {
    public:
    treeobj() {
        parent = this; sibling_left=this; sibling_right=this; child_first = this; child_last = this;
        type = ""; content="";
    }
    void delete_children() {
        treeobj *looper, *nextlooper;
        looper=get_first_child();
        while(looper != this) {
            nextlooper = looper->get_next_child();
            looper->delete_children(); looper->my_attributes.destroy(); delete looper;
            looper = nextlooper;
        }
        child_first = this; child_last = this;
    }
    bool is_terminal() {return this == child_first;}
    treeobj *new_child(string settype) {
        treeobj *newguy = new treeobj;
        newguy->type = settype;
        if (this->is_terminal()) {
            newguy->parent = this; newguy->sibling_left = this; newguy->sibling_right = this;
            child_first = newguy; child_last = newguy; return newguy;
        }
        child_last->sibling_right = newguy;
        newguy->sibling_left = child_last;
        newguy->sibling_right = this;
        child_last = newguy;
        return newguy;
    }
    treeobj *get_first_child() {return child_first;}
    treeobj *get_next_child() {return sibling_right;}
    treeobj *add_attribute(string sname, string svalue) {my_attributes.add_attribute(sname,svalue); return this;}
    treeobj *add_content(string setcontent) {content = content + setcontent; return this;}
    string to_string(string indent="") {
        string result = "";
        string starter = indent + "<" + type;
        string attrstr = my_attributes.to_string();
        string eol = "";
        if (attrstr != "") starter += attrstr;
        treeobj *looper;
        for(looper=get_first_child(); looper != this; looper = looper->get_next_child()) {
            result = result + looper->to_string(indent + "  ");
        }
        if ((result == "")&&(content == "")) {result = starter + "/>\n"; return result;}
        if (result == "") {result = starter + ">" + XMLescape(content) + "</" + type + ">\n"; return result;}
        if (content == "") {result = starter + ">\n" + result + indent + "</" + type + ">\n"; return result;}
        result = starter + ">\n" + indent + XMLescape(content) + "\n" + result + indent + "</" + type + ">\n";
        return result;
    }
    treeobj *set_type(string settype) {type = settype; return this;}
    treeobj *set_content(string setcontent) {content = setcontent; return this;}
    treeobj *get_node_type(string typesearch) {
        treeobj *looper, *candidate;
        for(looper=get_first_child(); looper != this; looper = looper->get_next_child()) {
            candidate = looper->get_node_type(typesearch);
            if (candidate->type == typesearch) return candidate;
        }
        return this;
    }
    treeobj *get_node_content(string typesearch) {
        treeobj *looper, *candidate;
        for(looper=get_first_child(); looper != this; looper = looper->get_next_child()) {
            candidate = looper->get_node_content(typesearch);
            if (candidate->content == typesearch) return candidate;
        }
        return this;
    }
    protected:
    treeobj *parent, *sibling_left, *sibling_right, *child_first, *child_last;
    string type,content;
    attributelist my_attributes;
    public:
    treeobj *initialize() {
        set_type("questestinterop")
        ->add_attribute("xmlns","http://www.imsglobal.org/xsd/ims_qtiasiv1p2")
        ->add_attribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance")
        ->add_attribute("xsi:schemaLocation","http://www.imsglobal.org/xsd/ims_qtiasiv1p2 http://www.imsglobal.org/xsd/ims_qtiasiv1p2p1.xsd");
        return this;
    }
    treeobj *add_bank(string bankid, string bankname) {
        treeobj *metadata, *result;
        result = this->new_child("objectbank");
        metadata = result->add_attribute("ident",bankid)
        ->new_child("qtimetadata")->new_child("qtimetadatafield");
        metadata->new_child("fieldlabel")->set_content("bank_title");
        metadata->new_child("fieldentry")->set_content(bankname);
        return result;
    }
    treeobj *add_item(string itemid, string itemname, string itemtype) {
        treeobj *working1, *working2, *working3, *result;
        result = this->new_child("item");
        result->add_attribute("ident",itemid)->add_attribute("title",itemname);
        working1 = result->new_child("itemmetadata")->new_child("qtimetadata");
        working2 = working1->new_child("qtimetadatafield");
        working2->new_child("fieldlabel")->add_content("question_type");
        working2->new_child("fieldentry")->add_content(itemtype);
        working2 = working1->new_child("qtimetadatafield");
        working2->new_child("fieldlabel")->add_content("points_possible");
        working2->new_child("fieldentry")->add_content("1.0");
        working2 = working1->new_child("qtimetadatafield");
        working2->new_child("fieldlabel")->add_content("original_answer_ids");
        working2->new_child("fieldentry");
        working3 = result->new_child("presentation");
        working3->new_child("material")->new_child("mattext")->add_attribute("texttype","text/html");
        result->new_child("resprocessing")->new_child("outcomes")->new_child("decvar")
        ->add_attribute("maxvalue","100")->add_attribute("minvalue","0")
        ->add_attribute("varname","SCORE")->add_attribute("vartype","Decimal");
        
        if (itemtype == "short_answer_question") {
            // working3 points to the <presentation> tag
            working3->new_child("response_str")->add_attribute("ident","response1")
            ->add_attribute("rcardinality","Single")
            ->new_child("render_fib")->new_child("response_label")
            ->add_attribute("ident","answer1")
            ->add_attribute("rshuffle","No");
        } else if (itemtype == "multiple_choice_question") {
            working3->new_child("response_lid")->add_attribute("ident","response1")
            ->add_attribute("rcardinality","Single")
            ->new_child("render_choice");
        }
        
        return result;
    }
    treeobj *record_answerid(string answerid) {
        treeobj *working;
        string currentids;
        working = this->get_node_content("original_answer_ids");
        if (working->content != "original_answer_ids") {cerr << "ERROR: original_answer_ids metafield not found.\n";}
        working = working->parent->get_node_type("fieldentry");
        if (working->type != "fieldentry") {cerr << "ERROR: original_answer_ids metafield not found.\n";}
        currentids = working->content;
        if (currentids == "") {working->content = answerid; return this;}
        currentids += "," + answerid;
        working->content = currentids;
        return this;
    }
    treeobj *record_material(string matstring) {
        treeobj *working;
        working = this->get_node_type("material");
        if (working->type != "material") {cerr << "ERROR: material field not found.\n";}
        working = working->get_node_type("mattext");
        if (working->type != "mattext") {cerr << "ERROR: mattext field not found.\n";}
        working->set_content(LaTeX2Canvas(matstring));
        return this;
    }
    treeobj *prepare_for_responses(string genfbid, string correctfbid, string wrongfbid)
    {
        treeobj *working, *working2;
        working = this->get_node_type("resprocessing");
        if (working->type != "resprocessing") {cerr << "ERROR: resprocessing field not found.\n";}
        if (genfbid != "") {
            working->new_child("respcondition")->add_attribute("continue","Yes")
            ->new_child("conditionvar")->new_child("other")->parent->parent->new_child("displayfeedback")
            ->add_attribute("feedbacktype","Response")->add_attribute("linkrefid",genfbid);
        }
        working2 = working->new_child("respcondition")->add_attribute("continue","No")
        ->new_child("conditionvar")->parent;
        working2->new_child("setvar")->add_attribute("action","Set")->add_attribute("varname","SCORE")->set_content("100");
        if (correctfbid != "") {
            working2->new_child("displayfeedback")
            ->add_attribute("feedbacktype","Response")->add_attribute("linkrefid",correctfbid);
        }
        if (wrongfbid != "") {
            working->new_child("respcondition")->add_attribute("continue","Yes")
            ->new_child("conditionvar")->new_child("other")->parent->parent->new_child("displayfeedback")
            ->add_attribute("feedbacktype","Response")->add_attribute("linkrefid",wrongfbid);
        }
        return this;
        
    }
    treeobj *record_itemfeedback(string idstr, string message) {
        this->new_child("itemfeedback")->add_attribute("ident",idstr)
        ->new_child("flow_mat")->new_child("material")
        ->new_child("mattext")->add_attribute("texttype","text/html")->set_content(LaTeX2Canvas(message));
        return this;
    }
    treeobj *record_response(string idstr, string message,bool correct) {
        treeobj *working, *working2, *working3;
        string currentids;
        working = this->get_node_content("question_type");
        if (working->content != "question_type") {cerr << "ERROR: question_type metafield not found.\n"; return this;}
        working = working->parent->get_node_type("fieldentry");
        if (working->type != "fieldentry") {cerr << "ERROR: question_type metafield not found.\n"; return this;}
        
        if (working->content == "multiple_choice_question") {
            if (correct) {
                working2 = this->get_node_type("setvar");
                if (working2->type != "setvar") {cerr << "ERROR: setvar field not found.\n"; return this;}
                working3 = working2->sibling_left->get_node_type("varequal");
                if (working3->type == "varequal") {cerr << "ERROR: Multiple choice questions can't have multiple correct answers. Option will not be included. \n"; return this;}
                working2->sibling_left->new_child("varequal")->add_attribute("respident","response1")->set_content(idstr);
            }
            working2 = this->get_node_type("render_choice");
            if (working2->type != "render_choice") {cerr << "ERROR: render_choice field not found.\n"; return this;}
            working2->new_child("response_label")->add_attribute("ident",idstr)
            ->new_child("material")->new_child("mattext")->add_attribute("texttype","text/html")
            ->set_content(LaTeX2Canvas(message));
            record_answerid(idstr);
        } else if (working->content == "short_answer_question") {
            if (!correct) {cerr << "ERROR: Short answer questions don't specify wrong answers. Option will not be included. \n"; return this;}
            working2 = this->get_node_type("setvar");
            if (working2->type != "setvar") {cerr << "ERROR: setvar field not found.\n"; return this;}
            working2->sibling_left->new_child("varequal")->add_attribute("respident","response1")->set_content(message);
            record_answerid(idstr);
        }
        return this;
    }
};




#endif /* nodedata_h */
