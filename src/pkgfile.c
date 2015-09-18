#include "upkg.h"
#include "list.h"
#include "util.h"
#include "errcode.h"


FieldType ConvertFieldType( char *cpSrc )
{
    if( strcasecmp( cpSrc, "BITMAP") == 0 )
        return BITMAP;
     else if( strcasecmp( cpSrc, "COMMON") == 0 )
         return COMMON;
     else if( strcasecmp( cpSrc, "SUBFIELD") == 0 )
         return SUBFIELD;
#ifdef LOOPSUPPORT
     else if( strcasecmp( cpSrc, "LOOPBEGIN") == 0 )
         return LOOPBEGIN;
     else if( strcasecmp( cpSrc, "LOOPEND") == 0 )
         return LOOPEND;
     else if( strcasecmp( cpSrc, "LOOPTIMES") == 0 )
         return LOOPTIMES;
#endif         
     return -1;
}
CodingType ConvertCodingType( char *cpSrc )
{
     if( strcasecmp( cpSrc, "BYTE") == 0 )
         return BYTE;
     else if( strcasecmp( cpSrc, "BCD") == 0 )
         return BCD;
     else if( strcasecmp( cpSrc, "ASCIIHEX") == 0 )
         return ASCIIHEX;
     return -1;
}
LengthType ConvertLengthType( char *cpSrc )
{
    if( strcasecmp( cpSrc, "VAR") == 0 )
        return VAR;
    else if( strcasecmp( cpSrc, "LLVAR") == 0 )
        return LLVAR;
    else if( strcasecmp( cpSrc, "LLLVAR") == 0 )
        return LLLVAR;
    return -1;
}   
PkgType ConvertPkgType( char *cpSrc )
{
    if( strcasecmp( cpSrc, "8583") == 0 )
        return ISO8583;
    else if( strcasecmp( cpSrc, "FIXED") == 0 )
        return FIXED;
    else if( strcasecmp( cpSrc, "TLV") == 0 )
        return TLV;
#ifdef SEPSUPPORT        
    else if( strcasecmp( cpSrc, "SEP") == 0 )
        return SEP;
#endif        
    return -1;
}

FileType ConvertEnumFileType( char *cpSrc )
{
    if( strcasecmp( cpSrc, "INI") == 0 )
        return FileTypeIni;
    else if ( strcasecmp( cpSrc, "SEP" ) == 0 )
        return FileTypeSep;
    else
        return -1;
}

