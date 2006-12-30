#include <stdio.h>
#include <stdlib.h>

#include "qcam.h"
#include "qcam_gfx.h"

#define NUM_FUZZY_SETS 3
int qc_getgravity_point (int *histogram);
unsigned char *qc_fuzzify (unsigned char **fuzzy_sets, int value);

/******************************************************************/
 
unsigned char *qc_fuzzify (unsigned char **fuzzy_sets, int value)
{
  int set_count, point_count;
  unsigned char *percentage;

  unsigned char fuzzy_sets[NUM_FUZZY_SETS][4]=
  {{0, 0, 30, 50},					// too low set
   {30, 50, 100, 120},					// ok set
   {100, 120, 255, 255}};				// too high set

fprintf (stderr, "VALUE: %d\n", value);

  percentage = malloc (NUM_FUZZY_SETS * sizeof (unsigned char));

/* fuzzify our inputs */

  for (set_count=0; set_count < NUM_FUZZY_SETS; set_count++) {
    percentage[set_count] = 0;

    if ((value < fuzzy_sets[set_count][0]) || (value > fuzzy_sets[set_count][3]))
      continue;					// value out of range

    
    for (point_count = 0; point_count < 3; point_count++) {
      if ((value >= fuzzy_sets[set_count][point_count]) &&
	  (value <= fuzzy_sets[set_count][point_count+1])) {
	if (point_count == 1) {	    		 // we are at the top of the triangle
	  percentage[set_count] = 100;
	}
	else {
	  if (fuzzy_sets[set_count][point_count+1]-fuzzy_sets[set_count][point_count]) {
	    percentage[set_count] = 100/(fuzzy_sets[set_count][point_count+1]- fuzzy_sets[set_count][point_count]);

	    if (point_count == 0) {
	      percentage [set_count] = percentage [set_count] * (value - fuzzy_sets[set_count][point_count]);
	    }
	    else if (point_count == 2) {
	      percentage [set_count] = percentage[set_count] * (fuzzy_sets[set_count][point_count+1] - value);
	    }
	  }
	  else
	    percentage[set_count] = 100;
	}
      }
    }
  }

fprintf (stderr, "low: %d ok: %d high: %d\n", percentage[0],percentage[1],percentage[2]);

  return (percentage);
}
  
/******************************************************************/
/* Get gravity point */

int qc_getgravity_point (int *histogram)
{
  int i;
  int high_pos=0,
    low_pos=0,
    max_pos=0,
    max_ampl=0,
    low_val=0,
    high_val=0;

  high_pos=254;					//DENT: change it !!!
  low_pos = 0;

  for (i=0; high_pos > low_pos; i++) {
    if (max_ampl < histogram[i]) {
      max_ampl = histogram[i];
      max_pos = i;
    }
    
    if (high_val > low_val) {
      low_val += histogram[low_pos++];
    }
    else {
      high_val += histogram[high_pos--];
    }
  }

  return (high_pos);
}

/******************************************************************/

int qc_autobrightness (struct qcam *q, scanbuf *scan)
{

  static int whitebal,
    brightness;
  unsigned char fuzzy_sets[NUM_FUZZY_SETS][4]=
  {{0, 0, 30, 50},					// too low set
   {30, 50, 120, 170},					// ok set
   {120, 170, 255, 255}};				// too high set

  /* Some Fuzzy Logic theory ;-)
     //
     // | low      ok      high
     // |_____    ____    _____
     // |     \  /    \  /
     // |      \/      \/
     // |      /\      /\
     // |     /  \    /  \
     // +-----+--+----+--+-----+
     */

  int *histogram;
  static unsigned char *percentage;

  histogram = qc_gethistogram (q, scan);

  percentage = qc_fuzzify (fuzzy_sets, qc_getgravity_point (histogram));

  free (histogram);

#ifdef DEBUG
  fprintf (stderr, "(qc_autobrightness) fuzzy: low: %d ok: %d high: %d\n",percentage[0], percentage[1], percentage[2]);
#endif

  whitebal = qc_getwhitebal (q);
  brightness = qc_getbrightness (q);
/*
  if (percentage [0]) {
    whitebal += 255*percentage[0]/100/10;

    whitebal = (whitebal > 255) ? 255 : whitebal;
  }
  else if (percentage [2]) {
    whitebal -= 255*percentage[2]/100/10;

    whitebal = (whitebal < 0) ? 0 : whitebal;
  }

  qc_setwhitebal (q, whitebal);
*/
  if (percentage [0]) {
    brightness += (255*percentage[0])/1000;

    brightness = (brightness > 200) ? 200 : brightness;
  }
  else if (percentage [2]) {
    brightness -= (255*percentage[2])/1000;

    brightness = (brightness < 0) ? 0 : brightness;
  }

  qc_setbrightness (q, brightness);

  free (percentage);

  return (1);
}

