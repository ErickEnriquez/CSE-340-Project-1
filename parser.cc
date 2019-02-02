/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */
#include <iostream>
#include <cstdlib>
#include "parser.h"


using namespace std;
///////////////////////////////////////////////////////////////////////////////////////////
int* mem = new int[1000];// dynamic array of 1000 elements to hold data
symbol_table* table = new symbol_table[1000];// create a new symbol table of size 1000
int next_availible;
struct stack* stack = new struct stack();//stack to store inputs
struct stack * s2 = new struct stack(); //since inputs will be reverse need another stack to put them in order
struct linkedList* list = new struct linkedList();//create a list to hold the statments

///////////////////////////////////////////////////////////////////////////////////////////
//this function wil take in a string which is the token t.lexme and will look to see if it already exists in the symbol table
//and return its location in the array 
int location_in_s_table(Token t, symbol_table* table) {
	int count = 0;
	while (count < 1000  ) {
		if (table[count].symbol.lexeme == t.lexeme) {
			return count;
		}
		count++;
	}
	return -1;//if not found in the symbol table we return a -1
}
/*the function checks to see if the symbol is in the table and if it is we */
void allocate(Token t, symbol_table* table, int& next_avail, int* mem) {
	if (location_in_s_table(t, table) != -1)//if we dont get a -1 from the symbol table then we need to add the symbol to both tables
		return;
	else {//if we havent seen the symbol before
		if (t.token_type == NUM) {
			table[next_avail].constant = true;//if the token is a constant make not of it
			mem[next_avail] = stoi(t.lexeme);//store the quantity in the memory table because its a constant
		}
		else {
			table[next_avail].constant = false;//make note that this is not a constant
		}
		table[next_avail].symbol = t;//store the symbol at the next available location in memory
		table[next_avail].location = next_avail;//store the location of the symbol for the mem table
		next_avail++;
	}
}

//this function will execute the program from a given linked list
void execute_program(linkedList* list){
	stmt_node* pc = list->start;//get the start for the linked list
	while(pc != nullptr){
	if(pc->statement_type == OUTPUT)//if the statment type is an output statement
		cout<<mem[pc->op1] ;
	else if(pc->statement_type == INPUT){
		mem[pc->op1] =  stoi(s2->pop().lexeme);//pop the input from the stack and store it at the memory location requested
	}
	else if(pc->statement_type == ASSIGN){//if  we get an assin statment find out which one it could be
		switch(pc->operator_type){
			case PLUS: mem[pc->LHS] = mem[pc->op1]+ mem[pc->op2]; break;
			case MINUS: mem[pc->LHS] = mem[pc->op1]- mem[pc->op2]; break;
			case MULT: mem[pc->LHS] = mem[pc->op1]* mem[pc->op2]; break;
			case DIV: mem[pc->LHS] = mem[pc->op1]/ mem[pc->op2]; break;
			case NOOP: mem[pc->LHS]= mem[pc->op1];break;// if we are just assigning with 1 operand
			default: break;
		}
	}
	//else()needs more of the different cases here
	pc= pc->next;//moce down the list
	}
}

void Parser::syntax_error()
{
	cout << "SYNTAX ERROR\n";
	exit(1);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
	Token t = lexer.GetToken();
	if (t.token_type != expected_type)
		syntax_error();
	return t;
}

// this function simply checks the next token without
// consuming the input
// Written by Mohsen Zohrevandi
Token Parser::peek()
{
	Token t = lexer.GetToken();
	lexer.UngetToken(t);
	return t;
}

// Parsing//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//parses the begining of the input
void Parser::parse_input() {
	parse_program();
	parse_inputs();
	return;
}

void Parser::parse_program() {
	Token t = lexer.GetToken();
	if (t.token_type == MAIN) {
		lexer.UngetToken(t);
		parse_main();
	}
	else if (t.token_type == PROC) {
		lexer.UngetToken(t);
		parse_proc_decl_section();
		parse_main();
	}
	else {
		syntax_error();
	}
	return;
}

void Parser::parse_proc_decl_section() {
	Token t = lexer.GetToken();// if main we are done with this section
	if (t.token_type == MAIN) {
		lexer.UngetToken(t);
		return;
	}
	else if (t.token_type == PROC) {
		lexer.UngetToken(t);
		parse_proc_decl();
		parse_proc_decl_section();
	}
	else
		syntax_error();
}

