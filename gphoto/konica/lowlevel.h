char qm100_readByte(int serialdev);
char qm100_readCodedByte(int serialdev);
char qm100_readTimedByte(int serialdev);
void qm100_writeByte(int serialdev, char data);
void qm100_iostat(unsigned char *str, unsigned char *buf, int len);

extern int qm100_showReadBytes;
extern int qm100_showWriteBytes;
