//==============================================================================
// bdf2h.c
//==============================================================================
// Bitmap Distribution Format (BDF) stores a bitmap font. This will convert it t
// o a C header file.
// Uso:
// ./bdf2h < input.bdf > out.h
//==============================================================================


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


void process_bdf(FILE * bdf, FILE * out, char *name);


//==============================================================================
//
//==============================================================================

int main() {
	char *name;
	name = "font";

	process_bdf(stdin, stdout, name);
	return 0;
}


//==============================================================================
// info | defines struct and font info
//==============================================================================
//	out
//	name font variable name in C source file
//	width character width of font
//	height character height of font
//	chars number of characters
//==============================================================================

void info(FILE *out, char *name, int w, int h, int chars) {

	fprintf(out, "// Bitmap font struct def.\n");
	fprintf(out, "typedef struct {\n");
	fprintf(out, "\tint width;\n");
	fprintf(out, "\tint height;\n");
	fprintf(out, "\tint chars;\n");
	fprintf(out, "} bmp_font;\n\n");

	fprintf(out, "bmp_font %s = { %d, %d, %d };\n\n", name, w, h, chars);
}


//==============================================================================
// processChar
//==============================================================================

void processChar(FILE * out, unsigned char *bitmap, int fontwidth,
	int fontheight, int fontyoffset, int charheight, int charyoffset) {

	int x;
	int y;
	int c;

	// Rows from the top of bounding box is the top of character.
	int yoffset = fontheight - charheight + (fontyoffset - charyoffset);

	for (y = 0; y < fontheight; ++y) {
		fputc('\t', out);
		for (x = 0; x < fontwidth; x += 8) {

			// If current row is above or below the bitmap, output a blank row.
			if(y < yoffset || y > yoffset + charheight)
				c = 0;
			else
				c = bitmap[(y - yoffset) * ((fontwidth + 7) / 8) + x / 8];

			if (c & 0x80)
				fputc('#', out);
			else
				fputc('-', out);

			if (c & 0x40)
				fputc('#', out);
			else
				fputc('-', out);

			if (c & 0x20)
				fputc('#', out);
			else
				fputc('-', out);

			if (c & 0x10)
				fputc('#', out);
			else
				fputc('-', out);

			if (c & 0x08)
				fputc('#', out);
			else
				fputc('-', out);

			if (c & 0x04)
				fputc('#', out);
			else
				fputc('-', out);

			if (c & 0x02)
				fputc('#', out);
			else
				fputc('-', out);

			if (c & 0x01)
				fputc('#', out);
			else
				fputc('-', out);

				fputc(',', out);
		}
		fputc('\n', out);
	}
}


//==============================================================================
//
//==============================================================================
// @param p	hex input character (0-9a-fA-F)
//
// @returns converted integer
//==============================================================================

int hex2int(char *p) {
	if (*p <= '9')
		return *p - '0';
	else if (*p <= 'F')
		return *p - 'A' + 10;
	else
		return *p - 'a' + 10;
}

///
///	Rotate bitmap.
///
///	@param bitmap	input bitmap
///	@param shift	rotate counter (0-7)
///	@param width	character width
///	@param height	character height
///
void RotateBitmap(unsigned char *bitmap, int shift, int width, int height)
{
	int x;
	int y;
	int c;
	int o;

	if (shift < 0 || shift > 7) {
	fprintf(stderr, "This shift isn't supported\n");
	exit(-1);
	}

	for (y = 0; y < height; ++y) {
	o = 0;
	for (x = 0; x < width; x += 8) {
		c = bitmap[y * ((width + 7) / 8) + x / 8];
		bitmap[y * ((width + 7) / 8) + x / 8] = c >> shift | o;
		o = c << (8 - shift);
	}
	}
}


//==============================================================================
//	Read BDF font file.
//==============================================================================
//	@param bdf	file stream for input (bdf file)
//	@param out	file stream for output (C source file)
//	@param name	font variable name in C source file
//==============================================================================

