#include <string.h>
#include <stdint.h>

struct UBXMsgType
{
	uint8_t clsID;
	uint8_t msgID;
};

static int SendUbloxRequest(const uint8_t *pUbloxPacket, uint32_t dwSendLength);
static void EncodeToUbloxMessage(ubloxMsgType sMsgType, const void *pData, uint8_t *pOutData,int iDataSize);
static void CalcChecksum(uint8_t *ck, const uint8_t* pCalcData, int iDataLen);
static int WriteUbxCfgMsg(struct UBXMsgType sMsgType, uint8_t rate);
static int WriteUbxPollCFGNAVX5(void);
static int WriteUbxSendCFGNAVX5(uint16_t wkn);
static int WriteUbxCfgRst(uint16_t type, uint8_t chResetType);


int WriteUbxCfgRate(void)
{
	int iSendErrCnt = 0;

	UBLOX_CFG_RATE_PAYLOAD Payload;
	memset(&Payload, 0, sizeof(Payload));

	Payload.measRate = MEASUREMENT_RATE;
	Payload.navRate = NAVI_RATE;
	Payload.timeRef = TIME_ALIGNMENT;

	uint8_t pUbloxPacket[LENGTH_CFG_RATE] = {0};

	EncodeToUbloxMessage(TYPE_CFG_RATE, &Payload, pUbloxPacket, sizeof(Payload));

	return  SendUbloxRequest(pUbloxPacket, LENGTH_CFG_RATE);
}

int SetGpsVerion(void)
{
	int iSendErrCnt = 0;

	uint8_t pUbloxPacket[FIXLENGTH_NOPAYLOAD] = {0};
	EncodeToUbloxMessage(TYPE_MON_VER, NULL, pUbloxPacket, 0);

	return SendUbloxRequest(pUbloxPacket, FIXLENGTH_NOPAYLOAD);
}

int GpsNmeaSetting(int gstFlag)
{
	if (gstFlag) {
		return WriteUbxCfgMsg(TYPE_NMEA_GST, GPS_MSG_SEND_RATE);
	}

	return 0;
}

int ResetGpsReceiver(uint16_t type)
{
	uint8_t chResetType = CTRL_SOFTWARE_RESET_GPSONLY;
	return WriteUbxCfgRst(type, chResetType);
}

int BookUbxNAVTIMEUTC(int bBookFlag)
{
	if(1 == bBookFlag) {
		return WriteUbxCfgMsg(TYPE_NAV_TIMEUTC, GPS_MSG_SEND_RATE);
	}
	else {
		return WriteUbxCfgMsg(TYPE_NAV_TIMEUTC, GPS_MSG_NOSEND_RATE);
	}
}

int BookUbxCFGNAVX5(int bBookFlag)
{
	uint16_t wknRollover = WEEK_ROLLOVER;

	if(1 == bBookFlag){
		return WriteUbxSendCFGNAVX5(wknRollover);
	}
	else{
		return WriteUbxPollCFGNAVX5();
	}
}

static int WriteUbxCfgRst(uint16_t type, uint8_t chResetType)
{
	int iSendErrCnt = 0;

	UBLOX_CFG_RST_PAYLOAD Payload;
	memset(&Payload, 0, sizeof(Payload));

	Payload.navBbrMask = type;
	Payload.resetMode = chResetType;

	uint8_t pUbloxPacket[LENGTH_CFG_RST] = {0};
	
	EncodeToUbloxMessage(TYPE_CFG_RST, &Payload, pUbloxPacket, sizeof(Payload));

	return SendUbloxRequest(pUbloxPacket, LENGTH_CFG_RST);
}

static int WriteUbxCfgMsg(struct UBXMsgType sMsgType, uint8_t rate)
{
	int iSendErrCnt = 0;

	UBLOX_CFG_MSG_PAYLOAD Payload;
	memset(&Payload, 0, sizeof(Payload));

	Payload.msgClass = sMsgType.clsID;
	Payload.msgID = sMsgType.msgID;
	Payload.rate = rate;

	uint8_t pUbloxPacket[LENGTH_CFG_MSG] = {0};

	EncodeToUbloxMessage(TYPE_CFG_MSG, &Payload, pUbloxPacket, sizeof(Payload));

	return SendUbloxRequest(pUbloxPacket, LENGTH_CFG_MSG);
}

