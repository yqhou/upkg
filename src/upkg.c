#include "upkg.h"
#include "list.h"
#include "util.h"
#define LOGFILE "upkg.log"


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

int LoadPkgFile( char *pkgFile, UpkgDef *ud )
{
    FILE *fp;
    char caFieldList[4096];
    int ret;
    if( StringIsNullOrWhiteSpace(pkgFile) )
        return -1;
    if( ud == NULL )
        return -2;
    char caBuf[128];
    memset( caBuf, 0, sizeof(caBuf) );
    GetIniConfig( pkgFile, "PKGDEF", "logLevel", caBuf );
    if( StringIsNull(caBuf) )
        ud->iLogLevel = 0;
    else
        ud->iLogLevel = atoi( caBuf );
    
    GetIniConfig( pkgFile, "PKGDEF", "logFile", ud->logFile );
    if( StringIsNull(ud->logFile) )
    {
        memcpy( ud->logFile, LOGFILE, strlen(LOGFILE) );
    }
    
    GetIniConfig( pkgFile, "PKGDEF", "pkgId", ud->pkgDef->pkgId );
    if( StringIsNull( ud->pkgDef->pkgId ) )
    {
        sprintf( ud->err, "取 [%s] pkgId 失败\0", pkgFile );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return -7;
    }
    GetIniConfig( pkgFile, "PKGDEF", "pkgName", ud->pkgDef->pkgName );
    if( StringIsNull( ud->pkgDef->pkgName ) )
    {
        sprintf( ud->err, "取 [%s] pkgName 失败\0", pkgFile );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return -8;
    }
    GetIniConfig( pkgFile, "PKGDEF", "pkgType", ud->pkgDef->pkgType );
    if( StringIsNull( ud->pkgDef->pkgType ) )
    {
        sprintf( ud->err, "取 [%s] pkgType 失败\0", pkgFile );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return -9;
    }    
    GetIniConfig( pkgFile, "PKGDEF", "pkgMsgFmt", ud->pkgDef->pkgMsgFmt );
    ud->pkgDef->ePkgType = ConvertPkgType( ud->pkgDef->pkgType );
    if( ud->pkgDef->ePkgType == -1 )
    {
        sprintf( ud->err, "[%s] pkgType [%s] 无效\0", pkgFile, ud->pkgDef->pkgType );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return -10;
    }
    memset( caBuf, 0, sizeof(caBuf) );
    GetIniConfig( pkgFile, "PKGDEF", "fieldCount", caBuf );
    if( StringIsNull( caBuf ) )
    {
        sprintf( ud->err, "取 [%s] fieldCount 失败\0", pkgFile );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return -4;
    }
    ud->pkgDef->fieldCount = atoi( caBuf );
    if( ud->pkgDef->fieldCount < 0 )
    {
        sprintf( ud->err, "fieldCount < 0 \0" );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return -5;
    }
    GetIniConfig( pkgFile, "PKGDEF", "fieldList", ud->pkgDef->fieldList );
    if( StringIsNull(ud->pkgDef->fieldList) && ud->pkgDef->fieldCount != 0 )
    {
        sprintf( ud->err, "取 [%s] fieldList 失败, 但fieldCount[%d] 不为0\0", pkgFile, ud->pkgDef->fieldCount );
        LOG( ud->iLogLevel, ud->logFile, ud->err );
        return -6;
    }
    
    LOG( ud->iLogLevel, ud->logFile, "pkgid[%s], pkgName[%s], pkgType[%s] pkgMsgFmt[%s] ePkgType[%d] fieldCount[%d]", 
                 ud->pkgDef->pkgId, ud->pkgDef->pkgName, ud->pkgDef->pkgType, ud->pkgDef->pkgMsgFmt, ud->pkgDef->ePkgType, ud->pkgDef->fieldCount );
 
    /*** FIELD ***/
    LOG( ud->iLogLevel, ud->logFile, "========开始读取域配置========" );
    int i = 0;
    for( i=1; i<=ud->pkgDef->fieldCount; i++ )
    {
        char caNode[128];
        memset( caNode, 0, sizeof(caNode) );
        GetListByDiv( ud->pkgDef->fieldList, ',', i, caNode );
        if( StringIsNull(caNode) )
        {
            LOG( ud->iLogLevel, ud->logFile, "从fieldList取第%d个域节点名称失败", i );
            continue;
        }
        
        UpkgFldDefList *ufdl = NewUpkgFldDefList();
        
        /*** fieldId ***/
        memset( caBuf, 0, sizeof(caBuf) );
        ret = GetIniConfig( pkgFile, caNode, "fieldId", caBuf );
        if( StringIsNull(caBuf) )
        {
            /*LOG( ud->iLogLevel, ud->logFile, "取 fieldId 失败, caBuf=[%s], [%d]", caBuf, ret);*/
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "==== [%s] begin ====", caNode );*/
        ufdl->fieldId = atoi( caBuf );
        if( SearchFieldListByFieldId( ud->fieldHead, ufdl->fieldId) )
        {
            LOG( ud->iLogLevel, ud->logFile, "fieldId[%s][%d]重复", caBuf, ufdl->fieldId );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldId = [%d]", ufdl->fieldId );*/
        
        /*** fieldName ***/
        GetIniConfig( pkgFile, caNode, "fieldName", ufdl->fieldName );
        if( StringIsNull(ufdl->fieldName) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] 取 fieldName 失败", caNode );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldName = [%s]", ufdl->fieldName );*/
        
        /*** fieldType ***/
        GetIniConfig( pkgFile, caNode, "fieldType", ufdl->fieldType );
        if( StringIsNull(ufdl->fieldType) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] 取 fieldType 失败", caNode );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        ufdl->eFieldType = ConvertFieldType( ufdl->fieldType );
        if( ufdl->eFieldType == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] fieldType [%s] 无效", caNode, ufdl->fieldType );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldType = [%s][%d]", ufdl->fieldType, ufdl->eFieldType );*/
        
        /*** subPkgFile ***/
        GetIniConfig( pkgFile, caNode, "subPkgFile", ufdl->subPkgFile );
        /*LOG( ud->iLogLevel, ud->logFile, "subPkgFile = [%s]", ufdl->subPkgFile );*/
        if( StringIsNull(ufdl->subPkgFile) && ufdl->eFieldType == SUBFIELD )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] fieldType=[%s]但subPkgFile为空", caNode, ufdl->fieldType );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        
        /*** fieldLengthType ***/
        GetIniConfig( pkgFile, caNode, "fieldLengthType", ufdl->fieldLengthType );
        if( StringIsNull(ufdl->fieldLengthType) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] 取 fieldLengthType 失败", caNode );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        ufdl->eFieldLengthType = ConvertLengthType( ufdl->fieldLengthType );
        if( ufdl->eFieldLengthType == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] fieldLengthType [%s] 无效", caNode, ufdl->fieldLengthType );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLengthType = [%s][%d]", ufdl->fieldLengthType, ufdl->eFieldLengthType );*/
        
        /*** fieldLength ***/
        memset( caBuf, 0, sizeof(caBuf) );
        GetIniConfig( pkgFile, caNode, "fieldLength", caBuf );
        if( StringIsNull(caBuf) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] 取 fieldLength 失败, 取默认 = 0, caBuf=[%s]",caNode, caBuf );
            caBuf[0] = '0';
        }
        ufdl->fieldLength = atoi( caBuf );
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLength = [%d][%s]", ufdl->fieldLength, caBuf );*/
        
        /*** fieldLengthCoding ***/
        GetIniConfig( pkgFile, caNode, "fieldLengthCoding", ufdl->fieldLengthCoding );
        if( StringIsNull(ufdl->fieldLengthCoding) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] 取 fieldLengthCoding 失败", caNode );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        ufdl->eFieldLengthCoding = ConvertCodingType( ufdl->fieldLengthCoding );
        if( ufdl->eFieldLengthCoding == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] fieldLengthCoding [%s] 无效", caNode, ufdl->fieldLengthCoding );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLengthCoding = [%s][%d]", ufdl->fieldLengthCoding, ufdl->eFieldLengthCoding );*/
        
        /*** fieldCoding ***/
        GetIniConfig( pkgFile, caNode, "fieldCoding", ufdl->fieldCoding );
        if( StringIsNull(ufdl->fieldCoding) )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] 取 fieldCoding 失败", caNode );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        ufdl->eFieldCoding = ConvertCodingType( ufdl->fieldCoding );
        if( ufdl->eFieldCoding == -1 )
        {
            LOG( ud->iLogLevel, ud->logFile, "[%s] fieldCoding [%s] 无效", caNode, ufdl->fieldCoding );
            DeleteUpkgFldDefList( ufdl );
            continue;
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldCoding = [%s][%d]", ufdl->fieldCoding, ufdl->eFieldCoding );*/
        
        /*** fieldFmt ***/
        GetIniConfig( pkgFile, caNode, "fieldFmt", ufdl->fieldFmt );
        if( ufdl->eFieldType == BITMAP )
        {
            if( strcmp( ufdl->fieldFmt, "ByteToBitmap") && strcmp( ufdl->fieldFmt, "AsciiHexToBitmap") )
            {
                sprintf( ud->err, "fieldType=[%s],but fieldFmt is not ByteToBitmap either AsciiHexToBitmap", ufdl->fieldType );
                LOG( ud->iLogLevel, ud->logFile, ud->err );
                return -10;
            }
        }
        /*LOG( ud->iLogLevel, ud->logFile, "fieldFmt = [%s]", ufdl->fieldFmt );*/
        /*** fieldLengthFmt ***/
        GetIniConfig( pkgFile, caNode, "fieldLengthFmt", ufdl->fieldLengthFmt );
        /*LOG( ud->iLogLevel, ud->logFile, "fieldLengthFmt = [%s]", ufdl->fieldLengthFmt );*/
        /*** comment ***/
        GetIniConfig( pkgFile, caNode, "comment", ufdl->comment );
        /*LOG( ud->iLogLevel, ud->logFile, "comment = [%s]", ufdl->comment );*/
        
        /** sepChar1 **/
        memset( caBuf, 0, sizeof(caBuf) );
        GetIniConfig( pkgFile, caNode, "sepChar1", caBuf );
        ufdl->sepChar1 = caBuf[0];
        memset( caBuf, 0, sizeof(caBuf) );
        GetIniConfig( pkgFile, caNode, "sepChar2", caBuf );
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
 
    LOG( ud->iLogLevel, ud->logFile, "========结束读取域配置========" );
    return 0;
}

