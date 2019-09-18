#ifndef latex_h
#define latex_h
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

string cssoptions = "style=\"vertical-align:middle;\"";
string cssoptions_displayed = "style=\"vertical-align:middle;text-align:center;\"";

string findreplace(std::string s,const std::string& toReplace,const std::string& replaceWith)
{
    std::size_t pos = s.find(toReplace);
    if (pos == std::string::npos) return s;
    return s.replace(pos, toReplace.length(), replaceWith);
}



int min(int a, int b) {
    int v = a;
    if (b < v) v = b;
    return v;
}

int find_adjust(int a, int b) {
    if ((a < 0) || (a > b)) {return b;}
    return a;
}

size_t findbrace(string s, size_t start, size_t end, int parity)
{
    size_t open,close,where;
    int tally = 0;
    where = start;
    while (1 == 1) {
        open = s.find("{",where);
        close = s.find("}",where);
        if ((open > end)&&(close > end)) return ::string::npos;
        if (open < close) {tally++; where = open;}
        if (open > close) {tally--; where = close;}
        if (parity == tally) {return where;}
        where++;
    }
}

string LaTeX_suppress_comments(string line)
{
    string filedata = "";

    
        if (line.find("%") < line.length()) {
            filedata += line.substr(0,line.find("%"));
        } else {filedata += line;}

    return filedata;
    
}



string LaTeXmakedollarinert(string math)
{
    string result;
    string altmath = math;
    string mod = "";
    size_t where1, where2;
    // Very first thing you do to process individual lines of LaTeX. Make dangerous symbols safe to work with.
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(altmath,"&","@amp;"); // all ampersands become @amp;
    }
    
    mod = altmath + "E"; // Add the "E" as a throw-away so that they start different.
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"\\$","&#36;");
        altmath = findreplace(altmath,"\\@amp;","&CARElitamp;");
        altmath = findreplace(altmath,"\\#","&CAREhashtag;");
        altmath = findreplace(altmath,"\\%","&#37;");
        altmath = findreplace(altmath,"\\{","&#123;");
        altmath = findreplace(altmath,"\\}","&#125;");
        altmath = findreplace(altmath,"\\_","&CAREunderscore;");
        altmath = findreplace(altmath,"<","&lt;");
        altmath = findreplace(altmath,">","&gt;");
    }
    return altmath;
    
}

string LaTeX_global_preprocess(string rawstring) // Do this first to the whole file.
{
    string filedata = "";
    string line;
    bool goodline;
    
    
    stringstream mystream;
    
    mystream.str(rawstring);
    
    goodline = false;
    while (getline(mystream,line)) {
        line = LaTeXmakedollarinert(line);
        line = LaTeX_suppress_comments(line);
        filedata += line + "\n";
    }
    return filedata;
}

string LaTeXrevivedollarinmath(string math)
{
    string result;
    string altmath = math;
    string mod = "";
    size_t where1, where2;

    // If they appear in math mode, you want to completely restore all the changes that you made above.
    mod = altmath + "E"; // Add the "E" as a throw-away so that they start different.
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"&#36;","\\$");
        altmath = findreplace(mod,"&CARElitamp;","\\&");
        altmath = findreplace(altmath,"&CAREhashtag;","\\#");
        altmath = findreplace(altmath,"&#37;","\\%");
        altmath = findreplace(altmath,"&#123;","\\{");
        altmath = findreplace(altmath,"&#125;","\\}");
        altmath = findreplace(altmath,"&CAREunderscore;","\\_");
        altmath = findreplace(altmath,"&lt;","<");
        altmath = findreplace(altmath,"&gt;",">");
        altmath = findreplace(altmath,"@amp;","&");
    }

    return altmath;
}

