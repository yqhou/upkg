#ifndef __LIST_H__
#define __LIST_H__

#include "upkg.h"

UpkgResultList *NewUpkgResultList();
void DeleteUpkgResult(UpkgResultList *ur);
UpkgResultList* CopyUpkgResultList( UpkgResultList *urFrom );
void PrintUpkgResultList( UpkgResultList *ur );


UpkgFldDefList *NewUpkgFldDefList();
void DeleteUpkgFldDefList(UpkgFldDefList *ufdn );
UpkgFldDefList *SearchFieldListByFieldId( UpkgFldDefList* head, int fieldId );
void UpkgResultListToString( UpkgResultList *ur, char *cpOut );

UpkgPkgDefNode *NewUpkgPkgDefNode();
void DeleteUpkgPkgDefNode( UpkgPkgDefNode *updn );
void UpkgFldDefToString( UpkgFldDefList *ufdl, char *cpOut, int fieldId );
void UpkgFldDefListToString( UpkgFldDefList *ufdl, char *cpOut );

UpkgDef *NewUpkgDef();
void DeleteUpkgDef( UpkgDef* ud);

#endif