//
//  conversion.cpp
//  
//
//  Created by Philip Gressman on 8/30/19.
//

#include <iostream>
using namespace std;
#include <cstdlib>
#include "nodedata.h"
#include "iohandler.h"

int VERBOSE = 5; // Goes from zero to 6.

string to_string_pad(int i)
{
    if ((i >= 0)&&(i<=999)) {
        char buffer[10];
        sprintf(buffer,"%03d",i);
        string result = buffer;
        return result;
    }
    return ::to_string(i);
}

int clip_environment(string raw, string title, int &startpos, int &nextpos, int endpos = -1) {
    size_t found1,found2,found3;
    string finder;
    finder = "\\begin{" + title + "}";
    found1 = raw.find(finder,startpos);

    if (found1 == ::string::npos) {
        return -1;
    }
    startpos = found1+finder.length();
    if ((startpos >= endpos)&&(endpos >= 0)) return -1;
    finder = "\\end{" + title + "}";
    found3 = raw.find(finder,found1);
    if (found3 == ::string::npos) {
        return -2;
    }
    if ((found3 >= endpos)&&(endpos >= 0)) return -2;
    
    finder = "\\begin{" + title + "}";
    found2 = raw.find(finder,found1+1);
    if ((found1 == ::string::npos)||(found2 < found3)) {
        return -3;
    }
    nextpos = found3;
    return 1;
}

string generic_error(string raw,int pos) {
    int i,linecount;
    linecount = 1;
    for(i=0;i<pos;i++) {if (raw[i] == '\n') linecount++;}
    return "ERROR Line " + ::to_string(linecount) + ":\n" + raw.substr(pos,100) + "\n";
}

string starting_option(string raw, int &startpos, int nextpos) {
    int i = startpos;
    int firstone = 0, tally = 0;
    int firstpos = 0;
    string result;
    for(i=startpos;i<nextpos;i++) {
        if (!isspace(raw[i])) {
            if ((firstone== 0)&&(raw[i] != '[')) {return "";}
            if (firstone==0) {firstone = 1; tally = 1; firstpos = i;}
            else {
                if (raw[i] == '[') {tally++;} else if (raw[i] == ']') {tally--;}
            }
            if (tally==0) {
                result = raw.substr(firstpos+1,i-firstpos-1);
                startpos = i+1;
                return result;
            }
        }
    }
    
    cerr << "ERROR: No closing bracked ] found.\n";
    cerr << generic_error(raw,firstpos);
    exit(-1);
    return "";
}

int preserve_earliest(int defaul,int latest)
{
    if ((defaul < 0)&&(latest < 0)) return -1;
    if (defaul < 0) return latest;
    if (defaul > latest) return latest;
    return defaul;
}

char nextnonspace(string raw,int pos) {
    if (pos >= raw.length()) return ' ';
    for(int i = pos;i<raw.length();i++) {
        if (!isspace(raw[i])) return raw[i];
    }
    return ' ';
}


int find_choice(string raw, int start, int end)
{
    int whereami;
    char whatsnext;
    whereami = raw.find("\\choice",start);
    if ((whereami < start)||(whereami > end)) {
        return -1;
    }
    whatsnext = nextnonspace(raw,whereami+7);
    if ((whatsnext != '[')&&(whatsnext != '{')) {
        if (whatsnext + 1 >= end) {
            if ((whereami < start)||(whereami > end)) {
                return -1;
            }
        }
        return find_choice(raw, whereami+ 1, end);
    }
    return whereami+7;
}

