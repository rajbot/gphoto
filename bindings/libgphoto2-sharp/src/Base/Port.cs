using System;
using System.Runtime.InteropServices;
using Gphoto2;

namespace LibGPhoto2
{
    internal enum PortType
    {
        None    = 0,
        Serial  = 1 << 0,
        USB     = 1 << 2,
        Disk    = 1 << 3,
        PTPIP   = 1 << 4
    }

    internal enum PortSerialParity
    {
        Off = 0,
        Even,
        Odd
    }
    
    internal enum Pin
    {
        RTS,
        DTR,
        CTS,
        DSR,
        CD,
        RING
    }
    
    internal enum Level
    {
        Low = 0,
        High = 1
    }
    
    [StructLayout(LayoutKind.Sequential)]
    internal struct PortPrivateLibrary
    {
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct PortPrivateCore
    {
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct PortSettingsSerial
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=128)] public char[] port;
        public int speed;
        public int bits;
        public PortSerialParity parity;
        public int stopbits;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct PortSettingsUSB
    {
        public int inep, outep, intep;
        public int config;
        public int pinterface;
        public int altsetting;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct PortSettingsDisk
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=128)] public char[] mountpoint;
    }


    [StructLayout(LayoutKind.Explicit)]
    internal struct PortSettings
    {
        [FieldOffset(0)] public PortSettingsSerial serial;
        [FieldOffset(0)] public PortSettingsUSB usb;
        [FieldOffset(0)] public PortSettingsDisk disk;
    }

#if false
    [StructLayout(LayoutKind.Sequential)]
    internal struct _Port
    {
        PortType type;

        PortSettings settings;
        PortSettings settings_pending;

        int timout;

        PortPrivateLibrary *pl;
        PortPrivateCore *pc;

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_new (out _Port *port);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_free (_Port *port);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_set_info (_Port *port, ref _PortInfo info);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_get_info (_Port *port, out _PortInfo info);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_open (_Port *port);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_close (_Port *port);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_read (_Port *port, [MarshalAs(UnmanagedType.LPTStr)] byte[] data, int size);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_write (_Port *port, [MarshalAs(UnmanagedType.LPTStr)] byte[] data, int size);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_get_settings (_Port *port, out PortSettings settings);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_set_settings (_Port *port, PortSettings settings);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_get_timeout (_Port *port, int *timeout);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_set_timeout (_Port *port, int timeout);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_get_pin (_Port *port, Pin pin, Level *level);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_set_pin (_Port *port, Pin pin, Level level);

        [DllImport ("libgphoto2_port.so")]
        private static extern char* gp_port_get_error (_Port *port);

        /* TODO: implement
        [DllImport ("libgphoto2_port.so")]
        private static extern int gp_port_set_error (_Port *port, const char *format, ...);
        */
    }
#endif

    internal class Port : Object
    {
        public Port()
        {
            IntPtr native;

            Error.CheckError (gp_port_new (out native));

            this.handle = new HandleRef (this, native);
        }
        
        protected override void Dispose (bool disposing)
        {
            if(!Disposed)
            {
                // Don't check the error as we don't want to throw an exception if it fails
                gp_port_free (this.handle);
                base.Dispose(disposing);
            }
        }

        public void SetInfo (PortInfo info)
        {
            Error.CheckError (gp_port_set_info (this.Handle, ref info.Handle));
        }
        
        public PortInfo GetInfo ()
        {
            PortInfo info = new PortInfo (); 

            Error.CheckError (gp_port_get_info (this.Handle, out info.Handle));

            return info;
        }
        
        public void Open ()
        {
            Error.CheckError (gp_port_open (this.Handle));
        }
        
        public void Close ()
        {
            Error.CheckError (gp_port_close (this.Handle));
        }
        
        public byte[] Read (int size)
        {
            byte[] data = new byte[size];

            Error.CheckError (gp_port_read (this.Handle, data, size));

            return data;
        }
        
        public void Write (byte[] data)
        {
            Error.CheckError (gp_port_write (this.Handle, data, data.Length));
        }
        
        public void SetSettings (PortSettings settings)
        {
            Error.CheckError (gp_port_set_settings (this.Handle, settings));
        }
        
        public PortSettings GetSettings ()
        {
            PortSettings settings;

            Error.CheckError (gp_port_get_settings (this.Handle, out settings));

            return settings;
        }
        
        public int Timeout
        {
            get {
                int timeout;

                Error.CheckError (gp_port_get_timeout (this.Handle, out timeout));

                return timeout;
            }
            set {
                Error.CheckError (gp_port_set_timeout (this.Handle, value));
            }
        }
        
        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_new (out IntPtr port);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_free (HandleRef port);
        
        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_set_info (HandleRef port, ref _PortInfo info);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_get_info (HandleRef port, out _PortInfo info);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_open (HandleRef port);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_close (HandleRef port);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_read (HandleRef port, [MarshalAs(UnmanagedType.LPTStr)] byte[] data, int size);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_write (HandleRef port, [MarshalAs(UnmanagedType.LPTStr)] byte[] data, int size);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_set_settings (HandleRef port, PortSettings settings);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_get_settings (HandleRef port, out PortSettings settings);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_get_timeout (HandleRef port, out int timeout);

        [DllImport ("libgphoto2_port.so")]
        private static extern ErrorCode gp_port_set_timeout (HandleRef port, int timeout);

        //[DllImport ("libgphoto2_port.so")]
        //private static extern ErrorCode gp_port_get_pin (HandleRef port, Pin pin, out Level level);

        //[DllImport ("libgphoto2_port.so")]
        //private static extern ErrorCode gp_port_set_pin (HandleRef port, Pin pin, Level level);

        //[DllImport ("libgphoto2_port.so")]
        //private static extern string gp_port_get_error (HandleRef port);
    }
}
