/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	@Project: TaaR_Coding_Style
 *	@File	: main.cpp
 *
 *	Created	: 10/17/2024 5:52:14 PM
 *	Author	: Nghia-Taarabt
 *	Link repository: https://github.com/nghia12a1-t-ara/Embedded_MyTools/tree/master/TaaR_Coding_Style
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
#ifndef __MAIN_H__
#define __MAIN_H__

// for console build only
#include "TRLanguage.h"
#include "TaaRRule.h"
#include <sstream>
#include <ctime>
#ifdef _MSC_VER
	#include <sys/utime.h>
	#include <sys/stat.h>
#else
	#include <utime.h>
	#include <sys/stat.h>
#endif		// end compiler checks
#define _(a)	localizer.settext(a)

//----------------------------------------------------------------------------
// TRStreamIterator class
// typename will be istringstream for GUI and istream otherwise
// TRSourceIterator is an abstract class defined in TaaRRule.h
//----------------------------------------------------------------------------
namespace TaaRRule {
template<typename T>
class TRStreamIterator : public TRSourceIterator
{
	public:
		bool checkForEmptyLine;

		// function declarations
		TRStreamIterator(T* in);
		virtual ~TRStreamIterator();
		bool getLineEndChange(int lineEndFormat) const;
		string nextLine(bool emptyLineWasDeleted);
		string peekNextLine();
		void peekReset();
		void saveLastInputLine();

	private:
		TRStreamIterator(const TRStreamIterator &copy);       // copy constructor not to be imlpemented
		TRStreamIterator &operator=(TRStreamIterator &);      // assignment operator not to be implemented
		T* inStream;           // pointer to the input stream
		string buffer;         // current input line
		string prevBuffer;     // previous input line
		int eolWindows;        // number of Windows line endings, CRLF
		int eolLinux;          // number of Linux line endings, LF
		int eolMacOld;         // number of old Mac line endings. CR
		char outputEOL[4];     // next output end of line char
		streamoff peekStart;   // starting position for peekNextLine
		bool prevLineDeleted;  // the previous input line was deleted

	public:	// inline functions
		bool compareToInputBuffer(const string &nextLine_) const
		{ return (nextLine_ == prevBuffer); }
		const char* getOutputEOL() const { return outputEOL; }
		bool hasMoreLines() const { return !inStream->eof(); }
};

//----------------------------------------------------------------------------
// ASOptions class for options processing
// used by both console and library builds
//----------------------------------------------------------------------------

class ASOptions
{
	public:
		ASOptions(TRFormatter &formatterArg) : formatter(formatterArg) {}
		string getOptionErrors();
		void importOptions(istream &in, vector<string> &optionsVector);
		bool parseOptions(vector<string> &optionsVector, const string &errorInfo);

	private:
		// variables
		TRFormatter &formatter;			// reference to the TRFormatter object
		stringstream optionErrors;		// option error messages

		// functions
		ASOptions &operator=(ASOptions &);         // not to be implemented
		string getParam(const string &arg, const char* op);
		string getParam(const string &arg, const char* op1, const char* op2);
		bool isOption(const string &arg, const char* op);
		bool isOption(const string &arg, const char* op1, const char* op2);
		void isOptionError(const string &arg, const string &errorInfo);
		bool isParamOption(const string &arg, const char* option);
		bool isParamOption(const string &arg, const char* option1, const char* option2);
		void parseOption(const string &arg, const string &errorInfo);
};

//----------------------------------------------------------------------------
// ASConsole class for console build
//----------------------------------------------------------------------------

class ASConsole
{
	private:	// variables
		TRFormatter &formatter;				// reference to the TRFormatter object
		TRLanguage localizer;				// TRLanguage object
		// command line options
		bool isRecursive;                   // recursive option
		string origSuffix;                  // suffix= option
		bool noBackup;                      // suffix=none option
		bool preserveDate;                  // preserve-date option
		bool isVerbose;                     // verbose option
		bool isQuiet;                       // quiet option
		bool isFormattedOnly;               // formatted lines only option
		bool ignoreExcludeErrors;           // don't abort on unmatched excludes
		bool ignoreExcludeErrorsDisplay;    // don't display unmatched excludes
		bool optionsFileRequired;           // options= option
		bool useAscii;                      // ascii option
		// other variables
		bool hasWildcard;                   // file name includes a wildcard
		size_t mainDirectoryLength;         // directory length to be excluded in displays
		bool filesAreIdentical;				// input and output files are identical
		bool lineEndsMixed;					// output has mixed line ends
		int  linesOut;                      // number of output lines
		int  filesFormatted;                // number of files formatted
		int  filesUnchanged;                // number of files unchanged
		char outputEOL[4];					// current line end
		char prevEOL[4];					// previous line end

		string optionsFileName;             // file path and name of the options file to use
		string targetDirectory;             // path to the directory being processed
		string targetFilename;              // file name being processed

