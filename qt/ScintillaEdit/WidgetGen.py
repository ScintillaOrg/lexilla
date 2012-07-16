#!/usr/bin/env python
# WidgetGen.py - regenerate the ScintillaWidgetCpp.cpp and ScintillaWidgetCpp.h files
# Check that API includes all gtkscintilla2 functions

import sys
import os
import getopt

scintillaDirectory = "../.."
scintillaIncludeDirectory = os.path.join(scintillaDirectory, "include")
sys.path.append(scintillaIncludeDirectory)
import Face

def Contains(s,sub):
	return s.find(sub) != -1

def underscoreName(s):
	# Name conversion fixes to match gtkscintilla2
	irregular = ['WS', 'EOL', 'AutoC', 'KeyWords', 'BackSpace', 'UnIndents', 'RE', 'RGBA']
	for word in irregular:
		replacement = word[0] + word[1:].lower()
		s = s.replace(word, replacement)

	out = ""
	for c in s:
		if c.isupper():
			if out:
				out += "_"
			out += c.lower()
		else:
			out += c
	return out

def normalisedName(s, options, role=None):
	if options["qtStyle"]:
		if role == "get":
			s = s.replace("Get", "")
		return s[0].lower() + s[1:]
	else:
		return underscoreName(s)

typeAliases = {
	"position": "int",
	"colour": "int",
	"keymod": "int",
	"string": "const char *",
	"stringresult": "const char *",
	"cells": "const char *",
}

def cppAlias(s):
	if s in typeAliases:
		return typeAliases[s]
	else:
		return s

understoodTypes = ["", "void", "int", "bool", "position",
	"colour", "keymod", "string", "stringresult", "cells"]

def checkTypes(name, v):
	understandAllTypes = True
	if v["ReturnType"] not in understoodTypes:
		#~ print("Do not understand", v["ReturnType"], "for", name)
		understandAllTypes = False
	if v["Param1Type"] not in understoodTypes:
		#~ print("Do not understand", v["Param1Type"], "for", name)
		understandAllTypes = False
	if v["Param2Type"] not in understoodTypes:
		#~ print("Do not understand", v["Param2Type"], "for", name)
		understandAllTypes = False
	return understandAllTypes

def arguments(v, stringResult, options):
	ret = ""
	p1Type = cppAlias(v["Param1Type"])
	if p1Type:
		ret = ret + p1Type + " " + normalisedName(v["Param1Name"], options)
	p2Type = cppAlias(v["Param2Type"])
	if p2Type and not stringResult:
		if p1Type:
			ret = ret + ", "
		ret = ret + p2Type + " " + normalisedName(v["Param2Name"], options)
	return ret

def printPyFile(f,out, options):
	for name in f.order:
		v = f.features[name]
		if v["Category"] != "Deprecated":
			feat = v["FeatureType"]
			if feat in ["val"]:
				out.write(name + "=" + v["Value"] + "\n")
			if feat in ["evt"]:
				out.write("SCN_" + name.upper() + "=" + v["Value"] + "\n")

def printHFile(f,out, options):
	for name in f.order:
		v = f.features[name]
		if v["Category"] != "Deprecated":
			feat = v["FeatureType"]
			if feat in ["fun", "get", "set"]:
				if checkTypes(name, v):
					constDeclarator = " const" if feat == "get" else ""
					returnType = cppAlias(v["ReturnType"])
					stringResult = v["Param2Type"] == "stringresult"
					if stringResult:
						returnType = "QByteArray"
					out.write("\t" + returnType + " " + normalisedName(name, options, feat) + "(")
					out.write(arguments(v, stringResult, options))
					out.write(")" + constDeclarator + ";\n")

def methodNames(f, options):
	for name in f.order:
		v = f.features[name]
		if v["Category"] != "Deprecated":
			feat = v["FeatureType"]
			if feat in ["fun", "get", "set"]:
				if checkTypes(name, v):
					yield normalisedName(name, options)

def printCPPFile(f,out, options):
	for name in f.order:
		v = f.features[name]
		if v["Category"] != "Deprecated":
			feat = v["FeatureType"]
			if feat in ["fun", "get", "set"]:
				if checkTypes(name, v):
					constDeclarator = " const" if feat == "get" else ""
					featureDefineName = "SCI_" + name.upper()
					returnType = cppAlias(v["ReturnType"])
					stringResult = v["Param2Type"] == "stringresult"
					if stringResult:
						returnType = "QByteArray"
					returnStatement = ""
					if returnType != "void":
						returnStatement = "return "
					out.write(returnType + " ScintillaEdit::" + normalisedName(name, options, feat) + "(")
					out.write(arguments(v, stringResult, options))
					out.write(")" + constDeclarator + " {\n")
					if stringResult:
						out.write("    " + returnStatement + "TextReturner(" + featureDefineName + ", ")
						if "*" in cppAlias(v["Param1Type"]):
							out.write("(uptr_t)")
						if v["Param1Name"]:
							out.write(normalisedName(v["Param1Name"], options))
						else:
							out.write("0")
						out.write(");\n")
					else:
						out.write("    " + returnStatement + "send(" + featureDefineName + ", ")
						if "*" in cppAlias(v["Param1Type"]):
							out.write("(uptr_t)")
						if v["Param1Name"]:
							out.write(normalisedName(v["Param1Name"], options))
						else:
							out.write("0")
						out.write(", ")
						if "*" in cppAlias(v["Param2Type"]):
							out.write("(sptr_t)")
						if v["Param2Name"]:
							out.write(normalisedName(v["Param2Name"], options))
						else:
							out.write("0")
						out.write(");\n")
					out.write("}\n")
					out.write("\n")

