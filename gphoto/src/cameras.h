extern struct _Camera konica_qm100;
extern struct _Camera casio_qv;
extern struct _Camera philips;
extern struct _Camera fuji;
extern struct _Camera kodak_dc2x;
extern struct _Camera kodak_dc210;
extern struct _Camera ricoh_300z;
extern struct _Camera olympus;
extern struct _Camera sony_dscf1;
extern struct _Camera directory;

struct Model cameras[] = {
	{"Browse Directory", &directory},
	{"Agfa ePhoto 307", &olympus},
	{"Agfa ePhoto 780", &olympus},
	{"Agfa ePhoto 1280", &olympus},
	{"Agfa ePhoto 1680", &olympus},
	{"Apple QuickTake 150", &olympus},
	{"Apple QuickTake 200", &olympus},
	{"Casio QV-10", &casio_qv},
	{"Casio QV-11", &casio_qv},
	{"Casio QV-100", &casio_qv},
	{"Casio QV-200", &casio_qv},
	{"Chinon ES-1000", &olympus},
	{"Epson PhotoPC 500", &olympus},
	{"Epson PhotoPC 550", &olympus},
	{"Epson PhotoPC 600", &olympus},
	{"Epson PhotoPC 700", &olympus},
	{"Fuji DS-7", &fuji},
	{"Hewlett Packard C20 (unverified)", &konica_qm100},
	{"Hewlett Packard C30", &konica_qm100},
	{"Kodak DC20", &kodak_dc2x},
	{"Kodak DC25", &kodak_dc2x},
	{"Kodak DC210", &kodak_dc210},
	{"Konica QM100", &konica_qm100},
	{"Konica QM100V", &konica_qm100},
	{"Minolta Dimmage", &olympus},
	{"Nikon CoolPix 100 (unverified)", &olympus},
	{"Nikon CoolPix 300 (unverified)", &olympus},
	{"Nikon CoolPix 600 (unverified)", &olympus},
	{"Nikon CoolPix 900", &olympus},
	{"Nikon CoolPix 900S", &olympus},
	{"Nikon CoolPix 950", &olympus},
	{"Olympus D-220L", &olympus},
	{"Olympus D-300L (unverified)", &olympus},
	{"Olympus D-320L", &olympus},
	{"Olympus D-340L", &olympus},
	{"Olympus D-500L", &olympus},
	{"Olympus D-600L", &olympus},
	{"Olympus C-400L", &olympus},
	{"Olympus C-410L (unverified)", &olympus},
	{"Olympus C-800L (unverified)", &olympus},
	{"Olympus C-820L", &olympus},
	{"Olympus C-900L Zoom", &olympus},
	{"Olympus C-1000L", &olympus},
	{"Olympus C-1400L", &olympus},
	{"Olympus C-2000Z", &olympus},
	{"Panasonic Coolshot KXl-600A (unverified)", &olympus},
	{"Philips ESP80", &philips},
	{"Polaroid PDC 640", &olympus},
	{"Ricoh RDC-300", &ricoh_300z},
        {"Ricoh RDC-300Z", &ricoh_300z},
  	{"Ricoh RDC-4200 (unverified)", &philips},
	{"Ricoh RDC-4300 (unverified)", &philips},
	{"Sanyo VPC-G210", &olympus},
	{"Sanyo VPC-G250", &olympus},
	{"Sony DSC-F1", &sony_dscf1},
	{"Sony DSC-F1 (unverified/other library)", &olympus},
	{"", NULL}
};










