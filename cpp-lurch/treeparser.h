//
//  words.h
//  
//
//  Created by Philip Gressman on 12/14/18.
//

#ifndef treeparser_h
#define treeparser_h
#include "trees.h"
#include "treesroutines.h"
// -std=c++11


string optagstring(string operatorstring, int& taken)
{
    taken = 0;
    int pos = 0;
    while((pos < operatorstring.length())&&(isspace(operatorstring[pos]))) {pos++;}
    if (pos == operatorstring.length()) {return 0;}
    taken = pos+1;
    if (operatorstring[pos]=='+') {return "<sum precedence=\"1\" parsed=\"true\">";}
    if (operatorstring[pos]=='-') {return "<minus precedence=\"1\" parsed=\"true\">";}
    if (operatorstring[pos]=='*') {return "<prod precedence=\"2\" parsed=\"true\">";}
    if (operatorstring[pos]=='/') {return "<divide precedence=\"2\" parsed=\"true\">";}
    if (operatorstring[pos]=='^') {return "<exponent precedence=\"3\" parsed=\"true\">";}
    if (operatorstring[pos]=='%') {return "<mod precedence=\"1\" parsed=\"true\">";}
    if (operatorstring[pos]=='(') {return "<paren precedence=\"4\" parsed=\"true\">";}
    if (operatorstring[pos]==')') {return "</paren precedence=\"4\" parsed=\"true\">";}
    if (operatorstring.find("==")==pos) {taken = pos+2; return "<equal precedence=\"0\" parsed=\"true\">";}
    if (operatorstring.find("!=")==pos) {taken = pos+2; return "<unequal precedence=\"0\" parsed=\"true\">";}
    if (operatorstring[pos]=='=') {return "<define precedence=\"0\" parsed=\"true\">";}
    taken = 0;
    return "";
}



int noun_grab(string s,string& outtag)
{
    size_t pos,startpos=0;
    size_t len = s.length();
    string grabbed, err_msg;
    int i;
    pos = 0;
    while (isspace(s[pos])&&(pos < len)) {pos++;}
    startpos = pos;
    err_msg = "";
    bool isnumber = false;
    outtag = "";
    if (s[pos] == '"') {
        // grabbed = "\""; We're not grabbing the quote anymore.
        pos++;
        while ((s[pos] != '"')&&(pos < len)) {grabbed += s[pos]; pos++;}
        if ((pos == len)||(s[pos] != '"')) {err_msg += "ERROR (parsing): unmatched \" while parsing for nouns.\n";  return 0;}
        //grabbed += "\"";
        outtag = "<string content=\"" + grabbed + "\"/>";
        return pos+1;
    }
    
    grabbed = "";
    
    while ((s[pos]=='_')||(s[pos] == '@')||(s[pos] == '\\')||((s[pos] >= '0')&&(s[pos] <='9'))||((s[pos] >= 'A')&&(s[pos] <='Z'))||((s[pos] >= 'a')&&(s[pos] <='z'))) {grabbed += s[pos]; pos++;}
    if (grabbed != "") {
        isnumber = true;
        for(i=0;i<grabbed.length();i++) if ((grabbed[i] < '0')||(grabbed[i] > '9')) isnumber = false;
        if (isnumber) {
            outtag = "<number content=\"" + grabbed + "\"/>";
            return pos;
        }
        outtag = "<variable name=\"" + grabbed + "\"/>";
        return pos;
    }
    
    grabbed = "";
    pos = startpos;
    
    
    return 0;
    
    
    
}


