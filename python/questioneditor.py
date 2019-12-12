import tkinter as tk
from tkinter import messagebox
from tkinter import filedialog
from tkinter import ttk
from latexquestions import *

class LaTeXQuestionsWindow(object):
    def add_choice(self):
        self.ansstates.append(tk.IntVar(self.answerframe))
        self.ansbreaks.append(tk.IntVar(self.answerframe))
        self.ansstates[-1].set(0)
        self.ansbreaks[-1].set(0)
        index = len(self.ansstates)
        T = tk.Text(self.ansframe2,height=4,width=85,highlightthickness=0,wrap='word',undo=True)
        T.grid(row = index*2, rowspan=2,column = 1,columnspan=2,sticky='nsew',pady=3)
        def responder1():
            self.remove_specific_choice(index-1)
        def responder2():
            self.clone_specific_choice(index-1)
        B = ttk.Checkbutton(self.ansframe2,variable=self.ansstates[-1])
        B.grid(row=index*2, rowspan=2,column=0)
        D = ttk.Button(self.ansframe2,text='-',width=1,command=responder1)
        D.grid(row = index*2,column = 4,sticky='w')
        P = ttk.Button(self.ansframe2,text='+',width=1,command=responder2)
        P.grid(row = index*2+1,column = 4,sticky='w')
        E = ttk.Checkbutton(self.ansframe2,variable=self.ansbreaks[-1])
        E.grid(row = index*2,rowspan=2,column=5,sticky='s')
        self.ansobjs.append([T,B,D,P,E])

    def remove_choice(self):
        if len(self.ansobjs) == 0:
            return
        for piece in self.ansobjs[-1]:
            piece.grid_forget()
            piece.destroy()