int LoadPkgDef( char *pkgFile, UpkgDef *ud )
{
    int ret;
    if( StringIsNullOrWhiteSpace(pkgFile) )
        return PKGFILEISNULL;
    if( ud == NULL )
        return UPKGDEFISNULL;
    char caBuf[128];
    memset( caBuf, 0, sizeof(caBuf) );
    GetIniConfig( pkgFile, "PKGDEF", "logLevel", caBuf, sizeof(caBuf)-1 );
    if( StringIsNull(caBuf) )
        ud->iLogLevel = 0;
    else
        ud->iLogLevel = atoi( caBuf );
    
    GetIniConfig( pkgFile, "PKGDEF", "logFile", ud->logFile, sizeof(ud->logFile)-1 );
    if( StringIsNull(ud->logFile) )
    {
        memcpy( ud->logFile, LOGFILE, strlen(LOGFILE) );
    }
    LOG( ud->iLogLevel, ud->logFile, "=========��ʼ��ȡ��������[%s]=======", pkgFile );
    GetIniConfig( pkgFile, "PKGDEF", "pkgId", ud->pkgDef->pkgId, sizeof(ud->pkgDef->pkgId)-1 );
    if( StringIsNull( ud->pkgDef->pkgId ) )
    {
        sprintf( ud->err, "ȡ [%s] pkgId ʧ��\0", pkgFile );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return GETPKGIDFAIL;
    }
    GetIniConfig( pkgFile, "PKGDEF", "pkgName", ud->pkgDef->pkgName, sizeof(ud->pkgDef->pkgName)-1 );
    if( StringIsNull( ud->pkgDef->pkgName ) )
    {
        sprintf( ud->err, "ȡ [%s] pkgName ʧ��\0", pkgFile );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return GETPKGNAMEFAIL;
    }
    GetIniConfig( pkgFile, "PKGDEF", "pkgType", ud->pkgDef->pkgType, sizeof(ud->pkgDef->pkgType)-1 );
    if( StringIsNull( ud->pkgDef->pkgType ) )
    {
        sprintf( ud->err, "ȡ [%s] pkgType ʧ��\0", pkgFile );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return GETPKGTYPEFAIL;
    }    
    GetIniConfig( pkgFile, "PKGDEF", "pkgMsgFmt", ud->pkgDef->pkgMsgFmt, sizeof(ud->pkgDef->pkgMsgFmt)-1 );
    ud->pkgDef->ePkgType = ConvertPkgType( ud->pkgDef->pkgType );
    if( ud->pkgDef->ePkgType == -1 )
    {
        sprintf( ud->err, "[%s] pkgType [%s] ��Ч\0", pkgFile, ud->pkgDef->pkgType );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return PKGTYPEINVALID;
    }
    
    GetIniConfig( pkgFile, "PKGDEF", "fileType", ud->pkgDef->fileType, sizeof(ud->pkgDef->fileType)-1 );
    if( StringIsNull( ud->pkgDef->fileType ) )
    {
        LOG( ud->iLogLevel, ud->logFile, "fileType is null, set default INI" );
        memcpy( ud->pkgDef->fileType, "INI\0", 4 );
    }
    ud->pkgDef->eFileType = ConvertEnumFileType( ud->pkgDef->fileType );
    if( ud->pkgDef->eFileType == -1 )
    {
        sprintf( ud->err, "[%s] fileType [%s] ��Ч\0", pkgFile, ud->pkgDef->fileType );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return PkgFileTypeInvalid;
    }
    
    memset( caBuf, 0, sizeof(caBuf) );
    GetIniConfig( pkgFile, "PKGDEF", "IFS", caBuf, sizeof(caBuf)-1 );
    if( StringIsNull( caBuf ) )
        ud->pkgDef->IFS = '|';
    else
        ud->pkgDef->IFS = caBuf[0];
    
    if( ud->pkgDef->eFileType == FileTypeIni )
    {
        memset( caBuf, 0, sizeof(caBuf) );
        LOG( ud->iLogLevel, ud->logFile, "pkgFileType = [%s], ȡfieldCount��fieldList", ud->pkgDef->fileType );
        GetIniConfig( pkgFile, "PKGDEF", "fieldCount", caBuf, sizeof(caBuf)-1 );
        if( StringIsNull( caBuf ) )
        {
            sprintf( ud->err, "ȡ [%s] fieldCount ʧ��\0", pkgFile );
            LOG( ud->iLogLevel, ud->logFile, ud->err );
            return GETFIELDCOUNTFAIL;
        }
        ud->pkgDef->fieldCount = atoi( caBuf );
        if( ud->pkgDef->fieldCount < 0 )
        {
            sprintf( ud->err, "fieldCount < 0 \0" );
            LOG( ud->iLogLevel, ud->logFile, ud->err );
            return FIELDCOUNTINVALID;
        }
        GetIniConfig( pkgFile, "PKGDEF", "fieldList", ud->pkgDef->fieldList, sizeof(ud->pkgDef->fieldList)-1 );
        if( StringIsNull(ud->pkgDef->fieldList) && ud->pkgDef->fieldCount != 0 )
        {
            sprintf( ud->err, "ȡ [%s] fieldList ʧ��, ��fieldCount[%d] ��Ϊ0\0", pkgFile, ud->pkgDef->fieldCount );
            LOG( ud->iLogLevel, ud->logFile, ud->err );
            return GETFIELDLISTFAIL;
        }
    }
    LOG( ud->iLogLevel, ud->logFile, "pkgid[%s], pkgName[%s], pkgType[%s] pkgMsgFmt[%s] ePkgType[%d] fileType[%s], eFileType[%d], fieldCount[%d]", 
                 ud->pkgDef->pkgId, ud->pkgDef->pkgName, ud->pkgDef->pkgType, ud->pkgDef->pkgMsgFmt, 
                 ud->pkgDef->ePkgType, ud->pkgDef->fileType, ud->pkgDef->eFileType, ud->pkgDef->fieldCount );
    LOG( ud->iLogLevel, ud->logFile, "=========������ȡ��������[%s]=======", pkgFile );
}

