/////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//

#include <string.h>
#include <stdio.h>
#include <windows.h>


char	gszPort[] = "COM1";
HANDLE	hComm;

long	PortSpeed;


/***************************************************************
*
*
*/
BOOL WINAPI DllMain(HANDLE hModule, DWORD fdwreason, LPVOID lpReserved )
{
    switch(fdwreason)
	{
		case DLL_PROCESS_ATTACH:
			// The DLL is being mapped into process's address space
			//  Do any required initialization on a per application basis, return FALSE if failed
			break;
		case DLL_THREAD_ATTACH:
			// A thread is created. Do any required initialization on a per thread basis
			break;
		case DLL_THREAD_DETACH:
			// Thread exits with  cleanup
			break;
		case DLL_PROCESS_DETACH:
			// The DLL unmapped from process's address space. Do necessary cleanup
			break;
    }

    return TRUE;
}



/***************************************************************
*
*
*/
int SetPortSpeed()
{
	DCB dcb;

	if(!GetCommState(hComm, &dcb))
	{
	  // Error in GetCommState
	  return FALSE;
	}

	// Update DCB rate.
	dcb.BaudRate = PortSpeed;

	// Set new state.
	if (!SetCommState(hComm, &dcb))
		exit(1);

	return 0;
}



/***************************************************************
*
*
*/
int TransferRateID(int baud)
{
	int r = 0;

	switch (baud)
	{
		case 115200: 
			r = 4;
			break;
		case 57600: 
			r = 3;
			break;
		case 38400: 
			r = 2;
			break;
		case 19200: 
			r = 1;
			break;	/* works on sun */
		default:
		case 9600:
			r = 0;
			break;	/* works on sun */
	}

	return r;
}


/***************************************************************
**
**
*/
int ConfigDSCF55Speed(char *spd_buf, int verbose)
{

	if(!strcmp(spd_buf, "115200"))
		PortSpeed = CBR_115200;
#if defined(B76800)
	else if(!strcmp(spd_buf,  "76800"))
		PortSpeed = CBR_76800;
	else
#endif
	if(!strcmp(spd_buf,  "57600"))
		PortSpeed = CBR_57600;
	else if(!strcmp(spd_buf,  "38400"))
		PortSpeed = CBR_38400;
	else if(!strcmp(spd_buf,  "19200"))
		PortSpeed = CBR_19200;
	else if(!strcmp(spd_buf,   "9600"))
		PortSpeed = CBR_9600;

//	if(verbose > 1)
		printf("Speed set to %u (%s bps)\n", PortSpeed, spd_buf);

  return PortSpeed;
}

/***************************************************************
*
*
*/
void ClosePort()
{
}
    
int InitSonyDSCF55(char *devicename)
{
	return InitSonyDevice(devicename);
}

/***************************************************************
*
*
*/
InitSonyDevice(char *devicename)
{
	long ulrc;
	DCB dcb;
	COMMTIMEOUTS timeouts;


	PortSpeed = CBR_9600;

	hComm = CreateFile(gszPort, GENERIC_READ | GENERIC_WRITE, 0, 0, 
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

	if(hComm == INVALID_HANDLE_VALUE)
	{
		printf("Error opening port\n");
		exit(1);
	}


	FillMemory(&dcb, sizeof(dcb), 0);

	if(!GetCommState(hComm, &dcb))
		return FALSE;

	// Update DCB rate.
	dcb.BaudRate = CBR_9600 ;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity = FALSE;

	if (!SetCommState(hComm, &dcb))
		exit(1);


	timeouts.ReadIntervalTimeout = MAXDWORD; 
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 50;

	if(!SetCommTimeouts(hComm, &timeouts))
	{
                printf("Error setting port timeouts");
		exit(1);
	}


	SetPortSpeed();

	return TRUE;
}


/***************************************************************
*
*
*/
void CloseSonyDSCF55()
{
    ClosePort();
}


/***************************************************************
*
*
*/
int dscSetSpeed(int speed)
{
	switch(speed)
	{
		case 115200:
			PortSpeed = CBR_115200;
			break;
		case 57600:
			PortSpeed = CBR_57600;
			break;
		case 38400:
			PortSpeed = CBR_38400;
			break;
		case 19200:
			PortSpeed = CBR_19200;
			break;
		default:
			PortSpeed = CBR_9600;
			break;
	}

	return SetPortSpeed();
}


/***************************************************************
*
*
*/
DWORD extRead(unsigned char *lpBuf, int *length)
{
        int n;
	DWORD bytes_read;

        for(n=0; n<5; n++)
	{
		ReadFile(hComm, lpBuf, (DWORD)*length, &bytes_read, NULL);

		if(bytes_read>0)
			break;

		Sleep(50); // 50 ms
	}

	return bytes_read;
}


/***************************************************************
*
*
*/
int ReadCommByte(unsigned char *byte)
{
	static unsigned char buf[256];
	static int bytes_read = 0;
	static int bytes_returned = 0;
	int	buflen=256;

	if(bytes_returned < bytes_read)
	{
		*byte = buf[bytes_returned++];
		return 1;
	}

	bytes_read = extRead(buf, &buflen);

	bytes_returned = 0;

	if(bytes_read)
		*byte = buf[bytes_returned++];

	return (bytes_read > 0) ? 1 : bytes_read;
}


/***************************************************************
*
*
*/
int extWrite(unsigned char *lpBuf, DWORD  dwLength)
{
	DWORD dwWritten;
	BOOL fRes = TRUE;

	WriteFile(hComm, lpBuf, dwLength, &dwWritten, NULL);

	return dwWritten;
}



/***************************************************************
*
*
*/
void DumpData(unsigned char *buffer, int length)
{
	int n=0;

	printf("Dumping :");

	for(n=0; n<length; n++)
	{
		printf("%u ", (int) ((unsigned char )buffer[n]));
	}

	printf("\n");

	fflush(stdout);
}





