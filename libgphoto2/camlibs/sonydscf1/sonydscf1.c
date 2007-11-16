#include "config.h"
#include "common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gphoto2/gphoto2.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (GETTEXT_PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define _(String) (String)
#  define N_(String) (String)
#endif

#include "common.h"
#include "command.h"
#include "pmp.h"

#define MAX_PICTURE_NUM 200

static u_char picture_index[MAX_PICTURE_NUM];
static u_short picture_thumbnail_index[MAX_PICTURE_NUM];
static u_char picture_protect[MAX_PICTURE_NUM];
static u_char picture_rotate[MAX_PICTURE_NUM];

static  int     errflg = 0;

static int
make_jpeg_comment(u_char *buf, u_char *jpeg_comment)
{
  int i, cur = 0;
  int reso, shutter;

  struct resolution {
    int reso_val;
    char *reso_conv;
  } reso_tab[] = {
    {PMP_FIN, "fine"},
    {PMP_STD, "standard"},
    {PMP_ECM, "economy"},
    {0,       "unknown"},
  };

  struct sh_speed {
    int spd_val;
    char *spd_conv;
  } sh_speed_tab[] = {
    {0x0123, "1/7.5"},
    {0x0187, "1/15"},
    {0x01eb, "1/30"},
    {0x024f, "1/60"},
    {0x0298, "1/100"},
    {0x031d, "1/250"},
    {0x0381, "1/500"},
    {0x03e5, "1/1000"},
    {0,      "unknown"},
  };

  jpeg_comment[0] = 0xff;
  jpeg_comment[1] = 0xd8;
  jpeg_comment[2] = 0xff;
  jpeg_comment[3] = 0xfe;

  /* resolution */
  reso = *(buf+PMP_RESOLUTION);

  i = 0;
  while (1) {
    if ((reso == reso_tab[i].reso_val) || (reso_tab[i].reso_val == 0)) {
      cur = 6 + sprintf(&jpeg_comment[6], "Resolution: %s\n",
                          reso_tab[i].reso_conv);
      break;
    }
    i++;
  }

  /* shutter speed */
  shutter = (buf[PMP_SPEED]<<8)|buf[PMP_SPEED+1];

  i = 0;
  while (1) {
    if ((shutter == sh_speed_tab[i].spd_val) ||
        (sh_speed_tab[i].spd_val == 0)) {
      cur = cur + sprintf(&jpeg_comment[cur], "Shutter-speed: %s\n",
                          sh_speed_tab[i].spd_conv);
      break;
    }
    i++;
  }

  /* PMP comment */
  if (*(buf+PMP_COMMENT)) {
    cur = cur + sprintf(&jpeg_comment[cur], "Comment: %s\n",
                        (char *)(buf+PMP_COMMENT));
  }

  /* taken date */
  if (*(buf+PMP_TAKE_YEAR) == 0xff) {
    cur = cur + sprintf(&jpeg_comment[cur],
                        "Date-Taken: ----/--/-- --:--:--\n");
  }
  else {
    cur = cur + sprintf(&jpeg_comment[cur],
                        "Date-Taken: %d/%02d/%02d %02d:%02d:%02d\n",
    2000+(*(buf+PMP_TAKE_YEAR)), *(buf+PMP_TAKE_MONTH),
    *(buf+PMP_TAKE_DATE), *(buf+PMP_TAKE_HOUR), *(buf+PMP_TAKE_MINUTE),
    *(buf+PMP_TAKE_SECOND));
  }

  /* edited date */
  if (*(buf+PMP_EDIT_YEAR) == 0xff) {
    cur = cur + sprintf(&jpeg_comment[cur],
                        "Date-Edited: ----/--/-- --:--:--\n");
  }
  else {
    cur = cur + sprintf(&jpeg_comment[cur],
                        "Date-Edited: %d/%02d/%02d %02d:%02d:%02d\n",
    2000+(*(buf+PMP_EDIT_YEAR)), *(buf+PMP_EDIT_MONTH),
    *(buf+PMP_EDIT_DATE), *(buf+PMP_EDIT_HOUR), *(buf+PMP_EDIT_MINUTE),
    *(buf+PMP_EDIT_SECOND));
  }

  /* use flash? */
  if (*(buf+PMP_FLASH) != 0) {
    cur = cur + sprintf(&jpeg_comment[cur], "Flash: on\n");
  }

  /* insert total jpeg comment length */
  jpeg_comment[4] = (u_char)((cur - 4) >> 8);
  jpeg_comment[5] = (u_char)(cur - 4);

  return cur;
}