int LoadPkgFileSep( char *pkgFile, UpkgDef *ud )
{
    int ret;
    FILE *fp;
    char caReadBuf[4096+1];
    char caBuf[128+1];
    if( StringIsNullOrWhiteSpace(pkgFile) )
        return PKGFILEISNULL;
    if( ud == NULL )
        return UPKGDEFISNULL;    
    fp = GetIniFileNodePosition( pkgFile, "FIELD", fp );
    if( fp == NULL )
    {
        sprintf( ud->err, "[%s]û��[FIELD]�ڵ�\0", pkgFile );
        return PkgFileFormatError;
    }
    while( 1 )
    {
        memset( caReadBuf, 0, sizeof(caReadBuf) );
        if( fgets( caReadBuf, sizeof(caReadBuf)-1, fp) == NULL )
            break;
        trim( caReadBuf );
        if( StringIsNull( caReadBuf ) )
            continue;
        if( caReadBuf[0] == '#' )
            continue;
        if( caReadBuf[0] == '[' )
            break;
        UpkgFldDefList *ufdl = NewUpkgFldDefList();
        /* fieldId */

        /*** fieldId ***/
        memset( caBuf, 0, sizeof(caBuf) );
        ret = GetListByDiv( caReadBuf, ud->pkgDef->IFS, FieldIdIdx, caBuf, sizeof(caBuf)-1 );
        if( StringIsNull(caBuf) )
        {
            /*LOG( ud->iLogLevel, ud->logFile, "ȡ fieldId ʧ��, caBuf=[%s], [%d]", caBuf, ret);*/
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "==== [%s] begin ====", caNode );*/
        ufdl->fieldId = atoi( caBuf );
        if( SearchFieldListByFieldId( ud->fieldHead, ufdl->fieldId) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s].fieldId[%s][%d]�ظ�", pkgFile, caBuf, ufdl->fieldId );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldId = [%d]", ufdl->fieldId );*/
        
        /*** fieldName ***/
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, FieldNameIdx, ufdl->fieldName, sizeof(ufdl->fieldName)-1 );
        if( StringIsNull(ufdl->fieldName) )
        {
            LOG( ud->iLogLevel, ud->logFile, "fieldId[%d] ȡ fieldName ʧ��", ufdl->fieldId );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldName = [%s]", ufdl->fieldName );*/
        
        
        /*** fieldLength ***/
        memset( caBuf, 0, sizeof(caBuf) );
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, FieldLengthIdx, caBuf, sizeof(caBuf)-1 );
        if( ! StringIsDigit( caBuf ) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%d] ȡ fieldLength ʧ��, ȡĬ�� = 0, caBuf=[%s]",ufdl->fieldId, caBuf );
            caBuf[0] = '0';
        }
        ufdl->fieldLength = atoi( caBuf );
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLength = [%d][%s]", ufdl->fieldLength, caBuf );*/
        
        
        /*** fieldType ***/
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, FieldTypeIdx, ufdl->fieldType, sizeof(ufdl->fieldType)-1 );
        if( StringIsNull(ufdl->fieldType) )
        {
            /*LOG( ud->iLogLevel, ud->logFile, "[%d] ȡ fieldType ʧ��,ȡĬ��COMMON", ufdl->fieldId );*/
            memcpy( ufdl->fieldType, "COMMON\0", 7 );           
        }
        ufdl->eFieldType = ConvertFieldType( ufdl->fieldType );
        if( ufdl->eFieldType == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%d] fieldType [%s] ��Ч", ufdl->fieldId, ufdl->fieldType );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldType = [%s][%d]", ufdl->fieldType, ufdl->eFieldType );*/
        
        /*** subPkgFile ***/
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, SubPkgFileIdx, ufdl->subPkgFile, sizeof(ufdl->subPkgFile)-1 );
        /*LOG( ud->iLogLevel, ud->logFile, "subPkgFile = [%s]", ufdl->subPkgFile );*/
        if( StringIsNull(ufdl->subPkgFile) && ufdl->eFieldType == SUBFIELD )
        {
            /*LOG( ud->iLogLevel, ud->logFile, "[%d] fieldType=[%s]��subPkgFileΪ��", ufdl->fieldId, ufdl->fieldType );*/
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        
        /*** fieldLengthType ***/
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, FieldLengthTypeIdx, ufdl->fieldLengthType, sizeof(ufdl->fieldLengthType)-1 );
        if( StringIsNull(ufdl->fieldLengthType) )
        {
            /*LOG( ud->iLogLevel, ud->logFile, "[%d] ȡ fieldLengthType ʧ��, ȡĬ��VAR", ufdl->fieldId );*/
            memcpy( ufdl->fieldLengthType, "VAR\0", 4 );
        }
        ufdl->eFieldLengthType = ConvertLengthType( ufdl->fieldLengthType );
        if( ufdl->eFieldLengthType == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%d] fieldLengthType [%s] ��Ч", ufdl->fieldId, ufdl->fieldLengthType );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLengthType = [%s][%d]", ufdl->fieldLengthType, ufdl->eFieldLengthType );*/
        
        
        /*** fieldCoding ***/
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, FieldCodingIdx, ufdl->fieldCoding, sizeof(ufdl->fieldCoding)-1 );
        if( StringIsNull(ufdl->fieldCoding) )
        {
            /*LOG( ud->iLogLevel, ud->logFile, "[%d] ȡ fieldCoding ʧ��, ȡĬ��BYTE", ufdl->fieldId );*/
            memcpy( ufdl->fieldCoding, "BYTE\0", 5 );
        }
        ufdl->eFieldCoding = ConvertCodingType( ufdl->fieldCoding );
        if( ufdl->eFieldCoding == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%d] fieldCoding [%s] ��Ч", ufdl->fieldId, ufdl->fieldCoding );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldCoding = [%s][%d]", ufdl->fieldCoding, ufdl->eFieldCoding );*/
        
        /*** fieldLengthCoding ***/
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, FieldLengthCodingIdx, ufdl->fieldLengthCoding, sizeof(ufdl->fieldLengthCoding)-1 );
        if( StringIsNull(ufdl->fieldLengthCoding) )
        {
            /*LOG( ud->iLogLevel, ud->logFile, "[%d] ȡ fieldLengthCoding ʧ��,ȡĬ��BYTE", ufdl->fieldId );*/
            memcpy( ufdl->fieldLengthCoding, "BYTE\0", 5 );
        }
        ufdl->eFieldLengthCoding = ConvertCodingType( ufdl->fieldLengthCoding );
        if( ufdl->eFieldLengthCoding == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%d] fieldLengthCoding [%s] ��Ч", ufdl->fieldId, ufdl->fieldLengthCoding );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLengthCoding = [%s][%d]", ufdl->fieldLengthCoding, ufdl->eFieldLengthCoding );*/
        
        /*** fieldFmt ***/
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, FieldFormatFuncIdx, ufdl->fieldFmt, sizeof(ufdl->fieldFmt)-1 );
        if( ufdl->eFieldType == BITMAP )
        {
            if( strcmp( ufdl->fieldFmt, "ByteToBitmap") && strcmp( ufdl->fieldFmt, "AsciiHexToBitmap") )
            {
                sprintf( ud->err, "fieldType=[%s],but fieldFmt is not ByteToBitmap either AsciiHexToBitmap\0", ufdl->fieldType );
                LOG( ud->iLogLevel, ud->logFile, ud->err );
                return -10;
            }
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldFmt = [%s]", ufdl->fieldFmt );*/
        /*** fieldLengthFmt ***/
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, FieldLengthFormatFuncIdx, ufdl->fieldLengthFmt, sizeof(ufdl->fieldLengthFmt)-1 );
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLengthFmt = [%s]", ufdl->fieldLengthFmt );*/
        /*** comment ***/
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, CommentIdx, ufdl->comment, sizeof(ufdl->comment)-1 );
        /*LOG( ud->iLogLevel, ud->logFile, "comment = [%s]", ufdl->comment );*/
        
        /** sepChar1 **/
        memset( caBuf, 0, sizeof(caBuf) );
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, SepChar1Idx, caBuf, sizeof(caBuf)-1 );
        trim(caBuf);
        ufdl->sepChar1 = caBuf[0];
        memset( caBuf, 0, sizeof(caBuf) );
        GetListByDiv( caReadBuf, ud->pkgDef->IFS, SepChar2Idx, caBuf, sizeof(caBuf)-1 );
        trim(caBuf);
        ufdl->sepChar2 = caBuf[0];
        
        if( ud->fieldHead == NULL )
        {
            ud->fieldHead = ufdl;
            ud->fieldTail = ufdl;
        }
        else
        {
            ud->fieldTail->next = ufdl;
            ud->fieldTail = ufdl;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "==== [%s] end   ====", caNode );*/
        
    }
    return SUCCESS;
}

