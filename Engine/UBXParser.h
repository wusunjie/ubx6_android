#ifndef _UBX_PARSER_H
#define _UBX_PARSER_H

#include "Engine/GpsEngine.h"

extern void UBXParserInit(struct GpsDataCallbacks *cb);

extern int UBXPacketRead(void);

#endif
