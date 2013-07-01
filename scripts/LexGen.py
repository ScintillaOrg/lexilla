#!/usr/bin/env python
# LexGen.py - implemented 2002 by Neil Hodgson neilh@scintilla.org
# Released to the public domain.

# Regenerate the Scintilla and SciTE source files that list
# all the lexers and all the properties files.
# Should be run whenever a new lexer is added or removed.
# Requires Python 2.4 or later
# Most files are regenerated in place with templates stored in comments.
# The VS .NET project file is generated into a different file as the
# VS .NET environment will not retain comments when modifying the file.
# The format of generation comments is documented in FileGenerator.py.

import string
import sys
import os
import glob
import codecs
import datetime

from FileGenerator import UpdateFile, Generate, Regenerate, UpdateLineInFile, lineEnd

def FindModules(lexFile):
    modules = []
    with open(lexFile) as f:
        for l in f.readlines():
            if l.startswith("LexerModule"):
                l = l.replace("(", " ")
                modules.append(l.split()[1])
    return modules

# Properties that start with lexer. or fold. are automatically found but there are some
# older properties that don't follow this pattern so must be explicitly listed.
knownIrregularProperties = [
    "fold",
    "styling.within.preprocessor",
    "tab.timmy.whinge.level",
    "asp.default.language",
    "html.tags.case.sensitive",
    "ps.level",
    "ps.tokenize",
    "sql.backslash.escapes",
    "nsis.uservars",
    "nsis.ignorecase"
]

def FindProperties(lexFile):
    properties = {}
    with open(lexFile) as f:
        for l in f.readlines():
            if ("GetProperty" in l or "DefineProperty" in l) and "\"" in l:
                l = l.strip()
                if not l.startswith("//"):	# Drop comments
                    propertyName = l.split("\"")[1]
                    if propertyName.lower() == propertyName:
                        # Only allow lower case property names
                        if propertyName in knownIrregularProperties or \
                            propertyName.startswith("fold.") or \
                            propertyName.startswith("lexer."):
                            properties[propertyName] = 1
    return properties

def FindPropertyDocumentation(lexFile):
    documents = {}
    with open(lexFile) as f:
        name = ""
        for l in f.readlines():
            l = l.strip()
            if "// property " in l:
                propertyName = l.split()[2]
                if propertyName.lower() == propertyName:
                    # Only allow lower case property names
                    name = propertyName
                    documents[name] = ""
            elif "DefineProperty" in l and "\"" in l:
                propertyName = l.split("\"")[1]
                if propertyName.lower() == propertyName:
                    # Only allow lower case property names
                    name = propertyName
                    documents[name] = ""
            elif name:
                if l.startswith("//"):
                    if documents[name]:
                        documents[name] += " "
                    documents[name] += l[2:].strip()
                elif l.startswith("\""):
                    l = l[1:].strip()
                    if l.endswith(";"):
                        l = l[:-1].strip()
                    if l.endswith(")"):
                        l = l[:-1].strip()
                    if l.endswith("\""):
                        l = l[:-1]
                    # Fix escaped double quotes
                    l = l.replace("\\\"", "\"")
                    documents[name] += l
                else:
                    name = ""
    for name in list(documents.keys()):
        if documents[name] == "":
            del documents[name]
    return documents

def ciCompare(a,b):
    return cmp(a.lower(), b.lower())

def ciKey(a):
    return a.lower()

def sortListInsensitive(l):
    try:    # Try key function
        l.sort(key=ciKey)
    except TypeError:    # Earlier version of Python, so use comparison function
        l.sort(ciCompare)

host = "prdownloads.sourceforge.net/"
def UpdateDownloadLinks(path, version):
    lines = []
    with open(path, "r") as f:
        for l in f.readlines():
            l = l.rstrip()
            if host in l:
                start, prd, rest = l.partition(host)
                pth, dot, ending = rest.partition(".")
                pthNew = pth[:-3] + version.rstrip()
                lineWithNewVersion = start + prd +pthNew + dot + ending
                lines.append(lineWithNewVersion)
            else:
                lines.append(l)
    contents = lineEnd.join(lines) + lineEnd
    UpdateFile(path, contents)

