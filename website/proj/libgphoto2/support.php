<?
	require ("../../include.php");

	printHeader ("Projects :: libgphoto2 :: supported cameras", "", 1);

	printMenu ("proj");
?>

<table cellpadding="3" cellspacing="3" witdh="100%">
<tr class="text"><td>
<p>
On this page, you find list of the supported cameras as of the 
current release.
</p>
<p>
Support for additional cameras may be in the current 
libgphoto2 SVN trunk code.
They will be added to the next release. 
</p>
<p>If your camera is neither supported in the current release nor in current SVN trunk, it is possible that
<ul>
<li>it is an old camera for which the original <a href="/proj/gphoto/">gPhoto</a> driver has not been ported yet (mostly due to lack of demand)</li>
<li>it is a new camera for which there is no support at all</li>
</ul>
See the 
<a href="http://www.teaser.fr/~hfiguiere/linux/digicam.html">Digital
Camera Support for UN*X</a> page for details about just any camera.
</p>

<pre>
========================================================================
Sun Jul 13 23:58:07 CEST 2008
========================================================================
gphoto2 2.4.2

Copyright (c) 2000-2008 Lutz Mueller and others

gphoto2 comes with NO WARRANTY, to the extent permitted by law. You may
redistribute copies of gphoto2 under the terms of the GNU General Public
License. For more information about these matters, see the files named COPYING.