bool parser_post(DataTree *root)
{
    bool state = true,continues = true;
    bool hasleft, hasright;
    bool isbinaryminus=false;
    DataTree *stepper = root;
    DataTree *ancestor;
    bool restart;
    while(continues) {
        restart = false;
        if ((stepper->get_tag()->get_attribute("parsed")=="true")&&(stepper->is_terminal())) {
            hasleft = false; hasright = false;
            if (stepper->sibl_prev != stepper->parent) {hasleft = true;}
            if (stepper->sibl_next != stepper->parent) {hasright = true;}

            if ((stepper->sibl_prev->sibl_prev != stepper->parent)&&(stepper->sibl_prev->sibl_prev->get_tag()->get_attribute("parsed")=="true")&&(stepper->sibl_prev->sibl_prev->get_tag()->get_type() != "paren")&&(stepper->sibl_prev->sibl_prev->get_tag()->get_attribute("precedence")[0] > stepper->get_tag()->get_attribute("precedence")[0])&&(stepper->sibl_prev->sibl_prev->total_children()==0)) {hasleft = false;}
            if ((stepper->sibl_next->sibl_next != stepper->parent)&&(stepper->sibl_next->sibl_next->get_tag()->get_attribute("parsed")=="true")&&(stepper->sibl_next->sibl_next->get_tag()->get_type() != "paren")&&(stepper->sibl_next->sibl_next->get_tag()->get_attribute("precedence")[0] > stepper->get_tag()->get_attribute("precedence")[0])&&(stepper->sibl_next->sibl_next->total_children()==0)) {hasright = false;}
            
            if (hasleft && (stepper->sibl_prev->get_tag()->get_attribute("parsed")=="true") && (stepper->sibl_prev->is_terminal())) {hasleft = false;}
            if (hasright && (stepper->sibl_next->get_tag()->get_attribute("parsed")=="true") && (stepper->sibl_next->is_terminal())) {hasright = false;}
 
            if ((stepper->get_tag()->get_type()=="minus")&&(stepper->get_tag()->get_attribute("binary")=="true")) {
                ancestor = stepper; isbinaryminus = false;
                while(!ancestor->is_root()&&(!isbinaryminus)) {
                    ancestor = ancestor->parent;
                    if (ancestor->get_tag()->get_type()=="paren") {isbinaryminus = true;}
                }
            } else {isbinaryminus = false;}

            if ((stepper->get_tag()->get_type()=="minus")&&(!isbinaryminus)) {
                if (hasright) {
                    stepper->clone_as_child(stepper->sibl_next);
                    stepper->parent->remove_child(stepper->sibl_next);
                    restart = true;
                }
            } else if (hasleft && hasright) {
                stepper->clone_as_child(stepper->sibl_prev);
                stepper->clone_as_child(stepper->sibl_next);
                stepper->parent->remove_child(stepper->sibl_prev);
                stepper->parent->remove_child(stepper->sibl_next);
                restart = true;
            }
            
        }
        if (!restart) {
            stepper = stepper->tree_step(true,state,continues);
        } else {
            state = true; continues = true; stepper = root;
        }
    }
    return true;
}

void ParserErrorDetails(string xml,int lineno, int linestart)
{
    cout << "On line " << lineno << ": ";
    int pos = 0; // Right now linestart remembers the position inside a string you don't have;
    while ((pos < xml.length())&&(xml[pos] != '\n')) {cout << xml[pos]; pos++;}
    cout << "\n";
}


