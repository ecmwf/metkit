%{

/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/* Since we store C++ ojects on the parser stack, BYSON/YACC cannot rellocater the stack              */
/* Therefore we define YYINITDEPTH (initial stack size) to a high enough value so we never rellocate  */
/* We also set YYMAXDEPTH to the same value, since there is no need to grow the stack                 */

#ifdef __APPLE__
/* On the mac, the stack is more limited */
/* This should be selected by CMAKE */
#define YYINITDEPTH 2500
#define YYMAXDEPTH  2500
#else
#define YYINITDEPTH 10000
#define YYMAXDEPTH  10000
#endif

#include <unistd.h>

#include "metkit/mars/rules/MarsExpression.h"

#ifdef YYBISON
#define YYSTYPE_IS_DECLARED
int yylex();
extern int yydebug;
extern "C" int isatty(int);
#endif

struct YYSTYPE {
	Expression<const MarsTaskProxy> *con;
	Value      val;
	long long     num;
	QueuePermission *per;
    AccessRule     *acc;
	Ordinal		    ord;
	std::vector<Expression<const MarsTaskProxy>*> vec;
};

template<class T>
struct is_in {
	bool operator()(const Value& a,const Value& b)
	{
		std::vector<Value> v = b;
		return find(v.begin(),v.end(),a) != v.end();
	}
};

template<class T>
struct match {
	bool operator()(const Value& a,const Value& b)
	{
		// TODO: Use regex
		std::vector<Value> va = a;
		std::vector<Value> vb = b;

		for(int i=0;i<va.size();i++)
		{
			std::string sa = va[i];
			for(int j=0;j<vb.size();j++)
			{
				std::string sb = vb[j];
				//std::cout << "MATCH  [" << sa << "] [" << sb << "]" << std::endl;
				if (sa.find(sb) != std::string::npos) return true;
			}
		}

		return false;
	}
};

inline const char *opname(const is_in<Value>&)     { return "in";}
inline const char *opname(const match<Value>&)     { return "match";}

typedef CondUnary<std::negate<Value>,const MarsTaskProxy >          CondNEG;
typedef CondBinary<std::multiplies<Value>,const MarsTaskProxy >     CondMUL;
typedef CondBinary<std::divides<Value>,const MarsTaskProxy >        CondDIV;
typedef CondBinary<std::modulus<Value>,const MarsTaskProxy >        CondMOD;
typedef CondBinary<std::plus<Value>,const MarsTaskProxy >           CondADD;
typedef CondBinary<std::minus<Value>,const MarsTaskProxy >          CondSUB;
typedef CondBinary<std::greater<Value>,const MarsTaskProxy >        CondGT;
typedef CondBinary<std::equal_to<Value>,const MarsTaskProxy >       CondEQ;
typedef CondBinary<std::less<Value>,const MarsTaskProxy >           CondLT;
typedef CondBinary<std::greater_equal<Value>,const MarsTaskProxy >  CondGE;
typedef CondBinary<std::less_equal<Value>,const MarsTaskProxy >     CondLE;
typedef CondBinary<std::not_equal_to<Value>,const MarsTaskProxy >   CondNE;
typedef CondUnary<std::logical_not<Value>,const MarsTaskProxy >     CondNOT;
typedef CondBinary<std::logical_and<Value>,const MarsTaskProxy >    CondAND;
typedef CondBinary<std::logical_or<Value>,const MarsTaskProxy >     CondOR;

typedef CondBinary<is_in<Value>,const MarsTaskProxy >          CondIN;
typedef CondBinary<match<Value>,const MarsTaskProxy >          CondMATCH;

#ifdef COME_BACK
inline   Value CondBinary<logical_and<Value>, const MarsTaskProxy>::eval(const MarsTaskProxy& task) const
{
	Value v = left_->eval(task);
	if(v)
		return right_->eval(task);

	return v;
}
#endif

%}

%token <val>STRING
%token <val>IDENT
%token <num>INTEGER
%token <num>UNIT
%token IF
%token SWITCH
%token CASE
%token DEFAULT
%token AND
%token NOT
%token OR
%token GE
%token LE
%token NE
%token EQ
%token IN
%token CONTAINS
%token IF
%token THEN
%token ELSE
%token END
%token SELECT
%token WHEN
%token OTHERWISE

%token NODE
%token NAME
%token ACL
%token NOTE
%token SHAPE
%token HOOK
%token AXIS

%token INSERT
%token ACCEPT

%token FAIL
%token PRINT

%token FUNCTION

%token LIMIT
%token PRIORITY
%token PERMISSION
%token ACCESS
%token SCHEDULE
%token MESSAGE
%token WARNING
%token ERROR

%token INCLUDE

%token PUSH
%token POP

%token MATCH

%type <con>condition;
%type <con>atom_or_number;
%type <con>expression;
%type <con>factor;
%type <con>power;
%type <con>term;
%type <con>conjonction;
%type <con>disjonction;
%type <con>accessor;
%type <vec>expression_list;
%type <vec>expression_two;

%type <val>number
%type <val>class_name;
%type <val>template_param;

%type <val>value;
%type <val>value_list;

%type <val>hash_entry;
%type <val>hash_entries;
%type <val>hash_map;

%type <acc>accesses
%type <acc>access

%type <per>permissions
%type <per>permission


%token HEXABYTE
%token PETABYTE
%token TERABYTE
%token GIGABYTE
%token MEGABYTE
%token KILOBYTE
%token BYTE


%%

