#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include "matrixf.h"
#include "listmf.h"

int varGlobal = 0;
pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;

matrixf *grayScale(png_bytep *row_pointers, int height, int width) {
  matrixf *mf = createMF(height, width);
  for(int y = 0; y < height; y++) {
    png_bytep row = row_pointers[y];
    for(int x = 0; x < width; x++) {
      png_bytep px = &(row[x * 4]);
	  float prom = px[0]*0.299+px[1]*0.587+px[2]*0.114;
	  mf = setDateMF(mf, y, x, prom);
    }
  }
  return mf;
}

matrixf* leerPNG(char *nombre, matrixf *mf, int width, int height, png_byte color_type,
  png_byte bit_depth, png_bytep *row_pointers) {
  FILE *archivo = fopen(nombre, "rb");
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info = png_create_info_struct(png);
  png_init_io(png, archivo);
  png_read_info(png, info);
  width      = png_get_image_width(png, info);
  height     = png_get_image_height(png, info);
  color_type = png_get_color_type(png, info);
  bit_depth  = png_get_bit_depth(png, info);
  png_read_update_info(png, info);
  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
  }
  png_read_image(png, row_pointers);
  fclose(archivo);
  png_destroy_read_struct(&png, &info, NULL);
  mf = grayScale(row_pointers, height, width);
  return mf;
  //return lf;
}

matrixf *bidirectionalConvolution(matrixf *mf, matrixf *filter){
	if ((countFil(filter) == countColumn(filter))&&(countFil(filter)%2 == 1)){
		int increase = 0, initial = countFil(filter);
		while (initial != 1){
			initial = initial - 2;
			increase = increase + 1;
		}
		for (int cont = 0; cont < increase; cont++){
			mf = amplifyMF(mf);
		}
		for (int fil = 0; fil < countFil(filter) - countFil(filter); fil++){
			for (int col = 0; col < countColumn(filter) - countColumn(filter); col++){
				float sum = 0.0000;
				for (int y = 0; y < countFil(filter); y++){
					for (int x = 0; x < countColumn(filter); x++){
						float result = getDateMF(mf, y, x)*getDateMF(filter, y + fil, x + col);
						sum = sum + result;
					}
				}
				mf = setDateMF(mf, fil + (countFil(filter)/2), col + (countColumn(filter)/2), sum);
			}
		}
		for (int cont2 = 0; cont2 < increase; cont2++){
			mf = decreaseMF(mf);
		}
		return mf;
	}
	else{
		return mf;
	}
}

matrixf *rectification(matrixf *mf){
	for (int y = 0; y < countFil(mf); y++){
		for (int x = 0; x < countColumn(mf); x++){
			if (getDateMF(mf,y,x) < 0.0000){
				mf = setDateMF(mf, y, x, 0.0000);
			}
			if (getDateMF(mf,y,x) > 255.0000){
				mf = setDateMF(mf, y, x, 255.0000);
			}
		}
	}
	return mf;
}

matrixf *pooling(matrixf *mf){
	int heigth = 0, width = 0;
	if (countFil(mf)%2 == 0){
		heigth = countFil(mf)/2;
	}
	if (countFil(mf)%2 == 1){
		heigth = (countFil(mf)/2) + 1;
	}
	if (countColumn(mf)%2 == 0){
		width = countColumn(mf)/2;
	}
	if (countColumn(mf)%2 == 1){
		width = (countColumn(mf)/2) + 1;
	}
	matrixf *newmf = createMF(heigth, width);
	int fil = 0, col = 0, fil2 = 0, col2 = 0;
	while (fil < countFil(mf)){
		col2 = 0;
		while (col < countColumn(mf)){
			float max = 0.0000;
			for (int y = 0; y < 2; y++){
				for (int x = 0; x < 2; x++){
					if (max > getDateMF(mf, y, x)){
						max = getDateMF(mf, y, x);
					}
				}
			}
			newmf = setDateMF(newmf, fil2, col2, max);
			col = col + 2;
			col2 = col2 + 1;
		}
		fil = fil + 2;
		fil2 = fil2 + 1;
	}
	return newmf;
}

