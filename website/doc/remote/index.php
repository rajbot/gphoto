<?
	require ("../../include.php");

	printHeader ("Doc :: Remote controlling cameras", "", 1);

	printMenu ("doc");
?>

<h1>Remote controlling cameras</h1>
<hr />
<p>
This page lists cameras remotely controllable for capture.<br/>
It is meant to be wiki-like, so if
you have any additions, please mail them to <a
href="mailto:gphoto-devel@lists.sourceforge.net">gphoto-devel@lists.sourceforge.net</a>
mailinglist or to <a href="mailto:marcus@jet.franken.de">marcus@jet.franken.de</a>.
<br/>
The list is incomplete and might be incorrect.
</p>
<p>
If you start doing remote capturing, please begin by installing the
latest libgphoto2 and gphoto2 stable releases, as remote control improvements and fixes are
being added continously.
</p>
<hr />
<h2>Configuring for capture</h2>
<p>
When doing remote capture you likely want to modify several on camera parameters.
</p>
<p>
The gphoto2 commandline frontend offers both a ncurses GUI mode (using <code>--config</code>)
or scriptable commandline options to do configuration:
</p>
<ul>
<li><code>--list-config</code> - This will list all possible configuration options.<br>Please note that for some Canon cameras
the complete list will only show after <code>gphoto2 --set-config capture=on</code> is run.</li>
<li><code>--get-config <b>name</b></code> - This will get the current configuration of <b>name</b> and its possible values.</li>
<li><code>--set-config <b>name</b>=<b>value</b></code> - This will set the configuration of <b>name</b> to <b>value</b>.</li>
</ul>
<p>
Most of them are self explaining, but some interesting ones:
</p>
<ul>
<li>Canon cameras only: <b>capture</b> - setting this to <b>on</b> will extract the lens and make it read for SDRAM based capture.
	Setting it <b>off</b> will retract the lens again.
</li>
<li>Canon and some Nikon cameras only: <b>capturetarget</b> - setting this to <b>sdram</b> will make the camera capture directly into
the camera RAM and not on the memory card. You need to download the image in the same gphoto2 call, otherwise it will gone
when the connection is closed. Use <code>--capture-image-and-download</code> to capture and download instantely.<br/>
Set it to <b>card</b> to capture to the memory card.
</li>
</ul>


<h2>Continuous / Interval capture</h2>
<p>
The options <code>-F <i>frames</i></code> and <code>-I <i>seconds</i></code> can be used to support continuous
capture. <code>-F 0</code> will capture images ad-infinitum.
</p><p>
This can be used with either <code>--capture-image</code> which would leave all images on the card, or <code>--capture-image-and-download</code> which captures and downloads the images immediately.
</p>

<h2>Movie Capture</h2>
<p>
Capturing movies with sound is currently possible with:
<ul>
<li>Newer Nikon DSLRs: <code>gphoto2 --set-config movie=1 --wait-event=10s --set-config movie=0 --wait-event-and-download=2s</code>. Replace 10s by the number of seconds you want to have your movie long. This started being supported around 2012, around the D7000 release.
<li>Newer Canon EOS DSLRs (around 7D and later) : Switch the camera to movie record mode on the camera. Then run <code>gphoto2 --set-config movierecord=4 --wait-event=10s --set-config movierecord=0 --wait-event-and-download=2s</code>
<li>Older cameras with preview capture ability: <code>gphoto2 --capture-movie=10s</code> . This will capture 10 seconds of preview frames and concatenate them in a MotionJPEG style stream.
</ul>
</p>

<h2>Bulb Capture</h2>
<p>
Bulb capture is a bit difficult in a command-response style setting. Various cameras handle it differently.
<ul>
<li>Nikon DSLR: We do not know how to do Bulb capture yet.</li>
<li>Older Canon EOS DSLR:
<p>
<code>gphoto2 --set-config shutterspeed=bulb</code><br/>
<code>gphoto2 --set-config bulb=1 --wait-event=30s --set-config bulb=0 --wait-event-and-download=2s</code>
</p>
</li>
<li>Newer Canon EOS DSLR:
<p>
<code>gphoto2 --set-config shutterspeed=bulb</code><br/>
<code>gphoto2 --set-config eosremoterelease=Immediate --wait-event=30s --set-config eosremoterelease=Off --wait-event-and-download=2s</code>
</p><p>
The <code>eosremoterelease</code> refers to the direct shutter button manipulation, so it can be used for different scenarios
outside of immediate shutter releases.
</p>
</li>
</ul>
</p>

