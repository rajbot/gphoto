extern struct _Camera konica_qm100;
extern struct _Camera konica_qm1xx;
extern struct _Camera konica_qm2xx;
extern struct _Camera barbie;
extern struct _Camera canon;
extern struct _Camera casio_qv;
extern struct _Camera coolpix600;
extern struct _Camera philips;
extern struct _Camera fuji;
extern struct _Camera kodak_dc2x;
extern struct _Camera kodak_dc210;
extern struct _Camera kodak;
extern struct _Camera mustek_mdc800_rs232;
extern struct _Camera mustek_mdc800_usb;
extern struct _Camera ricoh_300z;
extern struct _Camera olympus;
extern struct _Camera samsung800k;
extern struct _Camera sony_dscf1;
extern struct _Camera sony_dscf55;
extern struct _Camera digita;
extern struct _Camera dimage_v;
extern struct _Camera jd11;
extern struct _Camera dimera_3500;
extern struct _Camera polaroid_pdc700;
extern struct _Camera directory;

struct Model cameras[] = {
	{"Browse Directory", &directory, 0, 0},
	{"Agfa ePhoto 307", &olympus, 0, 0},
	{"Agfa ePhoto 780", &olympus, 0, 0},
	{"Agfa ePhoto 780C", &olympus, 0, 0},
	{"Agfa ePhoto 1280", &olympus, 0, 0},
	{"Agfa ePhoto 1680", &olympus, 0, 0},
	{"Apple QuickTake 150", &olympus, 0, 0},
	{"Apple QuickTake 200", &olympus, 0, 0},
	{"Barbie Camera", &barbie, 0, 0},
	{"Canon PowerShot A5", &canon, 0, 0},
	{"Canon PowerShot A50", &canon, 0, 0},
	{"Canon PowerShot G1", &canon, 0x4a9,0x3048},
	{"Canon PowerShot S10", &canon, 0x4a9, 0x3041},
	{"Canon PowerShot S20", &canon, 0x4a9, 0x3043},
    {"Canon PowerShot S100", &canon, 0x4a9, 0x3045},
	{"Canon Digital Ixus", &canon, 0x4a9, 0x3047},
	{"Casio QV-10", &casio_qv, 0, 0},
	{"Casio QV-10A", &casio_qv, 0, 0},
	{"Casio QV-11", &casio_qv, 0, 0},
	{"Casio QV-30", &casio_qv, 0, 0},
	{"Casio QV-70", &casio_qv, 0, 0},
	{"Casio QV-100", &casio_qv, 0, 0},
	{"Casio QV-200", &casio_qv, 0, 0},
	{"Casio QV-700", &casio_qv, 0, 0},
	{"Casio QV-5000SX", &casio_qv, 0, 0},
	{"Chinon ES-1000", &olympus, 0, 0},
	{"Dimera 3500", &dimera_3500, 0, 0},
	{"Epson PhotoPC 500", &olympus, 0, 0},
	{"Epson PhotoPC 550", &olympus, 0, 0},
	{"Epson PhotoPC 600", &olympus, 0, 0},
	{"Epson PhotoPC 700", &olympus, 0, 0},
	{"Epson PhotoPC 800", &olympus, 0, 0},
	{"Fuji DS-7", &fuji, 0, 0},
	{"Fuji DX-5", &fuji, 0, 0},
	{"Fuji DX-10", &fuji, 0, 0},
	{"Fuji MX-600", &fuji, 0, 0},
	{"Fuji MX-1700", &fuji, 0, 0},
	{"Fuji MX-2700", &fuji, 0, 0},
	{"Hewlett Packard PhotoSmart C20", &konica_qm100, 0, 0},
	{"Hewlett Packard PhotoSmart C30", &konica_qm100, 0, 0},
	{"Hewlett Packard PhotoSmart C200", &konica_qm100, 0, 0},
	{"Hot Wheels Camera", &barbie, 0, 0},
	{"JenCam JD11", &jd11, 0, 0},
	{"Kodak DC20", &kodak_dc2x, 0, 0},
	{"Kodak DC25", &kodak_dc2x, 0, 0},
	{"Kodak DC200+", &kodak_dc210, 0, 0},
	{"Kodak DC210", &kodak_dc210, 0, 0},
	{"Kodak DC210+ Zoom", &kodak_dc210, 0, 0},
	{"Kodak DC215 Zoom", &kodak_dc210, 0, 0},
	{"Kodak DC220", &digita, 0x040a, 0x0100},
	{"Kodak DC220+", &kodak, 0, 0},
	{"Kodak DC240", &kodak, 0, 0},
	{"Kodak DC260", &digita, 0x040a, 0x0110},
	{"Kodak DC265", &digita, 0x040a, 0x0111},
	{"Kodak DC280", &kodak, 0, 0},
	{"Kodak DC290", &digita, 0x040a, 0x0112},
	{"Konica QM100", &konica_qm100, 0, 0},
	{"Konica QM100V", &konica_qm100, 0, 0},
	{"Konica Q-EZ", &konica_qm100, 0, 0},
	{"Konica Q-M100(beta test)",  &konica_qm1xx, 0, 0},
	{"Konica Q-M100V(beta test)", &konica_qm1xx, 0, 0},
	{"Konica Q-M200(beta test)",  &konica_qm2xx, 0, 0},
	{"Minolta Dimage V", &dimage_v, 0, 0},
	{"Mustek MDC 800 (rs232)",&mustek_mdc800_rs232, 0, 0},
	{"Mustek MDC 800 (usb)",&mustek_mdc800_usb, 0x055f,0xa800},
	{"Mustek VDC 3500", &dimera_3500, 0, 0},
	{"Nikon CoolPix 100", &olympus, 0, 0},
	{"Nikon CoolPix 300", &olympus, 0, 0},
	{"Nikon CoolPix 600", &coolpix600, 0, 0},
	{"Nikon CoolPix 700", &olympus, 0, 0},
	{"Nikon CoolPix 900", &olympus, 0, 0},
	{"Nikon CoolPix 900S", &olympus, 0, 0},
	{"Nikon CoolPix 950", &olympus, 0, 0},
	{"Nikon CoolPix 950S", &olympus, 0, 0},
	{"Olympus D-100Z", &olympus, 0, 0},
	{"Olympus D-200L", &olympus, 0, 0},
	{"Olympus D-220L", &olympus, 0, 0},
	{"Olympus D-300L", &olympus, 0, 0},
	{"Olympus D-320L", &olympus, 0, 0},
	{"Olympus D-330R", &olympus, 0, 0},
	{"Olympus D-340L", &olympus, 0, 0},
	{"Olympus D-340R", &olympus, 0, 0},
	{"Olympus D-360L", &olympus, 0, 0},
	{"Olympus D-400L Zoom", &olympus, 0, 0},
	{"Olympus D-500L", &olympus, 0, 0},
	{"Olympus D-600L", &olympus, 0, 0},
	{"Olympus D-620L", &olympus, 0, 0},
	{"Olympus C-400L", &olympus, 0, 0},
	{"Olympus C-410L", &olympus, 0, 0},
	{"Olympus C-800L", &olympus, 0, 0},
	{"Olympus C-820L", &olympus, 0, 0},
	{"Olympus C-830L", &olympus, 0, 0},
	{"Olympus C-840L", &olympus, 0, 0},
	{"Olympus C-900 Zoom", &olympus, 0, 0},
	{"Olympus C-900L Zoom", &olympus, 0, 0},
	{"Olympus C-1000L", &olympus, 0, 0},
	{"Olympus C-1400L", &olympus, 0, 0},
	{"Olympus C-2000Z", &olympus, 0, 0},
	{"Olympus C-2500L", &olympus, 0, 0},
	{"Olympus C-3030Z", &olympus, 0x07b4, 0x0100},
	{"Panasonic Coolshot KXl-600A", &olympus, 0, 0},
	{"Panasonic Cardshot NV-DCF5E", &olympus, 0, 0},
	{"Philips ESP60", &philips, 0, 0},
	{"Philips ESP80", &philips, 0, 0},
	{"Polaroid PDC 640", &olympus, 0, 0},
        {"Polaroid PDC 700", &polaroid_pdc700, 0, 0},
	{"Relisys Dimera 3500", &dimera_3500, 0, 0},
	{"Ricoh RDC-300", &ricoh_300z, 0, 0},
	{"Ricoh RDC-300Z", &ricoh_300z, 0, 0},
  	{"Ricoh RDC-4200", &philips, 0, 0},
	{"Ricoh RDC-4300", &philips, 0, 0},
	{"Ricoh RDC-5000", &philips, 0, 0},
	{"Samsung Digimax 800k", &samsung800k, 0, 0},
	{"Sanyo VPC-G210", &olympus, 0, 0},
	{"Sanyo VPC-G200", &olympus, 0, 0},
	{"Sanyo VPC-G250", &olympus, 0, 0},
	{"Sanyo VPC-X350", &olympus, 0, 0},
	{"Sony DSC-F1", &sony_dscf1, 0, 0},
	{"Sony DSC-F55", &sony_dscf55, 0, 0},
	{"Sony DSC-F505", &sony_dscf55, 0, 0},
	{"Trust DC 3500", &dimera_3500, 0, 0},
	{"Sony Memory Stick Adapter", &sony_dscf55, 0, 0},
	{"Toshiba PDR-M1", &fuji, 0, 0},
	{"WWF Camera", &barbie, 0, 0},
	{NULL, NULL}
};

