%option noyywrap

%{
#undef _POSIX_SOURCE
#define _POSIX_SOURCE
#define YY_PROTO(proto) proto
#define YY_DECL char **yylex YY_PROTO(( void ))
int _numargs = 10;
char *_args[10];
int _argcount = 0;
int readInputForLexer(char *buffer, int *numBytesRead, int maxBytesToRead);

#include <stdio.h>
int fileno(FILE *);
#include <stdlib.h>
#include <string.h>

#undef YY_INPUT
#define YY_INPUT(b,r,s) readInputForLexer(b,&r,s)
%}

WORD	[a-zA-Z0-9\/\.\-\=]+
SPECIAL	[()><|&;*]

%%
	_argcount = 0; 
	_args[0] = NULL; 

{WORD}|{SPECIAL} {  
	  if(_argcount < _numargs-1) {
	    _args[_argcount++] = (char *)strdup(yytext);
	    _args[_argcount] = NULL;
	  }
	}

\n	return _args;

[ \t]+

.

%%

char **get_line(void) {
  return (char **)yylex();
}

int readInputForLexer(char *buffer, int *numBytesRead, int maxBytesToRead)
{
    do {
        int c = getc(stdin);
        if (c == EOF) {
            /* EOF indicates either EOF or error */
            if (feof(stdin)) {
                *numBytesRead = YY_NULL;
                return 0;
            }
            /* Must have been a signal interrupting us */
            clearerr(stdin);
            
        } else {
            buffer[0] = c;
            *numBytesRead = 1;
            return 0;
        }
    } while(1);
}
