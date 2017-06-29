#include <string.h>
#include <stdint.h>

#include "Device/GPSDeviceIF.h"

// CFG_ANT payload struct
typedef struct
{
	uint16_t flags;		// antenna flag mask	-------> set open check dection enabled
	uint16_t pins;		// antenna pin configuration

} UBLOX_CFG_ANT_PAYLOAD;

// CFG_RST struct
typedef struct
{
	uint16_t navBbrMask;	//clear backup data
					// 0x0000 Hotstart
					// 0x0001 WarmStart
					// 0xFFFF clodstart
	uint8_t resetMode;	// reset type
					// 0x00 HardWare reset (Watchdog) immediately
					// 0x01 controlled software reset
					// 0x02 controlled software reset (GPS only)
					// 0x04 Hardware reset (watchdog) after shutdown
					// 0x08 controlled GPS stop
					// ox09 controlled GPS start
	uint8_t reserved1;	// reserved

} UBLOX_CFG_RST_PAYLOAD;

// CFG_RATE struct
typedef struct
{
	uint16_t measRate;	//measurement rate
	uint16_t navRate;	//navi rate
	uint16_t timeRef;	//reference time alignment:0 = UTC time,1 = GPS time


} UBLOX_CFG_RATE_PAYLOAD;

// CFG_MSG struct
typedef struct
{
	uint8_t msgClass;	// Message class
	uint8_t msgID;	// Message identifier
	uint8_t rate;	// Message send rate

} UBLOX_CFG_MSG_PAYLOAD;

// CFG_NAVX5 struct
typedef struct
{
	uint16_t version;	// message version.current verison is 0.
	uint16_t mask1;           // bit14:aop;bit13:ppp;bit9:wknRoll;bit6:3dfix;bit3:minCno;bit2:minMax.
	uint32_t reserved0;
	uint8_t reserved1;
	uint8_t reserved2;
	uint8_t minSVs; 	// minimum number of satellites for navigation.
	uint8_t maxSVs;	// maximum number of satellites for navigation.
	uint8_t minCNO;       // minimum satellite signal level for navigation.
	uint8_t reserved5;
	uint8_t iniFix3D;	// inital fix must be 3D flag(0=false/1=true).
	uint8_t reserved6;
	uint8_t reserved7;
	uint8_t reserved8;
	uint16_t wknRollover;	// Gps week ro;;over nnumber.from this week up to 1024 weeks after this week.
	uint32_t reserved9;
	uint8_t reserved10;
	uint8_t reserved11;
	uint8_t usePPP;	// use precise point positioning flag(0=flagse/1=true).
	uint8_t useAOP;	// AssistNow Autonomous.
	uint8_t reserved12;
	uint8_t reserved13;
	uint16_t aopOrbMaxErr;	// maximum acceptable(modelled) AssistNow Autonomous orbit error(valid range =5...1000,or 0=reset to firmware default).
	uint32_t reserved3;
	uint32_t reserved4;
} UBLOX_CFG_NAVX5_PAYLOAD;

struct UBXMsgType
{
	uint8_t clsID;
	uint8_t msgID;
};

static const uint8_t UBLOX_SYNC1 = 0xB5;    // First synchronization character of UBX Protocol
static const uint8_t UBLOX_SYNC2 = 0x62;   // Second synchronization character of UBX Protocol

static const uint8_t UBX_CLASS_NAV = 0x01;	// NAV class
static const uint8_t UBX_CLASS_CFG = 0x06;	// CFG class
static const uint8_t UBX_CLASS_MON = 0x0A;	// MON class
static const uint8_t UBX_CLASS_ACK = 0x05;	// ACK class
static const uint8_t UBX_NMEA = 0xF0;   // NMEA standard message class 

static const uint8_t UBX_ID_CFG_RST = 0x04;	// set gps reset way
static const uint8_t UBX_ID_CFG_ANT = 0x13;	// set antenna open and short detection status
static const uint8_t UBX_ID_MON_HW = 0x09;	// get antenna status
static const uint8_t UBX_ID_MON_VER = 0x04;	// get GPS soft version
static const uint8_t UBX_ID_CFG_RATE = 0x08;	// set gps data rate
static const uint8_t UBX_ID_CFG_MSG = 0x01;	// set message msg
static const uint8_t UBX_ID_CFG_NAVX5 = 0x23;	// set message NAVX5
static const uint8_t UBX_ID_NAV_SOL = 0x06;	// navigation solution information
static const uint8_t UBX_ID_NAV_SVINFO = 0x30;	// space vehicle information
static const uint8_t UBX_ID_NAV_TIMEUTC = 0x21;	// space vehicle information
static const uint8_t UBX_ID_ACK_ACK = 0x01;	// space vehicle information
static const uint8_t UBX_ID_ACK_NAK = 0x00;	// space vehicle information

static const uint8_t UBX_ID_GST = 0x07;	// NMEA GST mssage ID

// definition for UBLOX_MON_HW_PAYLOAD-------status of the Antenna Supervisor State Machine

static const uint8_t ANTENNA_INIT = 0;		// antenna status is init	
static const uint8_t ANTENNA_DONTKNOW = 1;		// don't know antenna status
static const uint8_t ANTENNA_OK = 2;		// status is OK
static const uint8_t ANTENNA_SHORT = 3;		// GPS antenna status is short
static const uint8_t ANTENNA_OPEN = 4;		// GPS antenna status is open

static const int ASTATUS_BYTE_OFFSET = 20;	// byte offset of antenna status data in Payload Content
static const int VALID_BYTE_OFFSET = 19;	// byte offset of valid data in Payload Content

