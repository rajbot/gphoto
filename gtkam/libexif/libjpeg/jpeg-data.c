/* jpeg-data.c
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
#include "jpeg-data.h"

#include <stdlib.h>
#include <stdio.h>

//#define DEBUG

struct _JPEGDataPrivate
{
	unsigned int ref_count;
};

JPEGData *
jpeg_data_new (void)
{
	JPEGData *data;

	data = malloc (sizeof (JPEGData));
	if (!data)
		return (NULL);
	memset (data, 0, sizeof (JPEGData));
	data->priv = malloc (sizeof (JPEGDataPrivate));
	if (!data->priv) {
		free (data);
		return (NULL);
	}
	memset (data->priv, 0, sizeof (JPEGDataPrivate));
	data->priv->ref_count = 1;

	return (data);
}

static void
jpeg_data_append_section (JPEGData *data)
{
	JPEGSection *s;

	if (!data->count)
		s = malloc (sizeof (JPEGSection));
	else
		s = realloc (data->sections,
			     sizeof (JPEGSection) * (data->count + 1));
	if (!s)
		return;

	data->sections = s;
	data->count++;
}

void
jpeg_data_save_file (JPEGData *data, const char *path)
{
	FILE *f;
	unsigned char *d = NULL;
	unsigned int size = 0;

	jpeg_data_save_data (data, &d, &size);
	if (!d)
		return;

	f = fopen (path, "w");
	if (!f) {
		free (d);
		return;
	}
	fwrite (d, sizeof (char), size, f);
	fclose (f);
	free (d);
}

void
jpeg_data_save_data (JPEGData *data, unsigned char **d, unsigned int *ds)
{
	unsigned int i, len;
	JPEGSection s;

	if (!data)
		return;
	if (!d)
		return;
	if (!ds)
		return;

	for (*ds = i = 0; i < data->count; i++) {
		s = data->sections[i];
#ifdef DEBUG
		printf ("Writing marker 0x%x at position %i...\n",
			s.marker, *ds);
#endif
		switch (s.marker) {
		case JPEG_MARKER_SOF0:
		case JPEG_MARKER_SOF1:
		case JPEG_MARKER_SOF2:
		case JPEG_MARKER_SOF3:
		case JPEG_MARKER_SOF5:
		case JPEG_MARKER_SOF6:
		case JPEG_MARKER_SOF7:
		case JPEG_MARKER_SOF9:
		case JPEG_MARKER_SOF10:
		case JPEG_MARKER_SOF11:
		case JPEG_MARKER_SOF13:
		case JPEG_MARKER_SOF14:
		case JPEG_MARKER_SOF15:
			len = 10;
			*d = realloc (*d, sizeof (char) * (*ds + len));
			(*d)[*ds + 0] = 0xff;
			(*d)[*ds + 1] = s.marker;
			(*d)[*ds + 2] = 10 >> 8;
			(*d)[*ds + 3] = 10 >> 0;
			(*d)[*ds + 4] = s.content.sof.precision;
			(*d)[*ds + 5] = s.content.sof.height >> 8;
			(*d)[*ds + 6] = s.content.sof.height >> 0;
			(*d)[*ds + 7] = s.content.sof.width  >> 8;
			(*d)[*ds + 8] = s.content.sof.width  >> 0;
			(*d)[*ds + 9] = s.content.sof.components;
			break;
		case JPEG_MARKER_SOI:
		case JPEG_MARKER_EOI:
			len = 2;
			*d = realloc (*d, sizeof (char) * (*ds + len));
			(*d)[*ds + 0] = 0xff;
			(*d)[*ds + 1] = s.marker;
			break;
		case JPEG_MARKER_SOS:
			len = s.content.sos.size + 2;
			*d = realloc (*d, sizeof (char) * (*ds + len));
			(*d)[*ds + 0] = 0xff;
			(*d)[*ds + 1] = s.marker;
			memcpy (&((*d)[*ds + 2]), s.content.sos.data, 
				s.content.sos.size);
			break;
		default:
			len = 0;
			break;
		}
		*ds += len;
	}
}

JPEGData *
jpeg_data_new_from_data (const unsigned char *d,
			 unsigned int size)
{
	JPEGData *data;

	data = jpeg_data_new ();
	jpeg_data_load_data (data, d, size);
	return (data);
}

void
jpeg_data_load_data (JPEGData *data, const unsigned char *d,
		     unsigned int size)
{
	unsigned int i, o, len;
	JPEGSection *s;
	JPEGMarker marker;

	if (!data)
		return;
	if (!d)
		return;

#ifdef DEBUG
	printf ("Parsing %i bytes...\n", size);
#endif

	for (o = 0; o < size;) {

		/*
		 * JPEG sections start with 0xff. The first byte that is
		 * not 0xff is a marker (hopefully).
		 */
		for (i = 0; i < 7; i++)
			if (d[o + i] != 0xff)
				break;
		if (!JPEG_IS_MARKER (d[o + i]))
			return;
		marker = d[o + i];

