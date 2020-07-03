/***************************************************************
*      scanner routine for Mini C language                    *
*                                   2003. 3. 10               *
***************************************************************/

#pragma once


#define NO_KEYWORD 16
#define ID_LENGTH 12
#define STR_LENGTH 2000

struct tokenType {
	int number;
	union {
		char id[ID_LENGTH];
		int num;
		char ch; //문자 리터럴을 넣기 위해 추가
		char str[STR_LENGTH]; //문자열 리터럴을 넣기 위해 추가
		double real; //실수 리터럴을 넣기 위해 추가
	} value;
};


enum tsymbol {
	tnull = -1,
	tnot, tnotequ, tremainder, tremAssign, tident, tnumber,
	/* 0          1            2         3            4          5     */
	tand, tlparen, trparen, tmul, tmulAssign, tplus,
	/* 6          7            8         9           10         11     */
	tinc, taddAssign, tcomma, tminus, tdec, tsubAssign,
	/* 12         13          14        15           16         17     */
	tdiv, tdivAssign, tsemicolon, tless, tlesse, tassign,
	/* 18         19          20        21           22         23     */
	tequal, tgreat, tgreate, tlbracket, trbracket, teof,
	/* 24         25          26        27           28         29     */
	//   ...........    word symbols ................................. //
	/* 30         31          32        33           34         35     */
	tconst, telse, tif, tint, treturn, tvoid,
	/* 36         37          38        39           40         41     */
	twhile, tlbrace, tor, trbrace, tchar, tdouble,
	/* 42		  43          44        45           46         47     */
	tstring, tfor, tswitch, tcase, tdefault, tcontinue,
	/* 48		  49          50        51           52         53     */
	tbreak, tlitchar, tlitstr, tlitreal, tcolon
};//char, double, string, for, switch, case, default, continue, break symbol 추가


struct tokenType scanner();
void printToken(struct tokenType token);
