Tokenizer README

 //========================\\
//	Author:	Tracy Parsons	\\
||	Date:	09 / 16 / 13	||
||	Class:	CSE 3341		||
\\	Instructor: Wayne Heym	//
 \\========================//


PRECOMPILATION:

	In this program, there are three preprocessor definitions that can be left in or commented out to change the appearance and behavior of the program:
	'debug' turns on extensive debugging and error checking messages;
	'fancy' changes the program output to show the token number, token string, the next character in the stream, and the token value.
	'AbortOnError' makes the program print an error message and return should the tokenizer return an error or invalid token.

COMPILATION:
	
	To compile, enter: g++ Tokenizer.cpp -o Tokenizer

USE:

	To use, enter: Tokenizer filename
	
		OR
	
	./Tokenizer filename
	
	as dictated by your system

==============
DOCUMENTATION:
==============

USER MANUAL:

	This program is intended to tokenize core program files; However, if AbortOnError is commented out, this program can be used to tokenize valid non-core program files into core tokens and garbage tokens. This program returns an error if the file name is omitted or if the file itself is non-existent or invalid. This program was written to be used in a sane Linux environment, and may not work without modification on other operating systems.

DESIGN:
	
	The Tokenizer class has two values: 'str', and 'value'. These correspond to the token text, e.g. 'while', and the token value, e.g. '8'.
	The Tokenizer class also has two methods:
	
	1: GARBAGE_DAY (ifstream *in, char &c);
	
	'GARBAGE_DAY' grabs characters off of the ifstream 'in' and appends them onto the tokenizer string until it finds whitespace, at which point it returns.
	This function is used to get all of a garbage token from the ifstream, instead of just returning the immediate invalid token and leaving garbage in the ifstream.

	2: GetToken (ifstream *in, char &buffman);
	
	'GetToken' grabs characters off of the ifstream 'in' until it reaches a non-whitespace character or the end of the file. Next, it examines the buffered character to see if it matches the beginning of a reserved word or special character. If buffman could be the start of a reserved word or special character, 'GetToken' grabs characters off of 'in' until the token string matches the reserved word or becomes invalid. If the token string and the buffered character are both valid, 'GetToken' sets the token value and returns. Otherwise, 'GetToken' calls 'GARBAGE_DAY' and returns.
	
	If the buffered character is a capital letter, then 'GetToken' starts appending characters from 'in' onto 'str' until it reaches a non-capital letter, then appends onto 'str' until it hits a non-number character. If the buffered character is a non-whitespace and non-special character, the token is invalid and 'GARBAGE_DAY' is called. Otherwise, 'GetToken' determines the token value based on the first character of the token string, then returns.
	
	These two methods allow Tokenizer to pull tokens off of an ifstream passed to it, determine their validity, and store the associated values.

TESTING:

	For testing, this program was fed both core programs, non-core programs, and non-existent files.
