/* ********************

	CLASS	:	CSE 3341
	PROJECT	:	CORE TOKENIZER
	AUTHOR	:	TRACY PARSONS
	DATE	:	10/14/13

	See README for notes on compilation, execution, and documentation

********************* */
// RESERVED WORDS: (1 - 11)
/*
	program	1
	begin
	end
	int
	if
	then	6
	else
	while
	loop
	read
	write	11
*/

// SPECIAL SYMBOLS: (12 - 30)
/*
	;		12
	,
	=
	!		15
	[
	]
	&&
	||
	(		20
	)
	+
	-
	*
	!=		25
	==
	<
	>
	<=
	>=		30
*/

// INTEGERS: (31)
//	unsigned, with leading zeroes possible

// IDENTIFIERS: (32)
//	start with uppercase letter, followed by zero or more uppercase letters and ending with zero or more digits

// EOF: (33)

/* BNF for Core:
	<prog>		::= 'program' <decl seq> 'begin' <stmt seq> 'end'
	<decl seq>	::= <decl> | <decl> <decl seq>
	<stmt seq>	::= <stmt> | <stmt> <stmt seq>
	<decl>		::= 'int' <id list>';'
	<id list>	::= <id> | <id>',' <id list>
	<stmt>		::= <assign> | <if> | <loop> | <in> | <out>
	<assign>	::= <id> '=' <exp>';'
	<if>		::= 'if' <cond> 'then' <stmt seq> 'end;' | 'if' <cond> 'then' <stmt seq> 'else' <stmt seq> 'end;'
	<loop>		::= 'while' <cond> 'loop' <stmt seq> 'end;'
	<in>		::= 'read' <id list>';'
	<out>		::= 'write' <id list>';'
	<cond>		::= <cond> | !<cond> | <cond> '&&' <cond> | <cond> '||' <cond>
	<comp>		::= '('<op> <comp> <op>')'
	<exp>		::= <trm> | <trm> '+' <exp> | <trm> '-' <exp>
	<trm>		::= <op> | <op> '*' <trm>
	<op>		::= <no> | <id> | '('<exp>')'
	<comp op>	::= '!=' | '==' | '<' | '>' | '<=' | '>='
	<id>		::= <let> | <let><id> | <let><no>
	<let>		::= 'A'|'B'|'C'|'D'|'E'|'F'|'G'|'H'|'I'|'J'|'K'|'L'|'M'|'N'|'O'|'P'|'Q'|'R'|'S'|'T'|'U'|'V'|'W'|'X'|'Y'|'Z'
	<no>		::= <digit> | <digit><no>
	<digit>		::= '0'|'1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'

*/


#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

//#define debug
//#define fancy
#define AbortOnError
#define UseParser

using namespace std;

int length (string in) {
	int i = 0;
	while (in[i]) {i++;}
	return i;
}

ifstream *Read (const char *filename) {	// return iostream from specified file
	
#ifdef debug
	cout << "Read in..."; fflush (0);
#endif
	
	ifstream *ifs = new (ifstream);
	ifs->open (filename, ifstream::in);
	if (!ifs->good ()) { cout << "Bad things have happened!!\n"; ifs = 0; }
	//else { cout << "File probably opened okay...\n"; }
	
#ifdef debug
	cout << "Read out!\n"; fflush (0);
#endif
	
	return ifs;
}

int CharType (char in) {
	if (in >= '0' && in <= '9') { return 1; }
	else if (in >= 'A' && in <= 'Z') { return 2; }
	else if (in >= 'a' && in <= 'z') { return 3; }
	else if (in == ' ' || in == '\t' || in == '\n') { return 4; }	// space, tab, and newline
	// SPECIAL SYMBOLS: ; , = ! [ ] & | ( ) + - * ! < >
	else if (in == '!' || (in >= '(' && in <= '-') || (in >= ';' && in <= '>') || in == '[' || in == ']' || in == '|') { return 5; }
	return 0;
}

class Tokenizer {
	char buffman;
	ifstream *in;
	void GARBAGE_DAY ();
	
	public:
	string str;
	int value;
	int count;
	
	void GetToken ();
	
	Tokenizer (ifstream *stuff) {
		buffman = 0;
		count = 0;
		in = stuff;
		GetToken ();		// get first token
	}
};

void Tokenizer::GARBAGE_DAY () {
	count++;
	while (!in->eof() && CharType (buffman) != 4) {
		str += buffman;
		buffman = in->get ();
	}
	return;
}
	