#        self.ansstates[-1].destroy()
        del self.ansobjs[-1]
        del self.ansstates[-1]
        del self.ansbreaks[-1]

    def set(self,envname,envdat):
        if envname == 'title':
            self.titleobj.delete('1.0',tk.END)
            self.titleobj.insert(tk.END,envdat)
            return
        for item in self.frameobjs:
            if envname == item[0]:
                item[1].delete('1.0',tk.END)
                item[1].insert(tk.END,envdat)
                return
        if envname == 'type':
            self.typeobj.set(envdat)
            return
        if envname == 'selected':
            self.selectobj.set(envdat)
            return
        if envname == 'silent':
            self.silentobj.set(envdat)
            return
        if envname == 'choice_breaks':
            while (len(envdat) > 0) and (max(envdat) > len(self.ansstates)):
                self.add_choice()
            for index in range(len(self.ansbreaks)):
                if index+1 in envdat:
                    self.ansbreaks[index].set(1)
                else:
                    self.ansbreaks[index].set(0)
            return
        if envname == 'choices':
            current_type = self.typeobj.get()
            all_choices = envdat[0]
            correct_choices = envdat[1]
            while len(all_choices) > len(self.ansstates):
                self.add_choice()
            while len(all_choices) < len(self.ansstates):
                self.remove_choice()
            for index in range(len(all_choices)):
                item = self.ansobjs[index][0]
                item.delete('1.0',tk.END)
                item.insert(tk.END,all_choices[index])
                if (all_choices[index] in correct_choices) or (current_type=='shortanswer'):
                    self.ansstates[index].set(1)
                else:
                    self.ansstates[index].set(0)
            return

    def get(self,envname):
        def strip(string):
            retstr = re.sub(r"^\s*","",string)
            return re.sub(r"\s*$","",string)
        if envname == 'title':
            return strip(self.titleobj.get('1.0',tk.END))
        for item in self.frameobjs:
            if envname == item[0]:
                return strip(item[1].get('1.0',tk.END))
        if envname == 'type':
            return self.typeobj.get()
        if envname == 'selected':
            return self.selectobj.get()
        if envname == 'silent':
            return self.silentobj.get()
        if envname == 'choices':
            current_type = self.typeobj.get()
            all_choices = []
            correct_choices = []
            for index in range(len(self.ansobjs)):
                item = self.ansobjs[index][0]
                text = strip(item.get('1.0',tk.END))
                if text != '':
                    all_choices.append(text)
                    if (self.ansstates[index].get() > 0) or (current_type=='shortanswer'):
                        if text not in correct_choices:
                            correct_choices.append(text)
            return [all_choices,correct_choices]
        if envname == 'choice_breaks':
            cbreaks = []
            for index in range(len(self.ansbreaks)):
                val = self.ansbreaks[index].get()
                if val > 0:
                    cbreaks.append(index + 1)
            return cbreaks

    def remove_specific_choice(self,index):
        def strip(string):
            retstr = re.sub(r"^\s*","",string)
            return re.sub(r"\s*$","",string)
        contents = []
        marked = []
        for thisindex in range(len(self.ansobjs)):
            if (thisindex != index):
                localstuff = strip(self.ansobjs[thisindex][0].get('1.0',tk.END))
                contents.append(localstuff)
                if self.ansstates[thisindex].get() == 1:
                    marked.append(localstuff)
        self.set('choices',[contents,marked])
    def clone_specific_choice(self,index):
        def strip(string):
            retstr = re.sub(r"^\s*","",string)
            return re.sub(r"\s*$","",string)
        contents = []
        marked = []
        for thisindex in range(len(self.ansobjs)):
            localstuff = strip(self.ansobjs[thisindex][0].get('1.0',tk.END))
            contents.append(localstuff)
            if self.ansstates[thisindex].get() == 1:
                marked.append(localstuff)
            if thisindex == index:
                contents.append(localstuff)
        self.set('choices',[contents,marked])

    def prev_button(self):
        if self.selectobj.get() == 0:
            if self.latexfileptr not in self.latexunsel:
                self.latexunsel.append(self.latexfileptr)
        else:
            if self.latexfileptr in self.latexunsel:
                self.latexunsel.remove(self.latexfileptr)
        self.store(True)
        if self.latexfileptr > 0:
            self.latexfileptr -= 1
            self.populate(True)
    def next_button(self):
        if self.selectobj.get() == 0:
            if self.latexfileptr not in self.latexunsel:
                self.latexunsel.append(self.latexfileptr)
        else:
            if self.latexfileptr in self.latexunsel:
                self.latexunsel.remove(self.latexfileptr)
        self.store(True)
        if self.latexfileptr < len(self.latexfile)-1:
            self.latexfileptr += 1
            self.populate(True)


    def populate(self,separator=False,question=None):
        if question is None:
            question = self.latexfile[self.latexfileptr]
        pieces = ['title','type','statement','image','praise','feedback','comments','keywords','notes','choice_breaks','answer']
        for item in pieces:
            self.set(item,question.get(item))
        self.set('choices',[question.get('all_choices'),question.get('correct_choices')])
        if separator:
            for item in self.frameobjs:
                item[1].edit_reset()
            for item in self.ansobjs:
                item[0].edit_reset()
        if question.get('silent'):
            self.silentobj.set(1)
        else:
            self.silentobj.set(0)
        if self.latexfileptr in self.latexunsel:
            self.selectobj.set(0)
        else:
            self.selectobj.set(1)
        self.infobj.config(text=str(self.latexfileptr + 1) + " of " + str(len(self.latexfile)))
        self.backuplabel.configure(text='remaining: ' + str(len(self.latexfile[self.latexfileptr]._backups)))

    def store(self,alsobackup=False):
        pieces = ['title','type','statement','image','praise','feedback','comments','keywords','notes','choice_breaks','answer']

        for item in pieces:
            self.latexfile[self.latexfileptr].set(item,self.get(item))
        choices = self.get('choices')
        self.latexfile[self.latexfileptr].set('all_choices',choices[0])
        self.latexfile[self.latexfileptr].set('correct_choices',choices[1])

        value = self.get('silent')
        if (value > 0):
            self.latexfile[self.latexfileptr].set('silent',True)
        else:
            self.latexfile[self.latexfileptr].set('silent',False)
        self.infobj.config(text=str(self.latexfileptr + 1) + " of " + str(len(self.latexfile)))
        if (len(self.mymacrowindow) >= 3):
            string = self.mymacrowindow[2].get('1.0',tk.END)
            self.latexfile.read_macros(string)
        if alsobackup:
            self.latexfile[self.latexfileptr].backup(True)

    def open_routine(self,extrainfo=None):
        filename = filedialog.askopenfilename(initialdir = self.prevopendir,title = "Select file",filetypes = (("LaTeX files","*.tex"),("all files","*.*")))
        if filename != "":
            self.prevopendir = os.path.dirname(filename)
            shortfile = os.path.basename(filename)
            self.myroot.title('LTQ: ' + shortfile)
            self.latexfile = LaTeXQuestions(filename,False,'')
            for index in range(len(self.latexfile)):
                self.latexfile[index].backup()
            self.latexfileptr = 0
            self.latexunsel = []
            self.populate()
            if len(self.mymacrowindow) >= 3:
                string = self.latexfile.write_macros()
                self.mymacrowindow[2].delete('1.0',tk.END)
                self.mymacrowindow[2].insert(tk.END,string)
        self.myroot.focus_set()
        return
    def save_as_routine(self,extrainfo=None):
        self.store(True)
        filename = filedialog.asksaveasfilename(initialdir = self.prevsavedir,title = "Save as",filetypes = (("LaTeX files","*.tex"),("all files","*.*")))
        if filename != "":
            self.prevsavedir = os.path.dirname(filename)
            self.latexfile.select_all()
            selectrange = []
            for index in range(len(self.latexfile)):
                if index not in self.latexunsel:
                    selectrange.append(index)
            self.latexfile._questions_selected = selectrange
            self.latexfile.write_LaTeX(filename)
        self.myroot.focus_set()
        return

    def preview(self,extrastuff=None):
        if len(self.preview_windows) == 0:
            wndw = tk.Toplevel()
            def on_close():
                self.preview_windows[0].destroy()
                self.preview_windows = []
            wndw.protocol("WM_DELETE_WINDOW", on_close)
            wndw.geometry('612x792')
        else:
            wndw = self.preview_windows[0]
            for index in range(len(self.preview_windows)):
                if index > 0:
                    item = self.preview_windows[index]
                    item.grid_forget()
                    item.destroy()
        self.preview_windows = [wndw]

        tempsel = self.latexfile._questions_selected
        self.store(True)
        self.latexfile._questions_selected = [self.latexfileptr]
        self.latexfile.write_LaTeX('temptex.tex')
        self.latexfile._questions_selected = tempsel