string processLaTeXtext(string math)
{
    string result;
    string altmath = math;
    string mod = "";
    size_t where1, where2;

    
    mod = altmath + "E"; // Add the "E" as a throw-away so that they start different.
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"\\textasciitilde","~");
        altmath = findreplace(altmath,"\\textasciicircum","^");
        altmath = findreplace(altmath,"\\textbackslash","\\");

    }
    
    mod = altmath + "E"; // Add the "E" as a throw-away so that they start different.
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"``","\"");
    }
    
    mod = altmath + "E"; // Add the "E" as a throw-away so that they start different.
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"``","\"");
    }
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"''","\"");
    }
    
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        where1 = mod.find("\\it");
        if (where1 < mod.length()) {
            where2 = findbrace(mod,where1,math.length(),-1);
            if (where2 < mod.length()) {
                altmath = mod.replace(where2,1,"</i>}");
            } else {altmath = altmath + "</i>";}
            altmath = altmath.replace(where1,3,"<i>");
        }
    }
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        where1 = mod.find("\\textit");
        if (where1 < mod.length()) {
            where2 = findbrace(mod,where1,math.length(),0);
            if (where2 < mod.length()) {
                altmath = mod.replace(where2,1,"</i>}");
            } else {altmath = altmath + "</i>";}
            altmath = altmath.replace(where1,7,"<i>");
        }
    }
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        where1 = mod.find("\\bf");
        if (where1 < mod.length()) {
            where2 = findbrace(mod,where1,math.length(),-1);
            if (where2 < mod.length()) {
                altmath = mod.replace(where2,1,"</b>}");
            } else {altmath = altmath + "</b>";}
            altmath = altmath.replace(where1,3,"<b>");
        }
    }
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        where1 = mod.find("\\textbf");
        if (where1 < mod.length()) {
            where2 = findbrace(mod,where1,math.length(),0);
            if (where2 < mod.length()) {
                altmath = mod.replace(where2,1,"</b>}");
            } else {altmath = altmath + "</b>";}
            altmath = altmath.replace(where1,7,"<b>");
        }
    }
    
    mod = altmath + "E"; // Add the "E" as a throw-away so that they start different.
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"{","");
    }
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"}","");
    }
    
    mod = altmath + "E"; // Add the "E" as a throw-away so that they start different.
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"&#37;","%");
        altmath = findreplace(altmath,"&#123;","{");
        altmath = findreplace(altmath,"&#125;","}");
        altmath = findreplace(altmath,"&#36;","$");
        altmath = findreplace(altmath,"&CARElitamp;","&amp;");
        altmath = findreplace(altmath,"@amp;","&amp;");
        altmath = findreplace(altmath,"@CAREbacksl;","\\");
        altmath = findreplace(altmath,"&CAREunderscore;","_");
        altmath = findreplace(altmath,"&CAREhashtag;","#");
    }
    
    

    
    return altmath;
}



string XMLescape(string math)
{
    string result;
    string altmath = math;
    string mod = "";
    
    mod = altmath + "E"; // Add the "E" as a throw-away so that they start different.
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"&","@amp;");
    }
    
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"@amp;","&amp;");
    }
    
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"<","&lt;");
    }
    
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,">","&gt;");
    }
    
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"\"","&quot;");
    }
    
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"\'","&apos;");
    }
    
    return altmath;
}

string CANVASmath(string math) {
    string result;
    string altmath = math;
    string mod = "";
    string piece1, piece2, piece3, piece4, piece5;

    // This is the HTML escaping part...
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"&","@amp;");
    }
    
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"@amp;","&amp;");
    }
    
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"\"","&quot;");
    }
    
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"<","&lt;");
    }
    
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,">","&gt;");
    }
    
    mod = altmath + "E";
    while (mod != altmath) {
        mod = altmath;
        altmath = findreplace(mod,"'","&apos;");
    }
    
    char hexchar[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    string URI = "";
    int i;
    for(i=0;i<math.length();i++) {
        if ((math[i] >= '0')&&(math[i] <= '9')) {
            URI = URI + "X";
            URI[URI.length()-1] = math[i];
        } else if ((math[i] >= 'A')&&(math[i] <= 'Z')) {
            URI = URI + "X";
            URI[URI.length()-1] = math[i];
        } else if ((math[i] >= 'a')&&(math[i] <= 'z')) {
            URI = URI + "X";
            URI[URI.length()-1] = math[i];
        } else {
            URI = URI + "%25XX";
            URI[URI.length()-2] = hexchar[((math[i])>>4)%16];
            URI[URI.length()-1] =  hexchar[math[i]%16];
        }
    }
    
    piece1 = "<img class=\"equation_image\" " + cssoptions;
    piece1 = piece1 + " title=\"";  // Here goes escaped LaTeX
    piece2 = "\" src=\"/equation_images/"; // Here goes URI LaTeX
    piece3 = "\" alt=\"LaTeX: "; // Here goes escaped LaTeX
    piece4 = "\"  data-equation-content=\""; // Here goes escaped LaTeX
    piece5 = "\"/>";
    return piece1 + altmath + piece2 + URI + piece3 + altmath + piece4 + altmath + piece5;
}