<h2>Manual Focus</h2>
<p>
<ul>
<li>Nikon DSLR: Manual focusing only works in the "liveview" aka "preview" mode (with mirror up). In "non preview" mode the focus motor is only controllable by the autofocus engine. 
<code>--set-config manualfocusdrive=<b>step size</b> </code><br/>Here a value between <code>-32768</code> and <code>32767</code> can be specified that is the direction and pulse length to the focus ring motor.<br/>Setting this value causes a relative movement, fixed positions cannot be driven to. To achieve the wanted focus, multiple calls might need to be done.
<br/>
An error will be reported if the end of the focus range is reached.
</li>
<li>Canon EOS DSLR: Manual focus driving only works on the "liveview" aka "preview" mode (with mirror up). In "non preview" mode the focus motor is only controllable by the autofocus
engine.
<code>--set-config manualfocusdrive=<b>Mode</b></code> where mode is "Near 1" "Near 2" "Near 3" "Far 1" "Far 2" "Far 3". These are 3 different relative stepsizes for both focusing directions. To achieve focusing, multiple calls might need to be done.
</li>
</ul>
</p>
<h2>List of cameras</h2>
<hr />
<table border="1">
<tr>
	<th>Camera Name</th>
	<th>Libgphoto2 capture support</th>
	<th>Controllable aspects</th>
	<th>Megapixel</th>
	<th>Notes</th>
</tr>

<tr>
	<td>Canon Digital IXUS II/PowerShot SD100</td>
	<td>Yes</td>
	<td>Quality, Imagesize, ISO, Whitebalance, Photoeffect, Zoom, Assistlight, ExpComp, Flashmode, Aperture, Focuspoints, Shutterspeed, Metering Mode, AF Distance, Focus Locking, Viewfinder</td>
	<td>3</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot SD110</td>
	<td>Yes</td>
	<td>All (like SD100),Viewfinder</td>
	<td>3</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon Digital IXUS 30</td>
	<td>Yes</td>
	<td>All,Viewfinder</td>
	<td>3.2</td>
	<td>&nbsp;</td>
</tr>
<tr>
	<td>Canon Digital IXUS 300</td>
	<td>Yes</td>
	<td>All,Viewfinder</td>
	<td>2.1</td>
	<td>&nbsp;</td>
</tr>
<tr>
	<td>Canon Digital IXUS 330</td>
	<td>Yes</td>
	<td>All,Viewfinder</td>
	<td>2</td>
	<td>&nbsp;</td>
</tr>
<tr>
	<td>Canon Digital IXUS 400</td>
	<td>Yes</td>
	<td>All,Viewfinder</td>
	<td>4</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon Digital IXUS 430 / ELPH S410</td>
	<td>Yes</td>
	<td>?,Viewfinder</td>
	<td>4</td>
	<td>In PTP mode</td>
</tr>

<tr>
	<td>Canon IXY Digital 300</td>
	<td>Yes</td>
	<td>All,Viewfinder</td>
	<td>2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon Digital IXUS 500</td>
	<td>Yes</td>
	<td>All,Viewfinder</td>
	<td>5</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon EOS 1D Mark III</td>
	<td>Yes</td>
	<td>All</td>
	<td>10.1</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon EOS 1D Mark IV</td>
	<td>Yes</td>
	<td>All, LiveView</td>
	<td>16.1</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon EOS 1D X</td>
	<td>Yes</td>
	<td>All, LiveView</td>
	<td>18.1</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon EOS 1000D / Rebel XS / Kiss F</td>
	<td>Yes</td>
	<td>Image Format, ISO, WhiteBalance, Whitebalance Adjust, DriveMode, Picture Style, Bulb Mode, BracketMode, Aperture, ShutterSpeed, Autofocus (in LiveView mode), Manual Focus (in LiveView mode), Viewfinder</td>
	<td>10</td>
	<td>Use libgphoto2 2.4.9 or newer<br>
	Use the Modewheel on the Camera to get to different
	settings.<br>
	Shutterspeed and Aperture not available in Auto or P
	setting, only in the more manual ones.<br>
	For Bulb mode: Switch dial to 'M'anual mode, gphoto2 --set-config shutterspeed=bulb , and run something like:

	gphoto2 --set-config bulb=1 --wait-event=10s --set-config bulb=0 --wait-event-and-download=5s

	</td>
