#include "upkg.h"
#include "list.h"
#include "util.h"
#include "errcode.h"



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
            sprintf( ud->err, "eFieldLengthCoding无效, [%s][%d]\0", ufdl->fieldLengthCoding, ufdl->eFieldLengthCoding );
            LOG( 1, ud->logFile, ud->err );
            return FAILED;
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
            sprintf( ud->err,  "fieldLengthFmt无效,[%s]", ufdl->fieldLengthFmt );
            LOG(1 , ud->logFile, ud->err );
            return FAILED;
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
            sprintf( ud->err, "fieldFmt域格式转换函数不存在[%s]", fmtFunction );
            LOG( 1, ud->logFile, ud->err );
            return FieldFmtFunctionInvalid ;
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
            LOG( 1, ud->logFile, "子报文处理失败, %s. 查看[%s]", subUd->err, subUd->logFile );
            ur->subfld = CopyUpkgResultList( subUd->result ) ;
            DeleteUpkgDef( subUd );
        }
        LOG( ud->iLogLevel, ud->logFile, "子报文结束...." );
    }
    return SUCCESS;
}

int Unpack8583Field( UpkgDef *ud, int *offset, int fieldId )
{
    if( ud == NULL ) 
            return UPKGDEFISNULL;
    LOG( ud->iLogLevel, ud->logFile, "域 %d: ", fieldId );
    UpkgFldDefList* ufdl = SearchFieldListByFieldId( ud->fieldHead, fieldId );
    if( ufdl == NULL )
    {
        sprintf( ud->err, "[%s][%s]取域[%d]配置失败,\0", ud->pkgFile, ud->pkgDef->pkgId, fieldId );
        LOG( 1, ud->logFile, ud->err );
        return FieldDefNotExist;
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
        LOG( ud->iLogLevel, ud->logFile, "域长度类型:[%s],域长度编码类型[%s]，域长度的实际长度=[%d], 域长度格式化函数[%s],域长度: ", 
                ufdl->fieldLengthType, ufdl->fieldLengthCoding, fieldLengthActualLength, ufdl->fieldLengthFmt );
        memset( caTemp, 0, sizeof(caTemp) );
        if( (*offset) + fieldLengthActualLength > ud->packageLen )
        {
            sprintf( ud->err, "offset=[%d],报文长度[%d], 需要的长度=%d\0", (*offset), ud->packageLen, fieldLengthActualLength  );
            LOG( 1, ud->logFile, ud->err );
            return PackageLengthNotEnough;
        }
        memcpy( caTemp, ud->package+(*offset), fieldLengthActualLength );
        (*offset) = (*offset) + fieldLengthActualLength;
        LOGHEX( ud->iLogLevel, ud->logFile, caTemp, fieldLengthActualLength );
        fieldValueLength = ConvertFieldValueLength( caTemp,fieldLengthActualLength, ufdl, ud );
        if( fieldValueLength < 0 )
        {
            /*sprintf( ud->err, "域值长度转换失败, ret=%d\0", fieldValueLength );*/
            LOG( 1, ud->logFile, ud->err );
            return ConvertFieldValueLengthFail;
        }
        LOG( ud->iLogLevel, ud->logFile, "域值长度: [%d]", fieldValueLength );
    }
    fieldValueActualLength = ufdl->eFieldCoding == BCD ? ((int)( (fieldValueLength+1) / 2 ) ) : 
                            ( ufdl->eFieldCoding == ASCIIHEX ? fieldValueLength*2 : fieldValueLength );
    memset( caTemp, 0, sizeof(caTemp) );
    LOG( ud->iLogLevel, ud->logFile, "域值长度[%d],域值编码类型[%s],取域值报文实际长度=[%d],域值格式化函数[%s]",
         fieldValueLength, ufdl->fieldCoding, fieldValueActualLength, ufdl->fieldFmt );
    if( (*offset) + fieldValueActualLength > ud->packageLen )
    {
        sprintf( ud->err, "offset=[%d],报文长度[%d]不足,需要的长度=%d\0", (*offset), ud->packageLen, fieldValueActualLength  );
        LOG( 1, ud->logFile, ud->err );
        return PackageLengthNotEnough;
    }
    memcpy( caTemp, ud->package+(*offset), fieldValueActualLength );
    (*offset) = (*offset) + fieldValueActualLength;
    LOG( ud->iLogLevel, ud->logFile, "域值:" );
    LOGHEX( ud->iLogLevel, ud->logFile, caTemp, fieldValueActualLength );
    UpkgResultList *ur = NewUpkgResultList();
    int ret = ConvertFieldValue( caTemp, fieldValueActualLength, ufdl, ur, ud);
    if( ret != 0 )
    {
        /*sprintf( ud->err, "域值格式转换失败,ret=%d\0", ret );*/
        LOG( 1, ud->logFile, ud->err );
        DeleteUpkgResultList( ur );
        return ret;
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
    return SUCCESS;
}

int Upkg8583( UpkgDef *ud )
{
    LOG( ud->iLogLevel, ud->logFile, "%s begin...pkgId=[%s],pkgFile=[%s], buffer: ",__FUNCTION__, ud->pkgDef->pkgId, ud->pkgFile );
    LOGHEX( 1, ud->logFile, ud->package, ud->packageLen );
    int offset = 0, i = 0, ret = 0;
    ret = Unpack8583Field( ud, &offset, 0 );
    if( ret != 0 )
    {
        LOG( 1, ud->logFile, "解包 %d 域失败, ret=%d", 0, ret );
        return ret;
    }
    ret = Unpack8583Field( ud, &offset, 1 );
    if( ret != 0 )
    {
        LOG( 1, ud->logFile, "解包 %d 域失败, ret=%d", 1, ret );
        return ret;
    }
    if( StringIsNull( ud->bitmap ) )
    {
        sprintf( ud->err, "位图域未配置或配置不正确\0" );
        LOG( 1, ud->logFile, ud->err );
        return BitMapNotExist;
    }
    for( i = 1; i<strlen(ud->bitmap); i++ )
    {
        if( ud->bitmap[i] == '1' )
        {
            ret = Unpack8583Field( ud, &offset, i+1 );
            if( ret != 0 )
            {
                LOG( 1, ud->logFile, "解包 %d 域失败, ret=%d", i+1, ret );
                return ret;
            }
        }   
    }
    
    LOG( ud->iLogLevel, ud->logFile, "%s end...", __FUNCTION__ );
    return SUCCESS; 
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
        LOG( ud->iLogLevel, ud->logFile, "域值长度[%d],域值编码类型[%s],取域值报文实际长度=[%d],域值格式化函数[%s]",
            ufdl->fieldLength, ufdl->fieldCoding, fieldValueActualLength, ufdl->fieldFmt );
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
            /*sprintf( ud->err, "域值格式转换失败\0" );*/
            LOG( 1, ud->logFile, ud->err );
            DeleteUpkgResultList( ur );
            return ret;
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
        {
            LOG( ud->iLogLevel, ud->logFile, "offset[%d] >= packageLen[%d], over.", offset, ud->packageLen );
            break;  
        }
    }
    
    LOG( ud->iLogLevel, ud->logFile, "%s end...", __FUNCTION__ );
    return SUCCESS;    
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
            sprintf( ud->err, "长度不足, offset=[%d], iTagLen=[%d], packageLen=[%d]\0", offset, iTagLen * 2, ud->packageLen );
            LOG( 1, ud->logFile, ud->err );
            return TlvPackageLengthNotEnough;
        }
        memcpy( caTagValue, ud->package+offset, iTagLen * 2 );
        offset += iTagLen * 2;      
        
        UpkgResultList *ur = NewUpkgResultList();
        ur->fieldId = count;
        ur->length = iTagLen;
        GetIniConfig( ud->pkgFile, caTagBuf, "name", caTagName, sizeof(caTagName)-1 );
        GetIniConfig( ud->pkgFile, caTagBuf, "fmtFunc", caFmtFunc, sizeof(caFmtFunc)-1 );
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
        LOG( ud->iLogLevel, ud->logFile, "[%d].TAG=[%s][%s], length=[%d], value[%s][%s][%s]", 
                count, ur->name, ur->nameZh, iTagLen, caTagValue, ur->value, caFmtFunc );
        
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
    return SUCCESS;    
}

