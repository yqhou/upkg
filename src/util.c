#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"
/* **/

struct function_map_t functionMap[] = 
{
    {"DeleteReturnChars", 1, DeleteReturnChars, NULL },
	{"DeleteNotHexChars",  1, DeleteNotHexChars, NULL},
	{"ByteToAsciiHex", 1, ByteToAsciiHex, NULL },
	{"AsciiHexToByte",  1, AsciiHexToByte, NULL},
	{"BcdToByte",  1, BcdToByte, NULL},
	{"BcdToAsciiHex",  1, BcdToAsciiHex, NULL},
	{"ByteToBitmap",  1, ByteToBitmap, NULL},
	{"AsciiHexToBitmap",  1, AsciiHexToBitmap, NULL},
	{"ByteHexToIntEx",  1, ByteHexToIntEx, NULL},
	{"AsciiHexToIntEx",  1, AsciiHexToIntEx, NULL},
	{"ByteHexToInt",  2, NULL, ByteHexToInt},
	{"AsciiHexToInt",  2, NULL, AsciiHexToInt},
	{ "",0,NULL,NULL }
};

int GetConvertFunctionIdx( char *convertFunctionName, int type )
{
    int i  = 0;
    for( i = 0; ! StringIsNull(functionMap[i].functionName); i++ )
    {
        if( strcmp( convertFunctionName, functionMap[i].functionName ) == 0 && functionMap[i].type == type )
            return i;
    }
    return -1;
}


int IsHexChar( unsigned char c )
{
    if( c >= '0' && c <= '9' )
        return 1;
    else if ( c >= 'a' && c <= 'f' )
        return 1;
    else if ( c >= 'A' && c <= 'F' )
        return 1;
    else 
        return 0;
}

int DeleteNotHexChars(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)
{
    *ipOutLen = 0;
    int i = 0, j = 0;
    for( i=0; i<iInLen; i++ )
    {
        if( IsHexChar( (unsigned char)cpSrc[i]) )
        {
            cpDest[j++] = cpSrc[i];           
        }
    }
    cpDest[j] = 0;
    *ipOutLen = j;
    return 0;
}

int DeleteReturnChars(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)
{
    *ipOutLen = 0;
    int i = 0, j = 0;
    for( i=0; i<iInLen; i++ )
    {
        if( (unsigned char)cpSrc[i] != '\n' && (unsigned char)cpSrc[i] != '\r' )
        {
            cpDest[j++] = cpSrc[i];           
        }
    }
    cpDest[j] = 0;
    *ipOutLen = j;
    return 0;
}

/**
  ByteToAsciiHex 原值转换为16进制字符串
  0x12 0x34 ==> "1234"
**/
int ByteToAsciiHex(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)
{
    int i = 0;
    unsigned char c = '\0';

    *ipOutLen = 0;
    for (i = 0; i < iInLen; i++)
    {
        c = (unsigned char)cpSrc[i] >> 4;
        cpDest[i * 2] = (c > 9) ? (c - 10 + 'A') : (c + '0');
        c = (unsigned char)cpSrc[i] & 0x0f;
        cpDest[i * 2 + 1] = (c > 9) ? (c - 10 + 'A') : (c + '0');
    }
    cpDest[iInLen * 2] = '\0';
    *ipOutLen = iInLen * 2;

    return 0;
}

/***
AsciiHexToByte 16进制字符串转换成原值
"1234abcd" ==> 0x12 0x34 0xab 0xcd
***/
int AsciiHexToByte(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)
{
    int  i = 0;
    int  iLen = 0;
    unsigned char cTmp = 0;
    cpDest[0] = 0x00;

    *ipOutLen = 0;
    
    char *cpBuf = (char*)malloc(iInLen+5);
    memset( cpBuf, 0, sizeof(iInLen+5));
    DeleteNotHexChars( cpSrc, iInLen, cpBuf, &iLen);
    memcpy( cpSrc, cpBuf, iLen );
    free( cpBuf );
    for (i = 0; i < iLen; i++)
    {
        if (cpSrc[i] < 'A')
            cTmp =(unsigned char)cpSrc[i] - '0';
        else if (cpSrc[i] < 'a')
            cTmp = (unsigned char)cpSrc[i] - 'A' + 10;
        else
            cTmp = (unsigned char)cpSrc[i] - 'a' + 10;

        if (iLen % 2)
        {
            if (i % 2)
                cpDest[(i + 1) / 2] = (cTmp << 4);
            else
                cpDest[(i + 1) / 2] |= cTmp;
        }
        else
        {
            if (i % 2)
                cpDest[i / 2] |= cTmp;
            else
                cpDest[i / 2] = (cTmp << 4);
        }
    }
    *ipOutLen = (iLen + 1) / 2;
    return 0;
}