</tr>

<tr>
	<td>Canon EOS 1100D / Rebel ?? / Kiss ?</td>
	<td>Yes</td>
	<td>Image Format, ISO, WhiteBalance, Whitebalance Adjust, DriveMode, Picture Style, Bulb Mode, BracketMode, Aperture, ShutterSpeed, Autofocus (in LiveView mode), Manual Focus (in LiveView mode), Viewfinder</td>
	<td>10</td>
	<td>Use libgphoto2 2.4.14 or newer<br>
	Use the Modewheel on the Camera to get to different settings.<br>
	Shutterspeed and Aperture not available in Auto or P
	setting, only in the more manual ones.<br>
	For Bulb mode: Switch dial to 'M'anual mode, gphoto2 --set-config shutterspeed=bulb , and run something like:

gphoto2 --wait-event=2s --set-config eosremoterelease=Immediate --wait-event=5s --set-config eosremoterelease=Off --wait-event-and-download=5s

	</td>
</tr>


<tr>
	<td>Canon EOS 300D/Digital Rebel</td>
	<td>Yes</td>
	<td>ISO, Shutterspeed, Zoom (? likely read only), Aperture, Resolution (RAW, Normal JPEG, ...), Focus Mode (read-only?), Flash Mode (read-only)</td>
	<td>6.5</td>
	<td>Uses "Normal" mode in the camera (and the "canon" driver in libgphoto2). Set Modewheel to "M" to get all settings.</td>
</tr>

<tr>
	<td>Canon EOS 40D</td>
	<td>Yes</td>
	<td>limited level of configurability (see EOS 1000D), Viewfinder</td>
	<td>10.1</td>
	<td>use libgphoto2 2.4.9 or newer</td>
</tr>

<tr>
	<td>Canon EOS 400D / Rebel XTi / Kiss Digital X</td>
	<td>Yes</td>
	<td>limited level of configurability (see EOS 1000D), no Viewfinder</td>
	<td>10.1</td>
	<td>use libgphoto2 2.4.9</td>
</tr>

<tr>
	<td>Canon EOS 450D / Rebel XSi / Kiss X2</td>
	<td>Yes</td>
	<td>Image Format, ISO, WhiteBalance, DriveMode, Picture Style, Aperture, Shutterspeed, Viewfinder</td>
	<td>12</td>
	<td>Use libgphoto2 2.4.9.
	Use the ModeWheel on the Camera to get to different
	settings. Shutterspeed and Aperture not available in Auto or P
	setting, only in the more manual ones.</td>
</tr>

<tr>
	<td>Canon EOS 50D</td>
	<td>Yes</td>
	<td>limited level of configurability (see other EOS), Viewfinder</td>
	<td>15.1</td>
	<td>Use libgphoto2 2.4.9.
	Use the ModeWheel on the Camera to get to different
	settings. Shutterspeed and Aperture not available in Auto or P
	setting, only in the more manual ones.</td>
</tr>

<tr>
	<td>Canon EOS 500D / Rebel T1i / Kiss X3</td>
	<td>Yes</td>
	<td>limited level of configurability (see other EOS), Viewfinder</td>
	<td>15.1</td>
	<td>Use libgphoto2 2.4.9.
	Use the ModeWheel on the Camera to get to different
	settings. Shutterspeed and Aperture not available in Auto or P
	setting, only in the more manual ones.</td>
</tr>

<tr>
	<td>Canon EOS 550D / Rebel T2i / Kiss X4</td>
	<td>Yes</td>
	<td>limited level of configurability (see other EOS), Viewfinder</td>
	<td>18.0</td>
	<td>Use libgphoto2 2.4.9.
	Use the ModeWheel on the Camera to get to different
	settings.
	Shutterspeed and Aperture not available in Auto or P
        setting, only in the more manual ones.
	</td>
</tr>

