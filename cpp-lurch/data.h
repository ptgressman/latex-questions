//
//  words.h
//  
//
//  Created by Philip Gressman on 12/14/18.
//

#ifndef data_h
#define data_h

#include "iohandler.h"
#include "integers.h"

// -std=c++11


class Data {
public:
    Data(){myname = ""; numerical = Number(0); content = ""; isname = false; isstring = false; isnumber = false;}
    Data(string s) {myname = ""; numerical = Number(0); content = s;  isstring = true; isnumber = false; isname = false;}
    Data(Number n) {numerical = n; content = ""; myname = ""; isstring = false; isnumber = true; isname = false;}
    int grab(string inflow);
    bool is_null() {return !(isnumber || isstring || isname);}
    bool is_defined() {return (isnumber || isstring || isname);}
    bool is_string() {return isstring;}
    bool is_number() {return isnumber;}
    bool is_name() {return isname;}

    bool operator==(Data d) {
        if (isstring&&d.isstring) { return (content==d.content);}
        if (isnumber&&d.isnumber) { return (numerical==d.numerical);}
        if (isname&&d.isname) {return (myname==d.myname);}
        if (is_null()&&d.is_null()) {return true;}
        return false;
    }
    
    bool operator<=(Data d) {
        if (!isnumber&&!isstring) return false;
        if (isnumber && d.isstring) return true;
        if (isnumber && d.isnumber) {return (numerical < d.numerical) || (numerical == d.numerical);}
        if (isstring && d.isstring) {return content <= d.content;}
        return true;
    }
    
    Data operator+(Data d) {
        if (isnumber && d.isnumber) {return Data(numerical+d.numerical);}
        return Data(value()+d.value());
    }
    string value() {
        if (isstring) {return content;} else if (isnumber) {return numerical.to_string();} else if (isname) {return myname;} else { return "[[Undefined]]"; }}
    void copy(Data d) {numerical = d.numerical; content = d.content; isstring = d.isstring; isnumber = d.isnumber; myname = d.myname; isname = d.isname;}
    Number to_Number() {return numerical;}
    string to_string() {return content;}
    string to_name() {return myname;}
    string type() {if (isnumber) {return "number";} else if (isstring) {return "string";} else if (isname) {return "variable";} else {return "data";}}
    string to_common_string(bool usequotes=true) {
        string special = "";
        if (usequotes) special = "\"";
        if (isstring) return special + content + special;
        if (isnumber) return numerical.to_string();
        if (isname) return special + myname + special;
        return "[unknown]";
    }
protected:
    Number numerical;
    string content;
    string myname;
    bool isstring, isnumber, isname;
};


int Data::grab(string inflow) {
    int pos = 0, length = inflow.length();
    int start,i,numvalue;
    while((pos < length)&&(isspace(inflow[pos]))) {pos++;}
    if (pos == length) {
        numerical = Number(0); content = ""; isstring = false; isnumber = false;
        myname = ""; isname = false;
        return 0;
    }
    if (inflow[pos] == '"') {
        pos++; start = pos; while((pos < length)&&(inflow[pos]!='"')) {pos++;}
        if (pos == length) {
            numerical = Number(0); content = ""; isstring = false; isnumber = false;
            myname = ""; isname = false;
            return 0;
        }
        content = inflow.substr(start,pos-start);
        isstring = true; isnumber = false; isname = false; return pos+1;
    }
    myname = "";
    while ((pos < length)&&((inflow[pos]=='_')||(inflow[pos] == '@')||(inflow[pos] == '\\')||((inflow[pos] >= '0')&&(inflow[pos] <='9'))||((inflow[pos] >= 'A')&&(inflow[pos] <='Z'))||((inflow[pos] >= 'a')&&(inflow[pos] <='z'))))
    {
        myname += inflow[pos];
        pos++;}
    if (myname != "") {
        isnumber = true;
        if ((sizeof(int)!=4)&&(myname.length() > 4)) {isnumber = false;}
        if ((sizeof(int)==4)&&(myname.length() > 9)) {isnumber = false;}
        if (isnumber) {
            numvalue = 0;
            for(i=0;i<myname.length();i++) {
                if ((myname[i] < '0')||(myname[i] > '9')) {isnumber = false; i = myname.length();} else {numvalue = 10 * numvalue + (int)(myname[i]) - (int)('0');}
            }
        }
        if (isnumber) { numerical = Number(numvalue); isname = false; isstring = false; content = ""; myname = "";} else { isname = true; isstring = false; isnumber = false; content = ""; numerical = Number(0);}
         return pos;
    }
    myname = ""; numerical = Number(0); content = ""; isname = false; isstring = false; isnumber = false; return 0;
}



#endif /* data_h */