#ifdef DEBUG
		printf ("Found marker 0x%x ('%s') at %i.\n", marker,
			jpeg_marker_get_name (marker), o + i);
#endif

		/* Append this section */
		jpeg_data_append_section (data);
		s = &data->sections[data->count - 1];
		s->marker = marker;

		switch (marker) {
		case JPEG_MARKER_SOF0:
                case JPEG_MARKER_SOF1:
                case JPEG_MARKER_SOF2:
                case JPEG_MARKER_SOF3:
                case JPEG_MARKER_DHT:
                case JPEG_MARKER_SOF5:
                case JPEG_MARKER_SOF6:
                case JPEG_MARKER_SOF7:
                case JPEG_MARKER_JPG:
                case JPEG_MARKER_SOF9:
                case JPEG_MARKER_SOF10:
                case JPEG_MARKER_SOF11:
                case JPEG_MARKER_DAC:
                case JPEG_MARKER_SOF13:
                case JPEG_MARKER_SOF14:
                case JPEG_MARKER_SOF15:
		case JPEG_MARKER_RST0:
		case JPEG_MARKER_RST1:
		case JPEG_MARKER_RST2:
		case JPEG_MARKER_RST3:
		case JPEG_MARKER_RST4:
		case JPEG_MARKER_RST5:
		case JPEG_MARKER_RST6:
		case JPEG_MARKER_RST7:
		case JPEG_MARKER_DQT:
                case JPEG_MARKER_DNL:
                case JPEG_MARKER_DRI:
                case JPEG_MARKER_DHP:
                case JPEG_MARKER_EXP:
                case JPEG_MARKER_APP0:
		case JPEG_MARKER_APP1:
		case JPEG_MARKER_APP2:
		case JPEG_MARKER_APP3:
		case JPEG_MARKER_APP4:
		case JPEG_MARKER_APP5:
		case JPEG_MARKER_APP6:
		case JPEG_MARKER_APP7:
		case JPEG_MARKER_APP8:
		case JPEG_MARKER_APP9:
		case JPEG_MARKER_APP10:
		case JPEG_MARKER_APP11:
		case JPEG_MARKER_APP12:
		case JPEG_MARKER_APP13:
		case JPEG_MARKER_APP14:
		case JPEG_MARKER_APP15:
		case JPEG_MARKER_JPG0:
		case JPEG_MARKER_JPG1:
		case JPEG_MARKER_JPG2:
		case JPEG_MARKER_JPG3:
		case JPEG_MARKER_JPG4:
		case JPEG_MARKER_JPG5:
		case JPEG_MARKER_JPG6:
		case JPEG_MARKER_JPG7:
		case JPEG_MARKER_JPG8:
		case JPEG_MARKER_JPG9:
		case JPEG_MARKER_JPG10:
		case JPEG_MARKER_JPG11:
		case JPEG_MARKER_JPG12:
		case JPEG_MARKER_JPG13:
		case JPEG_MARKER_COM:
			
			/* Read the length of the section */
			len = (d[o + i + 1] << 8) | d[o + i + 2];

			switch (marker) {
			case JPEG_MARKER_APP1:
				s->content.exif = exif_data_new_from_data (
					&d[o + i], size - o - i);
				break;
			case JPEG_MARKER_SOF0:
			case JPEG_MARKER_SOF1:
			case JPEG_MARKER_SOF2:
			case JPEG_MARKER_SOF3:
			case JPEG_MARKER_SOF5:
			case JPEG_MARKER_SOF6:
			case JPEG_MARKER_SOF7:
			case JPEG_MARKER_SOF9:
			case JPEG_MARKER_SOF10:
			case JPEG_MARKER_SOF11:
			case JPEG_MARKER_SOF13:
			case JPEG_MARKER_SOF14:
			case JPEG_MARKER_SOF15:
				s->content.sof.precision =  d[o + i + 3];
				s->content.sof.height =    (d[o + i + 4] << 8) |
							    d[o + i + 5];
				s->content.sof.width =     (d[o + i + 6] << 8) |
							    d[o + i + 7];
				s->content.sof.components = d[o + i + 8];
				break;
			default:
				break;
			}
			break;

		case JPEG_MARKER_SOS:

			/*
			 * Length is all bytes up to EOF except the 
			 * last two bytes.
			 */
			len = size - 2 - o - i - 1;

			s->content.sos.data = malloc (sizeof (char) * len);
			if (!s->content.sos.data)
				return;
			memcpy (s->content.sos.data, d + o + i + 1,
				len);
			s->content.sos.size = len;
			break;

		case JPEG_MARKER_SOI:
		case JPEG_MARKER_EOI:
			len = 0;
			break;
		default:
			return;
		}

		/* Jump to next section */
		o += i + 1 + len;
	}
}

