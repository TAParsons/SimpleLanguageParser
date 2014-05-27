Parser README

 //========================\\
//	Author:	Tracy Parsons	\\
||	Date:	10 / 14 / 13	||
||	Class:	CSE 3341		||
\\	Instructor: Wayne Heym	//
 \\========================//


PRECOMPILATION:

	In this program, there are four preprocessor definitions that can be left in or commented out to change the appearance and behavior of the program:
	'VERBOSE'	makes the int main () function display messages affirming the success of functions it calls, instead of only displaying error messages when they fail.
	'MUTE'		supresses the 'ID = ' and newline after each ID value is displayed by out. This results in a series of space separated integers.
	'ASCII' 	renders the ID values shown by out as characters.
	'DEBUG'		displays additional messages showing information about the program, step-through of mathematical functions, and 

COMPILATION:
	
	To compile, enter: g++ Parser.cpp -o Parser

USE:

	To use, enter: Parser filename datafile
	
		OR
	
	./Parser filename datafile
	
	as dictated by your system.
	
	NOTE: The data file is optional. If omitted, the program with go into interactive mode, meaning the user will be prompted to input data for all In (read) commands.

==============
DOCUMENTATION:
==============

USER MANUAL:

	This program is intended to parse and interpret core program files, then run the resulting program using the data file as the input data. The program will print the core program text in a tidy, aesthetically pleasing fashion, then execute the program. If the program does not have any 'write' commands, then nothing will be output during execution. In addition, there are no safeguards against infinite loops, so it is up to the user to ensure they will not occur. If the program does get stuck for whatever reason, press Ctrl-c to interrupt exectution and exit.
	
	This program was written to be used in a sane Linux environment, and may not work without modification on other operating systems.

DESIGN:
	
	Internally, the Parser class has a subclass for each definition in the core BNF. Each of these classes has a Parse method, and most have an Execute method. The Parse methods recursively call each other, processing the core program file and storing relevant data in each class and IDs in the IDSTRUCT. Each Parse method prints out the core program as the file is evaluated, stopping as soon as an error occurs. This provides help when debugging, since the user can see exactly where the parsing failed.
	
	Once the core program is completely parsed, the Execute method of the Parser can be called. This method calls the Execute methods of the relevant subclasses, which recursively call each other. The Parser executes the core program stored throughout the subclasses, requesting input from the data file and spewing output messages to the command line. If an error occurs during execution, the Parser immediately stops and prints an error message. This is helpful for debugging the core program file as it shows where the error occured. However, since the execution phase generally only displays Identifier values when called for by the core program, there is information on exactly where the program failed.
	
	The IDSTRUCT class is implemented as a hash tree. Each node in the tree has:
		1 Private integer recording the depth of the node;
		1 Public integer recording the ID value;
		1 Public string storing the ID name;
		37 Pointers to child nodes; one for each capital letter and digit, and one for a null value. The root node is basically a freebie for the first ID put in.
		
	This class has five methods; 
	
	1:	Initialize (int d, string n, int v); Initializes the hash tree with an inital ID as the root. Generally, placeholder values are put in so that the first real ID can occupy the root node.
	
	2:	Print (); Used to display the tree during debugging;
	
	3:	Find (string in); Returns a pointer to a node in the tree. If an ID with the same name as the input string exists in the tree, then this function returns a pointer to that ID's node. If that ID does not exist, then this function returns the address of whatever node would be the parent.
	
	4:	Add (string in); Finds the appropriate parent node and creates a child node with the 'in' as name.
	
	5:	Change (string in, int val); Finds the existing node and overwrites the value with 'val'.
	
	The Parser also defines the structure ParserStruct, containing pointers to a Tokenizer and to an IDSTRUCT, and two integers, linenum and linetoken. 'linenum' keeps track of the number of lines processed by the Parser, and 'linetoken' records the number of tokens processed at the beginning of each line. This allows the Parser to print error messages that indicate the line number and the number of tokens into the line that the error occurred. The Parser has two pointers to ParserStructs; one instance, passed through the Parse subclass methods, which has a Tokenizer connected to the program file, and one passed through the Execute subclass methods, which has a Tokenizer connected to the data file (or null value if no data file is specified). This allows separate information to be stored for debugging and easy determination of whether to operate in interactive mode without needing to reinitialize any values.
	
	Externally, the Parser class has no visible values.
	
	The Parser class has three visible methods:
	
	1: Constructor (ifstream *stuff, ifstream *Data); Creates an instance of the Parser class and initializes the ParserStruct tokenizers with the specified input streams.
	
	2: Parse (); Calls the Program Parse method and outputs an error message if it fails.
	
	3: Execute (); Calls the Program Execute method and outputs an error message if it fails.
	
TESTING:

	For testing, this program was given valid core programs, invalid core programs, non-core files, and non-existent files.
