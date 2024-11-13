/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	@Project: TaaR_Coding_Style
 *	@File	: TRLanguage.cpp
 *
 *	Created	: 10/17/2024 5:52:14 PM
 *	Author	: Nghia-Taarabt
 *	Link repository: https://github.com/nghia12a1-t-ara/Embedded_MyTools/tree/master/TaaR_Coding_Style
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
#include "TRLanguage.h"
#include <windows.h>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <typeinfo>

#ifdef _MSC_VER
	#pragma warning(disable: 4996)		// secure version deprecation warnings
#endif

//----------------------------------------------------------------------------
// TRLanguage class methods.
//----------------------------------------------------------------------------
// Set the Language information.
namespace TaaRRule {
TRLanguage::TRLanguage()
{
	m_localeName = "UNKNOWN";
	m_langID = "en";
	m_lcid = 0;
	m_subLangID.clear();
	m_translation = NULL;

	char* localeName = setlocale(LC_ALL, "");
	if (localeName == NULL)		// use the english (ascii) defaults
	{
		fprintf(stderr, "\n%s\n\n", "Cannot set native locale, reverting to English");
		setTranslationClass();
		return;
	}
	size_t lcid = GetUserDefaultLCID();
	setLanguageFromLCID(lcid);
}

// Delete dynamically allocated memory.
TRLanguage::~TRLanguage()
{
	delete m_translation;
}

struct WinLangCode
{
	size_t winLang;
	char canonicalLang[3];
};

static WinLangCode wlc[] =
{
	{ LANG_ENGLISH,    "en" },
	{ LANG_HINDI,      "hi" },
	{ LANG_KOREAN,     "ko" },
};

// Windows get the language to use from the user locale.
void TRLanguage::setLanguageFromLCID(size_t lcid)
{
	m_lcid = lcid;
	m_langID == "en";		// default to english

	size_t lang = PRIMARYLANGID(LANGIDFROMLCID(m_lcid));
	size_t sublang = SUBLANGID(LANGIDFROMLCID(m_lcid));
	// find language in the wlc table
	size_t count = sizeof(wlc) / sizeof(wlc[0]);
	for (size_t i = 0; i < count; i++ )
	{
		if (wlc[i].winLang == lang)
		{
			m_langID = wlc[i].canonicalLang;
			break;
		}
	}
	setTranslationClass();
}

// Returns the language ID in m_langID.
string TRLanguage::getLanguageID() const
{
	return m_langID;
}

// Returns the name of the translation class in m_translation.  Used for testing.
const Translation* TRLanguage::getTranslationClass() const
{
	assert(m_translation);
	return m_translation;
}

// Call the settext class and return the value.
const char* TRLanguage::settext(const char* textIn) const
{
	assert(m_translation);
	const string stringIn = textIn;
	return m_translation->translate(stringIn).c_str();
}

void TRLanguage::setTranslationClass()
{
	assert(m_langID.length());
	// delete previously set (--ascii option)
	if (m_translation)
	{
		delete m_translation;
		m_translation = NULL;
	}
	if (m_langID == "en")
		m_translation =  new English;
	else if (m_langID == "hi")
		m_translation = new Hindi;
	else if (m_langID == "ko")
		m_translation = new Korean;
	else	// default
		m_translation = new English;
}

//----------------------------------------------------------------------------
// Translation base class methods.
//----------------------------------------------------------------------------

// Add a string pair to the translation vector.
void Translation::addPair(const string &english, const wstring &translated)
{
	pair<string, wstring> entry (english, translated);
	m_translation.push_back(entry);
}

// Convert wchar_t to a multibyte string using the currently assigned locale.
// Return an empty string if an error occurs.
string Translation::convertToMultiByte(const wstring &wideStr) const
{
	static bool msgDisplayed = false;
	// get length of the output excluding the NULL and validate the parameters
	size_t mbLen = wcstombs(NULL, wideStr.c_str(), 0);
	if (mbLen == string::npos)
	{
		if (!msgDisplayed)
		{
			fprintf(stderr, "\n%s\n\n", "Cannot convert to multi-byte string, reverting to English");
			msgDisplayed = true;
		}
		return "";
	}
	// convert the characters
	char* mbStr = new(nothrow) char[mbLen + 1];
	if (mbStr == NULL)
	{
		if (!msgDisplayed)
		{
			fprintf(stderr, "\n%s\n\n", "Bad memory alloc for multi-byte string, reverting to English");
			msgDisplayed = true;
		}
		return "";
	}
	wcstombs(mbStr, wideStr.c_str(), mbLen + 1);
	// return the string
	string mbTranslation = mbStr;
	delete [] mbStr;
	return mbTranslation;
}

// Return the translation vector size.  Used for testing.
size_t Translation::getTranslationVectorSize() const
{
	return m_translation.size();
}

// Get the wide translation string. Used for testing.
bool Translation::getWideTranslation(const string &stringIn, wstring &wideOut) const
{
	for (size_t i = 0; i < m_translation.size(); i++)
	{
		if (m_translation[i].first == stringIn)
		{
			wideOut = m_translation[i].second;
			return true;
		}
	}
	// not found
	wideOut = L"";
	return false;
}

// Translate a string.
// Return a static string instead of a member variable so the method can have a "const" designation.
// This allows "settext" to be called from a "const" method.
string &Translation::translate(const string &stringIn) const
{
	static string mbTranslation;
	mbTranslation.clear();
	for (size_t i = 0; i < m_translation.size(); i++)
	{
		if (m_translation[i].first == stringIn)
		{
			mbTranslation = convertToMultiByte(m_translation[i].second);
			break;
		}
	}
	// not found, return english
	if (mbTranslation.empty())
		mbTranslation = stringIn;
	return mbTranslation;
}

//----------------------------------------------------------------------------
// Translation class methods.
// These classes have only a constructor which builds the language vector.
//----------------------------------------------------------------------------
// this class is NOT translated
English::English()
{}

