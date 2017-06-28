

static int SendUbloxRequest(const BYTE *pUbloxPacket, DWORD dwSendLength);
static void EncodeToUbloxMessage(ubloxMsgType sMsgType, const VOID *pData, BYTE *pOutData,INT iDataSize);
static void CalcChecksum(BYTE *ck, const BYTE* pCalcData, INT iDataLen);


int WriteUbxCfgRst(WORD type, BYTE chResetType)
{
	INT iSendErrCnt = 0;

	UBLOX_CFG_RST_PAYLOAD Payload;
	memset(&Payload, 0, sizeof(Payload));

	Payload.navBbrMask = type;
	Payload.resetMode = chResetType;

	BYTE pUbloxPacket[LENGTH_CFG_RST];
	memset(pUbloxPacket,0,LENGTH_CFG_RST);
	
	EncodeToUbloxMessage(TYPE_CFG_RST, &Payload, pUbloxPacket, sizeof(Payload));

	return SendUbloxRequest(pUbloxPacket, LENGTH_CFG_RST);
}

int WriteUbxCfgRate(void)
{
	INT iSendErrCnt = 0;
	// CFG_RATE payload content
	UBLOX_CFG_RATE_PAYLOAD Payload;
	memset(&Payload, 0, sizeof(Payload));

	Payload.measRate = MEASUREMENT_RATE;   // 1000ms
	Payload.navRate = NAVI_RATE;		// 1 cycle
	Payload.timeRef = TIME_ALIGNMENT;	// 1,gps time

	// allocate memory for ublox packet
	BYTE pUbloxPacket[LENGTH_CFG_RATE];
	memset(pUbloxPacket,0,LENGTH_CFG_RATE);
	// encode
	EncodeToUbloxMessage(TYPE_CFG_RATE, &Payload, pUbloxPacket, sizeof(Payload));

	return  SendUbloxRequest(pUbloxPacket, LENGTH_CFG_RATE);
}

int WriteUbxCfgMsg(ubloxMsgType sMsgType, BYTE rate)
{
	INT iSendErrCnt = 0;

	UBLOX_CFG_MSG_PAYLOAD Payload;
	memset(&Payload, 0, sizeof(Payload));

	Payload.msgClass = sMsgType.clsID;
	Payload.msgID = sMsgType.msgID;
	Payload.rate = rate;  	// send rate

	// allocate memory for ublox packet
	BYTE pUbloxPacket[LENGTH_CFG_MSG];
	memset(pUbloxPacket,0,LENGTH_CFG_MSG);

	EncodeToUbloxMessage(TYPE_CFG_MSG, &Payload, pUbloxPacket, sizeof(Payload));

	return SendUbloxRequest(pUbloxPacket, LENGTH_CFG_MSG);
}

int WriteUbxPollCFGNAVX5(void)
{
	INT iSendErrCnt = 0;
	// allocate memory for ublox packet
	BYTE pUbloxPacket[FIXLENGTH_NOPAYLOAD];
	memset(pUbloxPacket, 0, FIXLENGTH_NOPAYLOAD);

	// encode
	EncodeToUbloxMessage(TYPE_CFG_NAVX5, NULL, pUbloxPacket, 0);

	return  SendUbloxRequest(pUbloxPacket, FIXLENGTH_NOPAYLOAD);
}