def UpdateVersionNumbers(root):
    with open(root + "scintilla/version.txt") as f:
        version = f.read()
    versionDotted = version[0] + '.' + version[1] + '.' + version[2]
    versionCommad = version[0] + ', ' + version[1] + ', ' + version[2] + ', 0'
    with open(root + "scintilla/doc/index.html") as f:
        dateModified = [l for l in f.readlines() if "Date.Modified" in l][0].split('\"')[3]
        # 20130602
        # index.html, SciTE.html
        dtModified = datetime.datetime.strptime(dateModified, "%Y%m%d")
        yearModified = dateModified[0:4]
        monthModified = dtModified.strftime("%B")
        dayModified = "%d" % dtModified.day
        mdyModified = monthModified + " " + dayModified + " " + yearModified
        # May 22 2013
        # index.html, SciTE.html
        dmyModified = dayModified + " " + monthModified + " " + yearModified
        # 22 May 2013
        # ScintillaHistory.html -- only first should change
        myModified = monthModified + " " + yearModified
        # scite/src/SciTE.h
        #define COPYRIGHT_DATES "December 1998-May 2013"
        #define COPYRIGHT_YEARS "1998-2013"
        dateLine = f.readlines()

    UpdateLineInFile(root + "scintilla/win32/ScintRes.rc", "#define VERSION_SCINTILLA",
        "#define VERSION_SCINTILLA \"" + versionDotted + "\"")
    UpdateLineInFile(root + "scintilla/win32/ScintRes.rc", "#define VERSION_WORDS",
        "#define VERSION_WORDS " + versionCommad)
    UpdateLineInFile(root + "scintilla/qt/ScintillaEditBase/ScintillaEditBase.pro",
        "VERSION =",
        "VERSION = " + versionDotted)
    UpdateLineInFile(root + "scintilla/qt/ScintillaEdit/ScintillaEdit.pro",
        "VERSION =",
        "VERSION = " + versionDotted)
    UpdateLineInFile(root + "scintilla/doc/ScintillaDownload.html", "       Release",
        "       Release " + versionDotted)
    UpdateDownloadLinks(root + "scintilla/doc/ScintillaDownload.html", version)
    UpdateLineInFile(root + "scintilla/doc/index.html",
        '          <font color="#FFCC99" size="3"> Release version',
        '          <font color="#FFCC99" size="3"> Release version ' + versionDotted + '<br />')
    UpdateLineInFile(root + "scintilla/doc/index.html",
        '           Site last modified',
        '           Site last modified ' + mdyModified + '</font>')
    UpdateLineInFile(root + "scintilla/doc/ScintillaHistory.html",
        '	Released ',
        '	Released ' + dmyModified + '.')

    if os.path.exists(root + "scite"):
        UpdateLineInFile(root + "scite/src/SciTE.h", "#define VERSION_SCITE",
            "#define VERSION_SCITE \"" + versionDotted + "\"")
        UpdateLineInFile(root + "scite/src/SciTE.h", "#define VERSION_WORDS",
            "#define VERSION_WORDS " + versionCommad)
        UpdateLineInFile(root + "scite/src/SciTE.h", "#define COPYRIGHT_DATES",
            '#define COPYRIGHT_DATES "December 1998-' + myModified + '"')
        UpdateLineInFile(root + "scite/src/SciTE.h", "#define COPYRIGHT_YEARS",
            '#define COPYRIGHT_YEARS "1998-' + yearModified + '"')
        UpdateLineInFile(root + "scite/doc/SciTEDownload.html", "       Release",
            "       Release " + versionDotted)
        UpdateDownloadLinks(root + "scite/doc/SciTEDownload.html", version)
        UpdateLineInFile(root + "scite/doc/SciTE.html",
            '          <font color="#FFCC99" size="3"> Release version',
            '          <font color="#FFCC99" size="3"> Release version ' + versionDotted + '<br />')
        UpdateLineInFile(root + "scite/doc/SciTE.html",
            '           Site last modified',
            '           Site last modified ' + mdyModified + '</font>')
        UpdateLineInFile(root + "scite/doc/SciTE.html",
            '    <meta name="Date.Modified"',
            '    <meta name="Date.Modified" content="' + dateModified + '" />')

def RegenerateAll():
    root="../../"

    # Find all the lexer source code files
    lexFilePaths = glob.glob(root + "scintilla/lexers/Lex*.cxx")
    sortListInsensitive(lexFilePaths)
    lexFiles = [os.path.basename(f)[:-4] for f in lexFilePaths]
    print(lexFiles)
    lexerModules = []
    lexerProperties = {}
    propertyDocuments = {}
    for lexFile in lexFilePaths:
        lexerModules.extend(FindModules(lexFile))
        for k in FindProperties(lexFile).keys():
            lexerProperties[k] = 1
        documents = FindPropertyDocumentation(lexFile)
        for k in documents.keys():
            if k not in propertyDocuments:
                propertyDocuments[k] = documents[k]
    sortListInsensitive(lexerModules)
    lexerProperties = list(lexerProperties.keys())
    sortListInsensitive(lexerProperties)

    # Generate HTML to document each property
    # This is done because tags can not be safely put inside comments in HTML
    documentProperties = list(propertyDocuments.keys())
    sortListInsensitive(documentProperties)
    propertiesHTML = []
    for k in documentProperties:
        propertiesHTML.append("\t<tr id='property-%s'>\n\t<td>%s</td>\n\t<td>%s</td>\n\t</tr>" %
            (k, k, propertyDocuments[k]))

    # Find all the SciTE properties files
    otherProps = ["abbrev.properties", "Embedded.properties", "SciTEGlobal.properties", "SciTE.properties"]
    if os.path.exists(root + "scite"):
        propFilePaths = glob.glob(root + "scite/src/*.properties")
        sortListInsensitive(propFilePaths)
        propFiles = [os.path.basename(f) for f in propFilePaths if os.path.basename(f) not in otherProps]
        sortListInsensitive(propFiles)
        print(propFiles)

    Regenerate(root + "scintilla/src/Catalogue.cxx", "//", lexerModules)
    Regenerate(root + "scintilla/win32/scintilla.mak", "#", lexFiles)
    if os.path.exists(root + "scite"):
        Regenerate(root + "scite/win32/makefile", "#", propFiles)
        Regenerate(root + "scite/win32/scite.mak", "#", propFiles)
        Regenerate(root + "scite/src/SciTEProps.cxx", "//", lexerProperties)
        Regenerate(root + "scite/doc/SciTEDoc.html", "<!--", propertiesHTML)
        Generate(root + "scite/boundscheck/vcproj.gen",
         root + "scite/boundscheck/SciTE.vcproj", "#", lexFiles)

    UpdateVersionNumbers(root)

RegenerateAll()
