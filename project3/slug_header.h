/* Justin Kwok, Ian Hamilton, Yugraj Singh, Ben Lieu, Greg Arnheiter */
/* Created 5/20/14 */

#define FUNCTIONIZE(a,b)    a(b)
#define STRINGIZE(a)        #a
#define INT2STRING(i)       FUNCTIONIZE(STRINGIZE, i)
#define FILE_POS            __FILE__ ":" INT2STRING(__LINE__)

#define malloc(s) slug_malloc((s), FILE_POS)
#define free(s) slug_free((s), FILE_POS)