/* ********************

	CLASS	:	CSE 3341
	PROJECT	:	CORE PARSER
	AUTHOR	:	TRACY PARSONS
	DATE	:	10/14/13

	See README for notes on compilation, execution, and documentation

********************* */
// RESERVED WORDS: (1 - 11)
/*
	program
	begin
	end
	int
	if
	then
	else
	while
	loop
	read
	write
*/

// SPECIAL SYMBOLS: (12 - 30)
/*
	;
	,
	=
	!
	[
	]
	&&
	||
	(
	)
	+
	-
	*
	!=
	==
	<
	>
	<=
	>=
*/

// INTEGERS: (31)
//	unsigned, with leading zeroes possible

// IDENTIFIERS: (32)
//	start with uppercase letter, followed by zero or more uppercase letters and ending with zero or more digits

// EOF: (33)

/* BNF for Core:
	<let>		::= 'A'|'B'|'C'|'D'|'E'|'F'|'G'|'H'|'I'|'J'|'K'|'L'|'M'|'N'|'O'|'P'|'Q'|'R'|'S'|'T'|'U'|'V'|'W'|'X'|'Y'|'Z'
	<digit>		::= '0'|'1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'

*/

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "Tokenizer.fixed.cpp"

//=================================================================
// these are optional settings to change the behavior of the parser
// feel free to try them out
//=================================================================

//#define VERBOSE
//#define MUTE
//#define ASCII
//#define DEBUG
//#define R_TO_L

using namespace std;

class Parser {		// this is the main class that .
	private:
	
	class IDSTRUCT {			// tree of hash tables (hash tree?) that keeps track of all of the IDs and their values.
		int depth;				// ID's hashed based on depth'th character value
		IDSTRUCT *Next[37];		// one for each digit and letter, plus one for null.
		public:
		int Value;		// ID value
		string Name;	// ID name
		
		void Initialize (int d, string n, int v) {	// initializes values and child pointers
			depth = d;
			Value = v;
			Name = n;
			
			for (int i = 0; i<37; i++) { Next[i] = 0; }	// 0:null, 1-10:'0'-'9', 11-36:'A'-'Z'
			return;
		}
		
		void Print () {		// Print out tree. Used for debugging
			for (int i=depth; i>0; --i) { cout << "	"; }
			cout << "ID: '" << Name << "'; Value: " << Value << ";\n"; fflush (0);
			for (int i=0; i<37; i++) {
				if (Next[i]) { Next[i]->Print (); }
			}
			return;
		}
		
		IDSTRUCT *Find (string in) {	// Find and return ID node with same name as input string, or parent node if ID does not exist.
			bool alpha = in[depth] >= 'A' && in[depth] <= 'Z';
			bool num = in[depth] >= '0' && in[depth] <= '9';
			if (alpha || num || !in[depth]) {	// check if character valid
				if (in == Name || !Next[in[depth]-alpha*('A'-11)-num*('0'-1)]) { return this; }	// ID = input string OR this is the parent node; return self
				else { return Next[in[depth]-alpha*('A'-11)-num*('0'-1)]->Find (in); }			// else: go deeper
			}
			cout << "\nMighty bad things have happened; '" << in << "' has non-letters!";
			return 0;
		}
		
		bool Add (string in) {			// Create and add ID with input string as name to tree
			IDSTRUCT *n = Find (in);	
			if ( !n ) {					// Couldn't find appropriate node
				cout << "\nAdd Identifier: Find failed and returned null node! Abort!"; fflush (0);
				return false;
			}
			else if ( n->Name == in ) {	// duplicate! ID already in tree
				cout << "\nAdd Identifier: '" << in << "' already exists! No duplicate identifiers allowed!";
				return false;
			}
			else if ( n->Name == "invalid\0" ) {	// root node initialized with invalid name; basically a freebie for the first ID
				n->Name = in;
				n->Value = 0;
			}
			else {	// if all else fails, make a new node and add identifier
				bool alpha = (in[n->depth] >= 'A' && in[n->depth] <= 'Z');
				bool num = (in[n->depth] >= '0' && in[n->depth] <= '9');
				if (!alpha && !num && in[n->depth]) {
					cout << "\nAdd ID: Error! '" << in[n->depth] << "'(" << (int)in[n->depth] << ") is an invalid character!";
					return false;
				}
				n->Next[in[n->depth]-alpha*('A'-11)-num*('0'-1)] = new IDSTRUCT;
				n->Next[in[n->depth]-alpha*('A'-11)-num*('0'-1)]->Initialize (n->depth+1, in, 0);
				free (n = 0);
			}
			return true;
		}
		
