/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	@Project: TaaR_Coding_Style
 *	@File	: TRResource.cpp
 *
 *	Created	: 10/17/2024 5:52:14 PM
 *	Author	: Nghia-Taarabt
 *	Link repository: https://github.com/nghia12a1-t-ara/Embedded_MyTools/tree/master/TaaR_Coding_Style
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
#include "TaaRRule.h"
#include <algorithm>

namespace TaaRRule {

const string TRResource::AS_IF = string("if");
const string TRResource::AS_ELSE = string("else");
const string TRResource::AS_FOR = string("for");
const string TRResource::AS_DO = string("do");
const string TRResource::AS_WHILE = string("while");
const string TRResource::AS_SWITCH = string("switch");
const string TRResource::AS_CASE = string("case");
const string TRResource::AS_DEFAULT = string("default");
const string TRResource::AS_CLASS = string("class");
const string TRResource::AS_VOLATILE = string("volatile");
const string TRResource::AS_STRUCT = string("struct");
const string TRResource::AS_UNION = string("union");
const string TRResource::AS_INTERFACE = string("interface");
const string TRResource::AS_NAMESPACE = string("namespace");
const string TRResource::AS_END = string("end");
const string TRResource::AS_SELECTOR = string("selector");
const string TRResource::AS_EXTERN = string("extern");
const string TRResource::AS_ENUM = string("enum");
const string TRResource::AS_PUBLIC = string("public");
const string TRResource::AS_PROTECTED = string("protected");
const string TRResource::AS_PRIVATE = string("private");
const string TRResource::AS_STATIC = string("static");
const string TRResource::AS_SYNCHRONIZED = string("synchronized");
const string TRResource::AS_OPERATOR = string("operator");
const string TRResource::AS_TEMPLATE = string("template");
const string TRResource::AS_TRY = string("try");
const string TRResource::AS_CATCH = string("catch");
const string TRResource::AS_THROW = string("throw");
const string TRResource::AS_FINALLY = string("finally");
const string TRResource::_AS_TRY = string("__try");
const string TRResource::_AS_FINALLY = string("__finally");
const string TRResource::_AS_EXCEPT = string("__except");
const string TRResource::AS_THROWS = string("throws");
const string TRResource::AS_CONST = string("const");
const string TRResource::AS_SEALED = string("sealed");
const string TRResource::AS_OVERRIDE = string("override");
const string TRResource::AS_WHERE = string("where");
const string TRResource::AS_NEW = string("new");

const string TRResource::AS_ASM = string("asm");
const string TRResource::AS__ASM__ = string("__asm__");
const string TRResource::AS_MS_ASM = string("_asm");
const string TRResource::AS_MS__ASM = string("__asm");

const string TRResource::AS_BAR_DEFINE = string("#define");
const string TRResource::AS_BAR_INCLUDE = string("#include");
const string TRResource::AS_BAR_IF = string("#if");
const string TRResource::AS_BAR_EL = string("#el");
const string TRResource::AS_BAR_ENDIF = string("#endif");

const string TRResource::AS_OPEN_BRACKET = string("{");
const string TRResource::AS_CLOSE_BRACKET = string("}");
const string TRResource::AS_OPEN_LINE_COMMENT = string("//");
const string TRResource::AS_OPEN_COMMENT = string("/*");
const string TRResource::AS_CLOSE_COMMENT = string("*/");

const string TRResource::AS_ASSIGN = string("=");
const string TRResource::AS_PLUS_ASSIGN = string("+=");
const string TRResource::AS_MINUS_ASSIGN = string("-=");
const string TRResource::AS_MULT_ASSIGN = string("*=");
const string TRResource::AS_DIV_ASSIGN = string("/=");
const string TRResource::AS_MOD_ASSIGN = string("%=");
const string TRResource::AS_OR_ASSIGN = string("|=");
const string TRResource::AS_AND_ASSIGN = string("&=");
const string TRResource::AS_XOR_ASSIGN = string("^=");
const string TRResource::AS_GR_GR_ASSIGN = string(">>=");
const string TRResource::AS_LS_LS_ASSIGN = string("<<=");
const string TRResource::AS_GR_GR_GR_ASSIGN = string(">>>=");
const string TRResource::AS_LS_LS_LS_ASSIGN = string("<<<=");
const string TRResource::AS_GCC_MIN_ASSIGN = string("<?");
const string TRResource::AS_GCC_MAX_ASSIGN = string(">?");

const string TRResource::AS_RETURN = string("return");
const string TRResource::AS_CIN = string("cin");
const string TRResource::AS_COUT = string("cout");
const string TRResource::AS_CERR = string("cerr");

const string TRResource::AS_EQUAL = string("==");
const string TRResource::AS_PLUS_PLUS = string("++");
const string TRResource::AS_MINUS_MINUS = string("--");
const string TRResource::AS_NOT_EQUAL = string("!=");
const string TRResource::AS_GR_EQUAL = string(">=");
const string TRResource::AS_GR_GR = string(">>");
const string TRResource::AS_GR_GR_GR = string(">>>");
const string TRResource::AS_LS_EQUAL = string("<=");
const string TRResource::AS_LS_LS = string("<<");
const string TRResource::AS_LS_LS_LS = string("<<<");
const string TRResource::AS_QUESTION_QUESTION = string("??");
const string TRResource::AS_LAMBDA = string("=>");            // C# lambda expression arrow
const string TRResource::AS_ARROW = string("->");
const string TRResource::AS_AND = string("&&");
const string TRResource::AS_OR = string("||");
const string TRResource::AS_SCOPE_RESOLUTION = string("::");

const string TRResource::AS_PLUS = string("+");
const string TRResource::AS_MINUS = string("-");
const string TRResource::AS_MULT = string("*");
const string TRResource::AS_DIV = string("/");
const string TRResource::AS_MOD = string("%");
const string TRResource::AS_GR = string(">");
const string TRResource::AS_LS = string("<");
const string TRResource::AS_NOT = string("!");
const string TRResource::AS_BIT_OR = string("|");
const string TRResource::AS_BIT_AND = string("&");
const string TRResource::AS_BIT_NOT = string("~");
const string TRResource::AS_BIT_XOR = string("^");
const string TRResource::AS_QUESTION = string("?");
const string TRResource::AS_COLON = string(":");
const string TRResource::AS_COMMA = string(",");
const string TRResource::AS_SEMICOLON = string(";");

const string TRResource::AS_FOREACH = string("foreach");
const string TRResource::AS_LOCK = string("lock");
const string TRResource::AS_UNSAFE = string("unsafe");
const string TRResource::AS_FIXED = string("fixed");
const string TRResource::AS_GET = string("get");
const string TRResource::AS_SET = string("set");
const string TRResource::AS_ADD = string("add");
const string TRResource::AS_REMOVE = string("remove");
const string TRResource::AS_DELEGATE = string("delegate");
const string TRResource::AS_UNCHECKED = string("unchecked");

const string TRResource::AS_CONST_CAST = string("const_cast");
const string TRResource::AS_DYNAMIC_CAST = string("dynamic_cast");
const string TRResource::AS_REINTERPRET_CAST = string("reinterpret_cast");
const string TRResource::AS_STATIC_CAST = string("static_cast");

const string TRResource::AS_NS_DURING = string("NS_DURING");
const string TRResource::AS_NS_HANDLER = string("NS_HANDLER");

/**
 * Sort comparison function.
 * Compares the length of the value of pointers in the vectors.
 * The LONGEST strings will be first in the vector.
 *
 * @params the string pointers to be compared.
 */
bool sortOnLength(const string* a, const string* b)
{
	return (*a).length() > (*b).length();
}

/**
 * Sort comparison function.
 * Compares the value of pointers in the vectors.
 *
 * @params the string pointers to be compared.
 */
bool sortOnName(const string* a, const string* b)
{
	return *a < *b;
}

/**
 * Build the vector of assignment operators.
 * Used by BOTH TRFormatter.cpp and ASBeautifier.cpp
 *
 * @param assignmentOperators   a reference to the vector to be built.
 */
void TRResource::buildAssignmentOperators(vector<const string*>* assignmentOperators)
{
	assignmentOperators->push_back(&AS_ASSIGN);
	assignmentOperators->push_back(&AS_PLUS_ASSIGN);
	assignmentOperators->push_back(&AS_MINUS_ASSIGN);
	assignmentOperators->push_back(&AS_MULT_ASSIGN);
	assignmentOperators->push_back(&AS_DIV_ASSIGN);
	assignmentOperators->push_back(&AS_MOD_ASSIGN);
	assignmentOperators->push_back(&AS_OR_ASSIGN);
	assignmentOperators->push_back(&AS_AND_ASSIGN);
	assignmentOperators->push_back(&AS_XOR_ASSIGN);

	// Unknown
	assignmentOperators->push_back(&AS_LS_LS_LS_ASSIGN);

	sort(assignmentOperators->begin(), assignmentOperators->end(), sortOnLength);
}

/**
 * Build the vector of C++ cast operators.
 * Used by ONLY TRFormatter.cpp
 *
 * @param castOperators     a reference to the vector to be built.
 */
void TRResource::buildCastOperators(vector<const string*>* castOperators)
{
	castOperators->push_back(&AS_CONST_CAST);
	castOperators->push_back(&AS_DYNAMIC_CAST);
	castOperators->push_back(&AS_REINTERPRET_CAST);
	castOperators->push_back(&AS_STATIC_CAST);
}

/**
 * Build the vector of header words.
 * Used by BOTH TRFormatter.cpp and ASBeautifier.cpp
 *
 * @param headers       a reference to the vector to be built.
 */
void TRResource::buildHeaders(vector<const string*>* headers, bool beautifier)
{
	headers->push_back(&AS_IF);
	headers->push_back(&AS_ELSE);
	headers->push_back(&AS_FOR);
	headers->push_back(&AS_WHILE);
	headers->push_back(&AS_DO);
	headers->push_back(&AS_SWITCH);
	headers->push_back(&AS_CASE);
	headers->push_back(&AS_DEFAULT);
	headers->push_back(&AS_TRY);
	headers->push_back(&AS_CATCH);

	headers->push_back(&_AS_TRY);		// __try
	headers->push_back(&_AS_FINALLY);	// __finally
	headers->push_back(&_AS_EXCEPT);	// __except

	if (beautifier)
	{
		headers->push_back(&AS_TEMPLATE);
	}
	sort(headers->begin(), headers->end(), sortOnName);
}

/**
 * Build the vector of indentable headers.
 * Used by ONLY ASBeautifier.cpp
 *
 * @param indentableHeaders     a reference to the vector to be built.
 */
void TRResource::buildIndentableHeaders(vector<const string*>* indentableHeaders)
{
	indentableHeaders->push_back(&AS_RETURN);

	sort(indentableHeaders->begin(), indentableHeaders->end(), sortOnName);
}

/**
 * Build the vector of non-assignment operators.
 * Used by ONLY ASBeautifier.cpp
 *
 * @param nonAssignmentOperators       a reference to the vector to be built.
 */
void TRResource::buildNonAssignmentOperators(vector<const string*>* nonAssignmentOperators)
{
	nonAssignmentOperators->push_back(&AS_EQUAL);
	nonAssignmentOperators->push_back(&AS_PLUS_PLUS);
	nonAssignmentOperators->push_back(&AS_MINUS_MINUS);
	nonAssignmentOperators->push_back(&AS_NOT_EQUAL);
	nonAssignmentOperators->push_back(&AS_GR_EQUAL);
	nonAssignmentOperators->push_back(&AS_GR_GR_GR);
	nonAssignmentOperators->push_back(&AS_GR_GR);
	nonAssignmentOperators->push_back(&AS_LS_EQUAL);
	nonAssignmentOperators->push_back(&AS_LS_LS_LS);
	nonAssignmentOperators->push_back(&AS_LS_LS);
	nonAssignmentOperators->push_back(&AS_ARROW);
	nonAssignmentOperators->push_back(&AS_AND);
	nonAssignmentOperators->push_back(&AS_OR);
	nonAssignmentOperators->push_back(&AS_LAMBDA);

	sort(nonAssignmentOperators->begin(), nonAssignmentOperators->end(), sortOnLength);
}

/**
 * Build the vector of header non-paren headers.
 * Used by BOTH TRFormatter.cpp and ASBeautifier.cpp.
 * NOTE: Non-paren headers should also be included in the headers vector.
 *
 * @param nonParenHeaders       a reference to the vector to be built.
 */
void TRResource::buildNonParenHeaders(vector<const string*>* nonParenHeaders, bool beautifier)
{
	nonParenHeaders->push_back(&AS_ELSE);
	nonParenHeaders->push_back(&AS_DO);
	nonParenHeaders->push_back(&AS_TRY);
	nonParenHeaders->push_back(&AS_CATCH);		// can be paren or non-paren
	nonParenHeaders->push_back(&AS_CASE);		// can be paren or non-paren
	nonParenHeaders->push_back(&AS_DEFAULT);
	nonParenHeaders->push_back(&_AS_TRY);		// __try
	nonParenHeaders->push_back(&_AS_FINALLY);	// __finally

	if (beautifier)
	{
		nonParenHeaders->push_back(&AS_TEMPLATE);
	}
	sort(nonParenHeaders->begin(), nonParenHeaders->end(), sortOnName);
}

/**
 * Build the vector of operators.
 * Used by ONLY TRFormatter.cpp
 *
 * @param operators             a reference to the vector to be built.
 */
void TRResource::buildOperators(vector<const string*>* operators)
{
	operators->push_back(&AS_PLUS_ASSIGN);
	operators->push_back(&AS_MINUS_ASSIGN);
	operators->push_back(&AS_MULT_ASSIGN);
	operators->push_back(&AS_DIV_ASSIGN);
	operators->push_back(&AS_MOD_ASSIGN);
	operators->push_back(&AS_OR_ASSIGN);
	operators->push_back(&AS_AND_ASSIGN);
	operators->push_back(&AS_XOR_ASSIGN);
	operators->push_back(&AS_EQUAL);
	operators->push_back(&AS_PLUS_PLUS);
	operators->push_back(&AS_MINUS_MINUS);
	operators->push_back(&AS_NOT_EQUAL);
	operators->push_back(&AS_GR_EQUAL);
	operators->push_back(&AS_GR_GR_GR_ASSIGN);
	operators->push_back(&AS_GR_GR_ASSIGN);
	operators->push_back(&AS_GR_GR_GR);
	operators->push_back(&AS_GR_GR);
	operators->push_back(&AS_LS_EQUAL);
	operators->push_back(&AS_LS_LS_LS_ASSIGN);
	operators->push_back(&AS_LS_LS_ASSIGN);
	operators->push_back(&AS_LS_LS_LS);
	operators->push_back(&AS_LS_LS);
	operators->push_back(&AS_QUESTION_QUESTION);
	operators->push_back(&AS_LAMBDA);
	operators->push_back(&AS_ARROW);
	operators->push_back(&AS_AND);
	operators->push_back(&AS_OR);
	operators->push_back(&AS_SCOPE_RESOLUTION);
	operators->push_back(&AS_PLUS);
	operators->push_back(&AS_MINUS);
	operators->push_back(&AS_MULT);
	operators->push_back(&AS_DIV);
	operators->push_back(&AS_MOD);
	operators->push_back(&AS_QUESTION);
	operators->push_back(&AS_COLON);
	operators->push_back(&AS_ASSIGN);
	operators->push_back(&AS_LS);
	operators->push_back(&AS_GR);
	operators->push_back(&AS_NOT);
	operators->push_back(&AS_BIT_OR);
	operators->push_back(&AS_BIT_AND);
	operators->push_back(&AS_BIT_NOT);
	operators->push_back(&AS_BIT_XOR);
	operators->push_back(&AS_GCC_MIN_ASSIGN);
	operators->push_back(&AS_GCC_MAX_ASSIGN);
	
	sort(operators->begin(), operators->end(), sortOnLength);
}

/**
 * Build the vector of pre-block statements.
 * Used by ONLY ASBeautifier.cpp
 * NOTE: Cannot be both a header and a preBlockStatement.
 *
 * @param preBlockStatements        a reference to the vector to be built.
 */
void TRResource::buildPreBlockStatements(vector<const string*>* preBlockStatements)
{
	preBlockStatements->push_back(&AS_CLASS);
	preBlockStatements->push_back(&AS_STRUCT);
	preBlockStatements->push_back(&AS_UNION);
	preBlockStatements->push_back(&AS_NAMESPACE);
		
	sort(preBlockStatements->begin(), preBlockStatements->end(), sortOnName);
}

/**
 * Build the vector of pre-command headers.
 * Used by BOTH TRFormatter.cpp and ASBeautifier.cpp.
 * NOTE: Cannot be both a header and a preCommandHeader.
 *
 * A preCommandHeader is in a function definition between
 * the closing paren and the opening bracket.
 * e.g. in "void foo() const {}", "const" is a preCommandHeader.
 */
void TRResource::buildPreCommandHeaders(vector<const string*>* preCommandHeaders)
{
	preCommandHeaders->push_back(&AS_CONST);
	preCommandHeaders->push_back(&AS_VOLATILE);
	preCommandHeaders->push_back(&AS_SEALED);		// Visual C only
	preCommandHeaders->push_back(&AS_OVERRIDE);		// Visual C only

	sort(preCommandHeaders->begin(), preCommandHeaders->end(), sortOnName);
}

/**
 * Build the vector of pre-definition headers.
 * Used by ONLY TRFormatter.cpp
 * NOTE: Do NOT add 'enum' here. It is an array type bracket.
 * NOTE: Do NOT add 'extern' here. Do not want an extra indent.
 *
 * @param preDefinitionHeaders      a reference to the vector to be built.
 */
void TRResource::buildPreDefinitionHeaders(vector<const string*>* preDefinitionHeaders)
{
	preDefinitionHeaders->push_back(&AS_CLASS);
	preDefinitionHeaders->push_back(&AS_STRUCT);
	preDefinitionHeaders->push_back(&AS_UNION);
	preDefinitionHeaders->push_back(&AS_NAMESPACE);
	
	sort(preDefinitionHeaders->begin(), preDefinitionHeaders->end(), sortOnName);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                             TRBase Functions
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// check if a specific line position contains a keyword.
bool TRBase::findKeyword(const string &line, int i, const string &keyword) const
{
	assert(isCharPotentialHeader(line, i));
	// check the word
	const size_t keywordLength = keyword.length();
	const size_t wordEnd = i + keywordLength;
	if (wordEnd > line.length())
		return false;
	if (line.compare(i, keywordLength, keyword) != 0)
		return false;
	// check that this is not part of a longer word
	if (wordEnd == line.length())
		return true;
	if (isLegalNameChar(line[wordEnd]))
		return false;
	// is not a keyword if part of a definition
	const char peekChar = peekNextChar(line, wordEnd - 1);
	if (peekChar == ',' || peekChar == ')')
		return false;
	return true;
}

// get the current word on a line
// index must point to the beginning of the word
string TRBase::getCurrentWord(const string &line, size_t index) const
{
	assert(isCharPotentialHeader(line, index));
	size_t lineLength = line.length();
	size_t i;
	for (i = index; i < lineLength; i++)
	{
		if (!isLegalNameChar(line[i]))
			break;
	}
	return line.substr(index, i - index);
}

}   // end namespace TaaRRule
