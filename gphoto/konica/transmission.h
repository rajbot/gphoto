int qm100_transmit(int serialdev, unsigned char *cmd, int cmd_len, qm100_packet_block *packet);
void qm100_attention(int serialdev);
void qm100_sendPacket(int serialdev, unsigned char *cmd, int cmd_len);
void qm100_getAck(int serialdev);
int qm100_getPacket(int serialdev, qm100_packet_block *packet);
void qm100_endTransmit(int serialdev);
void qm100_continueTransmission(int serialdev);
int qm100_getRealPicNum(int serialdev, int picNum);

extern int qm100_showReadPackages;
extern int qm100_showWritePackages;

