import re,os
import cgi,urllib
import imsqtiwriter,zipfile
from datetime import datetime


#Collects expressions inside braces: {expr1} ... {exprn}
#Terminates when things come between the braces
#including two newlines a la LaTeX
#tofind = -1 indicates find all that are possible
#any other value is an upper bound for the number to return
def bracket_exprs(haystack, tostart=0, toend=0, allowed = [["{"],["}"]],tofind=-1):
    bdepth = 0
    escape = 0
    juststarted = 0
    justended = 1
    newlines = 0
    if toend == 0:
        toend = len(haystack)
    current_working = ""
    brackets_found = []
    for c in haystack[tostart:toend:1]:
        if c in allowed[0] and escape == 0:
            bdepth += 1
            if bdepth == 1:
                current_working = ""
                juststarted = 1
        elif c in allowed[1] and escape == 0:
            bdepth -= 1
            if bdepth == 0:
                justended = 1
                newlines = 0
                brackets_found.append(current_working)
                if len(brackets_found) == tofind:
                    break
            if bdepth == -1:
                break
        if bdepth == 0 and justended == 0:
            if c == "\n":
                newlines += 1
                if newlines == 2:
                    break
            # Stop if you find anything at all outside the braces you're looking for
            if justended == 0 and c.isspace() == 0:
                break
        if bdepth >= 1 and juststarted == 0:
            current_working += c
        juststarted = 0
        justended = 0
        if c == "\\":
            escape = 1
        else:
            escape = 0
    return brackets_found

# FixedMacro is a list of LaTeX snippets (strings) which correspond to the
# parts that one would find in a well-formed call to \newcommand
class FixedMacro(object):
    def __init__(self,listobj):
        self._macrodef = listobj

    def __len__(self):
        return len(self._macrodef)

    def to_LaTeX(self):
        resstr = "\\newcommand"
        for index in range(len(self._macrodef)):
            if (index == len(self._macrodef)-1) or (index == 0):
                resstr += "{" + self._macrodef[index] + "}"
            else:
                resstr += "[" + self._macrodef[index] + "]"
        return resstr

    def apply_to(self,data):
        loopgoes = 1
        replacestring = self._macrodef[len(self._macrodef)-1]
        if len(self._macrodef) == 2:
            args = 0
        else:
            args = int(self._macrodef[1])
        if len(self._macrodef) == 4:
            default1 = self._macrodef[2]
        else:
            default1 = ""
        while loopgoes == 1:
            loopgoes = 0
            before = data
            finder = r"(?<!\\)" + re.escape(self._macrodef[0]) + r"(?![a-zA-Z])"
            searchobj = re.search(finder,data)
            if searchobj:
                loopgoes = 1
                substitute = replacestring
                arguments = bracket_exprs(data,searchobj.end(),len(data),[["{"],["}"]],args)
                arg_offset = 0
                if len(arguments) < args:
                    argname = "#1"
                    substitute = re.sub(argname,lambda x: default1,substitute)
                    arg_offset = 1
                for i in range(len(arguments)):
                    argname = "#" + str(i+1+arg_offset)
                    substitute = re.sub(argname, lambda x: arguments[i],substitute)
                    finder += r"\s*\n?\s*\{" + re.escape(arguments[i]) + "\}"
                data = re.sub(finder,lambda x: substitute,data)

        return data

# FloatingMacro is a thing like "a \over b" or "\it texthere" which is constrained
# by brackets it is contained inside. FloatingMacros always have two parts:
# Part #1 is everything in the brackets before the macro, and #2 is everything in
# the brackets which comes after the macros
# To construct, give two strings: first the name, then the definition in terms of #1 and #2
class FloatingMacro(object):
    def __init__(self,name,definition):
        self._name = name
        self._definition = definition

    def apply_to(self,data):
        looping = 1
        allowed = [["{"],["}"]]
        while looping == 1:
            finder = re.search(r"\s*(?<!\\)" + re.escape(self._name) + r"(?![a-zA-Z])\s*",data)
            if not finder:
                return data
            looping = 1
            escaped = 0
            bkdepth = 1
            closepos = len(data) #index of the closing brace
            for index in range(finder.end(0),len(data)):
                c = data[index]
                if c == "\\":
                    escaped = 1 - escaped
                elif c in allowed[0]:
                    if escaped == 0:
                        bkdepth += 1
                    escaped = 0
                elif c in allowed[1]:
                    if escaped == 0:
                        bkdepth -= 1
                    escaped = 0
                else:
                    escaped = 0
                if bkdepth == 0:
                    closepos = index
                    break
            escaped = 0
            bkdepth = 1
            pending_change = 0
            pending_spot = finder.start(0)
            openpos = -1 # Index of the opening brace
            for index in range(finder.start(0),-1,-1):
                c = data[index]
                if c != "\\":
                    if (escaped == 0) and (pending_change != 0):
                        bkdepth += pending_change
                    pending_change = 0
                    escaped = 0
                    if bkdepth == 0:
                        openpos = pending_spot
                        break
                else:
                    escaped = 1 - escaped
                if c in allowed[0]:
                    pending_change = -1
                    pending_spot = index
                elif c in allowed[1]:
                    pending_change = 1
                    pending_spot = index
            if (bkdepth != 0):
                if (escaped == 0) and (bkdepth + pending_change == 0):
                    openpos = pending_spot
            part1 = data[openpos+1:finder.start(0)]
            part2 = data[finder.end(0):closepos]
            if (openpos > -1):
                before = data[0:openpos+1]
            else:
                before = ""
            if (closepos < len(data)):
                after = data[closepos:len(data)]
            else:
                after = ""
            expanded = re.sub(r"\#1",lambda x : part1, self._definition)
            expanded = re.sub(r"\#2",lambda x : part2, expanded)
            data = before + expanded + after
        return data

