/*  Copyright (C) 2012 Roberto Guido <rguido@src.gnome.org>
 *
 *  This file is part of wallmemo
 *
 *  wallmemo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _XOPEN_SOURCE	500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <librsvg/rsvg.h>

static gboolean init = FALSE;
static gchar *conf_file = NULL;
static gchar *content_file = NULL;
static int delete = -1;
static int replace = -1;
static int position = -1;
static gboolean noline = FALSE;
static gboolean empty = FALSE;

static gchar *bg_color = "#000000";
static gchar *fg_color = "#FFFFFF";
static int width = 1366;
static int height = 768;
static gchar *output_file = NULL;

static GOptionEntry entries[] =
{
	{ "init", 0, 0, G_OPTION_ARG_NONE, &init, "Initialize the configuration file. This is implicit if the file doesn't exists", NULL },
	{ "conf", 'c', 0, G_OPTION_ARG_FILENAME, &conf_file, "Use the specified configuration file (default: ~/.config/wallmemo/config)", "<file>" },
	{ "file", 'f', 0, G_OPTION_ARG_FILENAME, &content_file, "Use the specified contents file (default: ~/.config/wallmemo/contents)", "<file>" },
	{ "delete", 'd', 0, G_OPTION_ARG_INT, &delete, "Deletes row N in contents file", "N" },
	{ "replace", 'r', 0, G_OPTION_ARG_INT, &replace, "Replaces row N in contents file with new supplied content", "N" },
	{ "position", 'p', 0, G_OPTION_ARG_INT, &position, "Force the new supplied content at row N, shifting existing contents down. By default, new contents are placed at the end of file", "N" },
	{ "noline", 'n', 0, G_OPTION_ARG_NONE, &noline, "Rewrites the graphic file without row numbers", NULL },
	{ "empty", 'e', 0, G_OPTION_ARG_NONE, &empty, "Flush the contents file", NULL },
	{ NULL }
};

static gboolean write_default_configuration (GError **error) {
	gboolean ret;
	gchar *c;
	GKeyFile *conf;
	GFile *file;
	GFileOutputStream *stream;

	if (output_file == NULL)
		output_file = g_build_filename (g_get_user_config_dir (), "wallmemo", "wallpaper.png", NULL);

	conf = g_key_file_new ();

	g_key_file_set_string (conf, "Image", "bgcolor", bg_color);
	g_key_file_set_comment (conf, "Image", "bgcolor", "Background color of the image, in exadecimal format (e.g. #FF00FF)", NULL);

	g_key_file_set_integer (conf, "Image", "width", width);
	g_key_file_set_comment (conf, "Image", "width", "Width of the image, in pixels", NULL);

	g_key_file_set_integer (conf, "Image", "height", height);
	g_key_file_set_comment (conf, "Image", "height", "height of the image, in pixels", NULL);

	g_key_file_set_string (conf, "Image", "fgcolor", fg_color);
	g_key_file_set_comment (conf, "Image", "fgcolor", "Color for text, in exadecimal format (e.g. #FF00FF)", NULL);

	g_key_file_set_string (conf, "Image", "fgcolor", fg_color);
	g_key_file_set_comment (conf, "Image", "fgcolor", "Color for text, in exadecimal format (e.g. #FF00FF)", NULL);

	g_key_file_set_string (conf, "Image", "output", output_file);
	g_key_file_set_comment (conf, "Image", "output", "Absolute path of the PNG output graphic file to be used as wallpaper", NULL);

	c = g_key_file_to_data (conf, NULL, NULL);
	file = g_file_new_for_path (conf_file);

	ret = TRUE;

	stream = g_file_replace (file, NULL, FALSE, G_FILE_CREATE_REPLACE_DESTINATION, NULL, error);
	if (stream == NULL) {
		ret = FALSE;
	}
	else {
		if (g_output_stream_write (G_OUTPUT_STREAM (stream), c, strlen (c), NULL, error) == -1)
			ret = FALSE;
		g_object_unref (stream);
	}

	g_free (c);
	g_key_file_free (conf);
	return ret;
}

static gboolean read_configuration (GError **error) {
	gchar *dir;
	GKeyFile *conf;

	if (conf_file == NULL)
		conf_file = g_build_filename (g_get_user_config_dir (), "wallmemo", "config", NULL);

	/*
		TODO	Report error if unable to find or create the folder
	*/
	dir = g_path_get_dirname (conf_file);
	g_mkdir_with_parents (dir, 0777);
	g_free (dir);

	if (init == TRUE) {
		return write_default_configuration (error);
	}
	else {
		conf = g_key_file_new ();
		if (g_key_file_load_from_file (conf, conf_file, G_KEY_FILE_NONE, NULL) == FALSE)
			return write_default_configuration (error);

		bg_color = g_key_file_get_string (conf, "Image", "bgcolor", error);
		if (bg_color == NULL)
			return FALSE;

		fg_color = g_key_file_get_string (conf, "Image", "fgcolor", error);
		if (fg_color == NULL)
			return FALSE;

		width = g_key_file_get_integer (conf, "Image", "width", error);
		if (width == 0)
			return FALSE;

		height = g_key_file_get_integer (conf, "Image", "height", error);
		if (height == 0)
			return FALSE;

		output_file = g_key_file_get_string (conf, "Image", "output", error);
		if (output_file == NULL)
			return FALSE;

		return TRUE;
	}
}

