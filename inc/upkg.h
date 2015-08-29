#ifndef __UPKG_H__
#define __UPKG_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

typedef enum EnumCodingType
{
    BYTE = 0,
    BCD ,
    ASCIIHEX
}CodingType;

typedef enum EnumFieldType
{
    BITMAP = 0,
    COMMON,
    SUBFIELD,
    LOOPBEGIN,
    LOOPEND,
    LOOPTIMES
} FieldType;

typedef enum EnumLengthType
{
    VAR = 0,
    LLVAR = 2,
    LLLVAR = 3
} LengthType;

typedef enum EnumPkgType
{
    ISO8583 = 1,
    FIXED ,
    TLV ,
    SEP 
} PkgType;

typedef struct upkg_result_list_t
{
    int fieldId;
    char name[128+1];
    char nameZh[128+1];
    char value[1024+1];
    char comment[128+1];
    int length;
    struct upkg_result_list_t *subfld; 
    struct upkg_result_list_t *next;
} UpkgResultList;

typedef struct upkg_field_def_list
{
    int fieldId;
    char fieldName[128+1];
    char fieldType[32+1];
    FieldType eFieldType;
    char subPkgFile[512+1];
    char fieldLengthType[32+1];
    LengthType eFieldLengthType;
    int fieldLength;
    char fieldLengthCoding[32+1];
    CodingType eFieldLengthCoding;
    char fieldCoding[32+1];
    CodingType eFieldCoding;
    char fieldFmt[64+1];
    char fieldLengthFmt[64+1];
    char comment[128+1];
    char sepChar1;
    char sepChar2;
    struct upkg_field_def_list *next;
} UpkgFldDefList;

typedef struct upkg_pkgdef_node
{
    char pkgId[32+1];
    char pkgName[128+1];
    char pkgType[16+1];
    PkgType ePkgType;
    char pkgMsgFmt[32+1];
    int fieldCount;
    char fieldList[1024+1];   
} UpkgPkgDefNode;

typedef struct upkg_def
{
    char pkgFile[512+1];
    char orgMsg[4096+1];
    char package[4096+1];
    int  packageLen;
    char bitmap[256+1];
    char err[512+1];
    int iLogLevel;
    char logFile[512+1];
    UpkgPkgDefNode *pkgDef;
    UpkgFldDefList *fieldHead;
    UpkgFldDefList *fieldTail;
    UpkgResultList *result;
    UpkgResultList *resultTail;
} UpkgDef;


int UpkgMain( UpkgDef *ud )    ;
int Unpack( char *cpMsg, int iInLen, char *pkgFile, char *cpOut );
#endif