# A RegexpMacro is a wrapper for re.sub. To contstruct, you need the pattern string and
# the replacewith string
class RegexpMacro(object):
    def __init__(self,findstring,replaceobj):
        self._my_pattern = re.compile(findstring)
        self._replaceobj = replaceobj
    def apply_to(self,data):
        return re.sub(self._my_pattern,self._replaceobj,data)


###############################################################################
###############################################################################
#####
#####  LaTeX_to_HTML(string) : simple conversion of 'string' from LaTeX to HTML
#####
###############################################################################
###############################################################################
##### Step 1: Apply _HTML_start_macros to contiguous blocks of text (no math)
##### Step 2: Apply _HTML_global_macros to Step 1 output *with* math included
#####         (so nothing that would affect math mode should be done here)
##### Step 3: Apply _HTML_end_macros to contiguous blocks of texts from Step 2

# _HTML_maintag is the workhorse tag that will indicate blocks
_HTML_maintag = "div"
_HTML_partag = "p"
# Local HTML conversions to be applied once in text mode only
_HTML_start_macros = [
RegexpMacro(r"(?<!\\)<","&#161;"),RegexpMacro(r"(?<!\\)>","&#191;"),
RegexpMacro(r"(?<!\\)\\\$","&#36;"),
RegexpMacro(r"(?<!\\)''","&rdquo;"),RegexpMacro(r"(?<!\\)``","&ldquo;"),
RegexpMacro(r"(?<!\\)\"","&quot;"),RegexpMacro(r"(?<!\\)'","&apos;"),
RegexpMacro(r"(?<!\\)\\&","&amp;"),RegexpMacro(r"(?<!\\)\\#","&#35;"),
RegexpMacro(re.escape(r"\_"),"&#95;"),
RegexpMacro(r"(?<!\\)\\textasciitilde(?!\w)","&#126;"),
RegexpMacro(r"(?<!\\)\\textasciicircum(?!\w)","&#94;"),
RegexpMacro(r"(?<!\\)\\textbackslash(?!\w)",r"&#92;"),
RegexpMacro(r"(?<!\\)\\{","&#123;"),RegexpMacro(r"(?<!\\)\\}","&#125;"),
RegexpMacro(r"\s*\n\s*\n\s*","</" + _HTML_maintag + ">\n<" + _HTML_maintag + ">"),
RegexpMacro(r"(?<!\\)\\newline(?!\w)",""),
FixedMacro([r"\vspace","1",""])
]
_HTML_diac_latexmk = ["`","'","\"","^","~","=","."]
_HTML_diac_accname = ["grave","acute","uml","circ","tilde","macr","dot"]
_HTML_diac_moremk  = ["u","v","c"]
_HTML_diac_morenm  = ["breve","caron","cedil"]
for index in range(len(_HTML_diac_latexmk)):
    _HTML_start_macros.append(RegexpMacro(r"(?<!\\)\\"+
    re.escape(_HTML_diac_latexmk[index])+r"\s*{\s*(\w)\s*}","&\\1"+_HTML_diac_accname[index] + ";"))
    _HTML_start_macros.append(RegexpMacro(r"(?<!\\)\\"+
    re.escape(_HTML_diac_latexmk[index])+r"\s*(\w)","&\\1"+_HTML_diac_accname[index] + ";"))
for index in range(len(_HTML_diac_moremk)):
    _HTML_start_macros.append(RegexpMacro(r"(?<!\\)\\"+
    re.escape(_HTML_diac_moremk[index])+r"\s*{\s*(\w)\s*}","&\\1"+_HTML_diac_morenm[index] + ";"))

# Do these once in order; they may span across math mode. Nothing here should
# Be expected to affect math mode. SO: \mbox inside math mode isn't banned, but
# it should not include any text modifiers like bold or italic, etc.
_HTML_global_macros = [
RegexpMacro(r"\\begin{enumerate}\s*\\item","</" + _HTML_maintag + "><ol><li><" + _HTML_maintag + ">"),
RegexpMacro(r"\\begin{itemize}\s*\\item","</" + _HTML_maintag + "><ul><li><" + _HTML_maintag + ">"),
RegexpMacro(r"\s*\\item","</" + _HTML_maintag + "></li><li><" + _HTML_maintag + ">"),
RegexpMacro(r"\s*\\end{enumerate}\s*","</" + _HTML_maintag + "></li></ol><" + _HTML_maintag + ">"),
RegexpMacro(r"\s*\\end{itemize}\s*","</" + _HTML_maintag + "></li></ul><" + _HTML_maintag + ">"),
RegexpMacro(r"\\begin\s*{center}","</" + _HTML_maintag + "><" + _HTML_maintag + " style=\"text-align:center\"><" + _HTML_maintag + ">"),
RegexpMacro(r"\\end\s*{center}","</" + _HTML_maintag + "></" + _HTML_maintag + "><" + _HTML_maintag + ">"),
RegexpMacro(r"\\includegraphics[^{]*\{([^}]*)\}","<img src=\"\\1\" style=\"float: left\" width=\"35%\"/>"),
FixedMacro([r"\textbf","1","<b>#1</b>"]),
FixedMacro([r"\textit","1","<i>#1</i>"]),
FixedMacro([r"\emph","1","<em>#1</em>"]),
FixedMacro([r"\underline","1",r"<span style=\"text-decoration:underline\">#1</span>"]),
RegexpMacro(r"<" + _HTML_maintag + "></" + _HTML_maintag + ">","")
]