		bool Change (string in, int val) {	// change value of an existing ID
			IDSTRUCT *n = Find (in);
			if ( n && n->Name == in ) { n->Value = val; }
			else {
				cout << "\nChange Identifier: '" << in << "' not found in id tree! Cannot change non-existant identifier!";
				return false;
			}
			return true;
		}
	};
	
	Tokenizer *PT;	// pointer to prevent automatic initialization; tokenizes program file
	Tokenizer *DT;	// tokenizes data file; 
	
	struct ParserStruct {
		Tokenizer *Bank;	// tokenizer; program or data file, depending on parsing or execution
		IDSTRUCT *StoredIDs;		// structure keeping track of IDs and corresponding values
		int linenum; int linetoken;
	} *ProgStruct, *ExecStruct;
	
	class Number {	//	<no>		::= <digit> | <digit><no>	; Terminal
		public:
		int val;
		
		Number () { val = 0; }
		
		bool Parse (Tokenizer *ProgramToken) {
			if (ProgramToken->value == 31) {
				int i = -1;
				while (ProgramToken->str[++i]) {
					val *= 10;
					val += ProgramToken->str[i]-'0';
				}
			}
			else {
				cout << "\t<<\nParse Number: '" << ProgramToken->str << "' not a number!";
				return false;
			}
			ProgramToken->GetToken ();
			return true;
		}
	};
	
	class Identifier {	//	<id>		::= <let> | <let><id> | <let><no>	; Terminal
		public:
		string val;
		
		Identifier () { val = ""; }
		
		bool Parse (Tokenizer *ProgramToken) {
			if (ProgramToken->value == 32) {
				cout << ProgramToken->str;
				val = ProgramToken->str;
			}
			else {
				cout << "\t<<\nParse Identifier: '" << ProgramToken->str << "' not a valid Identifier!";
				return false;
			}
			ProgramToken->GetToken ();
			return true;
		}
	};
	
	class Id_List {	//	<id list>	::= <id> | <id>',' <id list>
		public:
		Identifier i;
		Id_List *l;
		
		Id_List () { i = Identifier (); l = 0; }
		
		bool Parse (Tokenizer *ProgramToken) {
			if (!i.Parse (ProgramToken)) { return false; }
			if (ProgramToken->value == 13) {
				cout << ", ";
				ProgramToken->GetToken ();
				l = new (Id_List);
				if (!l->Parse (ProgramToken)) { return false; }
			}
			return true;
		}
		bool UpdateIDs (IDSTRUCT *Ego) {
			if (Ego->Add (i.val)) {
				if (l) { return l->UpdateIDs (Ego); }
				return true;
			}
			cout << "\nUpdateIDs: '" << i.val << "' could not be added!";
			return false;
		}
	};
	
	class Comp_Op {	//	<comp op>	::= '!=' | '==' | '<' | '>' | '<=' | '>='	; Terminal
		public:
		int val;
		
		Comp_Op () { val = -1; }
		
		bool Parse (Tokenizer *ProgramToken) {
			switch (ProgramToken->value) {
				case 25:
					val = 0;
					break;
				case 26:
					val = 1;
					break;
				case 27:
					val = 2;
					break;
				case 28:
					val = 3;
					break;
				case 29:
					val = 4;
					break;
				case 30:
					val = 5;
					break;
				default:
					cout << "\t<<\nParse Comp_Op: Token '" << ProgramToken->str << "' not a comp_op!";
					return false;
					break;
			}
			cout << " " << ProgramToken->str << " ";
			ProgramToken->GetToken ();
			return true;
		}
		
	};
	
	// prototype Expression class needed by Operation class
	class Expression;
	
	class Operation {	//	<op>		::= <no> | <id> | '('<exp>')'
		public:
		Number *n;		// pointers to objects used since they are manually initialized.
		Identifier *id;	// keeps track of what actually needs to be parsed/executed
		Expression *e;
		int val;
		
		Operation () {
			n = 0; id = 0; e = 0;
			val = 0;
		}
		
