
struct Image *kodak_dc210_get_picture (int picNum, int thumbnail);
unsigned char kodak_dc210_checksum(char packet[],int length);
unsigned char kodak_dc210_read_byte ( int serialdev );
int kodak_dc210_write_byte ( int serialdev, char b );

