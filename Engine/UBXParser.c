#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Device/GPSDeviceIF.h"

enum UBXClassID {
	UBX_CLASS_ID_NAV = 0x01,
	UBX_CLASS_ID_RXM = 0x02,
	UBX_CLASS_ID_INF = 0x04,
	UBX_CLASS_ID_ACK = 0x05,
	UBX_CLASS_ID_CFG = 0x06,
	UBX_CLASS_ID_MON = 0x0A,
	UBX_CLASS_ID_AID = 0x0B,
	UBX_CLASS_ID_TIM = 0x0D,
	UBX_CLASS_ID_ESF = 0x10,
};

enum NMEAMsgType {
	NMEA_MSG_TYPE_DTM,
	NMEA_MSG_TYPE_GBS,
	NMEA_MSG_TYPE_GGA,
	NMEA_MSG_TYPE_GLL,
	NMEA_MSG_TYPE_GPQ,
	NMEA_MSG_TYPE_GRS,
	NMEA_MSG_TYPE_GSA,
	NMEA_MSG_TYPE_GST,
	NMEA_MSG_TYPE_GSV,
	NMEA_MSG_TYPE_RMC,
	NMEA_MSG_TYPE_THS,
	NMEA_MSG_TYPE_TXT,
	NMEA_MSG_TYPE_VTG,
	NMEA_MSG_TYPE_ZDA,
};

struct UBXPacketHeader {
	uint8_t sync_char[2];
	uint8_t ubx_class;
	uint8_t id;
	uint16_t length;
	uint8_t payload_check[0];
};

static int IsUBXPacketValid(struct UBXPacketHeader *header);
static int CheckUBXPacket(struct UBXPacketHeader *header);
static int UBXPacketParse(struct UBXPacketHeader *header);

static int ReadNMEAString(void);
static enum NMEAMsgType ParseNMEAMsgType(char *str);

int UBXPacketRead(void)
{
	int ret = -1;
	struct UBXPacketHeader header;
	uint8_t start = 0;
	GetGPSComDevice()->read(&start, 1);
	if (0xB5 == start) {
		header.sync_char[0] = start;
		GetGPSComDevice()->read(&(header.sync_char[1]), sizeof(header) - 1);
		if (!IsUBXPacketValid(&header)) {
			struct UBXPacketHeader *packet =
			(struct UBXPacketHeader *)malloc(sizeof(header) + header.length + 2);
			memcpy(packet, &header, sizeof(header));
			GetGPSComDevice()->read(packet->payload_check, packet->length + 2);
			if (!CheckUBXPacket(packet)) {
				if (!UBXPacketParse(packet)) {
					ret = 0;
				}
			}
			free(packet);
		}
	}
	else if ('$' == start) {
		ReadNMEAString();
	}
	return ret;
}

static int IsUBXPacketValid(struct UBXPacketHeader *header)
{
	if (0xB5 != header->sync_char[0]) {
		return -1;
	}
	if (0x62 != header->sync_char[1]) {
		return -1;
	}

	return 0;
}

static int CheckUBXPacket(struct UBXPacketHeader *header)
{
	uint8_t check_a = 0, check_b = 0;
	size_t i;
	uint8_t *buffer = &(header->ubx_class);
	for (i = 0; i < 4 + header->length; i++) {
		check_a += buffer[i];
		check_b += check_a;
	}

	if (check_a != header->payload_check[header->length]) {
		return -1;
	}
	if (check_b != header->payload_check[header->length + 1]) {
		return -1;
	}

	return 0;
}

static int UBXPacketParse(struct UBXPacketHeader *header)
{
	switch (header->ubx_class) {
		case UBX_CLASS_ID_ACK:
		{
			switch (header->id)
			{
				case 0x01:
				{

				}
				break;
				default:
				break;
			}
		}
		break;
		default:
		break;
	}
	return 0;
}

static int ReadNMEAString(void)
{
	uint8_t status = 0;
	uint8_t data = 0;
	char strBuffer[254] = {0};
	size_t pos = 0;
	while (1 == GetGPSComDevice()->read(&data, 1)) {
		switch (status) {
			case 0:
			{
				if ('\r' == data) {
					status = 1;
				}
				else {
					strBuffer[pos] = data;
					pos++;
				}
			}
			break;
			case 1:
			{
				if ('\n' == data) {
					if ('*' == strBuffer[pos - 3]) {
						size_t i;
						uint16_t checksum = 0;
						uint16_t sum = 0;
						sscanf(&strBuffer[pos - 2], "%hx", &checksum);
						for (i = 0; i < pos - 3; i++) {
							sum += strBuffer[i];
						}
						if (sum == checksum) {
							enum NMEAMsgType type;
							char *pch = strtok(strBuffer, ",");
							if (pch) {
								int flag = 0;
								if ('P' == pch[0]) {
									flag = 1;
								}
								else if (('G' == pch[0]) && ('P' == pch[1])) {
									flag = 2;
								}
								if (flag) {
									type = ParseNMEAMsgType(pch + flag);
									(void)type;
									do {
										pch = strtok(NULL, ",");
									} while (pch);
									return 0;
								}
							}
						}
					}
				}
				else {
					return -1;
				}
			}
			break;
		}
	}
	return -1;
}

static enum NMEAMsgType ParseNMEAMsgType(char *str)
{
	(void)str;
	return NMEA_MSG_TYPE_ZDA;
}