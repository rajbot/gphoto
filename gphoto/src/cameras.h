extern struct _Camera konica_qm100;
extern struct _Camera konica_qm1xx;
extern struct _Camera konica_qm2xx;
extern struct _Camera canon;
extern struct _Camera casio_qv;
extern struct _Camera coolpix600;
extern struct _Camera philips;
extern struct _Camera fuji;
extern struct _Camera kodak_dc2x;
extern struct _Camera kodak_dc210;
extern struct _Camera kodak;
extern struct _Camera mustek_mdc800;
extern struct _Camera ricoh_300z;
extern struct _Camera olympus;
extern struct _Camera samsung800k;
extern struct _Camera sony_dscf1;
extern struct _Camera sony_dscf55;
extern struct _Camera sony_msac_sr1;
extern struct _Camera dimage_v;
extern struct _Camera directory;

struct Model cameras[] = {
  {"Browse Directory", &directory, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Agfa ePhoto 307", &olympus, "Gary del Valle <gdv@bewellnet.com>"},
  {"Agfa ePhoto 780", &olympus, "Eric Berenguier <bereng@cybercable.fr>\n<koch@math.utexas.edu>"},
  {"Agfa ePhoto 780C", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Agfa ePhoto 1280", &olympus, "Peter Kaempf <pkaempf@kaempf.ch>"},
  {"Agfa ePhoto 1680", &olympus, "Richard Caley <rjc@cstr.ed.ac.uk>"},
  {"Apple QuickTake 150", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Apple QuickTake 200", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Canon PowerShot A5", &canon, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Canon PowerShot A5 Zoom", &canon, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Canon PowerShot A50", &canon, "Peter Hunter <peter@hds.demon.co.uk>"},
  {"Canon PowerShot A70", &canon, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Canon PowerShot S10", &canon, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Canon PowerShot S20", &canon, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Casio QV-10", &casio_qv, "Gary Ross <gdr@hooked.net>"},
  {"Casio QV-10A", &casio_qv, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Casio QV-11", &casio_qv, "Reklaw <reklaw@disinfo.net>"},

  {"Casio QV-30", &casio_qv, "Dean Mills <deanmills@home.com>"},
  {"Casio QV-70", &casio_qv, "Russell Nelson <nelson@crynwr.com>"},
  {"Casio QV-100", &casio_qv, "Rolf Offermanns <rolf@kawo2.rwth-aachen.de>"},
  {"Casio QV-200", &casio_qv, "Nils Faerber <nils@unix-ag.org>"},
  {"Casio QV-700", &casio_qv, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Casio QV-5000SX", &casio_qv, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Chinon ES-1000", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Epson PhotoPC 500", &olympus, "Rene Adad <adren@wanadoo.fr>"},
  {"Epson PhotoPC 550", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Epson PhotoPC 600", &olympus, "Shunsuke Sumitani <blunted@purdue.edu>"},
  {"Epson PhotoPC 700", &olympus, "Berjoan Damien <damien@casm.insa-lyon.fr>"},
  {"Epson PhotoPC 800", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Fuji DS-7", &fuji, "Matt Martin <matt.martin@ieee.org>"},
  {"Fuji DX-5", &fuji, "Thomas Kuehn <th.kuehn@fh-wolfenbuettel.de>"},
  {"Fuji DX-10", &fuji, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Fuji MX-500", &fuji, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Fuji MX-600", &fuji, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Fuji MX-700", &fuji, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Fuji MX-1200", &fuji, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Fuji MX-2700", &fuji, "Trevor Boicey <tboicey@brit.ca>"},
  {"Hewlett Packard PhotoSmart C20", &konica_qm100, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Hewlett Packard PhotoSmart C30", &konica_qm100, "Simon Walker <simon@intent.demon.co.uk>"},
  {"Hewlett Packard PhotoSmart C200", &konica_qm100, "Stan Gregory <sgregory@sprynet.com>"},
  {"Kodak DC20", &kodak_dc2x, "Del Simmons <del@cimedia.com>"},
  {"Kodak DC25", &kodak_dc2x, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Kodak DC200+", &kodak_dc210, "William R. McDonough <wrmcd@wilmcd.com>"},
  {"Kodak DC210", &kodak_dc210, "Wesley (Buck) Lemke <wesley@dwave.net>"},
  {"Kodak DC210+ Zoom", &kodak_dc210, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Kodak DC215 Zoom", &kodak_dc210, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Kodak DC220+", &kodak, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Kodak DC240", &kodak, "Randy D. Scott <scottr@wwa.com> (serial)\nDavid Brownell <david-b@pacbell.net> (USB)"},
  {"Kodak DC280", &kodak, "Robert Kaye <robert@moon.eorbit.net>"},
  {"Konica QM100", &konica_qm100, "Phill Hugo <plh102@york.ac.uk>"},
  {"Konica QM100V", &konica_qm100, "Chris Worley <cworley@altatech.com>"},
  {"Konica Q-EZ", &konica_qm100, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Konica Q-M100(beta test)",  &konica_qm1xx, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Konica Q-M100V(beta test)", &konica_qm1xx, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Konica Q-M200(beta test)",  &konica_qm2xx, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Leica Digilux Zoom", &fuji, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Minolta Dimage V", &dimage_v, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Mustek MDC 800 v2",&mustek_mdc800, "Don Cockman <don@hauraki.co.uk>"},
  {"Nikon CoolPix 100", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Nikon CoolPix 300", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Nikon CoolPix 600", &coolpix600, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Nikon CoolPix 700", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Nikon CoolPix 800", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Nikon CoolPix 900", &olympus, "Russ Wagner <RussWagner@worlnet.att.net>"},
  {"Nikon CoolPix 900S", &olympus, "Ike Hishikawa <ike@whitedragon.org>"},
  {"Nikon CoolPix 950", &olympus, "Michael C. Piantedosi <drclaw@excelr8.net>"},
  {"Nikon CoolPix 950S", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Olympus D-100Z", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},

  {"Olympus D-200L", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Olympus D-220L", &olympus, "Scott Fritzinger <scottf@scs.unr.edu>"},
  {"Olympus D-300L", &olympus, "Andrew Burd <fretless@bookastudios.com>"},
  {"Olympus D-320L", &olympus, "Jon Stroud <jgstroud@eos.ncsu.edu>"},
  {"Olympus D-330R", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Olympus D-340L", &olympus, "Shaun Moran <Shaun@TheMorans.com>"},
  {"Olympus D-340R", &olympus, "Ben Konosky <bkonosky@texas.net>"},
  {"Olympus D-400L Zoom", &olympus, "Michael Ball <michael.ball@db.com>"},
  {"Olympus D-450Z", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Olympus D-500L", &olympus, "Wylie Swanson <wswanson@pingzero.net>"},
  {"Olympus D-600L", &olympus, "Wylie Swanson <wswanson@pingzero.net>"},
  {"Olympus D-620L", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Olympus C-400L", &olympus, "Ole Aamot <ole@gnu.org>"},
  {"Olympus C-410L", &olympus, "Martin Preishuber <Martin.Preishuber@stuco.uni-klu.ac.at>"},
  {"Olympus C-800L", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Olympus C-820L", &olympus, "Tore Halvorsen <torehalv@stud.ntnu.no>"},
  {"Olympus C-830L", &olympus, "Thorkild Stray <thorkild@ifi.uio.no>"},
  {"Olympus C-840L", &olympus, "Andrew Gray <andrew@mercury.demon.co.uk>"},
  {"Olympus C-900 Zoom", &olympus, "Per H. Isaksen <PerHenning.Isaksen@marintek.sintef.no>"},
  {"Olympus C-900L Zoom", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Olympus C-1000L", &olympus, "Gael LeDimet <gledimet@capgemini.fr>"},
  {"Olympus C-1400L", &olympus, "Oliver Graf <ograf@fga.de>"},
  {"Olympus C-2000Z", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Panasonic Coolshot KXl-600A", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Panasonic Cardshot NV-DCF5E", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Philips ESP60", &philips, "Scott Matthews <overhead@charter.net>"},
  {"Philips ESP80", &philips, "Bob Paauwe <bpaauwe@bobsplace.com>"},
  {"Polaroid PDC 640", &olympus, "Brad Willson <cpu@ifixcomputers.com>"},
  {"Ricoh RDC-300", &ricoh_300z, "Clifford Wright <cliff@snipe444.org>"},
  {"Ricoh RDC-300Z", &ricoh_300z, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Ricoh RDC-4200", &philips, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Ricoh RDC-4300", &philips, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Ricoh RDC-5000", &philips, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Samsung Kenox SSC-350N", &fuji, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Samsung Digimax 800K", &samsung800k, "James Mckenzie <james@fishsoup.dhs.org>"},
  {"Sanyo VPC-G210", &olympus, "Tim Fletcher <tim@night-shade.demon.co.uk>"},
  {"Sanyo VPC-G200", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Sanyo VPC-G250", &olympus, "Igor Sarychkin <igor@cccp.demon.co.uk>"},
  {"Sanyo VPC-X350", &olympus, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Sony DSC-F1", &sony_dscf1, "M. Adam Kendall <joker@penguinpub.com>"},
  {"Sony DSC-F55", &sony_dscf55, "Mark Davies <mdavies@dial.pipex.com>"},
  {"Sony DSC-F505", &sony_dscf55, "Mark Davies <mdavies@dial.pipex.com>"},
  {"Sony Memory Stick Adapter", &sony_dscf55, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Sony MSAC-SR1 and DCR-PC100", &sony_msac_sr1, "None since 2000/03/05.\nMail gphoto-devel@gphoto.org."},
  {"Toshiba PDR-M1", &fuji, "None since 2000/03/05.\ngphoto-devel@gphoto.org."},
  {"", NULL}
};