def CopyWithInsertion(input, output, genfn, definition, options):
	copying = 1
	for line in input.readlines():
		if copying:
			output.write(line)
		if "/* ++Autogenerated" in line or "# ++Autogenerated" in line or "<!-- ++Autogenerated" in line:
			copying = 0
			genfn(definition, output, options)
		# ~~ form needed as XML comments can not contain --
		if "/* --Autogenerated" in line or "# --Autogenerated" in line or "<!-- ~~Autogenerated" in line:
			copying = 1
			output.write(line)

def contents(filename):
	with open(filename, "U") as f:
		t = f.read()
		return t

def Generate(templateFile, destinationFile, genfn, definition, options):
	inText = contents(templateFile)
	try:
		currentText = contents(destinationFile)
	except IOError:
		currentText = ""
	tempname = "WidgetGen.tmp"
	with open(tempname, "w") as out:
		with open(templateFile, "U") as hfile:
			CopyWithInsertion(hfile, out, genfn, definition, options)
	outText = contents(tempname)
	if currentText == outText:
		os.unlink(tempname)
	else:
		try:
			os.unlink(destinationFile)
		except OSError:
			# Will see failure if file does not yet exist
			pass
		os.rename(tempname, destinationFile)

def gtkNames():
	# The full path on my machine: should be altered for anyone else
	p = "C:/Users/Neil/Downloads/wingide-source-4.0.1-1/wingide-source-4.0.1-1/external/gtkscintilla2/gtkscintilla.c"
	with open(p) as f:
		for l in f.readlines():
			if "gtk_scintilla_" in l:
				name = l.split()[1][14:]
				if '(' in name:
					name = name.split('(')[0]
					yield name

def usage():
	print("WidgetGen.py [-c|--clean][-h|--help][-u|--underscore-names]")
	print("")
	print("Generate full APIs for ScintillaEdit class and ScintillaConstants.py.")
	print("")
	print("options:")
	print("")
	print("-c --clean remove all generated code from files")
	print("-h --help  display this text")
	print("-u --underscore-names  use method_names consistent with GTK+ standards")

def readInterface(cleanGenerated):
	f = Face.Face()
	if not cleanGenerated:
		f.ReadFromFile("../../include/Scintilla.iface")
	return f

def main(argv):
	# Using local path for gtkscintilla2 so don't default to checking
	checkGTK = False
	cleanGenerated = False
	qtStyleInterface = True
	# The --gtk-check option checks for full coverage of the gtkscintilla2 API but
	# depends on a particular directory so is not mentioned in --help.
	opts, args = getopt.getopt(argv, "hcgu", ["help", "clean", "gtk-check", "underscore-names"])
	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
			sys.exit()
		elif opt in ("-c", "--clean"):
			cleanGenerated = True
		elif opt in ("-g", "--gtk-check"):
			checkGTK = True
		elif opt in ("-u", "--underscore-names"):
			qtStyleInterface = False

	options = {"qtStyle": qtStyleInterface}
	f = readInterface(cleanGenerated)
	try:
		Generate("ScintillaEdit.cpp.template", "ScintillaEdit.cpp", printCPPFile, f, options)
		Generate("ScintillaEdit.h.template", "ScintillaEdit.h", printHFile, f, options)
		Generate("../ScintillaEditPy/ScintillaConstants.py.template",
			"../ScintillaEditPy/ScintillaConstants.py",
			printPyFile, f, options)
		if checkGTK:
			names = set(methodNames(f))
			#~ print("\n".join(names))
			namesGtk = set(gtkNames())
			for name in namesGtk:
				if name not in names:
					print(name, "not found in Qt version")
			for name in names:
				if name not in namesGtk:
					print(name, "not found in GTK+ version")
	except:
		raise

	if cleanGenerated:
		for file in ["ScintillaEdit.cpp", "ScintillaEdit.h", "../ScintillaEditPy/ScintillaConstants.py"]:
			try:
				os.remove(file)
			except OSError:
				pass
		
if __name__ == "__main__":
	main(sys.argv[1:])