int ConvertFieldValueLength( char *cpFieldLength, int iInLen, UpkgFldDefList *ufdl, UpkgDef *ud )
{
    char caTemp[128+1];
    int iLen;
    memset( caTemp, 0, sizeof(caTemp) );
    int fieldValueLength;
    switch( ufdl->eFieldLengthCoding )
    {
        case BYTE:
            memcpy( caTemp, cpFieldLength, strlen(cpFieldLength) );
            iLen = iInLen;
            break;
        case BCD:
            BcdToByte( cpFieldLength, iInLen, caTemp, &iLen );
            break;
        case ASCIIHEX:
            AsciiHexToByte( cpFieldLength, iInLen, caTemp, &iLen );
            break;
        default:
            LOG( 1, ud->logFile, "eFieldLengthCoding无效, [%s][%d]", ufdl->fieldLengthCoding, ufdl->eFieldLengthCoding );
            return -1;
    }
    if( StringIsNull( ufdl->fieldLengthFmt ) )
    {
        fieldValueLength = atoi( caTemp );
        return fieldValueLength;
    }
    else
    {
        int functionIdx = GetConvertFunctionIdx( ufdl->fieldLengthFmt, 2 );
        if( functionIdx < 0 )
        {
            LOG( 1, ud->logFile, "fieldLengthFmt无效,[%s]", ufdl->fieldLengthFmt );
            return -2;
        }
        functionMap[functionIdx].ConvertLengthFunction( caTemp, iInLen, &fieldValueLength );
        return fieldValueLength;
    }
}