void pull_responses(treeobj *itempos, string raw, int start, int end,string questiontype)
{
    int whereami = 0;
    int choiceno = 0;
    int bracket,brace,bracketend,braceend,i,tally,cfind;
    bool foundcorrect;
    bool foundanswer = false;
    string respid;
    string content;
    
    whereami = find_choice(raw,start,end);
    if (whereami < 0) {cerr << "ERROR: No \\choice items found.\n"; exit(-1);}
    
    while(whereami > 0) {
        choiceno++;
        respid = "R" + to_string_pad(choiceno);
        
        foundcorrect = false;
        bracket = raw.find("[",whereami);
        if ((bracket >= start)&&(bracket <= end)) {
            tally = 0;
            for(i=bracket;i<raw.length();i++) {
                if (raw[i] == '[') {tally++;}
                if (raw[i] == ']') {tally--;}
                if (tally == 0) {bracketend = i; i = raw.length();}
            }
            if (tally !=0) {
                cerr << "ERROR: No closing bracket.\n" << generic_error(raw,bracket); exit(-1);
            }
            cfind = raw.find("correct",bracket);
            if ((cfind > bracket)&&(cfind<bracketend)) {foundcorrect = true;}
        } else {bracket = raw.length(); bracketend = raw.length();}
        brace = raw.find("{",whereami);
        tally = 0;
        for(i=brace;i<raw.length();i++) {
            if (raw[i] == '{') {tally++;}
            if (raw[i] == '}') {tally--;}
            if (tally == 0) {braceend = i; i = raw.length();}
        }
        if (tally !=0) {
            cerr << "ERROR: No closing braces.\n" << generic_error(raw,brace); exit(-1);
        }
        
        if (questiontype == "short_answer_question")
        {
            foundcorrect = true;
        } else {
            if (cfind >= brace) {foundcorrect = false;}
        }
        
        if (VERBOSE > 3) {
            cout << "RESPONSE " << respid;
            if (foundcorrect) cout << "*";
            cout << ":" << raw.substr(brace+1,braceend-brace-1) << "\n";
        }
        
        if (foundcorrect) {
            if ((foundanswer)&&(questiontype=="multiple_choice_question")) {cerr << "ERROR: Multiple choice does not admit multiple correct answers.\n" << generic_error(raw,whereami); exit(-1);}
            foundanswer = true;
        }
        
        content = raw.substr(brace+1,braceend-brace-1);
        itempos->record_response(respid,content,foundcorrect);
        
        
        whereami = find_choice(raw,whereami+1,end);
    }
    if (!foundanswer) {cerr << "ERROR: Must specify a correct answer.\n" << generic_error(raw,start); exit(-1);}
    if (VERBOSE > 2) cout << "ANSWER OPTIONS: " << choiceno << " in total.\n";
    
}