<tr>
	<td>Canon EOS 60D</td>
	<td>Yes</td>
	<td>limited level of configurability (see other EOS), Viewfinder, Bulb</td>
	<td>18</td>
	<td>Use libgphoto2 2.4.11 or newer.</td>
</tr>

<tr>
	<td>Canon EOS 600D</td>
	<td>Yes</td>
	<td>limited level of configurability (see other EOS), Viewfinder</td>
	<td>18.0</td>
	<td>Use libgphoto2 2.4.14 or newer
	</td>
</tr>

<tr>
	<td>Canon EOS 650D</td>
	<td>Yes</td>
	<td>limited level of configurability (see other EOS), Viewfinder</td>
	<td>18.0</td>
	<td>Use libgphoto2 2.4.14 or newer
	</td>
</tr>

<tr>
	<td>Canon EOS 5D Mark II</td>
	<td>Yes</td>
	<td>see other EOS like 1000D, Viewfinder</td>
	<td>10.1</td>
	<td>use libgphoto2 2.4.9 or newer</td>
</tr>

<tr>
	<td>Canon EOS 5D Mark III</td>
	<td>Yes</td>
	<td>see other EOS like 1000D, Viewfinder</td>
	<td>22</td>
	<td>use libgphoto2 2.4.14 or newer</td>
</tr>

<tr>
	<td>Canon EOS 6D</td>
	<td>Yes</td>
	<td>see other EOS like 1000D, Viewfinder</td>
	<td>20</td>
	<td>use libgphoto2 2.4.14 or newer</td>
</tr>

<tr>
	<td>Canon EOS 7D</td>
	<td>Yes</td>
	<td>ImageFormat, ISO, WhiteBalance, WhiteBalanceAdjust, DriveMode, PictureStyle, Aperture, Shutterspeed, MeteringMode, BracketMode, AutoExposure Bracketing, Viewfinder</td>
	<td>18</td>
	<td>Use libgphoto2 2.4.9. For Bulb capture turn rotary dial to 'B'.</td>
</tr>

<tr>
	<td>Canon EOS 70D</td>
	<td>Yes</td>
	<td>ImageFormat, ISO, WhiteBalance, WhiteBalanceAdjust, DriveMode, PictureStyle, Aperture, Shutterspeed, MeteringMode, BracketMode, AutoExposure Bracketing, Viewfinder</td>
	<td>20.2</td>
	<td>&nbsp;</td>
</tr>


<tr>
	<td>Canon PowerShot A10</td>
	<td>Yes</td>
	<td>unknown, Viewfinder</td>
	<td>1.3</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A100</td>
	<td>Yes</td>
	<td>unknown, Viewfinder</td>
	<td>1.2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A20</td>
	<td>Yes</td>
	<td>unknown, Viewfinder</td>
	<td>2.1</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A200</td>
	<td>Yes</td>
	<td>unknown, Viewfinder</td>
	<td>2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A30</td>
	<td>Yes</td>
	<td>unknown, Viewfinder</td>
	<td>1.32</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A300</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>3.2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A310</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>3.2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A40</td>
	<td>Yes</td>
	<td>unknown, Viewfinder</td>
	<td>2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A400</td>
	<td>Yes</td>
	<td>ImageQuality, ImageSize, FlashMode, ShootingMode, DriveMode, Zoom, MeteringMode, AF Distance, FocusingPoint, WhiteBalance, ISO, Aperture, ShutterSpeed, ExpComp, PhotoEffect, AssistLight, Focus Locking, Viewfinder</td>
	<td>3.2</td>
	<td>From reporter. Only captures with PTP driver.</td>
</tr>

<tr>
	<td>Canon PowerShot A510</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>3.2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A520</td>
	<td>Yes</td>
	<td>ViewFinder, FocusLock, SyncDateTime, DateTime, Output, Ownername, CaptureTarget,
