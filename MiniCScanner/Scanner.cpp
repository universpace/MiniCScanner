/***************************************************************
*      scanner routine for Mini C language                    *
***************************************************************/

/*
- 인식 키워드 추가: char, double, string, for, switch, case, default, continue, break
- 인식 리터럴 추가: 문자, 실수, 문자열
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "Scanner.h"

extern FILE *sourceFile;                       // miniC source program


int superLetter(char ch);
int superLetterOrDigit(char ch);
void getNumber(struct tokenType* token ,char firstCharacter);
int hexValue(char ch);
void lexicalError(int n);


char *tokenName[] = {
	"!",        "!=",      "%",       "%=",     "%ident",   "%number",
	/* 0          1           2         3          4          5        */
	"&&",       "(",       ")",       "*",      "*=",       "+",
	/* 6          7           8         9         10         11        */
	"++",       "+=",      ",",       "-",      "--",	    "-=",
	/* 12         13         14        15         16         17        */
	"/",        "/=",      ";",       "<",      "<=",       "=",
	/* 18         19         20        21         22         23        */
	"==",       ">",       ">=",      "[",      "]",        "eof",
	/* 24         25         26        27         28         29        */
	//   ...........    word symbols ................................. //
	/* 30         31         32        33         34         35        */
	"const",    "else",     "if",      "int",     "return",  "void",
	/* 36         37         38        39         40         41        */
	"while",    "{",        "||",       "}",	  "char",    "double",
	/* 42         43         44        45         46         47        */
	"string",   "for",      "switch",   "case",   "default", "continue",
	/* 48         49         50        51                           */
	"break",	"%char", "%string", "%real", "colon"
};//char, double, string, for, switch, case, default, continue, break , character literal, string literal, real number 추가

char *keyword[NO_KEYWORD] = {
	"const", "else", "if", "int", "return", "void", "while", "char", "double", "string", "for", "switch", "case", "default", "continue", "break"
};//char, double, string, for, switch, case, default, continue, break 추가

enum tsymbol tnum[NO_KEYWORD] = {
	tconst, telse, tif, tint, treturn, tvoid, twhile, tchar, tdouble, tstring, tfor, tswitch, tcase, tdefault, tcontinue, tbreak
};//char, double, string, for, switch, case, default, continue, break에 해당하는 tsymbol 추가

struct tokenType scanner()
{
	struct tokenType token;
	int i, index;
	char ch, id[ID_LENGTH];

	token.number = tnull;