		vector<string> excludeVector;       // exclude from wildcard hits
		vector<bool>   excludeHitsVector;   // exclude flags for error reporting
		vector<string> fileNameVector;      // file paths and names from the command line
		vector<string> optionsVector;       // options from the command line
		vector<string> fileOptionsVector;   // options from the options file
		vector<string> fileName;            // files to be processed including path

	public:
		ASConsole(TRFormatter &formatterArg) : formatter(formatterArg) {
			// command line options
			isRecursive = false;
			origSuffix = ".orig";
			noBackup = false;
			preserveDate = false;
			isVerbose = false;
			isQuiet = false;
			isFormattedOnly = false;
			ignoreExcludeErrors = false;
			ignoreExcludeErrorsDisplay = false;
			optionsFileRequired = false;
			useAscii = false;
			// other variables
			hasWildcard = false;
			filesAreIdentical = true;
			lineEndsMixed = false;
			outputEOL[0] = '\0';
			prevEOL[0] = '\0';
			mainDirectoryLength = 0;
			filesFormatted = 0;
			filesUnchanged = 0;
			linesOut = 0;
		}

		// public functions
		void convertLineEnds(ostringstream &out, int lineEnd);
		FileEncoding detectEncoding(const char* data, size_t dataSize) const;
		void error() const;
		void error(const char* why, const char* what) const;
		void formatCinToCout();
		vector<string> getArgvOptions(int argc, char** argv) const;
		bool fileNameVectorIsEmpty();
		int  getFilesFormatted();
		bool getIgnoreExcludeErrors();
		bool getIgnoreExcludeErrorsDisplay();
		bool getIsFormattedOnly();
		bool getIsQuiet();
		bool getIsRecursive();
		bool getIsVerbose();
		bool getLineEndsMixed();
		bool getNoBackup();
		string getLanguageID() const;
		string getNumberFormat(int num, size_t = 0) const ;
		string getNumberFormat(int num, const char* groupingArg, const char* separator) const;
		string getOptionsFileName();
		string getOrigSuffix();
		bool getPreserveDate();
		void processFiles();
		void processOptions(vector<string> &argvOptions);
		void setIgnoreExcludeErrors(bool state);
		void setIgnoreExcludeErrorsAndDisplay(bool state);
		void setIsFormattedOnly(bool state);
		void setIsQuiet(bool state);
		void setIsRecursive(bool state);
		void setIsVerbose(bool state);
		void setNoBackup(bool state);
		void setOptionsFileName(string name);
		void setOrigSuffix(string suffix);
		void setPreserveDate(bool state);
		void standardizePath(string &path, bool removeBeginningSeparator = false) const;
		bool stringEndsWith(const string &str, const string &suffix) const;
		void updateExcludeVector(string suffixParam);
		size_t Utf8LengthFromUtf16(const char* data, size_t len, FileEncoding encoding) const;
		size_t Utf8ToUtf16(char* utf8In, size_t inLen, FileEncoding encoding, char* utf16Out) const;
		size_t Utf16LengthFromUtf8(const char* data, size_t len) const;
		size_t Utf16ToUtf8(char* utf16In, size_t inLen, FileEncoding encoding, bool firstBlock, char* utf8Out) const;

		// for unit testing
		vector<string> getExcludeVector();
		vector<bool>   getExcludeHitsVector();
		vector<string> getFileNameVector();
		vector<string> getOptionsVector();
		vector<string> getFileOptionsVector();
		vector<string> getFileName();

	private:	// functions
		ASConsole &operator=(ASConsole &);         // not to be implemented
		void correctMixedLineEnds(ostringstream &out);
		void formatFile(const string &fileName_);
		string getCurrentDirectory(const string &fileName_) const;
		void getFileNames(const string &directory, const string &wildcard);
		void getFilePaths(string &filePath);
		string getParam(const string &arg, const char* op);
		void initializeOutputEOL(LineEndFormat lineEndFormat);
		bool isOption(const string &arg, const char* op);
		bool isOption(const string &arg, const char* op1, const char* op2);
		bool isParamOption(const string &arg, const char* option);
		bool isPathExclued(const string &subPath);
		void printHelp() const;
		void printMsg(const char* msg, const string &data) const;
		void printSeparatingLine() const;
		void printVerboseHeader() const;
		void printVerboseStats(clock_t startTime) const;
		FileEncoding readFile(const string &fileName, stringstream &in) const;
		void removeFile(const char* fileName_, const char* errMsg) const;
		void renameFile(const char* oldFileName, const char* newFileName, const char* errMsg) const;
		void setOutputEOL(LineEndFormat lineEndFormat, const char* currentEOL);
		void sleep(int seconds) const;
		int  swap8to16bit(int value) const;
		int  swap16bit(int value) const;
		int  waitForRemove(const char* oldFileName) const;
		int  wildcmp(const char* wild, const char* data) const;
		void writeFile(const string &fileName_, FileEncoding encoding, ostringstream &out) const;
		void displayLastError();
};

//----------------------------------------------------------------------------

}   // end of namespace TaaRRule

#endif /* __MAIN_H__ */