void escribirPNG(char *filename, matrixf *mf) {
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

void classification(matrixf *mf, int umbral, char *namefile){
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
/**/
matrixf *convertFilter(char **datefilter, int cont){
	int colfilter = 1;
	for (int x = 0; x < strlen(datefilter[0]); x++){
		if ((datefilter[0][x] == '-') || (datefilter[0][x] == '0') || (datefilter[0][x] == '1') 
			|| (datefilter[0][x] == '2') || (datefilter[0][x] == '3') || (datefilter[0][x] == '4') 
		|| (datefilter[0][x] == '5') || (datefilter[0][x] == '6') || (datefilter[0][x] == '7') 
		|| (datefilter[0][x] == '8') || (datefilter[0][x] == '9')){
			colfilter = colfilter + 0;
		}
		else if (datefilter[0][x] == ' '){
			colfilter = colfilter + 1;
		}
	}
	matrixf *filter = createMF(cont, colfilter);
	int fil = 0, col = 0, pos = 0;
	char *digit = (char *)malloc(10*sizeof(char));
	for (int a = 0; a < cont; a++){
		col = 0;
		for (int b = 0; b < strlen(datefilter[a]); b++){
			if ((datefilter[a][b] == '-') || (datefilter[a][b] == '0') || (datefilter[a][b] == '1') 
			    || (datefilter[a][b] == '2') || (datefilter[a][b] == '3') || (datefilter[a][b] == '4') 
		        || (datefilter[a][b] == '5') || (datefilter[a][b] == '6') || (datefilter[a][b] == '7') 
		        || (datefilter[a][b] == '8') || (datefilter[a][b] == '9')){
			digit[pos] = datefilter[a][b];
			if (b == strlen(datefilter[a]) - 1){
				pos = 0;
				filter = setDateMF(filter, fil, col, (atoi(digit)) * 1.0000);
				col = col + 1;
				digit = (char *)malloc(10*sizeof(char));
			}
			pos = pos + 1;
			}
			else if((datefilter[a][b] == ' ') || (b == strlen(datefilter[a]) - 1)){
				pos = 0;
				filter = setDateMF(filter, fil, col, (atoi(digit)) * 1.0000);
				col = col + 1;
				digit = (char *)malloc(10*sizeof(char));
			}
		}
		fil = fil + 1;
	}
	return filter;
}


// Funcion main: Funcion que toma por parametros los datos entrantes y pasa a la etapa de lectura,
//la matriz del filtro para convulocion y el nombre de las imagenes.
// 
// Entrada: los parametros ingresados por el usuario.
// Salida: Entero que representa fin de su ejecucion.

int main(int argc, char *argv[]){

    char *cflag = (char*)malloc(100*sizeof(char));
    char *mflag = (char*)malloc(100*sizeof(char));
    char *nflag = (char*)malloc(100*sizeof(char));
	char *hflag = (char*)malloc(100*sizeof(char));
	char *tflag = (char*)malloc(100*sizeof(char));
    int numeroImagenes=0;
	int numeroHebras=0;
	int largoBuffer=0;
    int umbralClasificacion=0;

    int pUmbral[2];
    int pNombre[2];
    int status;
    pid_t pid;

    
    int caso, aux=0;
    while((caso=getopt(argc,argv, "c:m:n:h:t:b"))!= -1){
        switch(caso){
            case 'c':
        
                strcpy(cflag, optarg); /*Numero Cantidad imagenes*/
        
                break;    
            case 'm':
                strcpy(mflag, optarg); /*Archivo mascara filtro .txt*/
                break;  
            
            case 'n':
                strcpy(nflag, optarg); /*Numero Umbral clasificacion*/
                break;
				
			case 'h':
                strcpy(hflag, optarg); /*Numero Cantidad hebras*/
                break;
			
			case 't':
                strcpy(tflag, optarg); /*Numero Largo Buffer*/
                break;
        
            case 'b': /*Se muestra o no por pantalla*/
                aux=1;
                break;
             default:
                abort();
            
            
        }   
    }
	char **datefilter = (char **)malloc(2000*sizeof(char *));
	char *date = (char *)malloc(2000*sizeof(char));
	FILE *filefilter = fopen(mflag,"r");
	int error = 0, cont = 0;
	while(error == 0){
		fseek(filefilter, 0, SEEK_END);
		if ((filefilter == NULL) || (ftell(filefilter) == 0)){
			perror("Error en lectura. Ingrese el nombre de un archivo existente.\n");
			error = 1;
		}
		else{
			date=(char*)malloc(2000*sizeof(char));
			rewind(filefilter);
			while(feof(filefilter) == 0){
				date = fgets(date, 1000, filefilter);
				datefilter[cont] = date;
				date = (char*)malloc(1000*sizeof(char));
				cont = cont + 1;
			}
			error = 1;
		}
	}
	rewind(filefilter);
	fclose(filefilter);
	matrixf *filter = convertFilter(datefilter, cont);
    numeroImagenes = atoi(cflag);
	numeroHebras = atoi(hflag);
	largoBuffer = atoi(tflag);
  	umbralClasificacion = atoi(nflag);
	/*int pDateMatrix[2];
	int pFilMatrix[2];
	int pColMatrix[2];*/
	listmf *buffer = createArrayListMF(largoBuffer);
	pthread_t *hebrasConsumidoras = (pthread_t *)malloc(numeroHebras*sizeof(pthread_t));
  	printf("\n|     Imagen     |     Nearly Black     |\n");
  	for(int image = 1; image <= numeroImagenes; image++){
	    /*char cantidadImg[10];
	    sprintf(cantidadImg,"%d",image); 
	    char *nombreFiltroConvolucion= mflag;
	    char imagenArchivo[] = "imagen_"; 
	    char extension[] = ".png"; 
	    strcat(imagenArchivo,cantidadImg); 
	    strcat(imagenArchivo,extension); 
	    pipe(pNombre);
	    pipe(pUmbral);
	    pipe(pDateMatrix);
		pipe(pFilMatrix);
		pipe(pColMatrix);
	    pid = fork();
	    if(pid>0){
	    	close(pNombre[0]);
	      	write(pNombre[1],imagenArchivo,(strlen(imagenArchivo)+1));
	      	close(pUmbral[0]);
	      	write(pUmbral[1],umbralClasificacion,sizeof(umbralClasificacion));
			close(pDateMatrix[0]);
			close(pFilMatrix[0]);
			close(pColMatrix[0]);
			int filmatrix = countFil(filter);
			int colmatrix = countColumn(filter);
			write(pFilMatrix[1], &filmatrix, sizeof(filmatrix));
			write(pColMatrix[1], &colmatrix, sizeof(colmatrix));
			for (int y = 0; y < countFil(filter); y++){
				for (int x = 0; x < countColumn(filter); x++){
					float datematrix = getDateMF(filter, y, x);
					write(pDateMatrix[1], &datematrix, sizeof(datematrix));
				}
			}
	      	waitpid(pid,&status,0);

	    }else{
	      	close(pNombre[1]);
	      	dup2(pNombre[0],3);
	      	close(pUmbral[1]);
	      	dup2(pUmbral[0],4);
			
			close(pDateMatrix[1]);
	      	dup2(pDateMatrix[0], 7);
			close(pFilMatrix[1]);
	      	dup2(pFilMatrix[0], 8);
			close(pColMatrix[1]);
	      	dup2(pColMatrix[0], 9);
	      	
	      	char *argvHijo[] = {"lectura",NULL};
	      	execv(argvHijo[0],argvHijo);
	    }*/
  	}
}