		bool Parse (Tokenizer *ProgramToken) {
			switch (ProgramToken->value) {
				case 31:	// number
					n = new Number ();
					if (!n->Parse (ProgramToken)) { return false; }
					cout << n->val;
					break;
				case 32:	// letter
					id = new Identifier ();
					if (!id->Parse (ProgramToken)) { return false; }
					break;
				case 20:	// parenthesis
					cout << "( ";
					ProgramToken->GetToken ();
					
					e = new Expression ();
					if (!e->Parse (ProgramToken)) { return false; }
					
					if (ProgramToken->value != 21) {
						cout << "\t<<\nParse Operation: Got '" << ProgramToken->str << "', Expected ')'!";
						return false;
					}
					cout << " )";
					ProgramToken->GetToken ();
					break;
				default:
					cout << "\t<<\nParse Operation: '" << ProgramToken->str << "' not operation!";
					break;
			}
			return true;
		}
		bool Execute (IDSTRUCT *Ego) {
			if (n) {
				val = n->val;
#ifdef DEBUG
				cout << val;
#endif
			}
			else if (id) {
				IDSTRUCT *tmp = Ego->Find (id->val);
				if (tmp && tmp->Name == id->val) { val = tmp->Value; }
				else {
					cout << "\nExecute Operation: Identifier '" << id->val << "' does not exist!";
					val = 0;
					return false;
				}
#ifdef DEBUG
				cout << val;
#endif
				free (tmp = 0);
			}
			else if (e) {	
				if (!e->Execute (Ego)) { return false; }				
				val = e->val;
			}
			return true;
		}
	};
	
	class Comparison {	//	<comp>		::= '('<op> <comp_op> <op>')'
		public:
		Operation a, b;
		Comp_Op c;
		bool val;
		
		Comparison () {
			a = Operation (); b = Operation (); c = Comp_Op ();
			val = false;
		}
		
		bool Parse (Tokenizer *ProgramToken) {
			if (ProgramToken->value == 20) {	// (
				cout << "( ";
				ProgramToken->GetToken ();
				
				if (!a.Parse (ProgramToken)) { return false; }
				if (!c.Parse (ProgramToken)) { return false; }
				if (!b.Parse (ProgramToken)) { return false; }
				if (ProgramToken->value == 21) { cout << " )"; }
				else {
					cout << "\t<<\nParse Comparison: Got '" << ProgramToken->str << "', expected ')'!";
					return false;
				}
			}
			else {
				cout << "\t<<\nParse Comparision: Got '" << ProgramToken->str << "', expected '('!";
				return false;
			}
			return true;
		}
		bool Execute (IDSTRUCT *Ego) {
#ifdef DEBUG
			cout << "(";
#endif
			if (!a.Execute (Ego)) { return false; }
#ifdef DEBUG
			cout << " # ";
#endif
			if (!b.Execute (Ego)) { return false; }
			switch (c.val) { // 0:'!=' | 1:'==' | 2:'<' | 3:'>' | 4:'<=' | 5:'>='
				case 0:
					val = a.val != b.val;
					break;
				case 1:
					val = a.val == b.val;
					break;
				case 2:
					val = a.val < b.val;
					break;
				case 3:
					val = a.val > b.val;
					break;
				case 4:
					val = a.val <= b.val;
					break;
				case 5:
					val = a.val >= b.val;
					break;
				default:
					cout << "\nExecute Comparison: Radiation levels too high! Save yourself!";
					return false;
					break;
			}
#ifdef DEBUG
			cout << " = " << val << ")";
#endif
			return true;
		}
	};
	
	class Condition {	//	<cond>		::= <comp> | !<cond> | [<cond> '&&' <cond>] | [<cond> '||' <cond>]
		public:
		Condition *a, *b;
		Comparison *c;
		int mode;	// -1 = !, 1 = &&, 2 = ||
		bool val;
		
		Condition () {
			a = 0; b = 0; c = 0;
			mode = 0; val = -1;
		}
		
		bool Parse (Tokenizer *ProgramToken) {
			if (ProgramToken->value == 16) {	// '[', so [cond &| cond]
				cout << " [ ";
				ProgramToken->GetToken ();
				
				a = new Condition ();
				b = new Condition ();
				if (!a->Parse (ProgramToken)) { return false; }
				
				if (ProgramToken->value == 18) { mode = 1; }
				else if (ProgramToken->value == 19) { mode = 2; }
				else {
					cout << "\t<<\nParse Conditon: Got '" << ProgramToken->str << "', Expected '&&' or '||'!";
					return false;
				}
				cout << " " << ProgramToken->str << " ";
				ProgramToken->GetToken ();
				
				if (!b->Parse (ProgramToken)) { return false; }
				if (ProgramToken->value != 17) {
					cout << "\t<<\nParse Conditon: Got '" << ProgramToken->str << "', Expected ']'!";
					return false;
				}
				cout << " ] ";
				ProgramToken->GetToken ();
			}
			else if (ProgramToken->value == 15) {	// !cond
				mode = -1;
				cout << " !";
				ProgramToken->GetToken ();
				a = new Condition ();
				if (!a->Parse (ProgramToken)) { return false; }
			}
			else { // comparision!
				c = new Comparison ();
				if (!c->Parse (ProgramToken)) { return false; }
				if (ProgramToken->value != 21) {
					cout << "\t<<\nParse Condition: Got '" << ProgramToken->str << "', Expected ')'!";
					free (c = 0);
					return false;
				}
				ProgramToken->GetToken ();
			}
			return true;
		}
		bool Execute (IDSTRUCT *Ego) {
#ifdef DEBUG
			if (mode) { cout << "("; }
#endif			
			switch (mode) {
				case -1:	// not
#ifdef DEBUG
					cout << "!";
#endif
					if (!a->Execute (Ego)) { return false; }
					val = !a->val;
					break;
				case 0:		// comparision 
					if (!c->Execute (Ego)) { return false; }
					val = c->val;
					break;
				case 1:		// and
					if (!a->Execute (Ego)) { return false; }
#ifdef DEBUG
					cout << " && ";
#endif
					if (!b->Execute (Ego)) { return false; }
					val = a->val && b->val;
					break;
				case 2:		// or
					if (!a->Execute (Ego)) { return false; }
#ifdef DEBUG
					cout << " || ";
#endif
					if (!b->Execute (Ego)) { return false; }
					val = a->val || b->val;
					break;
			}
#ifdef DEBUG
			if (mode) { cout << " = " << val << ")"; }
#endif
			return true;
		}
	};
	
