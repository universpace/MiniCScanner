/***************************************************************
*      scanner routine for Mini C language                    *
***************************************************************/

/*
- �ν� Ű���� �߰�: char, double, string, for, switch, case, default, continue, break
- �ν� ���ͷ� �߰�: ����, �Ǽ�, ���ڿ�
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
};//char, double, string, for, switch, case, default, continue, break , character literal, string literal, real number �߰�

char *keyword[NO_KEYWORD] = {
	"const", "else", "if", "int", "return", "void", "while", "char", "double", "string", "for", "switch", "case", "default", "continue", "break"
};//char, double, string, for, switch, case, default, continue, break �߰�

enum tsymbol tnum[NO_KEYWORD] = {
	tconst, telse, tif, tint, treturn, tvoid, twhile, tchar, tdouble, tstring, tfor, tswitch, tcase, tdefault, tcontinue, tbreak
};//char, double, string, for, switch, case, default, continue, break�� �ش��ϴ� tsymbol �߰�

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
					printf("Documented Comments ��� : ");
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
		case '\''://���� ���ͷ��� ��
			ch = fgetc(sourceFile);//���� ���ڸ� �޴´�
			token.number = tlitchar;//token number�� ���� ���ͷ� symbol�� ����
			token.value.ch = ch;//token value�� ���� ���ڷ� ����
			ch = fgetc(sourceFile);
			if (ch != '\'')//���� �ڿ� ' �� �ȿ��� ���� ���
				lexicalError(5);
			break;
		case '"'://���ڿ� ���ͷ��� ��
			i = 0;//���ڿ� �ε���
			token.number = tlitstr;//token number�� ���ڿ� ���ͷ� symbol�� ����
			do {
				ch = fgetc(sourceFile);//���� ���ڸ� �޴´�
				if (ch == '\\'){// \�� ������ ���� ���ڴ� escape ������.
					ch = fgetc(sourceFile);
					switch (ch) {
					case 't':// \t�� ���� ���̹Ƿ� ���� �߰�����
						if (i < STR_LENGTH) token.value.str[i++] = '\t';
						break;
					case '\'':// \'�� ���� ����ǥ�� �ν��ϰ� ��.
						if (i < STR_LENGTH) token.value.str[i++] = ch;
						break;
					case '\\': //\\�� \�� �ν��ϰ� ��.
						if (i < STR_LENGTH) token.value.str[i++] = ch;
						break;
					default:
						break;
					}
					ch = fgetc(sourceFile);
				}
				if (i < STR_LENGTH) token.value.str[i++] = ch;//token value�� �����ϴ� ���ڿ� �迭�� ���ڸ� �־��ְ� �ε��� ����
			} while (ch != '"');
			if (i >= STR_LENGTH) 
				lexicalError(6);// �Է� ������ ���ڿ� ���̸� �Ѿ�� ���� ���
			token.value.str[i - 1] = '\0';//token value ���ڿ� �迭�� ���Ṯ�ڸ� �־���
			break;
		case '(': token.number = tlparen;         break;
		case ')': token.number = trparen;         break;
		case ',': token.number = tcomma;          break;
		case ';': token.number = tsemicolon;      break;
		case ':': token.number = tcolon;		  break;//case���� �ִ� �ݷ�(:)�ν� �߰�
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
	int num = 0;//�������� ����
	int value;
	double real = 0;//�Ǽ����� ����
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
		else if ((ch == '.')) {//�Ǽ��� �Ǵ�(0.xxx ����)
			int i = 1;
			do {
				ch = fgetc(sourceFile);//���� ���� ����
				if (!isdigit(ch))//���� ���ڰ� ���ڰ� �ƴϸ� while�� Ż��
					break;
				double temp = (double)(ch - '0');//�ӽ÷� ������ double�� ����
				real = real + temp / pow(10, i);//�Ǽ��� ������ ������ 10�� i������ ����� �� �ڸ������� ������.
				i++;
			} while (1);
			if (ch == 'e') {//0.xxxxe+xxx���� �ν�
				ch = fgetc(sourceFile);//���� ���ڸ� ����
				if (ch == '+') {//���� ���ڰ� + �� ��
					int tempN=0;//�ű� �ڸ����� ������ ����
					do {
						ch = fgetc(sourceFile);//���� ���ڸ�����
						if (!isdigit(ch)) 
							break;
						
						tempN = 10 * tempN + (int)(ch - '0');//���� ����
					} while (1);
					for (int i = 0; i < tempN; i++)//������ ������ŭ �ݺ��Ͽ� �ڸ��� ����
						real *= 10;
				}
				else if(ch=='-'){//0.xxxe-xxx���� �ν�
					int tempN = 0;//�ű� �ڸ����� ������ ����
					do {
						ch = fgetc(sourceFile);//���� ���� ����
						if (!isdigit(ch)) 
							break;
						
						tempN = 10 * tempN + (int)(ch - '0');//���� ����
					} while (1);
					for (int i = 0; i < tempN; i++)//������ ������ŭ �ݺ��Ͽ� �ڸ��� ����
						real /= 10;
				}	
			}
			token->number = tlitreal;//token number�� �Ǽ� ���ͷ� symbol�� ����
			token->value.real = real;//token value�� ������ �Ǽ��� ����
			return;//�Լ� ����
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
		if ((ch == '.')) {//�Ǽ��� �Ǵ�.(x.xxx����)
			int i = 1;
			do {
				ch = fgetc(sourceFile);//���� ���� ����
				if (!isdigit(ch))//���� ���ڰ� ���ڰ� �ƴϸ� while�� Ż��
					break;
				double temp = (double)(ch - '0');//�ӽ÷� ������ double�� ����
				real = real + temp / pow(10, i);//�Ǽ��� ������ ������ 10�� i������ ����� �� �ڸ������� ������.
				i++;
			} while (1);
			if (ch == 'e') {
				ch = fgetc(sourceFile);
				if (ch == '+') {//���� ���ڰ� + �� ��
					int tempN = 0;//�ű� �ڸ����� ������ ����
					do {
						ch = fgetc(sourceFile);//���� ���ڸ�����
						if (!isdigit(ch))
							break;
						tempN = 10 * tempN + (int)(ch - '0');//���� ����
					} while (1);
					for (int i = 0; i < tempN; i++)//������ ������ŭ �ݺ��Ͽ� �ڸ��� ����
						real *= 10;
				}
				else if (ch == '-') {//0.xxxe-xxx���� �ν�
					int tempN = 0;//�ű� �ڸ����� ������ ����
					do {
						ch = fgetc(sourceFile);//���� ���� ����
						if (!isdigit(ch)) 
							break;
						
						tempN = 10 * tempN + (int)(ch - '0');//���� ����
					} while (1);
					for (int i = 0; i < tempN; i++)//������ ������ŭ �ݺ��Ͽ� �ڸ��� ����
						real /= 10;
				}
				token->number = tlitreal;//token number�� �Ǽ� ���ͷ� symbol ����
				token->value.real = real;//token value�� ������ �Ǽ��� ����
				return;//�Լ�����
			}
		}
	}
	//������ ��
	token->number = tnumber;//token number�� ���� symbol ����
	token->value.num = num;//token value�� ������ ���� ����
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
	else if (token.number == tnumber)//token�� ������ ��
		printf("number: %d(%s), value: %d\n\n", token.number, tokenName[token.number], token.value.num);
	else if (token.number == tlitchar)//token�� ���� ���ͷ��� ��
		printf("number: %d(%s), value: %c\n\n", token.number, tokenName[token.number], token.value.ch);
	else if (token.number == tlitstr)//token�� ���ڿ� ���ͷ��� ��
		printf("number: %d(%s), value: %s\n\n", token.number, tokenName[token.number], token.value.str);
	else if (token.number == tlitreal)//token�� �Ǽ� ���ͷ��� ��
		printf("number: %d(%s), value: %.10lf\n\n", token.number, tokenName[token.number], token.value.real);
	else
		printf("number: %d(%s)\n\n", token.number, tokenName[token.number]);

}