int ConvertFieldValue( char *cpSrc, int iInLen, UpkgFldDefList *ufdl, UpkgResultList *ur, UpkgDef *ud )
{
    int ret = 0;
    int iLen = 0;
    char fmtFunction[64+1];
    memset( fmtFunction, 0, sizeof(fmtFunction) );
    if( strcmp( ufdl->fieldFmt, "ByteToBitmap" ) == 0 )
        memcpy( fmtFunction, "ByteToAsciiHex", sizeof(fmtFunction)-1 );
    else if ( strcmp( ufdl->fieldFmt, "AsciiHexToBitmap") )
        memcpy( fmtFunction, ufdl->fieldFmt, sizeof(fmtFunction)-1 );
        
    if( ! StringIsNull( fmtFunction ) )
    {

        int functionIdx = GetConvertFunctionIdx( fmtFunction, 1 );
        if( functionIdx < 0 )
        {
            LOG( 1, ud->logFile, "域值格式转换函数错误[%s]", fmtFunction );
            return -1;
        }
        functionMap[functionIdx].ConvertStringFunction( cpSrc, iInLen, ur->value, &iLen ); 
        ur->length = iLen;
    }
    else
    {
        memcpy( ur->value, cpSrc, iInLen );
        ur->value[iInLen] = 0;
    }
    /** 
    子报文
    **/
    if( ufdl->eFieldType == SUBFIELD )
    {
        LOG( ud->iLogLevel, ud->logFile, "子报文开始...." );
        UpkgDef *subUd = NewUpkgDef();
        memcpy( subUd->orgMsg, ur->value, strlen(ur->value) );
        memcpy( subUd->pkgFile, ufdl->subPkgFile,strlen(ufdl->subPkgFile) );
        ret = UpkgMain( subUd );
        if( ret == 0 )
        {
            char o[10240];
            memset( o, 0, sizeof(o) );
            ur->subfld = CopyUpkgResultList( subUd->result ) ;           
            DeleteUpkgDef( subUd );
        }
        else
        {
            LOG( 1, ud->logFile, "子报文处理失败, %s", subUd->err );
            ur->subfld = CopyUpkgResultList( subUd->result ) ;
            DeleteUpkgDef( subUd );
        }
        LOG( ud->iLogLevel, ud->logFile, "子报文结束...." );
    }
    return 0;
}