static gboolean read_contents (GList **contents, GError **error) {
	gboolean ret;
	gchar *dir;
	gchar *row;
	GList *l;
	GFile *file;
	GFileInputStream *stream;
	GDataInputStream *dstream;

	if (content_file == NULL)
		content_file = g_build_filename (g_get_user_config_dir (), "wallmemo", "contents", NULL);

	/*
		TODO	Report error if unable to find or create the folder
	*/
	dir = g_path_get_dirname (content_file);
	g_mkdir_with_parents (dir, 0777);
	g_free (dir);

	ret = TRUE;

	file = g_file_new_for_path (content_file);
	if (g_file_query_exists (file, NULL) == TRUE) {
		stream = g_file_read (file, NULL, error);

		if (stream == NULL) {
			ret = FALSE;
		}
		else {
			if (empty == FALSE) {
				l = NULL;

				dstream = g_data_input_stream_new (G_INPUT_STREAM (stream));
				for (row = g_data_input_stream_read_line (dstream, NULL, NULL, error); row; row = g_data_input_stream_read_line (dstream, NULL, NULL, error))
					l = g_list_prepend (l, row);

				g_object_unref (dstream);

				if (l != NULL)
					l = g_list_reverse (l);

				*contents = l;
			}

			g_object_unref (stream);
		}
	}

	g_object_unref (file);
	return ret;
}

static GList* delete_row (GList *contents, int row) {
	GList *cursor;

	cursor = g_list_nth (contents, row);
	if (cursor != NULL) {
		g_free (cursor->data);
		contents = g_list_delete_link (contents, cursor);
	}

	return contents;
}

static GList* pre_contents_management (GList *contents) {
	if (delete != -1)
		contents = delete_row (contents, delete);

	if (replace != -1) {
		contents = delete_row (contents, replace);
		position = replace;
	}

	return contents;
}

static void write_graphic_file (GList *contents) {
	int y;
	int line;
	gchar *row;
	GList *cursor;
	GString *text;
	RsvgHandle *handle;
	GdkPixbuf *pixbuf;

	handle = rsvg_handle_new ();
	rsvg_handle_set_dpi (handle, 90);

	text = g_string_new ("");
	g_string_append_printf (text, "<svg \
		xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" \
		width=\"%d\" height=\"%d\" \
		id=\"svg2\" version=\"1.1\" >\n\
		<defs id=\"defs4\" />\n", width, height);

	g_string_append_printf (text, "<g id=\"layer1\" transform=\"translate(0,-284.36217)\"> \
		<rect style=\"fill:%s;fill-opacity:1;stroke:none\" id=\"rect3027\" \
		width=\"%d\" height=\"%d\" \
		x=\"0\" y=\"284.36218\" ry=\"0\" />\n", bg_color, width, height);

	y = 100;
	line = 0;

	for (cursor = contents; cursor; cursor = cursor->next) {
		if (noline == TRUE)
			row = g_strdup (cursor->data);
		else
			row = g_strdup_printf ("%d) %s", line, (gchar*) cursor->data);

		g_string_append_printf (text, "<text \
			xml:space=\"preserve\" \
			style=\"font-size:30px;font-style:normal;font-weight:normal;line-height:125%%;letter-spacing:0px;word-spacing:0px;fill:%s;fill-opacity:1;stroke:none;font-family:Sans\" \
			x=\"150.00000\" \
			y=\"%d\" \
			id=\"text%d\" \
			transform=\"translate(0,284.36217)\"><tspan \
			id=\"tspan2987\" \
			x=\"150.00000\" \
			y=\"%d\">%s</tspan></text>", fg_color, y, line, y, row);

		line++;
		y += 40;
		g_free (row);
	}

	g_string_append_printf (text, "</g></svg>");

	row = g_string_free (text, FALSE);
	rsvg_handle_write (handle, row, strlen (row), NULL);
	rsvg_handle_close (handle, NULL);
	pixbuf = rsvg_handle_get_pixbuf (handle);
	gdk_pixbuf_save (pixbuf, output_file, "png", NULL, NULL);
	g_free (row);
}