static int
get_picture_information(GPPort *port,int *pmx_num, int outit)
{
  u_char buforg[PMF_MAXSIZ];
  char name[64];
  long len;
  int i, n;
  int j, k;
  char *buf = (char *) &buforg;

  strcpy(name, "/PIC_CAM/PIC00000/PIC_INF.PMF");
  F1ok(port);
  len = F1getdata(port, name, buf, 0);

  n = buf[26] * 256 + buf[27]; /* how many files */
  *pmx_num = buf[31];  /* ??? */

  if(n ==10)
     buf++;

  k = 0;
  for(i = 0 ; i < (int) *pmx_num ; i++){
    for(j = 0 ; j < buforg[0x20 + 4 * i + 3]; j++){
      picture_thumbnail_index[k] = (j << 8) | buforg[0x20 + 4 * i] ;
      k++;
    }
  }
  for(i = 0 ; i < n ; i++){
    picture_index[i] = buf[0x420 + 0x10 * i + 3];
    picture_rotate[i] = buf[0x420 + 0x10 * i + 5];
    picture_protect[i] = buf[0x420 + 0x10 * i + 14];
  }

  if(outit == 2){
      fprintf(stdout," No:Internal name:Thumbnail name(Nth):Rotate:Protect\n");
    for(i = 0 ; i < n ; i++){
      fprintf(stdout,"%03d:", i + 1);
      fprintf(stdout," PSN%05d.PMP:", picture_index[i]);
      fprintf(stdout,"PIDX%03d.PMX(%02d)    :",
              0xff & picture_thumbnail_index[i],
              0xff & (picture_thumbnail_index[i] >> 8));
      switch(picture_rotate[i]){
      case 0x00:
        fprintf(stdout,"     0:");
        break;
      case 0x04:
        fprintf(stdout,"   270:");
        break;
      case 0x08:
        fprintf(stdout,"   180:");
        break;
      case 0x0c:
        fprintf(stdout,"    90:");
        break;
      default:
        fprintf(stdout,"   ???:");
        break;
      }
      if(picture_protect[i])
        fprintf(stdout,"on");
      else
        fprintf(stdout,"off");
      fprintf(stdout,"\n");
    }
  }
  return(n);
}

static long
get_file(GPPort *port,char *name, char **data, int format)
{
  u_long filelen;
  u_long total = 0;
  long len,memcpylen;
  char *ptr = NULL;
  u_char buf[0x400];
  u_char jpeg_comment[256];

  F1ok(port);
  F1status(port,0);

  filelen = F1finfo(port,name);
  if(filelen == 0)
    return(0);

  if(F1fopen(port,name) != 0)
    return(0);

  if(format != JPEG)
    return(0);

  len = F1fread(port, buf, 126);
  if( len < 126){
    F1fclose(port);
    return(0);
  }
  memcpylen=make_jpeg_comment(buf, jpeg_comment);
  ptr = malloc(memcpylen+filelen);
  *data=ptr;
  ptr = memcpy(ptr,jpeg_comment,memcpylen);
  total = 126;
  ptr +=memcpylen;

  while((len = F1fread(port, buf, 0x0400)) != 0){
    if(len < 0)
      return(0);
    total = total + len;
/* gp_camera_progress(camera, ((float)total / (float)filelen)); */
    memcpylen=len;
    ptr = memcpy(ptr,buf,memcpylen);
    ptr +=memcpylen;

  }
  F1fclose(port);
  return(total);
}

static long
get_thumbnail(GPPort *port,char *name, char **data, int format, int n)
{
  u_long filelen;
  u_long total = 0;
  long len;
  int i;
  u_char buf[0x1000];
  u_char *p;
  char *ptr;
  /* printf("name %s,%d\n",name,n); */
  p = buf;

  F1ok(port);
  F1status(port,0);

  filelen = F1finfo(port,name);
  if(filelen == 0)
    return(0);

  if(F1fopen(port,name) != 0)
    return(0);

  for( i = 0 ; i < n ; i++)
    len = F1fseek(port, 0x1000, 1);

  while((len = F1fread(port, p, 0x0400)) != 0){
    if(len < 0){
      F1fclose(port);
      return(0);
    }
    total = total + len;
    p = p + len;
    if(total >= 0x1000)
      break;
  }
  F1fclose(port);

  filelen = buf[12] * 0x1000000 + buf[13] * 0x10000 +
    buf[14] * 0x100 + buf[15];

  ptr = malloc(filelen);
  *data=ptr;
  ptr = memcpy(ptr,&buf[256],filelen);

  /* write_file(&buf[256], (int) filelen, fp); */
  return(total);
}

