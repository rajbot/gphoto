
/* Windows Portability
   ------------------------------------------------------------------ */
#ifdef WIN32

GP_DIR GP_OPENDIR (char *dirname) {

	GPWINDIR *wd;
	char dirn[1024];

	wd = (GPWINDIR*)malloc(sizeof(GPWINDIR));

	/* Append the wildcard */
	strcpy(dirn, dirname);
	strcat(dirn, "\\*");

	wd->got_first = 0;
}

GP_DIRENT GP_READDIR (GP_DIR d) {

}

char *GP_FILENAME (GP_DIRENT de) {

}

int  GP_CLOSEDIR (GP_DIR dir) {

}

int GP_IS_FILE (char *filename) {

}

int GP_IS_DIR (char *dirname) {

}


#else

GP_DIR GP_OPENDIR (char *dirname) {
	return (opendir(dirname));
}

GP_DIRENT GP_READDIR (GP_DIR d) {
	return (readdir(d));
}

char *GP_FILENAME (GP_DIRENT de) {
	return (de->d_name);
}

int GP_CLOSEDIR (GP_DIR dir) {
	closedir(dir);
}

int GP_IS_FILE (char *filename) {
	struct stat st;

	if (stat(filename, &st)==0)
		return 0;
	return (!S_ISDIR(st.st_mode));
}

int GP_IS_DIR (char *dirname) {
	struct stat st;

	if (stat(dirname, &st)==0)
		return 0;
	return (S_ISDIR(st.st_mode));
}
#endif
