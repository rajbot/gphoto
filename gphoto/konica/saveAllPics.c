#include "saveAllPics.h"

void qm100_saveAllPics()
{
  int pic=1, counter=1, total=PICTURE_COUNT, gap=0, ret;
  char filename[64]="test.jpg";

  while (counter <= total)
    {
      printf("Saving picture %d of %d to %s\n", counter, total, filename);
      do
        {
          ret = qm100_savePic(filename, counter+gap);
          if (ret == 0)
            {
              gap++;
              printf("qm100 : skipping deleted picture\n");
            }
        }
      while (ret == 0);
      //      qm100_erasePic(counter+gap);
      pic++;
      counter++;
    }
}