	do {
		while (isspace(ch = fgetc(sourceFile)));	// state 1: skip blanks
		if (superLetter(ch)) { // identifier or keyword
			i = 0;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(sourceFile);
			} while (superLetterOrDigit(ch));
			if (i >= ID_LENGTH) lexicalError(1);
			id[i] = '\0';
			ungetc(ch, sourceFile);  //  retract
									 // find the identifier in the keyword table
			for (index = 0; index < NO_KEYWORD; index++)
				if (!strcmp(id, keyword[index])) break;
			if (index < NO_KEYWORD)    // found, keyword exit
				token.number = tnum[index];
			else {                     // not found, identifier exit
				token.number = tident;
				strcpy_s(token.value.id, id);
			}
		}  // end of identifier or keyword
		else if (isdigit(ch)) {  // number
			getNumber(&token, ch);
		}
		else switch (ch) {  // special character
		case '/':
			ch = fgetc(sourceFile);
			if (ch == '*') {
				ch = fgetc(sourceFile);
				if (ch == '*') { //document comment
					printf("Documented Comments 출력 : ");
					do {
						while (1) {
							ch = fgetc(sourceFile);
							if (ch == '*')
								break;
							putchar(ch);
						}
						ch = fgetc(sourceFile);
					} while (ch != '/');
					printf("\n");
				}
				else { // text comment
					do {
						while (ch != '*') {
							ch = fgetc(sourceFile);
						}
						ch = fgetc(sourceFile);
					} while (ch != '/');
				}
			}
			else if (ch == '/')		// line comment
				while (fgetc(sourceFile) != '\n');
			else if (ch == '=')  token.number = tdivAssign;
			else {
				token.number = tdiv;
				ungetc(ch, sourceFile); // retract
			}
			break;
		case '!':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tnotequ;
			else {
				token.number = tnot;
				ungetc(ch, sourceFile); // retract
			}
			break;
		case '%':
			ch = fgetc(sourceFile);
			if (ch == '=') {
				token.number = tremAssign;
			}
			else {
				token.number = tremainder;
				ungetc(ch, sourceFile);
			}
			break;
		case '&':
			ch = fgetc(sourceFile);
			if (ch == '&')  token.number = tand;
			else {
				lexicalError(2);
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '*':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tmulAssign;
			else {
				token.number = tmul;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '+':
			ch = fgetc(sourceFile);
			if (ch == '+')  token.number = tinc;
			else if (ch == '=') token.number = taddAssign;
			else {
				token.number = tplus;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '-':
			ch = fgetc(sourceFile);
			if (ch == '-')  token.number = tdec;
			else if (ch == '=') token.number = tsubAssign;
			else {
				token.number = tminus;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '<':
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tlesse;
			else {
				token.number = tless;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '=':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tequal;
			else {
				token.number = tassign;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '>':
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tgreate;
			else {
				token.number = tgreat;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '|':
			ch = fgetc(sourceFile);
			if (ch == '|')  token.number = tor;
			else {
				lexicalError(3);
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '\''://문자 리터럴일 때
			ch = fgetc(sourceFile);//다음 문자를 받는다
			token.number = tlitchar;//token number를 문자 리터럴 symbol로 설정
			token.value.ch = ch;//token value를 받은 문자로 설정
			ch = fgetc(sourceFile);
			if (ch != '\'')//문자 뒤에 ' 이 안오면 오류 출력
				lexicalError(5);
			break;
		case '"'://문자열 리터럴일 때
			i = 0;//문자열 인덱스
			token.number = tlitstr;//token number를 문자열 리터럴 symbol로 설정
			do {
				ch = fgetc(sourceFile);//다음 문자를 받는다
				if (ch == '\\'){// \가 나오면 다음 문자는 escape 문자임.
					ch = fgetc(sourceFile);
					switch (ch) {
					case 't':// \t는 수평 탭이므로 따로 추가해줌
						if (i < STR_LENGTH) token.value.str[i++] = '\t';
						break;
					case '\'':// \'를 작은 따옴표만 인식하게 함.
						if (i < STR_LENGTH) token.value.str[i++] = ch;
						break;
					case '\\': //\\를 \만 인식하게 함.
						if (i < STR_LENGTH) token.value.str[i++] = ch;
						break;
					default:
						break;
					}
					ch = fgetc(sourceFile);
				}
				if (i < STR_LENGTH) token.value.str[i++] = ch;//token value에 존재하는 문자열 배열에 문자를 넣어주고 인덱스 증가
			} while (ch != '"');
			if (i >= STR_LENGTH) 
				lexicalError(6);// 입력 가능한 문자열 길이를 넘어서면 오류 출력
			token.value.str[i - 1] = '\0';//token value 문자열 배열에 종결문자를 넣어줌
			break;
		case '(': token.number = tlparen;         break;
		case ')': token.number = trparen;         break;
		case ',': token.number = tcomma;          break;
		case ';': token.number = tsemicolon;      break;
		case ':': token.number = tcolon;		  break;//case문에 있는 콜론(:)인식 추가
		case '[': token.number = tlbracket;       break;
		case ']': token.number = trbracket;       break;
		case '{': token.number = tlbrace;         break;
		case '}': token.number = trbrace;         break;
		case EOF: token.number = teof;            break;
			ch = fgetc(sourceFile);
		default: {
			printf("Current character : %c", ch);
			lexicalError(4);
			break;
		}

		} // switch end
	} while (token.number == tnull);
	return token;
} // end of scanner

void lexicalError(int n)
{
	printf(" *** Lexical Error : ");
	switch (n) {
	case 1: printf("an identifier length must be less than 12.\n");
		break;
	case 2: printf("next character must be &\n");
		break;
	case 3: printf("next character must be |\n");
		break;
	case 4: printf("invalid character\n");
		break;
	case 5: printf("next character must be ' \n");
		break;
	case 6: printf("String length must be less than 2000\n");
		break;
	}
}

int superLetter(char ch)
{
	if (isalpha(ch) || ch == '_') return 1;
	else return 0;
}

int superLetterOrDigit(char ch)
{
	if (isalnum(ch) || ch == '_') return 1;
	else return 0;
}

void getNumber(struct tokenType *token , char firstCharacter)
{
	int num = 0;//정수형을 저장
	int value;
	double real = 0;//실수형을 저장
	char ch;

	if (firstCharacter == '0') {
		ch = fgetc(sourceFile);
		if ((ch == 'X') || (ch == 'x')) {		// hexa decimal
			while ((value = hexValue(ch = fgetc(sourceFile))) != -1)
				num = 16 * num + value;
		}
		else if ((ch >= '0') && (ch <= '7')) {	// octal
			do {
				num = 8 * num + (int)(ch - '0');
				ch = fgetc(sourceFile);
			} while ((ch >= '0') && (ch <= '7'));
		}
		else if ((ch == '.')) {//실수형 판단(0.xxx 형태)
			int i = 1;
			do {
				ch = fgetc(sourceFile);//다음 문자 받음
				if (!isdigit(ch))//다음 문자가 숫자가 아니면 while문 탈출
					break;
				double temp = (double)(ch - '0');//임시로 생성한 double형 변수
				real = real + temp / pow(10, i);//실수를 저장할 변수에 10의 i제곱을 나누어서 각 자리수별로 더해줌.
				i++;
			} while (1);
			if (ch == 'e') {//0.xxxxe+xxx형태 인식
				ch = fgetc(sourceFile);//다음 문자를 받음
				if (ch == '+') {//다음 문자가 + 일 때
					int tempN=0;//옮길 자리수를 저장할 변수
					do {
						ch = fgetc(sourceFile);//다음 문자를받음
						if (!isdigit(ch)) 
							break;
						
						tempN = 10 * tempN + (int)(ch - '0');//정수 생성
					} while (1);
					for (int i = 0; i < tempN; i++)//생성한 정수만큼 반복하여 자리수 증가
						real *= 10;
				}
				else if(ch=='-'){//0.xxxe-xxx형태 인식
					int tempN = 0;//옮길 자리수를 저장할 변수
					do {
						ch = fgetc(sourceFile);//다음 문자 받음
						if (!isdigit(ch)) 
							break;
						
						tempN = 10 * tempN + (int)(ch - '0');//정수 생성
					} while (1);
					for (int i = 0; i < tempN; i++)//생성한 정수만큼 반복하여 자리수 감소
						real /= 10;
				}	
			}
			token->number = tlitreal;//token number을 실수 리터럴 symbol로 지정
			token->value.real = real;//token value를 생성한 실수로 지정
			return;//함수 종료
		}
		else num = 0;						// zero
	}
	else {									// decimal
		ch = firstCharacter;
		real = (double)(ch - '0');
		do {
			num = 10 * num + (int)(ch - '0');
			ch = fgetc(sourceFile);
		} while (isdigit(ch));
		if ((ch == '.')) {//실수형 판단.(x.xxx형태)
			int i = 1;
			do {
				ch = fgetc(sourceFile);//다음 문자 받음
				if (!isdigit(ch))//다음 문자가 숫자가 아니면 while문 탈출
					break;
				double temp = (double)(ch - '0');//임시로 생성한 double형 변수
				real = real + temp / pow(10, i);//실수를 저장할 변수에 10의 i제곱을 나누어서 각 자리수별로 더해줌.
				i++;
			} while (1);
			if (ch == 'e') {
				ch = fgetc(sourceFile);
				if (ch == '+') {//다음 문자가 + 일 때
					int tempN = 0;//옮길 자리수를 저장할 변수
					do {
						ch = fgetc(sourceFile);//다음 문자를받음
						if (!isdigit(ch))
							break;
						tempN = 10 * tempN + (int)(ch - '0');//정수 생성
					} while (1);
					for (int i = 0; i < tempN; i++)//생성한 정수만큼 반복하여 자리수 증가
						real *= 10;
				}
				else if (ch == '-') {//0.xxxe-xxx형태 인식
					int tempN = 0;//옮길 자리수를 저장할 변수
					do {
						ch = fgetc(sourceFile);//다음 문자 받음
						if (!isdigit(ch)) 
							break;
						
						tempN = 10 * tempN + (int)(ch - '0');//정수 생성
					} while (1);
					for (int i = 0; i < tempN; i++)//생성한 정수만큼 반복하여 자리수 감소
						real /= 10;
				}
				token->number = tlitreal;//token number에 실수 리터럴 symbol 지정
				token->value.real = real;//token value에 생성한 실수값 지정
				return;//함수종료
			}
		}
	}
	//정수일 때
	token->number = tnumber;//token number에 정수 symbol 지정
	token->value.num = num;//token value에 생성한 정수 지정
	ungetc(ch, sourceFile);  /*  retract  */
}

int hexValue(char ch)
{
	switch (ch) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return (ch - '0');
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return (ch - 'A' + 10);
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return (ch - 'a' + 10);
	default: return -1;
	}
}

void printToken(struct tokenType token)
{
	if (token.number == tident)
		printf("number: %d, value: %s\n\n", token.number, token.value.id);
	else if (token.number == tnumber)//token이 정수일 때
		printf("number: %d(%s), value: %d\n\n", token.number, tokenName[token.number], token.value.num);
	else if (token.number == tlitchar)//token이 문자 리터럴일 때
		printf("number: %d(%s), value: %c\n\n", token.number, tokenName[token.number], token.value.ch);
	else if (token.number == tlitstr)//token이 문자열 리터럴일 때
		printf("number: %d(%s), value: %s\n\n", token.number, tokenName[token.number], token.value.str);
	else if (token.number == tlitreal)//token이 실수 리터럴일 때
		printf("number: %d(%s), value: %.10lf\n\n", token.number, tokenName[token.number], token.value.real);
	else
		printf("number: %d(%s)\n\n", token.number, tokenName[token.number]);

}