int Unpack8583Field( UpkgDef *ud, int *offset, int fieldId )
{
    if( ud == NULL ) 
            return -39;
    LOG( ud->iLogLevel, ud->logFile, "域 %d: ", fieldId );
    UpkgFldDefList* ufdl = SearchFieldListByFieldId( ud->fieldHead, fieldId );
    if( ufdl == NULL )
    {
        sprintf( ud->err, "[%s][%s]取域[%d]配置失败,\0", ud->pkgFile, ud->pkgDef->pkgId, fieldId );
        LOG( 1, ud->logFile, ud->err );
        return -30;
    }
    char caTemp[1024+1];
    int fieldValueLength = ufdl->fieldLength;  /*** 域值长度 **/
    int fieldValueActualLength = ufdl->fieldLength; /** 域值报文实际长度 **/
    int fieldLengthActualLength = 0; /** 域长度的实际长度 **/
    if( ufdl->eFieldLengthType != VAR )
    {
        fieldLengthActualLength = ufdl->eFieldLengthType == LLVAR ? 2 : 3;
        if( ufdl->eFieldLengthCoding == BCD )
            fieldLengthActualLength = (int)( (fieldLengthActualLength+1) / 2 );
        else if ( ufdl->eFieldLengthCoding == ASCIIHEX )
            fieldLengthActualLength = fieldLengthActualLength*2;
        LOG( ud->iLogLevel, ud->logFile, "域长度类型:[%s],域长度编码类型[%s]，域长度的实际长度=[%d], 域长度: ", 
                ufdl->fieldLengthType, ufdl->fieldLengthCoding, fieldLengthActualLength );
        memset( caTemp, 0, sizeof(caTemp) );
        if( (*offset) + fieldLengthActualLength > ud->packageLen )
        {
            sprintf( ud->err, "offset=[%d],报文长度[%d]不足", (*offset), ud->packageLen  );
            LOG( 1, ud->logFile, ud->err );
            return -32;
        }
        memcpy( caTemp, ud->package+(*offset), fieldLengthActualLength );
        (*offset) = (*offset) + fieldLengthActualLength;
        LOGHEX( ud->iLogLevel, ud->logFile, caTemp, fieldLengthActualLength );
        fieldValueLength = ConvertFieldValueLength( caTemp,fieldLengthActualLength, ufdl, ud );
        if( fieldValueLength < 0 )
        {
            sprintf( ud->err, "域值长度转换失败");
            LOG( 1, ud->logFile, ud->err );
            return -31;
        }
        LOG( ud->iLogLevel, ud->logFile, "域值长度: [%d]", fieldValueLength );
    }
    fieldValueActualLength = ufdl->eFieldCoding == BCD ? ((int)( (fieldValueLength+1) / 2 ) ) : 
                            ( ufdl->eFieldCoding == ASCIIHEX ? fieldValueLength*2 : fieldValueLength );
    memset( caTemp, 0, sizeof(caTemp) );
    LOG( ud->iLogLevel, ud->logFile, "域值长度[%d],域值编码类型[%s],取域值报文实际长度=[%d]",
         fieldValueLength, ufdl->fieldCoding, fieldValueActualLength );
    if( (*offset) + fieldValueActualLength > ud->packageLen )
    {
        sprintf( ud->err, "offset=[%d],报文长度[%d]不足", (*offset), ud->packageLen  );
        LOG( 1, ud->logFile, ud->err );
        return -33;
    }
    memcpy( caTemp, ud->package+(*offset), fieldValueActualLength );
    (*offset) = (*offset) + fieldValueActualLength;
    LOG( ud->iLogLevel, ud->logFile, "域值:" );
    LOGHEX( ud->iLogLevel, ud->logFile, caTemp, fieldValueActualLength );
    UpkgResultList *ur = NewUpkgResultList();
    int ret = ConvertFieldValue( caTemp, fieldValueActualLength, ufdl, ur, ud);
    if( ret != 0 )
    {
        sprintf( ud->err, "域值格式转换失败" );
        LOG( 1, ud->logFile, ud->err );
        DeleteUpkgResultList( ur );
        return -34;
    }
    if( ufdl->eFieldType == BITMAP )
    {
        int t;
        if( strcmp( ufdl->fieldFmt, "ByteToBitmap") == 0 )
            ByteToBitmap( caTemp, fieldValueActualLength, ud->bitmap, &t);
        else if ( strcmp( ufdl->fieldFmt, "AsciiHexToBitmap") == 0 )
            AsciiHexToBitmap( caTemp, fieldValueActualLength, ud->bitmap, &t );
    }
    
    ur->length = fieldValueLength;
    ur->fieldId = ufdl->fieldId;
    memcpy( ur->name, ufdl->fieldName, strlen(ufdl->fieldName) );
    if( ! StringIsNull(ufdl->comment) )
        memcpy( ur->comment, ufdl->comment, strlen(ufdl->comment) );
    
    LOG( ud->iLogLevel, ud->logFile, "Field %d, [%s][%d][%s]", ur->fieldId, ur->name, ur->length, ur->value );
    
    if( ud->result == NULL )
    {
        ud->result = ur;
        ud->resultTail = ur;
    }
    else
    {
        ud->resultTail->next = ur;
        ud->resultTail = ur;
    }
    LOG( ud->iLogLevel, ud->logFile, "域 %d end.", fieldId );
    return 0;
}