int WriteUbxSendCFGNAVX5(USHORT wkn)
{
	INT iSendErrCnt = 0;

	UBLOX_CFG_NAVX5_PAYLOAD Payload;
	memset(&Payload, 0, sizeof(Payload));

	Payload.version= 0x0000;			// message version.current verison is 0.
	Payload.mask1 = 0xFFFF;             // bit14:aop;bit13:ppp;bit9:wknRoll;bit6:3dfix;bit3:minCno;bit2:minMax.
	Payload.reserved0 = 0x00000003;
	Payload.reserved1 = 0x03;
	Payload.reserved2 = 0x02;
	Payload.minSVs = 0x03; 				// minimum number of satellites for navigation.
	Payload.maxSVs = 0x10;				// maximum number of satellites for navigation.
	Payload.minCNO = 0x07;       		// minimum satellite signal level for navigation.
	Payload.reserved5 = 0x00;
	Payload.iniFix3D = 0x00;				// inital fix must be 3D flag(0=false/1=true).
	Payload.reserved6 = 0x01;
	Payload.reserved7 = 0x00;
	Payload.reserved8 = 0x00;
	Payload.wknRollover = wkn;			// Gps week ro;;over nnumber.from this week up to 1024 weeks after this week.
	Payload.reserved9 = 0x00000000;
	Payload.reserved10 = 0x01;
	Payload.reserved11 = 0x01;
	Payload.usePPP = 0x00;				// use precise point positioning flag(0=false/1=true).
	Payload.useAOP = 0x00;				// AssistNow Autonomous.
	Payload.reserved12 = 0x00;
	Payload.reserved13 = 0x64;
	Payload.aopOrbMaxErr = 0x0078;		// maximum acceptable(modelled) AssistNow Autonomous orbit error(valid range =5...1000,or 0=reset to firmware default).
	Payload.reserved3 = 0x00000000;
	Payload.reserved4 = 0x00000000;

	// allocate memory for ublox packet
	BYTE pUbloxPacket[LENGTH_CFG_NAVX5];
	memset(pUbloxPacket, 0, LENGTH_CFG_NAVX5);

	EncodeToUbloxMessage(TYPE_CFG_NAVX5, &Payload, pUbloxPacket, sizeof(Payload));

	return SendUbloxRequest(pUbloxPacket, LENGTH_CFG_NAVX5);
}

int WriteUbxMonVer(void)
{
	INT iSendErrCnt = 0;
	// allocate memory for ublox packet
	BYTE pUbloxPacket[FIXLENGTH_NOPAYLOAD];
	memset(pUbloxPacket, 0, FIXLENGTH_NOPAYLOAD);
	// encode
	EncodeToUbloxMessage(TYPE_MON_VER, NULL, pUbloxPacket, 0);

	return SendUbloxRequest(pUbloxPacket, FIXLENGTH_NOPAYLOAD);
}

static int SendUbloxRequest(const BYTE *pUbloxPacket, DWORD dwSendLength)
{
	DWORD dwWriteSize = 0;
	// null poINT protect
	if (NULL == pUbloxPacket) {
		return NC_FALSE;
	}

	// write command to serial port
	Write(pUbloxPacket, dwSendLength, &dwWriteSize);

	return (dwSendLength == dwWriteSize) ? NC_TRUE : NC_FALSE;
}

static void EncodeToUbloxMessage(ubloxMsgType sMsgType, const VOID *pData, BYTE *pOutData,INT iDataSize)
{
	BYTE head[6];
	BYTE Cksum[2] = {0,0};
	// header of ublox packet
	head[0] = 0xB5;
	head[1] = 0x62;
	head[2] = sMsgType.clsID;
	head[3] = sMsgType.msgID;
	head[4] = (BYTE)iDataSize;
	head[5] = (BYTE)(iDataSize >> 8);

	memcpy(pOutData, head, sizeof(head));

	// pData is the payload content
	if ((pData != NULL) && (iDataSize > 0)) {
		memcpy(&pOutData[sizeof(head)], pData, iDataSize);
	}

	// calculate checksum without payload
	CalcChecksum(Cksum, &pOutData[2], iDataSize+sizeof(head)-2);
	memcpy(&pOutData[iDataSize+sizeof(head)], Cksum, sizeof(Cksum));
	// finish encode message
}

static void CalcChecksum(BYTE *ck, const BYTE* pCalcData, INT iDataLen)
{
	if ((NULL == pCalcData) || (NULL == ck)) {
		return;
	}

	DWORD dwCK_A = 0;
	DWORD dwCK_B = 0;
	INT i = 0;
	// calculate check sum
	for (i = 0; i < iDataLen; i++) {
		dwCK_A =  dwCK_A + pCalcData[i];
		dwCK_B =  dwCK_B + dwCK_A;
	}

	ck[0] = dwCK_A & 0XFF;
	ck[1] = dwCK_B & 0XFF;
}