int UpkgMain( UpkgDef *ud )
{
    int ret;
    if( ud == NULL )
        return UPKGDEFISNULL;
    if( StringIsNull(ud->pkgFile) )
    {
        memset( ud->err, 0, sizeof(ud->err) );
        sprintf( ud->err, "pkgFile is null\0" );
        return PKGFILEISNULL;
    }
    if( StringIsNull(ud->orgMsg) )
    {
        sprintf( ud->err, "orgMsg is NULL\0" );
        return ORGMSGISNULL;
    }
    ret = LoadPkgFile( ud->pkgFile, ud );
    if( ret != 0 )
    {
        printf( "LoadPkgFile [%s]失败, 请检查,ret=%d\n", ud->pkgFile, ret );
        return ret;
    }
    /*** 预处理 ***/
    if( ! StringIsNull(ud->pkgDef->pkgMsgFmt) )
    {
        int functionIdx = GetConvertFunctionIdx( ud->pkgDef->pkgMsgFmt, 1 );
        if( functionIdx < 0 )
        {
            sprintf( ud->err, "pkgMsgFmt格式化函数不存在 [%s]\0", ud->pkgDef->pkgMsgFmt );
            return PackageFmtFunctionInvalid;
        }
        ret = functionMap[functionIdx].ConvertStringFunction( ud->orgMsg, strlen(ud->orgMsg), ud->package, &(ud->packageLen));
        if( ret != 0 )
        {
            sprintf( ud->err, "输入内容格式转换失败\0" );
            return PackageFmtFail;        
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
            ret = NotSupportFunction;
            break;
        default:
            sprintf( ud->err, "错误的pkgType [%s]\0", ud->pkgDef->pkgType );
            ret = PKGTYPEINVALID;
            break;
    }
    if( ret != 0 )
    {
        LOG( ud->iLogLevel, ud->logFile, "ret = %d, %s", ret, ud->err );
        return ret;
    }
    return SUCCESS;   
}

int Unpack( char *cpMsg, int iInLen, char *pkgFile, char *cpOut )
{
    int ret = 0;
    UpkgDef *ud = NewUpkgDef();
    memcpy( ud->orgMsg, cpMsg, iInLen );
    memcpy( ud->pkgFile, pkgFile, strlen(pkgFile) );
    ret = UpkgMain( ud );
    if( ret != 0 )
    {
        printf( "ret = %d, %s\n", ret, ud->err );
    }
    UpkgResultListToString( ud->result, cpOut );
    LOG( ud->iLogLevel, ud->logFile, "\n%s",cpOut );
    DeleteUpkgDef( ud );
    return SUCCESS;
}