string pluck_opener(string &content, string &nonmath, string &found_math) {
    string result;
    int position;
    int closepos;
    int length = content.length();
    int mstart,newstart;
    bool inl = true;
    nonmath = ""; found_math = "";

    position = find_adjust(content.find("$$"),length);
    position = min( find_adjust(content.find("\\["),length) , position);
    position = min( find_adjust(content.find("\\begin{equation}"),length) , position);
    position = min( find_adjust(content.find("\\begin{equation*}"),length) , position);
    position = min( find_adjust(content.find("\\("),length), position);
    position = min( find_adjust(content.find("$"),length), position);
    
    if (content.find("$$") == position) {
        mstart = position + 2;
        closepos = find_adjust(content.find("$$",position+2),length);
        newstart = closepos + 2;
        inl = false;
    }
    if (content.find("\\[") == position) {
        mstart = position + 2;
        closepos = find_adjust(content.find("\\]",position+2),length);
        newstart = closepos + 2;
        inl = false;
    }
    if (content.find("\\begin{equation}") == position) {
        mstart = position + 16;
        closepos = find_adjust(content.find("\\end{equation}",position+1),length);
        newstart = closepos + 14;
        inl = false;
    }
    if (content.find("\\begin{equation*}") == position) {
        mstart = position + 17;
        closepos = find_adjust(content.find("\\end{equation*}",position+1),length);
        newstart = closepos + 15;
        inl = false;
    }
    if (content.find("\\(") == position ) {
        mstart = position + 2;
        closepos = find_adjust(content.find("\\)",position+2),length);
        newstart = closepos + 2;
        inl = true;
    }
    if ((content.find("$") == position)&&(content.find("$$") != position)) {
        mstart = position + 1;
        closepos = find_adjust(content.find("$",position+1),length) ;
        newstart = closepos + 1;
        inl = true;
    }
    if ((position != length) && (closepos == length)) {cout << "Mismatched math mode delimiters.\n"; exit(-1);}
    if (position == length) {nonmath = content; found_math = ""; return "";}
    nonmath = content.substr(0,position);
    if (!inl) nonmath += "</p><p " + cssoptions_displayed + ">" ;
    found_math = content.substr(mstart,closepos-mstart);
    if (!inl) {found_math = "\\displaystyle " + found_math;}
    result = content.substr(newstart,length-newstart);
    if (!inl) {result = "</p><p>" + result; }
    
    return result;
}



string LaTeX2Canvas(string rawstring)
{
    string filedata = "";
    string line;
    string nonmath,math;
    string HTML;
    bool goodline;
    string spacer;
    
    
    stringstream mystream;

    mystream.str(rawstring);
    
    goodline = false;
    while (getline(mystream,line)) {
        //line = LaTeXmakedollarinert(line);
        //line = LaTeX_suppress_comments(line);
        spacer = " ";
        if ((line.length() > 0) && (!goodline)) {
            filedata.append("<p " + cssoptions + ">");
            goodline=true; spacer = "";
        }
        if (goodline) {filedata.append(spacer + line);}
        if (line.length() == 0) { if (goodline) {filedata.append("</p>"); goodline = false;} }
    }
    if (goodline) { filedata.append("</p>"); goodline = false;}

    HTML = "";
    while (filedata.length() > 0) {
        filedata = pluck_opener(filedata,nonmath,math); // filedata already has <p> tags in it.
        math = LaTeXrevivedollarinmath(math); // Math stuff goes back to literal LaTeX.
        nonmath = processLaTeXtext(nonmath);
        HTML = HTML + nonmath;
        if (math != "") HTML = HTML + CANVASmath(math);
    }
    return HTML;
}




#endif
    
    
    

