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


typedef struct {
	int bBox_width;		// boundingBox_width
	int bBox_height;	// boundingBox_height
	int bBox_xos;		// boundingBox_xos
	int bBox_yos;		// boundingBox_yos
	int nChars;
} t_bdf_data;

void process_bdf(FILE * bdf, FILE * out, char *fontName);


//==============================================================================
//
//==============================================================================

int main(int argc, char *argv[]) {
	char *fontName = "defaultFontName";
	int flag_h = 0;
	char *out_filename = NULL;
	char *in_filename = NULL;

	FILE * in = stdin;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			flag_h = 1;
		} else if (strcmp(argv[i], "-o") == 0) {
			if (i + 1 < argc) {
				out_filename = argv[i + 1];
				i++; // Skip filename in next iteration.
			} else {
				fprintf(stderr, "Error: -o flag requires a filename.\n");
				return 1;
			}
		} else if (strcmp(argv[i], "-i") == 0) {
			if (i + 1 < argc) {
				in_filename = argv[i + 1];
				i++; // Skip filename in next iteration.
			} else {
				fprintf(stderr, "Error: -i flag requires a filename.\n");
				return 1;
			}
		} else {
			printf("Unknown argument: %s\n", argv[i]);
		}
	}

	if(flag_h)
		printf("Usage: bdf2h");

	if(in_filename != NULL)
		in = fopen(in_filename, "r");
	process_bdf(in, stdout, fontName);
	return 0;
}







//==============================================================================
// info | defines struct and font info
//==============================================================================
// Argumentos:
// -- out = output stream.
// -- fontName
// -- t_bdf_data bdf_data
//==============================================================================

void write_bdf_data(FILE *out, char *name, t_bdf_data bdf_data) {

	fprintf(out, "// Bitmap font info struct def.\n");
	fprintf(out, "typedef struct {\n");
	fprintf(out, "\tint width;\n");
	fprintf(out, "\tint height;\n");
	fprintf(out, "\tint nChars;\n");
	fprintf(out, "} bmp_font_inf;\n\n");

	fprintf(out, "bmp_font_inf %s_inf = { %d, %d, %d };\n\n", name, bdf_data.bBox_width, bdf_data.bBox_height, bdf_data.nChars);
}


//==============================================================================
// write_char
//==============================================================================