int LoadPkgFileIni( char *pkgFile, UpkgDef *ud )
{
    int ret;
    char caBuf[128];
    if( StringIsNullOrWhiteSpace(pkgFile) )
        return PKGFILEISNULL;
    if( ud == NULL )
        return UPKGDEFISNULL;    
    memset( caBuf, 0, sizeof(caBuf) );
    int i = 0;
    for( i=1; i<=ud->pkgDef->fieldCount; i++ )
    {
        char caNode[128];
        memset( caNode, 0, sizeof(caNode) );
        GetListByDiv( ud->pkgDef->fieldList, ',', i, caNode, sizeof(caNode)-1 );
        if( StringIsNull(caNode) )
        {
            LOG( ud->iLogLevel, ud->logFile, "��fieldListȡ��%d����ڵ�����ʧ��", i );
            continue;
        }
        
        UpkgFldDefList *ufdl = NewUpkgFldDefList();
        
        /*** fieldId ***/
        memset( caBuf, 0, sizeof(caBuf) );
        ret = GetIniConfig( pkgFile, caNode, "fieldId", caBuf, sizeof(caBuf)-1 );
        if( StringIsNull(caBuf) )
        {
            /*LOG( ud->iLogLevel, ud->logFile, "ȡ fieldId ʧ��, caBuf=[%s], [%d]", caBuf, ret);*/
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "==== [%s] begin ====", caNode );*/
        ufdl->fieldId = atoi( caBuf );
        if( SearchFieldListByFieldId( ud->fieldHead, ufdl->fieldId) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s][%s].fieldId[%s][%d]�ظ�", pkgFile, caNode, caBuf, ufdl->fieldId );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldId = [%d]", ufdl->fieldId );*/
        
        /*** fieldName ***/
        GetIniConfig( pkgFile, caNode, "fieldName", ufdl->fieldName, sizeof(ufdl->fieldName)-1 );
        if( StringIsNull(ufdl->fieldName) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] ȡ fieldName ʧ��", caNode );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldName = [%s]", ufdl->fieldName );*/
        
        /*** fieldType ***/
        GetIniConfig( pkgFile, caNode, "fieldType", ufdl->fieldType, sizeof(ufdl->fieldType)-1 );
        if( StringIsNull(ufdl->fieldType) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] ȡ fieldType ʧ��", caNode );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        ufdl->eFieldType = ConvertFieldType( ufdl->fieldType );
        if( ufdl->eFieldType == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] fieldType [%s] ��Ч", caNode, ufdl->fieldType );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldType = [%s][%d]", ufdl->fieldType, ufdl->eFieldType );*/
        
        /*** subPkgFile ***/
        GetIniConfig( pkgFile, caNode, "subPkgFile", ufdl->subPkgFile, sizeof(ufdl->subPkgFile)-1 );
        /*LOG( ud->iLogLevel, ud->logFile, "subPkgFile = [%s]", ufdl->subPkgFile );*/
        if( StringIsNull(ufdl->subPkgFile) && ufdl->eFieldType == SUBFIELD )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] fieldType=[%s]��subPkgFileΪ��", caNode, ufdl->fieldType );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        
        /*** fieldLengthType ***/
        GetIniConfig( pkgFile, caNode, "fieldLengthType", ufdl->fieldLengthType, sizeof(ufdl->fieldLengthType)-1 );
        if( StringIsNull(ufdl->fieldLengthType) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] ȡ fieldLengthType ʧ��", caNode );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        ufdl->eFieldLengthType = ConvertLengthType( ufdl->fieldLengthType );
        if( ufdl->eFieldLengthType == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] fieldLengthType [%s] ��Ч", caNode, ufdl->fieldLengthType );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLengthType = [%s][%d]", ufdl->fieldLengthType, ufdl->eFieldLengthType );*/
        
        /*** fieldLength ***/
        memset( caBuf, 0, sizeof(caBuf) );
        GetIniConfig( pkgFile, caNode, "fieldLength", caBuf, sizeof(caBuf)-1 );
        if( StringIsNull(caBuf) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] ȡ fieldLength ʧ��, ȡĬ�� = 0, caBuf=[%s]",caNode, caBuf );
            caBuf[0] = '0';
        }
        ufdl->fieldLength = atoi( caBuf );
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLength = [%d][%s]", ufdl->fieldLength, caBuf );*/
        
        /*** fieldLengthCoding ***/
        GetIniConfig( pkgFile, caNode, "fieldLengthCoding", ufdl->fieldLengthCoding, sizeof(ufdl->fieldLengthCoding)-1 );
        if( StringIsNull(ufdl->fieldLengthCoding) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] ȡ fieldLengthCoding ʧ��", caNode );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        ufdl->eFieldLengthCoding = ConvertCodingType( ufdl->fieldLengthCoding );
        if( ufdl->eFieldLengthCoding == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] fieldLengthCoding [%s] ��Ч", caNode, ufdl->fieldLengthCoding );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLengthCoding = [%s][%d]", ufdl->fieldLengthCoding, ufdl->eFieldLengthCoding );*/
        
        /*** fieldCoding ***/
        GetIniConfig( pkgFile, caNode, "fieldCoding", ufdl->fieldCoding, sizeof(ufdl->fieldCoding)-1 );
        if( StringIsNull(ufdl->fieldCoding) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] ȡ fieldCoding ʧ��", caNode );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        ufdl->eFieldCoding = ConvertCodingType( ufdl->fieldCoding );
        if( ufdl->eFieldCoding == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] fieldCoding [%s] ��Ч", caNode, ufdl->fieldCoding );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldCoding = [%s][%d]", ufdl->fieldCoding, ufdl->eFieldCoding );*/
        
        /*** fieldFmt ***/
        GetIniConfig( pkgFile, caNode, "fieldFmt", ufdl->fieldFmt, sizeof(ufdl->fieldFmt)-1 );
        if( ufdl->eFieldType == BITMAP )
        {
            if( strcmp( ufdl->fieldFmt, "ByteToBitmap") && strcmp( ufdl->fieldFmt, "AsciiHexToBitmap") )
            {
                sprintf( ud->err, "fieldType=[%s],but fieldFmt is not ByteToBitmap either AsciiHexToBitmap\0", ufdl->fieldType );
                LOG( ud->iLogLevel, ud->logFile, ud->err );
                return -10;
            }
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldFmt = [%s]", ufdl->fieldFmt );*/
        /*** fieldLengthFmt ***/
        GetIniConfig( pkgFile, caNode, "fieldLengthFmt", ufdl->fieldLengthFmt, sizeof(ufdl->fieldLengthFmt)-1 );
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLengthFmt = [%s]", ufdl->fieldLengthFmt );*/
        /*** comment ***/
        GetIniConfig( pkgFile, caNode, "comment", ufdl->comment, sizeof(ufdl->comment)-1 );
        /*LOG( ud->iLogLevel, ud->logFile, "comment = [%s]", ufdl->comment );*/
        
        /** sepChar1 **/
        memset( caBuf, 0, sizeof(caBuf) );
        GetIniConfig( pkgFile, caNode, "sepChar1", caBuf, sizeof(caBuf)-1 );
        trim( caBuf );
        ufdl->sepChar1 = caBuf[0];
        memset( caBuf, 0, sizeof(caBuf) );
        GetIniConfig( pkgFile, caNode, "sepChar2", caBuf, sizeof(caBuf)-1 );
        trim( caBuf );
        ufdl->sepChar2 = caBuf[0];
        
        if( ud->fieldHead == NULL )
        {
            ud->fieldHead = ufdl;
            ud->fieldTail = ufdl;
        }
        else
        {
            ud->fieldTail->next = ufdl;
            ud->fieldTail = ufdl;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "==== [%s] end   ====", caNode );*/
    }
    return SUCCESS;
}