start : permissions { yypermissions = $1; }
      | accesses    { yyaccesses = $1; }
	  ;

class_name : STRING
		   | STRING '*' { $$ = std::string($1) + "*"; }
		   | STRING '<' template_param '>'
				{
					std::string s = std::string($1) + "<" + std::string($3);
					if(s[s.length()-1] == '>')
						s = s + " ";
					s = s + ">";
					$$ = s;
				}

		   ;

value      : STRING
		   | number
		   ;


value_list : value                { $$ = Value::makeList($1); }
		   | value_list ',' value { $1 += Value::makeList($3); $$ = $1; }
		   ;


hash_entry: STRING ':' value  { $$ = Value::makeMap(); $$[$1] = $3; }
          | STRING ':' '(' value_list ')' { $$ = Value::makeMap(); $$[$1] = $4; }
          ;

hash_entries: hash_entry
            | hash_entries ',' hash_entry { $$ = $1;
                                            eckit::ValueMap m = $3;
                                            for(auto j = m.begin(); j != m.end(); ++j) {
                                                $$[(*j).first] = (*j).second;
                                            }
                                            }
            ;

hash_map : '{' '}'                 { $$ = Value::makeMap(); }
         | '{' hash_entries '}'    { $$ = $2; }
         ;



template_param : class_name ',' template_param { $$ = std::string($1) + "," + std::string($3); }
		       | class_name { $$ = $1; }
			   ;

/*=======================================================================*/

accessor     : STRING '(' STRING ')'   { $$ = MarsExpressionFactory::create($1,$3); }
             | STRING '('  ')'         { $$ = MarsExpressionFactory::create($1,""); }
			 ;

atom_or_number : STRING                   { $$ = new StringExpression<const MarsTaskProxy>($1);    }
			   | '(' expression ')'       { $$ = $2;                                          }
			   | '$' STRING '$'           { $$ = new ParamExpression<std::string>($2);        }
			   | '%' STRING '%'           { $$ = new ParamExpression<long>($2);               }
			   | '@' STRING '@'           { $$ = new ParamExpression<Date>($2);               }
               | '?' STRING '?'           { $$ = new ParamDefinedExpression<std::string>($2); }
			   | '-' atom_or_number       { $$ = new CondNEG($2);                             }
			   | number                   { $$ = new NumberExpression<const MarsTaskProxy>($1);    }
			   | accessor
			   ;

number: INTEGER UNIT { $$ = Value($1*$2); }
	  | INTEGER      { $$ = Value($1); }
	  ;

expression_list : expression_two
				| expression_list ',' atom_or_number { $$ = $1; $$.push_back($3); $1.clear(); }
				;


expression_two  : atom_or_number ',' atom_or_number { $$.clear(); $$.push_back($1); $$.push_back($3); }
				;

/* note: a^b^c -> a^(b^c) as in fortran */

/*
power       : atom_or_number '^' power
            | atom_or_number_or_list
            ;
*/

power       : atom_or_number
			;

factor      : factor '*' power          { $$ = new CondMUL($1,$3);   }
            | factor '/' power          { $$ = new CondDIV($1,$3); }
            | factor '%' power          { $$ = new CondMOD($1,$3); }
            | power
            ;

term        : term '+' factor           { $$ = new CondADD($1,$3);   }
            | term '-' factor           { $$ = new CondSUB($1,$3);   }
            | term MATCH factor           { $$ = new CondMATCH($1,$3);   }
            /* | term '&' factor */
            | factor
            ;

condition   : condition '>' term        { $$ = new CondGT($1,$3);   }
            | condition EQ term        { $$ = new CondEQ($1,$3);   }
            | condition '<' term        { $$ = new CondLT($1,$3);   }
            | condition  GE term        { $$ = new CondGE($1,$3);   }
            | condition  LE term        { $$ = new CondLE($1,$3);   }
            | condition  NE term        { $$ = new CondNE($1,$3);   }
            | condition  IN term       { $$ = new CondIN($1,$3);   }
            | condition CONTAINS term  { $$ = new CondIN($3,$1);   }
            | condition  NOT IN term   { $$ =  new CondNOT(new CondIN($1,$4));   }
            | NOT condition             { $$ = new CondNOT($2);   }
            | term
            ;

conjonction : conjonction AND condition       { $$ = new CondAND($1,$3);   }
            | condition
            ;

disjonction : disjonction OR conjonction      { $$ = new CondOR($1,$3);   }
            | conjonction
            ;

expression  : disjonction
		    | expression_list          { $$ = new ListExpression<const MarsTaskProxy>($1); $1.clear(); }
            ;

empty :
      ;

permission:    PERMISSION STRING expression '=' expression
				{ $$ = new QueuePermission($2,$3,$5); }
        ;

access:    ACCESS STRING expression hash_map
                { $$ = new AccessRule($2,$3,$4); }
        |   ACCESS STRING expression
                { $$ = new AccessRule($2,$3,Value::makeMap()); }
        | ACCESS STRING STRING expression hash_map
                { $$ = new AccessRule($2,$4,$5,$3); }
        |   ACCESS STRING STRING expression
                { $$ = new AccessRule($2,$4,Value::makeMap(), $3); }
        ;

permissions : permission
	   | permission permissions { $$ = $1; $1->next($2); }
	   ;

accesses : access
       | access accesses { $$ = $1; $1->next($2); }
       ;

include : INCLUDE STRING { std::string x = $2; MarsParser::include(x); }
		;


%%
#include "rulesl.c"