#        compile_script = ["rm temptex.gif","pdflatex -interaction=nonstopmode temptex.tex","sips -s format gif -Z 1100 temptex.pdf --out temptex.gif"]
        compile_script = ["rm temptex.gif","pdflatex -interaction=nonstopmode temptex.tex","pdftoppm -rx 72 -ry 72 temptex.pdf temptex"]
        for line in compile_script:
            os.system(line)

        img = tk.PhotoImage(file="temptex-1.ppm")
        label = tk.Label(wndw,image=img)
        label.image = img
        label.grid(row = 0,column=0,sticky='nsew')
        self.preview_windows.append(label)

    def open_findrepl(self):
        if len(self.findreplace_windows) > 0:
            self.findreplace_windows[0].lift()
            return
        if len(self.findreplace_windows) == 0:
            wndw = tk.Toplevel()
            wndw.resizable(0,0)
            def on_close():
                self.findreplace_windows[0].destroy()
                self.findreplace_windows = []
            wndw.protocol("WM_DELETE_WINDOW", on_close)
            frame = ttk.Frame(wndw)
            frame.grid(row=0,column=0,sticky='news')
            self.findreplace_windows = [wndw,frame]
        ttk.Label(frame,text='Environment:').grid(row=0,column=0,sticky='w')
        envoptions = ['','statement','all_choices','image','answer','praise','feedback','comments','keywords','notes']
        envstringvar = tk.StringVar(frame)
        envstringvar.set('statement')
        typewdg = ttk.OptionMenu(frame,envstringvar,*envoptions)
        typewdg.grid(row=0,column=1,padx=5,pady=5,sticky='ew')