static void set_wallpaper () {
	gchar *output_uri;
	GSettings *setting;
	GSettingsSchemaSource *schema_source;
	GSettingsSchema *schema;

	schema_source = g_settings_schema_source_ref (g_settings_schema_source_get_default ());
	schema = g_settings_schema_source_lookup (schema_source, "org.gnome.desktop.background", TRUE);
	setting = g_settings_new_full (schema, NULL, NULL);

	output_uri = g_strdup_printf ("file://%s", output_file);
	g_settings_set_string (setting, "picture-uri", output_uri);
	g_free (output_uri);

	g_object_unref (setting);
	g_settings_schema_unref (schema);
	g_settings_schema_source_unref (schema_source);
}

/*
	Warning: this function provides to also free the "contents" GList
*/
static gboolean write_contents_file (GList *contents, GError **error) {
	gboolean ret;
	gchar *c;
	GList *cursor;
	GString *text;
	GFile *file;
	GFileOutputStream *stream;

	ret = TRUE;

	file = g_file_new_for_path (content_file);

	stream = g_file_replace (file, NULL, FALSE, G_FILE_CREATE_REPLACE_DESTINATION, NULL, error);
	if (stream == NULL) {
		for (cursor = contents; cursor; cursor = cursor->next)
			g_free (cursor->data);

		ret = FALSE;
	}
	else {
		text = g_string_new ("");

		for (cursor = contents; cursor; cursor = cursor->next) {
			g_string_append_printf (text, "%s\n", (gchar*) cursor->data);
			g_free (cursor->data);
		}

		c = g_string_free (text, FALSE);

		if (g_output_stream_write (G_OUTPUT_STREAM (stream), c, strlen (c), NULL, error) == -1)
			ret = FALSE;

		g_object_unref (stream);
		g_free (c);
	}

	g_list_free (contents);
	return ret;
}

int main (int argc, char **argv) {
	int i;
	GError *error;
	GList *contents;
	GOptionContext *context;

	g_type_init ();

	error = NULL;

	g_set_application_name ("wallmemo");
	context = g_option_context_new ("- desktop note taking utility");

	g_option_context_add_main_entries (context, entries, NULL);
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_print ("Option parsing failed: %s\n", error->message);
		g_error_free (error);
		exit (1);
	}

	if (read_configuration (&error) == FALSE) {
		g_print ("Configuration parsing failed: %s\n", error->message);
		g_error_free (error);
		exit (1);
	}

	contents = NULL;

	if (read_contents (&contents, &error) == FALSE) {
		g_print ("Contents parsing failed: %s\n", error->message);
		g_error_free (error);
		exit (1);
	}

	contents = pre_contents_management (contents);

	for (i = 1; i < argc; i++) {
		contents = g_list_insert (contents, g_strdup (argv [i]), position);
		if (position != -1)
			position++;
	}

	write_graphic_file (contents);
	set_wallpaper ();

	if (write_contents_file (contents, &error) == FALSE) {
		g_print ("Saving contents failed: %s\n", error->message);
		g_error_free (error);
		exit (1);
	}

	exit (0);
}
