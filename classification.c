#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <png.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "matrixf.h"

// Funcion: EScribe un archivo en formato png, resultante
// 
// Entrada: en nombre del archivo y lamatriz resultante.
// Salida: void
void escribirPNG(char *filename, matrixF *mf) {
  FILE *filepng = fopen(filename, "wb");
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info = png_create_info_struct(png);
  png_init_io(png, filepng);
  png_set_IHDR(
    png,
    info,
    countColumn(mf), countFil(mf),
    8,
    PNG_COLOR_TYPE_RGB,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_BASE,
    PNG_FILTER_TYPE_BASE
  );
  png_text title_text;
  title_text.compression = PNG_TEXT_COMPRESSION_NONE;
  title_text.key = filename;
  title_text.text = filename;
  png_set_text(png, info, &title_text, 1);
  png_write_info(png, info);  
  png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * countFil(mf));
  for(int y = 0; y < countFil(mf); y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
  }
  for(int y = 0; y < countFil(mf); y++) {
    for(int x = 0; x < countColumn(mf); x++) {
	  (&(row_pointers[y][x * 4]))[0] = (int)getDateMF(mf, y, x);
	  (&(row_pointers[y][x * 4]))[1] = (int)getDateMF(mf, y, x);
	  (&(row_pointers[y][x * 4]))[2] = (int)getDateMF(mf, y, x);
	  png_write_row(png, (&(row_pointers[y][x * 4])));
    }
  } 
  png_write_image(png, row_pointers);
  png_write_end(png, NULL); 
  for(int y = 0; y < countFil(mf); y++) {
    free(row_pointers[y]);
  }
  free(row_pointers);
  fclose(filepng);
  png_destroy_write_struct(&png, &info);
}


// Funcion: Permite clasificar una imagene de acuerdo a un umbral
// 
// Entrada: Matriz resultante desde etapa de pooling, umbral ingresado por usuario y el nombre d ela imagen.
// Salida: void

void classification(matrixF *mf, int umbral, char *namefile){
	int maxBlack = 0;
	for (int y = 0; y < countFil(mf); y++){
		for (int x = 0; x < countColumn(mf); x++){
			if (getDateMF(mf, y, x) == 0.0000){
				maxBlack = maxBlack + 1;
			}
		}
	}
	float porcentBlack = (maxBlack * 100.0000)/(countFil(mf) * countColumn(mf));
	if (porcentBlack >= umbral){
		printf("|   %s   |         yes        |\n",namefile);
	}
	if (porcentBlack < umbral){
		printf("|   %s   |         no         |\n",namefile);
	}
	strcat(namefile,"R.png");
	escribirPNG(namefile, mf);
}


int main(int argc, char *argv[]){
  /* matrixf clasfication;
  aqui iria la matriz para guardar el clasification*/ 
  matrixF *entrada;
  matrixF *salida;
  
  int fil, col;
  float date;

  char imagenArchivo[40]; /*Nombre del archivo imagen_1.png*/
  int umbralClasificacion[1]; /*numero del umbral*/

  pid_t pid;
  int status;


  int pUmbral[2]; /*para pasar el umbral para clasificacion*/
  int pNombre[2]; /*Para pasar nombre imagen_1.png*/
  //int pFiltroConvolucion[2]; /*para pasar filtro.txt*/
  int pImagen[2]; /*para pasar la imagen de pooling*/
  /*Se crean los pipes*/
  //pipe(pFiltroConvolucion);
  pipe(pUmbral);
  pipe(pNombre);
  pipe(pImagen);

  
  


  read(3,imagenArchivo,sizeof(imagenArchivo));
  read(4,umbralClasificacion,sizeof(umbralClasificacion));
  /*falta aqui read de la imagen desde pooling*/
  /*read(5, entrada,sizeof(matrixF) );*/
  read(8, &fil, sizeof(fil));
  read(9, &col, sizeof(col));
  entrada = createMF(fil, col);
  for (int y = 0; y < countFil(entrada); y++){
	for (int x = 0; x < countColumn(entrada); x++){
		read(7, &date, sizeof(date));
		entrada = setDateMF( entrada, y, x, date);
	}
  }
  char *imagefile = (char *)malloc(1000*sizeof(char));
  strncpy(imagefile, imagenArchivo, strlen(imagenArchivo) - 4);
  classification(entrada, umbralClasificacion[0],imagefile);
  return 0;
}
