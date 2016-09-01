
const size_t TAG_LENGTH = 11;
const size_t HEAD_LEN = 13;

double hexStr2double(const unsigned char* hex, const unsigned int length) {
    double ret = 0;
    char hexstr[length * 2];
    memset(hexstr, 0, sizeof(hexstr));

    unsigned int i;
    for (i = 0; i < length; i++) {
        sprintf(hexstr + i * 2, "%02x", hex[i]);
    }

    sscanf(hexstr, "%llx", (unsigned long long*) &ret);

    return ret;
}

int parseHeader(const unsigned char*     meta) {
    unsigned int arrayLen = 0;

    unsigned int offset = HEAD_LEN + TAG_LENGTH + 13;
    unsigned int nameLen = 0;
    unsigned int nameLength = 0;
    double numValue = 0;
    bool boolValue = false;

    if (meta[offset++] == 0x08) {
        arrayLen |= meta[offset++];
        arrayLen = arrayLen << 8;
        arrayLen |= meta[offset++];
        arrayLen = arrayLen << 8;
        arrayLen |= meta[offset++];
        arrayLen = arrayLen << 8;
        arrayLen |= meta[offset++];
    } else {
        return EXIT_FAILURE;
    }

    for (unsigned int i = 0; i < arrayLen; i++) {
        numValue = 0;
        boolValue = false;

        nameLen = 0;
        nameLen |= meta[offset++];
        nameLen = nameLen << 8;
        nameLen |= meta[offset++];
        nameLength = nameLen;
        if (nameLength > 31)
            nameLength = 31;
        char name[nameLength + 1];

        memset(name, 0, sizeof(name));
        memcpy(name, &meta[offset], nameLength);
        offset += nameLen;

        switch (meta[offset++]) {
            case 0x0: //Number type
                numValue = hexStr2double(&meta[offset], 8);
                offset += 8;
                break;

            case 0x1: //Boolean type
                if (offset++ != 0x00) {
                    boolValue = true;
                }
                break;
            case 0x2: //String type
                nameLen = 0;
                nameLen |= meta[offset++];
                nameLen = nameLen << 8;
                nameLen |= meta[offset++];
                offset += nameLen;
                break;
            case 0x12: //Long string type
                nameLen = 0;
                nameLen |= meta[offset++];
                nameLen = nameLen << 8;
                nameLen |= meta[offset++];
                nameLen = nameLen << 8;
                nameLen |= meta[offset++];
                nameLen = nameLen << 8;
                nameLen |= meta[offset++];
                offset += nameLen;
                break;
            default:
                break;
        }

        if (strncmp(name, "duration", 8) == 0) {
            duration_ = numValue;
        } else if (strncmp(name, "filesize", 8) == 0) {
            filesize_ = numValue;
        } else if (strncmp(name, "framerate", 9) == 0) {
            framerate_ = numValue;
        } else if (strncmp(name, "keyframes", 9) == 0) {
            for (unsigned int j = 0; j < 2; j++) {
                numValue = 0;
                boolValue = false;

                nameLen = 0;
                nameLen |= meta[offset++];
                nameLen = nameLen << 8;
                nameLen |= meta[offset++];
                nameLength = nameLen;
                if (nameLength > 31)
                    nameLength = 31;
                char tmpname[nameLength + 1];
                memset(tmpname, 0, sizeof(tmpname));
                memcpy(tmpname, &meta[offset], nameLength);
                tmpname[nameLength] = '\0';
                offset += nameLen;

                switch (meta[offset++]) {
                    case 0x0: //Number type
                        numValue = hexStr2double(&meta[offset], 8);
                        offset += 8;
                        break;
                    default:
                        break;
                }
                if (strncmp(tmpname, "filepositions", 13) == 0) {
                    offset += 4;
                    offset++;
                    offset += 8;
                    double positions;
                    int framenum = 0;
                    while (1) {
                        if(meta[offset + 2] == 0x74 && meta[offset+3] == 0x69 && meta[offset+4] == 0x6d
                                && meta[offset+5] == 0x65 && meta[offset+6] == 0x73)
                            break;
                        offset++;
                        positions = hexStr2double(&meta[offset], 8);
                        printf("keyframes position %f\n", positions);
                        offset += 8;
                        ++framenum;
                    }
                } else if (strncmp(tmpname, "times", 5) == 0) {
                    offset += 4;
                    offset++;
                    offset += 8;
                    double times;
                    int framenum = 0;
                    while (meta[offset] == 0x00) {
                        offset++;
                        times = hexStr2double(&meta[offset], 8);
                        printf("keyframes times %f\n", times);
                        offset += 8;
                        ++framenum;
                    }
                }
            }
        }
    }

    return EXIT_SUCCESS;
}