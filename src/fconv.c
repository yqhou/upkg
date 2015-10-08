#include <stdio.h>
#include "util.h"

typedef struct StFconvSep2FixedConfig
{
    int id;
    int length;
    struct StFconvSep2FixedConfig *next;
} FconvSep2FixedConfig;

FconvSep2FixedConfig* NewFconvSep2FixedConfig()
{
    FconvSep2FixedConfig *p = (FconvSep2FixedConfig*)malloc(sizeof(FconvSep2FixedConfig));
    memset( p, 0, sizeof(FconvSep2FixedConfig));
    p->id = 0;
    p->length = 0;
    p->next = NULL;
    return p;
}
void DeleteFconvSep2FixedConfig( FconvSep2FixedConfig *fconvSep2FixedConfig  )
{
    FconvSep2FixedConfig *p, *q;
    p = fconvSep2FixedConfig;
    while( p )
    {
        q = p->next;
        free( p );
        p = q;
    }
}

FconvSep2FixedConfig* LoadSep2FixedConfig( char *cfgFile )
{
    FconvSep2FixedConfig *fconvSep2FixedConfig = NULL, *tail = NULL;
    if( StringIsNull(cfgFile) )
        return NULL;
    FILE *fp = fopen( cfgFile, "rb" );
    if( fp == NULL )
    {
        printf( "open %s fail, %s\n", cfgFile, strerror(errno) );
        return NULL;
    }
   char caBuf[128], caReadBuf[512];
   int line = 0;
    while( ! feof( fp ) )
    {
        memset( caReadBuf, 0, sizeof(caReadBuf) );
        memset( caBuf, 0, sizeof(caBuf) );
        fgets( caReadBuf,  sizeof(caReadBuf)-1, fp );
        trim( caReadBuf );
        line ++;
        if( StringIsNull(caReadBuf) )
            continue;
        if( caReadBuf[0] == '#' )
            continue;
        GetListByDiv( caReadBuf, '|', 1, caBuf, sizeof(caBuf)-1 );
        if( StringIsNull(caBuf) || ! StringIsDigit(caBuf) )
        {
            printf( "line %d,[%s]不正确\n", line, caReadBuf );
            continue;   
        }
        FconvSep2FixedConfig *p = NewFconvSep2FixedConfig();
        p->id = atoi( caBuf );
        memset( caBuf, 0, sizeof(caBuf) );
        GetListByDiv( caReadBuf, '|', 2, caBuf, sizeof(caBuf)-1 );
        if( StringIsNull(caBuf) || ! StringIsDigit(caBuf) )
        {
            printf( "line %d,[%s]不正确\n", line, caReadBuf );
            DeleteFconvSep2FixedConfig( p );
            continue;   
        }  
        p->length = atoi( caBuf );
        if( fconvSep2FixedConfig == NULL )
        {
            fconvSep2FixedConfig = p;
            tail = p;
        }    
        else
        {
            tail->next = p;
            tail = p;
        }
    }
    fclose( fp );
    return fconvSep2FixedConfig;
}

int SepStr2FixedStr( FconvSep2FixedConfig *fsfc, char cSep, char *fromStr, char *toStr, int maxLen )
{
    FconvSep2FixedConfig *p;
    char caBuf[1024];
    for( p=fsfc; p; p=p->next )
    {
        memset( caBuf, 0, sizeof(caBuf) );
        GetListByDiv( fromStr, cSep, p->id, caBuf, sizeof(caBuf)-1 );
        strncat( toStr, caBuf, MIN(strlen(caBuf), p->length) );
        if( p->length > strlen(caBuf) )
            StrcatSpace( toStr, p->length - strlen(caBuf) );
    }
    return 0;
}
int Sep2Fixed( char cSep, char *cfgFile, char *fromFile, char *toFile )
{
    FconvSep2FixedConfig *fconvSep2FixedConfig = LoadSep2FixedConfig( cfgFile );
    if( fconvSep2FixedConfig == NULL )
        return -1;
    FILE *toFp, *fromFp;
    char caFromStr[4096], caToStr[4096];
    toFp = fopen( toFile, "wb" );
    if( toFp == NULL )
    {
        printf( "open %s fail, %s\n", toFile, strerror(errno) );
        return -2;
    }
    fromFp = fopen( fromFile, "rb" );
    if( fromFp == NULL )
    {
        printf( "open %s fail, %s\n", fromFile, strerror(errno) );
        return -3;
    }
    while( !feof( fromFp ) )
    {
        memset( caFromStr, 0, sizeof(caFromStr) );
        memset( caToStr, 0, sizeof(caToStr) );
        fgets( caFromStr, sizeof(caFromStr)-1, fromFp );
        trim( caFromStr );
        if( StringIsNull( caFromStr ) )
            continue;
        SepStr2FixedStr( fconvSep2FixedConfig, cSep, caFromStr,  caToStr, sizeof(caToStr)-1 );
        fprintf( toFp, "%s\n", caToStr );
    }
    fclose( fromFp );
    fclose( toFp );
    return 0;
}

int Main1()
{
    char fromFile[512], toFile[512], cfgFile[512], cSep;
    printf( "配置文件: " );
    fgets( cfgFile, sizeof(cfgFile)-1, stdin );
    trim( cfgFile );
    printf( "源文件: " );
    fgets( fromFile, sizeof(fromFile)-1, stdin );
    trim( fromFile );
    printf( "目的文件: " );
    fgets( toFile, sizeof(toFile)-1, stdin );
    trim( toFile );
    /*
    printf( "分隔符: " );
    cSep = getchar();
    fflush( stdin );
    */
    cSep = ',';
    int ret = Sep2Fixed( cSep, cfgFile, fromFile, toFile );
    return ret;
}
int main( int argc, char **argv )
{
    int ret = 0;
    printf( "分隔符文件转换为定长格式\n支持命令行. %s 配置文件名 源文件名 目的文件名 源文件分隔符", argv[0] );
    if( argc == 4 )
    {
        ret = Sep2Fixed( ',', argv[1], argv[2], argv[3] );
        printf( "ret = %d\n", ret );
    }
    else
    {
        ret = Main1();
        printf( "ret = %d\n", ret );
        system( "pause" );
    }
}