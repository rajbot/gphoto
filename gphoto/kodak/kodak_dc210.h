
extern struct Image *kodak_dc210_get_picture (int picNum, int thumbnail);
extern unsigned char kodak_dc210_checksum(char packet[],int length);

int kodak_dc210_read (unsigned char *buf, int nbytes );
int kodak_dc210_write (char byte);
