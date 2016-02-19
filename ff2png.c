/**
 * this code equals the logic from farbfeld v0.9, 
 * unless that this is not a standalone-tool anymore
 * 
 * based on the code from farbfeld:
 * (c) 2014-2016 Laslo Hunhold <dev@frign.de>
 * Thank you for sharing your code!
 */

#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

#define HDR "farbfeld########"

static char *argv0;

void
pngerr(png_structp pngs, const char *msg)
{
	fprintf(stderr, "%s: libpng: %s\n", argv0, msg);
}

/**
 * 
 * @param f must be a valid farbfeld-file
 * @return the path to the temp-png-file
 */
char*
ff2png(char *f)
{
	FILE *ffFile = fopen(f, "r");
	png_structp pngs;
	png_infop pngi;
	size_t rowlen;
	uint32_t width, height, i;
	uint16_t *row;
	uint8_t hdr[16];
	char *pngFile = calloc(200, sizeof(char));
	snprintf(pngFile, 200, "/tmp/ffviewer-%lu", time(NULL));

	FILE *fp = fopen(pngFile, "w");
	if (fp == NULL || ffFile == NULL) {
		return NULL;
	}
	
	// read header
	if (fread(hdr, 1, sizeof(HDR) - 1, ffFile) != sizeof(HDR) - 1) {
		return NULL;
	}

	width = ntohl(*((uint32_t *) (hdr + 8)));
	height = ntohl(*((uint32_t *) (hdr + 12)));

	/* load png */
	pngs = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, pngerr,
		NULL);
	pngi = png_create_info_struct(pngs);

	if (!pngs || !pngi) {
		fprintf(stderr, "%s: failed to initialize libpng\n", argv0);
		return NULL;
	}
	png_init_io(pngs, fp);
	png_set_IHDR(pngs, pngi, width, height, 16, PNG_COLOR_TYPE_RGB_ALPHA,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
		PNG_FILTER_TYPE_BASE);
	png_write_info(pngs, pngi);

	/* write rows */
	rowlen = (sizeof("RGBA") - 1) * width;
	if (!(row = malloc(rowlen * sizeof(uint16_t)))) {
		return NULL;
	}
	for (i = 0; i < height; ++i) {
		if (fread(row, sizeof(uint16_t), rowlen, ffFile) != rowlen) {
			goto readerr;
		}
		png_write_row(pngs, (uint8_t *) row);
	}
	png_write_end(pngs, NULL);
	png_destroy_write_struct(&pngs, NULL);
	fclose(fp);
	fclose(ffFile);
	return pngFile;
readerr:
	fclose(fp);
	fclose(ffFile);
	return NULL;
}
