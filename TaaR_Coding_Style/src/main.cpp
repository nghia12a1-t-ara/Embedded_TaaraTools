/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	@Project: TaaR_Coding_Style
 *	@File	: main.cpp
 *
 *	Created	: 10/17/2024 5:52:14 PM
 *	Author	: Nghia-Taarabt
 *	Link repository: https://github.com/nghia12a1-t-ara/Embedded_MyTools/tree/master/TaaR_Coding_Style
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
#include "main.h"
#include <algorithm>
#include <cstdlib>
#include <errno.h>
#include <fstream>
#include <sstream>
#include <windows.h>

namespace TaaRRule {

char g_fileSeparator = '\\';
bool g_isCaseSensitive = false;

// console build variables
ostream* _err = &cerr;           // direct error messages to cerr
ASConsole* g_console = NULL;     // class to encapsulate console variables

//-----------------------------------------------------------------------------
// TRStreamIterator class
// typename will be istringstream for GUI and istream otherwise
//-----------------------------------------------------------------------------

template<typename T>
TRStreamIterator<T>::TRStreamIterator(T* in)
{
	inStream = in;
	buffer.reserve(200);
	eolWindows = 0;
	eolLinux = 0;
	eolMacOld = 0;
	outputEOL[0] = '\0';
	peekStart = 0;
	prevLineDeleted = false;
	checkForEmptyLine = false;
}

template<typename T>
TRStreamIterator<T>::~TRStreamIterator()
{
}

/**
 * read the input stream, delete any end of line characters,
 *     and build a string that contains the input line.
 *
 * @return        string containing the next input line minus any end of line characters
 */
template<typename T>
string TRStreamIterator<T>::nextLine(bool emptyLineWasDeleted)
{
	// verify that the current position is correct
	assert (peekStart == 0);

	// a deleted line may be replaced if break-blocks is requested
	// this sets up the compare to check for a replaced empty line
	if (prevLineDeleted)
	{
		prevLineDeleted = false;
		checkForEmptyLine = true;
	}
	if (!emptyLineWasDeleted)
		prevBuffer = buffer;
	else
		prevLineDeleted = true;

	// read the next record
	buffer.clear();
	char ch;
	inStream->get(ch);

	while (!inStream->eof() && ch != '\n' && ch != '\r')
	{
		buffer.append(1, ch);
		inStream->get(ch);
	}

	if (inStream->eof())
	{
		return buffer;
	}

	int peekCh = inStream->peek();

	// find input end-of-line characters
	if (!inStream->eof())
	{
		if (ch == '\r')         // CR+LF is windows otherwise Mac OS 9
		{
			if (peekCh == '\n')
			{
				inStream->get();
				eolWindows++;
			}
			else
				eolMacOld++;
		}
		else                    // LF is Linux, allow for improbable LF/CR
		{
			if (peekCh == '\r')
			{
				inStream->get();
				eolWindows++;
			}
			else
				eolLinux++;
		}
	}
	else
	{
		inStream->clear();
	}

	// set output end of line characters
	if (eolWindows >= eolLinux)
	{
		if (eolWindows >= eolMacOld)
			strcpy(outputEOL, "\r\n");  // Windows (CR+LF)
		else
			strcpy(outputEOL, "\r");    // MacOld (CR)
	}
	else if (eolLinux >= eolMacOld)
		strcpy(outputEOL, "\n");		// Linux (LF)
	else
		strcpy(outputEOL, "\r");		// MacOld (CR)

	return buffer;
}

// save the current position and get the next line
// this can be called for multiple reads
// when finished peeking you MUST call peekReset()
// call this function from TRFormatter ONLY
template<typename T>
string TRStreamIterator<T>::peekNextLine()
{
	assert (hasMoreLines());
	string nextLine_;
	char ch;

	if (peekStart == 0)
		peekStart = inStream->tellg();

	// read the next record
	inStream->get(ch);
	while (!inStream->eof() && ch != '\n' && ch != '\r')
	{
		nextLine_.append(1, ch);
		inStream->get(ch);
	}

	if (inStream->eof())
	{
		return nextLine_;
	}

	int peekCh = inStream->peek();

	// remove end-of-line characters
	if (!inStream->eof())
	{
		if ((peekCh == '\n' || peekCh == '\r') && peekCh != ch)
			inStream->get();
	}

	return nextLine_;
}

// reset current position and EOF for peekNextLine()
template<typename T>
void TRStreamIterator<T>::peekReset()
{
	assert(peekStart != 0);
	inStream->clear();
	inStream->seekg(peekStart);
	peekStart = 0;
}

// save the last input line after input has reached EOF
template<typename T>
void TRStreamIterator<T>::saveLastInputLine()
{
	assert(inStream->eof());
	prevBuffer = buffer;
}

// check for a change in line ends
template<typename T>
bool TRStreamIterator<T>::getLineEndChange(int lineEndFormat) const
{
	assert(lineEndFormat == LINEEND_DEFAULT
	       || lineEndFormat == LINEEND_WINDOWS
	       || lineEndFormat == LINEEND_LINUX
	       || lineEndFormat == LINEEND_MACOLD);

	bool lineEndChange = false;
	if (lineEndFormat == LINEEND_WINDOWS)
		lineEndChange = (eolLinux + eolMacOld != 0);
	else if (lineEndFormat == LINEEND_LINUX)
		lineEndChange = (eolWindows + eolMacOld != 0);
	else if (lineEndFormat == LINEEND_MACOLD)
		lineEndChange = (eolWindows + eolLinux != 0);
	else
	{
		if (eolWindows > 0)
			lineEndChange = (eolLinux + eolMacOld != 0);
		else if (eolLinux > 0)
			lineEndChange = (eolWindows + eolMacOld != 0);
		else if (eolMacOld > 0)
			lineEndChange = (eolWindows + eolLinux != 0);
	}
	return lineEndChange;
}

//-----------------------------------------------------------------------------
// ASConsole class
// main function will be included only in the console build
//-----------------------------------------------------------------------------

// rewrite a stringstream converting the line ends
void ASConsole::convertLineEnds(ostringstream &out, int lineEnd)
{
	assert(lineEnd == LINEEND_WINDOWS || lineEnd == LINEEND_LINUX || lineEnd == LINEEND_MACOLD);
	const string &inStr = out.str();	// avoids strange looking syntax
	string outStr;						// the converted ouput
	int inLength = inStr.length();
	for (int pos = 0; pos < inLength; pos++)
	{
		if (inStr[pos] == '\r')
		{
			if (inStr[pos + 1] == '\n')
			{
				// CRLF
				if (lineEnd == LINEEND_CR)
				{
					outStr += inStr[pos];		// Delete the LF
					pos++;
					continue;
				}
				else if (lineEnd == LINEEND_LF)
				{
					outStr += inStr[pos + 1];		// Delete the CR
					pos++;
					continue;
				}
				else
				{
					outStr += inStr[pos];		// Do not change
					outStr += inStr[pos + 1];
					pos++;
					continue;
				}
			}
			else
			{
				// CR
				if (lineEnd == LINEEND_CRLF)
				{
					outStr += inStr[pos];		// Insert the CR
					outStr += '\n';				// Insert the LF
					continue;
				}
				else if (lineEnd == LINEEND_LF)
				{
					outStr += '\n';				// Insert the LF
					continue;
				}
				else
				{
					outStr += inStr[pos];		// Do not change
					continue;
				}
			}
		}
		else if (inStr[pos] == '\n')
		{
			// LF
			if (lineEnd == LINEEND_CRLF)
			{
				outStr += '\r';				// Insert the CR
				outStr += inStr[pos];		// Insert the LF
				continue;
			}
			else if (lineEnd == LINEEND_CR)
			{
				outStr += '\r';				// Insert the CR
				continue;
			}
			else
			{
				outStr += inStr[pos];		// Do not change
				continue;
			}
		}
		else
		{
			outStr += inStr[pos];		// Write the current char
		}
	}
	// replace the stream
	out.str(outStr);
}

void ASConsole::correctMixedLineEnds(ostringstream &out)
{
	LineEndFormat lineEndFormat = LINEEND_DEFAULT;
	if (strcmp(outputEOL, "\r\n") == 0)
		lineEndFormat = LINEEND_WINDOWS;
	if (strcmp(outputEOL, "\n") == 0)
		lineEndFormat = LINEEND_LINUX;
	if (strcmp(outputEOL, "\r") == 0)
		lineEndFormat = LINEEND_MACOLD;
	convertLineEnds(out, lineEndFormat);
}

// check files for 16 or 32 bit encoding
// the file must have a Byte Order Mark (BOM)
// NOTE: some string functions don't work with NULLs (e.g. length())
FileEncoding ASConsole::detectEncoding(const char* data, size_t dataSize) const
{
	FileEncoding encoding = ENCODING_8BIT;

	if (dataSize >= 4 && memcmp(data, "\x00\x00\xFE\xFF", 4) == 0)
		encoding = UTF_32BE;
	else if (dataSize >= 4 && memcmp(data, "\xFF\xFE\x00\x00", 4) == 0)
		encoding = UTF_32LE;
	else if (dataSize >= 2 && memcmp(data, "\xFE\xFF", 2) == 0)
		encoding = UTF_16BE;
	else if (dataSize >= 2 && memcmp(data, "\xFF\xFE", 2) == 0)
		encoding = UTF_16LE;

	return encoding;
}

// error exit without a message
void ASConsole::error() const
{
	(*_err) << _("\nArtistic Style has terminated") << endl;
	exit(EXIT_FAILURE);
}

// error exit with a message
void ASConsole::error(const char* why, const char* what) const
{
	(*_err) << why << ' ' << what << endl;
	error();
}

/**
 * If no files have been given, use cin for input and cout for output.
 *
 * This is used to format text for text editors like TextWrangler (Mac).
 * Do NOT display any console messages when this function is used.
 */
void ASConsole::formatCinToCout()
{
	// Using cin.tellg() causes problems with both Windows and Linux.
	// The Windows problem occurs when the input is not Windows line-ends.
	// The tellg() will be out of sequence with the get() statements.
	// The Linux cin.tellg() will return -1 (invalid).
	// Copying the input sequentially to a stringstream before
	// formatting solves the problem for both.
	istream* inStream = &cin;
	stringstream outStream;
	char ch;
	while (!inStream->eof())
	{
		inStream->get(ch);
		outStream.put(ch);
	}
	TRStreamIterator<stringstream> streamIterator(&outStream);
	// Windows pipe or redirection always outputs Windows line-ends.
	// Linux pipe or redirection will output any line end.
	LineEndFormat lineEndFormat = formatter.getLineEndFormat();
	initializeOutputEOL(lineEndFormat);
	formatter.init(&streamIterator);

	while (formatter.hasMoreLines())
	{
		cout << formatter.nextLine();
		if (formatter.hasMoreLines())
		{
			setOutputEOL(lineEndFormat, streamIterator.getOutputEOL());
			cout << outputEOL;
		}
		else
		{
			// this can happen if the file if missing a closing bracket and break-blocks is requested
			if (formatter.getIsLineReady())
			{
				setOutputEOL(lineEndFormat, streamIterator.getOutputEOL());
				cout << outputEOL;
				cout << formatter.nextLine();
			}
		}
	}
	cout.flush();
}

/**
 * Open input file, format it, and close the output.
 *
 * @param fileName_     The path and name of the file to be processed.
 */
void ASConsole::formatFile(const string &fileName_)
{
	stringstream in;
	ostringstream out;
	FileEncoding encoding = readFile(fileName_, in);

	// Unless a specific language mode has been set, set the language mode
	// according to the file's suffix.
	if (!formatter.getModeManuallySet())
	{
		formatter.setCStyle();
	}

	// set line end format
	string nextLine;				// next output line
	filesAreIdentical = true;		// input and output files are identical
	LineEndFormat lineEndFormat = formatter.getLineEndFormat();
	initializeOutputEOL(lineEndFormat);
	// do this AFTER setting the file mode
	TRStreamIterator<stringstream> streamIterator(&in);
	formatter.init(&streamIterator);

	// format the file
	while (formatter.hasMoreLines())
	{
		nextLine = formatter.nextLine();
		out << nextLine;
		linesOut++;
		if (formatter.hasMoreLines())
		{
			setOutputEOL(lineEndFormat, streamIterator.getOutputEOL());
			out << outputEOL;
		}
		else
		{
			streamIterator.saveLastInputLine();     // to compare the last input line
			// this can happen if the file if missing a closing bracket and break-blocks is requested
			if (formatter.getIsLineReady())
			{
				setOutputEOL(lineEndFormat, streamIterator.getOutputEOL());
				out << outputEOL;
				nextLine = formatter.nextLine();
				out << nextLine;
				linesOut++;
				streamIterator.saveLastInputLine();
			}
		}

		if (filesAreIdentical)
		{
			if (streamIterator.checkForEmptyLine)
			{
				if (nextLine.find_first_not_of(" \t") != string::npos)
					filesAreIdentical = false;
			}
			else if (!streamIterator.compareToInputBuffer(nextLine))
				filesAreIdentical = false;
			streamIterator.checkForEmptyLine = false;
		}
	}
	// correct for mixed line ends
	if (lineEndsMixed)
	{
		correctMixedLineEnds(out);
		filesAreIdentical = false;
	}

	// remove targetDirectory from filename if required by print
	string displayName;
	if (hasWildcard)
		displayName = fileName_.substr(targetDirectory.length() + 1);
	else
		displayName = fileName_;

	// if file has changed, write the new file
	if (!filesAreIdentical || streamIterator.getLineEndChange(lineEndFormat))
	{
		writeFile(fileName_, encoding, out);
		printMsg(_("Formatted  %s\n"), displayName);
		filesFormatted++;
	}
	else
	{
		if (!isFormattedOnly)
			printMsg(_("Unchanged  %s\n"), displayName);
		filesUnchanged++;
	}

	assert(formatter.getChecksumDiff() == 0);
}

// build a vector of argv options
// the program path argv[0] is excluded
vector<string> ASConsole::getArgvOptions(int argc, char** argv) const
{
	vector<string> argvOptions;
	for (int i = 1; i < argc; i++)
	{
		argvOptions.push_back(string(argv[i]));
	}
	return argvOptions;
}

// for unit testing
vector<bool> ASConsole::getExcludeHitsVector()
{ return excludeHitsVector; }

// for unit testing
vector<string> ASConsole::getExcludeVector()
{ return excludeVector; }

// for unit testing
vector<string> ASConsole::getFileName()
{ return fileName; }

// for unit testing
vector<string> ASConsole::getFileNameVector()
{ return fileNameVector; }

// for unit testing
vector<string> ASConsole::getFileOptionsVector()
{ return fileOptionsVector; }

// for unit testing
int ASConsole::getFilesFormatted()
{ return filesFormatted; }

// for unit testing
bool ASConsole::getIgnoreExcludeErrors()
{ return ignoreExcludeErrors; }

// for unit testing
bool ASConsole::getIgnoreExcludeErrorsDisplay()
{ return ignoreExcludeErrorsDisplay; }

// for unit testing
bool ASConsole::getIsFormattedOnly()
{ return isFormattedOnly; }

// for unit testing
string ASConsole::getLanguageID() const
{ return localizer.getLanguageID(); }

// for unit testing
bool ASConsole::getIsQuiet()
{ return isQuiet; }

// for unit testing
bool ASConsole::getIsRecursive()
{ return isRecursive; }

// for unit testing
bool ASConsole::getIsVerbose()
{ return isVerbose; }

// for unit testing
bool ASConsole::getLineEndsMixed()
{ return lineEndsMixed; }

// for unit testing
bool ASConsole::getNoBackup()
{ return noBackup; }

// for unit testing
string ASConsole::getOptionsFileName()
{ return optionsFileName; }

// for unit testing
vector<string> ASConsole::getOptionsVector()
{ return optionsVector; }

// for unit testing
string ASConsole::getOrigSuffix()
{ return origSuffix; }

// for unit testing
bool ASConsole::getPreserveDate()
{ return preserveDate; }

string ASConsole::getParam(const string &arg, const char* op)
{
	return arg.substr(strlen(op));
}

// initialize output end of line
void ASConsole::initializeOutputEOL(LineEndFormat lineEndFormat)
{
	assert(lineEndFormat == LINEEND_DEFAULT
	       || lineEndFormat == LINEEND_WINDOWS
	       || lineEndFormat == LINEEND_LINUX
	       || lineEndFormat == LINEEND_MACOLD);

	outputEOL[0] = '\0';		// current line end
	prevEOL[0] = '\0';			// previous line end
	lineEndsMixed = false;		// output has mixed line ends, LINEEND_DEFAULT only

	if (lineEndFormat == LINEEND_WINDOWS)
		strcpy(outputEOL, "\r\n");
	else if (lineEndFormat == LINEEND_LINUX)
		strcpy(outputEOL, "\n");
	else if (lineEndFormat == LINEEND_MACOLD)
		strcpy(outputEOL, "\r");
	else
		outputEOL[0] = '\0';
}


FileEncoding ASConsole::readFile(const string &fileName_, stringstream &in) const
{
	const int blockSize = 65536;	// 64 KB
	ifstream fin(fileName_.c_str(), ios::binary);
	if (!fin)
		error("Cannot open input file", fileName_.c_str());
	char* data = new(nothrow) char[blockSize];
	if (!data)
		error("Cannot allocate memory for input file", fileName_.c_str());
	fin.read(data, sizeof(data));
	if (fin.bad())
		error("Cannot read input file", fileName_.c_str());
	size_t dataSize = static_cast<size_t>(fin.gcount());
	FileEncoding encoding = detectEncoding(data, dataSize);
	if (encoding ==  UTF_32BE || encoding ==  UTF_32LE)
		error(_("Cannot process UTF-32 encoding"), fileName_.c_str());
	bool firstBlock = true;
	while (dataSize)
	{
		if (encoding == UTF_16LE || encoding == UTF_16BE)
		{
			// convert utf-16 to utf-8
			size_t utf8Size = Utf8LengthFromUtf16(data, dataSize, encoding);
			char* utf8Out = new(nothrow) char[utf8Size];
			if (!utf8Out)
				error("Cannot allocate memory for utf-8 conversion", fileName_.c_str());
			size_t utf8Len = Utf16ToUtf8(data, dataSize, encoding, firstBlock, utf8Out);
			assert(utf8Len == utf8Size);
			in << string(utf8Out, utf8Len);
			delete []utf8Out;
		}
		else
			in << string(data, dataSize);
		fin.read(data, sizeof(data));
		if (fin.bad())
			error("Cannot read input file", fileName_.c_str());
		dataSize = static_cast<size_t>(fin.gcount());
		firstBlock = false;
	}
	fin.close();
	delete [] data;
	return encoding;
}

void ASConsole::setIgnoreExcludeErrors(bool state)
{ ignoreExcludeErrors = state; }

void ASConsole::setIgnoreExcludeErrorsAndDisplay(bool state)
{ ignoreExcludeErrors = state; ignoreExcludeErrorsDisplay = state; }

void ASConsole::setIsFormattedOnly(bool state)
{ isFormattedOnly = state; }

void ASConsole::setIsQuiet(bool state)
{ isQuiet = state; }

void ASConsole::setIsRecursive(bool state)
{ isRecursive = state; }

void ASConsole::setIsVerbose(bool state)
{ isVerbose = state; }

void ASConsole::setNoBackup(bool state)
{ noBackup = state; }

void ASConsole::setOptionsFileName(string name)
{ optionsFileName = name; }

void ASConsole::setOrigSuffix(string suffix)
{ origSuffix = suffix; }

void ASConsole::setPreserveDate(bool state)
{ preserveDate = state; }

// set outputEOL variable
void ASConsole::setOutputEOL(LineEndFormat lineEndFormat, const char* currentEOL)
{
	if (lineEndFormat == LINEEND_DEFAULT)
	{
		strcpy(outputEOL, currentEOL);
		if (strlen(prevEOL) == 0)
			strcpy(prevEOL, outputEOL);
		if (strcmp(prevEOL, outputEOL) != 0)
		{
			lineEndsMixed = true;
			filesAreIdentical = false;
			strcpy(prevEOL, outputEOL);
		}
	}
	else
	{
		strcpy(prevEOL, currentEOL);
		if (strcmp(prevEOL, outputEOL) != 0)
			filesAreIdentical = false;
	}
}

/**
 * WINDOWS function to display the last system error.
 */
void ASConsole::displayLastError()
{
	LPSTR msgBuf;
	DWORD lastError = GetLastError();
	FormatMessage(
	    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	    NULL,
	    lastError,
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Default language
	    (LPSTR) &msgBuf,
	    0,
	    NULL
	);
	// Display the string.
	(*_err) << "Error (" << lastError << ") " << msgBuf << endl;
	// Free the buffer.
	LocalFree(msgBuf);
}

/**
 * WINDOWS function to get the current directory.
 * NOTE: getenv("CD") does not work for Windows Vista.
 *        	The Windows function GetCurrentDirectory is used instead.
 *
 * @return	The path of the current directory
 */
string ASConsole::getCurrentDirectory(const string &fileName_) const
{
	char currdir[MAX_PATH];
	currdir[0] = '\0';
	if (!GetCurrentDirectory(sizeof(currdir), currdir))
		error("Cannot find file", fileName_.c_str());
	return string(currdir);
}

/**
 * WINDOWS function to resolve wildcards and recurse into sub directories.
 * The fileName vector is filled with the path and names of files to process.
 *
 * @param directory     The path of the directory to be processed.
 * @param wildcard      The wildcard to be processed (e.g. *.cpp).
 */
void ASConsole::getFileNames(const string &directory, const string &wildcard)
{
	vector<string> subDirectory;    // sub directories of directory
	WIN32_FIND_DATA findFileData;   // for FindFirstFile and FindNextFile

	// Find the first file in the directory
	// Find will get at least "." and "..".
	string firstFile = directory + "\\*";
	HANDLE hFind = FindFirstFile(firstFile.c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		// Error (3) The system cannot find the path specified.
		// Error (123) The filename, directory name, or volume label syntax is incorrect.
		// ::FindClose(hFind); before exiting
		displayLastError();
		error(_("Cannot open directory"), directory.c_str());
	}

	// save files and sub directories
	do
	{
		// skip hidden or read only
		if (findFileData.cFileName[0] == '.'
		        || (findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
		        || (findFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
			continue;

		// is this a sub directory
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!isRecursive)
				continue;
			// if a sub directory and recursive, save sub directory
			string subDirectoryPath = directory + g_fileSeparator + findFileData.cFileName;
			if (isPathExclued(subDirectoryPath))
				printMsg(_("Exclude  %s\n"), subDirectoryPath.substr(mainDirectoryLength));
			else
				subDirectory.push_back(subDirectoryPath);
			continue;
		}

		// save the file name
		string filePathName = directory + g_fileSeparator + findFileData.cFileName;
		// check exclude before wildcmp to avoid "unmatched exclude" error
		bool isExcluded = isPathExclued(filePathName);
		// save file name if wildcard match
		if (wildcmp(wildcard.c_str(), findFileData.cFileName))
		{
			if (isExcluded)
				printMsg(_("Exclude  %s\n"), filePathName.substr(mainDirectoryLength));
			else
				fileName.push_back(filePathName);
		}
	}
	while (FindNextFile(hFind, &findFileData) != 0);

	// check for processing error
	::FindClose(hFind);
	DWORD dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
		error("Error processing directory", directory.c_str());

	// recurse into sub directories
	// if not doing recursive subDirectory is empty
	for (unsigned i = 0; i < subDirectory.size(); i++)
		getFileNames(subDirectory[i], wildcard);

	return;
}

/**
 * WINDOWS function to format a number according to the current locale.
 * This formats positive integers only, no float.
 *
 * @param num		The number to be formatted.
 * @param lcid		The LCID of the locale to be used for testing.
 * @return			The formatted number.
 */
string ASConsole::getNumberFormat(int num, size_t lcid) const
{
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__GNUC__)
	// Compilers that don't support C++ locales should still support this assert.
	// The C locale should be set but not the C++.
	// This function is not necessary if the C++ locale is set.
	// The locale().name() return value is not portable to all compilers.
	assert(locale().name() == "C");
#endif
	// convert num to a string
	stringstream alphaNum;
	alphaNum << num;
	string number = alphaNum.str();
	if (useAscii)
		return number;

	// format the number using the Windows API
	if (lcid == 0)
		lcid = LOCALE_USER_DEFAULT;
	int outSize = ::GetNumberFormat(lcid, 0, number.c_str(), NULL, NULL, 0);
	char* outBuf = new(nothrow) char[outSize];
	if (outBuf == NULL)
		return number;
	::GetNumberFormat(lcid, 0, number.c_str(), NULL, outBuf, outSize);
	string formattedNum(outBuf);
	delete [] outBuf;
	// remove the decimal
	int decSize = ::GetLocaleInfo(lcid, LOCALE_SDECIMAL, NULL, 0);
	char* decBuf = new(nothrow) char[decSize];
	if (decBuf == NULL)
		return number;
	::GetLocaleInfo(lcid, LOCALE_SDECIMAL, decBuf, decSize);
	size_t i = formattedNum.rfind(decBuf);
	delete [] decBuf;
	if (i != string::npos)
		formattedNum.erase(i);
	if (!formattedNum.length())
		formattedNum = "0";
	return formattedNum;
}

// get individual file names from the command-line file path
void ASConsole::getFilePaths(string &filePath)
{
	fileName.clear();
	targetDirectory = string();
	targetFilename = string();

	// separate directory and file name
	size_t separator = filePath.find_last_of(g_fileSeparator);
	if (separator == string::npos)
	{
		// if no directory is present, use the currently active directory
		targetDirectory = getCurrentDirectory(filePath);
		targetFilename  = filePath;
		mainDirectoryLength = targetDirectory.length() + 1;    // +1 includes trailing separator
	}
	else
	{
		targetDirectory = filePath.substr(0, separator);
		targetFilename  = filePath.substr(separator + 1);
		mainDirectoryLength = targetDirectory.length() + 1;    // +1 includes trailing separator
	}

	if (targetFilename.length() == 0)
	{
		fprintf(stderr, _("Missing filename in %s\n"), filePath.c_str());
		error();
	}

	// check filename for wildcards
	hasWildcard = false;
	if (targetFilename.find_first_of( "*?") != string::npos)
		hasWildcard = true;

	// clear exclude hits vector
	for (size_t ix = 0; ix < excludeHitsVector.size(); ix++)
		excludeHitsVector[ix] = false;

	// If the filename is not quoted on Linux, bash will replace the
	// wildcard instead of passing it to the program.
	if (isRecursive && !hasWildcard)
	{
		fprintf(stderr, "%s\n", _("Recursive option with no wildcard"));
		error();
	}

	// display directory name for wildcard processing
	if (hasWildcard)
	{
		printSeparatingLine();
		printMsg(_("Directory  %s\n"), targetDirectory + g_fileSeparator + targetFilename);
	}

	// create a vector of paths and file names to process
	if (hasWildcard || isRecursive)
		getFileNames(targetDirectory, targetFilename);
	else
	{
		// verify a single file is not a directory (needed on Linux)
		string entryFilepath = targetDirectory + g_fileSeparator + targetFilename;
		struct stat statbuf;
		if (stat(entryFilepath.c_str(), &statbuf) == 0 && (statbuf.st_mode & S_IFREG))
			fileName.push_back(entryFilepath);
	}

	// check for unprocessed excludes
	bool excludeErr = false;
	for (size_t ix = 0; ix < excludeHitsVector.size(); ix++)
	{
		if (excludeHitsVector[ix] == false)
		{
			excludeErr = true;
			if (!ignoreExcludeErrorsDisplay)
			{
				if (ignoreExcludeErrors)
					printMsg(_("Exclude (unmatched)  %s\n"), excludeVector[ix]);
				else
					fprintf(stderr, _("Exclude (unmatched)  %s\n"), excludeVector[ix].c_str());
			}
			else
			{
				if (!ignoreExcludeErrors)
					fprintf(stderr, _("Exclude (unmatched)  %s\n"), excludeVector[ix].c_str());
			}
		}
	}

	if (excludeErr && !ignoreExcludeErrors)
	{
		if (hasWildcard && !isRecursive)
			fprintf(stderr, "%s\n", _("Did you intend to use --recursive"));
		error();
	}

	// check if files were found (probably an input error if not)
	if (fileName.empty())
	{
		fprintf(stderr, _("No file to process %s\n"), filePath.c_str());
		if (hasWildcard && !isRecursive)
			fprintf(stderr, "%s\n", _("Did you intend to use --recursive"));
		error();
	}

	if (hasWildcard)
		printSeparatingLine();
}

bool ASConsole::fileNameVectorIsEmpty()
{
	return fileNameVector.empty();
}

bool ASConsole::isOption(const string &arg, const char* op)
{
	return arg.compare(op) == 0;
}

bool ASConsole::isOption(const string &arg, const char* a, const char* b)
{
	return (isOption(arg, a) || isOption(arg, b));
}

bool ASConsole::isParamOption(const string &arg, const char* option)
{
	bool retVal = arg.compare(0, strlen(option), option) == 0;
	// if comparing for short option, 2nd char of arg must be numeric
	if (retVal && strlen(option) == 1 && arg.length() > 1)
		if (!isdigit((unsigned char)arg[1]))
			retVal = false;
	return retVal;
}

// compare a path to the exclude vector
// used for both directories and filenames
// updates the g_excludeHitsVector
// return true if a match
bool ASConsole::isPathExclued(const string &subPath)
{
	bool retVal = false;

	// read the exclude vector checking for a match
	for (size_t i = 0; i < excludeVector.size(); i++)
	{
		string exclude = excludeVector[i];

		if (subPath.length() < exclude.length())
			continue;

		size_t compareStart = subPath.length() - exclude.length();
		// subPath compare must start with a directory name
		if (compareStart > 0)
		{
			char lastPathChar = subPath[compareStart - 1];
			if (lastPathChar != g_fileSeparator)
				continue;
		}

		string compare = subPath.substr(compareStart);
		if (!g_isCaseSensitive)
		{
			// make it case insensitive for Windows
			for (size_t j = 0; j < compare.length(); j++)
				compare[j] = (char)tolower(compare[j]);
			for (size_t j = 0; j < exclude.length(); j++)
				exclude[j] = (char)tolower(exclude[j]);
		}
		// compare sub directory to exclude data - must check them all
		if (compare == exclude)
		{
			excludeHitsVector[i] = true;
			retVal = true;
			break;
		}
	}
	return retVal;
}

void ASConsole::printHelp() const
{
	(*_err) << endl;
	(*_err) << "                     Artistic Style for Embedded C/C++\n";
	(*_err) << "                     Maintained by: Nghia Taarabt\n";
	(*_err) << "                     Original Author: Tal Davidson\n";
	(*_err) << endl;
	(*_err) << "Usage  :  TaaRRule [options] Source1.cpp Source2.cpp  [...]\n";
	(*_err) << "          TaaRRule [options] < Original > Beautified\n";
	(*_err) << endl;
	(*_err) << "When indenting a specific file, the resulting indented file RETAINS the\n";
	(*_err) << "original file-name. The original pre-indented file is renamed, with a\n";
	(*_err) << "suffix of \".orig\" added to the original filename.\n";
	(*_err) << endl;
	(*_err) << "Wildcards (* and ?) may be used in the filename.\n";
	(*_err) << "A \'recursive\' option can process directories recursively.\n";
	(*_err) << endl;
	(*_err) << "By default, TaaRRule is set up to indent C/C++ files, with four\n";
	(*_err) << "spaces per indent, a maximal indentation of 40 spaces inside continuous\n";
	(*_err) << "statements, a minimum indentation of eight spaces inside conditional\n";
	(*_err) << "statements, and NO formatting options.\n";
	(*_err) << endl;
	(*_err) << "Option Format:\n";
	(*_err) << "--------------\n";
	(*_err) << "    Long options (starting with '--') must be written one at a time.\n";
	(*_err) << "    Short options (starting with '-') may be appended together.\n";
	(*_err) << "    Thus, -bps4 is the same as -b -p -s4.\n";
	(*_err) << endl;
	(*_err) << "Default Option File:\n";
	(*_err) << "---------------------\n";
	(*_err) << "    Artistic Style looks for a default options file in the\n";
	(*_err) << "    following order:\n";
	(*_err) << "    1. The contents of the ARTISTIC_STYLE_OPTIONS environment\n";
	(*_err) << "       variable if it exists.\n";
	(*_err) << "    2. The file called .astylerc in the directory pointed to by the\n";
	(*_err) << "       HOME environment variable ( i.e. $HOME/.astylerc ).\n";
	(*_err) << "    3. The file called astylerc in the directory pointed to by the\n";
	(*_err) << "       USERPROFILE environment variable ( i.e. %USERPROFILE%\\astylerc ).\n";
	(*_err) << "    If a default options file is found, the options in this file\n";
	(*_err) << "    will be parsed BEFORE the command-line options.\n";
	(*_err) << "    Long options within the default option file may be written without\n";
	(*_err) << "    the preliminary '--'.\n";
	(*_err) << endl;
	(*_err) << "Bracket Style Options:\n";
	(*_err) << "----------------------\n";
	(*_err) << "    --style=allman  OR  --style=ansi   OR  --style=bsd\n";
	(*_err) << "                    OR  --style=break  OR  -A1\n";
	(*_err) << "    Allman style formatting/indenting.\n";
	(*_err) << "    Broken brackets.\n";
	(*_err) << endl;
	(*_err) << "    --style=java  OR  --style=attach  OR  -A2\n";
	(*_err) << "    Java style formatting/indenting.\n";
	(*_err) << "    Attached brackets.\n";
	(*_err) << endl;
	(*_err) << "    --style=kr  OR  --style=k&r  OR  --style=k/r  OR  -A3\n";
	(*_err) << "    Kernighan & Ritchie style formatting/indenting.\n";
	(*_err) << "    Linux brackets.\n";
	(*_err) << endl;
	(*_err) << "    --style=stroustrup  OR  -A4\n";
	(*_err) << "    Stroustrup style formatting/indenting.\n";
	(*_err) << "    Stroustrup brackets.\n";
	(*_err) << endl;
	(*_err) << "    --style=whitesmith  OR  -A5\n";
	(*_err) << "    Whitesmith style formatting/indenting.\n";
	(*_err) << "    Broken, indented brackets.\n";
	(*_err) << "    Indented class blocks and switch blocks.\n";
	(*_err) << endl;
	(*_err) << "    --style=banner  OR  -A6\n";
	(*_err) << "    Banner style formatting/indenting.\n";
	(*_err) << "    Attached, indented brackets.\n";
	(*_err) << "    Indented class blocks and switch blocks.\n";
	(*_err) << endl;
	(*_err) << "    --style=gnu  OR  -A7\n";
	(*_err) << "    GNU style formatting/indenting.\n";
	(*_err) << "    Broken brackets, indented blocks.\n";
	(*_err) << endl;
	(*_err) << "    --style=linux  OR  --style=knf  OR  -A8\n";
	(*_err) << "    Linux style formatting/indenting.\n";
	(*_err) << "    Linux brackets, minimum conditional indent is one-half indent.\n";
	(*_err) << endl;
	(*_err) << "    --style=horstmann  OR  -A9\n";
	(*_err) << "    Horstmann style formatting/indenting.\n";
	(*_err) << "    Run-in brackets, indented switches.\n";
	(*_err) << endl;
	(*_err) << "    --style=1tbs  OR  --style=otbs  OR  -A10\n";
	(*_err) << "    One True Brace Style formatting/indenting.\n";
	(*_err) << "    Linux brackets, add brackets to all conditionals.\n";
	(*_err) << endl;
	(*_err) << "    --style=google  OR  -A14\n";
	(*_err) << "    Google style formatting/indenting.\n";
	(*_err) << "    Attached brackets, indented class modifiers.\n";
	(*_err) << endl;
	(*_err) << "    --style=pico  OR  -A11\n";
	(*_err) << "    Pico style formatting/indenting.\n";
	(*_err) << "    Run-in opening brackets and attached closing brackets.\n";
	(*_err) << "    Uses keep one line blocks and keep one line statements.\n";
	(*_err) << endl;
	(*_err) << "    --style=lisp  OR  -A12\n";
	(*_err) << "    Lisp style formatting/indenting.\n";
	(*_err) << "    Attached opening brackets and attached closing brackets.\n";
	(*_err) << "    Uses keep one line statements.\n";
	(*_err) << endl;
	(*_err) << "Tab Options:\n";
	(*_err) << "------------\n";
	(*_err) << "    default indent option\n";
	(*_err) << "    If no indentation option is set, the default\n";
	(*_err) << "    option of 4 spaces per indent will be used.\n";
	(*_err) << endl;
	(*_err) << "    --indent=spaces=#  OR  -s#\n";
	(*_err) << "    Indent using # spaces per indent. Not specifying #\n";
	(*_err) << "    will result in a default of 4 spaces per indent.\n";
	(*_err) << endl;
	(*_err) << "    --indent=tab  OR  --indent=tab=#  OR  -t  OR  -t#\n";
	(*_err) << "    Indent using tab characters, assuming that each\n";
	(*_err) << "    indent is # spaces long. Not specifying # will result\n";
	(*_err) << "    in a default assumption of 4 spaces per indent.\n";
	(*_err) << endl;
	(*_err) << "    --indent=force-tab=#  OR  -T#\n";
	(*_err) << "    Indent using tab characters, assuming that each\n";
	(*_err) << "    indent is # spaces long. Force tabs to be used in areas\n";
	(*_err) << "    AStyle would prefer to use spaces.\n";
	(*_err) << endl;
	(*_err) << "    --indent=force-tab-x=#  OR  -xT#\n";
	(*_err) << "    Allows the tab length to be set to a length that is different\n";
	(*_err) << "    from the indent length. This may cause the indentation to be\n";
	(*_err) << "    a mix of both spaces and tabs. This option sets the tab length.\n";
	(*_err) << endl;
	(*_err) << "Bracket Modify Options:\n";
	(*_err) << "-------------------------\n";
	(*_err) << "    --attach-namespaces  OR  -xn\n";
	(*_err) << "    Attach brackets to a namespace statement.\n";
	(*_err) << endl;
	(*_err) << "    --attach-classes  OR  -xc\n";
	(*_err) << "    Attach brackets to a class statement.\n";
	(*_err) << endl;
	(*_err) << "    --attach-inlines  OR  -xl\n";
	(*_err) << "    Attach brackets to class inline function definitions.\n";
	(*_err) << endl;
	(*_err) << "    --attach-extern-c  OR  -xk\n";
	(*_err) << "    Attach brackets to an extern \"C\" statement.\n";
	(*_err) << endl;
	(*_err) << "Indentation Options:\n";
	(*_err) << "--------------------\n";
	(*_err) << "    --indent-classes  OR  -C\n";
	(*_err) << "    Indent 'class' blocks so that the entire block is indented.\n";
	(*_err) << endl;
	(*_err) << "    --indent-modifiers  OR  -xG\n";
	(*_err) << "    Indent 'class' access modifiers, 'public:', 'protected:' or\n";
	(*_err) << "    'private:', one half indent. The rest of the class is not\n";
	(*_err) << "    indented. \n";
	(*_err) << endl;
	(*_err) << "    --indent-switches  OR  -S\n";
	(*_err) << "    Indent 'switch' blocks, so that the inner 'case XXX:'\n";
	(*_err) << "    headers are indented in relation to the switch block.\n";
	(*_err) << endl;
	(*_err) << "    --indent-cases  OR  -K\n";
	(*_err) << "    Indent case blocks from the 'case XXX:' headers.\n";
	(*_err) << "    Case statements not enclosed in blocks are NOT indented.\n";
	(*_err) << endl;
	(*_err) << "    --indent-namespaces  OR  -N\n";
	(*_err) << "    Indent the contents of namespace blocks.\n";
	(*_err) << endl;
	(*_err) << "    --indent-labels  OR  -L\n";
	(*_err) << "    Indent labels so that they appear one indent less than\n";
	(*_err) << "    the current indentation level, rather than being\n";
	(*_err) << "    flushed completely to the left (which is the default).\n";
	(*_err) << endl;
	(*_err) << "    --indent-preproc-define  OR  -w\n";
	(*_err) << "    --indent-preprocessor has been depreciated.\n";
	(*_err) << "    Indent multi-line preprocessor #define statements.\n";
	(*_err) << endl;
	(*_err) << "    --indent-preproc-cond  OR  -xw\n";
	(*_err) << "    Indent preprocessor conditional statements #if/#else/#endif\n";
	(*_err) << "    to the same level as the source code.\n";
	(*_err) << endl;
	(*_err) << "    --indent-col1-comments  OR  -Y\n";
	(*_err) << "    Indent line comments that start in column one.\n";
	(*_err) << endl;
	(*_err) << "    --min-conditional-indent=#  OR  -m#\n";
	(*_err) << "    Indent a minimal # spaces in a continuous conditional\n";
	(*_err) << "    belonging to a conditional header.\n";
	(*_err) << "    The valid values are:\n";
	(*_err) << "    0 - no minimal indent.\n";
	(*_err) << "    1 - indent at least one additional indent.\n";
	(*_err) << "    2 - indent at least two additional indents.\n";
	(*_err) << "    3 - indent at least one-half an additional indent.\n";
	(*_err) << "    The default value is 2, two additional indents.\n";
	(*_err) << endl;
	(*_err) << "    --max-instatement-indent=#  OR  -M#\n";
	(*_err) << "    Indent a maximal # spaces in a continuous statement,\n";
	(*_err) << "    relative to the previous line.\n";
	(*_err) << "    The valid values are 40 thru 120.\n";
	(*_err) << "    The default value is 40.\n";
	(*_err) << endl;
	(*_err) << "Padding Options:\n";
	(*_err) << "----------------\n";
	(*_err) << "    --break-blocks  OR  -f\n";
	(*_err) << "    Insert empty lines around unrelated blocks, labels, classes, ...\n";
	(*_err) << endl;
	(*_err) << "    --break-blocks=all  OR  -F\n";
	(*_err) << "    Like --break-blocks, except also insert empty lines \n";
	(*_err) << "    around closing headers (e.g. 'else', 'catch', ...).\n";
	(*_err) << endl;
	(*_err) << "    --pad-oper  OR  -p\n";
	(*_err) << "    Insert space padding around operators.\n";
	(*_err) << endl;
	(*_err) << "    --pad-paren  OR  -P\n";
	(*_err) << "    Insert space padding around parenthesis on both the outside\n";
	(*_err) << "    and the inside.\n";
	(*_err) << endl;
	(*_err) << "    --pad-paren-out  OR  -d\n";
	(*_err) << "    Insert space padding around parenthesis on the outside only.\n";
	(*_err) << endl;
	(*_err) << "    --pad-first-paren-out  OR  -xd\n";
	(*_err) << "    Insert space padding around first parenthesis in a series on\n";
	(*_err) << "    the outside only.\n";
	(*_err) << endl;
	(*_err) << "    --pad-paren-in  OR  -D\n";
	(*_err) << "    Insert space padding around parenthesis on the inside only.\n";
	(*_err) << endl;
	(*_err) << "    --pad-header  OR  -H\n";
	(*_err) << "    Insert space padding after paren headers (e.g. 'if', 'for'...).\n";
	(*_err) << endl;
	(*_err) << "    --unpad-paren  OR  -U\n";
	(*_err) << "    Remove unnecessary space padding around parenthesis. This\n";
	(*_err) << "    can be used in combination with the 'pad' options above.\n";
	(*_err) << endl;
	(*_err) << "    --delete-empty-lines  OR  -xd\n";
	(*_err) << "    Delete empty lines within a function or method.\n";
	(*_err) << "    It will NOT delete lines added by the break-blocks options.\n";
	(*_err) << endl;
	(*_err) << "    --fill-empty-lines  OR  -E\n";
	(*_err) << "    Fill empty lines with the white space of their\n";
	(*_err) << "    previous lines.\n";
	(*_err) << endl;
	(*_err) << "    --align-pointer=type    OR  -k1\n";
	(*_err) << "    --align-pointer=middle  OR  -k2\n";
	(*_err) << "    --align-pointer=name    OR  -k3\n";
	(*_err) << "    Attach a pointer or reference operator (*, &, or ^) to either\n";
	(*_err) << "    the operator type (left), middle, or operator name (right).\n";
	(*_err) << "    To align the reference separately use --align-reference.\n";
	(*_err) << endl;
	(*_err) << "    --align-reference=none    OR  -W0\n";
	(*_err) << "    --align-reference=type    OR  -W1\n";
	(*_err) << "    --align-reference=middle  OR  -W2\n";
	(*_err) << "    --align-reference=name    OR  -W3\n";
	(*_err) << "    Attach a reference operator (&) to either\n";
	(*_err) << "    the operator type (left), middle, or operator name (right).\n";
	(*_err) << "    If not set, follow pointer alignment.\n";
	(*_err) << endl;
	(*_err) << "Formatting Options:\n";
	(*_err) << "-------------------\n";
	(*_err) << "    --break-closing-brackets  OR  -y\n";
	(*_err) << "    Break brackets before closing headers (e.g. 'else', 'catch', ...).\n";
	(*_err) << "    Use with --style=java, --style=kr, --style=stroustrup,\n";
	(*_err) << "    --style=linux, or --style=1tbs.\n";
	(*_err) << endl;
	(*_err) << "    --break-elseifs  OR  -e\n";
	(*_err) << "    Break 'else if()' statements into two different lines.\n";
	(*_err) << endl;
	(*_err) << "    --add-brackets  OR  -j\n";
	(*_err) << "    Add brackets to unbracketed one line conditional statements.\n";
	(*_err) << endl;
	(*_err) << "    --add-one-line-brackets  OR  -J\n";
	(*_err) << "    Add one line brackets to unbracketed one line conditional\n";
	(*_err) << "    statements.\n";
	(*_err) << endl;
	(*_err) << "    --remove-brackets  OR  -xj\n";
	(*_err) << "    Remove brackets from a bracketed one line conditional statements.\n";
	(*_err) << endl;
	(*_err) << "    --keep-one-line-blocks  OR  -O\n";
	(*_err) << "    Don't break blocks residing completely on one line.\n";
	(*_err) << endl;
	(*_err) << "    --keep-one-line-statements  OR  -o\n";
	(*_err) << "    Don't break lines containing multiple statements into\n";
	(*_err) << "    multiple single-statement lines.\n";
	(*_err) << endl;
	(*_err) << "    --convert-tabs  OR  -c\n";
	(*_err) << "    Convert tabs to the appropriate number of spaces.\n";
	(*_err) << endl;
	(*_err) << "    --close-templates  OR  -xy\n";
	(*_err) << "    Close ending angle brackets on template definitions.\n";
	(*_err) << endl;
	(*_err) << "    --remove-comment-prefix  OR  -xp\n";
	(*_err) << "    Remove the leading '*' prefix on multi-line comments and\n";
	(*_err) << "    indent the comment text one indent.\n";
	(*_err) << endl;
	(*_err) << "    --max-code-length=#    OR  -xC#\n";
	(*_err) << "    --break-after-logical  OR  -xL\n";
	(*_err) << "    max-code-length=# will break the line if it exceeds more than\n";
	(*_err) << "    # characters. The valid values are 50 thru 200.\n";
	(*_err) << "    If the line contains logical conditionals they will be placed\n";
	(*_err) << "    first on the new line. The option break-after-logical will\n";
	(*_err) << "    cause the logical conditional to be placed last on the\n";
	(*_err) << "    previous line.\n";
	(*_err) << endl;
	(*_err) << "    --mode=c\n";
	(*_err) << "    Indent a C or C++ source file (this is the default).\n";
	(*_err) << endl;
	(*_err) << "Objective-C Options:\n";
	(*_err) << "--------------------\n";
	(*_err) << "    --align-method-colon  OR  -xM\n";
	(*_err) << "    Align the colons in an Objective-C method definition.\n";
	(*_err) << endl;
	(*_err) << "    --pad-method-prefix  OR  -xQ\n";
	(*_err) << "    Insert space padding after the '-' or '+' Objective-C\n";
	(*_err) << "    method prefix.\n";
	(*_err) << endl;
	(*_err) << "    --unpad-method-prefix  OR  -xR\n";
	(*_err) << "    Remove all space padding after the '-' or '+' Objective-C\n";
	(*_err) << "    method prefix.\n";
	(*_err) << endl;
	(*_err) << "    --pad-method-colon=none    OR  -xP\n";
	(*_err) << "    --pad-method-colon=all     OR  -xP1\n";
	(*_err) << "    --pad-method-colon=after   OR  -xP2\n";
	(*_err) << "    --pad-method-colon=before  OR  -xP3\n";
	(*_err) << "    Add or remove space padding before or after the colons in an\n";
	(*_err) << "    Objective-C method call.\n";
	(*_err) << endl;
	(*_err) << "Other Options:\n";
	(*_err) << "--------------\n";
	(*_err) << "    --suffix=####\n";
	(*_err) << "    Append the suffix #### instead of '.orig' to original filename.\n";
	(*_err) << endl;
	(*_err) << "    --suffix=none  OR  -n\n";
	(*_err) << "    Do not retain a backup of the original file.\n";
	(*_err) << endl;
	(*_err) << "    --recursive  OR  -r  OR  -R\n";
	(*_err) << "    Process subdirectories recursively.\n";
	(*_err) << endl;
	(*_err) << "    --exclude=####\n";
	(*_err) << "    Specify a file or directory #### to be excluded from processing.\n";
	(*_err) << endl;
	(*_err) << "    --ignore-exclude-errors  OR  -i\n";
	(*_err) << "    Allow processing to continue if there are errors in the exclude=###\n";
	(*_err) << "    options. It will display the unmatched excludes.\n";
	(*_err) << endl;
	(*_err) << "    --ignore-exclude-errors-x  OR  -xi\n";
	(*_err) << "    Allow processing to continue if there are errors in the exclude=###\n";
	(*_err) << "    options. It will NOT display the unmatched excludes.\n";
	(*_err) << endl;
	(*_err) << "    --errors-to-stdout  OR  -X\n";
	(*_err) << "    Print errors and help information to standard-output rather than\n";
	(*_err) << "    to standard-error.\n";
	(*_err) << endl;
	(*_err) << "    --preserve-date  OR  -Z\n";
	(*_err) << "    Preserve the original file's date and time modified. The time\n";
	(*_err) << "     modified will be changed a few micro seconds to force a compile.\n";
	(*_err) << endl;
	(*_err) << "    --verbose  OR  -v\n";
	(*_err) << "    Verbose mode. Extra informational messages will be displayed.\n";
	(*_err) << endl;
	(*_err) << "    --formatted  OR  -Q\n";
	(*_err) << "    Formatted display mode. Display only the files that have been\n";
	(*_err) << "    formatted.\n";
	(*_err) << endl;
	(*_err) << "    --quiet  OR  -q\n";
	(*_err) << "    Quiet mode. Suppress all output except error messages.\n";
	(*_err) << endl;
	(*_err) << "    --lineend=windows  OR  -z1\n";
	(*_err) << "    --lineend=linux    OR  -z2\n";
	(*_err) << "    --lineend=macold   OR  -z3\n";
	(*_err) << "    Force use of the specified line end style. Valid options\n";
	(*_err) << "    are windows (CRLF), linux (LF), and macold (CR).\n";
	(*_err) << endl;
	(*_err) << "Command Line Only:\n";
	(*_err) << "------------------\n";
	(*_err) << "    --options=####\n";
	(*_err) << "    Specify an options file #### to read and use.\n";
	(*_err) << endl;
	(*_err) << "    --options=none\n";
	(*_err) << "    Disable the default options file.\n";
	(*_err) << "    Only the command-line parameters will be used.\n";
	(*_err) << endl;
	(*_err) << "    --ascii  OR  -I\n";
	(*_err) << "    The displayed output will be ascii characters only.\n";
	(*_err) << endl;
	(*_err) << "    --version  OR  -V\n";
	(*_err) << "    Print version number.\n";
	(*_err) << endl;
	(*_err) << "    --help  OR  -h  OR  -?\n";
	(*_err) << "    Print this help message.\n";
	(*_err) << endl;
}


/**
 * Process files in the fileNameVector.
 */
void ASConsole::processFiles()
{
	if (isVerbose)
		printVerboseHeader();

	clock_t startTime = clock();     // start time of file formatting

	// loop thru input fileNameVector and process the files
	for (size_t i = 0; i < fileNameVector.size(); i++)
	{
		getFilePaths(fileNameVector[i]);

		// loop thru fileName vector formatting the files
		for (size_t j = 0; j < fileName.size(); j++)
			formatFile(fileName[j]);
	}

	// files are processed, display stats
	if (isVerbose)
		printVerboseStats(startTime);
}

// process options from the command line and options file
// build the vectors fileNameVector, excludeVector, optionsVector, and fileOptionsVector
void ASConsole::processOptions(vector<string> &argvOptions)
{
	string arg;
	bool ok = true;
	bool shouldParseOptionsFile = true;

	// get command line options
	for (size_t i = 0; i < argvOptions.size(); i++)
	{
		arg = argvOptions[i];

		if ( isOption(arg, "-I" )
		        || isOption(arg, "--ascii") )
		{
			useAscii = true;
			setlocale(LC_ALL, "C");		// use English decimal indicator
		}
		else if ( isOption(arg, "--options=none") )
		{
			shouldParseOptionsFile = false;
		}
		else if ( isParamOption(arg, "--options=") )
		{
			optionsFileName = getParam(arg, "--options=");
			optionsFileRequired = true;
			if (optionsFileName.compare("") == 0)
				setOptionsFileName(" ");
		}
		else if ( isOption(arg, "-h")
		          || isOption(arg, "--help")
		          || isOption(arg, "-?") )
		{
			printHelp();
			exit(EXIT_SUCCESS);
		}
		else if ( isOption(arg, "-V" )
		          || isOption(arg, "--version") )
		{
			(*_err) << "Artistic Style for Embedded C/C++ \n";
			exit(EXIT_SUCCESS);
		}
		else if (arg[0] == '-')
		{
			optionsVector.push_back(arg);
		}
		else // file-name
		{
			standardizePath(arg);
			fileNameVector.push_back(arg);
		}
	}

	// get options file path and name
	if (shouldParseOptionsFile)
	{
		if (optionsFileName.compare("") == 0)
		{
			char* env = getenv("ARTISTIC_STYLE_OPTIONS");
			if (env != NULL)
				setOptionsFileName(env);
		}
		if (optionsFileName.compare("") == 0)
		{
			char* env = getenv("HOME");
			if (env != NULL)
				setOptionsFileName(string(env) + "/.astylerc");
		}
		if (optionsFileName.compare("") == 0)
		{
			char* env = getenv("USERPROFILE");
			if (env != NULL)
				setOptionsFileName(string(env) + "/astylerc");
		}
		if (optionsFileName.compare("") != 0)
			standardizePath(optionsFileName);
	}

	// create the options file vector and parse the options for errors
	ASOptions options(formatter);
	if (optionsFileName.compare("") != 0)
	{
		ifstream optionsIn(optionsFileName.c_str());
		if (optionsIn)
		{
			options.importOptions(optionsIn, fileOptionsVector);
			ok = options.parseOptions(fileOptionsVector,
			                          string(_("Invalid option file options:")));
		}
		else
		{
			if (optionsFileRequired)
				error(_("Cannot open options file"), optionsFileName.c_str());
			optionsFileName.clear();
		}
		optionsIn.close();
	}
	if (!ok)
	{
		(*_err) << options.getOptionErrors() << endl;
		(*_err) << _("For help on options type 'TaaRRule -h'") << endl;
		error();
	}

	// parse the command line options vector for errors
	ok = options.parseOptions(optionsVector,
	                          string(_("Invalid command line options:")));
	if (!ok)
	{
		(*_err) << options.getOptionErrors() << endl;
		(*_err) << _("For help on options type 'TaaRRule -h'") << endl;
		error();
	}
}

// remove a file and check for an error
void ASConsole::removeFile(const char* fileName_, const char* errMsg) const
{
	if (remove(fileName_))
	{
		if (errno == ENOENT)        // no file is OK
			errno = 0;
		if (errno)
		{
			perror("errno message");
			error(errMsg, fileName_);
		}
	}
}

// rename a file and check for an error
void ASConsole::renameFile(const char* oldFileName, const char* newFileName, const char* errMsg) const
{
	int result = rename(oldFileName, newFileName);
	if (result != 0)
	{
		// if file still exists the remove needs more time - retry
		if (errno == EEXIST)
		{
			errno = 0;
			waitForRemove(newFileName);
			result = rename(oldFileName, newFileName);
		}
		if (result != 0)
		{
			perror("errno message");
			error(errMsg, oldFileName);
		}
	}
}

// make sure file separators are correct type (Windows or Linux)
// remove ending file separator
// remove beginning file separator if requested and NOT a complete file path
void ASConsole::standardizePath(string &path, bool removeBeginningSeparator /*false*/) const
{
	// make sure separators are correct type (Windows or Linux)
	for (size_t i = 0; i < path.length(); i++)
	{
		i = path.find_first_of("/\\", i);
		if (i == string::npos)
			break;
		path[i] = g_fileSeparator;
	}
	if (removeBeginningSeparator && (path[0] == g_fileSeparator))
	{
		path.erase(0, 1);
	}
}

void ASConsole::printMsg(const char* msg, const string &data) const
{
	if (isQuiet)
		return;
	printf(msg, data.c_str());
}

void ASConsole::printSeparatingLine() const
{
	string line;
	for (size_t i = 0; i < 60; i++)
		line.append("-");
	printMsg("%s\n", line);
}

void ASConsole::printVerboseHeader() const
{
	assert(isVerbose);
	if (isQuiet)
		return;
	// get the date
	struct tm* ptr;
	time_t lt;
	char str[20];
	lt = time(NULL);
	ptr = localtime(&lt);
	strftime(str, 20, "%x", ptr);
	if (!optionsFileName.empty())
	{
		printf(_("Using default options file %s\n"), optionsFileName.c_str());
	}
}

void ASConsole::printVerboseStats(clock_t startTime) const
{
	assert(isVerbose);
	if (isQuiet)
		return;
	if (hasWildcard)
		printSeparatingLine();
	string formatted = getNumberFormat(filesFormatted);
	string unchanged = getNumberFormat(filesUnchanged);
	printf(_(" %s formatted   %s unchanged   "), formatted.c_str(), unchanged.c_str());

	// show processing time
	clock_t stopTime = clock();
	float secs = (stopTime - startTime) / float (CLOCKS_PER_SEC);
	if (secs < 60)
	{
		if (secs < 2.0)
			printf("%.2f", secs);
		else if (secs < 20.0)
			printf("%.1f", secs);
		else
			printf("%.0f", secs);
		printf("%s", _(" seconds   "));
	}
	else
	{
		// show minutes and seconds if time is greater than one minute
		int min = (int) secs / 60;
		secs -= min * 60;
		int minsec = int (secs + .5);
		printf(_("%d min %d sec   "), min, minsec);
	}

	string lines = getNumberFormat(linesOut);
	printf(_("%s lines\n"), lines.c_str());
}

void ASConsole::sleep(int seconds) const
{
	clock_t endwait;
	endwait = clock_t (clock () + seconds * CLOCKS_PER_SEC);
	while (clock() < endwait) {}
}

bool ASConsole::stringEndsWith(const string &str, const string &suffix) const
{
	int strIndex = (int) str.length() - 1;
	int suffixIndex = (int) suffix.length() - 1;

	while (strIndex >= 0 && suffixIndex >= 0)
	{
		if (tolower(str[strIndex]) != tolower(suffix[suffixIndex]))
			return false;

		--strIndex;
		--suffixIndex;
	}
	// suffix longer than string
	if (strIndex < 0 && suffixIndex >= 0)
		return false;
	return true;
}

// Swap the two low order bytes of an integer value
// and convert 8 bit encoding to 16 bit.
int ASConsole::swap8to16bit(int value) const
{
	return ( ((value & 0xFF) << 8) + (value >> 8) );
}

// Swap the two low order bytes of a 16 bit integer value.
int ASConsole::swap16bit(int value) const
{
	return ( ((value & 0xff) << 8) | ((value & 0xff00) >> 8) );
}

void ASConsole::updateExcludeVector(string suffixParam)
{
	excludeVector.push_back(suffixParam);
	standardizePath(excludeVector.back(), true);
	excludeHitsVector.push_back(false);
}

// Adapted from SciTE UniConversion.cxx.
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// Modified for Artistic Style by Jim Pattee.
//
// Compute the length of an output utf-8 file given a utf-16 file.
// Input tlen is the size in BYTES (not wchar_t).
size_t ASConsole::Utf8LengthFromUtf16(const char* data, size_t tlen, FileEncoding encoding) const
{
	enum { SURROGATE_LEAD_FIRST = 0xD800 };
	enum { SURROGATE_TRAIL_LAST = 0xDFFF };

	size_t len = 0;
	size_t wcharLen = tlen / 2;
	const short* uptr = reinterpret_cast<const short*>(data);
	for (size_t i = 0; i < wcharLen && uptr[i];)
	{
		size_t uch = encoding == UTF_16BE ? swap16bit(uptr[i]) : uptr[i];
		if (uch < 0x80)
		{
			len++;
		}
		else if (uch < 0x800)
		{
			len += 2;
		}
		else if ((uch >= SURROGATE_LEAD_FIRST) && (uch <= SURROGATE_TRAIL_LAST))
		{
			len += 4;
			i++;
		}
		else
		{
			len += 3;
		}
		i++;
	}
	return len;
}

// Adapted from SciTE Utf8_16.cxx.
// Copyright (C) 2002 Scott Kirkwood.
// Modified for Artistic Style by Jim Pattee.
//
// Convert a utf-8 file to utf-16.
size_t ASConsole::Utf8ToUtf16(char* utf8In, size_t inLen, FileEncoding encoding, char* utf16Out) const
{
	typedef unsigned short utf16;	// 16 bits
	typedef unsigned char  ubyte;	// 8 bits
	enum { SURROGATE_LEAD_FIRST = 0xD800 };
	enum { SURROGATE_LEAD_LAST = 0xDBFF };
	enum { SURROGATE_TRAIL_FIRST = 0xDC00 };
	enum { SURROGATE_TRAIL_LAST = 0xDFFF };
	enum { SURROGATE_FIRST_VALUE = 0x10000 };
	enum eState { eStart, eSecondOf4Bytes, ePenultimate, eFinal };

	int nCur = 0;
	ubyte* pRead = reinterpret_cast<ubyte*>(utf8In);
	utf16* pCur = reinterpret_cast<utf16*>(utf16Out);
	const ubyte* pEnd = pRead + inLen;
	const utf16* pCurStart = pCur;
	eState eState = eStart;

	// the BOM will automatically be converted to utf-16
	while (pRead < pEnd)
	{
		switch (eState)
		{
		case eStart:
			if ((0xF0 & *pRead) == 0xF0)
			{
				nCur = (0x7 & *pRead) << 18;
				eState = eSecondOf4Bytes;
			}
			else if ((0xE0 & *pRead) == 0xE0)
			{
				nCur = (~0xE0 & *pRead) << 12;
				eState = ePenultimate;
			}
			else if ((0xC0 & *pRead) == 0xC0)
			{
				nCur = (~0xC0 & *pRead) << 6;
				eState = eFinal;
			}
			else
			{
				nCur = *pRead;
				eState = eStart;
			}
			break;
		case eSecondOf4Bytes:
			nCur |= (0x3F & *pRead) << 12;
			eState = ePenultimate;
			break;
		case ePenultimate:
			nCur |= (0x3F & *pRead) << 6;
			eState = eFinal;
			break;
		case eFinal:
			nCur |= (0x3F & *pRead);
			eState = eStart;
			break;
		default:
			error("Bad eState value", "Utf8ToUtf16()");
		}
		++pRead;

		if (eState == eStart)
		{
			int codePoint = nCur;
			if (codePoint >= SURROGATE_FIRST_VALUE)
			{
				codePoint -= SURROGATE_FIRST_VALUE;
				int lead = (codePoint >> 10) + SURROGATE_LEAD_FIRST;
				*pCur++ = static_cast<utf16>((encoding == UTF_16BE) ?
				                             swap8to16bit(lead) : lead);
				int trail = (codePoint & 0x3ff) + SURROGATE_TRAIL_FIRST;
				*pCur++ = static_cast<utf16>((encoding == UTF_16BE) ?
				                             swap8to16bit(trail) : trail);
			}
			else
			{
				*pCur++ = static_cast<utf16>((encoding == UTF_16BE) ?
				                             swap8to16bit(codePoint) : codePoint);
			}
		}
	}
	// return value is the output length in BYTES (not wchar_t)
	return (pCur - pCurStart) * 2;
}

// Adapted from SciTE UniConversion.cxx.
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// Modified for Artistic Style by Jim Pattee.
//
// Compute the length of an output utf-16 file given a utf-8 file.
// Return value is the size in BYTES (not wchar_t).
size_t ASConsole::Utf16LengthFromUtf8(const char* data, size_t len) const
{
	size_t ulen = 0;
	size_t charLen;
	for (size_t i = 0; i < len;)
	{
		unsigned char ch = static_cast<unsigned char>(data[i]);
		if (ch < 0x80)
			charLen = 1;
		else if (ch < 0x80 + 0x40 + 0x20)
			charLen = 2;
		else if (ch < 0x80 + 0x40 + 0x20 + 0x10)
			charLen = 3;
		else
		{
			charLen = 4;
			ulen++;
		}
		i += charLen;
		ulen++;
	}
	// return value is the length in bytes (not wchar_t)
	return ulen * 2;
}

// Adapted from SciTE Utf8_16.cxx.
// Copyright (C) 2002 Scott Kirkwood.
// Modified for Artistic Style by Jim Pattee.
//
// Convert a utf-16 file to utf-8.
size_t ASConsole::Utf16ToUtf8(char* utf16In, size_t inLen, FileEncoding encoding,
                              bool firstBlock, char* utf8Out) const
{
	typedef unsigned short utf16;	// 16 bits
	typedef unsigned char  ubyte;	// 8 bits
	enum { SURROGATE_LEAD_FIRST = 0xD800 };
	enum { SURROGATE_LEAD_LAST = 0xDBFF };
	enum { SURROGATE_TRAIL_FIRST = 0xDC00 };
	enum { SURROGATE_TRAIL_LAST = 0xDFFF };
	enum { SURROGATE_FIRST_VALUE = 0x10000 };
	enum eState { eStart, eSecondOf4Bytes, ePenultimate, eFinal };

	int nCur16 = 0;
	int nCur = 0;
	ubyte* pRead = reinterpret_cast<ubyte*>(utf16In);
	ubyte* pCur = reinterpret_cast<ubyte*>(utf8Out);
	const ubyte* pEnd = pRead + inLen;
	const ubyte* pCurStart = pCur;
	static eState eState = eStart;	// eState is retained for subsequent blocks
	if (firstBlock)
		eState = eStart;

	// the BOM will automatically be converted to utf-8
	while (pRead < pEnd)
	{
		switch (eState)
		{
		case eStart:
			if (pRead >= pEnd)
			{
				++pRead;
				break;
			}
			if (encoding == UTF_16LE)
			{
				nCur16 = *pRead++;
				nCur16 |= static_cast<utf16>(*pRead << 8);
			}
			else
			{
				nCur16 = static_cast<utf16>(*pRead++ << 8);
				nCur16 |= static_cast<utf16>(*pRead);
			}
			if (nCur16 >= SURROGATE_LEAD_FIRST && nCur16 <= SURROGATE_LEAD_LAST)
			{
				++pRead;
				int trail;
				if (encoding == UTF_16LE)
				{
					trail = *pRead++;
					trail |= static_cast<utf16>(*pRead << 8);
				}
				else
				{
					trail = static_cast<utf16>(*pRead++ << 8);
					trail |= static_cast<utf16>(*pRead);
				}
				nCur16 = (((nCur16 & 0x3ff) << 10) | (trail & 0x3ff)) + SURROGATE_FIRST_VALUE;
			}
			++pRead;

			if (nCur16 < 0x80)
			{
				nCur = static_cast<ubyte>(nCur16 & 0xFF);
				eState = eStart;
			}
			else if (nCur16 < 0x800)
			{
				nCur = static_cast<ubyte>(0xC0 | (nCur16 >> 6));
				eState = eFinal;
			}
			else if (nCur16 < SURROGATE_FIRST_VALUE)
			{
				nCur = static_cast<ubyte>(0xE0 | (nCur16 >> 12));
				eState = ePenultimate;
			}
			else
			{
				nCur = static_cast<ubyte>(0xF0 | (nCur16 >> 18));
				eState = eSecondOf4Bytes;
			}
			break;
		case eSecondOf4Bytes:
			nCur = static_cast<ubyte>(0x80 | ((nCur16 >> 12) & 0x3F));
			eState = ePenultimate;
			break;
		case ePenultimate:
			nCur = static_cast<ubyte>(0x80 | ((nCur16 >> 6) & 0x3F));
			eState = eFinal;
			break;
		case eFinal:
			nCur = static_cast<ubyte>(0x80 | (nCur16 & 0x3F));
			eState = eStart;
			break;
		default:
			error("Bad eState value", "Utf16ToUtf8()");
		}
		*pCur++ = static_cast<ubyte>(nCur);
	}
	return pCur - pCurStart;
}

int ASConsole::waitForRemove(const char* newFileName) const
{
	struct stat stBuf;
	int seconds;
	// sleep a max of 20 seconds for the remove
	for (seconds = 1; seconds <= 20; seconds++)
	{
		sleep(1);
		if (stat(newFileName, &stBuf) != 0)
			break;
	}
	errno = 0;
	return seconds;
}

// From The Code Project http://www.codeproject.com/string/wildcmp.asp
// Written by Jack Handy - jakkhandy@hotmail.com
// Modified to compare case insensitive for Windows
int ASConsole::wildcmp(const char* wild, const char* data) const
{
	const char* cp = NULL, *mp = NULL;
	bool cmpval;

	while ((*data) && (*wild != '*'))
	{
		if (!g_isCaseSensitive)
			cmpval = (tolower(*wild) != tolower(*data)) && (*wild != '?');
		else
			cmpval = (*wild != *data) && (*wild != '?');

		if (cmpval)
		{
			return 0;
		}
		wild++;
		data++;
	}

	while (*data)
	{
		if (*wild == '*')
		{
			if (!*++wild)
			{
				return 1;
			}
			mp = wild;
			cp = data + 1;
		}
		else
		{
			if (!g_isCaseSensitive)
				cmpval = (tolower(*wild) == tolower(*data) || (*wild == '?'));
			else
				cmpval = (*wild == *data) || (*wild == '?');

			if (cmpval)
			{
				wild++;
				data++;
			}
			else
			{
				wild = mp;
				data = cp++;
			}
		}
	}

	while (*wild == '*')
	{
		wild++;
	}
	return !*wild;
}

void ASConsole::writeFile(const string &fileName_, FileEncoding encoding, ostringstream &out) const
{
	// save date accessed and date modified of original file
	struct stat stBuf;
	bool statErr = false;
	if (stat(fileName_.c_str(), &stBuf) == -1)
		statErr = true;

	// create a backup
	if (!noBackup)
	{
		string origFileName = fileName_ + origSuffix;
		removeFile(origFileName.c_str(), "Cannot remove pre-existing backup file");
		renameFile(fileName_.c_str(), origFileName.c_str(), "Cannot create backup file");
	}

	// write the output file
	ofstream fout(fileName_.c_str(), ios::binary | ios::trunc);
	if (!fout)
		error("Cannot open output file", fileName_.c_str());
	if (encoding == UTF_16LE || encoding == UTF_16BE)
	{
		// convert utf-8 to utf-16
		size_t utf16Size = Utf16LengthFromUtf8(out.str().c_str(), out.str().length());
		char* utf16Out = new char[utf16Size];
		size_t utf16Len = Utf8ToUtf16(const_cast<char*>(out.str().c_str()), out.str().length(), encoding, utf16Out);
		assert(utf16Len == utf16Size);
		fout << string(utf16Out, utf16Len);
		delete []utf16Out;
	}
	else
		fout << out.str();

	fout.close();

	// change date modified to original file date
	// Embarcadero must be linked with cw32mt not cw32
	if (preserveDate)
	{
		if (!statErr)
		{
			struct utimbuf outBuf;
			outBuf.actime = stBuf.st_atime;
			// add ticks so 'make' will recoginze a change
			// Visual Studio 2008 needs more than 1
			outBuf.modtime = stBuf.st_mtime + 10;
			if (utime(fileName_.c_str(), &outBuf) == -1)
				statErr = true;
		}
		if (statErr)
		{
			perror("errno message");
			(*_err) << "*********  Cannot preserve file date" << endl;
		}
	}
}

//-----------------------------------------------------------------------------
// ASOptions class
// used by both console and library builds
//-----------------------------------------------------------------------------

/**
 * parse the options vector
 * optionsVector can be either a fileOptionsVector (options file) or an optionsVector (command line)
 *
 * @return        true if no errors, false if errors
 */
bool ASOptions::parseOptions(vector<string> &optionsVector, const string &errorInfo)
{
	vector<string>::iterator option;
	string arg, subArg;
	optionErrors.clear();

	for (option = optionsVector.begin(); option != optionsVector.end(); ++option)
	{
		arg = *option;

		if (arg.compare(0, 2, "--") == 0)
			parseOption(arg.substr(2), errorInfo);
		else if (arg[0] == '-')
		{
			size_t i;

			for (i = 1; i < arg.length(); ++i)
			{
				if (i > 1
				        && isalpha((unsigned char)arg[i])
				        && arg[i - 1] != 'x')
				{
					// parse the previous option in subArg
					parseOption(subArg, errorInfo);
					subArg = "";
				}
				// append the current option to subArg
				subArg.append(1, arg[i]);
			}
			// parse the last option
			parseOption(subArg, errorInfo);
			subArg = "";
		}
		else
		{
			parseOption(arg, errorInfo);
			subArg = "";
		}
	}
	if (optionErrors.str().length() > 0)
		return false;
	return true;
}

void ASOptions::parseOption(const string &arg, const string &errorInfo)
{
	if ( isOption(arg, "style=allman") || isOption(arg, "style=ansi")
	        || isOption(arg, "style=bsd") || isOption(arg, "style=break") )
	{
		formatter.setFormattingStyle(STYLE_ALLMAN);
	}
	else if ( isOption(arg, "style=java") || isOption(arg, "style=attach") )
	{
		formatter.setFormattingStyle(STYLE_JAVA);
	}
	else if ( isOption(arg, "style=k&r") || isOption(arg, "style=kr") || isOption(arg, "style=k/r") )
	{
		formatter.setFormattingStyle(STYLE_KR);
	}
	else if ( isOption(arg, "style=stroustrup") )
	{
		formatter.setFormattingStyle(STYLE_STROUSTRUP);
	}
	else if ( isOption(arg, "style=whitesmith") )
	{
		formatter.setFormattingStyle(STYLE_WHITESMITH);
	}
	else if ( isOption(arg, "style=banner") )
	{
		formatter.setFormattingStyle(STYLE_BANNER);
	}
	else if ( isOption(arg, "style=gnu") )
	{
		formatter.setFormattingStyle(STYLE_GNU);
	}
	else if ( isOption(arg, "style=linux") || isOption(arg, "style=knf") )
	{
		formatter.setFormattingStyle(STYLE_LINUX);
	}
	else if ( isOption(arg, "style=horstmann") )
	{
		formatter.setFormattingStyle(STYLE_HORSTMANN);
	}
	else if ( isOption(arg, "style=1tbs") || isOption(arg, "style=otbs") )
	{
		formatter.setFormattingStyle(STYLE_1TBS);
	}
	else if ( isOption(arg, "style=google") )
	{
		formatter.setFormattingStyle(STYLE_GOOGLE);
	}
	else if ( isOption(arg, "style=pico") )
	{
		formatter.setFormattingStyle(STYLE_PICO);
	}
	else if ( isOption(arg, "style=lisp") || isOption(arg, "style=python") )
	{
		formatter.setFormattingStyle(STYLE_LISP);
	}
	else if ( isParamOption(arg, "A") )
	{
		int style = 0;
		string styleParam = getParam(arg, "A");
		if (styleParam.length() > 0)
			style = atoi(styleParam.c_str());
		if (style == 1)
			formatter.setFormattingStyle(STYLE_ALLMAN);
		else if (style == 2)
			formatter.setFormattingStyle(STYLE_JAVA);
		else if (style == 3)
			formatter.setFormattingStyle(STYLE_KR);
		else if (style == 4)
			formatter.setFormattingStyle(STYLE_STROUSTRUP);
		else if (style == 5)
			formatter.setFormattingStyle(STYLE_WHITESMITH);
		else if (style == 6)
			formatter.setFormattingStyle(STYLE_BANNER);
		else if (style == 7)
			formatter.setFormattingStyle(STYLE_GNU);
		else if (style == 8)
			formatter.setFormattingStyle(STYLE_LINUX);
		else if (style == 9)
			formatter.setFormattingStyle(STYLE_HORSTMANN);
		else if (style == 10)
			formatter.setFormattingStyle(STYLE_1TBS);
		else if (style == 11)
			formatter.setFormattingStyle(STYLE_PICO);
		else if (style == 12)
			formatter.setFormattingStyle(STYLE_LISP);
		else if (style == 14)
			formatter.setFormattingStyle(STYLE_GOOGLE);
		else isOptionError(arg, errorInfo);
	}
	else if ( isOption(arg, "mode=c") )
	{
		formatter.setCStyle();
		formatter.setModeManuallySet(true);
	}
	else if ( isParamOption(arg, "t", "indent=tab=") )
	{
		int spaceNum = 4;
		string spaceNumParam = getParam(arg, "t", "indent=tab=");
		if (spaceNumParam.length() > 0)
			spaceNum = atoi(spaceNumParam.c_str());
		if (spaceNum < 2 || spaceNum > 20)
			isOptionError(arg, errorInfo);
		else
		{
			formatter.setTabIndentation(spaceNum, false);
		}
	}
	else if ( isOption(arg, "indent=tab") )
	{
		formatter.setTabIndentation(4);
	}
	else if ( isParamOption(arg, "T", "indent=force-tab=") )
	{
		int spaceNum = 4;
		string spaceNumParam = getParam(arg, "T", "indent=force-tab=");
		if (spaceNumParam.length() > 0)
			spaceNum = atoi(spaceNumParam.c_str());
		if (spaceNum < 2 || spaceNum > 20)
			isOptionError(arg, errorInfo);
		else
		{
			formatter.setTabIndentation(spaceNum, true);
		}
	}
	else if ( isOption(arg, "indent=force-tab") )
	{
		formatter.setTabIndentation(4, true);
	}
	else if ( isParamOption(arg, "xT", "indent=force-tab-x=") )
	{
		int tabNum = 8;
		string tabNumParam = getParam(arg, "xT", "indent=force-tab-x=");
		if (tabNumParam.length() > 0)
			tabNum = atoi(tabNumParam.c_str());
		if (tabNum < 2 || tabNum > 20)
			isOptionError(arg, errorInfo);
		else
		{
			formatter.setForceTabXIndentation(tabNum);
		}
	}
	else if ( isOption(arg, "indent=force-tab-x") )
	{
		formatter.setForceTabXIndentation(8);
	}
	else if ( isParamOption(arg, "s", "indent=spaces=") )
	{
		int spaceNum = 4;
		string spaceNumParam = getParam(arg, "s", "indent=spaces=");
		if (spaceNumParam.length() > 0)
			spaceNum = atoi(spaceNumParam.c_str());
		if (spaceNum < 2 || spaceNum > 20)
			isOptionError(arg, errorInfo);
		else
		{
			formatter.setSpaceIndentation(spaceNum);
		}
	}
	else if ( isOption(arg, "indent=spaces") )
	{
		formatter.setSpaceIndentation(4);
	}
	else if ( isParamOption(arg, "m", "min-conditional-indent=") )
	{
		int minIndent = MINCOND_TWO;
		string minIndentParam = getParam(arg, "m", "min-conditional-indent=");
		if (minIndentParam.length() > 0)
			minIndent = atoi(minIndentParam.c_str());
		if (minIndent >= MINCOND_END)
			isOptionError(arg, errorInfo);
		else
			formatter.setMinConditionalIndentOption(minIndent);
	}
	else if ( isParamOption(arg, "M", "max-instatement-indent=") )
	{
		int maxIndent = 40;
		string maxIndentParam = getParam(arg, "M", "max-instatement-indent=");
		if (maxIndentParam.length() > 0)
			maxIndent = atoi(maxIndentParam.c_str());
		if (maxIndent < 40)
			isOptionError(arg, errorInfo);
		else if (maxIndent > 120)
			isOptionError(arg, errorInfo);
		else
			formatter.setMaxInStatementIndentLength(maxIndent);
	}
	else if ( isOption(arg, "N", "indent-namespaces") )
	{
		formatter.setNamespaceIndent(true);
	}
	else if ( isOption(arg, "C", "indent-classes") )
	{
		formatter.setClassIndent(true);
	}
	else if ( isOption(arg, "xG", "indent-modifiers") )
	{
		formatter.setModifierIndent(true);
	}
	else if ( isOption(arg, "S", "indent-switches") )
	{
		formatter.setSwitchIndent(true);
	}
	else if ( isOption(arg, "K", "indent-cases") )
	{
		formatter.setCaseIndent(true);
	}
	else if ( isOption(arg, "L", "indent-labels") )
	{
		formatter.setLabelIndent(true);
	}
	else if ( isOption(arg, "w", "indent-preproc-define") )
	{
		formatter.setPreprocDefineIndent(true);
	}
	else if ( isOption(arg, "xw", "indent-preproc-cond") )
	{
		formatter.setPreprocConditionalIndent(true);
	}
	else if ( isOption(arg, "y", "break-closing-brackets") )
	{
		formatter.setBreakClosingHeaderBracketsMode(true);
	}
	else if ( isOption(arg, "O", "keep-one-line-blocks") )
	{
		formatter.setBreakOneLineBlocksMode(false);
	}
	else if ( isOption(arg, "o", "keep-one-line-statements") )
	{
		formatter.setSingleStatementsMode(false);
	}
	else if ( isOption(arg, "P", "pad-paren") )
	{
		formatter.setParensOutsidePaddingMode(true);
		formatter.setParensInsidePaddingMode(true);
	}
	else if ( isOption(arg, "d", "pad-paren-out") )
	{
		formatter.setParensOutsidePaddingMode(true);
	}
	else if ( isOption(arg, "xd", "pad-first-paren-out") )
	{
		formatter.setParensFirstPaddingMode(true);
	}
	else if ( isOption(arg, "D", "pad-paren-in") )
	{
		formatter.setParensInsidePaddingMode(true);
	}
	else if ( isOption(arg, "H", "pad-header") )
	{
		formatter.setParensHeaderPaddingMode(true);
	}
	else if ( isOption(arg, "U", "unpad-paren") )
	{
		formatter.setParensUnPaddingMode(true);
	}
	else if ( isOption(arg, "p", "pad-oper") )
	{
		formatter.setOperatorPaddingMode(true);
	}
	else if ( isOption(arg, "xe", "delete-empty-lines") )
	{
		formatter.setDeleteEmptyLinesMode(true);
	}
	else if ( isOption(arg, "E", "fill-empty-lines") )
	{
		formatter.setEmptyLineFill(true);
	}
	else if ( isOption(arg, "c", "convert-tabs") )
	{
		formatter.setTabSpaceConversionMode(true);
	}
	else if ( isOption(arg, "xy", "close-templates") )
	{
		formatter.setCloseTemplatesMode(true);
	}
	else if ( isOption(arg, "F", "break-blocks=all") )
	{
		formatter.setBreakBlocksMode(true);
		formatter.setBreakClosingHeaderBlocksMode(true);
	}
	else if ( isOption(arg, "f", "break-blocks") )
	{
		formatter.setBreakBlocksMode(true);
	}
	else if ( isOption(arg, "e", "break-elseifs") )
	{
		formatter.setBreakElseIfsMode(true);
	}
	else if ( isOption(arg, "j", "add-brackets") )
	{
		formatter.setAddBracketsMode(true);
	}
	else if ( isOption(arg, "J", "add-one-line-brackets") )
	{
		formatter.setAddOneLineBracketsMode(true);
	}
	else if ( isOption(arg, "xj", "remove-brackets") )
	{
		formatter.setRemoveBracketsMode(true);
	}
	else if ( isOption(arg, "Y", "indent-col1-comments") )
	{
		formatter.setIndentCol1CommentsMode(true);
	}
	else if ( isOption(arg, "align-pointer=type") )
	{
		formatter.setPointerAlignment(PTR_ALIGN_TYPE);
	}
	else if ( isOption(arg, "align-pointer=middle") )
	{
		formatter.setPointerAlignment(PTR_ALIGN_MIDDLE);
	}
	else if ( isOption(arg, "align-pointer=name") )
	{
		formatter.setPointerAlignment(PTR_ALIGN_NAME);
	}
	else if ( isParamOption(arg, "k") )
	{
		int align = 0;
		string styleParam = getParam(arg, "k");
		if (styleParam.length() > 0)
			align = atoi(styleParam.c_str());
		if (align < 1 || align > 3)
			isOptionError(arg, errorInfo);
		else if (align == 1)
			formatter.setPointerAlignment(PTR_ALIGN_TYPE);
		else if (align == 2)
			formatter.setPointerAlignment(PTR_ALIGN_MIDDLE);
		else if (align == 3)
			formatter.setPointerAlignment(PTR_ALIGN_NAME);
	}
	else if ( isOption(arg, "align-reference=none") )
	{
		formatter.setReferenceAlignment(REF_ALIGN_NONE);
	}
	else if ( isOption(arg, "align-reference=type") )
	{
		formatter.setReferenceAlignment(REF_ALIGN_TYPE);
	}
	else if ( isOption(arg, "align-reference=middle") )
	{
		formatter.setReferenceAlignment(REF_ALIGN_MIDDLE);
	}
	else if ( isOption(arg, "align-reference=name") )
	{
		formatter.setReferenceAlignment(REF_ALIGN_NAME);
	}
	else if ( isParamOption(arg, "W") )
	{
		int align = 0;
		string styleParam = getParam(arg, "W");
		if (styleParam.length() > 0)
			align = atoi(styleParam.c_str());
		if (align < 0 || align > 3)
			isOptionError(arg, errorInfo);
		else if (align == 0)
			formatter.setReferenceAlignment(REF_ALIGN_NONE);
		else if (align == 1)
			formatter.setReferenceAlignment(REF_ALIGN_TYPE);
		else if (align == 2)
			formatter.setReferenceAlignment(REF_ALIGN_MIDDLE);
		else if (align == 3)
			formatter.setReferenceAlignment(REF_ALIGN_NAME);
	}
	else if ( isParamOption(arg, "max-code-length=") )
	{
		int maxLength = 50;
		string maxLengthParam = getParam(arg, "max-code-length=");
		if (maxLengthParam.length() > 0)
			maxLength = atoi(maxLengthParam.c_str());
		if (maxLength < 50)
			isOptionError(arg, errorInfo);
		else if (maxLength > 200)
			isOptionError(arg, errorInfo);
		else
			formatter.setMaxCodeLength(maxLength);
	}
	else if ( isParamOption(arg, "xC") )
	{
		int maxLength = 50;
		string maxLengthParam = getParam(arg, "xC");
		if (maxLengthParam.length() > 0)
			maxLength = atoi(maxLengthParam.c_str());
		if (maxLength > 200)
			isOptionError(arg, errorInfo);
		else
			formatter.setMaxCodeLength(maxLength);
	}
	else if ( isOption(arg, "xL", "break-after-logical") )
	{
		formatter.setBreakAfterMode(true);
	}
	else if ( isOption(arg, "xc", "attach-classes") )
	{
		formatter.setAttachClass(true);
	}
	else if ( isOption(arg, "xk", "attach-extern-c") )
	{
		formatter.setAttachExternC(true);
	}
	else if ( isOption(arg, "xn", "attach-namespaces") )
	{
		formatter.setAttachNamespace(true);
	}
	else if ( isOption(arg, "xl", "attach-inlines") )
	{
		formatter.setAttachInline(true);
	}
	else if ( isOption(arg, "xp", "remove-comment-prefix") )
	{
		formatter.setStripCommentPrefix(true);
	}
	// Objective-C options
	else if ( isOption(arg, "xM", "align-method-colon") )
	{
		formatter.setAlignMethodColon(true);
	}
	else if ( isOption(arg, "xQ", "pad-method-prefix") )
	{
		formatter.setMethodPrefixPaddingMode(true);
	}
	else if ( isOption(arg, "xR", "unpad-method-prefix") )
	{
		formatter.setMethodPrefixUnPaddingMode(true);
	}
	else if ( isOption(arg, "xP0", "pad-method-colon=none") )
	{
		formatter.setObjCColonPaddingMode(COLON_PAD_NONE);
	}
	else if ( isOption(arg, "xP1", "pad-method-colon=all") )
	{
		formatter.setObjCColonPaddingMode(COLON_PAD_ALL);
	}
	else if ( isOption(arg, "xP2", "pad-method-colon=after") )
	{
		formatter.setObjCColonPaddingMode(COLON_PAD_AFTER);
	}
	else if ( isOption(arg, "xP3", "pad-method-colon=before") )
	{
		formatter.setObjCColonPaddingMode(COLON_PAD_BEFORE);
	}
	else if ( isOption(arg, "indent-preprocessor") )	// depreciated release 2.04
	{
		formatter.setPreprocDefineIndent(true);
	}
	else if ( isOption(arg, "n", "suffix=none") )
	{
		g_console->setNoBackup(true);
	}
	else if ( isParamOption(arg, "suffix=") )
	{
		string suffixParam = getParam(arg, "suffix=");
		if (suffixParam.length() > 0)
		{
			g_console->setOrigSuffix(suffixParam);
		}
	}
	else if ( isParamOption(arg, "exclude=") )
	{
		string suffixParam = getParam(arg, "exclude=");
		if (suffixParam.length() > 0)
			g_console->updateExcludeVector(suffixParam);
	}
	else if ( isOption(arg, "r", "R") || isOption(arg, "recursive") )
	{
		g_console->setIsRecursive(true);
	}
	else if ( isOption(arg, "Z", "preserve-date") )
	{
		g_console->setPreserveDate(true);
	}
	else if ( isOption(arg, "v", "verbose") )
	{
		g_console->setIsVerbose(true);
	}
	else if ( isOption(arg, "Q", "formatted") )
	{
		g_console->setIsFormattedOnly(true);
	}
	else if ( isOption(arg, "q", "quiet") )
	{
		g_console->setIsQuiet(true);
	}
	else if ( isOption(arg, "i", "ignore-exclude-errors") )
	{
		g_console->setIgnoreExcludeErrors(true);
	}
	else if ( isOption(arg, "xi", "ignore-exclude-errors-x") )
	{
		g_console->setIgnoreExcludeErrorsAndDisplay(true);
	}
	else if ( isOption(arg, "X", "errors-to-stdout") )
	{
		_err = &cout;
	}
	else if ( isOption(arg, "lineend=windows") )
	{
		formatter.setLineEndFormat(LINEEND_WINDOWS);
	}
	else if ( isOption(arg, "lineend=linux") )
	{
		formatter.setLineEndFormat(LINEEND_LINUX);
	}
	else if ( isOption(arg, "lineend=macold") )
	{
		formatter.setLineEndFormat(LINEEND_MACOLD);
	}
	else if ( isParamOption(arg, "z") )
	{
		int lineendType = 0;
		string lineendParam = getParam(arg, "z");
		if (lineendParam.length() > 0)
			lineendType = atoi(lineendParam.c_str());
		if (lineendType < 1 || lineendType > 3)
			isOptionError(arg, errorInfo);
		else if (lineendType == 1)
			formatter.setLineEndFormat(LINEEND_WINDOWS);
		else if (lineendType == 2)
			formatter.setLineEndFormat(LINEEND_LINUX);
		else if (lineendType == 3)
			formatter.setLineEndFormat(LINEEND_MACOLD);
	}
	else
		isOptionError(arg, errorInfo);
}	// End of parseOption function

void ASOptions::importOptions(istream &in, vector<string> &optionsVector)
{
	char ch;
	string currentToken;

	while (in)
	{
		currentToken = "";
		do
		{
			in.get(ch);
			if (in.eof())
				break;
			// treat '#' as line comments
			if (ch == '#')
				while (in)
				{
					in.get(ch);
					if (ch == '\n' || ch == '\r')
						break;
				}

			// break options on spaces, tabs, commas, or new-lines
			if (in.eof() || ch == ' ' || ch == '\t' || ch == ',' || ch == '\n' || ch == '\r')
				break;
			else
				currentToken.append(1, ch);

		}
		while (in);

		if (currentToken.length() != 0)
			optionsVector.push_back(currentToken);
	}
}

string ASOptions::getOptionErrors()
{
	return optionErrors.str();
}

string ASOptions::getParam(const string &arg, const char* op)
{
	return arg.substr(strlen(op));
}

string ASOptions::getParam(const string &arg, const char* op1, const char* op2)
{
	return isParamOption(arg, op1) ? getParam(arg, op1) : getParam(arg, op2);
}

bool ASOptions::isOption(const string &arg, const char* op)
{
	return arg.compare(op) == 0;
}

bool ASOptions::isOption(const string &arg, const char* op1, const char* op2)
{
	return (isOption(arg, op1) || isOption(arg, op2));
}

void ASOptions::isOptionError(const string &arg, const string &errorInfo)
{
	if (optionErrors.str().length() == 0)
		optionErrors << errorInfo << endl;   // need main error message
	optionErrors << arg << endl;
}

bool ASOptions::isParamOption(const string &arg, const char* option)
{
	bool retVal = arg.compare(0, strlen(option), option) == 0;
	// if comparing for short option, 2nd char of arg must be numeric
	if (retVal && strlen(option) == 1 && arg.length() > 1)
		if (!isdigit((unsigned char)arg[1]))
			retVal = false;
	return retVal;
}

bool ASOptions::isParamOption(const string &arg, const char* option1, const char* option2)
{
	return isParamOption(arg, option1) || isParamOption(arg, option2);
}

//----------------------------------------------------------------------------

}   // end of TaaRRule namespace

//----------------------------------------------------------------------------

using namespace TaaRRule;

//----------------------------------------------------------------------------
// main function functions for Console build
//----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	// create objects
	TRFormatter formatter;
	g_console = new ASConsole(formatter);

	// process command line and options file
	// build the vectors fileNameVector, optionsVector, and fileOptionsVector
	vector<string> argvOptions;
	argvOptions = g_console->getArgvOptions(argc, argv);
	g_console->processOptions(argvOptions);

	// if no files have been given, use cin for input and cout for output
	if (g_console->fileNameVectorIsEmpty())
	{
		g_console->formatCinToCout();
		return EXIT_SUCCESS;
	}

	// process entries in the fileNameVector
	g_console->processFiles();

	delete g_console;
	return EXIT_SUCCESS;
}