void Tokenizer::GetToken () {	// get single token
#ifdef debug
	cout << "GetToken in..."; fflush (0);
#endif
	//======
	count++;
	//======
	value = 0;	// initialize token
	str = "";
	while (buffman <= ' ') {	// get rid of whitespace, newlines, weird teletype control characters, etc.
		if (in->eof ()) {		// at end of file, so return EOF token
			value = 33;
			
#ifdef debug
			cout << "EOF!! ";
			cout << "GetToken out!\n"; fflush (0);
			sleep (1);
#endif
			return;
		}
		buffman = in->get ();
	}
	string Ref;		// reference string and
	int i = 0;		// index to be used in switch case below
	switch (buffman) {
	// RESERVED WORDS
	case 'p':	// program
		Ref = "program";	// set reference string to expected value
		while (i < length (Ref) && buffman == Ref[i++]) {
			// step through Ref, adding characters to token string until done or error
			str += buffman;
			buffman = in->get ();
		}
		// if token string equals Ref AND character after token compatible, then set token value to appropriate number. Otherwise, don't change it.
		if (str == Ref && CharType (buffman) >= 4) { value = 1; }
		break;
		
	case 'b':	// begin
		Ref = "begin";
		while (i < length (Ref) && buffman == Ref[i++]) {
			str += buffman;
			buffman = in->get ();
		}
		if (str == Ref && CharType (buffman) >= 4) { value = 2; }
		break;
		
	case 'e':	// end, else
		Ref = "end";
		// note: Loops with additional possible token strings need to increment index outside of the condition check, or else the index gets advanced one too many for the next loop(s).
		while (i < length (Ref) && buffman == Ref[i]) {	
			str += buffman;
			buffman = in->get ();
			i++;
		}
		if (str == Ref && (CharType (buffman) >= 4 || buffman == ';')) { value = 3; break; }
		
		// Second possiblity! Set reference string value to next expected string and repeat!
		Ref = "else";
		while (i < length (Ref) && buffman == Ref[i++]) {
			str += buffman;
			buffman = in->get ();
		}
		if (str == Ref && (CharType (buffman) >= 4 || buffman == ';')) { value = 7; }
		break;
		
	case 'i':	// int, if
		Ref = "if";
		while (i < length (Ref) && buffman == Ref[i]) {
			str += buffman;
			buffman = in->get ();
			i++;
		}
		if (str == Ref && CharType (buffman) >= 4) { value = 5; break; }
		
		Ref = "int";
		while (i < length (Ref) && buffman == Ref[i++]) {
			str += buffman;
			buffman = in->get ();
		}
		if (str == Ref && CharType (buffman) >= 4) { value = 4; }
		break;
		
	case 't':	// then
		Ref = "then";
		while (i < length (Ref) && buffman == Ref[i++]) {
			str += buffman;
			buffman = in->get ();
		}
		if (str == Ref && CharType (buffman) >= 4) { value = 6; }
		break;
		
	case 'w':	// while, write
		Ref = "while";
		while (i < length (Ref) && buffman == Ref[i]) {
			str += buffman;
			buffman = in->get ();
			i++;
		}
		if (str == Ref && CharType (buffman) >= 4) { value = 8; break; }
		
		Ref = "write";
		while (i < length (Ref) && buffman == Ref[i++]) {
			str += buffman;
			buffman = in->get ();
		}
		if (str == Ref && CharType (buffman) >= 4) { value = 11; }
		break;
		
	case 'l':	// loop
		Ref = "loop";
		while (i < length (Ref) && buffman == Ref[i++]) {
			str += buffman;
			buffman = in->get ();
		}
		if (str == Ref && CharType (buffman) >= 4) { value = 9; }
		break;
	
	case 'r':	// read
		Ref = "read";
		while (i < length (Ref) && buffman == Ref[i++]) {
			str += buffman;
			buffman = in->get ();
		}
		if (str == Ref && CharType (buffman) >= 4) { value = 10; }
		break;
	
	// SPECIAL SYMBOLS
	//	Note: special symbol tokens do not need to check next character for validity
	case ';':	// end of stuff
		str += buffman;
		buffman = in->get ();
		if (str == ";") { value =  12; }
		break;
		
	case ',':
		str += buffman;
		buffman = in->get ();
		if (str == ",") { value =  13; }
		break;
		
	case '=':	// could be '=' or '=='
		str += buffman;
		buffman = in->get ();
		if (str == "=" && buffman != '=') { value =  14; break; }
		
		str += buffman;
		buffman = in->get ();
		value =  26;
		break;
		
	case '!':	// could be '!' or '!='
		str += buffman;
		buffman = in->get ();
		if (str == "!" && buffman != '=') { value =  15; break; }
		
		str += buffman;
		buffman = in->get ();
		value = 25;
		break;
		
	case '[':
		str += buffman;
		buffman = in->get ();
		if (str == "[") { value =  16; }
		break;
		
	case ']':
		str += buffman;
		buffman = in->get();
		if (str == "]") { value =  17; }
		break;
		
	case '&':
		str += buffman;
		str += in->get ();
		buffman = in->get ();
		if (str == "&&") { value =  18; }
		break;
	
	case '|':
		str += buffman;
		str += in->get ();
		buffman = in->get ();
		if (str == "||") { value =  19; }
		break;
	
	case '(':
		str += buffman;
		buffman = in->get();
		// could be id's or numbers after this token, not just white space
		if (str == "(") { value =  20; }
		break;
		
	case ')':
		str += buffman;
		buffman = in->get();
		if (str == ")") { value =  21; }
		break;
		
	case '+':
		str += buffman;
		buffman = in->get();
		if (str == "+") { value =  22; }
		break;
		
	case '-':
		str += buffman;
		buffman = in->get();
		if (str == "-") { value =  23; }
		break;
		
	case '*':
		str += buffman;
		buffman = in->get();
		if (str == "*") { value =  24; }
		break;
		
	case '<':	// could be '<' or '<='
		str += buffman;
		buffman = in->get();
		if (str == "<" && buffman != '=') { value =  27; break; }
		
		str += buffman;
		buffman = in->get();
		value =  29;
		break;
		
	case '>':	// could be '>' or '>='
		str += buffman;
		buffman = in->get();
		if (str == ">" && buffman != '=') { value =  28; break; }
		
		str += buffman;
		buffman = in->get();
		value =  30;
		break;
	}
	if (value) {	// non-zero value, so switch case found it! Otherwise, try other stuff...
		
#ifdef debug
		cout << "Switch case success! ";
		cout << "GetToken out!\n"; fflush (0);
#endif
		
		return;
	}
	
	// we've gone through the test cases. If there's anything in str, then we've had a problem.
	// GARBAGE DAY!
	if (str != "") {
		
#ifdef debug
		cout << "Error! '" << str << "' in str after switch cases! ";
		cout << "GetToken out!\n"; fflush (0);
#endif
		
		GARBAGE_DAY ();
		return;
	}
	
	// otherwise, buffman must be the start of an identifier or an integer
	while (CharType (buffman) == 2) {	// Captial letters; should be an identifier
		str += buffman;
		buffman = in->get ();
	}
	while (CharType (buffman) == 1) {	// Numbers; could be integer, so figure out value
		str += buffman;
		buffman = in->get ();
	}
	if (CharType (buffman) <= 3) {		// CharType (buffman) = 3 or 0 here
		// if we're here, then there's an error: someone mixed non numbers into our integer/non capital letters into our identifier!
		GARBAGE_DAY ();
	}
	else if (CharType (str[0]) == 2) { value =  32; }	// first character capital letter, therefore an identifier
	else if (CharType (str[0]) == 1) { value =  31; }	// first character number, therefore integer
	else { GARBAGE_DAY (); }							// otherwise, garbage
	
#ifdef debug
	if (value == 31) { cout << "str '" << str << "' is an integer!\n"; }
	else if (value == 32) { cout << "str '" << str << "' is an identifier!\n"; }
	else { cout << "Error! '" << str << "' is garbage! IT'S GARBAGE DAY!\n"; }
	cout << "GetToken out!\n"; fflush (0);
#endif
	
	return;
}
#ifndef UseParser
int main (int argc, char *argv[]) {	// test driver for tokenizer
	if (argc < 2) {
		cout << "No file specified!\n";
	}
	else {
		ifstream *FFF = Read (argv[1]);	// open up input stream
		if (!FFF) {	// mistakes were made
			cout << "Something wrong with specified file!! Aborting execution!\n";
			free (FFF);
			return 0;
		}
		Tokenizer T (FFF);// T.value = 0;
		//T.in = FFF;
#ifdef fancy
		int i = 0;
#endif
		do {		// Tokenize stream until we get to end of file
			
#ifdef fancy
			cout << "Token " << ++i << ": '" << T.str << "', '";	// output token number, string, and value
			if (B == '\n') { cout << "NEWLINE"; }
			else if (FFF->eof ()) { cout << "NULL"; }
			else { cout << T.buffman; }
			cout << "', " << T.value << '\n';
#else
			cout << T.value << '\n';
#endif
			T.GetToken ();	// Get token from stream	
#ifdef AbortOnError
		} while (T.value && T.value != 33);
		if (!T.value) { cout << "ERROR: Illegal token '" << T.str << "' encountered! Aborting execution!\n"; }
#else
		} while (T.value != 33);
#endif
		
		FFF->close ();		// close input stream and free pointer
		free ( FFF = 0 );
	}
	return 0;
}
#endif