	// NOTE: Scintilla based editors (CodeBlocks) cannot always edit Hindi.
	//       Use Visual Studio instead.
Hindi::Hindi()	// हिन्दी
{
	addPair("Formatted  %s\n", L"स्वरूपित किया  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"अपरिवर्तित     %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"निर्देशिका  %s\n");
	addPair("Exclude  %s\n", L"निकालना  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"अपवर्जित (बेजोड़)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s स्वरूपित किया   %s अपरिवर्तित   ");
	addPair(" seconds   ", L" सेकंड   ");
	addPair("%d min %d sec   ", L"%d मिनट %d सेकंड   ");
	addPair("%s lines\n", L"%s लाइनों\n");
	addPair("Using default options file %s\n", L"डिफ़ॉल्ट विकल्प का उपयोग कर फ़ाइल %s\n");
	addPair("Invalid option file options:", L"अवैध विकल्प फ़ाइल विकल्प हैं:");
	addPair("Invalid command line options:", L"कमांड लाइन विकल्प अवैध:");
	addPair("For help on options type 'TaaRRule -h'", L"विकल्पों पर मदद के लिए प्रकार 'TaaRRule -h'");
	addPair("Cannot open options file", L"विकल्प फ़ाइल नहीं खोल सकता है");
	addPair("Cannot open directory", L"निर्देशिका नहीं खोल सकता");
	addPair("Missing filename in %s\n", L"लापता में फ़ाइलनाम %s\n");
	addPair("Recursive option with no wildcard", L"कोई वाइल्डकार्ड साथ पुनरावर्ती विकल्प");
	addPair("Did you intend quote the filename", L"क्या आप बोली फ़ाइलनाम का इरादा");
	addPair("No file to process %s\n", L"कोई फ़ाइल %s प्रक्रिया के लिए\n");
	addPair("Did you intend to use --recursive", L"क्या आप उपयोग करना चाहते हैं --recursive");
	addPair("Cannot process UTF-32 encoding", L"UTF-32 कूटबन्धन प्रक्रिया नहीं कर सकते");
	addPair("\nArtistic Style has terminated", L"\nArtistic Style समाप्त किया है");
}

Korean::Korean()	// 한국의
{
	addPair("Formatted  %s\n", L"체재         %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"변하지 않은  %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"디렉토리  %s\n");
	addPair("Exclude  %s\n", L"제외  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"제외 (NO 일치) %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s 체재   %s 변하지 않은   ");
	addPair(" seconds   ", L" 초   ");
	addPair("%d min %d sec   ", L"%d 분 %d 초   ");
	addPair("%s lines\n", L"%s 라인\n");
	addPair("Using default options file %s\n", L"기본 구성 파일을 사용 %s\n");
	addPair("Invalid option file options:", L"잘못된 구성 파일 옵션 :");
	addPair("Invalid command line options:", L"잘못된 명령줄 옵션 :");
	addPair("For help on options type 'TaaRRule -h'", L"옵션 유형 'TaaRRule - H에 대한 도움말을 보려면");
	addPair("Cannot open options file", L"구성 파일을 열 수 없습니다");
	addPair("Cannot open directory", L"디렉토리를 열지 못했습니다");
	addPair("Missing filename in %s\n", L"%s 의에서 누락된 파일 이름\n");
	addPair("Recursive option with no wildcard", L"없이 와일드 카드로 재귀 옵션");
	addPair("Did you intend quote the filename", L"당신은 파일 이름을 인용하고자나요");
	addPair("No file to process %s\n", L"%s 을 (를) 처리하는 데 아무런 파일이 없습니다\n");
	addPair("Did you intend to use --recursive", L"당신이 사용하고자나요 --recursive");
	addPair("Cannot process UTF-32 encoding", L"UTF-32 인코딩을 처리할 수 없습니다");
	addPair("\nArtistic Style has terminated", L"\nArtistic Style 종료가");
}

}   // end of namespace TaaRRule
