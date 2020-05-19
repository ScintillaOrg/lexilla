#!/usr/bin/env python3
# LexillaGen.py - implemented 2019 by Neil Hodgson neilh@scintilla.org
# Released to the public domain.

# Regenerate the Lexilla source files that list all the lexers.
# Should be run whenever a new lexer is added or removed.
# Requires Python 3.6 or later
# Files are regenerated in place with templates stored in comments.
# The format of generation comments is documented in FileGenerator.py.

import pathlib, sys

sys.path.append(str(pathlib.Path(__file__).resolve().parent.parent.parent / "scripts"))

from FileGenerator import Regenerate, UpdateLineInFile, \
    ReplaceREInFile, UpdateLineInPlistFile, ReadFileAsList, UpdateFileFromLines, \
    FindSectionInList
import ScintillaData

def RegenerateAll(rootDirectory):

    root = pathlib.Path(rootDirectory)

    scintillaBase = root.resolve()

    sci = ScintillaData.ScintillaData(scintillaBase)

    lexillaDir = scintillaBase / "lexilla"
    srcDir = lexillaDir / "src"

    Regenerate(srcDir / "Lexilla.cxx", "//", sci.lexerModules)
    Regenerate(srcDir / "lexilla.mak", "#", sci.lexFiles)

    # Discover version information
    version = (lexillaDir / "version.txt").read_text().strip()
    versionDotted = version[0] + '.' + version[1] + '.' + version[2]
    versionCommad = versionDotted.replace(".", ", ") + ', 0'

    rcPath = srcDir / "LexillaVersion.rc"
    UpdateLineInFile(rcPath, "#define VERSION_LEXILLA",
        "#define VERSION_LEXILLA \"" + versionDotted + "\"")
    UpdateLineInFile(rcPath, "#define VERSION_WORDS",
        "#define VERSION_WORDS " + versionCommad)

if __name__=="__main__":
    RegenerateAll(pathlib.Path(__file__).resolve().parent.parent.parent)