int Upkg8583( UpkgDef *ud )
{
    LOG( ud->iLogLevel, ud->logFile, "%s begin...pkgId=[%s],pkgFile=[%s], buffer: ",__FUNCTION__, ud->pkgDef->pkgId, ud->pkgFile );
    LOGHEX( 1, ud->logFile, ud->package, ud->packageLen );
    int offset = 0, i = 0, ret = 0;
    ret = Unpack8583Field( ud, &offset, 0 );
    if( ret != 0 )
    {
        LOG( 1, ud->logFile, "解包 %d 域失败", 0 );
        return -12;
    }
    ret = Unpack8583Field( ud, &offset, 1 );
    if( ret != 0 )
    {
        LOG( 1, ud->logFile, "解包 %d 域失败", 1 );
        return -12;
    }
    if( StringIsNull( ud->bitmap ) )
    {
        sprintf( ud->err, "位图域未配置或配置不正确\0" );
        LOG( 1, ud->logFile, ud->err );
        return -11;
    }
    for( i = 1; i<strlen(ud->bitmap); i++ )
    {
        if( ud->bitmap[i] == '1' )
        {
            ret = Unpack8583Field( ud, &offset, i+1 );
            if( ret != 0 )
            {
                LOG( 1, ud->logFile, "解包 %d 域失败", i+1 );
                return -12;
            }
        }   
    }
    
    LOG( ud->iLogLevel, ud->logFile, "%s end...", __FUNCTION__ );
    return 0; 
}

