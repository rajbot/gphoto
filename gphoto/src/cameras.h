extern struct _Camera konica_qm100;
extern struct _Camera canon;
extern struct _Camera casio_qv;
extern struct _Camera philips;
extern struct _Camera fuji;
extern struct _Camera kodak_dc2x;
extern struct _Camera kodak_dc210;
extern struct _Camera kodak;
extern struct _Camera ricoh_300z;
extern struct _Camera olympus;
extern struct _Camera sony_dscf1;
extern struct _Camera dimage_v;
extern struct _Camera directory;

struct Model cameras[] = {
	{"Browse Directory", &directory},
	{"Agfa ePhoto 307", &olympus},
	{"Agfa ePhoto 780", &olympus},
	{"Agfa ePhoto 1280", &olympus},
	{"Agfa ePhoto 1680", &olympus},
	{"Apple QuickTake 150", &olympus},
	{"Apple QuickTake 200", &olympus},
	{"Canon PowerShot A50", &canon},
	{"Casio QV-10", &casio_qv},
	{"Casio QV-10A", &casio_qv},
	{"Casio QV-11", &casio_qv},
	{"Casio QV-30", &casio_qv},
	{"Casio QV-70", &casio_qv},
	{"Casio QV-100", &casio_qv},
	{"Casio QV-200", &casio_qv},
	{"Casio QV-700", &casio_qv},
	{"Casio QV-5000SX", &casio_qv},
	{"Chinon ES-1000", &olympus},
	{"Epson PhotoPC 500", &olympus},
	{"Epson PhotoPC 550", &olympus},
	{"Epson PhotoPC 600", &olympus},
	{"Epson PhotoPC 700", &olympus},
	{"Fuji DS-7", &fuji},
	{"Fuji DX-5", &fuji},
	{"Fuji DX-10", &fuji},
	{"Fuji MX-2700", &fuji},
	{"Hewlett Packard C20", &konica_qm100},
	{"Hewlett Packard C30", &konica_qm100},
	{"Kodak DC20", &kodak_dc2x},
	{"Kodak DC25", &kodak_dc2x},
	{"Kodak DC200+", &kodak_dc210},
	{"Kodak DC210", &kodak_dc210},
	{"Kodak DC210+ Zoom", &kodak_dc210},
        {"Kodak DC220+", &kodak},
        {"Kodak DC240", &kodak},
        {"Kodak DC280", &kodak},
	{"Konica QM100", &konica_qm100},
	{"Konica QM100V", &konica_qm100},
	{"Konica Q-EZ", &konica_qm100},
	{"Minolta Dimage V", &dimage_v},
	{"Nikon CoolPix 100", &olympus},
	{"Nikon CoolPix 300", &olympus},
	{"Nikon CoolPix 600", &olympus},
	{"Nikon CoolPix 900", &olympus},
	{"Nikon CoolPix 900S", &olympus},
	{"Nikon CoolPix 950", &olympus},
	{"Nikon CoolPix 950S", &olympus},
	{"Olympus D-100Z", &olympus},
	{"Olympus D-200L", &olympus},
	{"Olympus D-220L", &olympus},
	{"Olympus D-300L", &olympus},
	{"Olympus D-320L", &olympus},
	{"Olympus D-330R", &olympus},
	{"Olympus D-340L", &olympus},
	{"Olympus D-340R", &olympus},
	{"Olympus D-400L Zoom", &olympus},
	{"Olympus D-500L", &olympus},
	{"Olympus D-600L", &olympus},
	{"Olympus D-620L", &olympus},
	{"Olympus C-400L", &olympus},
	{"Olympus C-410L", &olympus},
	{"Olympus C-800L", &olympus},
	{"Olympus C-820L", &olympus},
	{"Olympus C-830L", &olympus},
	{"Olympus C-840L", &olympus},
	{"Olympus C-900 Zoom", &olympus},
	{"Olympus C-900L Zoom", &olympus},
	{"Olympus C-1000L", &olympus},
	{"Olympus C-1400L", &olympus},
	{"Olympus C-2000Z", &olympus},
	{"Panasonic Coolshot KXl-600A", &olympus},
	{"Panasonic Cardshot NV-DCF5E", &olympus},
	{"Philips ESP60", &philips},
	{"Philips ESP80", &philips},
	{"Polaroid PDC 640", &olympus},
	{"Ricoh RDC-300", &ricoh_300z},
	{"Ricoh RDC-300Z", &ricoh_300z},
  	{"Ricoh RDC-4200", &philips},
	{"Ricoh RDC-4300", &philips},
	{"Ricoh RDC-5000", &philips},
	{"Sanyo VPC-G210", &olympus},
	{"Sanyo VPC-G250", &olympus},
	{"Sanyo VPC-X350", &olympus},
	{"Sony DSC-F1", &sony_dscf1},
	{"", NULL}
};