Capture, Model, FirmwareRevision, Orientation, ImageQuality,
ImageFormat, ImageSize, ISO, WhiteBalance, PhotoEffect, Zoom,
AssistLight, AutoRotation, ExposureCompensation, Flashmode,
ShootingMode, Aperture, FocusingPoint, ShutterSpeed, MeteringMode,
AFDistance</td>
	<td>4</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A60</td>
	<td>Yes</td>
	<td>Unknown, Viewfinder</td>
	<td>2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A620</td>
	<td>Yes</td>
	<td>Unknown, Viewfinder</td>
	<td>7.1</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A640</td>
	<td>Yes</td>
	<td>ImageQuality, ImageSize, ISO, WhiteBalance, AssistLight, ExpComp, FlashMode, ShootingMode, Aperture, Shutterspeed, FocusingPoint, MeteringMode, AF Distance, Focus Locking, Viewfinder</td>
	<td>10</td>
	<td>One reference user: <a href="http://www.oldcapebridge.com">oldcapebridge.com</a></td>
</tr>

<tr>
	<td>Canon PowerShot A70</td>
	<td>Yes</td>
	<td>ImageQuality, ImageSize, FlashMode, ShootingMode, DriveMode, Zoom, MeteringMode, AF Distance, Focusing Point, WhiteBalance, ISO, Aperture, Shutterspeed, PhotoEffect, Focus Locking, Viewfinder</td>
	<td>3</td>
	<td>(from ptpcanon list)</td>
</tr>

<tr>
	<td>Canon PowerShot A75</td>
	<td>Yes</td>
	<td>Unknown, Viewfinder</td>
	<td>3</td>
	<td>from capture.sf.net notes</td>
</tr>

<tr>
	<td>Canon PowerShot A80</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>4</td>
	<td>capture.sf.net and user reported</td>
</tr>

<tr>
	<td>Canon PowerShot A85</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>4</td>
	<td></td>
</tr>

<tr>
	<td>Canon PowerShot A95</td>
	<td>Yes</td>
	<td>ImageQuality, ImageSize, FlashMode, ShootingMode, DriveMode, Zoom, MeteringMode, AF Distance, Focusing Point, WhiteBalance, ISO, Aperture, Shutterspeed, ExpComp, Focus Locking, Viewfinder</td>
	<td>5</td>
	<td>from capture.sf.net notes</td>
</tr>

<tr>
	<td>Canon PowerShot A520</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>4</td>
	<td>from capture.sf.net notes</td>
</tr>

<tr>
	<td>Canon PowerShot A620</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>7</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot A640</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>10</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot G1</td>
	<td>Yes</td>
	<td>Unknown</td>
	<td>3.3</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot G2</td>
	<td>Yes</td>
	<td>Unknown</td>
	<td>4</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot G3</td>
	<td>Yes</td>
	<td>Unknown, Viewfinder</td>
	<td>4</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot G5</td>
	<td>Yes</td>
	<td>ImageQuality, ImageSize, ISO, WhiteBalance, PhotoEffect, Zoom,AssistLight, ExpComp, Aperture, FocusingPoint, ShutterSpeed, MeteringMode, AF Distance, Viewfinder</td>
	<td>5</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot G6</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>7.1</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot G7</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>10</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot G9</td>
	<td>Yes</td>
	<td>ImageSize, ISO, WhiteBalance, Zoom, AssistLight, ExpComp, FlashComp, FlashMode, ShootingMode, Aperture, FocusingPoint, ShutterSpeed, MeteringMode, AF Distance, Viewfinder</td>
	<td>12</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot G10</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>14.6</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot Pro 90 IS</td>
	<td>Yes</td>
	<td>Unknown, Viewfinder</td>
	<td>2.6</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S1 IS</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>3</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S2 IS</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>5</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S3 IS</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>6</td>
	<td>confirmed by Canon</td>
</tr>

<tr>
	<td>Canon PowerShot S5 IS</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>8</td>
	<td>confirmed by user with 2.4.0</td>
</tr>

<tr>
	<td>Canon PowerShot S40</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>4</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S30</td>
	<td>Yes</td>
	<td>Unknown, Viewfinder</td>
	<td>3.2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S40</td>
	<td>Yes</td>
	<td>Unknown, Viewfinder</td>
	<td>4</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S45</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>4</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S50</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>5</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S60</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>5</td>
	<td>See <a href="http://www.escursionisticivatesi.it/webcam/">sample installation</a></td>
</tr>

