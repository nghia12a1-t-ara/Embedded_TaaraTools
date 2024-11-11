/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	@Project: TaaR_Coding_Style
 *	@File	: ASLocalizer.h
 *
 *	Created	: 10/17/2024 5:52:14 PM
 *	Author	: Nghia-Taarabt
 *	Link repository: 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#ifndef __ASLOCALIZER_H__
#define __ASLOCALIZER_H__

#include <string>
#include <vector>

using namespace std;

namespace astyle {

#ifndef ASTYLE_LIB

//-----------------------------------------------------------------------------
// ASLocalizer class for console build.
// This class encapsulates all language-dependent settings and is a
// generalization of the C locale concept.
//-----------------------------------------------------------------------------
class Translation;

class ASLocalizer
{
	public:		// functions
		ASLocalizer();
		virtual ~ASLocalizer();
		string getLanguageID() const;
		const Translation* getTranslationClass() const;
#ifdef _WIN32
		void setLanguageFromLCID(size_t lcid);
#endif
		void setLanguageFromName(const char* langID);
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

class ChineseSimplified : public Translation
{
	public:
		ChineseSimplified();
};

class ChineseTraditional : public Translation
{
	public:
		ChineseTraditional();
};

class Dutch : public Translation
{
	public:
		Dutch();
};

class English : public Translation
{
	public:
		English();
};

class Finnish : public Translation
{
	public:
		Finnish();
};

class French : public Translation
{
	public:
		French();
};

class German : public Translation
{
	public:
		German();
};

class Hindi : public Translation
{
	public:
		Hindi();
};

class Italian : public Translation
{
	public:
		Italian();
};

class Japanese : public Translation
{
	public:
		Japanese();
};

class Korean : public Translation
{
	public:
		Korean();
};

class Polish : public Translation
{
	public:
		Polish();
};

class Portuguese : public Translation
{
	public:
		Portuguese();
};

class Russian : public Translation
{
	public:
		Russian();
};

class Spanish : public Translation
{
	public:
		Spanish();
};

class Swedish : public Translation
{
	public:
		Swedish();
};

class Ukrainian : public Translation
{
	public:
		Ukrainian();
};


#endif	//  ASTYLE_LIB

}	// namespace astyle

#endif	//  __ASLOCALIZER_H__