bool DataTreeFromXML(DataTree *root, string xml, bool already_opened=false,int lineno=1, int linestart=0)
{
    int i,pos,pos2,oldlineno = lineno;
    string foundtag;
    Tag checkit;
    bool round_still_going;
    DataTree *newkid;
    DataTree *startingpoint = root->parent;
    while(true) {
        round_still_going = true;
        pos = 0;
        while((pos < xml.length())&&(isspace(xml[pos]))) {if (xml[pos] == '\n') {lineno++; linestart = lineno;} pos++;}
        if (pos == xml.length()) {
            if (root==startingpoint) {return parser_post(root);}
            cout << "PARSING ERROR: Couldn't find anything.\n";
            ParserErrorDetails(xml,lineno,linestart);
            return false;
        }
        
        if (xml[pos] != '<') {
            
            foundtag = optagstring(xml, i);
            
            if (i == 0) {
                foundtag = "";
                i = noun_grab(xml,foundtag);
            }
            if (i > 0) {
                if (!already_opened) {
                    cout << "PARSING ERROR: No parent tag open for children.\n";
                    ParserErrorDetails(xml,lineno,linestart);
                    return false;
                }
                if (!checkit.set_tag(foundtag)) {
                    cout << "PARSING ERROR: Invalid internal tag construction.\n";
                    ParserErrorDetails(xml,lineno,linestart);
                    return false;
                }
                
                if (checkit.get_attribute("parsed")!="true") {
                    newkid = root->add_child();
                    newkid->set_tag(foundtag); newkid->get_tag()->add_attribute("line",::to_string(lineno));
                    //return DataTreeFromXML(root,xml.substr(i,xml.length()-i),true,lineno,linestart);
                    xml = xml.substr(i,xml.length()-i); already_opened = true; oldlineno = lineno; round_still_going = false;
                } else {
                    if (checkit.get_type()=="paren") {
                        if (checkit.is_open()) {
                            newkid = root->add_child();
                            newkid->set_tag(foundtag);
                            newkid->get_tag()->add_attribute("line",::to_string(lineno));
                            //return DataTreeFromXML(newkid,xml.substr(i,xml.length()-i),true,lineno,linestart);
                            root = newkid; xml = xml.substr(i,xml.length()-i); already_opened = true; oldlineno = lineno;
                            round_still_going = false;
                        } else {
                            if ((root->is_root())||(root==startingpoint)) {
                                cout << "PARSING ERROR: Too many closing parentheses.\n";
                                ParserErrorDetails(xml,lineno,linestart);
                            }
                            //return DataTreeFromXML(root->parent,xml.substr(i,xml.length()-i),true,lineno,linestart);
                            root = root->parent; xml = xml.substr(i,xml.length()-i); already_opened = true; oldlineno = lineno;
                            round_still_going = false;
                        }
                    }
                    if (round_still_going) {
                        DataTree *newch = root->add_child();
                        newch->set_tag(foundtag);
                        newch->get_tag()->add_attribute("line",::to_string(lineno));
                        
                        if ((checkit.get_type()=="minus")&&(newch->sibl_prev != newch->parent)&&((newch->sibl_prev->get_tag()->get_type()=="paren")||(newch->sibl_prev->get_tag()->get_type()=="number")||(newch->sibl_prev->get_tag()->get_type()=="variable"))) {
                            newch->get_tag()->add_attribute("binary","true");
                        }
                        
                        //return DataTreeFromXML(root,xml.substr(i,xml.length()-i),true,lineno,linestart);
                        xml = xml.substr(i,xml.length()-i); already_opened = true;
                        oldlineno = lineno; round_still_going = false;
                    }
                }
            } else {
                cout << "PARSING ERROR: Unknown object.\n";
                ParserErrorDetails(xml,lineno,linestart);
                return false;
            }
            
        }
        if (round_still_going) {
            pos2 = pos;
            if (xml.find("<!--")==pos) {
                int secondpos = xml.find("-->");
                if ((secondpos < pos)||(secondpos > xml.length())) {
                    cout << "PARSING ERROR: Can't find comment closer.\n";
                    ParserErrorDetails(xml,lineno,linestart);
                    return false;
                }
                while ((pos2 <= secondpos)||(xml[pos2] != '>')) {if (xml[pos2] == '\n') {lineno++; linestart = lineno;} pos2++;}
                //return DataTreeFromXML(root,xml.substr(pos2+1,xml.length()-pos2-1),already_opened,lineno,linestart);
                xml = xml.substr(pos2+1,xml.length()-pos2-1);
                oldlineno = lineno; round_still_going = false;
            } else {
                while ((pos2 < xml.length())&&(xml[pos2] != '>')) {if (xml[pos2] == '\n') {lineno++; linestart = lineno;} pos2++;}
            }
        }
        if (round_still_going) {
            //pos2 = xml.find(">");
            
            if (pos2 == xml.length()) {
                cout << "PARSING ERROR: Tag does not close.\n";
                ParserErrorDetails(xml,lineno,linestart);
                return false;
            }
            foundtag = xml.substr(pos,pos2-pos+1);
            checkit.set_tag(foundtag);
            if (!already_opened) {
                if (!root->set_tag(foundtag)) {
                    cout << "PARSING ERROR: Malformed tag.\n";
                    ParserErrorDetails(xml,lineno,linestart);
                    return false;}
                root->get_tag()->add_attribute("line",::to_string(lineno));
                if (!root->mytag.is_open()) {
                    cout << "PARSING ERROR: Expected open tag.\n";
                    ParserErrorDetails(xml,lineno,linestart);
                    return false;}
                if (!root->mytag.is_close()) {
                    //return DataTreeFromXML(root,xml.substr(pos2+1,xml.length()-pos2-1),true,lineno,linestart);
                    xml = xml.substr(pos2+1,xml.length()-pos2-1); already_opened = true;
                    oldlineno = lineno;
                    round_still_going = false;
                } else if (root->mytag.is_close()) {/* Gotta be a clopen if you get here */
                    if ((root->is_root())||(root==startingpoint)) {
                        for(i = pos2+1;i<xml.length();i++) if (!isspace(xml[i])) {
                            cout << "PARSING ERROR: Extra material after final tag.\n";
                            ParserErrorDetails(xml,lineno,linestart);
                            return false;
                        }
                        return parser_post(root);
                    }
                    //return DataTreeFromXML(root->parent,xml.substr(pos2+1,xml.length()-pos2-1),true,lineno,linestart);
                    root = root->parent; xml = xml.substr(pos2+1,xml.length()-pos2-1);
                    already_opened = true; oldlineno = lineno;
                    round_still_going = false;
                }
            }
        }
        
        if (round_still_going) {
            if (checkit.is_open()) {
                // return DataTreeFromXML(root->add_child(),xml,false,oldlineno,linestart);
                lineno = oldlineno;
                root = root->add_child(); already_opened = false;
                round_still_going = false;
            }
        }
        if (round_still_going) {
            if (checkit.is_close()) {
                if (checkit.get_type() != root->get_tag()->get_type()) {
                    cout << "PARSING ERROR: Closing tag does not match opening.\n";
                    ParserErrorDetails(xml,lineno,linestart);
                    return false;
                }
                if ((root->is_root())||(root==startingpoint)) {
                    for(i = pos2+1;i<xml.length();i++) if (!isspace(xml[i])) {
                        cout << "PARSING ERROR: Unexpected material after close of main tag.\n";
                        ParserErrorDetails(xml,lineno,linestart);
                        return false;}
                    return parser_post(root);
                }
                //return DataTreeFromXML(root->parent,xml.substr(pos2+1,xml.length()-pos2-1),true,lineno,linestart);
                root = root->parent; xml = xml.substr(pos2+1,xml.length()-pos2-1); already_opened = true; oldlineno = lineno; round_still_going = false;
            }
        }
        if (round_still_going) {
            cout << "PARSING ERROR: Not sure what to do.\n";
            ParserErrorDetails(xml,lineno,linestart);
            return false;
        }
    }
    
}