/*** BCD转换成原值
 0x12 0x34 ==> 0x31 0x32 0x33 0x34
***/
int BcdToByte( char *cpSrc, int iInLen, char *cpDest, int *ipOutLen )
{
    int i;
    unsigned char ch;
    for(i = 0; i < iInLen; i++)
    {
        ch = (unsigned char)cpSrc[i] >> 4;
        cpDest[i * 2] = (ch > 9) ? ch - 10 + 'A' : ch + '0';
        ch = (unsigned char)cpSrc[i] & 0x0f;
        cpDest[i * 2 + 1] = (ch > 9) ? ch - 10 + 'A' : ch + '0';
    }
    *ipOutLen = iInLen * 2;
    return 0;
}

/** 
0x12 0x34 ==> "31323334"
***/
int BcdToAsciiHex(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)
{
    int iLen = 0;
    char *cpBuf = (char*)malloc(iInLen*3);
    memset( cpBuf, 0, iInLen*3);
    BcdToByte( cpSrc, iInLen, cpBuf, &iLen);
    ByteToAsciiHex( cpBuf, iLen, cpDest, ipOutLen );
    free( cpBuf );
    return 0;
}

void CharToBit( char c, char *cpDest )
{
    cpDest[0] = (( c & 0x80 )>>7) + '0'; /** 高8位 **/
    cpDest[1] = (( c & 0x40 )>>6) + '0';
    cpDest[2] = (( c & 0x20 )>>5) + '0';
    cpDest[3] = (( c & 0x10 )>>4) + '0';
    cpDest[4] = (( c & 0x08 )>>3) + '0';
    cpDest[5] = (( c & 0x04 )>>2) + '0';
    cpDest[6] = (( c & 0x02 )>>1) + '0';
    cpDest[7] = (( c & 0x01 )) + '0';
}
int ByteToBitmap( char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)
{
    int i = 0;
    for( i = 0; i<iInLen; i++ )
    {
        char sBit[8+1];
        memset( sBit,0, sizeof(sBit) );
        CharToBit( cpSrc[i], sBit );
        strncat( cpDest, sBit, 8 );       
    }
    *ipOutLen = strlen(cpDest);
    return 0;
}

int AsciiHexToBitmap(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)
{
    int iLen = 0, i = 0;
    char *cpBuf = (char*)malloc(iInLen);
    AsciiHexToByte( cpSrc, iInLen, cpBuf, &iLen );
    ByteToBitmap( cpBuf, iLen, cpDest, ipOutLen );
    free( cpBuf );
    return 0;
}


/*** 0x00 0x0d ===> 214 ***/
int ByteHexToIntEx(char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)
{
    int iVal = 0;
    if( ByteHexToInt( cpSrc, iInLen, &iVal ) != 0 )
        return -1;
    sprintf( cpDest, "%d", iVal );
    *ipOutLen = strlen(cpDest);
    return 0;
}

/**** "000d" ==> 0x00 0x0d ==> 214; **/
int AsciiHexToIntEx( char *cpSrc, int iInLen, char *cpDest, int *ipOutLen)
{
    int iVal = 0;
    if ( AsciiHexToInt( cpSrc, iInLen, &iVal ) != 0 )
        return -1;
    sprintf( cpDest, "%d", iVal );
    *ipOutLen = strlen( cpDest );
    return 0;
}

/*** 0x00 0x0d ===> 214 ***/
int ByteHexToInt(char *cpSrc, int iInLen, int *ipVal)
{
    int  iVal = 0;
    int  i = 0;

    if (iInLen > sizeof(int))
    {
        return -1;
    }

    for (i = 0; i < iInLen; i++)
    {
        iVal = iVal << 8 | (unsigned char)cpSrc[i];
    }
    *ipVal = iVal;
    return 0;
}

