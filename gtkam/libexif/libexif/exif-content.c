/* exif-content.c
 *
 * Copyright (C) 2001 Lutz M�ller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <config.h>
#include "exif-content.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libjpeg/jpeg-marker.h>

//#define DEBUG

static const unsigned char ExifHeader[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};

typedef struct _ExifContentNotifyData ExifContentNotifyData;
struct _ExifContentNotifyData {
	ExifContentEvent events;
	ExifContentNotifyFunc func;
	void *data;
};

struct _ExifContentPrivate
{
	ExifContentNotifyData *notifications;
	unsigned int count;

	unsigned int ref_count;
};

unsigned int
exif_content_add_notify (ExifContent *content, ExifContentEvent events,
			 ExifContentNotifyFunc func, void *data)
{
	if (!content)
		return (0);

	if (!content->priv->notifications)
		content->priv->notifications =
				malloc (sizeof (ExifContentNotifyData));
	else
		content->priv->notifications = 
			realloc (content->priv->notifications,
				 sizeof (ExifContentNotifyData) *
				 	(content->priv->count + 1));
	if (!content->priv->notifications)
		return (0);
	content->priv->notifications[content->priv->count].events = events;
	content->priv->notifications[content->priv->count].func = func;
	content->priv->notifications[content->priv->count].data = data;
	content->priv->count++;

	return (content->priv->count);
}

void
exif_content_remove_notify (ExifContent *content, unsigned int id)
{
	if (!content)
		return;
	if (id > content->priv->count)
		return;

	memmove (content->priv->notifications + id - 1,
		 content->priv->notifications + id,
		 sizeof (ExifContentNotifyData) *
		 	(content->priv->count - id));
	content->priv->count--;
}

ExifContent *
exif_content_new (void)
{
	ExifContent *content;

	content = malloc (sizeof (ExifContent));
	if (!content)
		return (NULL);
	memset (content, 0, sizeof (ExifContent));
	content->priv = malloc (sizeof (ExifContentPrivate));
	if (!content->priv) {
		free (content);
		return (NULL);
	}
	memset (content->priv, 0, sizeof (ExifContentPrivate));
	content->priv->ref_count = 1;

	return (content);
}

void
exif_content_ref (ExifContent *content)
{
	content->priv->ref_count++;
}

void
exif_content_unref (ExifContent *content)
{
	content->priv->ref_count--;
	if (!content->priv->ref_count)
		exif_content_free (content);
}

void
exif_content_free (ExifContent *content)
{
	unsigned int i;

	for (i = 0; i < content->count; i++)
		exif_entry_unref (content->entries[i]);
	free (content->entries);
	free (content->priv);
	free (content);
}

void
exif_content_parse (ExifContent *content, const unsigned char *data,
		    unsigned int size, unsigned int offset,
		    ExifByteOrder order)
{
	unsigned int i;
	ExifShort n;

	if (!content)
		return;

	content->order = order;

	/* Read number of entries */
	if (size < offset + 2)
		return;
	n = exif_get_short (data + offset, order);
#ifdef DEBUG
	printf ("Parsing directory with %i entries...\n", n);
#endif

	content->entries = malloc (sizeof (ExifEntry *) * n);
	if (!content->entries)
		return;
	memset (content->entries, 0, sizeof (ExifEntry *) * n);
	content->count = n;

	for (i = 0; i < n; i++) {
		content->entries[i] = exif_entry_new ();
		content->entries[i]->parent = content;
		exif_entry_parse (content->entries[i], data, size,
				  offset + 2 + 12 * i, order);
	}
}

void
exif_content_dump (ExifContent *content, unsigned int indent)
{
	char buf[1024];
	unsigned int i;

	for (i = 0; i < 2 * indent; i++)
		buf[i] = ' ';
	buf[i] = '\0';

	if (!content)
		return;

	printf ("%sDumping exif content (%i entries)...\n", buf,
		content->count);
	for (i = 0; i < content->count; i++)
		exif_entry_dump (content->entries[i], indent + 1);
}

void
exif_content_add_entry (ExifContent *content, ExifEntry *entry)
{
	unsigned int i;

	if (entry->parent)
		return;

	entry->parent = content;
	content->entries = realloc (content->entries,
				    sizeof (ExifEntry) * (content->count + 1));
	content->entries[content->count] = entry;
	exif_entry_ref (entry);
	content->count++;

	/* Notification */
	for (i = 0; i < content->priv->count; i++)
		if (content->priv->notifications[i].events & 
						EXIF_CONTENT_EVENT_ADD)
			content->priv->notifications[i].func (content, entry, 
				content->priv->notifications[i].data);
}

void
exif_content_remove_entry (ExifContent *content, ExifEntry *entry)
{
	unsigned int i;

	if (entry->parent != content)
		return;

	for (i = 0; i < content->count; i++)
		if (content->entries[i] == entry)
			break;
	if (i == content->count)
		return;

	memmove (&content->entries[i], &content->entries[i + 1],
		 sizeof (ExifEntry) * (content->count - i - 1));

	/* Notification */
	for (i = 0; i < content->priv->count; i++) 
		if (content->priv->notifications[i].events &
						EXIF_CONTENT_EVENT_REMOVE)
			content->priv->notifications[i].func (content, entry,
				content->priv->notifications[i].data);

	entry->parent = NULL;
	exif_entry_unref (entry);
}

ExifEntry *
exif_content_get_entry (ExifContent *content, ExifTag tag)
{
	unsigned int i;

	if (!content)
		return (NULL);

	for (i = 0; i < content->count; i++)
		if (content->entries[i]->tag == tag)
			return (content->entries[i]);
	return (NULL);
}
