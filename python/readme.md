# `LaTeXQuestions` Basic Information

### `LaTeXQuestions(filename='',verbose=True,options='system|user|online')`
Opens the LaTeX file with the specified string `filename` and generates a LaTeXQuestions object. The string `options` is a list of possible options separated by a vertical bar `|`. Possible items to include are:
- `system` to expand macros like `\bigmath` which are found in the LaTeX style file questions.sty
- `user` to expand user-defined `\newcommand` macros
- `offline` to include text in the source file inside `\offline{...}`
- `online` to include text in the source file inside `\online{...}`
By default, all questions within the file are selected and will be processed by later commands.
If `filename=''` then this call creates a new empty object with no questions
### `tikz_compile()`
This processes the currently-selected questions in the associated `LaTeXQuestions` object to compile tikz pictures and replace them with static image output. There are several items inside the `LaTeXQuestions` class that allow customization.
- `_tikz_compile_latex_template` is a template LaTeX document for a standalone tikzpicture. The precise tikzpicture code will be inserted inside `\begin{image}...\end{image}`
- `_tikz_compile_begin_format` is the file extension that should be used for saving the template
- `_tikz_compile_script` is what it sounds like--a list of system commands to produce an image file of whatever type you wish from a given standalone tikzpicture. The elements `[directory]` and `[filename]` will be replaced with the proper directory and filename, respectively, of whichever tikzpicture is currently being produced
- `_tikz_compile_local_directory` is a directory name relative to the LaTeX source location where the produced images should be stored
- `_tikz_compile_latex_replacement` is the LaTeX code that should replace the original tikzpicture once compiled.

## Selection Functions

### `select_all()`
### `deselect_all()`
These select and deselect, respectively, all questions in the LaTeXQuestions object. Questions not selected will not be processed or output by any subsequent instructions, etc.
### `select_mode(string)`
- "reset" means that every selection operation will start over from scratch and wipe previous selection. This is the default state
- "increasing" preserves the current selections so that further selections add to existing
- "decreasing" operates only on the current selections, so further selections retain only those matching the new criteria.
### `select_by_bank_id(int)`
Selects questions from the bank with the given id number in the file (0 = not banked; all other banks are numbered in order of appearance)
### `select_by_search(envname,regexp)`
Selects questions whose environment of the name `envname` matches the `regexp` search. Any previously-selected questions are still selected.
### `select_by_rules(rule_list)`
Rule list should be a list of triples: `[envname,regexp,bool]` which applies the given regexp search on the named environment. Results must match the given boolean. Questions are selected which match all the criteria in `rule_list`

## Output Options

### `write_LaTeX(filename)`
Writes a LaTeX file containing the currently-selected questions in this LaTeXQuestions object.
### `write_QTI(filename="",banktitle="Unnamed Bank",mathasimg=False)`
Writes the selected questions in a QTI zip file in a single bank of the given title. If `mathasimg` is `True`, it will generate images for all math objects of the sort that work for "old" Canvas quizzes. For "new" Canvas quizzes, `mathasimg` should be `False`, and MathJax-type HTML is produced. If `filename` is empty, it builds a name similar to the source file name.
### `write_QTI_in_banks(filename="",mathasimg=False)`
Writes a single QTI zip file in which questions are sorted into multiple banks. This works for "old" Canvas quizzes but so far
does not seem to work for "new" Canvas quizzes.

## Question Management

### `total_banks()`
Gives the number of question banks in this `LaTeXQuestions` object
### `bank_title(index)`
Gives the title of the bank with numerical index `index`
### `len(LaTeXQuestions)`
Returns the number of selected questions
### `LaTeXQuestions[index]`
Gives access to the selected `QuestionGem` with given `index`
### `new_bank(bank_title="Unnamed Bank")`
Creates a new bank with no questions and opens it to receive new questions. By default, the "Unbanked Questions" bank is open.
### `append(QuestionGem,bankno = None)`
Adds the question `QuestionGem` to bank number `bankno`. If `bankno` is not specified, questions are added to the currently open bank. Appended questions are automatically selected for future output.
### `merge_dupicates(LaTeXQuestions)`
Among selected questions in the `LaTeXQuestions` variable, if any has the same title and question type as a selected question in the original `LaTeXQuestions` object on which `merge_duplicates` is run, the original version will be replaced. In the variable `LaTeXQuestions` object, any questions which were merged with the original are deselected (for easy outputting of things that were not merged).
### `random_choice_reduction(totalopts=6,seedewith="gamma")`
Used to reduce the number of multiple choice options to equal `totalopts`. It will always retain consecutive choices in your original  `all_choices` (see below) and will attempt to arrange for equal numbers of questions with a given answer id (equal numbers of As,Bs,...Fs by default with six choices). A useful way to use this is to generate many more multiple choice answers than needed when writing multiple choice questions and then let LaTeXQuestions balance the answers for you.

# `QuestionGem` Basic Information

### `QuestionGem(latexstring='')`
Creates a question object parsed from `latexstring`. If the string is empty, it creates a question with no content.

### `get(objectname)`
Gets the property with specified name `objectname`. Options are
- `type` = question type. Possibilities are `multiplechoice`, `shortanswer`, `fileupload`, and `essay`
- `title` = question title
- `statement` = the question statement
- `all_choices` = list of LaTeX `\choice` entries associated with the question
- `correct_choices` = list of those `\choice` entries which are marked or otherwise known to be correct
- other relevant environments, like `image`, `keywords`, `praise`, `comments`, and `feedback`. Cannot be used to directly access `multiplechoice` or `shortanswer` environment contents: use `all_choices` instead.

### `set(objectname,contents,options)`
Sets `objectname` (same possibilities as for `get`) to  `contents` with `options` being the LaTeX options associated with the environment.