/**** "000d" ==> 0x00 0x0d ==> 214; **/
int AsciiHexToInt(char *cpSrc, int iInLen, int *ipVal )
{
    int iLen = 0;
    char *cpBuf = (char*)malloc(iInLen);
    AsciiHexToByte( cpSrc, iInLen, cpBuf, &iLen );
    int ret = ByteHexToInt( cpBuf, iLen, ipVal );
    free( cpBuf );
    return ret;
} 

/*** 
0x12 0x34 ==> 1234 

**/
int BcdToInt(char *cpSrc, int iInLen, int *ipVal )
{
    int iLen = 0;
    char *cpBuf = (char*)malloc( iInLen * 3 );
    BcdToByte( cpSrc, iInLen, cpBuf, &iLen );
    *ipVal = atoi( cpBuf );
    free( cpBuf );
    return 0;
}

char *GetTime( char *stime ) {
    struct tm *p;
    struct timeb tb;
    int hour = 0;
    int min = 0;
    int sec = 0;

    ftime( &tb );
    p = localtime( &(tb.time) );

    hour = p->tm_hour;
    min = p->tm_min;
    sec = p->tm_sec;


    sprintf( stime, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            (1900+p->tm_year),
            (1+p->tm_mon),
            p->tm_mday,
            hour,
            min,
            sec,
            tb.millitm );

    return stime;
}

int userlog( int iLogLevel, char *logfile, const char *file, const char *function, int line, char *fmt, ... ) 
{
    va_list args;
    FILE    *fp;
    int i = 0;
    char tmp[1024];
    char alTime[25];
    char *path;


    memset(tmp, 0x00, sizeof(tmp) );
    memset( alTime, 0x00, sizeof(alTime) );

    if( iLogLevel == 0 )
        return 0;

    /**/
    GetTime( alTime );

    if ( (fp=fopen(logfile,"a+")) == NULL ) {
        return -1;
    }

    va_start( args, fmt );
    fprintf( fp, "[%s][%-10s-L%04d-%-15s-%05d] ", alTime+2, file, line,function, getpid() );
    vfprintf( fp, fmt, args );
    fprintf( fp, "%s", NEWLINE );
    va_end( args );
    fclose( fp );
    return 0;
}


