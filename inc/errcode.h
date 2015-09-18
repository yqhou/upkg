#ifndef __ERR_CODE_H__
#define __ERR_CODE_H__

typedef enum EnumErrCode
{
	FAILED = -1,
	SUCCESS = 0,
	PKGFILEISNULL = 1,
	UPKGDEFISNULL, /*2*/
	GETPKGIDFAIL, /*3*/
	GETPKGNAMEFAIL, /*4*/
	GETPKGTYPEFAIL, /*5*/
	PKGTYPEINVALID, /*6*/
	GETFIELDCOUNTFAIL, /*7*/
	FIELDCOUNTINVALID, /*8*/
	GETFIELDLISTFAIL, /*9*/
	FieldLengthCodingInvalid, /*10*/
	FieldLengthFmtInvalid , /*11*/
	FieldFmtFunctionInvalid, /*12*/
	FieldDefNotExist, /*13*/
	PackageLengthNotEnough, /*14*/
	ConvertFieldValueLengthFail, /*15*/
	ConvertFieldValueFail, /* 16*/
	BitMapNotExist, /* 17 */
	Unpack8583FieldFailed, /* 18 */
	TlvPackageLengthNotEnough, /* 19 */
	ORGMSGISNULL, /* 20 */
	PackageFmtFunctionInvalid, /* 21 */
	PackageFmtFail, /* 22 */
	NotSupportFunction /* 23 */,
    PkgFileTypeInvalid,
    LoadPkgFileError,
    PkgFileFormatError
} ErrCode;


#endif