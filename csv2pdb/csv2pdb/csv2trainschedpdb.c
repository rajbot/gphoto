#include <stdio.h>
#include <time.h>
#include <stdlib.h>  
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NBMAXGARES 20
#define LGMAXNOMGARE 20
#define AFFMAX 10
#define CREATORID "TRSC"
#define DBTYPE "DATA"
#define FATTRIBS "\x20\x0" /* 0x0020 = RO | backup | OK to overwrite */
#define VERSION "\x1\x0" /* 0.1 */
#define TIME_CONSTANT 2082844800

/* Globals */

typedef struct station {
  char nom[20];
} station;

long end_header_pos;
unsigned int num_records=0;

/*
 * Write record info in header
 * arguments: 
 *    file descriptor
 *    record number
 */

int writerecordinfo(FILE *pdbfd, unsigned int record_num, long pos)
{
  char temp[4]={0};
  int i;
  long saved_pos=0;

  /* store current pos in pdb file */
  saved_pos = ftell(pdbfd);

  /* write record info */
  fseek(pdbfd,(record_num-1)*8 + end_header_pos,SEEK_SET);
  i = pos;
  //  printf("record: %i %i\n",record_num,i);
  temp[0]= i >> 24 ;
  temp[1]= i >> 16;
  temp[2]= i >> 8;
  temp[3]= i & 0xFFFFFF;
  fwrite(temp,4,1,pdbfd);

  /* go back to saved position */
  fseek(pdbfd,saved_pos,SEEK_SET);

  return 0;
}

