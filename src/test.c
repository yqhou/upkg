#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "list.h"
#include "upkg.h"

int main(int argc, char **argv )
{
    char ch = 0;
    int ret = 0;
    char out[20480];
    char *msg = "00F3600003837160310031070003205020060000C00214196212263301004540034000000000498001450907100001383830313535383738393832313031353431313334383101419F2608852251D267A1271C9F101307010103A02000010A010000000000CCF27CAF9F37043266F78A9F36020054950500000000009A031406269C01009F02060000000049805F2A02015682027C009F1A0201569F03060000000000009F3303E0E1C89F34030200009F3501229F1E0830313638323638398408A0000003330101019F090200209F4104000145090013000029532036000021363130303030303030303030303034393830313536";
    
    char *cupsmsg = 
    "30 33 32 39 2E 02 30 33  32 39 30 30 30 31 30 30  30 30 20 20 20 34 38 30\
32 30 30 30 30 20 20 20  00 00 00 00 30 30 30 30  30 30 30 30 30 30 30 30 \
30 30 30 32 30 30 F2 38  44 C1 A8 E0 98 10 00 00  00 00 00 00 00 01 31 36 \
34 33 34 39 31 30 30 37  31 30 37 38 37 39 38 37  30 30 30 30 30 30 30 30 \
30 30 30 30 32 33 33 38  30 30 30 36 31 32 30 30  30 30 30 32 32 32 39 35 \
34 36 30 30 30 30 30 32  30 36 31 32 37 35 32 33  30 32 31 30 30 30 36 30 \
38 34 38 30 32 35 38 31  30 30 38 34 38 30 32 30  30 30 30 33 32 34 33 34 \
39 31 30 30 37 31 30 37  38 37 39 38 37 3D 31 39  31 31 31 30 31 31 30 30 \
30 36 36 36 35 30 30 30  31 38 38 34 34 34 34 32  39 31 31 32 30 30 34 34 \
31 38 39 38 34 34 30 31  37 35 32 33 30 30 30 36  B9 E3 D6 DD CA D0 BB A8 \
B6 BC C7 F8 D0 C2 BB AA  B6 AB C9 D0 B8 BB D6 D0  CE F7 B2 CD CC FC 20 20 \
20 20 20 20 20 20 20 20  31 35 36 C1 A8 D8 D7 68  8A 10 95 32 36 30 30 30 \
30 30 30 30 30 30 30 30  30 30 30 30 32 33 30 30  30 30 30 35 30 30 30 33 \
30 30 30 30 30 30 34 31  30 30 30 30 31 32 33 46  37 34 30 43 31   ";
    char *rejectmsg = 
    "30 33 32 39 ";
    memset( out, 0, sizeof(out) );
    
    printf( "%s:\n1. POS≤‚ ‘\n2. CUPS≤‚ ‘\n3. CUPS ”–æ‹æ¯¬Î\nINPUT: ", argv[0] );
    ch = getchar();
    fflush( stdin );
    switch( ch )
    {
        case '2':
            ret = Unpack( cupsmsg, strlen(cupsmsg), "../pkg/cups.pkg", out );
            break;
        case '1':
            ret = Unpack( msg, strlen(msg), "d:\\program\\git\\upkg\\pkg\\pos.pkg", out );
            break;
        case '3':
            ret = Unpack( rejectmsg, strlen(rejectmsg), "../pkg/cupsReject.pkg", out );
            break;
        case '4':
            ret = Unpack( cupsmsg, strlen(cupsmsg), "../pkg/cupsIni.pkg", out );
            break;
        default:
            sprintf( out, "wrong choice" );
            break;
    }
    printf( "OUTPUT:\n[%s]\n", out );
    system("pause");
    /**
    ConvertPkgFileType( "../pkg/cups.pkg", "../pkg/cupsSep.pkg", FileTypeSep );
    ConvertPkgFileType( "../pkg/cups.pkg", "../pkg/cupsIni.pkg", FileTypeIni );
    ConvertPkgFileType( "../pkg/pos.pkg", "../pkg/posSep.pkg", FileTypeSep );
    ConvertPkgFileType( "../pkg/pos.pkg", "../pkg/posIni.pkg", FileTypeIni );
    **/
}