static void
get_date_info(GPPort *port, char *name, char *outfilename ,char *newfilename)
{
  char *p, *q;
  int year = 0;
  int month = 0;
  int date = 0;
  int hour = 0;
  int minute = 0;
  int second = 0;
  u_char buf[128];

  F1ok(port);
  F1status(port,0);

  (void) F1finfo(port, name);
  if(F1fopen(port, name) ==0){
    if(F1fread(port, buf, 126) == 126){
      if(*(buf+PMP_TAKE_YEAR) != 0xff){
        year = (int) *(buf+PMP_TAKE_YEAR);
        month = (int) *(buf+PMP_TAKE_MONTH);
        date = (int) *(buf+PMP_TAKE_DATE);
        hour = (int) *(buf+PMP_TAKE_HOUR);
        minute = (int) *(buf+PMP_TAKE_MINUTE);
        second = (int) *(buf+PMP_TAKE_SECOND);
      }
    }
    F1fclose(port);
  }

  p = outfilename;
  q = newfilename;
  while(*p){
    if(*p == '%'){
      p++;
      switch(*p){
      case '%':
        *q = '%';
        break;
      case 'H':
        q = q + sprintf(q, "%02d", hour);
        break;
      case 'M':
        q = q + sprintf(q, "%02d", minute);
        break;
       case 'S':
        q = q + sprintf(q, "%02d", second);
        break;
      case 'T':
#ifdef BINARYFILEMODE
        q = q + sprintf(q, "%02d%02d%02d", hour, minute, date);
#else
        q = q + sprintf(q, "%02d:%02d:%02d", hour, minute, date);
#endif
        break;
      case 'y':
        q = q + sprintf(q, "%02d", year);
        break;
      case 'm':
        q = q + sprintf(q, "%02d", month);
        break;
      case 'd':
        q = q + sprintf(q, "%02d", date);
        break;
      case 'D':
#ifdef BINARYFILEMODE
        q = q + sprintf(q, "%02d%02d%02d", year, month, date);
#else
        q = q + sprintf(q, "%02d_%02d_%02d", year, month, date);
#endif
        break;
      default:
        q = q + sprintf(q, "%%%c", *p);
        break;
      }
      p++;
    }else
      *q++ = *p++;
  }
  *q = 0;

}

static long
get_picture(GPPort *port, int n, char **data, int format, int ignore, int all_pic_num)
{
  long  len;
  char name[64];
  char name2[64];
  int i;

  all_pic_num = get_picture_information(port,&i,0);

retry:

  if (all_pic_num < n) {
    fprintf(stderr, "picture number %d is too large. %d\n",all_pic_num,n);
    errflg ++;
    return(GP_ERROR);
   }

  switch(format){
  case PMX:
    sprintf(name, "/PIC_CAM/PIC00000/PIDX%03d.PMX", n - 1);
    break;
  case JPEG_T:
    sprintf(name, "/PIC_CAM/PIC00000/PIDX%03d.PMX",
            (picture_thumbnail_index[n] & 0xff));
    break;
  case JPEG:
  case PMP:
  default:
    if(ignore)
      sprintf(name, "/PIC_CAM/PIC00000/PSN%05d.PMP", n);
    else
      sprintf(name, "/PIC_CAM/PIC00000/PSN%05d.PMP", picture_index[n]);
    break;
  }
  if(ignore)
    sprintf(name2, "/PIC_CAM/PIC00000/PSN%05d.PMP", n );
  else
    sprintf(name2, "/PIC_CAM/PIC00000/PSN%05d.PMP", picture_index[n]);

  /* printf("name %s, name2 %s, %d\n",name,name2,n); */

  if(0)
    switch(format){
    case PMX:
      fprintf(stdout, "pidx%03d.pmx: ", n -1 );
      break;
    case JPEG_T:
      fprintf(stderr, "Thumbnail %03d: ", n);
      break;
    case PMP:
    case JPEG:
    default:
      fprintf(stdout, "Picture %03d: ", n);
      break;
    }

  if(format == JPEG_T)
    len = get_thumbnail(port, name, data, format,
                        0xff & (picture_thumbnail_index[n] >> 8));
  else
    len = get_file(port, name, data, format);
  if(len == 0 ) {
    goto retry;
  }

  if (len < 0)
    errflg ++;

   return(len);
}

static int
delete_picture(GPPort *port, int n, int all_pic_num) {
	if (all_pic_num < n) {
		gp_log (GP_LOG_ERROR, "delete_picture", "picture number %d is too large. %d",n,all_pic_num);
		return GP_ERROR;
	}

	if(picture_protect[n-1] != 0x00){
		gp_log (GP_LOG_DEBUG, "delete_picture", "picture %d is protected.", n);
		return GP_ERROR;
	}
	return F1deletepicture(port, picture_index[n]);
}

int camera_id (CameraText *id) {

        strcpy(id->text, "sonydscf1-bvl");

        return (GP_OK);
}

