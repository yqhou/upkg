#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>

#define LOG( logLevel, logfile, ...) userlog(logLevel, logfile,__FILE__,__FUNCTION__, __LINE__, __VA_ARGS__)
#define LOGHEX( logLevel, logfile,buf, len) userloghex(logLevel, logfile, __FILE__, __FUNCTION__, __LINE__, buf,  len)

#define NEWLINE "\n"


struct function_map_t
{
	char functionName[32+1];
	int type; /* 1-ConvertStringFunction 2-ConvertLengthFunction */	
	int (*ConvertStringFunction)( char *, int, char *, int * );
	int (*ConvertLengthFunction)(char*, int, int *);
};

extern struct function_map_t functionMap[];

int GetConvertFunctionIdx( char *convertFunctionName, int type );   /* 1-ConvertStringFunction 2-ConvertLengthFunction */                   
int IsHexChar( unsigned char c )                                                                              ;
int DeleteNotHexChars(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)                          ;
int ByteToAsciiHex(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)                             ;
int AsciiHexToByte(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)                             ;
int BcdToByte( char *cpSrc, int iInLen, char *cpDest, int *ipOutLen )                                ;
int BcdToAsciiHex(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)                              ;
void CharToBit( char c, char *cpDest )                                                               ;
int ByteToBitmap( char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)                              ;
int AsciiHexToBitmap(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)                           ;
int ByteHexToIntEx(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)                             ;
int AsciiHexToIntEx( char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)                           ;
int ByteHexToInt(char *cpSrc, int iInLen, int *ipVal)                                                ;
int AsciiHexToInt(char *cpSrc, int iInLen, int *ipVal )                                              ;
int BcdToInt(char *cpSrc, int iInLen, int *ipVal )                                                   ;
int trim( char *s )                                                                                  ;
int StringIndexOf( char *cpString, char cSep )                                                       ;
int StringLastIndexOf( char *cpString, char cSep )                                                   ;
int GetListByDiv( char *cpBuf, char cSep, int n, char *cpValue )                                     ;
int GetIniConfig( char *cpFile, char *cpNode, char *cpCfgName, char *cpValue )                       ;
FILE* GetIniFileNodePos( char *cpFile, char *cpNode, FILE *fp )                                      ;


#endif