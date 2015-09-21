#include "upkg.h"
#include "list.h"
#include "util.h"
#include "errcode.h"

UpkgResultList* ReadFileToResultList( char *file )
{
    FILE *fp;
    UpkgResultList *url = NULL, *tail = NULL;
    char caReadBuf[10240+1];
    if( StringIsNull(file) )
        return NULL;
    fp = fopen( file, "rb" );
    if( fp == NULL )
    {
        printf( "open %s fail, %s\n", file, strerror(errno) );
        return NULL;
    }
    while( !feof(fp) )
    {
        memset( caReadBuf, 0, sizeof(caReadBuf) );
        fgets( caReadBuf, sizeof(caReadBuf)-1, fp );
        char caBuf[128+1];
        memset( caBuf, 0, sizeof(caBuf) );
        trim( caReadBuf );
        if( StringIsNull(caReadBuf) )
            continue;
        GetListByDiv( caReadBuf, '|', 1, caBuf, sizeof(caBuf)-1 );
        trim( caBuf );
        if( ! StringIsDigit(caBuf) )
            continue;
        UpkgResultList *up = NewUpkgResultList();
        up->fieldId = atoi( caBuf );
        if( SearchResultListByFieldId(url, up->fieldId) != NULL )
            continue;
        GetListByDiv( caReadBuf, '|', 2, up->name, sizeof(up->name)-1 );
        trim( up->name );
        GetListByDiv( caReadBuf, '|', 3, up->value, sizeof(up->value)-1 );
        trim( up->value );
        if( url == NULL )
        {
            url = up;
            tail = up;
        }
        else
        {
            tail->next = up;
            tail = up;
        }
    }
    fclose( fp );
    return url;
}

int PkgFixed(UpkgDef *ud, char *file, char *cpOut, int maxLen )
{
    if( ud == NULL )
        return FAILED;
    if( StringIsNull(file) )
        return FAILED;
    FILE *fp;
    if( ud->pkgDef->ePkgType != FIXED )
    {
        sprintf( ud->err, "报文类型不是定长,[%s]\0", ud->pkgDef->pkgType );
        return FAILED;
    }
    UpkgResultList *url = ReadFileToResultList( file );
    if( url == NULL )
        return FAILED;
    UpkgFldDefList *ufdl;
    for( ufdl=ud->fieldHead; ufdl; ufdl=ufdl->next )
    {
        UpkgResultList *up = SearchResultListByFieldId( url, ufdl->fieldId );
        if( up == NULL )
            StrcatSpace( cpOut, ufdl->fieldLength );
        else
        {
            int vLen = strlen( up->value );
            strncat( cpOut, up->value, MIN(vLen, ufdl->fieldLength) );
            if( ufdl->fieldLength > vLen )
                StrcatSpace( cpOut, ufdl->fieldLength - vLen );
        }
        if( strlen(cpOut) >= maxLen )
            break;          
    }
    DeleteUpkgResultList( url );
    return SUCCESS;
}

int Pkg( char *pkgFile, char *bufFile, char *cpOut, int maxLen )
{
    UpkgDef *ud = NewUpkgDef();
    int ret = LoadPkgFile( pkgFile, ud );
    if( ret != SUCCESS )
        return ret;
    if( ud->pkgDef->ePkgType == FIXED )
        ret = PkgFixed( ud, bufFile, cpOut, maxLen );
    if( ret != SUCCESS )
        printf( "%s\n", ud->err );
    DeleteUpkgDef( ud );
    return ret;
}