<tr>
	<td>Canon PowerShot S70</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>7</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S80</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>8</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S100</td>
	<td>Yes</td>
	<td>Unknown, Viewfinder</td>
	<td>2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S110</td>
	<td>Yes</td>
	<td>Unknown, Viewfinder</td>
	<td>2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S200</td>
	<td>Yes</td>
	<td>Unknown, Viewfinder</td>
	<td>2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S230</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>3</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S300</td>
	<td>Yes</td>
	<td>Unknown, Viewfinder</td>
	<td>2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot S400</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>4</td>
	<td>from capture.sf.net notes</td>
</tr>

<tr>
	<td>Canon PowerShot S410</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>4</td>
	<td>from capture.sf.net notes</td>
</tr>

<tr>
	<td>Canon PowerShot S500</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>5</td>
	<td>from capture.sf.net notes</td>
</tr>

<tr>
	<td>Canon PowerShot SD110</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>3</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon PowerShot SD430</td>
	<td>Yes, but only over WLAN</td>
	<td>All, Viewfinder</td>
	<td>5</td>
	<td>Unfortunately the WLAN (PTP/IP) mode to connect to this camera is not working yet.</td>
</tr>

<tr>
	<td>Canon PowerShot SX100 IS</td>
	<td>Yes</td>
	<td>Zoom, ISO, Quality, ImageSize, WhiteBalance, ExposureCompensation, FlashCompensation, CaptureMode, Aperture, Shutterspeed, MeteringMode, AF Distance,, Focus Locking, Viewfinder</td>
	<td>8</td>
	<td>1 frame capture and download - 2-3 seconds</td>
</tr>

<tr>
	<td>Canon PowerShot SX110 IS</td>
	<td>Yes</td>
	<td>All (likely same as SX100IS above), Viewfinder</td>
	<td>9</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Canon 20D, 350D/Digital Rebel XT</td>
	<td>Yes</td>
	<td>All</td>
	<td>8.2</td>
	<td>Needs 2.4.0 or newer</td>
</tr>

<tr>
	<td>Canon 5D</td>
	<td>Yes</td>
	<td>All</td>
	<td>12</td>
	<td>Needs 2.4.0 or newer</td>
</tr>

<tr>
	<td>Casio EX-F1</td>
	<td>Tethered</td>
	<td>Unknown</td>
	<td>6</td>
	<td>Only tethered shooting, <a href="http://www.dpreview.com/news/0906/09061801casioexf1software.asp?from=rss">Firmware 2.0 required</a>.</td>
</tr>

<tr>
	<td>Kodak DC280</td>
	<td>Yes</td>
	<td>None</td>
	<td>2</td>
	<td>None</td>
</tr>

<tr>
	<td>Nikon CoolPix 880</td>
	<td>Yes</td>
	<td>All</td>
	<td>3</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix 2500</td>
	<td>Yes</td>
	<td>All</td>
	<td>2</td>
	<td>In Sierra mode (USB Mass storage pass through)</td>
</tr>

<tr>
	<td>Nikon CoolPix 4300</td>
	<td>Yes</td>
	<td>All</td>
	<td>4</td>
	<td>In Sierra mode (USB Mass storage pass through)</td>
</tr>

<tr>
	<td>Nikon CoolPix 4500</td>
	<td>Yes</td>
	<td>All(?)</td>
	<td>4</td>
	<td>Only with updated firmware, see <a href="http://support.nikontech.com/cgi-bin/nikonusa.cfg/php/enduser/std_alp.php?p_prods=1%2C3&amp;p_pv=2.3&amp;p_cats=186&amp;p_cv=1.186">
here</a></td>
</tr>

<tr>
	<td>Nikon CoolPix 5000</td>
	<td>Yes</td>
	<td>All</td>
	<td>5</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix 5400</td>
	<td>Yes</td>
	<td>Unknown</td>
	<td>5</td>
	<td>&nbsp;</td>