	class Term {	//	<trm>		::= <op> | <op> '*' <trm>
		public:
		Operation op;
		Term *t;
		int val;
		
		Term () {
			op = Operation ();
			t = 0;
			val = 0;
		}
		
		bool Parse (Tokenizer *ProgramToken) {
			if (!op.Parse (ProgramToken)) { return false; }
			if ( ProgramToken->value == 24 ) {	// *
				cout << " * ";
				ProgramToken->GetToken ();
				t = new Term ();
				if (!t->Parse (ProgramToken)) { return false; }
			}
			return true;
		}
		bool Execute (IDSTRUCT *Ego) {
#ifdef DEBUG
			if (t) { cout << "["; }
#endif
			if (!op.Execute (Ego)) { return false; }
			val = op.val;

			if (t) {
#ifdef DEBUG
				cout << " * ";
#endif
				if (!t->Execute (Ego)) { return false; }
				val *= t->val;
#ifdef DEBUG
				cout << " = " << val;
#endif
			}
#ifdef DEBUG
			if (t) { cout << "]"; }
#endif
			return true;
		}
	};
	
	class Expression {	//	<exp>		::= <trm> | <trm> '+' <exp> | <trm> '-' <exp>
		public:
		Term t;
		int mode;
		Expression *e;
		int val;
		
		Expression () {
			t = Term ();
			mode = 0;
			e = 0;
			val = 0;
		}
		
		bool Parse (Tokenizer *ProgramToken) {
			if (!t.Parse (ProgramToken)) { return false; }
			if ( ProgramToken->value == 22 ) {	// +
				cout << " + ";
				ProgramToken->GetToken ();
				mode = 1;
				e = new Expression ();
				if (!e->Parse (ProgramToken)) { return false; }
			}
			else if (ProgramToken->value == 23) {	// -
				cout << " - ";
				ProgramToken->GetToken ();
				mode = -1;
				e = new Expression ();
				if (!e->Parse (ProgramToken)) { return false; }
			}
			return true;
		}
		
		bool Execute (IDSTRUCT *Ego) {
#ifdef DEBUG
			if (e) { cout << "{"; }
#endif
			if (!t.Execute (Ego)) { return false; }
			val = t.val;
			
			if (e) {
#ifdef DEBUG
				if (mode > 0) { cout << " + "; } else { cout << " - "; }
#endif				
				if (!e->Execute (Ego)) { return false; }
#ifdef R_TO_L
				val = val*mode+e->val;	// core evaluates Right-to-Left
#else
				val = val+e->val*mode;	// core evaluates Left-to-Right, but groups Right-to-Left
#endif

#ifdef DEBUG
				cout << " = " << val;
#endif
			}
#ifdef DEBUG
			if (e) { cout << "}"; }
#endif
			return true;
		}
	};
	
	class Out {	//	<out>		::= 'write' <id list>';'
		public:
		Id_List i;
		
		Out () { i = Id_List (); }
		
		bool Parse (Tokenizer *ProgramToken, string indent) {
			cout << indent << "write ";
			ProgramToken->GetToken ();
			if (!i.Parse (ProgramToken)) { return false; }
			if (ProgramToken->value != 12) {
				cout << "\t<<\nParse Out: Got '" << ProgramToken->str << "', Expected ';'!";
				return false;
			}
			cout << ";\n";
			ProgramToken->GetToken ();
			return true;
		}
		