int main(int argc, char *argv[])
{

  FILE *fd, *pdbfd;
  int c=1, nbgares=0, i,j=0, nbdays=0, nbdir=0, notrain=0;
  char temp[LGMAXNOMGARE+1]={0};
  int cur_record;
  long cur_record_pos=0;

  if (argc != 4) {
    printf("usage : %s <source> <dest> <PDB Name>\n",argv[0]);
    exit(0);
  }

  /* open source file */

  fd = fopen(argv[1],"r");
  if (fd == NULL) {
    printf("Erreur d'ouverture du fichier %s\n",argv[1]);
    exit(-1);
  }

  /* create destination file */
  
  pdbfd = fopen(argv[2],"w");
  if (pdbfd == NULL) {
    printf("Erreur de création du fichier %s\n",argv[1]);
    exit(-1);
  }

  /* header */
  
  /*PDB Name Max 32 chars , \0 terminated */
  fwrite(argv[3],1,strlen(argv[3]),pdbfd);

  for(i=0;i<(32-strlen(argv[3]));i++)
    fwrite("\x0",1,1,pdbfd);

  /* File attribs */
  fwrite(FATTRIBS,2,1,pdbfd);

  /* Version */
  fwrite(VERSION,2,1,pdbfd);

  /* creation date FIXME */
  fwrite("\xB7\x2D\xB1\x9A",4,1,pdbfd);

  /* Modification date FIXME */
  fwrite( "\xB7\x2D\xB1\x9A",4,1,pdbfd);

  /* last Backup date */
  fwrite( "\x0\x0\x0\x0",4,1,pdbfd);

  /* Modification number */
  fwrite( "\x0\x0\x0\x0",4,1,pdbfd);

  /* AppInfo Area */
  fwrite( "\x0\x0\x0\x0",4,1,pdbfd);

  /* SortInfo Area */
  fwrite( "\x0\x0\x0\x0",4,1,pdbfd);

  /* Database Type */
  fwrite( DBTYPE,4,1,pdbfd);

  /* Creator ID */
  fwrite( CREATORID,4,1,pdbfd);

  /* Unique ID Seed */
  fwrite( "\x0\x0\x0\x0",4,1,pdbfd);

  /* Next record List ID */
  fwrite("\x0\x0\x0\x0",4,1,pdbfd);
 
  /* Number of records to go here */
  fwrite("\x0\x0",2,1,pdbfd);
 
  end_header_pos=ftell(pdbfd);

  /* count number of records in source file */
  while ((c = fgetc(fd)) != EOF) {
    if (c == '\n')
      num_records++;
  }
  printf("There are %i lines (records)\n",num_records);

  /* save it in PDB Header */

  /*  write number of records in header */
  fseek(pdbfd,end_header_pos-2,SEEK_SET);
  temp[0] = num_records >> 8;
  temp[1] = num_records & 0xFF;
  
  fwrite(temp,2,1,pdbfd);

  /* leave room for records info */
  for (i=0;i<num_records;i++) {
    fwrite("\x0\x0\x0\x0\x0\x0\x0\x0",8,1,pdbfd);
  }

  rewind(fd);

  cur_record=1;
  cur_record_pos=ftell(pdbfd);

  /* read Station Names on 1st line */
  for(;;) {
    c = fgetc(fd);
    
    if(c == '\t') {
      nbgares++;
    }
    if (c != '\n')
      fwrite(&c,1,1,pdbfd);
    else {
      fwrite("\x0",1,1,pdbfd);
      break;
    }
  }  
  nbgares++;
  printf("j'ai trouvé %i gares\n",nbgares);

  /* Write record info in record List */
  writerecordinfo(pdbfd, cur_record, cur_record_pos);
  
  cur_record_pos=ftell(pdbfd);
  cur_record++;

  /* read Day List */
  for(;;) {
    c = fgetc(fd);
    if(c == '\t') {
      nbdays++;
    }
    if (c != '\n')
      fwrite(&c,1,1,pdbfd);
    else {
      fwrite("\x0",1,1,pdbfd);
      break;
    }
  }  
  nbdays++;
  printf("%i days found\n",nbdays);

  writerecordinfo(pdbfd, cur_record, cur_record_pos);

  cur_record_pos=ftell(pdbfd);
  cur_record++;

  /* Read line offset for the various days and directions schedules */
  i=0;
  for(;;) {
    c = fgetc(fd);
    if(c == '\t') {
      temp[i]='\0';
      j = atoi(temp);
      temp[0]=j >> 8;
      temp[1]=j & 0xFF;
      fwrite(temp,2,1,pdbfd);
      i=0;
      nbdir++;
    }
    if (c != '\n')
      temp[i++]=c;
    else {
      temp[i]='\0';
      j = atoi(temp);
      temp[0] = j >> 8;
      temp[1] = j & 0xFF;
      fwrite(temp,2,1,pdbfd);
      fwrite("\x0",1,1,pdbfd);
      break;
    }
  }  
  nbdir++;
  printf("%i time tables found\n",nbdir);

  writerecordinfo(pdbfd, cur_record, cur_record_pos);

  cur_record_pos=ftell(pdbfd);
  cur_record++;

  /* Let's go for the real thing */
  i=0;
  notrain=0;
  for(;;) {
    c=fgetc(fd);
    
    /* we write time as int */
    if ((c != '\t') && (c != '\n')) {
      if (c != ':')
	temp[i++]=c;
    }
    else if (c == '\t') {
      temp[i]='\0';

      for(j=0;j<i;j++) {
	if(temp[j]=='-') {
	  notrain=1;
	  fwrite("\xFF\xFF",2,1,pdbfd);
	  break;
	}
      }
      if (notrain==0) {
	j = atoi(temp);
	temp[0] = j >> 8;
	temp[1] = j & 0xFF;
	fwrite(temp,2,1,pdbfd);
      }

      notrain=0;
      i=0;
    }     

    if ((c == '\n') || (c == EOF)) {
      temp[i]='\0';

      for(j=0;j<i;j++) {
	if(temp[j]=='-') {
	  notrain=1;
	  fwrite("\xFF\xFF",2,1,pdbfd);
	  break;
	}
      }
      if (notrain==0) {
	j = atoi(temp);
	temp[0] = j >> 8;
	temp[1] = j & 0xFF;
	fwrite(temp,2,1,pdbfd);
      }

      notrain=0;
      i=0;

      if (c != EOF) {
	writerecordinfo(pdbfd, cur_record, cur_record_pos); 
	cur_record_pos=ftell(pdbfd);
	cur_record++;
      } else
	break;
    }

  }


  fflush(pdbfd);
  fclose(pdbfd);
  fclose(fd);

  return 0;
}