void pull_items(treeobj *bankpos, string raw, int start, int next)
{
    int result;
    int itemno = 0;
    int end = next;
    int dummys,dummye,dummyr;
    int response_start,response_end;
    int ismc, issa;
    int thingsfoundpos;
    treeobj *itempos;
    string type;
    string genfb_all,genfb_correct,genfb_incorrect;
    string fbcont0,fbcont1,fbcont2;
    string id, name;
    result = clip_environment(raw,"question",start,next,end);
    if (result <= 0) {cerr << "ERROR: No \\begin{question} found.\n";}
    while(result > 0) {
        itemno++; thingsfoundpos = next; // was -1
        id = "Q" + to_string_pad(itemno);
        name = starting_option(raw,start,next);
        if (name == "") {name = "Unnamed Question " + id;}
        if (VERBOSE > 1) {cout << "Processing Question " << id << ": " << name   << "\n";}
        
        // Determine Question Type
        issa = -1; ismc = -1;
        dummys = start; dummye = next;
        ismc = clip_environment(raw,"multiplechoice",dummys,dummye,next);
        if (ismc > 0) {
            thingsfoundpos = preserve_earliest(thingsfoundpos,dummys-8-14);
            response_start = dummys; response_end = dummye;
        }
        dummys = start; dummye = next;
        issa = clip_environment(raw,"shortanswer",dummys,dummye,next);
        if (issa > 0) {
            thingsfoundpos = preserve_earliest(thingsfoundpos,dummys-8-11);
            response_start = dummys; response_end = dummye;
        }
        
        if ((ismc <=0) && (issa <= 0)) {
            type = "file_upload_question";
        } else if ((ismc > 0)&&(issa > 0)) {
            cerr << "ERROR: Questions must contain exactly one multiplechoice or shortanswer environments.\n" << generic_error(raw,start); exit(-1);
        }
        
        if (ismc > 0) type = "multiple_choice_question";
        if (issa > 0) type = "short_answer_question";
        
        if (VERBOSE > 2) {cout << "TYPE: " << type << "\n";}
        
        itempos = bankpos->add_item(id,name,type);
        
        genfb_all = ""; genfb_correct =""; genfb_incorrect = "";
        dummys = start; dummye = next;
        dummyr = clip_environment(raw,"feedback",dummys,dummye,next); // general incorrect feedback
        if (dummyr > 0) {
            genfb_incorrect = "general_incorrect_fb";
            thingsfoundpos = preserve_earliest(thingsfoundpos,dummys-8-8);
            fbcont2 = raw.substr(dummys,dummye-dummys);
        }
        if ((VERBOSE > 3)&&(dummyr > 0)) {
            cout << "FOUND: Feedback for incorrect responses.\n";
            if (VERBOSE > 4) cout << fbcont2 << "\n";
        }
        dummys = start; dummye = next;
        dummyr = clip_environment(raw,"praise",dummys,dummye,next); // general correct feedback
        if (dummyr > 0) {
            genfb_correct = "general_correct_fb";
            thingsfoundpos = preserve_earliest(thingsfoundpos,dummys-8-6);
            fbcont1 = raw.substr(dummys,dummye-dummys);
        }
        if ((VERBOSE > 3)&&(dummyr > 0)) {
            cout << "FOUND: Feedback for correct responses.\n";
            if (VERBOSE > 4) cout << fbcont1 << "\n";
        }
        dummys = start; dummye = next;
        dummyr = clip_environment(raw,"comments",dummys,dummye,next); // general feedback
        if (dummyr > 0) {
            genfb_all = "general_fb";
            thingsfoundpos = preserve_earliest(thingsfoundpos,dummys-8-8);
            fbcont0 = raw.substr(dummys,dummye-dummys);
        }
        if ((VERBOSE > 3)&&(dummyr > 0)) {
            cout << "FOUND: Feedback general comments.\n";
            if (VERBOSE > 4) cout << fbcont0 << "\n";
        }
        
        if ((type == "file_upload_question")&&(genfb_all != "")) {
            if ((genfb_correct != "")||(genfb_incorrect != "")) {
                cerr << "ERROR: File upload questions cannot have comments for correct or incorrect responses.\n" << generic_error(raw,start); exit(-1);
            }
            itempos->prepare_for_responses(genfb_all,"","");
            itempos->record_itemfeedback(genfb_all,fbcont0);
            
        } else if (type != "file_upload_question") {
            itempos->prepare_for_responses(genfb_all,genfb_correct,genfb_incorrect);
            if (genfb_all != "") itempos->record_itemfeedback(genfb_all,fbcont0);
            if (genfb_correct != "") itempos->record_itemfeedback(genfb_correct,fbcont1);
            if (genfb_incorrect != "") itempos->record_itemfeedback(genfb_incorrect,fbcont2);
        }
        
        // Grab all the content inside the question tag that comes before answers or feedback
        itempos->record_material(raw.substr(start,thingsfoundpos-start));
        if (VERBOSE > 4) cout << "QUESTION CONTENT:\n" << raw.substr(start,thingsfoundpos-start) << "\n";
        
        if (type != "file_upload_question") pull_responses(itempos,raw,response_start,response_end,type);
        
        
        if (VERBOSE > 2) cout << "=====\n";
        start = next;
        result = clip_environment(raw,"question",start,next,end);
    }
    
    if (result == -2) {cerr << "ERROR: Closing \\end{questionbank} not found.\n" << generic_error(raw,start);}
    if (result == -3) {cerr << "ERROR: Improper nesting of questionbank environments.\n" << generic_error(raw,start);}
    
}