		bool Execute (IDSTRUCT *Ego) {
			Id_List *cur;
			cur = &i;
			while (cur) {
				IDSTRUCT *tmp = Ego->Find (cur->i.val);
				if ( tmp && tmp->Name == cur->i.val ) {
#ifndef MUTE
					cout << cur->i.val << " = ";
#endif
#ifdef ASCII
					cout << (char) tmp->Value;
#else
					cout << tmp->Value;
#endif
#ifdef MUTE
					cout << " ";
#else
					cout << ";\n";
#endif
				}
				else {
					cout << "\nExecute Out: Identifier '" << cur->i.val << "' not found!";
					return false;
				}
				cur = cur->l;
			}
			return true;
		}
	};
	
	class In {	//	<in>		::= 'read' <id list>';'
		public:
		Id_List i;
		
		In () { i = Id_List (); }
		
		bool Parse (Tokenizer *ProgramToken, string indent) {
			ProgramToken->GetToken ();
			cout << indent << "read ";
			if (!i.Parse (ProgramToken)) { return false; }
			if (ProgramToken->value != 12) {
				cout << "\t<<\nParse In: Got '" << ProgramToken->str << "', Expected ';'!";
				return false;
			}
			cout << ";\n";
			ProgramToken->GetToken ();
			return true;
		}
	
		bool Execute (ParserStruct *PassedIn) {
			Id_List *cur;
			cur = &i;
			while (cur) {	// ensure finds valid id; read from data file
				if (!PassedIn->Bank) {	// switch to istream >> int
					string str;
					cout << "Input value for " << cur->i.val << ": ";
					cin >> str;
					
					bool neg = false;
					int index = -1;
					if (CharType (str[0] == 23)) {	// negative
						neg = true;
						index++;
					}
					int num = 0;
					while (++index < length (str)) {
						num *= 10;
						if (CharType (str[index]) == 1) { num += str[index]-'0'; }
						else {
							cout << "\nRead error: '" << str[index] << "' is not a number!";
							return false;
						}
					}
					
					PassedIn->StoredIDs->Find (cur->i.val)->Value = num * (1 - 2*neg);
				}
				else {
					bool negative = false;
					if (PassedIn->Bank->value == 23) {	// str = '-'; negative number
						negative = true;
						PassedIn->Bank->GetToken ();
					}
					else if ( PassedIn->Bank->value == 33 ) {
						cout << "\nRead error: End of file encountered!";
						return false;
					}
					
					Number n = Number ();
					if (!n.Parse ( PassedIn->Bank )) { return false; }	
					PassedIn->StoredIDs->Find (cur->i.val)->Value = n.val * (1-2*negative);
#ifdef DEBUG
					cout << "read " << cur->i.val << " = " << n.val * (1-2*negative) << ";\n";
#endif
				}
				cur = cur->l;
			}
			return true;
		}
	};
	
	// prototype Loop, If, and Assign classes used by Statement
	class Loop;
	class If;
	class Assign;
	
	class Statement {	//	<stmt>		::= <assign> | <if> | <loop> | <in> | <out>
		Assign *a;
		If *i;
		Loop *l;
		In *n;
		Out *o;
		
		public:
		Statement () { a = 0; i = 0; l = 0; n = 0; o = 0; }
		
		bool Parse (ParserStruct *PassedIn, string indent) {
			switch (PassedIn->Bank->value) {
				case 32:	// identifier
					a = new Assign ();
					return a->Parse (PassedIn->Bank, indent);
					break;
				case 5:		// if
					i = new If ();
					return i->Parse (PassedIn, indent);
					break;
				case 8:		// while
					l = new Loop ();
					return l->Parse (PassedIn, indent);
					break;
				case 10:	// read
					n = new In ();
					return n->Parse (PassedIn->Bank, indent);
					break;
				case 11:	// write
					o = new Out ();
					return o->Parse (PassedIn->Bank, indent);
					break;
			}
			cout << "\t<<\nParse Statement: Got '" << PassedIn->Bank->str << "', Expected keyword!\n"; fflush (0);
			return false;
		}
		
		bool Execute (ParserStruct *PassedIn) {
			PassedIn->linenum++;
			if (PassedIn->Bank) { PassedIn->linetoken = PassedIn->Bank->count; }
			
			if (a) {	// assign
				return a->Execute (PassedIn->StoredIDs);
			}
			else if (i) {	// if
				return i->Execute (PassedIn);
			}
			else if (l) {	// loop (while)
				return l->Execute (PassedIn);
			}
			else if (n) {	// in (read)
				return n->Execute (PassedIn);
			}
			else if (o) {	// out (write)
				return o->Execute (PassedIn->StoredIDs);
			}
			return false;
		}
	};
	
	class Stmt_Sequence {	//	<stmt seq>	::= <stmt> | <stmt> <stmt seq>
		public:
		Statement s;
		Stmt_Sequence *ss;
		