void userloghex(int iLogLevel, char *logfile, const char *file, const char *function,int line, const unsigned char *buf, int len)
{
#define LINE_BLOCKS                     3                           /* 每行打印的块数 */
#define BLOCKE_BYTES                    8                           /* 每块打印的字节数 */
#define LINE_MAX_BUF_BYTES              (LINE_BLOCKS*BLOCKE_BYTES)  /* 每行最多打印的buf字节个数 */
    /* 原始数据显示启示位置 */
#define ORIG_DATA_START_POS             (5+LINE_MAX_BUF_BYTES*3+LINE_BLOCKS)
    /* 日志文件每行最大显示字节个数 */
#define LOG_MAX_LINE_LEN                (ORIG_DATA_START_POS+LINE_MAX_BUF_BYTES+3+1)

    /* 计算所需输出的行数 */
    int iLineCnt = 0;
    if (0 == len % LINE_MAX_BUF_BYTES)
    {
        iLineCnt = len / LINE_MAX_BUF_BYTES;
    }
    else
    {
        iLineCnt = len / LINE_MAX_BUF_BYTES + 1;
    }
    /* 首行及尾行 */
    iLineCnt += 2;
    /* 分配空间,用于最终写入文件 */
    char *pLogDataBuf = NULL;
    pLogDataBuf = (char *)calloc(iLineCnt*LOG_MAX_LINE_LEN, sizeof(char));

    /* 1. 获取待写入的日志信息 */
    sprintf(pLogDataBuf, "++++++++++++++ length[%d] ++++++++++++++\n", len);

    /* 入参buf的偏移量 */
    int     iBufOffset = 0;
    /* 当前行偏移字节数 */
    int     iLineCurrOffset = 0;
    /* 行首偏移量 */
    int     iLineStart = 0;
    /* 行数据 */
    char    caLineData[LOG_MAX_LINE_LEN+1] = {0};
    for (iBufOffset = 0; iBufOffset < len; iBufOffset++)
    {
        if (0 == iLineCurrOffset)
        {
            /* 获取行首偏移量 */
            iLineStart = iBufOffset;
            memset(caLineData, 0x00, sizeof(caLineData));
            sprintf(caLineData, "%04d:", iBufOffset);
        }
        /* 获取HEX数据 */
        char    caTmp[4+1] = {0};
        sprintf(caTmp, "%02X ", buf[iBufOffset]);
        strncat(caLineData, caTmp, strlen(caTmp));
        if (0 == (iLineCurrOffset+1) % BLOCKE_BYTES)
        {
            /* 每BLOCKE_BYTES字节添加间隔 */
            strncat(caLineData, " ", 1);
        }
        iLineCurrOffset++;

        if (LINE_MAX_BUF_BYTES == iLineCurrOffset ||                        /* 行尾 */
                LINE_MAX_BUF_BYTES != iLineCurrOffset && iBufOffset+1 == len)  /* 最后一行 */
        {
            int iLineBufBytes = 0;      /* 当前行所显示的buf字节数 */
            if (LINE_MAX_BUF_BYTES == iLineCurrOffset)
            {
                iLineBufBytes = LINE_MAX_BUF_BYTES;
            }
            else
            {
                iLineBufBytes = len % LINE_MAX_BUF_BYTES;
                size_t iPaddingLen = ORIG_DATA_START_POS - strlen(caLineData);
                if (iPaddingLen > 0)
                {
                    /* 填充数据 */
                    memset(&caLineData[strlen(caLineData)], 0x20, iPaddingLen);
                }
            }
            /* 获取源字符串进行显示 */
            int i = 0;
            for (i = 0; i < iLineBufBytes; i++)
            {
                if (isprint(buf[iLineStart+i]))
                {
                    strncat(caLineData, (const char *)&buf[iLineStart+i], 1);
                }
                else
                {
                    strncat(caLineData, ".", 1);
                }
                if (0 == (i+1) % BLOCKE_BYTES)
                {
                    /* 添加间隔 */
                    strncat(caLineData, " ", 1);
                }

            }
            strncat(caLineData, "\n", 1);
            strncat(pLogDataBuf, caLineData, strlen(caLineData));
            size_t iTmp = strlen(caLineData);
            iLineCurrOffset = 0;
        }
    }
    /* 添加分割线 */
    memset(caLineData, '-', LOG_MAX_LINE_LEN);
    strcat(pLogDataBuf, caLineData);

    /* 2. 写入文件 */
    userlog( iLogLevel, logfile, file, function, line, "%s", pLogDataBuf);
    /* 释放空间 */
    free(pLogDataBuf);
}

int trim( char *s )
{
    int i, l, r, len;
	
    for(len=0; s[len]; len++);
	if( len == 0 ) return 0;
    for(l=0; (s[l]==' ' || s[l]=='\t' || s[l]=='\n' || s[l] == '\r' ); l++);
    if(l==len)
    {
        s[0]='\0';
        return(0);
    }
    for(r=len-1; (s[r]==' ' || s[r]=='\t' || s[r]=='\n' || s[r] == '\r' ); r--);
    for(i=l; i<=r; i++) s[i-l]=s[i];
    s[r-l+1]='\0';
    return(0);
}

int StringIsNullOrWhiteSpace( char *cpStr )
{
    int i = 0;
    if( strlen( cpStr ) == 0 ) 
        return 1;
    while( cpStr[i] != 0 )
    {
        if( cpStr[i] != ' ' && cpStr[i] != '\t' && cpStr[i] != '\n' && cpStr[i] != '\r' )
            return 0;
    }
    return 1;
}
int StringIsNull( char *cpStr )
{
    if( strlen( cpStr ) == 0 || cpStr == NULL)
        return 1;
    return 0;
}
int StringIndexOf( char *cpString, char cSep )
{
    char *p = strchr( cpString, cSep );
    if( p == NULL )
        return -1;
    return p - cpString;
}