void Parser::parse_proc_decl() {
	Token t1 = lexer.GetToken();//PROC COMSUME TOKEN
	if (t1.token_type != PROC) {
		syntax_error();
	}
	parse_procedure_name();
	parse_procedure_body();
	Token t2 = lexer.GetToken();//ENDPROC CONSUME TOKEN

}

void Parser::parse_procedure_name() {
	Token t = lexer.GetToken();//ID OR NUM , CONSUME TOKEN
	if (t.token_type != ID && t.token_type != NUM)
		syntax_error();
	return;
}

void Parser::parse_procedure_body() {
	Token t = lexer.GetToken();//ID || OUPUT || INPUT || DO
	if (t.token_type != OUTPUT && t.token_type != INPUT && t.token_type != DO && t.token_type != ID) {//if the token given isn't any of the expected tokens throw a syntax error
		syntax_error();
	}
	lexer.UngetToken(t);
	parse_statement_list();
	return;
}

//parses main in input
void Parser::parse_main() {
	Token t = lexer.GetToken();//get the token but if we get the terminal that is expected you consume it
	if (t.token_type == MAIN) {
		parse_procedure_body();
	}
	else {
		syntax_error();
	}
	return;
}

// should go back into this becuase it could cause infinite recursion
void Parser::parse_statement_list() {

	Token t = lexer.GetToken();//if we get a number we have finished parsing the statements and we are in the inputs
	if (t.token_type == ENDPROC) {//if we get endproc then we are done parsing statements
		lexer.UngetToken(t);
		return;
	}
	if (t.token_type != NUM) {//while there are more statements to parse
		lexer.UngetToken(t);
		stmt_node* st = parse_statement();
		list->add_statement(st );//add the node into the statment list

	}
	else if (t.token_type == NUM) {
		lexer.UngetToken(t);
		return;
	}
	else if (t.token_type == END_OF_FILE) {
		syntax_error();
	}
	parse_statement_list();
}

struct stmt_node* Parser::parse_statement() {
	Token t1 = lexer.GetToken();// INPUT || OUTPUT || ID || DO
	Token t2 = lexer.GetToken();// EQUAL || SEMICOLON
	stmt_node* node = nullptr;
	if (t1.token_type == INPUT) {
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
		node = parse_input_statement();
	}
	else if (t1.token_type == OUTPUT) {
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
		node = parse_ouput_statement();
	}
	else if (t1.token_type == DO) {
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
		parse_do_statement();
	}
	else if (t1.token_type == ID && t2.token_type == EQUAL) {
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
		node = parse_assign_statement();
	}
	else if (t1.token_type == ID && t2.token_type == SEMICOLON) {
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
		parse_procedure_invocation();
	}
	else {
		syntax_error();
	}
	return node;

}

//this function takes in retuns a stmt_node* to be added into the linked list later
 stmt_node* Parser::parse_input_statement() {
	Token t1 = lexer.GetToken();//INPUT CONSUME
	Token t2 = lexer.GetToken();//ID	CONSUME
	stmt_node* st = nullptr;
	allocate(t2,table,next_availible,mem);//allocate the ID into the symbol table
	if (t1.token_type == INPUT && t2.token_type == ID) {
		st = new stmt_node();//create a new node 
		st->statement_type = INPUT;
		st->next  = nullptr;
		st->LHS = location_in_s_table(t2,table);//store the location of the left hand side
		st->op1 = location_in_s_table(t2,table);//store the location of  op1
		st->operator_type = NOOP;

	}
	else {
		syntax_error();
	}
	parse_expr(st);//
	Token t3 = lexer.GetToken();//SEMICOLON CONSUME
	if (t3.token_type != SEMICOLON) {
		syntax_error();
	}
	return st;
}

stmt_node* Parser::parse_ouput_statement() {
	Token t1 = lexer.GetToken();//OUTPUT CONSUME
	Token t2 = lexer.GetToken();//ID	CONSUME
	if (t1.token_type != OUTPUT || t2.token_type != ID) {
		syntax_error();
	}
	allocate(t2,table,next_availible,mem);//allocate the variable into the symbol table
	stmt_node* st = new stmt_node();//create a new statement node
	st->statement_type = OUTPUT; //mkae not of the type of statment we are making
	int x = location_in_s_table(t2,table);//make the LHS the location of the ID in the symbol table
	st->LHS = x;
	st->op1 = x;
	st->operator_type = NOOP;//the oeprator type for this statment is a no-op
	st->next = nullptr;
	parse_expr(st);
	Token t3 = lexer.GetToken();//SEMICOLON	CONSUME
	if (t3.token_type != SEMICOLON) {
		syntax_error();
	}
	return st;
}