int UpkgFIXED( UpkgDef *ud )
{
    LOG( ud->iLogLevel, ud->logFile, "%s begin...pkgId=[%s],pkgFile=[%s], buffer: ",__FUNCTION__, ud->pkgDef->pkgId, ud->pkgFile );
    LOGHEX( 1, ud->logFile, ud->package, ud->packageLen );
    
    int offset = 0,  fieldValueActualLength = 0;
    UpkgFldDefList *ufdl;
    char caTemp[1024+1];
    for( ufdl=ud->fieldHead; ufdl; ufdl=ufdl->next )
    {
        LOG( ud->iLogLevel, ud->logFile , "域: %d ...", ufdl->fieldId );
        fieldValueActualLength = ufdl->eFieldCoding == BCD ? ((int)( (ufdl->fieldLength+1) / 2 ) ) : 
                            ( ufdl->eFieldCoding == ASCIIHEX ? ufdl->fieldLength*2 : ufdl->fieldLength );
        LOG( ud->iLogLevel, ud->logFile, "域值长度[%d],域值编码类型[%s],取域值报文实际长度=[%d]",
            ufdl->fieldLength, ufdl->fieldCoding, fieldValueActualLength );
        if( offset+fieldValueActualLength > ud->packageLen )
            fieldValueActualLength = ud->packageLen - offset;
        memset( caTemp, 0, sizeof(caTemp) );
        memcpy( caTemp, ud->package+offset, fieldValueActualLength );
        offset += fieldValueActualLength;
        LOG( ud->iLogLevel, ud->logFile, "域值:" );
        LOGHEX( ud->iLogLevel, ud->logFile, caTemp, fieldValueActualLength );
        UpkgResultList *ur = NewUpkgResultList();
        int ret = ConvertFieldValue( caTemp, fieldValueActualLength, ufdl, ur, ud );
        if( ret != 0 )
        {
            sprintf( ud->err, "域值格式转换失败" );
            LOG( 1, ud->logFile, ud->err );
            DeleteUpkgResultList( ur );
            return -44;
        }
        ur->length = ufdl->fieldLength;
        ur->fieldId = ufdl->fieldId;
        memcpy( ur->name, ufdl->fieldName, strlen(ufdl->fieldName) );
        if( ! StringIsNull(ufdl->comment) )
            memcpy( ur->comment, ufdl->comment, strlen(ufdl->comment) );
        
        LOG( ud->iLogLevel, ud->logFile, "Field %d, [%s][%d][%s]", ur->fieldId, ur->name, ur->length, ur->value );
        
        if( ud->result == NULL )
        {
            ud->result = ur;
            ud->resultTail = ur;
        }
        else
        {
            ud->resultTail->next = ur;
            ud->resultTail = ur;
        }     
        LOG( ud->iLogLevel, ud->logFile , "域: %d end.", ufdl->fieldId );
        if( offset >= ud->packageLen )
            break;  
    }
    
    LOG( ud->iLogLevel, ud->logFile, "%s end...", __FUNCTION__ );
    return 0;    
}

