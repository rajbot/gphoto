/*
  EXIF file parser for GPHOTO package
  Copyright (C) 1999 Matthew G. Martin

  This routine works for Fuji DS-7 files.  Don't know about any others.

  Heavily ripped from  exifdump.py
  Written by Thierry Bousch <bousch@topo.math.u-psud.fr>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "exif.h"

main(int argc, char** argv){
  FILE *fd;
  unsigned char tmpbuff[32];
  unsigned int offset;
    long dataptr,dsize,tag,datvec,size,tmp;
  int i,j;

  struct exif_parser exifdat;

  fd=fopen(argv[1],"r");
  printf("Parsing %s:\n",argv[1]);
  if (!fd) return(0);

  fread(tmpbuff,1,12,fd);

  exifdat.header=tmpbuff;
  exif_header_parse(&exifdat);
  exifdat.preparsed=0;
  exifdat.data=malloc(exifdat.exiflen);

  dsize=fread(exifdat.data,1,exifdat.exiflen,fd);
#ifdef EXIF_DEBUG
  printf("%ld bytes read\n",dsize);
#endif
  
  fclose(fd);

  offset=lilend(exifdat.data+4,4);
#ifdef EXIF_DEBUG
  printf("Offset is %d bytes\n",offset);
#endif
  if (stat_exif(&exifdat)) exit(1);
  
  printf("Contains info for %d images\n",exifdat.ifdcnt);

  dump_exif(&exifdat);

};