		Stmt_Sequence () {
			s = Statement ();
			ss = 0;
		}
		
		bool Parse (ParserStruct *PassedIn, string indent) {
			PassedIn->linenum++;
			PassedIn->linetoken = PassedIn->Bank->count;
			
			if (!s.Parse (PassedIn, indent)) { return false; }
			if (PassedIn->Bank->value != 3 && PassedIn->Bank->value != 7) {	// stmt seqs end with 'end' or 'else'
				ss = new Stmt_Sequence ();
				if (!ss->Parse (PassedIn, indent)) { return false; }
			}
			return true;
		}
		
		bool Execute (ParserStruct *PassedIn) {
			if (!s.Execute (PassedIn)) { return false; }
			if (ss) {
				if (!ss->Execute (PassedIn)) { return false; }
			}
			return true;
		}
	};
		
	class Loop {	//	<loop>		::= 'while' <cond> 'loop' <stmt seq> 'end;'
		public:
		Condition c;
		Stmt_Sequence s;
		
		Loop () { c = Condition (); s = Stmt_Sequence (); }
		
		bool Parse (ParserStruct *PassedIn, string indent) {
			cout << indent << "while ";
			PassedIn->Bank->GetToken ();
			
			if (!c.Parse (PassedIn->Bank)) { return false; }
			if (PassedIn->Bank->value != 9) {
				cout << "\t<<\nParse Loop: Got '" << PassedIn->Bank->str << "', Expected 'loop'!";
				return false;
			}
			cout << " loop\n";
			PassedIn->Bank->GetToken ();
			
			if (!s.Parse (PassedIn, indent + "	")) { return false; }
			PassedIn->linenum++; PassedIn->linetoken = PassedIn->Bank->count;
			if (PassedIn->Bank->value != 3) {
				cout << "\t<<\nParse Loop: Got '" << PassedIn->Bank->str << "', Expected 'end'!";
				return false;
			}
			cout << indent << "end";
			PassedIn->Bank->GetToken ();
			
			if (PassedIn->Bank->value != 12) {
				cout << "\t<<\nParse Loop: Got '" << PassedIn->Bank->str << "', Expected ';'!";
				return false;
			}
			cout << ";\n";
			PassedIn->Bank->GetToken ();
			
			return true;
		}
		
		bool Execute (ParserStruct *PassedIn) {
			if (!c.Execute (PassedIn->StoredIDs)) { return false; }
			int actline = PassedIn->linenum;	// actual line number
#ifdef DEBUG
				cout << "\n";
#endif
			while (c.val) {
				PassedIn->linenum = actline;
				if (!s.Execute (PassedIn)) { return false; }
				if (!c.Execute (PassedIn->StoredIDs)) { return false; }
#ifdef DEBUG
				cout << "\n";
#endif
			}
			return true;
		}
	};
	
	class If {	//	<if>		::= 'if' <cond> 'then' <stmt seq> 'end;' | 'if' <cond> 'then' <stmt seq> 'else' <stmt seq> 'end;'
		public:
		Condition c;
		Stmt_Sequence s;
		Stmt_Sequence *e;
		
		If () {
			c = Condition (); s = Stmt_Sequence ();
			e = 0;
		}
		
		bool Parse (ParserStruct *PassedIn, string indent) {
			cout << indent << "if ";
			PassedIn->Bank->GetToken ();
			
			if (!c.Parse (PassedIn->Bank)) { return false; }
			if (PassedIn->Bank->value != 6) {
				cout << "\t<<\nParse If: Got '" << PassedIn->Bank->str << "', Expected 'then'!";
				return false;
			}
			cout << " then\n";
			PassedIn->Bank->GetToken ();
			
			if (!s.Parse (PassedIn, indent + "	")) { return false; }
			PassedIn->linenum++; PassedIn->linetoken = PassedIn->Bank->count;
			if (PassedIn->Bank->value == 7) {
				cout << indent << "else\n";
				PassedIn->Bank->GetToken ();
				e = new Stmt_Sequence ();
				if (!e->Parse (PassedIn, indent + "	")) { return false; }
			}
			
			PassedIn->linenum++; PassedIn->linetoken = PassedIn->Bank->count;
			if (PassedIn->Bank->value != 3) {
				cout << "\t<<\nParse If: Got '" << PassedIn->Bank->str << "', Expected 'end'!";
				return false;
			}
			cout << indent << "end";
			
			PassedIn->Bank->GetToken ();
			if (PassedIn->Bank->value != 12) {
				cout << "\t<<\nParse If: Got '" << PassedIn->Bank->str << "', Expected ';'!";
				return false;
			}
			cout << ";\n";
			PassedIn->Bank->GetToken ();
			
			return true;
		}
		