int UpkgTlv( UpkgDef *ud )
{
    LOG( ud->iLogLevel, ud->logFile, "%s begin...pkgId=[%s],pkgFile=[%s], buffer: ",__FUNCTION__, ud->pkgDef->pkgId, ud->pkgFile );
    LOGHEX( 1, ud->logFile, ud->package, ud->packageLen );
    
    int offset = 0, count = 0;
    while( offset < ud->packageLen )
    {
        char caTagBuf[4+1];
        char caTagLenBuf[16+1];
        char caTagLenByte[16+1];
        char caTagName[128+1];
        char caFmtFunc[128+1];
        int iTagLenLen = 0;
        char caTagValue[1024+1];
        int iTagLen = 0;
        memset( caTagBuf, 0, sizeof(caTagBuf) );
        memset( caTagLenBuf, 0, sizeof(caTagLenBuf) );
        memset( caTagLenByte, 0, sizeof(caTagLenByte) );
        memset( caTagValue, 0, sizeof(caTagValue) );
        memset( caTagName, 0, sizeof(caTagName) );
        memset( caFmtFunc, 0, sizeof(caFmtFunc) );
        count ++;
        /** tag **/
        memcpy( caTagBuf, ud->package+offset, 2 );
        offset += 2;
        if( caTagBuf[1] == 'f' || caTagBuf[1] == 'F' )
        {
            memcpy( caTagBuf, ud->package+offset-2, 4 );
            offset += 2;
        }
        
        /** length **/
        memcpy( caTagLenBuf, ud->package+offset, 2 );
        offset += 2;
        AsciiHexToByte( caTagLenBuf, 2, caTagLenByte, &iTagLenLen );
        if( caTagLenByte[0] >= 0x80 )
        {
            iTagLenLen = caTagLenByte[0] - 0x80;
            memset( caTagLenBuf, 0, sizeof(caTagLenBuf) );
            memcpy( caTagLenBuf, ud->package+offset, iTagLenLen*2 );
            offset += iTagLenLen * 2;
            AsciiHexToInt( caTagLenBuf, iTagLenLen*2, &iTagLen );            
        }
        else
        {
            ByteHexToInt( caTagLenByte, 1, &iTagLen );
        }
        
        /** value **/
        if( offset+iTagLen*2 > ud->packageLen )
        {
            sprintf( ud->err, "长度不足, offset=[%d], iTagLen=[%d], packageLen=[%d]", offset, iTagLen * 2, ud->packageLen );
            LOG( 1, ud->logFile, ud->err );
            return -55;
        }
        memcpy( caTagValue, ud->package+offset, iTagLen * 2 );
        offset += iTagLen * 2;      
        
        UpkgResultList *ur = NewUpkgResultList();
        ur->fieldId = count;
        ur->length = iTagLen;
        GetIniConfig( ud->pkgFile, caTagBuf, "name", caTagName );
        GetIniConfig( ud->pkgFile, caTagBuf, "fmtFunc", caFmtFunc );
        if( ! StringIsNull( caFmtFunc ) )
        {
            int funcIdx = GetConvertFunctionIdx( caFmtFunc, 1 );
            if( funcIdx >= 0 )
                functionMap[funcIdx].ConvertStringFunction( caTagValue, iTagLen*2, ur->value, &ur->length );
            else
                memcpy( ur->value, caTagValue, iTagLen*2 );
        }
        else
            memcpy( ur->value, caTagValue, iTagLen*2 );
        memcpy( ur->name, caTagBuf, strlen(caTagBuf) );
        if( ! StringIsNull( caTagName ) )
            memcpy( ur->nameZh, caTagName, strlen(caTagName) );
        LOG( ud->iLogLevel, ud->logFile, "[%d].TAG=[%s][%s], length=[%d], value[%s][%s]", 
                count, ur->name, ur->nameZh, iTagLen, caTagValue, ur->value );
        
        if( ud->result == NULL )
        {
            ud->result = ur;
            ud->resultTail = ur;
        }
        else
        {
            ud->resultTail->next = ur;
            ud->resultTail = ur;
        }       
    }           
    
    LOG( ud->iLogLevel, ud->logFile, "%s end...", __FUNCTION__ );
    return 0;    
}