# These last macros are applied to text only. Because of the way that \it and
# \bf are handled, it is to be expected that there will be stray brackets that
# will need to be cleaned up. One might as well also remove any obvious
# macro names that haven't been handled so far.
_HTML_end_macros = [
RegexpMacro(r"{",""),       # Strip remaining text-mode braces
RegexpMacro(r"}",""),
RegexpMacro(r"\\\w*",r""),    # Strip remaining text-mode macro names
RegexpMacro(r"~"," ")
]

_HTML_math_image = r'<img class="equation_image" style="" title="" src="" alt="" data-equation-content="" />'
_HTML_math_inline = r'style="vertical-align:middle;"'
_HTML_math_display = r'style="vertical-align:middle;"'

def _LaTeX_modes(data):
    dataparts = []
    escaped = 0
    current = ""
    for c in data:
        if c == "\\":
            if escaped == 1:
                current += "\\\\"
            escaped = 1 - escaped
        elif (c == "(") or (c == "["):
            if escaped == 0:
                current += c
            else:
                dataparts.append(["text",current])
                current = "\\" + c
                escaped = 0
        elif (c == ")") or (c == "]"):
            if escaped == 0:
                current += c
            else:
                current += "\\" + c
                dataparts.append(["math",current])
                current = ""
                escaped = 0
        else:
            if escaped == 1:
                current += "\\"
            current += c
            escaped = 0
    if escaped == 1:
        current += "\\"
    dataparts.append(["text",current])
    return dataparts

def LaTeX_to_HTML(data,mathasimg = False):
    dataparts = _LaTeX_modes(data)
    local_result = ""
    for part in dataparts:
        if part[0] == "math":
            local_result += part[1]
        else:
            local_part = part[1]
            for macro in _HTML_start_macros:
                local_part = macro.apply_to(local_part)
            local_result += local_part
    global_result = local_result
    for macro in _HTML_global_macros:
        global_result = macro.apply_to(global_result)
    global_result = "<" + _HTML_maintag + ">" + global_result + "</" + _HTML_maintag + ">"
    dataparts = _LaTeX_modes(global_result)
    local_result = ""
    for part in dataparts:
        if part[0] == "math":
            if not mathasimg:
                local_result += part[1]
            else:
                rawmath = part[1]
                if re.search(r"\\\(((.|\n)*)\\\)",rawmath):
                    style = _HTML_math_inline
                    math = re.search(r"\\\(((.|\n)*)\\\)",rawmath).group(1)
                    mathopen = ""
                    mathclose = ""
                elif re.search(r"\\\[((.|\n)*)\\\]",rawmath):
                    style = _HTML_math_display
                    math = re.search(r"\\\[((.|\n)*)\\\]",rawmath).group(1)
                    math = r"\displaystyle " + math
                    mathopen = "<" + _HTML_partag + " style=\"text-align:center;\">"
                    mathclose = "</" + _HTML_partag + ">"
                else:
                    style = _HTML_math_display
                    math = "UNKNOWN MATH FORMAT"
                html_escaped = cgi.escape(math)
                uri = urllib.quote(math)
                uri = re.sub(r"%","%25",uri)
                mathresult = _HTML_math_image
                mathresult = re.sub(r'style=""',lambda x : style,mathresult)
                mathresult = re.sub(r'title=""',lambda x : 'title="' + html_escaped + '"',mathresult)
                mathresult = re.sub(r'src=""',lambda x : 'src="/equation_images/' + uri + '"',mathresult)
                mathresult = re.sub(r'alt=""',lambda x: 'alt="'+ html_escaped + '"',mathresult)
                mathresult = re.sub(r'data-equation-content=""',lambda x : 'data-equation-content="'+ html_escaped + '"',mathresult)
                local_result += mathopen + mathresult + mathclose
        else:
            local_part = part[1]
            for macro in _HTML_end_macros:
                local_part = macro.apply_to(local_part)
            local_result += local_part
    return local_result



class LaTeXRaw(object):
    _system_macros_mandatory = []
    _system_macros_options = [
    ["system",FixedMacro(["\\bigmath","1","\\(\\displaystyle #1\\)"])],
    ["system",FloatingMacro("\\over","\\frac{#1}{#2}")],
    ["system",FloatingMacro("\\it","#1 \\textit{#2}")],
    ["system",FloatingMacro("\\bf","#1 \\textbf{#2}")],
    ["system",RegexpMacro(r"(?<!\\)\\mbox(?!\w)",r"\\text")],
    ["offline",FixedMacro(["\\offline","1","#1"])],
    ["online",FixedMacro(["\\online","1","#1"])],
    ["offline",FixedMacro(["\\online","1",""])],
    ["online",FixedMacro(["\\offline","1",""])]
    ]
    _system_macros_default = "system|online|user"

    def _expand_list(self,data,macrolist):
        loopgoes = 1
        if len(macrolist) == 0:
            return data
        macronumber = 0
        while loopgoes == 1:
            loopgoes = 0
            before = data
            data = macrolist[macronumber].apply_to(data)
            if (before != data) and (macronumber != 0):
                macronumber = 0
            else:
                macronumber += 1
            if macronumber < len(macrolist):
                loopgoes = 1
            else:
                loopgoes = 0
        return data

    def __init__(self,filename,optionlist):
        self._system_macros = self._system_macros_mandatory
        if optionlist is None:
            optionlist = self._system_macros_default
        specified_options = re.split(r"\|",optionlist)
        print "INFO  : Expanding macros for options " + str(specified_options)
        for macro in self._system_macros_options:
            if macro[0] in specified_options:
                self._system_macros.append(macro[1])
        if filename != "":
            self.srcfilename = os.path.abspath(filename)
        else:
            self.srcfilename = ""
        latexcode = open(filename,"r").read()
        self._user_macros = []
        self._srccode = latexcode
        # Strip comments; they are absolutely capable of causing problems if left intact
        # Convert windows line endings
        latexcode = re.sub(r"(?<!\\)%.*[\n]","",latexcode)
        latexcode = re.sub(r"(?<!\\)%.*$","",latexcode)
        latexcode = re.sub(r"\r","",latexcode)
        # Convert math modes to a smaller subset of completely equivalent things
        # In particular, we get rid of dollar signs which aren't escapes like \$
        latexcode = re.sub(r"(?<!\\)(\$\$)([^$]*[^\\])(\$\$)",r"\\[\2\\]",latexcode)
        latexcode = re.sub(r"(?<!\\)(\$)([^$]*[^\\])(\$)",r"\\(\2\\)",latexcode)
        latexcode = re.sub(r"\\begin\s*\n?\s*{equation\*}","\\[",latexcode)
        latexcode = re.sub(r"\\end\s*\n?\s*{equation\*}","\\]",latexcode)
        latexcode = re.sub(r"\\begin\s*\n?\s*{align\*}",r"\[\\begin{aligned}",latexcode)
        latexcode = re.sub(r"\\end\s*\n?\s*{align\*}","\\end{aligned}\\]",latexcode)

        self.cleaned_source = latexcode

        userkey = "user"
        if userkey in specified_options:
            deftype = r"\newcommand"
            pattern = re.compile(re.escape(deftype))
            for m in re.finditer(pattern,latexcode):
                guts = bracket_exprs(latexcode,m.end(0),len(latexcode),[["{","["],["}","]"]])
                self._user_macros.append(FixedMacro(guts))
            for foundmacro in self._user_macros:
                if (len(foundmacro) < 2) or (len(foundmacro) > 4):
                    raise Exception("Malformed \\newcommand in file " + self.srcfilename)
                patternstring = re.escape(deftype)
                for part in foundmacro._macrodef:
                    patternstring += "\s*[\[{]" + re.escape(part) + "[\]}]"
                patternstring += "\s*"
                self.cleaned_source = re.sub(patternstring,"",self.cleaned_source)
        self.cleaned_source = self._expand_list(self.cleaned_source,self._system_macros+self._user_macros)
        print "INFO  : Expansion complete."

