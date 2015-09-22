#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "list.h"
#include "upkg.h"

int ProcNoArg()
{
    int iChoice = 0;
    int ret = 0;
    char caOut[20480];
    char pkgFile[512+1], bufFile[512+1], outfile[512+1];
    
    printf( "input pkgfile: " );
    memset( pkgFile, 0, sizeof(pkgFile) );
    fgets( pkgFile, sizeof(pkgFile)-1, stdin );
    trim( pkgFile );
    fflush( stdin );
    memset( caOut, 0, sizeof(caOut) );
    printf( "input bufFile: " );
    memset( bufFile, 0, sizeof(bufFile) );
    fgets( bufFile, sizeof(bufFile)-1, stdin );
    trim( bufFile);
    fflush( stdin );
    printf( "input outfile: " );
    memset( outfile, 0, sizeof(outfile) );
    fgets( outfile, sizeof(outfile)-1, stdin );
    trim( outfile );
    fflush( stdin );
    FILE *fp = fopen( outfile, "wb" );
    if( fp == NULL )
    {
        printf( "oepn %s fail, %s\n", outfile, strerror(errno) );
        return -1;
    }
    ret = Pkg( pkgFile, bufFile, caOut, sizeof(caOut)-1 );
    fputs( caOut, fp );
    fclose( fp );
    printf( "\nOUT:\n%s\n", caOut );
    
    system( "pause" );
    return ret;
}

/* argv[0] pkgfile buffile outfile */
int main( int argc, char **argv )
{
    if( argc != 4 )
        ProcNoArg();
    else
    {
        printf( "start from args... [%s] [%s] [%s] [%s]", argv[0], argv[1], argv[2], argv[3] );
        char caOut[20480];
        memset( caOut, 0, sizeof(caOut) );
        FILE *fp = fopen( argv[3], "wb" );
        if( fp == NULL )
        {
            printf( "oepn %s fail, %s\n", argv[3], strerror(errno) );
            return -1;
        }
        int ret = Pkg( argv[1], argv[2], caOut, sizeof(caOut)-1 );
        fputs( caOut, fp );
        fclose( fp );
        printf( "ret = %d\nOUT:\n%s\n", ret, caOut );   
        system( "pause" );
    }
    return 0;
}