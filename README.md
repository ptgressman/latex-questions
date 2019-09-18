# latex-questions
Standardizing exam-type questions in LaTeX

This is a first attempt to create a simple, portable format for exam-type mathematics questions in LaTeX. Future work will include tools to convert question banks into a variety of formats (with QTI files importable as Canvas quiz banks and Ximera worksheets being the two highest priorities).

The file questions.sty is the LaTeX style file (you should put a copy somewhere that LaTeX can find it, e.g., in the same place as your source).

The file standardized.tex is a file that demonstrates syntax and capabilities. It *should* compile with no errors (unless future updates to various auxiliary LaTeX packages break it) and be able to handle any of the options and question banking features detailed in the comments found inside the file.