</tr>
<tr>
	<td>Nikon CoolPix 5600</td>
	<td>Yes</td>
	<td>All,no focus, no aperture</td>
	<td>5</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix 5700</td>
	<td>Yes</td>
	<td>All</td>
	<td>5</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix 5900</td>
	<td>Yes</td>
	<td>All</td>
	<td>5</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix AW100</td>
	<td>Yes</td>
	<td>Unknown</td>
	<td>16</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix L12</td>
	<td>Yes</td>
	<td>Unknown</td>
	<td>7</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix L16</td>
	<td>Yes</td>
	<td>Unknown</td>
	<td>7</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix L19</td>
	<td>Yes</td>
	<td>Unknown</td>
	<td>8</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix L110</td>
	<td>Yes</td>
	<td>Unknown</td>
	<td>12</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix L120</td>
	<td>Yes</td>
	<td>Unknown</td>
	<td>14</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix L820</td>
	<td>Yes</td>
	<td>Unknown</td>
	<td>16</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon CoolPix P1</td>
	<td>Yes</td>
	<td>Some</td>
	<td>8</td>
	<td>Only over USB, not over PTP/IP.</td>
</tr>

<tr>
	<td>Nikon CoolPix P2</td>
	<td>Yes</td>
	<td>Some</td>
	<td>5</td>
	<td>Only over USB, not over PTP/IP.</td>
</tr>

<tr>
	<td>Nikon CoolPix P3</td>
	<td>Yes</td>
	<td>Some</td>
	<td>8</td>
	<td>Only over USB, not over PTP/IP.</td>
</tr>

<tr>
	<td>Nikon CoolPix P50</td>
	<td>Yes</td>
	<td>None</td>
	<td>8</td>
	<td>not configurable, just trigger capture</td>
</tr>

<tr>
	<td>Nikon CoolPix P60</td>
	<td>Yes</td>
	<td>None</td>
	<td>8</td>
	<td>not configurable, just trigger capture</td>
</tr>

<tr>
	<td>Nikon CoolPix P80</td>
	<td>Yes</td>
	<td>None</td>
	<td>10</td>
	<td>not configurable, just trigger capture</td>
</tr>

<tr>
	<td>Nikon CoolPix P100</td>
	<td>Yes</td>
	<td>None</td>
	<td>10</td>
	<td>only image quality configurable</td>
</tr>

<tr>
	<td>Nikon CoolPix P5000</td>
	<td>Yes</td>
	<td>Some</td>
	<td>10</td>
	<td>only image quality configurable</td>
</tr>

<tr>
	<td>Nikon CoolPix S3300</td>
	<td>Yes</td>
	<td>Some</td>
	<td>16</td>
	<td>Only image quality/size, flash, focusmode configurable</td>
</tr>


<tr>
	<td>Nikon D2x</td>
	<td>Yes</td>
	<td>All</td>
	<td>12.5</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D3</td>
	<td>Yes</td>
	<td>All</td>
	<td>12.1</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D3s</td>
	<td>Yes</td>
	<td>All</td>
	<td>12.1</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D4</td>
	<td>Yes</td>
	<td>All</td>
	<td>16</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D40</td>
	<td>Yes</td>
	<td>All</td>
	<td>6</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D40x</td>
	<td>Yes</td>
	<td>All</td>
	<td>10</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D50</td>
	<td>Yes</td>
	<td>All</td>
	<td>6</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D60</td>
	<td>Yes</td>
	<td>All</td>
	<td>10</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D70</td>
	<td>Yes</td>
	<td>All</td>
	<td>6</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D70s</td>
	<td>Yes</td>
	<td>All</td>
	<td>6</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D80</td>
	<td>Yes</td>
	<td>All</td>
	<td>10</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D90</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>12</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D200</td>
	<td>Yes</td>
	<td>All</td>
	<td>10</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D300</td>
	<td>Yes</td>
	<td>All</td>
	<td>12</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D300s</td>
	<td>Yes</td>
	<td>All</td>
	<td>12</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D3000</td>
	<td>Yes</td>
	<td>Basic set of abilities (no SDRAM, no Viewfinder, just the basic capture settings)</td>
	<td>10</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D3100</td>
	<td>Yes</td>
	<td>Basic set of abilities (no SDRAM, no Viewfinder, just the basic capture settings)</td>
	<td>14.2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D5000</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>12</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D5100</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>16</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D5200</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>24.2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D600</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>24</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D700</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>12</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D7000</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>?</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D7100</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>24.1</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D800</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>36</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Nikon D800E</td>
	<td>Yes</td>
	<td>All, Viewfinder</td>
	<td>36</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Olympus C750UZ</td>
	<td>Yes</td>
	<td>All</td>
	<td>?</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Olympus C-2040Z</td>
	<td>Yes</td>
	<td>All</td>
	<td>2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Olympus C-2100Z</td>
	<td>Yes</td>
	<td>All</td>
	<td>2</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Olympus C-3040Z</td>
	<td>Yes</td>
	<td>All</td>
	<td>3</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Olympus C-4040Z</td>
	<td>Yes</td>
	<td>All</td>
	<td>4</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>Sony SLT-A58</td>
	<td>Yes</td>
	<td>Some</td>
	<td>20</td>
	<td>use libgphoto2 2.5.4 or newer</td>