int StringLastIndexOf( char *cpString, char cSep )
{
    char *p = strrchr( cpString, cSep );
    if( p == NULL )
        return -1;
    return p - cpString;
}

/***
假定 cpValue 长度足够,n从1开始
***/
int GetListByDiv( char *cpBuf, char cSep, int n, char *cpValue )
{
    int idx = 0;
    int c = 0;
    if( cSep == 0 || n <= 0 || StringIsNull(cpBuf) )
    {
        *cpValue = 0;
        return 0;
    }

    idx = StringIndexOf( cpBuf, cSep );
    if( idx < 0 && n == 1)
    {
        memcpy( cpValue, cpBuf, strlen(cpBuf) );
        return strlen(cpBuf);
    }
    else if(idx == 0 && n == 1 )
    {
        *cpValue = 0;
        return 0;
    }    
    else if ( idx < 0 )
    {
        *cpValue = 0;
        return 0;
    }         
    if( n == 1 )
    {
        memcpy( cpValue, cpBuf, idx );
        return idx;
    }
    else
        return GetListByDiv( cpBuf+idx+1, cSep, n-1, cpValue );
}

/***
[NodeName] 忽略大小写
xxx
^
****/
FILE* GetIniFileNodePos( char *cpFile, char *cpNode, FILE *fp )
{
	char caReadBuf[2048];
	char caNodeName[128];
	fp = fopen( cpFile, "rb" );
	if( fp == NULL )
		return NULL;
	while( 1 )
	{
		memset( caReadBuf, 0, sizeof(caReadBuf) );
		if( fgets( caReadBuf, sizeof(caReadBuf)-1, fp ) == NULL )
			break;
		trim( caReadBuf );
		if( StringIsNull(caReadBuf) )
			continue;
		if( caReadBuf[0] != '[' )
			continue;
        memset( caNodeName, 0, sizeof(caNodeName) );
		memcpy( caNodeName, caReadBuf+1, strlen(caReadBuf)-2 );
		trim( caNodeName );
		if( strcasecmp( caNodeName, cpNode)== 0 )
			return fp;		
	}
	fclose( fp );
	return NULL;
}
/***
[NodeName]
cfgName = cpValue
**/
int GetIniConfig( char *cpFile, char *cpNode, char *cpCfgName, char *cpValue )
{
    FILE *fp;
    int idx = 0;
    char caCfgNameTmp[128];
    char caReadBuf[2048];
    /**LOG( 1, "upkg.log", "cpFile=[%s], cpNode=[%s], cpCfgName=[%s]",cpFile, cpNode, cpCfgName );**/
    fp = GetIniFileNodePos( cpFile, cpNode, fp );
    if( fp == NULL )
    {
        return 2;
    }
	
    while( 1 )
    {
        memset( caReadBuf, 0, sizeof(caReadBuf) );
        if( fgets( caReadBuf, sizeof(caReadBuf)-1, fp ) == NULL )
            break;
        trim( caReadBuf );
        if( StringIsNull( caReadBuf ) )
            continue;
        if( caReadBuf[0] == '#' )
            continue;
        if( caReadBuf[0] == '[' )
            break;
        idx = StringIndexOf( caReadBuf, '=' );
        if( idx <= 0 )
            continue;
		memset( caCfgNameTmp, 0, sizeof(caCfgNameTmp) );
        memcpy( caCfgNameTmp, caReadBuf, idx );
        trim( caCfgNameTmp );
        if( strcmp( caCfgNameTmp, cpCfgName ) == 0 )
        {
            if( *(caReadBuf+idx+1) == 0 )
				*cpValue = 0;
			else
			{             
                int len = strlen(caReadBuf), vLen = 0;
                int idx1 = StringIndexOf( caReadBuf+idx+1, '#' );
                if( idx1 >= 0 )
                    vLen = idx1;
                else
                    vLen = len - idx - 1;
                memcpy( cpValue, caReadBuf+idx+1, vLen );
                cpValue[vLen] = 0;
				trim( cpValue );
			}
            /**LOG( 1, "upkg.log", "cpValue=[%s]", cpValue );**/
			fclose( fp );
			return 0;
        }        
    }
    fclose( fp );
    return -1;
}