int LoadPkgFile( char *pkgFile, UpkgDef *ud )
{
    int ret;
    
    if( StringIsNullOrWhiteSpace(pkgFile) )
        return PKGFILEISNULL;
    if( ud == NULL )
        return UPKGDEFISNULL;

    
    ret = LoadPkgDef( pkgFile, ud );
    if( ret != SUCCESS )
    {
        return ret;
    }
    /*** FIELD ***/
    LOG( ud->iLogLevel, ud->logFile, "========��ʼ��ȡ���������� eFileType=[%d]========", ud->pkgDef->eFileType );
    if( ud->pkgDef->eFileType == FileTypeIni )
        ret = LoadPkgFileIni( pkgFile, ud );
    else if ( ud->pkgDef->eFileType == FileTypeSep )
        ret = LoadPkgFileSep( pkgFile, ud );
    else
        ret = LoadPkgFileError;
    LOG( ud->iLogLevel, ud->logFile, "========�������Ķ�ȡ������========" );
    return ret;
}

int SavePkgDefToFile( UpkgDef* ud, char *cpPkgFile, FileType fileType )
{
    if( ud == NULL || StringIsNull( cpPkgFile ) )
        return PKGFILEISNULL;
    FILE *fp;
    fp = fopen( cpPkgFile, "wb" );
    if( fp == NULL )
    {
        sprintf( ud->err, "%d,%s\0", errno, strerror(errno) );
        return FAILED;
    }
    fprintf( fp, "### %s\n\n", cpPkgFile );
    fprintf( fp, "#fieldLengthFmt: int FunctionName( char *in, int length) \n\
    #ByteToInt   16����ת��10����,0X00 0X0D=>214 \n\
    #AsciiHexToInt  16�����ַ���ת��10����,\"000D\"=>214 \n\
    #BcdToInt     BCDѹ���ַ���ת��Int 0X0D --> 000D --> 214 \n\
#fieldFmt:  int FunctionName(char *in, int length, char *out, int *outlen) \n\
    #AsciiHexToByte     16�����ַ���ת��ԭֵ \n\
    #ByteToAsciiHex     ԭֵת����16�����ַ��� \n\
    #BcdToByte       BCDѹ��ת����ԭֵ \n\
    #BcdToAsciiHex       BCDѹ��ת����ԭֵ16�����ַ��� \n\
    #ByteToBitmap    ԭֵת����λͼ             λͼ�� ByteToBitmap AsciiHexToBitmap�����ѡһ \n\
    #AsciiHexToBitmap   16�����ַ���ת����Bitmap \n\
    #ByteHexToIntEx    16����ת��10����,0X00 0X0D=>214 \n\
    #AsciiHexToIntEx   16�����ַ���ת��10����,\"000D\"=>214 \n\
    #DeleteReturnChars ɾ���س��� \n\
    #DeleteNotHexChars ɾ����16�����ַ� \n\
#sepChar , | \\\n\n\n" );
    fprintf( fp, "[PKGDEF]\n" );
    fprintf( fp,  "logLevel  = %d # 0����ӡ��־, 1-��ӡ��־\nlogFile   = %s\n", ud->iLogLevel, ud->logFile );
    fprintf( fp,  "pkgId     = %s\n"
                  "pkgName   = %s\n"
                  "pkgType   = %s #FIXED(����) TLV 8583 SEP(�ָ���)\n"
                  "pkgMsgFmt = %s ##AsciiHexToByte DeleteNotHexChars\n"
                  "IFS       = %c\n",
            ud->pkgDef->pkgId, ud->pkgDef->pkgName, ud->pkgDef->pkgType, ud->pkgDef->pkgMsgFmt, ud->pkgDef->IFS );
    if( fileType == FileTypeIni )
    {
        int fieldCount = 0;
        UpkgFldDefList *ufdl;
        fprintf( fp, "fileType  = INI #INI SEP\n" );
        fprintf( fp, "fieldList = " );
        for( ufdl=ud->fieldHead; ufdl; ufdl=ufdl->next )
        {
            fieldCount ++;
            if( fieldCount == 1 )
                fprintf( fp, "F%d", ufdl->fieldId );
            else
                fprintf( fp, ",F%d", ufdl->fieldId );
        }
        fprintf( fp, "\nfieldCount = %d\n\n\n", fieldCount );

        for( ufdl=ud->fieldHead; ufdl; ufdl=ufdl->next )
        {
            fprintf( fp, "[F%d]\n", ufdl->fieldId );
            fprintf( fp, "fieldId           = %d\n", ufdl->fieldId );
            fprintf( fp, "fieldName         = %s\n", ufdl->fieldName );
            fprintf( fp, "fieldLength       = %d\n", ufdl->fieldLength );
            fprintf( fp, "fieldType         = %s            #fieldType: COMMON-һ���� SUBFIELD-�ӱ���  BITMAP- λͼ\n", ufdl->fieldType );
            fprintf( fp, "subPkgFile        = %s\n", ufdl->subPkgFile );
            fprintf( fp, "fieldLengthType   = %s             #fieldLengthType: VAR-�̶����� LLVAR-��λ�䳤 LLLVAR-��λ�䳤\n", ufdl->fieldLengthType );
            fprintf( fp, "fieldCoding       = %s            #Coding: Byte-ԭֵ AsciiHex-16�����ַ��� BCD-BCDѹ�� BIT-�����Ʊ���101010101010\n", ufdl->fieldCoding );
            fprintf( fp, "fieldLengthCoding = %s            #Coding: Byte-ԭֵ AsciiHex-16�����ַ��� BCD-BCDѹ�� BIT-�����Ʊ���101010101010\n", ufdl->fieldLengthCoding );
            fprintf( fp, "fieldFmt          = %s\n", ufdl->fieldFmt );
            fprintf( fp, "fieldLengthFmt    = %s\n", ufdl->fieldLengthFmt );
            fprintf( fp, "comment           = %s\n", ufdl->comment );
            if( isprint( ufdl->sepChar1 ) )
                fprintf( fp, "sepChar1          = %c\n", ufdl->sepChar1 );
            if( isprint( ufdl->sepChar2 ) )
                fprintf( fp, "sepChar2          = %c\n", ufdl->sepChar2 );
            fprintf( fp, "\n" );
        }
    }
    else if ( fileType == FileTypeSep )
    {
        UpkgFldDefList *ufdl;
        fprintf( fp, "fileType = SEP #INI SEP\n\n\n\n" );
        fprintf( fp, "[FIELD]\n#filedId|fieldName|fieldLength|fieldType|subPkgFile|filedLengthType|fieldCoding|fieldLengthCoding|fieldFormatFunc|fieldLengthFormatFunc|Comment\n" );
        fprintf( fp, "#fieldType: COMMON-һ���� SUBFIELD-�ӱ���  BITMAP- λͼ\n" );
        fprintf( fp, "#fieldLengthType: VAR-�̶����� LLVAR-��λ�䳤 LLLVAR-��λ�䳤\n" );
        fprintf( fp, "#Coding: Byte-ԭֵ AsciiHex-16�����ַ��� BCD-BCDѹ�� BIT-�����Ʊ���101010101010\n\n" );
        for( ufdl=ud->fieldHead; ufdl; ufdl=ufdl->next )
        {
            fprintf( fp, "%d|%s|%d|%s|%s|%s|%s|%s|%s|%s|%s",
            ufdl->fieldId, ufdl->fieldName, ufdl->fieldLength, 
            ufdl->fieldType, ufdl->subPkgFile, ufdl->fieldLengthType,
            ufdl->fieldCoding, ufdl->fieldLengthCoding, ufdl->fieldFmt, ufdl->fieldLengthFmt, 
            ufdl->comment );
            if( isprint( ufdl->sepChar1 ) )
            {
                fprintf( fp, "|%c|", ufdl->sepChar1 );
                if( isprint( ufdl->sepChar2 ) )
                    fprintf( fp, "%c", ufdl->sepChar2 );
            }
            else if( isprint( ufdl->sepChar2 ) )
                fprintf( fp, "||%c", ufdl->sepChar2 );
            fprintf( fp, "\n" );
        }
    }
    fclose( fp );
    return SUCCESS;
}
int ConvertPkgFileType( char *cpFromFile, char *cpToFile, FileType fileType )
{
    if( StringIsNull(cpFromFile) || StringIsNull(cpToFile) )
        return PKGFILEISNULL;
    if( strcasecmp( cpFromFile, cpToFile ) == 0 )
    {
        printf( "fromFile[%s] =  toFile[%s]\n", cpFromFile, cpToFile );
        return FAILED;
    }
    int ret = 0;
    UpkgDef *ud = NewUpkgDef();
    ret = LoadPkgFile( cpFromFile, ud );
    if( ret != SUCCESS )
    {
        printf( "%s\n", ud->err );
        DeleteUpkgDef( ud );
        return ret;
    }
    ret = SavePkgDefToFile( ud, cpToFile, fileType );
    DeleteUpkgDef( ud );
    return ret;
}