void process_bdf(FILE *bdf, FILE *out, char *name) {
	char linebuf[1024];
	char *s;
	char *p;
	int boundingBox_width;
	int boundingBox_height;
	int boundingBox_xoff;
	int boundingBox_yoff;
	int chars;
	int i;
	int j;
	int n;
	int scanline;
	char charname[1024];
	int encoding;
	int bbx;
	int bby;
	int bbw;
	int bbh;
	int width;
	unsigned *width_table;
	unsigned *encoding_table;
	unsigned char *bitmap;

	boundingBox_width = 0;
	boundingBox_height = 0;
	boundingBox_xoff = 0;
	boundingBox_yoff = 0;
	chars = 0;
	for (;;) {
	if (!fgets(linebuf, sizeof(linebuf), bdf)) {	// EOF
		break;
	}
	if (!(s = strtok(linebuf, " \t\n\r"))) {	// empty line
		break;
	}
	// printf("token:%s\n", s);
	if (!strcasecmp(s, "FONTBOUNDINGBOX")) {
		p = strtok(NULL, " \t\n\r");
		boundingBox_width = atoi(p);
		p = strtok(NULL, " \t\n\r");
		boundingBox_height = atoi(p);
		p = strtok(NULL, " \t\n\r");
		boundingBox_xoff = atoi(p);
		p = strtok(NULL, " \t\n\r");
		boundingBox_yoff = atoi(p);
	} else if (!strcasecmp(s, "CHARS")) {
		p = strtok(NULL, " \t\n\r");
		chars = atoi(p);
		break;
	}
	}

	info(out, name, boundingBox_width, boundingBox_height, chars);

	//	Some checks.
	if (boundingBox_width <= 0 || boundingBox_height <= 0) {
	fprintf(stderr, "Need to know the character size\n");
	exit(-1);
	}
	if (chars <= 0) {
	fprintf(stderr, "Need to know the number of characters\n");
	exit(-1);
	}

	//	Allocate tables
	width_table = malloc(chars * sizeof(*width_table));
	if (!width_table) {
	fprintf(stderr, "Out of memory\n");
	exit(-1);
	}
	encoding_table = malloc(chars * sizeof(*encoding_table));
	if (!encoding_table) {
	fprintf(stderr, "Out of memory\n");
	exit(-1);
	}

	bitmap =
	malloc(((boundingBox_width + 7) / 8) * boundingBox_height);
	if (!bitmap) {
	fprintf(stderr, "Out of memory\n");
	exit(-1);
	}

	fprintf(out, "const unsigned char %s_bitmap[] = {\n", name);// Begins array.

	scanline = -1;
	n = 0;
	encoding = -1;
	bbx = 0;
	bby = 0;
	bbw = 0;
	bbh = 0;
	width = INT_MIN;
	strcpy(charname, "unknown character");
	for (;;) {
	if (!fgets(linebuf, sizeof(linebuf), bdf)) {	// EOF
		break;
	}
	if (!(s = strtok(linebuf, " \t\n\r"))) {	// empty line
		break;
	}
	// printf("token:%s\n", s);
	if (!strcasecmp(s, "STARTCHAR")) {
		p = strtok(NULL, " \t\n\r");
		strcpy(charname, p);
	} else if (!strcasecmp(s, "ENCODING")) {
		p = strtok(NULL, " \t\n\r");
		encoding = atoi(p);
	} else if (!strcasecmp(s, "DWIDTH")) {
		p = strtok(NULL, " \t\n\r");
		width = atoi(p);
	} else if (!strcasecmp(s, "BBX")) {
		p = strtok(NULL, " \t\n\r");
		bbw = atoi(p);
		p = strtok(NULL, " \t\n\r");
		bbh = atoi(p);
		p = strtok(NULL, " \t\n\r");
		bbx = atoi(p);
		p = strtok(NULL, " \t\n\r");
		bby = atoi(p);
	} else if (!strcasecmp(s, "BITMAP")) {
		fprintf(out, "// %3d $%02x '%s'\n", encoding, encoding, charname);
		fprintf(out, "//\twidth %d, bbx %d, bby %d, bbw %d, bbh %d\n",
		width, bbx, bby, bbw, bbh);

		if (n == chars) {
		fprintf(stderr, "Too many bitmaps for characters\n");
		exit(-1);
		}
		if (width == INT_MIN) {
		fprintf(stderr, "character width not specified\n");
		exit(-1);
		}
		//
		//	Adjust width based on bounding box
		//
		if (bbx < 0) {
		width -= bbx;
		bbx = 0;
		}
		if (bbx + bbw > width) {
		width = bbx + bbw;
		}

		width_table[n] = width;
		encoding_table[n] = encoding;
		++n;

		scanline = 0;
	
		memset(bitmap, 0,
		((boundingBox_width + 7) / 8) * boundingBox_height);
	} else if (!strcasecmp(s, "ENDCHAR")) {
		if (bbx) {
		RotateBitmap(bitmap, bbx, boundingBox_width,
			boundingBox_height);
		}

		processChar(out, bitmap, boundingBox_width,
		boundingBox_height, boundingBox_yoff, bbh, bby);
		scanline = -1;
		width = INT_MIN;
	} else {
		if (scanline >= 0) {
		p = s;
		j = 0;
		while (*p) {
			i = hex2int(p);
			++p;
			if (*p) {
			i = hex2int(p) | i * 16;
			} else {
			bitmap[j + scanline * ((boundingBox_width +
					7) / 8)] = i;
			break;
			}
			/* printf("%d = %d\n",
			j + scanline * ((boundingBox_width + 7)/8), i); */
			bitmap[j + scanline * ((boundingBox_width + 7) / 8)] =
			i;
			++j;
			++p;
		}
		++scanline;
		}
	}
	}

	fprintf(out, "};\n"); // Cierre de corchete arreglo.


}