void write_char(FILE * out, unsigned char *bitmap, int fontwidth,
	int fontheight, int fontyoffset, int charheight, int charyoffset) {

	int x;
	int y;
	int c;

	// Rows from the top of bounding box is the top of character.
	int yoffset = fontheight - charheight + (fontyoffset - charyoffset);

	for (y = 0; y < fontheight; y++) {
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
// Argummentos:
// -- p = hex char 0-9 a-f A-F
//==============================================================================

int hexChar2int(char *p) {
	if (*p <= '9')
		return *p - '0';
	else if (*p <= 'F')
		return *p - 'A' + 10;
	else
		return *p - 'a' + 10;
}


//==============================================================================
// Returns:
// -- error count.
//==============================================================================

int write_check_errors(t_bdf_data bdf_data) {
	int errVal = 0;

	if (bdf_data.bBox_width <= 0 || bdf_data.bBox_height <= 0) {
		fprintf(stderr, "Boundingbox size error. { w, h } = { %d, %d }\n", bdf_data.bBox_width, bdf_data.bBox_height);
		errVal++;
	}
	if (bdf_data.nChars <= 0) {
		fprintf(stderr, "Number of chars = %d\n", bdf_data.nChars);
		errVal++;
	}

	return errVal;
}


//==============================================================================
//
//==============================================================================

t_bdf_data get_bdf_data(FILE *bdf) {

	char buff[1024];
	char *s, *p;

	t_bdf_data  bdf_data = {
		0,	// bBox_width;
		0,	// bBox_height;
		0,	// bBox_xos;
		0,	// bBox_yos;
		0,	// nChars;
	};
	
	while (1) {
		if (fgets(buff, sizeof(buff), bdf) == NULL)
			break;// EOF.

		s = strtok(buff, " \t\n\r");
		if (s == NULL)
			break;// Empty line.

		if (!strcasecmp(s, "FONTBOUNDINGBOX")) {
			p = strtok(NULL, " \t\n\r");
			bdf_data.bBox_width = atoi(p);
			p = strtok(NULL, " \t\n\r");
			bdf_data.bBox_height = atoi(p);
			p = strtok(NULL, " \t\n\r");
			bdf_data.bBox_xos = atoi(p);
			p = strtok(NULL, " \t\n\r");
			bdf_data.bBox_yos = atoi(p);
		} else if (!strcasecmp(s, "CHARS")) {
			p = strtok(NULL, " \t\n\r");
			bdf_data.nChars = atoi(p);
			break;
		}
	}
	return bdf_data;
}


//==============================================================================
//
//==============================================================================

void get_write_char(FILE *bdf, FILE *out, t_bdf_data bdf_data, unsigned char *bitmap) {

	int i, j;
	char *s, *p;

	char buff[1024];

	int n = 0, scanline = -1;

	char charname[1024];
	int encoding;
	int bbx;
	int bby;
	int bbw;
	int bbh;
	int width;

	encoding = -1;
	bbx = 0;
	bby = 0;
	bbw = 0;
	bbh = 0;
	width = INT_MIN;
	strcpy(charname, "unknown character");

	while (1) {
		if (!fgets(buff, sizeof(buff), bdf))
			break;// EOF.

		if (!(s = strtok(buff, " \t\n\r")))
			break;// Empty line.

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
			fprintf(out, "// %03d 0x%02x '%s'\n", encoding, encoding, charname);
			fprintf(out, "//\twidth %d, bbx %d, bby %d, bbw %d, bbh %d\n",
			width, bbx, bby, bbw, bbh);

			if (n == bdf_data.nChars) {
				fprintf(stderr, "Error: bdf file declares less than actually in file.\n");
				exit(-1);
			}
			if (width == INT_MIN) {
				fprintf(stderr, "Char width not specified.\n");
				exit(-1);
			}

			if (bbx < 0) {
				width -= bbx;//	Width adjustment.
				bbx = 0;
			}
			if (bbx + bbw > width)
				width = bbx + bbw;

			//width_table[n] = width;// No width table, will use monospace.
			//encoding_table[n] = encoding;// Will not use an encoding table.
			n++;

			scanline = 0;
		
			memset(bitmap, 0, ((bdf_data.bBox_width + 7) / 8) * bdf_data.bBox_height);
		} else if (!strcasecmp(s, "ENDCHAR")) {

			write_char(out, bitmap, bdf_data.bBox_width, bdf_data.bBox_height,
				bdf_data.bBox_yos, bbh, bby);
			scanline = -1;
			width = INT_MIN;
		} else {
			if (scanline >= 0) {
				p = s;
				j = 0;
				while (*p) {
					i = hexChar2int(p);
					p++;
					if (*p) {
						i = hexChar2int(p) | i * 16;
					} else {
						bitmap[j + scanline * ((bdf_data.bBox_width + 7) / 8)] = i;
						break;
					}
					bitmap[j + scanline * ((bdf_data.bBox_width + 7) / 8)] = i;
					j++;
					p++;
				}
				scanline++;
			}
		}
	}
}


//==============================================================================
//	process bdf font file.
//==============================================================================
// Recibe:
// -- bdf = stream input bdf file.
// -- out = output stream.
// -- name font variable name in C source file
//==============================================================================

void process_bdf(FILE *bdf, FILE *out, char *name) {

	t_bdf_data  bdf_data;

	//unsigned *width_table;
	//unsigned *encoding_table;
	unsigned char *bitmap;

	bdf_data = get_bdf_data(bdf);
	write_bdf_data(out, name, bdf_data);

	if(write_check_errors(bdf_data) > 0)
		exit(-1); 

	//	Allocate tables
	//width_table = malloc(bdf_data.nChars * sizeof(*width_table));
	//if (width_table == NULL) {
	//	fprintf(stderr, "Malloc allocation failed (width_table var).\n");
	//	exit(-1);
	//}

	//encoding_table = malloc(bdf_data.nChars * sizeof(*encoding_table));
	//if (encoding_table == NULL) {
	//	fprintf(stderr, "Malloc allocation failed (encoding_table var).\n");
	//	exit(-1);
	//}

	bitmap = malloc(((bdf_data.bBox_width + 7) / 8) * bdf_data.bBox_height);
	if (!bitmap) {
		fprintf(stderr, "Malloc allocation failed (bitmap var).\n");
		exit(-1);
	}

	fprintf(out, "const unsigned char %s_bmp[] = {\n", name);// Begins array.


	get_write_char(bdf, out, bdf_data, bitmap);

	fprintf(out, "};\n"); // Cierre de corchete arreglo.
}





