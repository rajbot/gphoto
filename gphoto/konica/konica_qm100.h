int konica_qm100_initialize();
int konica_qm100_number_of_pictures ();
int konica_qm100_take_picture();
struct Image *konica_qm100_get_picture (int picNum, int thumbnail);
int konica_qm100_delete_picture (int picNum);
struct Image *konica_qm100_get_preview ();
int konica_qm100_configure ();
char *konica_qm100_summary();
char *konica_qm100_description();
