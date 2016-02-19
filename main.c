/* 
 * File:   main.c
 * Author: protodev
 * License: see LICENSE
 *
 * Created on February 18, 2016, 12:20 PM
 */

#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/X.h>
#include <gtk-3.0/gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "main.h"
#include "ff2png.h"

char* isFarbfeldFile(char*);
void close_window(GtkWidget*, gpointer);
gboolean on_key_press(GtkWidget*, GdkEventKey*, gpointer);
void updateImage(void);
int main(int, char**);


GtkWidget *image;
GtkWidget *window;
GtkWidget *titlebar;

char **fileList;
int fileIndex = 0;
int filelistLength = 0;
char* prevTempFile;

/**
 * @param the *file, may be NULL
 * @return *file on success and NULL on failure
 */
char* isFarbfeldFile(char *file)
{
	if (strlen(file) == 0) {
		return NULL;
	}

	FILE *f = fopen(file, "r");

	if (f == NULL) {
		printf("empty file\n");
		return NULL;
	}

	char *ext = strrchr(file, '.');
	char *magic = calloc(8, 1);
	/* read the farbfeld magic */
	if (fread(magic, 1, 8, f) != 8) {
		printf("invalid magic\n");
		free(magic);
		fclose(f);
		return NULL;
	}

	if (ext && strcmp(ext + 1, "ff") == 0 &&
		strcmp(magic, "farbfeld") == 0) {
		free(magic);
		fclose(f);
		return file;
	} else {
		printf("invalid file - skipping\n");
		free(magic);
		fclose(f);
		return NULL;
	}
}

void close_window(GtkWidget *widget, gpointer window)
{
	if (prevTempFile != NULL && strlen(prevTempFile) != 0) {
		unlink(prevTempFile);
	}
	gtk_widget_destroy(window);
	exit(0);
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	switch (event->keyval) {
	case GDK_KEY_Left:
		fileIndex--;
		if (fileIndex < 0) fileIndex = filelistLength - 1;
		updateImage();
		break;
	case GDK_KEY_Right:
		fileIndex++;
		if (fileIndex >= filelistLength) fileIndex--;
		updateImage();
		break;
	case GDK_KEY_q:
		/* fall through */
	case GDK_KEY_Q:
		close_window(NULL, window);
		break;
	}
	return FALSE;
}

void updateImage()
{
	if (fileList == NULL || filelistLength == 0) {
		exit(0);
	}
	char *pngFile = ff2png(fileList[fileIndex]);
	// printf("pngFile: %s\n", pngFile);
	// on click
	gtk_image_set_from_file(GTK_IMAGE(image), pngFile);

	if (prevTempFile != NULL && strlen(prevTempFile) != 0) {
		unlink(prevTempFile);
	}
	prevTempFile = pngFile;
}

int main(int argc, char** argv)
{
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "ffviewer");
	gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
	gtk_window_set_wmclass(GTK_WINDOW(window), "ffviewer", "ffviewer");
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
	g_signal_connect(window, "delete_event", G_CALLBACK(close_window), NULL);
	g_signal_connect(window, "key_press_event", G_CALLBACK(on_key_press), NULL);

	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	image = gtk_image_new();
	gtk_box_pack_start(GTK_BOX(vbox), image, TRUE, TRUE, 0);
	//gtk_container_add(GTK_CONTAINER(window), image);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	fileList = calloc(argc, sizeof(char*));
	int index = 0;
	int i = 1;
	for (; i < argc; i++) {
		char *f = isFarbfeldFile(argv[i]);
		if (f != NULL) {
			fileList[index] = f;
			index++;
		}
	}
	filelistLength = index;

	updateImage();



	gtk_widget_show_all(window);
	gtk_widget_grab_focus(window);
	gtk_main();

	return(EXIT_SUCCESS);
}