int UpkgMain( UpkgDef *ud )
{
    int ret;
    if( ud == NULL )
        return -1;
    if( StringIsNull(ud->pkgFile) )
    {
        memset( ud->err, 0, sizeof(ud->err) );
        sprintf( ud->err, "pkgFile is null" );
        return -2;
    }
    if( StringIsNull(ud->orgMsg) )
    {
        sprintf( ud->err, "orgMsg is NULL\0" );
        return -3;
    }
    ret = LoadPkgFile( ud->pkgFile, ud );
    if( ret != 0 )
    {
        printf( "LoadPkgFile [%s]失败, 请检查\n", ud->pkgFile );
        return ret;
    }
    /*** 预处理 ***/
    if( ! StringIsNull(ud->pkgDef->pkgMsgFmt) )
    {
        int functionIdx = GetConvertFunctionIdx( ud->pkgDef->pkgMsgFmt, 1 );
        if( functionIdx < 0 )
        {
            sprintf( ud->err, "pkgMsgFmt格式化函数不存在 [%s]", ud->pkgDef->pkgMsgFmt );
            return -7;
        }
        ret = functionMap[functionIdx].ConvertStringFunction( ud->orgMsg, strlen(ud->orgMsg), ud->package, &(ud->packageLen));
        if( ret != 0 )
        {
            sprintf( ud->err, "输入内容格式转换失败" );
            return -6;        
        }
    }
    else
    {
        memcpy( ud->package, ud->orgMsg, strlen(ud->orgMsg) );
        ud->packageLen = strlen( ud->package );
    }
    switch( ud->pkgDef->ePkgType )
    {
        case ISO8583:
            ret = Upkg8583( ud );
            break;
        case FIXED:
            ret = UpkgFIXED( ud );            
            break;
        case TLV:
            ret = UpkgTlv( ud );
            break;
        case SEP:
            sprintf( ud->err, "分隔符暂不支持\0" );
            ret = -4;
            break;
        default:
            sprintf( ud->err, "错误的pkgType [%s]\0", ud->pkgDef->pkgType );
            ret = -5;
            break;
    }
    if( ret != 0 )
    {
        LOG( ud->iLogLevel, ud->logFile, "ret = %d, %s", ret, ud->err );
        return ret;
    }
    return 0;   
}

int Unpack( char *cpMsg, int iInLen, char *pkgFile, char *cpOut )
{
    int ret = 0;
    UpkgDef *ud = NewUpkgDef();
    memcpy( ud->orgMsg, cpMsg, strlen(cpMsg) );
    memcpy( ud->pkgFile, pkgFile, strlen(pkgFile) );
    ret = UpkgMain( ud );
    if( ret != 0 )
    {
        printf( "%s\n", ud->err );
    }
    UpkgResultListToString( ud->result, cpOut );
    LOG( 1, ud->logFile, "\n%s",cpOut );
    DeleteUpkgDef( ud );
    return 0;
}