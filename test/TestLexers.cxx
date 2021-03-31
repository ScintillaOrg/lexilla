// Lexilla lexer library
/** @file TestLexers.cxx
 ** Test lexers through Lexilla.
 **/
 // Copyright 2019 by Neil Hodgson <neilh@scintilla.org>
 // The License.txt file describes the conditions under which this software may be distributed.

#include <cassert>

#include <string>
#include <string_view>
#include <vector>
#include <map>

#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "ILexer.h"

#include "Lexilla.h"
#include "LexillaAccess.h"

#include "TestDocument.h"

namespace {

std::string ReadFile(std::filesystem::path path) {
	std::ifstream ifs(path, std::ios::binary);
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	return content;
}

std::string MarkedDocument(const Scintilla::IDocument *pdoc) {
	std::ostringstream os(std::ios::binary);
	char prevStyle = -1;
	for (Sci_Position pos = 0; pos < pdoc->Length(); pos++) {
		const char styleNow = pdoc->StyleAt(pos);
		if (styleNow != prevStyle) {
			os << "{" << static_cast<unsigned int>(styleNow) << "}";
			prevStyle = styleNow;
		}
		char ch = '\0';
		pdoc->GetCharRange(&ch, pos, 1);
		os << ch;
	}
	return os.str();
}

std::map<std::string, std::string> PropertiesFromFile(std::filesystem::path path) {
	std::map<std::string, std::string> m;
	std::ifstream ifs(path);
	std::string line;
	std::string logicalLine;
	while (std::getline(ifs, line)) {
		logicalLine += line;
		if (logicalLine.ends_with("\\")) {
			logicalLine.pop_back();
		} else {
			const size_t positionEquals = logicalLine.find("=");
			if (positionEquals != std::string::npos) {
				const std::string key = logicalLine.substr(0, positionEquals);
				const std::string value = logicalLine.substr(positionEquals+1);
				m[key] = value;
			}
			logicalLine.clear();
		}
	}
	return m;
}

int Substitute(std::string &s, const std::string &sFind, const std::string &sReplace) {
	int c = 0;
	const size_t lenFind = sFind.size();
	const size_t lenReplace = sReplace.size();
	size_t posFound = s.find(sFind);
	while (posFound != std::string::npos) {
		s.replace(posFound, lenFind, sReplace);
		posFound = s.find(sFind, posFound + lenReplace);
		c++;
	}
	return c;
}

const std::string BOM = "\xEF\xBB\xBF";

void TestCRLF(std::filesystem::path path, const std::string s, Scintilla::ILexer5 *plex) {
	// Convert all line ends to \r\n to check if styles change between \r and \n which makes
	// it difficult to test on different platforms when files may have line ends changed.
	std::string text = s;
	Substitute(text, "\r\n", "\n");
	Substitute(text, "\n", "\r\n");
	TestDocument doc;
	doc.Set(text);
	Scintilla::IDocument *pdoc = &doc;
	plex->Lex(0, pdoc->Length(), 0, pdoc);
	int prevStyle = -1;
	Sci_Position line = 1;
	for (Sci_Position pos = 0; pos < pdoc->Length(); pos++) {
		const int styleNow = pdoc->StyleAt(pos);
		char ch = '\0';
		pdoc->GetCharRange(&ch, pos, 1);
		if (ch == '\n') {
			if (styleNow != prevStyle) {
				std::cout << path.string() << ":" << line << ":" <<
					" different styles between \\r and \\n at " <<
					pos << ": " << prevStyle << ", " << styleNow << "\n";
			}
			line++;
		}
		prevStyle = styleNow;
	}
	plex->Release();
}

bool TestFile(std::filesystem::path path,
	std::map<std::string, std::string> properties) {
	// Find and create correct lexer
	std::string language;
	Scintilla::ILexer5 *plex = nullptr;
	for (auto const &[key, val] : properties) {
		if (key.starts_with("lexer.*")) {
			language = val;
			plex = Lexilla::MakeLexer(language);
			break;
		}
	}
	if (!plex) {
		std::cout << "\n" << path.string() << ":1: has no lexer\n\n";
		return false;
	}

	// Set parameters of lexer
	const std::string keywords = "keywords";
	for (auto const &[key, val] : properties) {
		if (key.starts_with("#")) {
			// Ignore comments
		} else if (key.starts_with("lexer.*")) {
			// Ignore
		} else if (key.starts_with("keywords")) {
			// Get character after keywords
			std::string afterKeywords = key.substr(keywords.length(), 1);
			char characterAfterKeywords = afterKeywords.empty() ? '1' : afterKeywords[0];
			if (characterAfterKeywords < '1' || characterAfterKeywords > '9')
				characterAfterKeywords = '1';
			const int wordSet = characterAfterKeywords - '1';
			plex->WordListSet(wordSet, val.c_str());
		} else {
			plex->PropertySet(key.c_str(), val.c_str());
		}
	}
	std::string text = ReadFile(path);
	if (text.starts_with(BOM)) {
		text.erase(0, BOM.length());
	}

	TestDocument doc;
	doc.Set(text);
	Scintilla::IDocument *pdoc = &doc;
	plex->Lex(0, pdoc->Length(), 0, pdoc);
	const std::string styledTextNew = MarkedDocument(pdoc);
	std::filesystem::path pathStyled = path;
	pathStyled += ".styled";
	const std::string styledText = ReadFile(pathStyled);
	if (styledTextNew != styledText) {
		std::cout << "\n" << path.string() << ":1: is different\n\n";
		std::filesystem::path pathNew = path;
		pathNew += ".new";
		std::ofstream ofs(pathNew, std::ios::binary);
		ofs << styledTextNew;
	}
	plex->Release();

	TestCRLF(path, text, Lexilla::MakeLexer(language));

	return true;
}

bool TestDirectory(std::filesystem::path directory, std::filesystem::path basePath) {
	const std::map<std::string, std::string> properties = PropertiesFromFile(directory / "SciTE.properties");
	bool success = true;
	for (auto &p : std::filesystem::directory_iterator(directory)) {
		if (!p.is_directory()) {
			const std::string extension = p.path().extension().string();
			if (extension != ".properties" && extension != ".styled" && extension != ".new") {
				const std::filesystem::path relativePath = p.path().lexically_relative(basePath);
				std::cout << "Lexing " << relativePath.string() << '\n';
				if (!TestFile(p, properties)) {
					success = false;
				}
			}
		}
	}
	return success;
}

bool AccessLexilla(std::filesystem::path basePath) {
	if (!std::filesystem::exists(basePath)) {
		std::cout << "No examples at " << basePath.string() << "\n";
		return false;
	}

	bool success = true;
	for (auto &p : std::filesystem::recursive_directory_iterator(basePath)) {
		if (p.is_directory()) {
			//std::cout << p.path().string() << '\n';
			if (!TestDirectory(p, basePath)) {
				success = false;
			}
		}
	}
	return success;
}

std::filesystem::path FindLexillaDirectory(std::filesystem::path startDirectory) {
	// Search up from startDirectory for a directory named "lexilla" or containing a "bin" subdirectory 
	std::filesystem::path directory = startDirectory;
	while (!directory.empty()) {
		//std::cout << "Searching " << directory.string() << "\n";
		const std::filesystem::path parent = directory.parent_path();
		const std::filesystem::path localLexilla = directory / "lexilla";
		const std::filesystem::directory_entry entry(localLexilla);
		if (entry.is_directory()) {
			std::cout << "Found Lexilla at " << entry.path().string() << "\n";
			return localLexilla;
		}
		const std::filesystem::path localBin = directory / "bin";
		const std::filesystem::directory_entry entryBin(localBin);
		if (entryBin.is_directory()) {
			std::cout << "Found Lexilla at " << directory.string() << "\n";
			return directory;
		}
		if (parent == directory) {
			std::cout << "Reached root at " << directory.string() << "\n";
			return std::filesystem::path();
		}
		directory = parent;
	}
	return std::filesystem::path();
}

}



int main() {
	bool success = false;
	// TODO: Allow specifying the base directory through a command line argument
	const std::filesystem::path baseDirectory = FindLexillaDirectory(std::filesystem::current_path());
	if (!baseDirectory.empty()) {
		const std::filesystem::path examplesDirectory = baseDirectory / "test" / "examples";
#ifdef LEXILLA_STATIC
		success = AccessLexilla(examplesDirectory);
#else
		const std::filesystem::path sharedLibrary = baseDirectory / "bin" / LEXILLA_LIB;
		if (Lexilla::Load(sharedLibrary.string())) {
			success = AccessLexilla(examplesDirectory);
		}
#endif
	}
	return success ? 0 : 1;
}