#        txtbox0 = tk.Text(frame,height=1,width=60,borderwidth=1,relief='sunken',highlightthickness=0,wrap='word',undo=True)
#        txtbox0.grid(row=1,column = 0,columnspan=6,padx=5,pady=5)
        ttk.Label(frame,text='Literal Search:').grid(row=2,column=0,sticky='w')
        txtbox1 = tk.Text(frame,height=4,width=60,borderwidth=1,relief='sunken',highlightthickness=0,wrap='word',undo=True)
        txtbox1.grid(row=3,column = 0,columnspan=6,padx=5,pady=5)
        ttk.Label(frame,text='Regexp Search:').grid(row=4,column=0,sticky='w')
        txtbox2 = tk.Text(frame,height=4,width=60,borderwidth=1,relief='sunken',highlightthickness=0,wrap='word',undo=True)
        txtbox2.grid(row=5,column = 0,columnspan=6,padx=5,pady=5)
        ttk.Label(frame,text='Regexp Replace:').grid(row=6,column=0,sticky='w')
        txtbox3= tk.Text(frame,height=4,width=60,borderwidth=1,relief='sunken',highlightthickness=0,wrap='word',undo=True)
        txtbox3.grid(row=7,column = 0,columnspan=6,padx=5,pady=5)
        self.findreplace_windows.append([envstringvar,txtbox1,txtbox2,txtbox3,typewdg,envoptions])
        def findit(forward = True):
            def strip(string):
                retstr = re.sub(r"^\s*","",string)
                return re.sub(r"\s*$","",string)
            envname = strip(self.findreplace_windows[2][0].get())
            literal = strip(self.findreplace_windows[2][1].get('1.0',tk.END))
            regexp = strip(self.findreplace_windows[2][2].get('1.0',tk.END))
            if literal != '':
                regexp = re.escape(literal)
            if regexp == '': #Can't find nothing
                return
            foundbefore = False
            beforeind = -1
            foundafter = False
            afterind = -1
            indexrange = range(len(self.latexfile))
            if forward == False:
                indexrange.reverse()
            for index in indexrange:
                if ((index < self.latexfileptr) and not foundbefore) or ((index > self.latexfileptr) and not foundafter):
                    text = self.latexfile[index].get(envname)
                    if envname=='all_choices':
                        flattext = ""
                        for item in text:
                            flattext += item + "\n\n"
                        text = flattext
                    if re.search(regexp,text):
                        if (index < self.latexfileptr):
                            foundbefore = True
                            beforeind = index
                        elif (index > self.latexfileptr):
                            foundafter = True
                            afterind = index
            if forward:
                if foundafter:
                    newind = afterind
                elif foundbefore:
                    newind = beforeind
                else:
                    return
            else:
                if foundbefore:
                    newind = beforeind
                elif foundafter:
                    newind = afterind
                else:
                    return
            self.store(True)
            self.latexfileptr = newind
            self.populate()
        def replaceit():
            def strip(string):
                retstr = re.sub(r"^\s*","",string)
                return re.sub(r"\s*$","",string)
            envname = strip(self.findreplace_windows[2][0].get())
            literal = strip(self.findreplace_windows[2][1].get('1.0',tk.END))
            regexp = strip(self.findreplace_windows[2][2].get('1.0',tk.END))
            repl = strip(self.findreplace_windows[2][3].get('1.0',tk.END))
            if literal != '':
                regexp = re.escape(literal)
            if regexp == '': #Can't find nothing
                return
            if envname != 'all_choices':
                text = self.get(envname)
                self.set(envname,re.sub(regexp,repl,text))
                return
            bundle = self.get('choices')
            newans = []
            newcor = []
            for item in bundle[0]:
                newans.append(re.sub(regexp,repl,item))
            for item in bundle[1]:
                newcor.append(re.sub(regexp,repl,item))
            self.set('choices',[newans,newcor])

        def find_next():
            findit(True)
        def find_prev():
            findit(False)
        def replacenext():
            replaceit()
            findit(True)
        def replaceall():
            startingpoint = self.latexfileptr
            replaceit()
            findit(True)
            if (self.latexfileptr == startingpoint):
                return
            donealready = [startingpoint]
            while self.latexfileptr not in donealready:
                replaceit()
                donealready.append(self.latexfileptr)
                findit(True)
        ttk.Button(frame,text='Find Next',command=find_next).grid(row=8,column=1,sticky='ew')
        ttk.Button(frame,text='Find Prev',command=find_prev).grid(row=8,column=0,sticky='ew')
        ttk.Button(frame,text='Replace',command=replaceit).grid(row=8,column=2,sticky='ew')
        ttk.Button(frame,text='Replace & Next',command=replacenext).grid(row=9,column=1,sticky='ew')
        ttk.Button(frame,text='Replace All',command=replaceall).grid(row=9,column=0,sticky='ew')

    def macro_window(self):
        if len(self.mymacrowindow) > 0:
            self.mymacrowindow[0].lift()
            return

        wndw = tk.Toplevel()
        wndw.resizable(0,0)
        def on_close():
            string = self.mymacrowindow[2].get('1.0',tk.END)
            self.latexfile.read_macros(string)
            self.mymacrowindow[0].destroy()
            self.mymacrowindow = []
        wndw.protocol("WM_DELETE_WINDOW", on_close)
        frame = ttk.Frame(wndw)
        frame.grid(row=0,column=0,sticky='news')
        ttk.Label(frame,text='Custom LaTeX Macros:').grid(row=0,column=0,sticky='w')
        self.mymacrowindow = [wndw,frame]
        mytxt = tk.Text(frame,height=15,width=50,borderwidth=1,relief='sunken',highlightthickness=0,wrap='word',undo=True)
        self.mymacrowindow.append(mytxt)
        mytxt.grid(row=1,column=0,sticky='news')
        string = self.latexfile.write_macros()
        mytxt.delete('1.0',tk.END)
        mytxt.insert(tk.END,string)

    def __init__(self,root):
        self.myroot = root
        root.title('LaTeXQuestions')
        root.resizable(0,0)
        s=ttk.Style()
        s.theme_use('aqua')
        def newlatexfile():
            self.latexfile = LaTeXQuestions('',False,'')
            self.latexfile.append(QuestionGem(''))
            self.latexfileptr = 0
            self.latexunsel = []
        def totallynewlatexfile():
            goahead = True
            if (len(self.latexfile) > 1):
                goahead = tkMessageBox.askokcancel('Open New?','Discard work and open new?')
            if goahead:
                newlatexfile()
                self.populate(True)
        newlatexfile()
        self.prevopendir = '.'
        self.prevsavedir = '.'
        self.preview_windows = []
        self.findreplace_windows = []
        self.mymacrowindow = []
        menu = tk.Menu(self.myroot)
        file = tk.Menu(menu)
        file.add_command(label='New',command=totallynewlatexfile)
        file.add_command(label='Open...',command=self.open_routine)
        file.add_command(label='Save as...',command=self.save_as_routine)
        menu.add_cascade(label='File',menu=file)
        edit = tk.Menu(menu)
        edit.add_command(label='Find/Replace...',command=self.open_findrepl)
        edit.add_command(label='Macros...',command=self.macro_window)
        menu.add_cascade(label='Edit',menu=edit)
        self.myroot.config(menu=menu)
        self.myroot.bind("<Command-o>", self.open_routine)
        self.myroot.bind("<Command-s>", self.save_as_routine)
        self.myroot.bind("<Command-p>", self.preview)

        def grabit(extrastuff):
            self.myroot.clipboard_clear()
            self.myroot.clipboard_append(self.latexfile[self.latexfileptr].to_LaTeX())
            self.myroot.update()
        self.myroot.bind("<Command-g>", grabit)

        self.myframe = ttk.Frame(root)
        self.myframe.grid(row = 0, column = 0)
        root = self.myframe

        ttk.Label(root,text='Title').grid(row = 0, column = 0,sticky='e')

        self.titleobj = tk.Text(root,height=1,width=20,borderwidth=1,relief='sunken',highlightthickness=0,wrap='word',undo=True)
        self.titleobj.grid(row = 0,column = 1,columnspan=5,sticky='ew')

        self.prevobj = ttk.Button(root,text='< Prev',command=self.prev_button)
        self.prevobj.grid(row = 0,column = 6,sticky='ew')
        self.nextobj = ttk.Button(root,text='Next >',command=self.next_button)
        self.nextobj.grid(row = 0,column = 7,sticky='ew')

        ttk.Button(root,text='Compile',command=self.preview).grid(row=1,column=5,sticky='nsew')

        self.infobj = ttk.Label(root,text='1 of 1')
        self.infobj.grid(row=1,column=6)
        ttk.Label(root,text='Type').grid(row = 1, column = 1,sticky='e')
        self.typeobj = tk.StringVar(root)
        self.typeoptions = ['','multiplechoice','shortanswer','fileupload','essay']
        self.typeobj.set('multiplechoice')
        typewdg = ttk.OptionMenu(root,self.typeobj,*self.typeoptions)
        typewdg.configure(width=11)
        #typewdg.config(width=10)
        typewdg.grid(row = 1, column = 2,sticky='ew')
        self.selectobj = tk.IntVar(root)
        self.selectobj.set(1)
        ttk.Checkbutton(root, text="Selected",variable=self.selectobj).grid(row=1, column = 7)

        self.silentobj = tk.IntVar(root)
        ttk.Checkbutton(root, text="Silent",variable=self.silentobj).grid(row=1, column = 3,sticky='w')

        def backupq():
            self.store()
            self.latexfile[self.latexfileptr].backup()
            self.populate()
        def restoreq():
            self.latexfile[self.latexfileptr].restore()
            self.populate()

        ttk.Button(root,text='Backup',command=backupq).grid(row=3,column=0,sticky='ew')
        ttk.Button(root,text='Restore',command=restoreq).grid(row=3,column=1,sticky='ew')
        self.backuplabel=ttk.Label(root,text='remaining: 0')
        self.backuplabel.grid(row=3,column=2,sticky='w')
        def deleteq():
            if len(self.latexfile) == 0:
                return
            if len(self.latexfile) == 1:
                self.latexfile.delete(0)
                self.latexfile.append(QuestionGem(''))
                self.latexfileptr = 0
                self.populate(True)
                return
            self.latexfile.delete(self.latexfileptr)
            if (self.latexfileptr >= len(self.latexfile)):
                self.latexfileptr = len(self.latexfile) - 1
            self.populate(True)

        def newq(extrastuff=None):
            self.store(True)
            self.latexfile.insert(self.latexfileptr+1,QuestionGem(''))
            self.latexfileptr += 1
            self.populate(True)
        def dupeq():
            self.store(True)
            question = copy.deepcopy(self.latexfile[self.latexfileptr])
            self.latexfile.insert(self.latexfileptr+1,question)
            self.latexfileptr += 1
            self.populate(True)
            mytitle = self.get('title') + " copy"
            self.set('title',mytitle)
        def remapq():
            self.store(True)
            tempq = QuestionGem(self.latexfile[self.latexfileptr].to_LaTeX())
            self.populate(False,tempq)
            self.store(True)

        self.myroot.bind("<Command-n>",newq)
        ttk.Button(root,text='Delete',command=deleteq).grid(row=3,column=7,sticky='ew')

        ttk.Button(root,text='New',command=newq).grid(row=3,column=3,sticky='ew')
        ttk.Button(root,text='Duplicate',command=dupeq).grid(row=3,column=4,sticky='ew')
        ttk.Button(root,text='Remap',command=remapq).grid(row=3,column=5,sticky='ew')

        self.notebook = ttk.Notebook(root)
        self.notebook.grid(row=2,column=0,columnspan=8,sticky='news')

        framedata = [['statement'],['image'],['answer'],['praise'],['feedback'],['comments'],['keywords'],['notes']]
        self.frameobjs = []
        for data in framedata:
            frame = ttk.Frame(self.notebook)
            T = tk.Text(frame,height=30,width=101,highlightthickness=0,wrap='word',undo=True)
            T.grid(row = 0, column = 0,columnspan=2,sticky='ns')
            S = ttk.Scrollbar(frame)
            S.grid(row = 0, column = 2,sticky='ns')
            S.config(command=T.yview)
            T.config(yscrollcommand=S.set)
            self.notebook.add(frame,text = data[0])
            self.frameobjs.append([data[0],T])

        self.answerframe = ttk.Frame(self.notebook)
        self.ansstates = []
        self.ansobjs = []
        self.ansbreaks = []

        self.ansaddbtn = ttk.Button(self.answerframe,text='+choice',command=self.add_choice)
        self.ansaddbtn.grid(row = 0,column = 2)


        canvas1 = tk.Canvas(self.answerframe)
        self.ansframe2 = ttk.Frame(canvas1)
        myscrollbar=ttk.Scrollbar(self.answerframe,orient="vertical",command=canvas1.yview)
        canvas1.configure(yscrollcommand=myscrollbar.set)
        myscrollbar.grid(column=9,row=4,sticky='ns')
        canvas1.grid(column=1,row=4,columnspan=8,sticky='nsew')
        canvas1.create_window((0,0),window=self.ansframe2,anchor='nw')
        def myfunction(event):
            canvas1.configure(scrollregion=canvas1.bbox("all"),width=695,height=445,bg='#ECECEC',highlightthickness=0)
        self.ansframe2.bind("<Configure>",myfunction)

        for index in range(1):
            self.add_choice()

        self.notebook.insert(1,self.answerframe,text = 'all_choices')
        self.notebook.enable_traversal()

root = tk.Tk()
root.resizable(0,0)
mywindow = LaTeXQuestionsWindow(root)


tk.mainloop()