string parsing_routine(string raw)
{
   
    int start = 0, next = 0;
    int result;
    int bankno = 0;
    treeobj myquestiontree;
    treeobj *bankpos;
    string id, name;
    result = clip_environment(raw,"questionbank",start,next);
    if (result <= 0) {cerr << "ERROR: No \\begin{questionbank} found.\n";}
    myquestiontree.initialize();
    while(result > 0) {
        bankno++;
        id = "B" + to_string_pad(bankno);
        name = starting_option(raw,start,next);
        if (name == "") {name = "Unnamed Bank " + id;}
        if (VERBOSE > 0) {cout << "Processing Bank " << id << ": " << name   << "\n";}
        bankpos = myquestiontree.add_bank(id,name);
        pull_items(bankpos,raw,start,next);
        
        
        start = next;
        result = clip_environment(raw,"questionbank",start,next);
    }
    
    if (result == -2) {cerr << "ERROR: Closing \\end{questionbank} not found.\n" << generic_error(raw,start);}
    if (result == -3) {cerr << "ERROR: Improper nesting of questionbank environments.\n" << generic_error(raw,start);}
    
    return myquestiontree.to_string();
    
}


int verboselevel(char *options) {
    if (options[0] != '-') return -1;
    int i;
    VERBOSE = 0;
    for(i=0;options[i] != 0;i++) {
        if (options[i] == 'V') VERBOSE++;
    }
    return 1;
}

string manifest(string filename)
{
    string output = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    output = output + "<manifest identifier=\"autogenerated000\" xmlns=\"http://www.imsglobal.org/xsd/imsccv1p1/imscp_v1p1\" xmlns:lom=\"http://ltsc.ieee.org/xsd/imsccv1p1/LOM/resource\" xmlns:imsmd=\"http://www.imsglobal.org/xsd/imsmd_v1p2\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.imsglobal.org/xsd/imsccv1p1/imscp_v1p1 http://www.imsglobal.org/xsd/imscp_v1p1.xsd http://ltsc.ieee.org/xsd/imsccv1p1/LOM/resource http://www.imsglobal.org/profile/cc/ccv1p1/LOM/ccv1p1_lomresource_v1p0.xsd http://www.imsglobal.org/xsd/imsmd_v1p2 http://www.imsglobal.org/xsd/imsmd_v1p2p2.xsd\">\n";
    output = output + "  <metadata>\n";
    output = output + "    <schema>IMS Content</schema>\n";
    output = output + "    <schemaversion>1.1.3</schemaversion>\n";
    output = output + "  </metadata>\n";
    output = output + "  <organizations/>\n";
    output = output + "  <resources>\n";
    output = output + "    <resource identifier=\"latex2qticontent\" href=\"" + filename +"\" type=\"imsqti_xmlv1p2\">\n";
    output = output + "      <file href=\"" + filename +"\"/>\n"; // Here you would add more lines for local refs to images, etc.
    output = output + "    </resource>\n";
    output = output + "  </resources>\n";
    output = output + "</manifest>\n";
    return output;
}

int main(int argc, char * argv [])
{
    IOHandler world;
    IORequest getit;
    string outname = "autogenerated.xml";
    VERBOSE = 0;
    int foundopts;
    
    if (argc == 2) {
        getit.file = argv[1];
    } else if (argc == 3) {
        foundopts = verboselevel(argv[1]);
        if (foundopts < 0) {
            getit.file = argv[1];
            outname = argv[2];
        } else {
            getit.file = argv[2];
        }
    } else if (argc == 4) {
        foundopts = verboselevel(argv[1]);
        if (foundopts < 0) cout << "USAGE: makecanvas [-V through -VVVVV] bankfile.tex\n";
        getit.file = argv[2];
        outname = argv[3];
        
    } else {
        cout << "USAGE: makecanvas [-V through -VVVVV] bankfile.tex\n";
        return 0;
    }
    
    getit.make_get_request();
    world.handle(getit);
    
    string output;
    
    output = parsing_routine(LaTeX_global_preprocess(getit.message));
    output = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + output;
    
    getit.make_send_request();
    getit.file = outname;
    getit.message = output;
    
    world.handle(getit);

    getit.make_send_request();
    getit.file = "imsmanifest.xml";
    getit.message = manifest(outname);
    world.handle(getit);
    
    string command = "tar -czf canvasautogenbank.zip imsmanifest.xml ";
    command += outname;
    const char *cmd = command.c_str();
    system(cmd);

    
    
    return 0;
    
}