static const unsigned long FIXLENGTH_HEAD  = 6;	// SYNC0 SYNC1 class ID length
static const unsigned long FIXLENGTH_END   = 2;		// checksum
static const unsigned long FIXLENGTH_NOPAYLOAD  = 8;	// length of message without payload
static const unsigned long LENGTH_CFG_ANT = 12;	// CFG_ANT message length
static const unsigned long LENGTH_CFG_RST  = 12;	// CFG_RST message length
static const unsigned long LENGTH_CFG_MSG = 11;	// CFG_MSG message length
static const unsigned long LENGTH_CFG_NAVX5 = 48;	// CFG_NAVX5 message length
static const unsigned long LENGTH_CFG_RATE = 14;	// CFG_RATE message length
static const unsigned long LENGTH_ACK_ACK = 10;	// ACK_ACK message length
static const unsigned long LENGTH_ACK_NAK= 10;	// ACK_NAK message length
// ublox message type

static const struct UBXMsgType TYPE_CFG_ANT 	= {UBX_CLASS_CFG, UBX_ID_CFG_ANT};
static const struct UBXMsgType TYPE_CFG_RST 	= {UBX_CLASS_CFG, UBX_ID_CFG_RST};
static const struct UBXMsgType TYPE_CFG_RATE	= {UBX_CLASS_CFG, UBX_ID_CFG_RATE};
static const struct UBXMsgType TYPE_CFG_MSG 	= {UBX_CLASS_CFG, UBX_ID_CFG_MSG};
static const struct UBXMsgType TYPE_CFG_NAVX5 	= {UBX_CLASS_CFG, UBX_ID_CFG_NAVX5};
static const struct UBXMsgType TYPE_MON_HW 		= {UBX_CLASS_MON, UBX_ID_MON_HW};
static const struct UBXMsgType TYPE_MON_VER 	= {UBX_CLASS_MON, UBX_ID_MON_VER};
static const struct UBXMsgType TYPE_NAV_SOL 	= {UBX_CLASS_NAV, UBX_ID_NAV_SOL};
static const struct UBXMsgType TYPE_NAV_SVINFO 	= {UBX_CLASS_NAV, UBX_ID_NAV_SVINFO};
static const struct UBXMsgType TYPE_NAV_TIMEUTC	= {UBX_CLASS_NAV, UBX_ID_NAV_TIMEUTC};
static const struct UBXMsgType TYPE_ACK_ACK	= {UBX_CLASS_ACK, UBX_ID_ACK_ACK};
static const struct UBXMsgType TYPE_ACK_NAK	= {UBX_CLASS_ACK, UBX_ID_ACK_NAK};
//const ubloxMsgType TYPE_NMEA_GST 	= {UBX_NMEA, UBX_ID_GST};

//
static const uint8_t GPS_MSG_SEND_RATE = 1;		// GPS Message send rate
static const uint8_t GPS_MSG_NOSEND_RATE = 0;		// GPS Message send rate
static const uint16_t MEASUREMENT_RATE = 1000;	// 1000ms,Measurement Rate,Gps Measurements are taken every measRate milliseconds
static const uint16_t NAVI_RATE = 1;			// Navigation Rate, in number of measurement cycles
static const uint16_t TIME_ALIGNMENT = 1;   		// Alignment to reference time: 0 = UTC time,1 = GPS time
///

static const uint8_t HARDWARE_RESET = 0X00;			// hardware reset(Watchdog),no stop events are generated
static const uint8_t CTRL_SOFTWARE_RESET = 0X01;		// terminates all running processes in an orderly manner 
static const uint8_t CTRL_SOFTWARE_RESET_GPSONLY = 0X02;	// only restarts the GPS tasks,without reinitializeizing the full system
static const uint8_t HARDWARE_RESET_WATCHDOG = 0X04;	// hardware reset(Watchdog)after shutdown
static const uint8_t CTRL_GPS_STOP = 0X08;			// stop all GPS tasks
static const uint8_t CTRL_GPS_START = 0X09;			// stop all GPS tasks

static const uint16_t WEEK_ROLLOVER = 1878;

//UBlox message to book NMEA
static const struct UBXMsgType TYPE_NMEA_GGA = {0xF0, 0x00};
static const struct UBXMsgType TYPE_NMEA_GSV = {0xF0, 0x03};
static const struct UBXMsgType TYPE_NMEA_GSA = {0xF0, 0x02};
static const struct UBXMsgType TYPE_NMEA_RMC = {0xF0, 0x04};
static const struct UBXMsgType TYPE_NMEA_GLL = {0xF0, 0x01};
static const struct UBXMsgType TYPE_NMEA_GST = {0xF0, 0x07};
static const struct UBXMsgType TYPE_NMEA_VTG = {0xF0, 0x05};


static int SendUbloxRequest(const uint8_t *pUbloxPacket, uint32_t dwSendLength);
static void EncodeToUbloxMessage(struct UBXMsgType sMsgType, const void *pData, uint8_t *pOutData,int iDataSize);
static void CalcChecksum(uint8_t *ck, const uint8_t* pCalcData, int iDataLen);
static int WriteUbxCfgMsg(struct UBXMsgType sMsgType, uint8_t rate);
static int WriteUbxPollCFGNAVX5(void);
static int WriteUbxSendCFGNAVX5(uint16_t wkn);
static int WriteUbxCfgRst(uint16_t type, uint8_t chResetType);


int SetGpsRate(void)
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

	dwWriteSize = GetGPSComDevice()->write((uint8_t *)pUbloxPacket, dwSendLength);

	return (dwSendLength == dwWriteSize) ? 0 : -1;
}

static void EncodeToUbloxMessage(struct UBXMsgType sMsgType, const void *pData, uint8_t *pOutData,int iDataSize)
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