		bool Execute (ParserStruct *PassedIn) {
			if (!c.Execute (PassedIn->StoredIDs)) { return false; }
			if (c.val) {
#ifdef DEBUG
				cout << "\n";
#endif
				if (!s.Execute (PassedIn)) { return false; }
				PassedIn->linenum++;
				if (PassedIn->Bank) { PassedIn->linetoken = PassedIn->Bank->count; }
			}
			else if (e) {
#ifdef DEBUG
				cout << "\n";
#endif
				if (!e->Execute (PassedIn)) { return false; }
				PassedIn->linenum++;
				if (PassedIn->Bank) { PassedIn->linetoken = PassedIn->Bank->count; }
			}
			return true;
		}
	};
	
	class Assign {	//	<assign>	::= <id> '=' <exp>';'
		public:
		Identifier i;
		Expression e;
		
		Assign () { i = Identifier (); e = Expression (); }
		
		bool Parse (Tokenizer *ProgramToken, string indent) {
			cout << indent;
			if (!i.Parse (ProgramToken)) { return false; }
			if (ProgramToken->value != 14) {
				cout << "\t<<\nParse Assign: Got '" << ProgramToken->str << "', Expected '='!";
				return false;
			} 
			cout << " = ";
			ProgramToken->GetToken ();
			
			if (!e.Parse (ProgramToken)) { return false; }
			if (ProgramToken->value != 12) {
				cout << "\t<<\nParse Assign: Got '" << ProgramToken->str << "', Expected ';'!";
				return false;
			} 
			cout << ";\n";
			ProgramToken->GetToken ();
			return true;
		}
		
		bool Execute (IDSTRUCT *Ego) {
#ifdef DEBUG
			cout << i.val << " = ";
#endif
			if (!e.Execute (Ego)) { return false; }
			if (!Ego->Change (i.val, e.val)) { return false; }
#ifdef DEBUG
			cout << "\n";
#endif			
			return true;
		}
	};
	
	class Declaration {	//	<decl>		::= 'int' <id list>';'
		public:
		Id_List i;
		
		Declaration () { i = Id_List (); }
		
		bool Parse (ParserStruct *PassedIn, string indent) {
			if (PassedIn->Bank->value != 4) {
				cout << "\t<<\nParse Declaration: Got '" << PassedIn->Bank->str << "', Expected 'int'!";
				return false;
			}
			cout << indent << "int ";
			PassedIn->Bank->GetToken ();
			
			if (!i.Parse (PassedIn->Bank)) { return false; }
			if (!i.UpdateIDs (PassedIn->StoredIDs)) { return false; }
			if (PassedIn->Bank->value != 12) {
				cout << "\t<<\nParse Declaration: Got '" << PassedIn->Bank->str << "', Expected ';'!\n";
				return false;
			}
			cout << ";\n";
			PassedIn->Bank->GetToken ();
			return true;
		}
	};
	
	class Decl_Sequence {	//	<decl seq>	::= <decl> | <decl> <decl seq>
		public:
		Declaration d;
		Decl_Sequence *ds;
		
		Decl_Sequence () {
			d = Declaration ();
			ds = 0;
		}
		
		bool Parse (ParserStruct *PassedIn, string indent) {
			PassedIn->linenum++;
			PassedIn->linetoken = PassedIn->Bank->count;
			
			if (!d.Parse (PassedIn, indent)) { return false; }
			if (PassedIn->Bank->value == 4) {	// program <declseq> begin <stmtseq> end ; begin = 2, int = 4
				ds = new Decl_Sequence ();
				if (!ds->Parse (PassedIn, indent)) { return false; }
			}
			return true;
		}
	};
	
	class Program {	//	<prog>		::= 'program' <decl seq> 'begin' <stmt seq> 'end'
		public:
		Decl_Sequence d;
		Stmt_Sequence s;
		
		Program () { d = Decl_Sequence (); s = Stmt_Sequence (); }
		
