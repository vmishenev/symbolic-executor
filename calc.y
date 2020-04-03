/* Mini Calculator */
/* calc.y */

%{
#include "heading.h"

//int yylex(void);

//extern "C"
//{
	int yyerror(char *s);
        int yyparse(void);
        int yylex(void);  
        //int yywrap()
       // {
       //         return 1;
      //  }

//}
	extern std::map<std::string, func_def*> ftable;
	
%}

%union{
  int		int_val;
  float         f_val;
  std::string*	op_val ;
   std::list<std::string> *largs;
   std::list<expr_t*> *cargs;
        oper_t* oper;
        expr_t* expr;

}

%start	pr_unit 

/*%token	<int_val>	INTEGER_LITERAL
%type	<int_val>	exp
*/
%token int_const char_const float_const id str_const
%token IF ELSE RETURN PUNC FUNC ASSIGN ASSERT or_const and_const eq_const neq_const lte_const gte_const

%left '+' '-'
%left '*' '/'
%nonassoc "then"
%nonassoc ELSE

%type<op_val> id unary_operator eq_const 
%type<int_val> int_const
%type<largs> args_list args
%type<cargs> call_args
%type<expr> exp primary_exp unary_exp mult_exp additive_exp logical_or_exp logical_and_exp equality_exp relational_exp
%type<oper>  body  stmt_list stmt expr_stmt selection_stmt assignment_exp return_stmt

%%

pr_unit: 		function_definition 									
				| pr_unit function_definition
				;

function_definition: 		FUNC  id args body { /* cout<<"func "<< *$2 <<endl; cout<<"args_list "<< $3->size() <<endl; $4->print(1);*/  ftable[*$2] = new func_def(*$2, $3, dynamic_cast<block*>($4)); }
				;

args: 				'(' ')' 		{ $$ =new std::list<std::string>();   }
				| '('args_list ')' 	{  $$ = $2; }
				;
args_list: 			id			{ $$ =new std::list<std::string>(); $$->push_back(*$1);  }
				| args_list ',' id 	{ $$->push_back(*$3); }
				;

body:				'{' stmt_list '}' 	{ $$ = $2; }
				;
stmt_list:			stmt     		{ $$ = new block($1); /*cout<<"stmt _list "<< $1 <<endl;*/ }					
				| stmt_list stmt 	{ dynamic_cast<block*>($$) -> append($2); }
				;

stmt:				expr_stmt
				| selection_stmt
				| return_stmt
				;

expr_stmt:                      ASSERT '('exp ')' ';'	{ $$ = new asserop($3); }
				| assignment_exp ';'	{ $$ = $1; }
				| exp ';'		{ $$ = new exprop($1); }
				| ';'			{ $$ = nullptr; }
				;
	
return_stmt:                    RETURN  exp ';'		{ $$ = new returnop($2); }
				;
	
assignment_exp: 		id ASSIGN exp	{ $$ = new assignop((*$1), $3);}		
				;

		
		
selection_stmt: 		IF '(' logical_or_exp ')' body 	%prec "then" { $$ = new ifop($3, $5, new block()); }	
				| IF '(' logical_or_exp ')' body ELSE body { $$ = new ifop($3, $5, $7); }
				;


exp:				logical_or_exp
				;


logical_or_exp				: logical_and_exp
					| logical_or_exp or_const logical_and_exp	{ $$ = new binary("or", $1, $3); }
					;

logical_and_exp				: equality_exp
					| logical_and_exp and_const equality_exp	{ $$ = new binary("and", $1, $3); }
					;

equality_exp				: relational_exp
					| equality_exp eq_const relational_exp	{ $$ = new binary("==", $1, $3); }
					| equality_exp neq_const relational_exp	{ $$ = new binary("!=", $1, $3); }	
					;

relational_exp				: additive_exp
					| relational_exp '<' additive_exp	{ $$ = new binary("<", $1, $3); }
					| relational_exp '>' additive_exp	{ $$ = new binary(">", $1, $3); }
					| relational_exp lte_const additive_exp { $$ = new binary("<=", $1, $3); }
					| relational_exp gte_const additive_exp { $$ = new binary(">=", $1, $3); }
					;


additive_exp				: mult_exp
					| additive_exp '+' mult_exp { $$ = new binary("+", $1, $3); }
					| additive_exp '-' mult_exp { $$ = new binary("-", $1, $3); }
					;

mult_exp				: primary_exp
					| mult_exp '*' primary_exp { $$ = new binary("*", $1, $3); }
					| mult_exp '/' primary_exp { $$ = new binary("/", $1, $3); }
					;

unary_exp:				unary_operator primary_exp { $$ = new unary(*$1, $2); }
					|primary_exp
					;					
primary_exp: 				id 			{ $$ = new value(*$1); }					
					| int_const 		{ $$ = new value_int($1); }			
					| '(' exp ')'		{ $$ = $2; }
					| id '(' call_args ')'                 { $$=new funcall(*$1, *$3); delete $3;}
					;

call_args: 						{$$ =new std::list<expr_t*>();}
				|  exp			{ $$ =new std::list<expr_t*>(); $$->push_back($1);  }
				| call_args ',' exp 	{ $$->push_back($3); }
				;

unary_operator				: '&' | '*' | '+' | '-' | '~' | '!' 				
					;



%%

int yyerror(std::string s)
{
  extern int yylineno;	// defined and maintained in lex.c
  extern char *yytext;	// defined and maintained in lex.c
  
  cerr << "ERROR: " << s << " at symbol \"" << yytext;
  cerr << "\" on line " << yylineno << endl;
  exit(1);
}

int yyerror(char *s)
{
  return yyerror(std::string(s));
}