</tr>

<tr>
	<td>
	SQ905 chipset cameras (check <a href="http://gphoto.svn.sourceforge.net/viewcvs.cgi/gphoto/trunk/libgphoto2/camlibs/sq905/library.c?view=markup">list in C file here</a>)
	</td>
	<td>Yes</td>
	<td>?</td>
	<td>320x240 pixel, 160x120 pixel, some also 640x480 pixel</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>
	Digigr8 chipset cameras (check <a href="http://gphoto.svn.sourceforge.net/viewcvs.cgi/gphoto/trunk/libgphoto2/camlibs/digigr8/library.c?view=markup">list in C file here</a>)
	</td>
	<td>Yes</td>
	<td>?</td>
	<td>320x240 pixel</td>
	<td>&nbsp;</td>
</tr>

<tr>
	<td>
	STV680 chipset cameras (check <a href="http://gphoto.svn.sourceforge.net/viewcvs.cgi/gphoto/trunk/libgphoto2/camlibs/stv0680/stv0680.c?view=markup">list in C file here</a>)
	</td>
	<td>Yes</td>
	<td>None</td>
	<td>320x240, 160x120, sometimes also 640x480 pixel</td>
	<td>&nbsp;</td>
</tr>
</table>
<p>
If a camera is not listed, it might still be the case that is able to support capture.
</p>
<p>
For instance, it is a very good sign if the vendor supplies software that supports remote
capture for this camera. Those are likely able to be supported by libgphoto2 if not already.
</p>
<hr/>
<h2>Cameras not able to do capture</h2>
<table border="1">
<tr><th>Name</th><th>Comment</th></tr>
<tr>
	<td>Kodak EasyShare Any</td>
	<td>None of those supports remote capture.</td>
</tr>
<tr>
	<td>Sony Cybershot DSC (PTP)</td>
	<td>None of those supports remote capture.</td>
</tr>
<tr>
	<td>Fuji Finepix (PTP)</td>
	<td>Most of these cameras do not support remote capture.
        Some professional Fuji S* pro cameras might support it, please see
        <a href="http://www.fujifilm.com/support/digital_cameras/compatibility/utility/">the Utility tool</a> page.
        </td>
</tr>
<tr><td>Various Canon cameras:</td>
<td>
All Powershots released after mid 2009 are not capable of remote control anymore.
<p>
 Statement from Canon SDK: <i>As a reminder:
PowerShots A410, A420, A430, A450, A460, A470,
A530, A540, A550, A560, A570 IS, A580, A590 IS,
A610, A630, A650 IS,
 A700, A710 IS, A720 IS,
A1000 IS, A2000 IS,
S10, S20, S330,
SD10, SD20, SD30, SD40, SD200, SD300, SD400, SD430, SD450, SD500, SD550, SD600, SD630, SD700 IS, SD750, SD770 IS,SD790 IS,SD800 IS, SD850 IS, SD870 IS, SD880 IS, SD890 IS, SD900, SD950 IS, SD990 IS, SD 1000, SD1100 IS,
SX 1 IS, and SX 10IS
do not support remote control or video out operation via the SDK.</i><br/>
 The same applies for libgphoto2 capture support.<p>

Also see <a href="http://www.usa.canon.com/cusa/consumer/standard_display/sdk_homepage">this Canon page</a> for a overview of which cameras are supported and which are not as of April 1st 2008.
</td>
</tr>
<tr>
	<td>Minolta Dimage Z2</td>
	<td>Do not support remote control. See <a href="http://ca.konicaminolta.com/support/americas/digital_cameras/dimage-z/dimage-z2/faq/071.html">Konica-Minolta FAQ</a></td>
</tr>
</table>

<?
	printFooter ();
?>
