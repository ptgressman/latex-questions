# LaTeXQuestions Basic Information


## `LaTeXQuestions(filename,options='system|user|online')`
Opens the LaTeX file with the specified string `filename` and generates a LaTeXQuestions object. The string `options` is a list of possible options separated by a vertical bar `|`. Possible items to include are:
- `system` to expand macros like `\bigmath` which are found in the LaTeX style file questions.sty
- `user` to expand user-defined `\newcommand` macros
- `offline` to include text in the source file inside `\offline{...}`
- `online` to include text in the source file inside `\online{...}`
By default, all questions within the file are selected and will be processed by later commands.

## `tikz_compile()`
This processes the currently-selected questions in the associated `LaTeXQuestions` object to compile tikz pictures and replace them with static image output. There are several items inside the `LaTeXQuestions` class that allow customization.
- `_tikz_compile_latex_template` is a template LaTeX document for a standalone tikzpicture. The precise tikzpicture code will be inserted inside `\begin{image}...\end{image}`
- `_tikz_compile_begin_format` is the file extension that should be used for saving the template
- `_tikz_compile_script` is what it sounds like--a list of system commands to produce an image file of whatever type you wish from a given standalone tikzpicture. The elements `[directory]` and `[filename]` will be replaced with the proper directory and filename, respectively, of whichever tikzpicture is currently being produced
- `_tikz_compile_local_directory` is a directory name relative to the LaTeX source location where the produced images should be stored
- `_tikz_compile_latex_replacement` is the LaTeX code that should replace the original tikzpicture once compiled.


# Selection Functions

## `select_all()`
## `deselect_all()`
These select and deselect, respectively, all questions in the LaTeXQuestions object. Questions not selected will not be processed or output by any subsequent instructions, etc.

## `select_by_search(envname,regexp)`
Selects questions whose environment of the name `envname` matches the `regexp` search. Any previously-selected questions are still selected.

## `keep_by_search(envname,regexp)`
Among questions already selected, retains only those whose environment of the name `envname` matches the `regexp` search.

# Output Options

## `write_LaTeX(filename)`
Writes a LaTeX file containing the currently-selected questions in this LaTeXQuestions object.
## `write_QTI(filename="",banktitle="Unnamed Bank",mathasimg=False)`
Writes the selected questions in a QTI zip file in a single bank of the given title. If `mathasimg` is `True`, it will generate images for all math objects of the sort that work for "old" Canvas quizzes. For "new" Canvas quizzes, `mathasimg` should be `False`, and MathJax-type HTML is produced. If `filename` is empty, it builds a name similar to the source file name.
## `write_QTI_in_banks(filename="",mathasimg=False)`
Writes a single QTI zip file in which questions are sorted into multiple banks. This works for "old" Canvas quizzes but so far
does not seem to work for "new" Canvas quizzes.

# Other

## total_banks()
Gives the number of question banks in this `LaTeXQuestions` object
## bank_title(index)
Gives the title of the bank with numerical index `index`
## len(LaTeXQuestions)
Returns the number of selected question
## LaTeXQuestions[index]
Gives access to the selected `QuestionGem` with given `index`

## `merge_dupicates(LaTeXQuestions)`
Among selected questions in the `LaTeXQuestions` variable, if any has the same title and question type as a selected question in the original `LaTeXQuestions` object on which `merge_duplicates` is run, the original version will be replaced. In the variable `LaTeXQuestions` object, any questions which were merged with the original are deselected (for easy outputting of things that were not merged).


## `QuestionGem`
- `bank_title` = title of bank from which the question came
- `bank_id` = the id number of the bank from which the question came
- `index_in_bank` = the index of the question itself within its bank
- `type` = which type of question this is. Possbilities are listed in `_question_types` but not all are currently implemented
- `srccode` = original LaTeX source (minus comments and with other small modifications)
- `statement` = the question statement
- `title` = question title
- `all_choices` = list of LaTeX `\choice` entries associated with the question
- `correct_choices` = list of those `\choice` entries which are marked or otherwise known to be correct
- `get(envname)` gets contents of the environment with name `envname`. Cannot be used on `_choice_provider` environments.
- `HTML_get(envname,mathasimg=False)` gets contents of the environment with name `envname` and converts to HTML. Cannot be used on `_choice_provider` environments.
- `set(envname,contents,options)` sets environment with name `envname` to the string `contents` with `options` being the LaTeX options associated with the environment. Cannot be used on `_choice_provider` environments