JPEGData *
jpeg_data_new_from_file (const char *path)
{
	JPEGData *data;

	data = jpeg_data_new ();
	jpeg_data_load_file (data, path);
	return (data);
}

void
jpeg_data_load_file (JPEGData *data, const char *path)
{
	FILE *f;
	unsigned char *d;
	unsigned int size;

	if (!data)
		return;
	if (!path)
		return;

	f = fopen (path, "r");
	if (!f)
		return;

	/* For now, we read the data into memory. Patches welcome... */
	fseek (f, 0, SEEK_END);
	size = ftell (f);
	fseek (f, 0, SEEK_SET);
	d = malloc (sizeof (char) * size);
	if (!d) {
		fclose (f);
		return;
	}
	if (fread (d, 1, size, f) != size) {
		free (d);
		fclose (f);
		return;
	}
	fclose (f);

	jpeg_data_load_data (data, d, size);
	free (d);
}

void
jpeg_data_ref (JPEGData *data)
{
	if (!data)
		return;

	data->priv->ref_count++;
}

void
jpeg_data_unref (JPEGData *data)
{
	if (!data)
		return;

	data->priv->ref_count--;
	if (!data->priv->ref_count)
		jpeg_data_free (data);
}

void
jpeg_data_free (JPEGData *data)
{
	unsigned int i;
	JPEGSection s;

	if (!data)
		return;

	if (data->count) {
		for (i = 0; i < data->count; i++) {
			s = data->sections[i];
			switch (s.marker) {
			case JPEG_MARKER_SOF0:
			case JPEG_MARKER_SOF1:
			case JPEG_MARKER_SOF2:
			case JPEG_MARKER_SOF3:
			case JPEG_MARKER_SOF5:
			case JPEG_MARKER_SOF6:
			case JPEG_MARKER_SOF7:
			case JPEG_MARKER_SOF9:
			case JPEG_MARKER_SOF10:
			case JPEG_MARKER_SOF11:
			case JPEG_MARKER_SOF13:
			case JPEG_MARKER_SOF14:
			case JPEG_MARKER_SOF15:
				break;
			case JPEG_MARKER_SOS:
				if (s.content.sos.size)
					free (s.content.sos.data);
				break;
			case JPEG_MARKER_APP1:
				if (s.content.exif)
					exif_data_unref (s.content.exif);
				break;
			default:
				break;
			}
		}
		free (data->sections);
	}
	free (data->priv);
	free (data);
}

void
jpeg_data_dump (JPEGData *data)
{
	unsigned int i;
	JPEGContent content;
	JPEGMarker marker;

	if (!data)
		return;

	printf ("Dumping JPEG data...\n");
	for (i = 0; i < data->count; i++) {
		marker = data->sections[i].marker;
		content = data->sections[i].content;
		printf ("Section %i (marker 0x%x - %s):\n", i, marker,
			jpeg_marker_get_name (marker));
		printf ("  Description: %s\n",
			jpeg_marker_get_description (marker));
		switch (marker) {
                case JPEG_MARKER_SOF0:
                case JPEG_MARKER_SOF1:
                case JPEG_MARKER_SOF2:
                case JPEG_MARKER_SOF3:
                case JPEG_MARKER_SOF5:
                case JPEG_MARKER_SOF6:
                case JPEG_MARKER_SOF7:
                case JPEG_MARKER_SOF9:
                case JPEG_MARKER_SOF10:
                case JPEG_MARKER_SOF11:
                case JPEG_MARKER_SOF13:
                case JPEG_MARKER_SOF14:
                case JPEG_MARKER_SOF15:
                        printf ("  Width: %i\n", content.sof.width);
                        printf ("  Height: %i\n", content.sof.height);
                        printf ("  Components: %i\n", content.sof.components);
                        printf ("  Precision: %i\n", content.sof.precision);
                        break;
                case JPEG_MARKER_SOI:
                case JPEG_MARKER_EOI:
                        /* No content. */
			break;
                case JPEG_MARKER_SOS:
                        printf ("  %i bytes.\n", content.sos.size);
                        break;
                case JPEG_MARKER_APP1:
			exif_data_dump (content.exif);
			break;
                default:
                        printf ("  Unknown content.\n");
                        break;
                }
        }
}

ExifData *
jpeg_data_get_exif_data (JPEGData *data)
{
	unsigned int i;

	if (!data)
		return NULL;

	for (i = 0; i < data->count; i++)
		if (data->sections[i].marker == JPEG_MARKER_APP1) {
			exif_data_ref (data->sections[i].content.exif);
			return (data->sections[i].content.exif);
		}

	return (NULL);
}
