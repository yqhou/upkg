#include "list.h"
#include "upkg.h"
#include "util.h"
/*** Upkg Result **/
UpkgResultList *NewUpkgResultList()
{
    UpkgResultList *url = (UpkgResultList*)malloc( sizeof(UpkgResultList) );
    memset( url, 0, sizeof(UpkgResultList) );
    return url;
}

void DeleteUpkgResultList(UpkgResultList *ur)
{
    if( ur == NULL )
        return;      
    UpkgResultList *p, *q;
    p = ur;
    while ( p )
    {
        q = p->next;
        if( p->subfld != NULL )
            DeleteUpkgResultList( p->subfld );
        free( p );
        p = q;
    }
}

UpkgResultList* CopyUpkgResultList( UpkgResultList *urFrom )
{
    if( urFrom == NULL ) return NULL;
    UpkgResultList* urTo = NewUpkgResultList();
    UpkgResultList *p, *tail = urTo;
    memcpy( urTo, urFrom, sizeof(UpkgResultList) );
    for( p=urFrom->next; p; p=p->next )
    {
        UpkgResultList *q = NewUpkgResultList();
        memcpy( q, p, sizeof(UpkgResultList) );
        tail->next = q;
        tail = q;
    }
    return urTo;
}
void GenSpace( int n, char *cpSpace )
{
    int i = 0;
    for( i=0; i<n; i++ )
        strcat( cpSpace, " " );
    cpSpace[n] = 0;
}

void OneUpkgResultListToString( UpkgResultList *p, char *cpOut, int depth )
{ 
    char caSpace[64+1];
    if( depth < 0 ) depth = 0;
    memset( caSpace, 0, sizeof(caSpace) );
    GenSpace( depth * 4, caSpace );
    if( ! StringIsNull(p->comment) )
    {
        if( StringIsNull(p->nameZh) )
            sprintf( cpOut, "%s[域 %03d.%-25s][长度 %03d][%s]    (%s)\n", caSpace, p->fieldId, p->name, p->length, p->value, p->comment );
        else
            sprintf( cpOut, "%s[域 %03d.%4s.%-20s][长度 %03d][%s]    (%s)\n", caSpace, p->fieldId, p->name, p->nameZh, p->length, p->value, p->comment );
    }
    else
    {
        if( StringIsNull(p->nameZh) )
            sprintf( cpOut, "%s[域 %03d.%-25s][长度 %03d][%s]\n", caSpace, p->fieldId, p->name, p->length, p->value );
        else
            sprintf( cpOut, "%s[域 %03d.%4s.%-20s][长度 %03d][%s]\n", caSpace, p->fieldId, p->name, p->nameZh, p->length, p->value );        
    }
}

void UpkgResultListToIndentString( UpkgResultList *ur, char *cpOut, int depth )
{
    UpkgResultList *p;
    for( p=ur; p; p=p->next )
    {
        char caTemp[2048+1];
        memset( caTemp, 0, sizeof(caTemp) );
        OneUpkgResultListToString( p, caTemp, depth );
        strcat( cpOut, caTemp );
        if( p->subfld != NULL )
        {
            char caSubFld[10241];
            memset( caSubFld, 0, sizeof(caSubFld) );
            UpkgResultListToIndentString( p->subfld, caSubFld, depth+1 );
            strcat( cpOut, caSubFld );
        }
    }
}
void UpkgResultListToString( UpkgResultList *ur, char *cpOut )
{
    UpkgResultListToIndentString( ur, cpOut, 0 );
}
void PrintUpkgResultList( UpkgResultList *ur )
{
    char caOut[20480];
    memset( caOut, 0, sizeof(caOut) );
    UpkgResultListToString( ur, caOut );
    printf( "\n%s\n", caOut );
}

