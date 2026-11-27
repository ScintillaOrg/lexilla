# Contributing

Lexilla is on GitHub. https://github.com/ScintillaOrg/lexilla

Bugs, fixes and features may be posted to the Issue Tracker.
https://github.com/ScintillaOrg/lexilla/issues

Fixes may be sent as diff or patch files or as a zip archive containing all the
changed files.

Git patch files are best as they include author information and commit
messages.

Code should follow the style guidelines on the Code Style page.
https://www.scintilla.org/SciCoding.html

Pull requests can be posted to this page.
https://github.com/ScintillaOrg/lexilla/pulls

## LLM created contributions

Open source communities and society as a whole are working through the legal,
ethical, and practical effects of LLM technology.

For now, LLM generated or assisted contributions are not accepted to Lexilla.

## Tests

Patches should include test cases.
Including test cases ensures that the change won't be undone by
other changes in the future and clarifies the intentions of the author.

Add a test case file in 
`lexilla/test/examples/<language>` and run the `TestLexers` program in 
`lexilla/test`.

The result will be new files with `.styled.new` and `.folded.new`
appended containing lexing or folding results.

For lexing, there are brace surrounded style numbers for each style
start as markup:

~~~
{5}import{0} {11}contextlib{0}
~~~

For folding, there are 4 columns of folding results preceding the example text:

~~~
2 400   0 + --[[ coding:UTF-8
0 402   0 | comment ]]
~~~

See the `test/README` file for more explanation of the folding results.

## Building

To build lexilla and run the tests with gcc there are Windows batch
and Unix shell files `scripts/RunTest.bat` and `scripts/RunTest.sh`.

On Windows, there are Visual Studio project files. 
`src/Lexilla.vcxproj` builds Lexilla and
`test/TestLexers.vcxproj` builds tests.

After running `TestLexers`, check the result of the `.new` files and, if correct,
rename replacing `.styled.new` with `.styled` and `.folded.new` with `.folded`.

On Windows, the `scripts/PromoteNew.bat` script will rename the `.new` files.

Run the tests again and success should be reported.

Include the `.styled` and `.folded` files in the patch.

## Questions

Questions should go to the scintilla-interest mailing list
https://groups.google.com/forum/#!forum/scintilla-interest

Lexilla is now on GitHub so use its facilities rather than the former
SourceForge host.

The neilh @ scintilla.org account receives much spam and is only checked
occasionally. Almost all Lexilla mail should go to the mailing list.

## Scintilla

Lexilla may be used to provide syntax styling to the related *Scintilla*
text editing component.

Scintilla is on SourceForge and issues with it should be reported there.
https://sourceforge.net/projects/scintilla/