/*
bool DataTreeFromXMLold(DataTree *root, string xml, bool already_opened=false,int lineno=1, int linestart=0)
{
    int i,pos,pos2,oldlineno = lineno;
    string foundtag;
    Tag checkit;
    
    pos = 0;
    while((pos < xml.length())&&(isspace(xml[pos]))) {if (xml[pos] == '\n') {lineno++; linestart = lineno;} pos++;}
    if (pos == xml.length()) {
        cout << "PARSING ERROR: Couldn't find anything.\n";
        ParserErrorDetails(xml,lineno,linestart);
        return false;
    }
    
    if (xml[pos] != '<') {
        
        foundtag = optagstring(xml, i);
        
        if (i == 0) {
            foundtag = "";
            i = noun_grab(xml,foundtag);
        }
        if (i > 0) {
            if (!already_opened) {
                cout << "PARSING ERROR: No parent tag open for children.\n";
                ParserErrorDetails(xml,lineno,linestart);
                return false;
            }
            if (!checkit.set_tag(foundtag)) {
                cout << "PARSING ERROR: Invalid internal tag construction.\n";
                ParserErrorDetails(xml,lineno,linestart);
                return false;
            }
            
            if (checkit.get_attribute("parsed")!="true") {
                root->add_child()->set_tag(foundtag);
                return DataTreeFromXML(root,xml.substr(i,xml.length()-i),true,lineno,linestart);
            } else {
                if (checkit.get_type()=="paren") {
                    if (checkit.is_open()) {
                        DataTree *newkid = root->add_child();
                        newkid->set_tag(foundtag);
                        return DataTreeFromXML(newkid,xml.substr(i,xml.length()-i),true,lineno,linestart);
                    } else {
                        if (root->is_root()) {
                            cout << "PARSING ERROR: Too many closing parentheses.\n";
                            ParserErrorDetails(xml,lineno,linestart);
                        }
                        return DataTreeFromXML(root->parent,xml.substr(i,xml.length()-i),true,lineno,linestart);
                    }
                }
                DataTree *newch = root->add_child();
                newch->set_tag(foundtag);
                
                if ((checkit.get_type()=="minus")&&(newch->sibl_prev != newch->parent)&&((newch->sibl_prev->get_tag()->get_type()=="paren")||(newch->sibl_prev->get_tag()->get_type()=="number")||(newch->sibl_prev->get_tag()->get_type()=="variable"))) {
                    newch->get_tag()->add_attribute("binary","true");
                }
                
                return DataTreeFromXML(root,xml.substr(i,xml.length()-i),true,lineno,linestart);
          
            }
        } else {
            cout << "PARSING ERROR: Unknown object.\n";
            ParserErrorDetails(xml,lineno,linestart);
            return false;
        }
        
    }
    pos2 = pos;
    if (xml.find("<!--")==pos) {
        int secondpos = xml.find("-->");
        if ((secondpos < pos)||(secondpos > xml.length())) {
            cout << "PARSING ERROR: Can't find comment closer.\n";
            ParserErrorDetails(xml,lineno,linestart);
            return false;
        }
        while ((pos2 <= secondpos)||(xml[pos2] != '>')) {if (xml[pos2] == '\n') {lineno++; linestart = lineno;} pos2++;}
        return DataTreeFromXML(root,xml.substr(pos2+1,xml.length()-pos2-1),already_opened,lineno,linestart);
    } else {
        while ((pos2 < xml.length())&&(xml[pos2] != '>')) {if (xml[pos2] == '\n') {lineno++; linestart = lineno;} pos2++;}
    }
    //pos2 = xml.find(">");

    if (pos2 == xml.length()) {
        cout << "PARSING ERROR: Tag does not close.\n";
        ParserErrorDetails(xml,lineno,linestart);
        return false;
    }
    foundtag = xml.substr(pos,pos2-pos+1);
    checkit.set_tag(foundtag);
    if (!already_opened) {
        if (!root->set_tag(foundtag)) {
            cout << "PARSING ERROR: Malformed tag.\n";
            ParserErrorDetails(xml,lineno,linestart);
            return false;}
        if (!root->mytag.is_open()) {
            cout << "PARSING ERROR: Expected open tag.\n";
            ParserErrorDetails(xml,lineno,linestart);
            return false;}
        if (!root->mytag.is_close()) {
            return DataTreeFromXML(root,xml.substr(pos2+1,xml.length()-pos2-1),true,lineno,linestart);}
        if (root->mytag.is_close()) {// Gotta be a clopen if you get here
            if (root->is_root()) {
                for(i = pos2+1;i<xml.length();i++) if (!isspace(xml[i])) {return false;}
                return parser_post(root);
            }
            return DataTreeFromXML(root->parent,xml.substr(pos2+1,xml.length()-pos2-1),true,lineno,linestart);
        }
    }
    
    
    
    if (checkit.is_open()) {
        return DataTreeFromXML(root->add_child(),xml,false,oldlineno,linestart);
    }
    
    if (checkit.is_close()) {
        if (checkit.get_type() != root->get_tag()->get_type()) {
            cout << "PARSING ERROR: Closing tag does not match opening.\n";
            ParserErrorDetails(xml,lineno,linestart);
            return false;
        }
        if (root->is_root()) {
            for(i = pos2+1;i<xml.length();i++) if (!isspace(xml[i])) {
                cout << "PARSING ERROR: Unexpected material after close of main tag.\n";
                ParserErrorDetails(xml,lineno,linestart);
                return false;}
            return parser_post(root);
        }
        return DataTreeFromXML(root->parent,xml.substr(pos2+1,xml.length()-pos2-1),true,lineno,linestart);
    }
    cout << "PARSING ERROR: Not sure what to do.\n";
    ParserErrorDetails(xml,lineno,linestart);
    return false;
    
    
}
*/





#endif /* data_h */