class QuestionGem(object):
    _question_type_default = "file_upload_question"
    _internals = ["image","answer","multiplechoice","shortanswer","feedback","praise","comments","notes","keywords","date","qid"]
    _comma_separated = ["keywords"]
    _choice_provider = ["multiplechoice","shortanswer"]
    _choice_provider_txt = ["shortanswer"]
    _output_first = ["image"]
    _question_types = ["file_upload_question","multiple_choice_question","short_answer_question","essay_question","multiple_answers_question"]
    def __init__(self,latexcode = ""):
        self.bank_title = ""
        self.bank_id = 0
        self.index_in_bank = 0
        self.type = self._question_type_default
        current_type = self.type
        self.srccode = latexcode
        self.statement = latexcode
        self.title = ""
        self._choicebreakpos = []
        self.all_choices = []
        self.correct_choices = []
        # A list of items of the form [envname,option,contents]
        self._environments = []
        # Find the first \begin{question} and the first \end{question}
        # The contents are whatever comes in between
        self._choices_in_txt = False
        if latexcode == "":
            for envname in self._internals:
                self._environments.append([envname,"",""])
            return
        starting = re.search(r"\\begin\s*\{question\}\s*(\[([^\]]*)\]\s*)?\s*",latexcode)
        if starting is None:
            raise Exception("No \\begin{question} here.\nIn " + srcfile +":\n" + self.srccode)
        ending  = re.search(r"\s*\\end\s*\{question\}",latexcode)
        raw_ending = re.search(r"\s*\\end\s*\{question\}",self.srccode)
        if ending is None:
            raise Exception("No \\end{question} here.\nIn " + srcfile +":\n" + self.srccode)
        if starting.end(0) > ending.start(0):
            raise Exception("Found \\end{question} before \\begin{question}.\nIn "+ srcfile +":\n" + self.srccode)
        self.statement = latexcode[starting.end(0):ending.start(0)]
        if starting.group(2) is not None:
            self.title = starting.group(2)
        else:
            self.title = ""
        # Now find all the environments of types identified by _internals list
        for envname in self._internals:
            patterndesc = r"\\begin\s*\{" + re.escape(envname) + r"\}\s*(\[([^\]]*)\]\s*)?((.|\n)*?)\s*\\end\s*\{" + re.escape(envname)+ r"\}"
            pattern = re.compile(patterndesc)
            resopt = ""
            ressrc = ""
            for result in re.finditer(pattern,latexcode):
                # First see if you can figure out the type of question it will be
                if envname == "multiplechoice":
                    current_type = "multiple_choice_question"
                elif envname == "shortanswer":
                    current_type = "short_answer_question"
                if envname in self._choice_provider_txt:
                    self._choices_in_txt = True
                if (self.type != current_type) and (self.type != self._question_type_default):
                    raise Exception("Conflicting type information.\nIn "+ srcfile +":\n" + self.srccode)
                else:
                    self.type = current_type
                if result.group(2) is not None:
                    # concatenated option lists are separated by commas
                    if resopt != "":
                        resopt += ","
                    resopt += result.group(2)
                if result.group(3) is not None:
                    if ressrc != "":
                        if envname in _comma_separated:
                            ressrc += ","
                        else:
                            ressrc += "\n\n"
                    ressrc += result.group(3)
                self.statement = re.sub(re.escape(result.group(0)) + r"\s*","",self.statement)
                ressrc = re.sub(r"^\s*","",ressrc)
                ressrc = re.sub(r"\s*$","",ressrc)
            self._environments.append([envname,resopt,ressrc])
            if envname in self._choice_provider and ressrc != "":
                # data will contain the combined environments of the choice providers
                data = ressrc
                ressrc = "Do not use--grab data from all_choices instead."
        self.statement = re.sub(r"\n\n+",r"\n\n",self.statement)
        self.statement = re.sub(r"\s*$","",self.statement)
        self.statement = re.sub(r"^\s*","",self.statement)
        # Proces options in _choice_provider environments
        options = []
        corrects = []
        pattern = re.compile(r"\\choice\s*(\[([^\]]*)\])?\s*(?=\{)")
        breaks = re.split(r"\\choicebreak",data)
        if (len(breaks) > 1):
            self._choicebreakpos = []
            del breaks[-1]
            choicefound = 0
            for cluster in breaks:
                choicefound += len(re.findall(r"(?<!\\)\\choice",cluster))
                self._choicebreakpos.append(choicefound)
        else:
            self._choicebreakpos = []
        for m in re.finditer(pattern,data):
            bkts = bracket_exprs(data,m.end(0),len(data))
            if (len(bkts) != 1):
                raise Exception("Malformed \\choice element.\nIn "+ srcfile +":\n" + self.srccode)
            # If the \choice has "correct" or if short answer, note the answer as correct
            if m.group(2):
                if re.search(r"(?<!\w)correct(?!\w)",m.group(2)) or self.type == "short_answer_question":
                    corrects.append(bkts[0])
            elif self.type == "short_answer_question":
                corrects.append(bkts[0])
            options.append(bkts[0])
        self.all_choices = options
        self.correct_choices = corrects

    def to_LaTeX(self):
        resultstring = "\\begin{question}"
        if self.title != "":
            resultstring += "[" + self.title + "]"
        resultstring += "\n"
        for envname in self._output_first:
            index = self._internals.index(envname)
            data = self._environments[index]
            if (data[2] != ""):
                resultstring += "\\begin{" + envname + "}"
                if (data[1] != ""):
                    resultstring += "[" + data[1] + "]"
                resultstring += "\n" + data[2] + "\n\\end{" + envname + "}\n"
        resultstring += self.statement + "\n"
        for envname in self._internals:
            if envname not in self._output_first:
                index = self._internals.index(envname)
                data = self._environments[index]
                if (data[2] != ""):
                    resultstring += "\\begin{" + envname + "}"
                    if (data[1] != ""):
                        resultstring += "[" + data[1] + "]"
                    resultstring += "\n"
                    if envname in self._choice_provider:
                        response_counter = 0
                        for respstr in self.all_choices:
                            response_counter += 1
                            resultstring += "\\choice"
                            if respstr in self.correct_choices:
                                resultstring += "[correct]"
                            resultstring += "{" + respstr + "}"
                            if response_counter in self._choicebreakpos:
                                resultstring += " \\choicebreak"
                            resultstring += "\n"
                    else:
                        resultstring += data[2] + "\n"
                    resultstring += "\\end{" + envname + "}\n"
        resultstring += "\\end{question}"
        return resultstring

    def get(self,envname):
        # Don't be naughty--the get function is for non-volatile things like comments
        if envname in self._choice_provider:
            print "WARN  : get(..) is not for accessing choice_providers"
            return None
        for envdata in self._environments:
            if envdata[0] == envname:
                return envdata[2]
        return None

    def set(self,envname,latexcontents,latexoption = ""):
        # Don't be naughty--the get function is for non-volatile things like comments
        if envname in self._choice_provider:
            print "WARN  : set(..) is not for accessing choice_providers"
            return None
        for index in range(len(self._environments)):
            if self._environments[index][0] == envname:
                self._environments[index][1] = latexoption
                self._environments[index][2] = latexcontents
                return self._environments[index][2]
        return latexcontents

    def HTML_statement(self,mathasimg=False):
        if (self.statement != ""):
            return LaTeX_to_HTML(self.statement,mathasimg)
        return ""
    #def HTML_feedback(self,mathasimg=False):
    #    rawlatex = self.get("feedback")
    #    if (rawlatex != ""):
    #        return LaTeX_to_HTML(rawlatex,mathasimg)
    #    return ""
    def HTML_get(self,type,mathasimg=False):
        rawlatex = self.get(type)
        if (rawlatex != ""):
            return LaTeX_to_HTML(rawlatex,mathasimg)
        return ""
    #def HTML_praise(self,mathasimg=False):
    #    rawlatex = self.get("praise")
    #    if (rawlatex != ""):
    #        return LaTeX_to_HTML(rawlatex,mathasimg)
    #    return ""
    #def HTML_comments(self,mathasimg=False):
    #    rawlatex = self.get("comments")
    #    if (rawlatex != ""):
    #        return LaTeX_to_HTML(rawlatex,mathasimg)
    #    return ""
    def HTML_all_choices(self,mathasimg=False):
        returnlist = []
        if self._choices_in_txt:
            return self.all_choices
        for choice in self.all_choices:
            returnlist.append(LaTeX_to_HTML(choice,mathasimg))
        return returnlist
    def HTML_correct_choices(self,mathasimg=False):
        returnlist = []
        if self._choices_in_txt:
            return self.correct_choices
        for choice in self.correct_choices:
            returnlist.append(LaTeX_to_HTML(choice,mathasimg))
        return returnlist

    def append(self,envname,latexcontents,latexoption = ""):
        # Don't be naughty--the get function is for non-volatile things like comments
        if envname in self._choice_provider:
            print "WARN  : append(..) is not for accessing choice_providers"
            return None
        for index in range(len(self._environments)):
            if self._environments[index][0] == envname:
                if self._environments[index][1] != "" and latexoption != "":
                    self._environments[index][1] += ","
                ressrc = self._environments[index][2]
                if ressrc != "" and latexcontents != "":
                    if envname in self._comma_separated:
                        ressrc += ","
                    else:
                        ressrc += "\n\n"
                self._environments[index][1] += latexoption
                self._environments[index][2] = ressrc + latexcontents
                return self._environments[index][2]
        return latexcontents

    def apply_macro(self,themacro):
        self.statement = themacro.apply_to(self.statement)
        for i in range(len(self._environments)):
            if self._environments[i][0] not in self._choice_provider:
                self._environments[i][2] = themacro.apply_to(self._environments[i][2])
        if not self._choices_in_txt:
            for i in range(len(self.all_choices)):
                self.all_choices[i] = themacro.apply_to(self.all_choices[i])
                print "Hello"
            for i in range(len(self.correct_choices)):
                self.correct_choices[i] = themacro.apply_to(self.correct_choices[i])

    def _expand_list(self,latexfile,macrolist):
        self.statement = latexfile._expand_list(self.statement,macrolist)
        for i in range(len(self._environments)):
            if self._environments[i][0] not in self._choice_provider:
                self._environments[i][2] = latexfile._expand_list(self._environments[i][2],macrolist)
        if not self._choices_in_txt:
            for i in range(len(self.all_choices)):
                self.all_choices[i] = latexfile._expand_list(self.all_choices[i],macrolist)
            for i in range(len(self.correct_choices)):
                self.correct_choices[i] = latexfile._expand_list(self.correct_choices[i],macrolist)

