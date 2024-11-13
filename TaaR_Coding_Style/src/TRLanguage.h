/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	@Project: TaaR_Coding_Style
 *	@File	: TRLanguage.h
 *
 *	Created	: 10/17/2024 5:52:14 PM
 *	Author	: Nghia-Taarabt
 *	Link repository: https://github.com/nghia12a1-t-ara/Embedded_MyTools/tree/master/TaaR_Coding_Style
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
#ifndef __TRLANGUAGE_H__
#define __TRLANGUAGE_H__

#include <string>
#include <vector>

using namespace std;

//-----------------------------------------------------------------------------
// TRLanguage class for console build.
// This class encapsulates all language-dependent settings and is a
// generalization of the C locale concept.
//-----------------------------------------------------------------------------
namespace TaaRRule {

class Translation;

class TRLanguage
{
	public:		// functions
		TRLanguage();
		virtual ~TRLanguage();
		string getLanguageID() const;
		const Translation* getTranslationClass() const;
		void setLanguageFromLCID(size_t lcid);
		const char* settext(const char* textIn) const;

	private:	// functions
		void setTranslationClass();

	private:	// variables
		Translation* m_translation;		// pointer to a polymorphic Translation class
		string m_langID;				// language identifier from the locale
		string m_subLangID;				// sub language identifier, if needed
		string m_localeName;			// name of the current locale (Linux only)
		size_t m_lcid;					// LCID of the user locale (Windows only)
};

//----------------------------------------------------------------------------
// Translation base class.
//----------------------------------------------------------------------------

// This base class is inherited by the language translation classes.
// Polymorphism is used to call the correct language translator.
// This class contains the translation vector and settext translation method.
class Translation
{
	public:
		Translation() {}
		virtual ~Translation() {}
		string convertToMultiByte(const wstring &wideStr) const;
		size_t getTranslationVectorSize() const;
		bool getWideTranslation(const string &stringIn, wstring &wideOut) const;
		string &translate(const string &stringIn) const;

	protected:
		void addPair(const string &english, const wstring &translated);
		// variables
		vector<pair<string, wstring> > m_translation;		// translation vector
};

//----------------------------------------------------------------------------
// Translation classes
// One class for each language.
// These classes have only a constructor which builds the language vector.
//----------------------------------------------------------------------------

class English : public Translation
{
	public:
		English();
};

class Hindi : public Translation
{
	public:
		Hindi();
};

class Korean : public Translation
{
	public:
		Korean();
};

}	// namespace TaaRRule

#endif	//  __TRLANGUAGE_H__
