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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#define YY_INPUT(buf,result,max_size) \
  { \
	int c = fgetc(yyin); \
	result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
   }

#endif

static long long ten24(int n)
{
	long long x = 1;
	while(n-->0) { x *= 1024; }
	return x;
}


%}
IDENT   [_A-Za-z]+[_0-9A-Za-z]*
NUMB    [0-9]*


%%
"accept"     return ACCEPT;
"acl"        return ACL;
"axisOf"     return AXIS;
"cluster"    return NAME;
"contains"   return CONTAINS;
"default"    return DEFAULT;
"else"       return ELSE;
"end"        return END;
"fail"       return FAIL;
"function"   return FUNCTION;
"hookOf"     return HOOK;
"if"         return IF;
"in"         return IN;
"not"        return NOT;
"include"    return INCLUDE;
"insert"     return INSERT;
"limit"      return LIMIT;
"access"     return ACCESS;
"schedule"   return SCHEDULE;
"name"       return NAME;
"node"       return NODE;
"note"       return NOTE;
"otherwise"  return OTHERWISE;
"permission" return PERMISSION;
"pop"        return POP;
"print"      return PRINT;
"priority"   return PRIORITY;
"push"       return PUSH;
"select"     return SELECT;
"shape"      return SHAPE;
"then"       return THEN;
"when"       return WHEN;
"match"      return MATCH;

"message"    return MESSAGE;
"warning"    return WARNING;
"error"      return ERROR;

"Hb"         { yylval.num = ten24(5); return UNIT; }
"Pb"         { yylval.num = ten24(5); return UNIT; }
"Tb"         { yylval.num = ten24(4); return UNIT; }
"Gb"         { yylval.num = ten24(3); return UNIT; }
"Mb"         { yylval.num = ten24(2); return UNIT; }
"Kb"         { yylval.num = ten24(1); return UNIT; }
"byte"       { yylval.num = ten24(0); return UNIT; }
"bytes"      { yylval.num = ten24(0); return UNIT; }


\"|\'  {
           int c,q = yytext[0];

           yyleng = 0;

           while((c = yyinput()) && c != q && c != '\n')
           {
               if(c == '\\') yytext[yyleng++] = yyinput();
               else yytext[yyleng++] =  c;
            }

            yytext[yyleng++] = 0;
            yylval.val = (const char *)yytext;
            return STRING;
        }

{IDENT}   { yylval.val = (const char *)yytext; /*printf("---%s---\n", yytext);*/ return STRING; }
{NUMB} 	  { yylval.num = Translator<std::string,long long>()(yytext); return INTEGER; }
"-"{NUMB} { yylval.num = Translator<std::string,long long>()(yytext); return INTEGER; }

\#      {
           int c;
           while((c = yyinput()) && (c != '\n'))
                    ;
        }
[ \t]*  ;
\n      ; // TODO: FIXME: lineno++;

"=="      return EQ;
"!="      return NE;
">="      return GE;
"<="      return LE;
"&&"      return AND;
"||"      return OR;
"!"       return NOT;

.		return *yytext;
%%