int camera_abilities (CameraAbilitiesList *list) {

        /* *count = 1; */
        CameraAbilities a;

        /* Fill in each camera model's abilities */
        /* Make separate entries for each conneciton type (usb, serial, etc...)
           if a camera supported multiple ways. */
	memset (&a, 0, sizeof(a));
        strcpy(a.model, "Sony:DSC-F1");
	a.status = GP_DRIVER_STATUS_EXPERIMENTAL;
        a.port=GP_PORT_SERIAL;
        a.speed[0] = 9600;
        a.speed[1] = 19200;
        a.speed[2] = 38400;
        a.operations        =  GP_OPERATION_NONE;
        a.file_operations   =  GP_FILE_OPERATION_DELETE |
                                GP_FILE_OPERATION_PREVIEW;
        a.folder_operations =  GP_FOLDER_OPERATION_NONE;
        gp_abilities_list_append(list, a);

        return (GP_OK);
}

static int camera_exit (Camera *camera, GPContext *context) {
        if(F1ok(camera->port))
           return(GP_ERROR);
        return (F1fclose(camera->port));
}

static int get_file_func (CameraFilesystem *fs, const char *folder,
			  const char *filename, CameraFileType type,
			  CameraFile *file, void *user_data, GPContext *context)
{
	Camera *camera = user_data;
        int num;
	long int size;
	char *data = NULL;

        gp_log (GP_LOG_DEBUG, "sonyf1/get_file_func","folder: %s, file: %s", folder, filename);

        if(!F1ok(camera->port))
           return (GP_ERROR);

	gp_file_set_name (file, filename);
	gp_file_set_mime_type (file, "image/jpeg");

        /* Retrieve the number of the photo on the camera */
	num = gp_filesystem_number(camera->fs, "/", filename, context);
	if (num < GP_OK)
		return num;

	switch (type) {
	case GP_FILE_TYPE_NORMAL:
		size = get_picture (camera->port, num, &data, JPEG, 0, F1howmany(camera->port));
		break;
	case GP_FILE_TYPE_PREVIEW:
		size = get_picture (camera->port, num, &data, JPEG_T, TRUE, F1howmany(camera->port));
		break;
	default:
		return (GP_ERROR_NOT_SUPPORTED);
	}

        if (!data)
                return GP_ERROR;
	gp_file_set_data_and_size (file, data, size);
        return GP_OK;
}

static int
delete_file_func (CameraFilesystem *fs, const char *folder,
	     const char *filename, void *data,
	     GPContext *context)
{
	Camera *camera = data;
        int max, num;

        num = gp_filesystem_number(camera->fs, "/", filename, context);
	if (num<GP_OK)
	   return num;
        max = gp_filesystem_count(camera->fs,folder, context);
	if (max<GP_OK)
	   return max;
        gp_log (GP_LOG_DEBUG, "sonydscf1", "file delete: %d",num);
        if(!F1ok(camera->port))
           return (GP_ERROR);
        return delete_picture(camera->port,num,max);
}

static int
camera_summary (Camera *camera, CameraText *summary, GPContext *context)
{
        int i;

        if(!F1ok(camera->port))
           return (GP_ERROR);
        get_picture_information(camera->port,&i,2);
        return (F1newstatus(camera->port, 1, summary->text));
}

static int camera_about (Camera *camera, CameraText *about, GPContext *context)
{
        strcpy(about->text,
_("Sony DSC-F1 Digital Camera Support\nM. Adam Kendall <joker@penguinpub.com>\nBased on the chotplay CLI interface from\nKen-ichi Hayashi\nGphoto2 port by Bart van Leeuwen <bart@netage.nl>"));

        return (GP_OK);
}

static int file_list_func (CameraFilesystem *fs, const char *folder,
			   CameraList *list, void *data, GPContext *context)
{
	Camera *camera = data;
        F1ok(camera->port);
        /*if(F1ok(camera->port))
           return(GP_ERROR);*/
        /* Populate the list */
        return gp_list_populate(list, "PSN%05i.jpg", F1howmany(camera->port));
}

static CameraFilesystemFuncs fsfuncs = {
	.file_list_func = file_list_func,
	.get_file_func = get_file_func,
	.del_file_func = delete_file_func,
};

int camera_init (Camera *camera, GPContext *context) {
        GPPortSettings settings;

        camera->functions->exit         = camera_exit;
        camera->functions->summary      = camera_summary;
        camera->functions->about        = camera_about;

	/* Configure the port */
        gp_port_set_timeout (camera->port, 5000);
	gp_port_get_settings (camera->port, &settings);
        settings.serial.bits    = 8;
        settings.serial.parity  = 0;
        settings.serial.stopbits= 1;
        gp_port_set_settings (camera->port, settings);

	/* Set up the filesystem */
	return gp_filesystem_set_funcs (camera->fs, &fsfuncs, camera);
}

