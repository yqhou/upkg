#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "list.h"
#include "upkg.h"

void ReadBufFromFile(  char *cpBuf, int maxLen )
{
    char caFile[512+1];
    char caReadBuf[512+1];
    FILE *fp;
    printf( "input filename: " );
    memset( caFile, 0, sizeof(caFile) );
    fgets( caFile, sizeof(caFile)-1, stdin );
    trim( caFile );
    fflush( stdin );  
    fp = fopen( caFile, "rb" );
    if ( ! fp )
    {
        printf( "open %s fail. %s\n", caFile, strerror(errno) );
        return;
    }
    while( !feof( fp ) )
    {
        memset( caReadBuf, 0, sizeof(caReadBuf) );
        fgets( caReadBuf, sizeof(caReadBuf)-1, fp );
        strncat( cpBuf, caReadBuf, MIN( maxLen-strlen(cpBuf), strlen(caReadBuf) ) );
    }  
    fclose( fp );
}

void ReadBufFromStdin( char *cpBuf, int maxLen )
{
    char caReadBuf[513];
    printf( "input package(end with Ctrl-Z): \n" );

    while( !feof(stdin) )
    {
        memset( caReadBuf, 0, sizeof(caReadBuf) );
        fgets( caReadBuf, sizeof(caReadBuf)-1, stdin );
        strcat( cpBuf, caReadBuf );
    }
}

int main(int argc, char **argv )
{
    int iChoice = 0;
    int ret = 0;
    char caOut[20480], caIn[10240], caReadBuf[6144];
    char pkgFile[512+1];
    
    printf( "input pkgfile: " );
    memset( pkgFile, 0, sizeof(pkgFile) );
    fgets( pkgFile, sizeof(pkgFile)-1, stdin );
    trim( pkgFile );
    fflush( stdin );
    memset( caIn, 0, sizeof(caIn) );
    memset( caOut, 0, sizeof(caOut) );
    printf( "1. read from file.\n2. read from stdin\nchoice: " );
    scanf( "%d", &iChoice );
    fflush( stdin );
    switch( iChoice )
    {
        case 1:
            ReadBufFromFile( caIn, sizeof(caIn)-1 );
            break;
        case 2:
            ReadBufFromStdin( caIn, sizeof(caIn)-1 );
            break;
        default:
            printf( "wrong choice.[%d]\n", iChoice );
            break;
    }
    ret = Unpack( caIn, strlen(caIn), pkgFile, caOut );
    printf( "\nOUT:\n%s\n", caOut );
    return ret;
}