static int WriteUbxPollCFGNAVX5(void)
{
	int iSendErrCnt = 0;

	uint8_t pUbloxPacket[FIXLENGTH_NOPAYLOAD] = {0};

	EncodeToUbloxMessage(TYPE_CFG_NAVX5, NULL, pUbloxPacket, 0);

	return  SendUbloxRequest(pUbloxPacket, FIXLENGTH_NOPAYLOAD);
}

static int WriteUbxSendCFGNAVX5(uint16_t wkn)
{
	int iSendErrCnt = 0;

	UBLOX_CFG_NAVX5_PAYLOAD Payload;
	memset(&Payload, 0, sizeof(Payload));

	Payload.version= 0x0000;
	Payload.mask1 = 0xFFFF;
	Payload.reserved0 = 0x00000003;
	Payload.reserved1 = 0x03;
	Payload.reserved2 = 0x02;
	Payload.minSVs = 0x03;
	Payload.maxSVs = 0x10;
	Payload.minCNO = 0x07;
	Payload.reserved5 = 0x00;
	Payload.iniFix3D = 0x00;
	Payload.reserved6 = 0x01;
	Payload.reserved7 = 0x00;
	Payload.reserved8 = 0x00;
	Payload.wknRollover = wkn;
	Payload.reserved9 = 0x00000000;
	Payload.reserved10 = 0x01;
	Payload.reserved11 = 0x01;
	Payload.usePPP = 0x00;
	Payload.useAOP = 0x00;
	Payload.reserved12 = 0x00;
	Payload.reserved13 = 0x64;
	Payload.aopOrbMaxErr = 0x0078;
	Payload.reserved3 = 0x00000000;
	Payload.reserved4 = 0x00000000;

	uint8_t pUbloxPacket[LENGTH_CFG_NAVX5] = {0};

	EncodeToUbloxMessage(TYPE_CFG_NAVX5, &Payload, pUbloxPacket, sizeof(Payload));

	return SendUbloxRequest(pUbloxPacket, LENGTH_CFG_NAVX5);
}

static int SendUbloxRequest(const uint8_t *pUbloxPacket, uint32_t dwSendLength)
{
	uint32_t dwWriteSize = 0;

	if (NULL == pUbloxPacket) {
		return -1;
	}

	dwWriteSize = GetGPSComDevice()->write(pUbloxPacket, dwSendLength);

	return (dwSendLength == dwWriteSize) ? 0 : -1;
}

static void EncodeToUbloxMessage(ubloxMsgType sMsgType, const void *pData, uint8_t *pOutData,int iDataSize)
{
	uint8_t head[6];
	uint8_t Cksum[2] = {0,0};

	head[0] = 0xB5;
	head[1] = 0x62;
	head[2] = sMsgType.clsID;
	head[3] = sMsgType.msgID;
	head[4] = (uint8_t)iDataSize;
	head[5] = (uint8_t)(iDataSize >> 8);

	memcpy(pOutData, head, sizeof(head));

	if ((pData != NULL) && (iDataSize > 0)) {
		memcpy(&pOutData[sizeof(head)], pData, iDataSize);
	}

	CalcChecksum(Cksum, &pOutData[2], iDataSize+sizeof(head)-2);
	memcpy(&pOutData[iDataSize+sizeof(head)], Cksum, sizeof(Cksum));
}

static void CalcChecksum(uint8_t *ck, const uint8_t* pCalcData, int iDataLen)
{
	if ((NULL == pCalcData) || (NULL == ck)) {
		return;
	}

	uint32_t dwCK_A = 0;
	uint32_t dwCK_B = 0;
	int i = 0;

	for (i = 0; i < iDataLen; i++) {
		dwCK_A =  dwCK_A + pCalcData[i];
		dwCK_B =  dwCK_B + dwCK_A;
	}

	ck[0] = dwCK_A & 0XFF;
	ck[1] = dwCK_B & 0XFF;
}