void Parser::parse_do_statement() {
	Token t1 = lexer.GetToken();//DO	CONSUME
	Token t2 = lexer.GetToken();//ID	CONSUME
	allocate(t2,table,next_availible,mem);//allocate the token into the symbol table
	if (t1.token_type != DO || t2.token_type != ID) {
		syntax_error();
	}
	parse_procedure_invocation();
	return;
}

void Parser::parse_procedure_invocation() {
	Token t1 = lexer.GetToken();//ID	CONSUME
	Token t2 = lexer.GetToken();//SEMICOLON	CONSUME
	allocate(t2,table,next_availible,mem);//allocate id to symbol table
	if (t1.token_type != ID || t2.token_type != SEMICOLON) {
		syntax_error();
	}
	return;
}

stmt_node* Parser::parse_assign_statement() {
	Token t1 = lexer.GetToken();//ID	CONSUME
	Token t2 = lexer.GetToken();//EQUAL	CONSUME
	allocate(t1 , table, next_availible , mem);//allocate the symbol to the table
	stmt_node* st = new stmt_node();
	st->statement_type = ASSIGN;
	st->LHS = mem[location_in_s_table(t1,table)];//get the address of the left hand side of the assign
	
	if (t1.token_type != ID || t2.token_type != EQUAL) {
		syntax_error();
	}
	
	parse_expr(st);
	Token t3 = lexer.GetToken();//SEMICOLON	CONSUME
	if (t3.token_type != SEMICOLON) {
		syntax_error();
	}
	return st;
}

void Parser::parse_operator(stmt_node* s) {
	Token t = lexer.GetToken();// ID || MINUS || DIV || MULT	CONSUME!!!!
	if (t.token_type != PLUS && t.token_type != MINUS && t.token_type != DIV && t.token_type != MULT)
		syntax_error();
	s->operator_type = t.token_type;//get the type of operator we will be using  + || - || * || /
	return ;
}


void Parser::parse_expr(stmt_node* st ) {
	Token t = lexer.GetToken();//If we get a semicolon then we are done parsing expression
	if (t.token_type == SEMICOLON) {
		lexer.UngetToken(t);
		return;
	}
	else {
		lexer.UngetToken(t);
		Token temp =  parse_primary();
		if(temp.token_type == NUM)//if we get a constant
			st->op1  = stoi(temp.lexeme);//if the first op is a constant store it in op1
		else
			st->op1 = mem[location_in_s_table(temp,table)];//store the value of the variable in op1
		
		t = lexer.GetToken();//check if expression is done
		if (t.token_type == SEMICOLON) {//if we get a semicolon the expression is done
			lexer.UngetToken(t);
			return;
		}
		lexer.UngetToken(t);//unget the token
		 parse_operator(st);
		
		temp =  parse_primary();
		if(temp.token_type == NUM)
			st->op2 = stoi(temp.lexeme);//we the 2nd operator is a constant then we can just store it in op2
		else
			st->op2 = mem[location_in_s_table(temp,table)];//store the address of op2
	}
}


Token Parser::parse_primary() {
	Token t = lexer.GetToken(); //ID || NUM
	if (t.token_type == ID || t.token_type == NUM ) {
		allocate(t,table,next_availible,mem);//allocate the token onto the symbol table
	}
	else {
		syntax_error();
	}
	return t;//return the token
}

void Parser::parse_inputs() {
	Token t = lexer.GetToken();// NUM	CONSUME!!!!!!
	if (t.token_type == NUM) {
		//do stuff here
		stack->push(t);//store the token on the stack
		parse_inputs();//recurse
	}
	else if (t.token_type == END_OF_FILE) {
		lexer.UngetToken(t);
		//program is done parsing input
	}
	else {
		syntax_error();
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{

	LexicalAnalyzer lexer;
	Token token;
	Parser p;//make a parser object to parse the input
	list->start = nullptr;//initialize the start of the list to be a nullptr
	for (int i = 0; i < 1000; i++) {//go through and make all of the quantities in the tables 0
		mem[i] = 0;
		table[i].location = 0;
		table[i].constant = false;
	}
	p.parse_input();
	while (stack->isEmpty() == false) {
		s2->push(stack->pop());
	}

	/*for(int i  = 0 ; i< 1000 ; i++){
		if(table[i].symbol.lexeme.empty() == false)
		cout<<table[i].symbol.lexeme << " location " << table[i].location << " contents " << mem[i] << "\n";
	}*/

	execute_program(list);//execute the program

	return 0;
}
