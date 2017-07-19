#include "Engine/UBXParser.h"

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "Engine/minmea.h"

#include "Device/GPSDeviceIF.h"

#include "Common/GPSLog.h"

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

struct UBXPacketHeader {
    uint8_t sync_char[2];
    uint8_t ubx_class;
    uint8_t id;
    uint16_t length;
    uint8_t payload_check[0];
} __attribute__ ((packed));

static struct GpsDataCallbacks *cbs = NULL;

static int IsUBXPacketValid(struct UBXPacketHeader *header);
static int CheckUBXPacket(struct UBXPacketHeader *header);
static int UBXPacketParse(struct UBXPacketHeader *header);
static int ReadNMEAString(void);

void UBXParserInit(struct GpsDataCallbacks *cb)
{
    cbs = cb;
}

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
                ret = UBXPacketParse(packet);
            }
            free(packet);
        }
    }
    else if ('$' == start) {
        ret = ReadNMEAString();
    }
    return ret;
}

static int IsUBXPacketValid(struct UBXPacketHeader *header)
{
    GPSLOGD("IsUBXPacketValid");
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
    GPSLOGD("CheckUBXPacket");
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
    GPSLOGD("UBXPacketParse:class 0x%x, id 0x%x", header->ubx_class, header->id);
    switch (header->ubx_class) {
        case UBX_CLASS_ID_ACK:
        {
            switch (header->id)
            {
                case 0x01:
                {
                    int numCh = header->payload_check[4];
                    int i;
                    struct GPS_SV_INFO *svinfos = (struct GPS_SV_INFO *)malloc(numCh * sizeof(*svinfos));
                    for (i = 0; i < numCh; i++) {
                        GPSLOGD("chn: %d, svid: %d, cno: %d, elev: %d",
                            header->payload_check[8 + 12 * i],
                            header->payload_check[9 + 12 * i],
                            header->payload_check[12 + 12 * i],
                            header->payload_check[14 + 12 * i]);

                        svinfos[i].chn = header->payload_check[8 + 12 * i];
                        svinfos[i].svid = header->payload_check[9 + 12 * i];
                        svinfos[i].flags = header->payload_check[10 + 12 * i];
                        svinfos[i].cno = header->payload_check[12 + 12 * i];
                        svinfos[i].elev = header->payload_check[13 + 12 * i];
                        svinfos[i].azim = ((int16_t)(header->payload_check[14 + 12 * i]) << 8) |
                        header->payload_check[15 + 12 * i];
                    }

                    cbs->svinfo_func(numCh, svinfos);

                    free(svinfos);
                }
                break;
                default:
                break;
            }
        }
        break;
        case UBX_CLASS_ID_MON:
        {
            switch (header->id) {
                case 0x04:
                {
                    GPSLOGD("swVersion: %s, hwVersion: %s, romVersion: %s",
                        (char *)(header->payload_check),
                        (char *)(header->payload_check + 30),
                        (char *)(header->payload_check + 40));
                }
                break;
                default:
                break;
            }
        }
        break;
        case UBX_CLASS_ID_NAV:
        {
            switch (header->id) {
                case 0x21:
                {
                    GPSLOGD("year: %d, month: %d, day: %d",
                        ((uint16_t)(header->payload_check[12]) << 8) | header->payload_check[13],
                        header->payload_check[14], header->payload_check[15]);

                    /* TODO: decode and transfer to utc time */
                    cbs->time_func(0);
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
    char strBuffer[256] = {0};
    size_t pos = 1;
    strBuffer[0] = '$';
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
                    enum minmea_sentence_id type = minmea_sentence_id(strBuffer, false);
                    if (MINMEA_INVALID == type) {
                        return -1;
                    }

                    cbs->nmea_func(strBuffer, strlen(strBuffer));

                    switch (type) {
                        case MINMEA_SENTENCE_RMC:
                        {
                            GPSLOGD("ReadNMEAString:MINMEA_SENTENCE_RMC");
                            struct minmea_sentence_rmc frame;
                            if (minmea_parse_rmc(&frame, strBuffer)) {
                                struct GPS_NMEA_RMC_DATA data;
                                memcpy(&data.time, &frame.time, sizeof(data.time));
                                data.valid = frame.valid;
                                data.latitude = minmea_tofloat(&frame.latitude);
                                data.longitude = minmea_tofloat(&frame.longitude);
                                data.speed = minmea_tofloat(&frame.speed);
                                data.course = minmea_tofloat(&frame.course);
                                memcpy(&data.date, &frame.date, sizeof(data.date));
                                data.variation = minmea_tofloat(&frame.variation);

                                cbs->rmc_func(&data);
                            }
                            else {

                            }
                        }
                        break;
                        case MINMEA_SENTENCE_GGA:
                        {
                            GPSLOGD("ReadNMEAString:MINMEA_SENTENCE_GGA");
                            struct minmea_sentence_gga frame;
                            if (minmea_parse_gga(&frame, strBuffer)) {
                                struct GPS_NMEA_GGA_DATA data;
                                memcpy(&data.time, &frame.time, sizeof(data.time));
                                data.latitude = minmea_tofloat(&frame.latitude);
                                data.longitude = minmea_tofloat(&frame.longitude);
                                data.fix_quality = frame.fix_quality;
                                data.satellites_tracked = frame.satellites_tracked;
                                data.hdop = minmea_tofloat(&frame.hdop);
                                data.altitude = minmea_tofloat(&frame.altitude);
                                data.altitude_units = frame.altitude_units;
                                data.height = minmea_tofloat(&frame.height);
                                data.height_units = frame.height_units;
                                data.dgps_age = frame.dgps_age;

                                cbs->gga_func(&data);
                            }
                            else {
                                
                            }
                        }
                        break;
                        case MINMEA_SENTENCE_GSA:
                        {

                        }
                        break;
                        case MINMEA_SENTENCE_GLL:
                        {

                        }
                        break;
                        case MINMEA_SENTENCE_GST:
                        {

                        }
                        break;
                        case MINMEA_SENTENCE_GSV:
                        {

                        }
                        break;
                        case MINMEA_SENTENCE_VTG:
                        {

                        }
                        break;
                        default:
                        {
                            return -1;
                        }
                        break;
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