This version of gphoto2 is using the following software versions and options:
gphoto2         2.4.2          gcc, popt(m), exif, cdk, aa, jpeg, readline
libgphoto2      2.4.2          gcc, ltdl, EXIF
libgphoto2_port 0.8.0          gcc, ltdl, USB, serial resmgr locking
========================================================================
Number of supported cameras: 1033
Supported cameras:
	"Achiever Digital Adc65"
	"AEG Snap 300"
	"Agfa ePhoto 1280"
	"Agfa ePhoto 1680"
	"Agfa ePhoto 307"
	"Agfa ePhoto 780"
	"Agfa ePhoto 780C"
	"Agfa ePhoto CL18"
	"Agfa ePhoto CL20" (EXPERIMENTAL)
	"Aiptek 1.3 mega PocketCam" (TESTING)
	"Aiptek PalmCam Trio"
	"Aiptek Pencam" (TESTING)
	"Aiptek PenCam Trio"
	"Aiptek PenCam VGA+" (TESTING)
	"Aiptek Pencam without flash" (TESTING)
	"Aiptek Smart Megacam" (TESTING)
	"Apple iPhone (PTP mode)"
	"Apple QuickTake 200"
	"Apple QuickTake 200"
	"Archos 104 (MTP mode)"
	"Archos 404 (MTP mode)"
	"Archos 504 (MTP mode)"
	"Archos 605 (MTP mode)"
	"Archos 704 mobile dvr"
	"Archos Gmini XS100"
	"Archos XS202 (MTP mode)"
	"Argus DC-100"
	"Argus DC-1500"
	"Argus DC-1510"
	"Argus DC-1610" (EXPERIMENTAL)
	"Argus DC-1620" (TESTING)
	"Argus DC-1730" (EXPERIMENTAL)
	"Argus DC-2000"
	"Argus DC-2200"
	"Argus QuickClix"
	"Barbie"
	"Benq DC1300" (TESTING)
	"Canon Digital IXUS"
	"Canon Digital IXUS 30 (PTP mode)"
	"Canon Digital IXUS 300"
	"Canon Digital IXUS 330"
	"Canon Digital IXUS 40 (PTP mode)"
	"Canon Digital IXUS 400 (PTP mode)"
	"Canon Digital IXUS 430 (normal mode)"
	"Canon Digital IXUS 430 (PTP mode)"
	"Canon Digital IXUS 50 (normal mode)"
	"Canon Digital IXUS 50 (PTP mode)"
	"Canon Digital IXUS 500 (PTP mode)"
	"Canon Digital IXUS 55 (PTP mode)"
	"Canon Digital IXUS 60 (PTP mode)"
	"Canon Digital IXUS 65 (PTP mode)"
	"Canon Digital IXUS 70 (PTP mode)"
	"Canon Digital IXUS 700 (PTP mode)"
	"Canon Digital IXUS 75 (PTP mode)"
	"Canon Digital IXUS 750 (PTP mode)"
	"Canon Digital IXUS 80 IS"
	"Canon Digital IXUS 800 (PTP mode)"
	"Canon Digital IXUS 850 IS (PTP mode)"
	"Canon Digital IXUS 860 IS"
	"Canon Digital IXUS 900Ti (PTP mode)"
	"Canon Digital IXUS 950 IS (PTP mode)"
	"Canon Digital IXUS 970 IS"
	"Canon Digital IXUS i (normal mode)"
	"Canon Digital IXUS i (PTP mode)"
	"Canon Digital IXUS i5 (normal mode)"
	"Canon Digital IXUS II (PTP mode)"
	"Canon Digital IXUS IIs"
	"Canon Digital IXUS IIs (PTP mode)"
	"Canon Digital IXUS iZ (PTP mode)"
	"Canon Digital IXUS v"
	"Canon Digital IXUS v2"
	"Canon Digital IXUS v3 (normal mode)"
	"Canon Digital IXUS v3 (PTP mode)"
	"Canon Digital IXUS Wireless (PTP mode)"
	"Canon Digital Rebel XT (normal mode)" (EXPERIMENTAL)
	"Canon Digital unknown 3"
	"Canon Elura 50 (normal mode)"
	"Canon Elura 50 (PTP mode)"
	"Canon Elura 65 (PTP mode)"
	"Canon EOS 10D"
	"Canon EOS 1D Mark II (PTP mode)"
	"Canon EOS 20D (normal mode)" (EXPERIMENTAL)
	"Canon EOS 20D (PTP mode)"
	"Canon EOS 300D (normal mode)"
	"Canon EOS 300D (PTP mode)"
	"Canon EOS 30D (PTP mode)"
	"Canon EOS 350D"
	"Canon EOS 350D (normal mode)" (EXPERIMENTAL)
	"Canon EOS 350D (PTP mode)"
	"Canon EOS 400D (PTP mode)"
	"Canon EOS 40D (PTP mode)"
	"Canon EOS 450D (PTP mode)"
	"Canon EOS 5D (normal mode)" (EXPERIMENTAL)
	"Canon EOS 5D (PTP mode)"
	"Canon EOS D30"
	"Canon EOS D60"
	"Canon EOS Digital Rebel (normal mode)"
	"Canon EOS Digital Rebel (PTP mode)"
	"Canon EOS Kiss Digital (normal mode)"
	"Canon EOS Kiss Digital (PTP mode)"
	"Canon EOS Kiss Digital N (normal mode)" (EXPERIMENTAL)
	"Canon FV M1 (normal mode)"
	"Canon IXY DIGITAL"
	"Canon IXY DIGITAL 300"
	"Canon IXY Digital 430 (normal mode)"
	"Canon IXY Digital 55 (normal mode)"
	"Canon IXY Digital L2 (normal mode)"
	"Canon IXY DV M"
	"Canon IXY DV M2 (normal mode)"
	"Canon MV630i (normal mode)"
	"Canon MV650i (normal mode)"
	"Canon MV750i (PTP mode)"
	"Canon MVX 10i (normal mode)"
	"Canon MVX 3i (normal mode)"
	"Canon MVX100i"
	"Canon MVX100i"
	"Canon MVX150i (normal mode)"
	"Canon MVX25i (normal mode)"
	"Canon MVX2i"
	"Canon MVX3i (PTP mode)"
	"Canon Optura 10"
	"Canon Optura 10"
	"Canon Optura 20"
	"Canon Optura 20 (normal mode)"
	"Canon Optura 200 MC"
	"Canon Optura 300 (normal mode)"
	"Canon Optura 40 (normal mode)"
	"Canon Optura 600 (PTP mode)"
	"Canon Optura Xi (normal mode)"
	"Canon PowerShot A10"
	"Canon PowerShot A100"
	"Canon PowerShot A20"
	"Canon PowerShot A200"
	"Canon PowerShot A30"
	"Canon PowerShot A300"
	"Canon PowerShot A300 (PTP mode)"
	"Canon PowerShot A310"
	"Canon PowerShot A310 (PTP mode)"
	"Canon PowerShot A40"
	"Canon PowerShot A400"
	"Canon PowerShot A400 (PTP mode)"
	"Canon PowerShot A410 (PTP mode)"
	"Canon PowerShot A420 (PTP mode)"
	"Canon PowerShot A430 (PTP mode)"
	"Canon PowerShot A450 (PTP mode)"
	"Canon PowerShot A460 (PTP mode)"
	"Canon PowerShot A5"
	"Canon PowerShot A5 Zoom"
	"Canon PowerShot A50"
	"Canon PowerShot A510 (normal mode)"
	"Canon PowerShot A510 (PTP mode)"
	"Canon PowerShot A520 (PTP mode)"
	"Canon PowerShot A530 (PTP mode)"
	"Canon PowerShot A540 (PTP mode)"
	"Canon PowerShot A550 (PTP mode)"
	"Canon PowerShot A560 (PTP mode)"
	"Canon PowerShot A570 IS (PTP mode)"
	"Canon PowerShot A590 IS"
	"Canon PowerShot A60"
	"Canon PowerShot A60 (PTP)"
	"Canon PowerShot A610 (PTP mode)"
	"Canon PowerShot A620 (PTP mode)"
	"Canon PowerShot A630 (PTP mode)"
	"Canon PowerShot A640 (PTP mode)"
	"Canon PowerShot A640 (PTP/MTP mode)"
	"Canon PowerShot A70 (PTP)"
	"Canon PowerShot A700 (PTP mode)"
	"Canon PowerShot A710 IS (PTP mode)"
	"Canon PowerShot A720 IS (PTP mode)"
	"Canon PowerShot A75"
	"Canon PowerShot A75 (PTP mode)"
	"Canon PowerShot A80 (PTP)"
	"Canon PowerShot A85 (normal mode)"
	"Canon PowerShot A85 (PTP mode)"
	"Canon PowerShot A95 (PTP mode)"
	"Canon PowerShot G1"
	"Canon PowerShot G2"
	"Canon PowerShot G3 (normal mode)"
	"Canon PowerShot G3 (PTP mode)"
	"Canon PowerShot G5 (normal mode)"
	"Canon PowerShot G5 (PTP mode)"
	"Canon PowerShot G6 (normal mode)"
	"Canon Powershot G6 (PTP mode)"
	"Canon PowerShot G7 (PTP mode)"
	"Canon PowerShot G9 (PTP mode)"
	"Canon PowerShot IXY Digital L (normal mode)"
	"Canon PowerShot Pro70"
	"Canon PowerShot Pro90 IS"
	"Canon PowerShot S1 IS (PTP mode)"
	"Canon PowerShot S10"
	"Canon PowerShot S100"
	"Canon PowerShot S110"
	"Canon PowerShot S2 IS (PTP mode)"
	"Canon PowerShot S20"
	"Canon PowerShot S200"
	"Canon PowerShot S230 (normal mode)"
	"Canon PowerShot S230 (PTP mode)"
	"Canon PowerShot S3 IS (PTP mode)"
	"Canon PowerShot S30"
	"Canon PowerShot S300"
	"Canon PowerShot S330"
	"Canon PowerShot S40"
	"Canon PowerShot S400 (PTP mode)"
	"Canon PowerShot S410 (PTP mode)"
	"Canon PowerShot S410 Digital ELPH (normal mode)"
	"Canon PowerShot S45 (normal mode)"
	"Canon PowerShot S45 (PTP mode)"
	"Canon PowerShot S5 IS (PTP mode)"
	"Canon PowerShot S50 (PTP mode)"
	"Canon PowerShot S500 (PTP mode)"
	"Canon PowerShot S60 (normal mode)"
	"Canon Powershot S60 (PTP mode)"
	"Canon PowerShot S70 (normal mode)"
	"Canon Powershot S70 (PTP mode)"
	"Canon PowerShot S80 (PTP mode)"
	"Canon PowerShot SD10 Digital ELPH (normal mode)"
	"Canon PowerShot SD100 (PTP mode)"
	"Canon PowerShot SD1000 (PTP mode)"
	"Canon PowerShot SD110 (PTP mode)"
	"Canon PowerShot SD110 Digital ELPH"
	"Canon PowerShot SD20 (normal mode)"
	"Canon PowerShot SD200 (PTP mode)"
	"Canon PowerShot SD40 (PTP mode)"
	"Canon PowerShot SD400 (normal mode)"
	"Canon PowerShot SD430 (PTP mode)"
	"Canon PowerShot SD450 (PTP mode)"
	"Canon PowerShot SD500 (PTP mode)"
	"Canon PowerShot SD600 (PTP mode)"
	"Canon PowerShot SD630 (PTP mode)"
	"Canon PowerShot SD700 (PTP mode)"
	"Canon PowerShot SD750 (PTP mode)"
	"Canon PowerShot SD850 (PTP mode)"
	"Canon PowerShot SD900 (PTP mode)"
	"Canon Powershot SX100 IS (PTP mode)"
	"Canon PowerShot unknown 1"
	"Canon PowerShot unknown 2"
	"Canon ZR70MC (normal mode)"
	"Casio EX-S770"
	"Casio EX-Z120"
	"Casio EX-Z700"
	"Casio LV 10" (EXPERIMENTAL)
	"Casio QV10"
	"Casio QV100"
	"Casio QV10A"
	"Casio QV300"
	"Casio QV70"
	"Casio QV700"
	"Casio QV770"
	"Che-ez Snap" (EXPERIMENTAL)
	"Che-Ez Snap SNAP-U" (EXPERIMENTAL)
	"Che-ez! Babe"
	"Che-ez! Splash" (EXPERIMENTAL)
	"Che-ez! SPYZ"
	"Chinon ES-1000"
	"Clever CAM 360" (TESTING)
	"Concord Eye-Q Duo" (EXPERIMENTAL)
	"Concord Eye-Q Easy" (EXPERIMENTAL)
	"Concord EyeQ 4330" (EXPERIMENTAL)
	"Concord EyeQMini_1" (EXPERIMENTAL)
	"Concord EyeQMini_2" (EXPERIMENTAL)
	"CoolCam CP086"
	"Cowon iAudio 7 (MTP mode)"
	"Cowon iAudio D2 (MTP mode)"
	"Cowon iAudio U3 (MTP mode)"
	"Creative Go Mini"
	"Creative PC-CAM 300" (EXPERIMENTAL)
	"Creative PC-CAM350" (EXPERIMENTAL)
	"Creative PC-CAM600" (EXPERIMENTAL)
	"Creative PC-CAM750" (EXPERIMENTAL)
	"Creative Portable Media Center"
	"Creative ZEN"
	"Creative ZEN Micro (MTP mode)"
	"Creative ZEN MicroPhoto"
	"Creative ZEN Sleek (MTP mode)"
	"Creative ZEN Sleek Photo"
	"Creative ZEN Touch (MTP mode)"
	"Creative ZEN V"
	"Creative ZEN V 2GB"
	"Creative ZEN V Plus"
	"Creative ZEN Vision"
	"Creative ZEN Vision W"
	"Creative ZEN Vision:M"
	"Creative ZEN Vision:M (DVP-HD0004)"
	"Creative ZEN Xtra (MTP mode)"
	"D-Link DSC 350+" (TESTING)
	"D-Link DSC 350F" (TESTING)
	"D-MAX DM3588" (EXPERIMENTAL)
	"DC-N130t" (EXPERIMENTAL)
	"DC-N130t" (EXPERIMENTAL)
	"DC31VC" (EXPERIMENTAL)
	"Dell Dell Pocket DJ (MTP mode)"
	"Dell DJ (2nd generation)"
	"Dell, Inc DJ Itty"
	"Digigr8" (EXPERIMENTAL)
	"Digital camera, CD302N" (EXPERIMENTAL)
	"Digitaldream DIGITAL 2000"
	"DigitalDream Enigma1.3" (EXPERIMENTAL)
	"DigitalDream l'elegante"
	"DigitalDream l'elite"
	"DigitalDream l'espion"
	"DigitalDream l'espion XS"
	"DigitalDream l'espion xtra" (EXPERIMENTAL)
	"DigitalDream l'esprit"
	"DigitalDream la ronde"
	"Directory Browse"
	"Disney MixMax"
	"Disney pix micro" (EXPERIMENTAL)
	"Dunlop MP3 player 1GB / EGOMAN MD223AFD"
	"Dynatron Dynacam 800"
	"Elta Medi@ digi-cam" (EXPERIMENTAL)
	"Emprex PCD3600" (EXPERIMENTAL)
	"Epson PhotoPC 3000z"
	"Epson PhotoPC 500"
	"Epson PhotoPC 550"
	"Epson PhotoPC 600"
	"Epson PhotoPC 650"
	"Epson PhotoPC 700"
	"Epson PhotoPC 800"
	"Epson PhotoPC 850z"
	"FOMA D905i"
	"FOMA F903iX HIGH-SPEED"
	"Fuji Axia Eyeplate" (EXPERIMENTAL)
	"Fuji Axia Slimshot" (EXPERIMENTAL)
	"Fuji DS-7"
	"Fuji DX-10"
	"Fuji DX-5"
	"Fuji DX-7"
	"Fuji FinePix A330"
	"Fuji FinePix A800"
	"Fuji FinePix A820"
	"Fuji FinePix E900"
	"Fuji FinePix F20"
	"Fuji FinePix F30"
	"Fuji FinePix F31fd"
	"Fuji FinePix F40fd"
	"Fuji FinePix F50fd"
	"Fuji FinePix S100fs"
	"Fuji FinePix S5700"
	"Fuji FinePix S6500fd"
	"Fuji FinePix S7000"
	"Fuji FinePix S9500"
	"Fuji FinePix Z100fd"
	"Fuji IX-1"
	"Fuji MX-1200"
	"Fuji MX-1700"
	"Fuji MX-2700"
	"Fuji MX-2900"
	"Fuji MX-500"
	"Fuji MX-600"
	"Fuji MX-700"
	"FujiFilm @xia ix-100"
	"Gear to go" (EXPERIMENTAL)
	"Generic SoundVision Clarity2"
	"Genius Smart 300, version 2" (EXPERIMENTAL)
	"GrandTek ScopeCam" (TESTING)
	"GTW Electronics" (EXPERIMENTAL)
	"Haier Ibiza Rhapsody"
	"Haier Ibiza Rhapsody"
	"Haimei Electronics HE-501A" (EXPERIMENTAL)
	"Hawking DC120 Pocketcam"
	"Hot Wheels"
	"HP PhotoSmart"
	"HP PhotoSmart 120 (PTP mode)"
	"HP PhotoSmart 215" (EXPERIMENTAL)
	"HP PhotoSmart 217 (PTP mode)"
	"HP PhotoSmart 317 (PTP mode)"
	"HP PhotoSmart 318 (PTP mode)"
	"HP PhotoSmart 320 (PTP mode)"
	"HP PhotoSmart 407 (PTP mode)"
	"HP PhotoSmart 417 (PTP mode)"
	"HP PhotoSmart 43x (PTP mode)"
	"HP PhotoSmart 507 (PTP mode)"
	"HP PhotoSmart 517 (PTP mode)"
	"HP PhotoSmart 607 (PTP mode)"
	"HP PhotoSmart 612 (PTP mode)"
	"HP PhotoSmart 618"
	"HP PhotoSmart 620 (PTP mode)"
	"HP PhotoSmart 635 (PTP mode)"
	"HP PhotoSmart 707 (PTP mode)"
	"HP PhotoSmart 715 (PTP mode)"
	"HP PhotoSmart 717 (PTP mode)"
	"HP PhotoSmart 720 (PTP mode)"
	"HP PhotoSmart 733 (PTP mode)"
	"HP PhotoSmart 735 (PTP mode)"
	"HP PhotoSmart 812 (PTP mode)"
	"HP PhotoSmart 817 (PTP mode)"
	"HP PhotoSmart 818 (PTP mode)"
	"HP PhotoSmart 850 (PTP mode)"
	"HP PhotoSmart 912"
	"HP PhotoSmart 935 (PTP mode)"
	"HP PhotoSmart 945 (PTP mode)"
	"HP PhotoSmart C20"
	"HP PhotoSmart C200"
	"HP PhotoSmart C30"
	"HP PhotoSmart C500"
	"HP PhotoSmart C500 (PTP mode)"
	"HP PhotoSmart C500 2"
	"HP PhotoSmart E327 (PTP mode)"
	"HP PhotoSmart E427 (PTP mode)"
	"HP PhotoSmart M22 (PTP mode)"
	"HP PhotoSmart M23 (PTP mode)"
	"HP PhotoSmart M307 (PTP mode)"
	"HP PhotoSmart M407 (PTP mode)"
	"HP PhotoSmart M415 (PTP mode)"
	"HP PhotoSmart M425 (PTP mode)"
	"HP PhotoSmart M525 (PTP mode)"
	"HP PhotoSmart M527 (PTP mode)"
	"HP PhotoSmart M547 (PTP mode)"
	"HP PhotoSmart M725 (PTP mode)"
	"HP PhotoSmart M727 (PTP mode)"
	"HP PhotoSmart M737 (PTP mode)"
	"HP PhotoSmart R742 (PTP mode)"
	"HP PhotoSmart R927 (PTP mode)"
	"HP PhotoSmart R967 (PTP mode)"
	"iClick 5X" (EXPERIMENTAL)
	"iConcepts digital camera"
	"INNOVAGE Mini Digital, CD302N" (TESTING)
	"Insignia NS-DV45"
	"Insignia Pilot 4GB"
	"Insignia Sport Player"
	"Intel Bandon Portable Media Center"
	"Intel Pocket PC Camera" (EXPERIMENTAL)
	"IOMagic MagicImage 400"
	"IOMagic MagicImage 420"
	"ION digital camera" (TESTING)
	"iRiver Clix"
	"iRiver Clix2"
	"iRiver H10"
	"iRiver H10 20GB"
	"iRiver iFP-880"
	"iRiver N12"
	"iRiver Portable Media Center"
	"iRiver Portable Media Center"
	"iRiver T10"
	"iRiver T10 2GB"
	"iRiver T10a"
	"iRiver T20"
	"iRiver T20"
	"iRiver T20 FM"
	"iRiver T30"
	"iRiver T60"
	"iRiver U10"
	"iRiver X20"
	"Isabella Her Prototype"
	"Ixla DualCam 640" (EXPERIMENTAL)
	"Jazz JDC9" (EXPERIMENTAL)
	"Jenoptik JD-3300z3" (EXPERIMENTAL)
	"Jenoptik JD-4100z3" (EXPERIMENTAL)
	"Jenoptik JD11"
	"Jenoptik JD12 800ff"
	"Jenoptik JD350 entrance" (TESTING)
	"Jenoptik JD350 video" (TESTING)
	"Jenoptik JDC 350" (EXPERIMENTAL)
	"JVC Alneo XA-HD500"
	"KBGear JamCam"
	"Kenwood Media Keg HD10GB7 Sport Player"
	"Kodak C300"
	"Kodak C310"
	"Kodak C330"
	"Kodak C340"
	"Kodak C360"
	"Kodak C433"
	"Kodak C530"
	"Kodak C533"
	"Kodak C613"
	"Kodak C633"
	"Kodak C643"
	"Kodak C653"
	"Kodak C743"
	"Kodak C875"
	"Kodak CD33"
	"Kodak CX4200"
	"Kodak CX4210"
	"Kodak CX4230"
	"Kodak CX4300"
	"Kodak CX4310"
	"Kodak CX6200"
	"Kodak CX6230"
	"Kodak CX6330"
	"Kodak CX6445"
	"Kodak CX7220"
	"Kodak CX7300"
	"Kodak CX7310"
	"Kodak CX7330"
	"Kodak CX7430"
	"Kodak CX7525"
	"Kodak CX7530"
	"Kodak DC120"
	"Kodak DC210"
	"Kodak DC215"
	"Kodak DC220"
	"Kodak DC240"
	"Kodak DC240 (PTP mode)"
	"Kodak DC260"
	"Kodak DC265"
	"Kodak DC280"
	"Kodak DC290"
	"Kodak DC3200"
	"Kodak DC3400"
	"Kodak DC4800"
	"Kodak DC5000"
	"Kodak DX3215"
	"Kodak DX3500"
	"Kodak DX3600"
	"Kodak DX3700"
	"Kodak DX3900"
	"Kodak DX4330"
	"Kodak DX4530"
	"Kodak DX4900"
	"Kodak DX6340"
	"Kodak DX6440"
	"Kodak DX6490"
	"Kodak DX7440"
	"Kodak DX7590"
	"Kodak DX7630"
	"Kodak EZ200"
	"Kodak LS420"
	"Kodak LS443"
	"Kodak LS663"
	"Kodak LS743"
	"Kodak LS753"
	"Kodak M753"
	"Kodak M883"
	"Kodak MC3"
	"Kodak P850"
	"Kodak P880"
	"Kodak V530"
	"Kodak V550"
	"Kodak V570"
	"Kodak V603"
	"Kodak V610"
	"Kodak V705"
	"Kodak Z612"
	"Kodak Z650"
	"Kodak Z700"
	"Kodak Z712 IS"
	"Kodak Z730"
	"Kodak Z740"
	"Kodak Z7590"
	"Kodak Z812 IS"
	"Kodak ZD710"
	"Konica e-mini"
	"Konica Q-EZ"
	"Konica Q-M100"
	"Konica Q-M100V"
	"Konica Q-M150" (EXPERIMENTAL)
	"Konica Q-M200"
	"Konica-Minolta DiMAGE A2 (PTP mode)"
	"Konica-Minolta DiMAGE A200 (PictBridge mode)"
	"Konica-Minolta DiMAGE X21 (PictBridge mode)"
	"Konica-Minolta DiMAGE Z2 (PictBridge mode)"
	"Konica-Minolta DiMAGE Z3 (PictBridge mode)"
	"Konica-Minolta DiMAGE Z5 (PictBridge mode)"
	"Largan Lmini" (EXPERIMENTAL)
	"Leica D-LUX 2"
	"Leica Digilux Zoom"
	"LG T5100" (EXPERIMENTAL)
	"LG UP3"
	"Lifetec LT 5995" (EXPERIMENTAL)
	"Logik LOG DAX MP3 and DAB Player"
	"Logitech Clicksmart 310" (TESTING)
	"Logitech Pocket Digital" (EXPERIMENTAL)
	"Maginon SX-410z" (EXPERIMENTAL)
	"Maginon SX330z" (EXPERIMENTAL)
	"Magpix B350" (EXPERIMENTAL)
	"Mass Storage Camera"
	"Maxell Max Pocket" (TESTING)
	"Media-Tech mt-406"
	"Medion MD 5319" (TESTING)
	"Medion MD 6000" (EXPERIMENTAL)
	"Medion MD 6126"
	"Medion MD 9700" (EXPERIMENTAL)
	"Micro-Star International P610/Model MS-5557"
	"Micromaxx Digital Camera"
	"Microsoft Zune"
	"Mini Shotz ms-350" (EXPERIMENTAL)
	"Minolta Dimage V"
	"Minton S-Cam F5" (TESTING)
	"Mitek CD10" (EXPERIMENTAL)
	"Mitek CD30P" (EXPERIMENTAL)
	"Motorola A1200"
	"Motorola K1"
	"Motorola RAZR2 V8/U9"
	"Motorola V3m verizon"
	"MTP Device" (TESTING)
	"Mustek gSmart 300" (EXPERIMENTAL)
	"Mustek gSmart 350" (EXPERIMENTAL)
	"Mustek gSmart mini" (TESTING)
	"Mustek gSmart mini 2" (TESTING)
	"Mustek gSmart mini 3" (TESTING)
	"Mustek VDC-3500"
	"Nexxtech Mini Digital Camera" (EXPERIMENTAL)
	"Nick Click"
	"Nikon CoolPix 100"
	"Nikon Coolpix 2000 (PTP mode)"
	"Nikon Coolpix 2100 (PTP mode)"
	"Nikon CoolPix 2100 (Sierra Mode)"
	"Nikon Coolpix 2200 (PTP mode)"
	"Nikon Coolpix 2500 (PTP mode)"
	"Nikon CoolPix 2500 (Sierra Mode)"
	"Nikon CoolPix 300"
	"Nikon Coolpix 3100 (PTP mode)"
	"Nikon Coolpix 3200 (PTP mode)"
	"Nikon Coolpix 3500 (PTP mode)"
	"Nikon CoolPix 3500 (Sierra Mode)"
	"Nikon Coolpix 3700 (PTP mode)"
	"Nikon Coolpix 4100 (PTP mode)"
	"Nikon Coolpix 4200 (PTP mode)"
	"Nikon Coolpix 4300 (PTP mode)"
	"Nikon CoolPix 4300 (Sierra Mode)"
	"Nikon Coolpix 4500 (PTP mode)"
	"Nikon Coolpix 4600 (PTP mode)"
	"Nikon Coolpix 4600a (PTP mode)"
	"Nikon Coolpix 4800 (PTP mode)"
	"Nikon Coolpix 5000 (PTP mode)"
	"Nikon Coolpix 5200 (PTP mode)"
	"Nikon Coolpix 5400 (PTP mode)"
	"Nikon Coolpix 5600 (PTP mode)"
	"Nikon Coolpix 5700 (PTP mode)"
	"Nikon Coolpix 5900 (PTP mode)"
	"Nikon CoolPix 600"
	"Nikon CoolPix 700"
	"Nikon Coolpix 7900 (PTP mode)"
	"Nikon CoolPix 800"
	"Nikon CoolPix 880"
	"Nikon Coolpix 885 (PTP mode)"
	"Nikon CoolPix 900"
	"Nikon CoolPix 900S"
	"Nikon CoolPix 910"
	"Nikon CoolPix 950"
	"Nikon CoolPix 950S"
	"Nikon CoolPix 990"
	"Nikon CoolPix 995"
	"Nikon Coolpix L1 (PTP mode)"
	"Nikon Coolpix L10 (PTP mode)"
	"Nikon Coolpix L11 (PTP mode)"
	"Nikon Coolpix L12 (PTP mode)"
	"Nikon Coolpix L3 (PTP mode)"
	"Nikon Coolpix L4 (PTP mode)"
	"Nikon Coolpix P1 (PTP mode)"
	"Nikon Coolpix P2 (PTP mode)"
	"Nikon Coolpix P4 (PTP mode)"
	"Nikon Coolpix P5000 (PTP mode)"
	"Nikon Coolpix P5100 (PTP mode)"
	"Nikon Coolpix P60 (PTP mode)"
	"Nikon Coolpix S2 (PTP mode)"
	"Nikon Coolpix S200 (PTP mode)"
	"Nikon Coolpix S4 (PTP mode)"
	"Nikon Coolpix S500 (PTP mode)"
	"Nikon Coolpix S6 (PTP mode)"
	"Nikon Coolpix SQ (PTP mode)"
	"Nikon D100 (Sierra Mode)"
	"Nikon D2H SLR (PTP mode)"
	"Nikon D2X SLR (PTP mode)"
	"Nikon D3 (PTP mode)"
	"Nikon D50 (PTP mode)"
	"Nikon DSC D100 (PTP mode)"
	"Nikon DSC D200 (PTP mode)"
	"Nikon DSC D300 (PTP mode)"
	"Nikon DSC D40 (PTP mode)"
	"Nikon DSC D40x (PTP mode)"
	"Nikon DSC D60 (PTP mode)"
	"Nikon DSC D70 (PTP mode)"
	"Nikon DSC D70s (PTP mode)"
	"Nikon DSC D80 (PTP mode)"
	"nisis Quickpix Qp3" (TESTING)
	"Nokia 3109c Mobile Phone"
	"Nokia 3110c Mobile Phone"
	"Nokia 5300 Mobile Phone"
	"Nokia 5700 XpressMusic Mobile Phone"
	"Nokia N73 Mobile Phone"
	"Nokia N75 Mobile Phone"
	"Nokia N80 Internet Edition (Media Player)"
	"Nokia N81 Mobile Phone"
	"Nokia N95 Mobile Phone"
	"Nokia N95 Mobile Phone 8GB"
	"Novatech Digital Camera CC30" (EXPERIMENTAL)
	"Olympus C-1000L"
	"Olympus C-1400L"
	"Olympus C-1400XL"
	"Olympus C-2000Z"
	"Olympus C-2020Z"
	"Olympus C-2040Z"
	"Olympus C-2100UZ"
	"Olympus C-2500L"
	"Olympus C-3000Z"
	"Olympus C-3020Z"
	"Olympus C-3030Z"
	"Olympus C-3040Z"
	"Olympus C-310Z"
	"Olympus C-350Z"
	"Olympus C-370Z"
	"Olympus C-400"
	"Olympus C-400L"
	"Olympus C-4040Z"
	"Olympus C-410"
	"Olympus C-410L"
	"Olympus C-420"
	"Olympus C-420L"
	"Olympus C-5050Z"
	"Olympus C-5500Z"
	"Olympus C-55Z"
	"Olympus C-700UZ"
	"Olympus C-750UZ"
	"Olympus C-770UZ"
	"Olympus C-800"
	"Olympus C-800L"
	"Olympus C-820"
	"Olympus C-820L"
	"Olympus C-830L"
	"Olympus C-840L"
	"Olympus C-860L"
	"Olympus C-900 Zoom"
	"Olympus C-900L Zoom"
	"Olympus C-990 Zoom"
	"Olympus D-100Z"
	"Olympus D-200L"
	"Olympus D-220L"
	"Olympus D-300L"
	"Olympus D-320L"
	"Olympus D-330R"
	"Olympus D-340L"
	"Olympus D-340R"
	"Olympus D-360L"
	"Olympus D-400L Zoom"
	"Olympus D-450Z"
	"Olympus D-460Z"
	"Olympus D-500L"
	"Olympus D-535Z"
	"Olympus D-540Z"
	"Olympus D-560Z"
	"Olympus D-600L"
	"Olympus D-600XL"
	"Olympus D-620L"
	"Olympus fe-200"
	"Olympus IR-300"
	"Olympus mju 500"
	"Olympus SP-500UZ"
	"Olympus X-100"
	"Olympus X-250"
	"Olympus X-450"
	"Oregon Scientific DShot II"
	"Oregon Scientific DShot III"
	"Palm / Handspring Pocket Tunes"
	"Palm Handspring Pocket Tunes 4"
	"Panasonic Coolshot KXL-600A"
	"Panasonic Coolshot KXL-601A"
	"Panasonic Coolshot NV-DCF5E"
	"Panasonic DC1000"
	"Panasonic DC1580"
	"Panasonic DMC-FZ20"
	"Panasonic DMC-FZ50"
	"Panasonic DMC-LC1"
	"Panasonic DMC-LS3"
	"Panasonic DMC-LZ2"
	"Panasonic Lumix FZ5"
	"Panasonic PV-L691"
	"Panasonic PV-L859"
	"Pencam TEVION MD 9456"
	"Pentax Optio 33WR"
	"Pentax Optio 43WR"
	"Pentax Optio 450"
	"Philips ESP2" (EXPERIMENTAL)
	"Philips ESP50" (EXPERIMENTAL)
	"Philips ESP60" (EXPERIMENTAL)
	"Philips ESP70" (EXPERIMENTAL)
	"Philips ESP80" (EXPERIMENTAL)
	"Philips ESP80SXG" (EXPERIMENTAL)
	"Philips GoGear Audio"
	"Philips GoGear SA3345"
	"Philips GoGear SA5145"
	"Philips GoGear SA6014/SA6015/SA6024/SA6025/SA6044/SA6045"
	"Philips GoGear SA6125/SA6145/SA6185"
	"Philips GoGear SA9200"
	"Philips HDD085/00 or HDD082/17"
	"Philips HDD1630/17"
	"Philips HDD6320"
	"Philips HDD6320/00 or HDD6330/17"
	"Philips P44417B keychain camera" (TESTING)
	"Philips PSA235"
	"Philips PSA610"
	"Philips SA1115/55"
	"Philips Shoqbox"
	"Phoebe Smartcam"
	"Pioneer DVR-LX60D"
	"Pixart Gemini Keychain Camera" (TESTING)
	"Pixie Princess Jelly-Soft" (EXPERIMENTAL)
	"PockCam" (EXPERIMENTAL)
	"Polaroid 640SE" (EXPERIMENTAL)
	"Polaroid DC700"
	"Polaroid Fun Flash 640" (EXPERIMENTAL)
	"Polaroid Fun! 320" (EXPERIMENTAL)
	"Polaroid PDC 2300Z"
	"Polaroid PDC 640"
	"Praktica QD500"
	"Praktica QD800"
	"Praktica Slimpix" (EXPERIMENTAL)
	"Precision Mini Digital Camera"
	"Precision Mini, Model HA513A" (EXPERIMENTAL)
	"Pretec dc530" (EXPERIMENTAL)
	"PTP/IP Camera" (TESTING)
	"PureDigital Ritz Disposable" (TESTING)
	"Quark Probe 99"
	"QuickPix QP1"
	"Radioshack Flatfoto" (EXPERIMENTAL)
	"RCA CDS1005" (EXPERIMENTAL)
	"Relisys Dimera 3500"
	"Request Ultra Slim" (EXPERIMENTAL)
	"Ricoh Capilo RX"
	"Ricoh Caplio 300G"
	"Ricoh Caplio G3"
	"Ricoh Caplio G4"
	"Ricoh Caplio GX"
	"Ricoh Caplio GX (PTP mode)"
	"Ricoh Caplio GX 8"
	"Ricoh Caplio GX 8 (PTP mode)"
	"Ricoh Caplio R1"
	"Ricoh Caplio R1v"
	"Ricoh Caplio R1v (PTP mode)"
	"Ricoh Caplio R2"
	"Ricoh Caplio R3"
	"Ricoh Caplio R3 (PTP mode)"
	"Ricoh Caplio R4"
	"Ricoh Caplio R5"
	"Ricoh Caplio R5 (PTP mode)"
	"Ricoh Caplio RR30"
	"Ricoh Caplio RR750 (PTP mode)"
	"Ricoh Caplio RZ1"
	"Ricoh RDC-1" (EXPERIMENTAL)
	"Ricoh RDC-100G" (EXPERIMENTAL)
	"Ricoh RDC-2" (EXPERIMENTAL)
	"Ricoh RDC-2E" (EXPERIMENTAL)
	"Ricoh RDC-300" (EXPERIMENTAL)
	"Ricoh RDC-300Z" (EXPERIMENTAL)
	"Ricoh RDC-4200" (EXPERIMENTAL)
	"Ricoh RDC-4300" (EXPERIMENTAL)
	"Ricoh RDC-5000" (EXPERIMENTAL)
	"Rollei dr5"
	"Rollei dr5 (PTP mode)"
	"Sakar 23070  Crayola Digital Camera" (EXPERIMENTAL)
	"Sakar 28290 and 28292  Digital Concepts Styleshot" (EXPERIMENTAL)
	"Sakar 92045  Spiderman" (EXPERIMENTAL)
	"Sakar Digital Keychain 11199" (EXPERIMENTAL)
	"Sakar Digital no, 6637x" (EXPERIMENTAL)
	"Sakar Digital no, 67480" (EXPERIMENTAL)
	"Sakar Digital no. 56379 Spyshot" (TESTING)
	"Sakar Digital no. 77379" (EXPERIMENTAL)
	"Sakar Kidz Cam" (EXPERIMENTAL)
	"Sakar Micro Digital 2428x" (EXPERIMENTAL)
	"Sakar no. 1638x CyberPix" (EXPERIMENTAL)
	"Samsung digimax 800k"
	"Samsung Juke (SCH-U470)"
	"Samsung Kenox SSC-350N"
	"Samsung U600 Mobile Phone"
	"Samsung X830 Mobile Phone"
	"Samsung YH-820"
	"Samsung YH-920"
	"Samsung YH-925(-GS)"
	"Samsung YH-925GS"
	"Samsung YH-999 Portable Media Center/SGH-A707/SGH-L760V"
	"Samsung YH-J70J"
	"Samsung YP-900"
	"Samsung YP-F2J"
	"Samsung YP-K3"
	"Samsung YP-K5"
	"Samsung YP-P2"
	"Samsung YP-S5"
	"Samsung YP-T10"
	"Samsung YP-T7J"
	"Samsung YP-T9"
	"Samsung YP-U2J (YP-U2JXB/XAA)"
	"Samsung YP-U3"
	"Samsung YP-Z5"
	"Samsung YP-Z5 2GB"
	"SanDisk Sansa c150"
	"SanDisk Sansa c240/c250"
	"SanDisk Sansa Clip"
	"SanDisk Sansa Connect"
	"SanDisk Sansa e200/e250/e260/e270/e280"
	"SanDisk Sansa e280"
	"SanDisk Sansa e280 v2"
	"SanDisk Sansa Express"
	"SanDisk Sansa Fuze"
	"SanDisk Sansa m230/m240"
	"SanDisk Sansa m240"
	"SanDisk Sansa View"
	"Sanyo DSC-X300"
	"Sanyo DSC-X350"
	"Sanyo VPC-C5 (PTP mode)"
	"Sanyo VPC-G200"
	"Sanyo VPC-G200EX"
	"Sanyo VPC-G210"
	"Sanyo VPC-G250"
	"ScanHex SX-35a" (TESTING)
	"ScanHex SX-35b" (TESTING)
	"ScanHex SX-35c" (TESTING)
	"ScanHex SX-35d" (TESTING)
	"Scott APX 30"
	"Sea &amp; Sea 5000G"
	"Sea &amp; Sea 5000G (PTP mode)"
	"Shark 2-in-1 Mini" (EXPERIMENTAL)
	"Shark SDC-513" (EXPERIMENTAL)
	"Shark SDC-519" (EXPERIMENTAL)
	"Sierra Imaging SD640"
	"SiPix Blink 2" (EXPERIMENTAL)
	"SiPix CAMeleon" (EXPERIMENTAL)
	"SiPix SC2100" (EXPERIMENTAL)
	"SiPix Snap" (EXPERIMENTAL)
	"SiPix Stylecam" (TESTING)
	"SiPix Web2" (EXPERIMENTAL)
	"Sirius Stiletto"
	"Sirius Stiletto 2"
	"Skanhex SX-330z" (EXPERIMENTAL)
	"SMaL Ultra-Pocket" (EXPERIMENTAL)
	"So. Show 301" (TESTING)
	"Sony DCR-PC100"
	"Sony DSC-F1" (EXPERIMENTAL)
	"Sony DSC-F55"
	"Sony DSC-F707V (PTP mode)"
	"Sony DSC-F717 (PTP mode)"
	"Sony DSC-F828 (PTP mode)"
	"Sony DSC-H1 (PTP mode)"
	"Sony DSC-H2 (PTP mode)"
	"Sony DSC-H5 (PTP mode)"
	"Sony DSC-N2 (PTP mode)"
	"Sony DSC-P10 (PTP mode)"
	"Sony DSC-P100 (PTP mode)"
	"Sony DSC-P120 (PTP mode)"
	"Sony DSC-P200 (PTP mode)"
	"Sony DSC-P30 (PTP mode)"
	"Sony DSC-P31 (PTP mode)"
	"Sony DSC-P32 (PTP mode)"
	"Sony DSC-P41 (PTP mode)"
	"Sony DSC-P43 (PTP mode)"
	"Sony DSC-P5 (PTP mode)"
	"Sony DSC-P50 (PTP mode)"
	"Sony DSC-P51 (PTP mode)"
	"Sony DSC-P52 (PTP mode)"
	"Sony DSC-P71 (PTP mode)"
	"Sony DSC-P72 (PTP mode)"
	"Sony DSC-P73 (PTP mode)"
	"Sony DSC-P92 (PTP mode)"
	"Sony DSC-P93 (PTP mode)"
	"Sony DSC-R1 (PTP mode)"
	"Sony DSC-S40 (PTP mode)"
	"Sony DSC-S60 (PTP mode)"
	"Sony DSC-S75 (PTP mode)"
	"Sony DSC-S85 (PTP mode)"
	"Sony DSC-T1 (PTP mode)"
	"Sony DSC-T10 (PTP mode)"
	"Sony DSC-T3 (PTP mode)"
	"Sony DSC-U10 (PTP mode)"
	"Sony DSC-U20 (PTP mode)"
	"Sony DSC-V1 (PTP mode)"
	"Sony DSC-W1 (PTP mode)"
	"Sony DSC-W12 (PTP mode)"
	"Sony DSC-W130 (PTP mode)"
	"Sony DSC-W200 (PTP mode)"
	"Sony DSC-W35 (PTP mode)"
	"Sony DSC-W55 (PTP mode)"
	"Sony MSAC-SR1"
	"Sony MVC-CD300 (PTP mode)"
	"Sony MVC-CD500 (PTP mode)"
	"Sony PTP"
	"Sony TRV-20E"
	"Sony Walkman NWZ-A728B"
	"Sony Walkman NWZ-A815/NWZ-A818"
	"Sony Walkman NWZ-A828/NWZ-A829"
	"Sony Walkman NWZ-S516"
	"Sony Walkman NWZ-S615F/NWZ-S616F/NWZ-S618F"
	"Sony Walkman NWZ-S716F"
	"SonyEricsson K850i"
	"SonyEricsson W890i"
	"SonyEricsson W910"
	"Soundstar TDC-35" (EXPERIMENTAL)
	"SpyPen Axys"
	"SpyPen Cleo"
	"SpyPen Luxo"
	"SpyPen Memo"
	"SpyPen Xion"
	"SQ chip camera" (EXPERIMENTAL)
	"StarCam CP086"
	"STM USB Dual-mode camera"
	"STV0680"
	"Suprema Digital Keychain Camera" (EXPERIMENTAL)
	"SY-2107C" (EXPERIMENTAL)
	"Tevion MD 81488"
	"Thomson / RCA Lyra HC308A"
	"Thomson / RCA Opal / Lyra MC4002"
	"Thomson EM28 Series"
	"Thomson RCA H106"
	"Thomson scenium E308"
	"Tiger Fast Flicks"
	"Timlex CP075"
	"Topfield TF5000PVR" (EXPERIMENTAL)
	"Toshiba Gigabeat"
	"Toshiba Gigabeat MEGF-40"
	"Toshiba Gigabeat MEU202"
	"Toshiba Gigabeat P10"
	"Toshiba Gigabeat P20"
	"Toshiba Gigabeat S"
	"Toshiba Gigabeat T"
	"Toshiba Gigabeat U"
	"Toshiba Gigabeat V30"
	"Toshiba PDR-M1"
	"Toshiba PDR-M11" (TESTING)
	"Toshiba PDR-M60"
	"Toshiba PDR-M61"
	"Toshiba PDR-M65"
	"Traveler SX330z" (EXPERIMENTAL)
	"Traveler SX410z" (EXPERIMENTAL)
	"TrekStor i.Beat Sweez FM"
	"TrekStor Vibez 8/12GB"
	"Trust DC-3500"
	"Trust Familycam 300" (TESTING)
	"Trust PowerC@m 350FS" (TESTING)
	"Trust PowerC@m 350FT" (TESTING)
	"Trust Spyc@m 100" (TESTING)
	"Trust Spyc@m 500F FLASH" (TESTING)
	"Typhoon StyloCam" (TESTING)
	"UMAX AstraPen"
	"UMAX AstraPix 320s" (TESTING)
	"USB PTP Class Camera" (TESTING)
	"ViviCam3350" (EXPERIMENTAL)
	"ViviCam5B" (EXPERIMENTAL)
	"Vivitar Mini Digital Camera" (TESTING)
	"Vivitar Vivicam 55" (EXPERIMENTAL)
	"Vivitar Vivicam3350B" (EXPERIMENTAL)
	"Vivitar Vivicam35" (EXPERIMENTAL)
	"Wild Planet Digital Spy Camera 70137" (EXPERIMENTAL)
	"WWF"
	"Yahoo!Cam" (EXPERIMENTAL)
	"ZINA Mini Digital Keychain Camera" (EXPERIMENTAL)
========================================================================
</pre>
</td></tr>
</table>
<?
	printFooter ();
?>