		bool Parse (ParserStruct *PassedIn, string indent) {
			if (PassedIn->Bank->value != 1) {
				cout << "\t<<\nParse Program: Got '" << PassedIn->Bank->str << "', Expected 'program'!";
				return false;
			}
			cout << "\n" << indent << "program\n";
			PassedIn->Bank->GetToken ();
			
			if (!d.Parse (PassedIn, indent + "	")) { return false; }
			
			if (PassedIn->Bank->value != 2) {
				cout << "\t<<\nParse Program: Got '" << PassedIn->Bank->str << "', Expected 'begin'!";
				return false;
			}
				
			PassedIn->linenum++; PassedIn->linetoken = PassedIn->Bank->count;
			cout << indent << "begin\n";
			PassedIn->Bank->GetToken ();
			
			if (!s.Parse (PassedIn, indent + "	")) { return false; }
			
			PassedIn->linenum++; PassedIn->linetoken = PassedIn->Bank->count;
			if (PassedIn->Bank->value != 3) {
				cout << "\t<<\nParse Program: Got '" << PassedIn->Bank->str << "', Expected 'end'!";
				return false;
			}
			cout << indent << "end\n";
			PassedIn->Bank->GetToken ();
			
			if (PassedIn->Bank->value != 33) {
				cout << "\t<<\nParse Program: Got '" << PassedIn->Bank->str << "', Expected End Of File!\n";
				return false;
			}
			return true;
		}
		
		bool Execute (ParserStruct *PassedIn) {
			return s.Execute (PassedIn);
		}
	} RootProg;
	
	public:	// only the Parser functions are public; all of the classes etc. inside are hidden
	Parser (ifstream *stuff, ifstream *Data) {	// construct the pointers to stuff
		ProgStruct = new ParserStruct;
		ProgStruct->Bank = new Tokenizer (stuff);
		ProgStruct->StoredIDs = new IDSTRUCT;
		ProgStruct->StoredIDs->Initialize (0, "invalid", -1);
		ProgStruct->linenum = 1;
		ProgStruct->linetoken = 1;
	
		ExecStruct = new ParserStruct;
		if (Data) { ExecStruct->Bank = new Tokenizer (Data); }
		else { ExecStruct->Bank = 0; }	
		ExecStruct->StoredIDs = ProgStruct->StoredIDs;	// IDSTRUCT needed by both; Both pointers go to the same instance
		ExecStruct->linenum = 1;
		ExecStruct->linetoken = 1;
	}
	
	bool Parse () {
		if (!RootProg.Parse (ProgStruct, "")) {
			cout << "\nError on line " << ProgStruct->linenum << ", token " << ProgStruct->Bank->count-ProgStruct->linetoken << " in line, token " << ProgStruct->Bank->count << " in total\n";
			return false;
		}
#ifdef DEBUG
			cout << "\n" << ProgStruct->linenum << " lines in program; " << ProgStruct->Bank->count << " tokens in program;\nIDs defined in program:\n";
			ProgStruct->StoredIDs->Print ();
#endif
		return true;
	}
	
	bool Execute () {
		if (!RootProg.Execute (ExecStruct)) {
			cout << "\nError on line " << ExecStruct->linenum << "\n";
			return false;
		}
#ifdef DEBUG
		if ( ExecStruct->Bank ) { cout << "\n" << ExecStruct->Bank->count << " tokens in data file;"; }
		cout << "\nID values:\n";
		ExecStruct->StoredIDs->Print ();
#endif
		return true;
	}
};
	
int main (int argc, char *argv[]) {	// thing to do the file stuff for parser
	if (argc < 2) { cout << "No program file specified!\n"; }	// fatal error
	else {
#ifdef VERBOSE
		cout << "Opening program file stream...";
#endif
		ifstream *Prog = Read (argv[1]);	// open up input stream
		ifstream *Data = 0;
		if (!Prog) {	// mistakes were made
			cout << "Something wrong with program file!! Aborting execution!\n";
			free (Prog = 0);
			return 0;
		}
		if (argc < 3) {		// not-so-fatal error. Just force the user to provide any data called for by read commands
			cout << "No data file specified! Interactive mode!\n";
		}
		else {
#ifdef VERBOSE
			cout << "Opening data file stream...";
#endif
			Data = Read (argv[2]);
			if (!Data) {	// data file didn't open
				cout << "Something wrong with data file!! Aborting execution!\n";
				Prog->close ();
				free (Prog = 0);
				free (Data = 0);
				return 0;
			}
		}
		
#ifdef VERBOSE
		cout << "Initializing Parser...";
#endif

		Parser TheRealDeal = Parser (Prog, Data);	// this here's the real deal
		
#ifdef VERBOSE		
		cout << "Parsing program file...";
#endif
		if ( TheRealDeal.Parse () ) {	
#ifdef VERBOSE
			cout << "\nParsed successfully!\n";
			sleep (2);	
			cout << "Executing program file...\n\n"; fflush (0);
#endif
			TheRealDeal.Execute ();
		}
#ifdef VERBOSE	
		cout << "All done, closing file stream(s)...\n";
#endif
		if (Data) {	// remember to clean up
			Data->close ();
			free ( Data = 0 );
		}
		Prog->close ();		// close input stream and free pointer
		free ( Prog = 0 );
	}
	return 0;
}