/*** Field ***/
UpkgFldDefList *NewUpkgFldDefList()
{
    UpkgFldDefList *ufdn = (UpkgFldDefList*)malloc(sizeof(UpkgFldDefList) );
    memset( ufdn, 0, sizeof(UpkgFldDefList) );
    ufdn->sepChar1 = 0;
    ufdn->sepChar2 = 0;
    return ufdn;
}
void DeleteUpkgFldDefList(UpkgFldDefList *ufdn )
{
    UpkgFldDefList *p, *q;
    p = ufdn;
    while( p )
    {
        q = p->next;
        free( p );
        p = q;
    }
}
UpkgFldDefList *SearchFieldListByFieldId( UpkgFldDefList* head, int fieldId )
{
    UpkgFldDefList *p;
    for( p=head; p; p=p->next )
    {
        if( p->fieldId == fieldId )
            return p;
    }
    return NULL;
}

void UpkgFldDefToString( UpkgFldDefList *ufdl, char *cpOut, int fieldId )
{
    UpkgFldDefList *p = SearchFieldListByFieldId( ufdl, fieldId );
    sprintf( cpOut, "%s-------- fieldId   = [%d]--------\n", cpOut,  p->fieldId );
    sprintf( cpOut, "%sfieldName          = [%s]\n", cpOut,  p->fieldName );
    sprintf( cpOut, "%sfieldType          = [%s]\n", cpOut,  p->fieldType );
    sprintf( cpOut, "%seFieldType         = [%d]\n", cpOut,  p->eFieldType );
    sprintf( cpOut, "%ssubPkgFile         = [%s]\n", cpOut,  p->subPkgFile );
    sprintf( cpOut, "%sfieldLengthType    = [%s]\n", cpOut,  p->fieldLengthType );
    sprintf( cpOut, "%seFieldLengthType   = [%d]\n", cpOut,  p->eFieldLengthType );
    sprintf( cpOut, "%sfieldLength        = [%d]\n", cpOut,  p->fieldLength );
    sprintf( cpOut, "%sfieldLengthCoding  = [%s]\n", cpOut,  p->fieldLengthCoding );
    sprintf( cpOut, "%seFieldLengthCoding = [%d]\n", cpOut,  p->eFieldLengthCoding );
    sprintf( cpOut, "%sfieldCoding        = [%s]\n", cpOut,  p->fieldCoding );
    sprintf( cpOut, "%seFieldCoding       = [%d]\n", cpOut,  p->eFieldCoding );
    sprintf( cpOut, "%sfieldFmt           = [%s]\n", cpOut,  p->fieldFmt );
    sprintf( cpOut, "%sfieldLengthFmt     = [%s]\n", cpOut,  p->fieldLengthFmt );
    sprintf( cpOut, "%scomment            = [%s]\n", cpOut,  p->comment );
    sprintf( cpOut, "%ssepChar1           = [%c]\n", cpOut,  p->sepChar1 );
    sprintf( cpOut, "%ssepChar1           = [%c]\n", cpOut,  p->sepChar1 );
    sprintf( cpOut, "%s-------- fieldId   = [%d] END----\n", cpOut,  p->fieldId );
}

void UpkgFldDefListToString( UpkgFldDefList *ufdl, char *cpOut )
{
    char caTemp[2048];
    UpkgFldDefList *p;
    for( p=ufdl; p; p=p->next )
    {
        UpkgFldDefToString( p, caTemp, p->fieldId );
        strcat( cpOut, caTemp );
    }
}
/*** pkgDefNode ***/
UpkgPkgDefNode *NewUpkgPkgDefNode()
{
    UpkgPkgDefNode *updn = (UpkgPkgDefNode*)malloc(sizeof(UpkgPkgDefNode) );
    memset( updn, 0, sizeof(UpkgPkgDefNode) );
    return updn;
}

void DeleteUpkgPkgDefNode( UpkgPkgDefNode *updn )
{
    free( updn );
}


/*** UpkgDef ***/
UpkgDef *NewUpkgDef()
{
    UpkgDef *ud = (UpkgDef*)malloc(sizeof(UpkgDef) );
    memset( ud, 0, sizeof(UpkgDef) );
    ud->pkgDef = NewUpkgPkgDefNode();
    return ud;
}

void DeleteUpkgDef( UpkgDef* ud)
{
    DeleteUpkgPkgDefNode( ud->pkgDef );
    DeleteUpkgFldDefList( ud->fieldHead );
    DeleteUpkgResultList( ud->result );
    free( ud );
}

