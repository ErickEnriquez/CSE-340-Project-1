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
	cout << "parse successful\n";
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
	if (t.token_type != ID || t.token_type != NUM)
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
	else if (t.token_type != NUM) {//while there are more statements to parse
		lexer.UngetToken(t);
		parse_statement();
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

void Parser::parse_statement() {
	Token t1 = lexer.GetToken();// INPUT || OUTPUT || ID || DO
	Token t2 = lexer.GetToken();// EQUAL || SEMICOLON
	if (t1.token_type == INPUT) {
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
		parse_input_statement();
	}
	else if (t1.token_type == OUTPUT) {
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
		parse_ouput_statement();
	}
	else if (t1.token_type == DO) {
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
		parse_do_statement();
	}
	else if (t1.token_type == ID && t2.token_type == EQUAL) {
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
		parse_assign_statement();
	}
	else if (t1.token_type == ID && t2.token_type == SEMICOLON) {
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
		parse_procedure_invocation();
	}
	else {
		syntax_error();
	}


}

void Parser::parse_input_statement() {
	Token t1 = lexer.GetToken();//INPUT CONSUME
	Token t2 = lexer.GetToken();//ID	CONSUME
	if (t1.token_type != INPUT || t2.token_type != ID) {
		syntax_error();
	}
	parse_expr();
	Token t3 = lexer.GetToken();//SEMICOLON CONSUME
	if (t3.token_type != SEMICOLON) {
		syntax_error();
	}
	return;
}

void Parser::parse_ouput_statement() {
	Token t1 = lexer.GetToken();//OUTPUT CONSUME
	Token t2 = lexer.GetToken();//ID	CONSUME
	if (t1.token_type != OUTPUT || t2.token_type != ID) {
		syntax_error();
	}
	parse_expr();
	Token t3 = lexer.GetToken();//SEMICOLON	CONSUME
	if (t3.token_type != SEMICOLON) {
		syntax_error();
	}
	return;
}

void Parser::parse_do_statement() {
	Token t1 = lexer.GetToken();//DO	CONSUME
	Token t2 = lexer.GetToken();//ID	CONSUME
	if (t1.token_type != DO || t2.token_type != ID) {
		syntax_error();
	}
	parse_procedure_invocation();
	return;
}

void Parser::parse_procedure_invocation() {
	Token t1 = lexer.GetToken();//ID	CONSUME
	Token t2 = lexer.GetToken();//SEMICOLON	CONSUME
	if (t1.token_type != ID || t2.token_type != SEMICOLON) {
		syntax_error();
	}
	return;
}

void Parser::parse_assign_statement() {
	Token t1 = lexer.GetToken();//ID	CONSUME
	Token t2 = lexer.GetToken();//EQUAL	CONSUME
	if (t1.token_type != ID || t2.token_type != EQUAL) {
		syntax_error();
	}
	parse_expr();
	Token t3 = lexer.GetToken();//SEMICOLON	CONSUME
	if (t3.token_type != SEMICOLON) {
		syntax_error();
	}
	return;
}

void Parser::parse_operator() {
	Token t = lexer.GetToken();// ID || MINUS || DIV || MULT	CONSUME!!!!
	if (t.token_type != PLUS && t.token_type != MINUS && t.token_type != DIV && t.token_type != MULT)
		syntax_error();
	return;
}



void Parser::parse_inputs() {
	Token t = lexer.GetToken();// NUM	CONSUME!!!!!!
	if (t.token_type == NUM) {
		//do stuff here
		parse_inputs();
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

void Parser::parse_expr() {
	Token t = lexer.GetToken();//If we get a semicolon then we are done parsing expression
	if (t.token_type == SEMICOLON) {
		lexer.UngetToken(t);
		return;
	}
	else {
		parse_primary();
		parse_operator();
		parse_primary();
	}
}


void Parser::parse_primary() {
	Token t = lexer.GetToken(); //ID || NUM
	if (t.token_type == ID) {

	}
	else if (t.token_type == NUM) {

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
	p.parse_input();

}