class LaTeXQuestions(LaTeXRaw):
    _tikz_compile_begin_format = ".tex"
    _tikz_compile_local_directory = "img"
    _tikz_compile_script = ["pdflatex -interaction=nonstopmode -output-directory [directory] [filename].tex > [filename].cpl","convert -density 250 [filename].pdf -quality 80 -background white -alpha remove -alpha off [filename].png"]
    _tikz_compile_latex_template = r"\documentclass[tikz,margin=5pt]{standalone} \usepackage{amsmath,amssymb,amsfonts} \begin{document} \begin{image} \end{image} \end{document}"
    _tikz_compile_latex_replacement = r"\includegraphics[width=2.5in]{[filename].png}"
    def __init__(self,filename,options = None):
        super(LaTeXQuestions,self).__init__(filename,options)
        self._question_list = []
        self._bank_sizes = [0]
        self._bank_titles = [""]
        remaining_questions = self.cleaned_source
        pattern = re.compile(r"\\begin\{questionbank\}\s*(\[([^\]]*)\]\s*)?\s*((.|\n)*?)\\end\{questionbank\}")
        banks_found = 0
        total_questions = 0
        for bank in re.finditer(pattern,self.cleaned_source):
            banks_found += 1
            remaining_questions = re.sub(re.escape(bank.group(0)),"",remaining_questions)
            banksrc = bank.group(0)
            if bank.group(2):
                bank_title = bank.group(2)
            else:
                bank_title = ""
            questionpattern = re.compile(r"\\begin\{question\}(.|\n)*?\\end\{question\}")
            print "INFO  : Bank " + str(banks_found).zfill(3) + " \"" + bank_title + "\"",
            questions_found = 0
            for question in re.finditer(questionpattern,banksrc):
                questions_found += 1
                thisquestion = QuestionGem(question.group(0))
                thisquestion.bank_id = banks_found
                thisquestion.bank_title = bank_title
                self._question_list.append(thisquestion)
            print ": " + str(questions_found) + " questions"
            total_questions += questions_found
            self._bank_sizes.append(questions_found)
            self._bank_titles.append(bank_title)
        questions_found = 0
        bank_id = 0
        bank_title = "Unbanked Questions"
        banksrc = remaining_questions
        questionpattern = re.compile(r"\\begin\{question\}(.|\n)*?\\end\{question\}")
        print "INFO  : Unbanked Questions",
        questions_found = 0
        for question in re.finditer(questionpattern,banksrc):
            questions_found += 1
            thisquestion = QuestionGem(question.group(0))
            thisquestion.bank_id = bank_id
            thisquestion.index_in_bank = questions_found
            thisquestion.bank_title = bank_title
            self._question_list.append(thisquestion)
        print ":  " + str(questions_found) + " questions"
        total_questions += questions_found
        self._bank_sizes[0] = questions_found
        self._bank_titles[0] = bank_title
        print "INFO  : " + str(total_questions) + " found total."
        self._questions_selected = range(len(self._question_list))

    def expand_list(self,macrolist):
        for i in range(len(self._question_list)):
            self._question_list[i]._expand_list(self,macrolist)

    def __len__(self):
        return len(self._questions_selected)

    def __getitem__(self,key):
        return self._question_list[self._questions_selected[key]]

    def __setitem__(self,key,value):
        self._question_list[self._questions_selected[key]] = value
        return self._question_list[self._questions_selected[key]]

    def select_all(self):
        self._questions_selected = range(len(self._question_list))
    def deselect_all(self):
        self._questions_selected = []
    def select_by_bank_id(self,bank_id):
        if bank_id >= len(self._bank_sizes):
            raise Exception("select_by_bank_id called on a non-existent bank.")
        for index in range(len(self._question_list)):
            if self._question_list[index].bank_id == bank_id and index not in self._questions_selected:
                self._questions_selected.append(index)
    def select_by_search(self,itemname,itemval):
        for index in range(len(self._question_list)):
            if re.search(itemval,self._question_list[index].get(itemname)) and index not in self._questions_selected:
                self._questions_selected.append(index)
    def keep_by_search(self,itemname,itemval):
        remaining = []
        for index in range(len(self._questions_selected)):
            trueindex = self._questions_selected[index]
            if re.search(itemval,self._question_list[trueindex].get(itemname)):
                remaining.append(trueindex)
        self._questions_selected = remaining

    def merge_duplicates(self,other):
        other_nonduplicates = []
        for questionindex in other._questions_selected:
            question = other._question_list[questionindex]
            was_duplicate = False
            for alreadyhaveindex in self._questions_selected:
                alreadyhave = self._question_list[alreadyhaveindex]
                if (alreadyhave.title == question.title) and (alreadyhave.type == question.type):
                    oldbank_title = alreadyhave.bank_title
                    oldbank_id = alreadyhave.bank_id
                    oldbank_index = alreadyhave.index_in_bank
                    self._question_list[alreadyhaveindex] = question
                    self._question_list[alreadyhaveindex].bank_title = oldbank_title
                    self._question_list[alreadyhaveindex].bank_id = oldbank_id
                    self._question_list[alreadyhaveindex].index_in_bank = oldbank_index
                    dt_string = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    self._question_list[alreadyhaveindex].append("notes",dt_string + " : Merged from file " + other.srcfilename)
                    was_duplicate = True
                    break
            if not was_duplicate:
                other_nonduplicates.append(questionindex)
        other._questions_selected = other_nonduplicates


    def total_banks(self):
        return len(self._bank_sizes)
    def bank_title(self,bank_id):
        if bank_id >= len(self._bank_sizes):
            raise Exception("select_by_bank_id called on a non-existent bank.")
        return self._bank_titles[bank_id]

    def tikz_compile(self):
        print "INFO  : Compiling tikzpicture environments to image files"
        _tikz_compile_directory = os.path.join(os.path.dirname(os.path.abspath(self.srcfilename)),self._tikz_compile_local_directory)
        namepart = re.sub(r"\.[^.]*$","",os.path.basename(self.srcfilename))
        if not os.path.exists(_tikz_compile_directory):
            os.mkdir(_tikz_compile_directory)
        tikzpattern = re.compile(r"\\begin{tikzpicture}((.|\n)*?)\\end{tikzpicture}")
        for tempindex in range(len(self._questions_selected)):
            i = self._questions_selected[tempindex]
            bankno = self._question_list[i].bank_id
            questno = self._question_list[i].index_in_bank
            imagecode = self._question_list[i].get("image")
            if re.search(tikzpattern,imagecode):
                localnamepart = namepart + "B" + str(bankno) + "Q" + str(questno)
                file = re.sub(r"\\begin{image}((.|\n)*?)\\end{image}",lambda x : imagecode,self._tikz_compile_latex_template)
                basename = os.path.join(_tikz_compile_directory,localnamepart)
                with open(basename + self._tikz_compile_begin_format, 'w') as destfile:
                    destfile.write(file)
                    destfile.close()
                for command in self._tikz_compile_script:
                    thiscodeline = re.sub(re.escape("[filename]"),lambda x: basename,command)
                    thiscodeline = re.sub(re.escape("[directory]"),lambda x: _tikz_compile_directory,thiscodeline)
                    os.system(thiscodeline)
                resultingimgname = os.path.join(self._tikz_compile_local_directory,localnamepart)
                resultingcode = re.sub(re.escape("[filename]"),lambda x: resultingimgname,self._tikz_compile_latex_replacement)
                self._question_list[i].set("image",resultingcode,"")

    def to_LaTeX(self):
        outsrc = "\\documentclass{article}\n\usepackage{questions}\n"
        for macro in self._user_macros:
            outsrc += macro.to_LaTeX() + "\n"
        outsrc += "\\begin{document}\n"
        for questionno in self._questions_selected:
            question = self._question_list[questionno]
            outsrc += question.to_LaTeX() + "\n"
        outsrc += "\\end{document}"
        return outsrc

    def write_LaTeX(self,filename):
        outsrc = "\\documentclass{article}\n\usepackage{questions}\n"
        for macro in self._user_macros:
            outsrc += macro.to_LaTeX() + "\n"
        outsrc += "\\begin{document}\n"
        banktexts = {}
        for questionno in self._questions_selected:
            question = self._question_list[questionno]
            if question.bank_id not in banktexts:
                banktexts[question.bank_id] = "\\begin{questionbank}[" + question.bank_title + "]\n"
            banktexts[question.bank_id] += question.to_LaTeX() + "\n"

        for bank in banktexts:
            outsrc += banktexts[bank] + "\\end{questionbank}"
        outsrc += "\\end{document}"
        open(filename,"w").write(outsrc)


    def write_QTI(self,filename = "",banktitle="Unnamed Bank",mathasimg = False):
        if len(self._questions_selected) == 0:
            print "INFO  : No questions selected for output."
            return
        myqti = imsqtiwriter.QTI_initialize()
        bankformalident = banktitle
        if filename != "":
            bankformalident = filename + " " + bankformalident
        mybank = imsqtiwriter.QTI_new_bank(myqti,bankformalident,banktitle)
        other_resources = []
        for index in range(len(self)):
            question = self[index]
            imagecode = question.HTML_get("image")
            basics = ["B" + str(question.bank_id).zfill(3), question.bank_title, "Q" + str(question.index_in_bank).zfill(3), question.title, question.type,imagecode + question.HTML_statement(mathasimg)]
            all_choices_raw = question.HTML_all_choices(mathasimg)
            correct_choices_raw = question.HTML_correct_choices(mathasimg)
            all_choices_fmt = []
            correct_choices_fmt = []
            for index in range(len(all_choices_raw)):
                package = ["R" + str(index+1).zfill(3),all_choices_raw[index]]
                all_choices_fmt.append(package)
                if all_choices_raw[index] in correct_choices_raw:
                    correct_choices_fmt.append(package[0])
            reactions = [question.HTML_get("comments",mathasimg),question.HTML_get("praise",mathasimg),question.HTML_get("feedback",mathasimg)]
            questiondata = [basics,all_choices_fmt,correct_choices_fmt,reactions]
            imsqtiwriter.QTI_add_question(mybank,questiondata)
            pictureplaces = [question.get("image"),question.statement,question.get("praise"),question.get("feedback"),question.get("comments")] + question.all_choices
            for codeblock in pictureplaces:
                searchpattern = re.compile(r"\\includegraphics")
                for image in re.finditer(searchpattern,codeblock):
                    associated_content = bracket_exprs(codeblock,image.end(0),len(codeblock),[["[","{"],["}","]"]])
                    other_resources.append(associated_content[-1])

        namebreak = os.path.splitext(os.path.basename(self.srcfilename))
        if len(namebreak) > 0:
            givenname = namebreak[0]
        else:
            givenname = ""
        if filename == "":
            if givenname == "":
                raise Exception("Cannot write zip file with empty filename")
            filename = givenname + ".zip"
        basename = os.path.splitext(os.path.basename(filename))[0]
        xmlname = basename + "_qti.xml"
        print "INFO  : Writing " + filename
        tinyfile = zipfile.ZipFile(filename,"w")
        tinyfile.writestr(xmlname,imsqtiwriter.QTI_generate_XML(myqti))
        tinyfile.writestr("imsmanifest.xml",imsqtiwriter.QTI_generate_manifest("autogenerated",basename,xmlname,other_resources))
        for file in other_resources:
            wheretofind = os.path.join(os.path.dirname(os.path.abspath(self.srcfilename)),file)
            tinyfile.write(wheretofind,file)
        tinyfile.close()

    def write_QTI_in_banks(self,filename="",mathasimg = False):
        myqti = imsqtiwriter.QTI_initialize()
        old_selection = self._questions_selected
        other_resources = []
        for bankindex in range(len(self._bank_sizes)):
            self.deselect_all()
            self.select_by_bank_id(bankindex)
            banktitle = self.bank_title(bankindex)
            if len(self._questions_selected) > 0:
                bankformalident = "B" + str(bankindex).zfill(3)
                # Canvas seems to only allow naming banks by their ident so we
                # Manually put banktitle in the first spot below to make this work
                if banktitle != "":
                    bankformalident = banktitle + " " + bankformalident
                mybank = imsqtiwriter.QTI_new_bank(myqti,bankformalident,banktitle)
                for index in range(len(self)):
                    question = self[index]
                    imagecode = question.HTML_get("image")
                    basics = ["B" + str(question.bank_id).zfill(3), question.bank_title, "Q" + str(question.index_in_bank).zfill(3), question.title, question.type,imagecode + question.HTML_statement(mathasimg)]
                    all_choices_raw = question.HTML_all_choices(mathasimg)
                    correct_choices_raw = question.HTML_correct_choices(mathasimg)
                    all_choices_fmt = []
                    correct_choices_fmt = []
                    for index in range(len(all_choices_raw)):
                        package = ["R" + str(index+1).zfill(3),all_choices_raw[index]]
                        all_choices_fmt.append(package)
                        if all_choices_raw[index] in correct_choices_raw:
                            correct_choices_fmt.append(package[0])
                    reactions = [question.HTML_get("comments",mathasimg),question.HTML_get("praise",mathasimg),question.HTML_get("feedback",mathasimg)]
                    questiondata = [basics,all_choices_fmt,correct_choices_fmt,reactions]
                    imsqtiwriter.QTI_add_question(mybank,questiondata)
                    pictureplaces = [question.get("image"),question.statement,question.get("praise"),question.get("feedback"),question.get("comments")] + question.all_choices
                    for codeblock in pictureplaces:
                        searchpattern = re.compile(r"\\includegraphics")
                        for image in re.finditer(searchpattern,codeblock):
                            associated_content = bracket_exprs(codeblock,image.end(0),len(codeblock),[["[","{"],["}","]"]])
                            other_resources.append(associated_content[-1])

        namebreak = os.path.splitext(os.path.basename(self.srcfilename))
        if len(namebreak) > 0:
            givenname = namebreak[0]
        else:
            givenname = ""
        if filename == "":
            if givenname == "":
                raise Exception("Cannot write zip file with empty filename")
            filename = givenname + ".zip"
        basename = os.path.splitext(os.path.basename(filename))[0]
        xmlname = basename + "_qti.xml"
        print "INFO  : Writing " + filename
        tinyfile = zipfile.ZipFile(filename,"w")
        tinyfile.writestr(xmlname,imsqtiwriter.QTI_generate_XML(myqti))
        tinyfile.writestr("imsmanifest.xml",imsqtiwriter.QTI_generate_manifest("autogenerated",basename,xmlname,other_resources))
        for file in other_resources:
            wheretofind = os.path.join(os.path.dirname(os.path.abspath(self.srcfilename)),file)
            tinyfile.write(wheretofind,file)
        tinyfile.close()
        self._questions_selected = old_selection




rawquestion = QuestionGem()
rawquestion.set("feedback","This is the feedback")
rawquestion.statement = "This is the statement"
rawquestion.type = "multiple_choice_question"
rawquestion.all_choices = ["First choice"]
print rawquestion.to_LaTeX()
quit()
# Currently ._environments does try to see if a  _choice_provider has source.

file = LaTeXQuestions("handbank.tex")
file.tikz_compile()
file.write_LaTeX("result.tex")
file.write_QTI_in_banks("bigtest.zip",True)


for bankid in range(file.total_banks()):
    file.deselect_all()
    file.select_by_bank_id(bankid)
    file.tikz_compile()
    newname = re.sub(" ","_",file.bank_title(bankid)) + ".zip"
    newname = re.sub("[:,]","",newname)
    file.write_QTI(newname,file.bank_title(bankid))


file.deselect_all()
file.select_by_search("keywords","sequence convergence|ODEs")
#file.select_by_bank_id(17)
file.tikz_compile()
#file[0].append("keywords","it works")
