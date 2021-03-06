/*
 * commands.c 
 *
 *  Command set for the digita cameras
 *
 * Copyright 1999-2000 Johannes Erdfelt
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <termios.h>

#include "digita.h"

#include "../src/gphoto.h"

#include <gpio.h>

int digita_num_pictures = 0;
struct file_item *digita_file_list = NULL;

void build_command(struct digita_command *cmd, int length, short command)
{
	/* Length is the sizeof the digita_command minus the length */
	/*  parameter, plus whatever other data we send */
	cmd->length = htonl(sizeof(struct digita_command) -
		sizeof(unsigned int) + length);
	cmd->unknown = cmd->status = 0;
	cmd->command = htons(command);
}

struct storage_status {
        struct digita_command cmd;
        
        unsigned int takencount;
        unsigned int availablecount;
        int rawcount;
};

int digita_get_storage_status(struct digita_device *dev, int *taken,
	int *available, int *rawcount)
{
	struct digita_command cmd;
	struct storage_status ss;
	int ret;

	build_command(&cmd, 0, DIGITA_GET_STORAGE_STATUS);

	ret = digita_send(dev, &cmd, sizeof(cmd));
	if (ret < 0) {
		fprintf(stderr, "digita_get_storage_status: error sending command\n");
		return -1;
	}

	ret = digita_read(dev, (unsigned char *)&ss, sizeof(ss));
	if (ret < 0) {
		fprintf(stderr, "digita_get_storage_status: error getting count\n");
		return -1;
	}

	if (taken)
		*taken = ntohl(ss.takencount);
	if (available)
		*available = ntohl(ss.availablecount);
	if (rawcount)
		*rawcount = ntohl(ss.rawcount);

	return 0;
}

int digita_get_file_list(struct digita_device *dev)
{
	struct digita_command cmd;
	struct get_file_list gfl;
	char *buffer;
	int ret, taken, buflen;

	if (digita_get_storage_status(dev, &taken, NULL, NULL) < 0)
		return -1;

	digita_num_pictures = taken;

	buflen = (taken * sizeof(struct file_item)) + sizeof(cmd) + 4;
	buffer = malloc(buflen);
	if (!buffer) {
		fprintf(stderr, "digita_get_file_list: error allocating %d ",
"bytes\n", buflen);
		return -1;
	}

	build_command(&gfl.cmd, sizeof(gfl) - sizeof(gfl.cmd), DIGITA_GET_FILE_LIST);
	gfl.listorder = htonl(1);

	ret = digita_send(dev, (void *)&gfl, sizeof(gfl));
	if (ret < 0) {
		fprintf(stderr, "digita_get_file_list: error sending command\n");
		return -1;
	}

	ret = digita_read(dev, (void *)buffer, buflen);
	if (ret < 0) {
		fprintf(stderr, "digita_get_file_list: error receiving data\n");
		return -1;
	}

	if (digita_file_list)
		free(digita_file_list);

	digita_file_list = malloc(taken * sizeof(struct file_item));
	if (!digita_file_list) {
		fprintf(stderr, "digita_get_file_list: error allocating file_list memory\n");
		return -1;
	}

	memcpy(digita_file_list, buffer + sizeof(cmd) + 4, taken * sizeof(struct file_item));

	free(buffer);

	return 0;
}

#define GFD_BUFSIZE 19432
int digita_get_file_data(struct digita_device *dev, int thumbnail,
	struct filename *filename, struct partial_tag *tag, void *buffer)
{
	struct get_file_data_send gfds;
	struct get_file_data_receive *gfdr;
	int ret;
	char *tbuf;

	build_command(&gfds.cmd, sizeof(gfds) - sizeof(gfds.cmd), DIGITA_GET_FILE_DATA);

	memcpy(&gfds.fn, filename, sizeof(gfds.fn));
	memcpy(&gfds.tag, tag, sizeof(gfds.tag));
	gfds.dataselector = htonl(thumbnail);

	tbuf = malloc(GFD_BUFSIZE + sizeof(*gfdr));
	if (!tbuf) {
		fprintf(stderr, "digita_get_file_data: unable to allocate %d bytes\n",
			GFD_BUFSIZE + sizeof(*gfdr));

		return -1;
	}
	gfdr = (struct get_file_data_receive *)tbuf;

	ret = digita_send(dev, &gfds, sizeof(gfds));
	if (ret < 0) {
		fprintf(stderr, "digita_get_file_data: error sending command\n");
		return -1;
	}

	ret = digita_read(dev, gfdr, GFD_BUFSIZE + sizeof(*gfdr));
	if (ret < 0) {
		fprintf(stderr, "digita_get_file_data: error reading data\n");
		return -1;
	}

	memcpy(buffer, tbuf + sizeof(*gfdr), ntohl(gfdr->tag.length));
	memcpy(tag, &gfdr->tag, sizeof(*tag));

	free(tbuf);

	return 0;
}

int digita_delete_picture(struct digita_device *dev, struct filename *filename)
{
	struct erase_file ef;
	struct digita_command response;
	int ret;

	build_command(&ef.cmd, sizeof(ef) - sizeof(ef.cmd), DIGITA_ERASE_FILE);

	memcpy(&ef.fn, filename, sizeof(ef.fn));

	ret = digita_send(dev, (unsigned char *)&ef, sizeof(ef));
	if (ret < 0) {
		fprintf(stderr, "error sending command\n");
		return -1;
	}

	ret = digita_read(dev, (unsigned char *)&response, sizeof(response));
	if (ret < 0) {
		fprintf(stderr, "error reading reply\n");
		return -1;
	}

	return 0;
}
