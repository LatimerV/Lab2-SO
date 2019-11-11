#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <png.h>
#include "matrixf.h"
#include "listmf.h"
#include "funciones.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;
funciones args;

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
  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
	  printf("|%f|",getDateMF(mf, y, x));
    }
	printf("\n");
  }
  return mf;
}

matrixf* readPNG(char *nombre, matrixf *mf, int width, int height, png_byte color_type,
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
		for(int y = 0; y < countFil(mf); y++) {
			for(int x = 0; x < countColumn(mf); x++) {
			  printf("|%f|",getDateMF(mf, y, x));
			}
			printf("\n");
		}
		for (int fil = 0; fil < countFil(mf) - countFil(filter); fil++){
			for (int col = 0; col < countColumn(mf) - countColumn(filter); col++){
				float sum = 0.0000;
				for (int y = 0; y < countFil(filter); y++){
					for (int x = 0; x < countColumn(filter); x++){
						float result = getDateMF(filter, y, x)*getDateMF(mf, y + fil, x + col);
						sum = sum + result;
					}
				}
				mf = setDateMF(mf, fil + (countFil(filter)/2), col + (countColumn(filter)/2), sum);
			}
		}
		for (int cont2 = 0; cont2 < increase; cont2++){
			mf = decreaseMF(mf);
		}
		for(int y = 0; y < countFil(mf); y++) {
			for(int x = 0; x < countColumn(mf); x++) {
			  printf("|%f|",getDateMF(mf, y, x));
			}
			printf("\n");
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
					if (max < getDateMF(mf, y+fil, x+col)){
						max = getDateMF(mf, y+fil, x+col);
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
	free(mf);
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

matrixf *classification(matrixf *mf, int umbral, char *namefile, int maxBlack, int actual, int maxfil){
	pthread_mutex_lock(&mutex2);
	for (int y = 0; y < countFil(mf); y++){
		for (int x = 0; x < countColumn(mf); x++){
			if (getDateMF(mf, y, x) == 0.0000){
				maxBlack = maxBlack + 1;
			}
		}
	}
	pthread_mutex_unlock(&mutex2);
	if (actual == maxfil-1){
		float porcentBlack = (maxBlack * 100.0000)/(countFil(mf) * countColumn(mf));
		if (porcentBlack >= umbral){
			printf("|   %s   |         yes        |\n",namefile);
		}
		if (porcentBlack < umbral){
			printf("|   %s   |         no         |\n",namefile);
		}
		escribirPNG(namefile, mf);
	}
	return mf;
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

void *hebraConsumidora(void* hebras){

	int *hebra = (int *) hebras;
	listmf *buffer = args.buffer;
	listmf *photothread = args.photothread;
	matrixf *filter = args.filter;
	int *datos = args.datos;
	char *imagenSalida=args.imagenSalida;
	printf("nombre %s\n",imagenSalida);
	printf("thread %d\n",*hebra);
	if ((getListMF(photothread,*hebra)==NULL)||
	((getListMF(photothread,*hebra)!=NULL)&&(countFil(getListMF(photothread,*hebra))<datos[2]))){
		pthread_mutex_lock(&mutex);
		matrixf *newmf;
		int maxrow = 0;
		for (int x=0;x<lengthListMF(buffer);x++){
			printf("HIHI %d de hebra %d\n",x,*hebra);
			if (((maxrow == datos[2])&&(*hebra<datos[3]-1))||((maxrow == datos[2]+datos[4])&&(*hebra==datos[3]-1))){
				break;
			}
			else{
				if (getListMF(buffer,x)!=NULL){
					if (getListMF(photothread,*hebra)==NULL){
						newmf = getListMF(buffer,x);
						photothread = setListMF(photothread,newmf,*hebra);
						buffer = setListMF(buffer,NULL,x);
						maxrow = maxrow + 1;
					}
					else{
						newmf = createMF(countFil(getListMF(photothread,*hebra))+1,countColumn(getListMF(buffer,x)));
						int fil = 0;
						for (int y=0;y<countFil(getListMF(photothread,*hebra));y++){
							for (int z=0;z<countColumn(getListMF(photothread,*hebra));z++){
								newmf = setDateMF(newmf,y,z,getDateMF(getListMF(photothread,*hebra),y,z));
							}
							fil = fil + 1;
						}
						for (int y=0;y<countFil(getListMF(buffer,x));y++){
							for (int z=0;z<countColumn(getListMF(buffer,x));z++){
								newmf = setDateMF(newmf,y+fil,z,getDateMF(getListMF(buffer,x),y,z));
							}
						}
						photothread = setListMF(photothread,newmf,*hebra);
						buffer = setListMF(buffer,NULL,x);
						maxrow = maxrow + 1;
					}
				}
			}
		}
		/*if (getListMF(photothread,*hebra)!=NULL){
			for (int y=0;y<countFil(getListMF(photothread,*hebra));y++){
				for (int z=0;z<countColumn(getListMF(photothread,*hebra));z++){
					printf ("(%f)",getDateMF(getListMF(photothread,*hebra),y,z));
				}
				printf("\n");
			}
		}*/
		args.buffer=buffer;
		args.photothread=photothread;
		args.filter=filter;
		args.datos=datos;
		args.imagenSalida=imagenSalida;
		pthread_mutex_unlock(&mutex);
	}
	else if (*hebra < 0){
		int auxhebra = ((*hebra)*-1)-1;
		printf("hola2 (hebra %d)\n",auxhebra);
		pthread_barrier_wait(&barrier);
		matrixf *mf=getListMF(photothread,auxhebra);
		mf = bidirectionalConvolution(mf,filter);
		mf = rectification(mf);
		mf = pooling(mf);
		mf = classification(mf,datos[0],imagenSalida,datos[1],auxhebra,datos[3]);
		photothread = setListMF(photothread,mf,auxhebra);
		args.buffer=buffer;
		args.photothread=photothread;
		args.filter=filter;
		args.datos=datos;
		args.imagenSalida=imagenSalida;
	}
}

// Funcion main: Funcion que toma por parametros los datos entrantes y pasa a la etapa de lectura,
//la matriz del filtro para convulocion y el nombre de las imagenes.
// 
// Entrada: los parametros ingresados por el usuario.
// Salida: Entero que representa fin de su ejecucion.

int main(int argc, char *argv[]){ /*Main principal de la funcion*/

    char *cflag = (char*)malloc(100*sizeof(char));
    char *mflag = (char*)malloc(100*sizeof(char));
    char *nflag = (char*)malloc(100*sizeof(char));
	char *hflag = (char*)malloc(100*sizeof(char));
	char *tflag = (char*)malloc(100*sizeof(char));
    int numeroImagenes=0;
	int numeroHebras=0;
	int largoBuffer=0;
	int umbral=0;
    int caso;
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
                break;
             default:
                abort();          
        }   
    }
	char **datefilter = (char **)malloc(2000*sizeof(char *));
	char *date = (char *)malloc(2000*sizeof(char));
	FILE *filefilter = fopen(mflag,"r");

	//funciones args; /*Se crea una structura de funciones, que contendran los datos necesarios para pasar por parametros a la funcion hebra consumidora;*/


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
  	umbral = atoi(nflag);
	pthread_mutex_init(&mutex,NULL);
	pthread_mutex_init(&mutex2,NULL);
	pthread_barrier_init(&barrier, NULL, numeroHebras);
  	printf("\n|     Imagen     |     Nearly Black     |\n");
  	for(int image = 1; image <= numeroImagenes; image++){
		listmf *buffer = createArrayListMF(largoBuffer); /*Lista de matrices*/
		listmf *photothread = createArrayListMF(numeroHebras);
		matrixf *photomf;
		int width, height, fil, col;
		float date;
		png_byte color_type;
		png_byte bit_depth;
		png_bytep *row_pointers = NULL;
		char cantidadImg[10];
		char cantidadImgSalida[10];
	    sprintf(cantidadImg,"%d",image); /*Pasar de numero a string, cantidadImagen*/
	    sprintf(cantidadImgSalida,"%d",image); /*Pasar de numero a string, cantidadImagen*/
	    char *nombreFiltroConvolucion= mflag;
	    char imagenArchivo[] = "imagen_";
		char imagenSalida[] = "out_";
	    char extension[] = ".png"; 
	    char extension2[] = ".png";
	    strcat(imagenArchivo,cantidadImg);
	    strcat(imagenArchivo,extension); /*imagen_1.png*/
		strcat(imagenSalida,cantidadImgSalida);
		strcat(imagenSalida,extension2); /*out_1.png*/

		//printf("%s\n",imagenSalida);
		
		photomf = readPNG(imagenArchivo, photomf, width, height, color_type, bit_depth, row_pointers); /*Matriz de la imagen*/
		int rowsXthread = countFil(photomf)/numeroHebras; /*Numero de filas por hebra*/
		printf("holahola %d, %d = %d\n",countFil(photomf),numeroHebras,rowsXthread);
		int aditionalRows = countFil(photomf)%numeroHebras; /*Numero de filas adicionales a ultima hebra*/
		int auxumbral = 0;
		int *datos=(int*)malloc(5*sizeof(int));
		datos[0]=umbral;
		datos[1]=auxumbral;
		datos[2]=rowsXthread;
		datos[3]=numeroHebras;
		datos[4]=aditionalRows;
		int *threads =(int*)malloc(numeroHebras*sizeof(int));
		/*Se guardan los datos en la estructura*/
		args.photothread = photothread;
		args.filter = filter;
		args.imagenSalida = imagenSalida;
		args.datos = datos;
		args.buffer = buffer;/*Se guarda el buffer aca, porque aqui se crea*/
		pthread_t *hebrasConsumidoras = (pthread_t *)malloc(numeroHebras*sizeof(pthread_t));
		for (int row=0;row<countFil(photomf);row++){
			matrixf *aux = createMF(1, countColumn(photomf));/*Matriz de una fila de la imagen con tantas columnas, vacia*/
			for (int x=0;x<countColumn(photomf);x++){
				aux=setDateMF(aux,0,x,getDateMF(photomf,row,x));
			}
			args.buffer = setListMF(args.buffer,aux,row%largoBuffer);
			if((fullListMF(args.buffer)==1)||(row==countColumn(photomf)-1)){
				for (int thread=0;thread<numeroHebras;thread++){
					threads[thread]=thread;
					printf("fila de imagen: %d\n",row);
					//pthread_create(hebras, NULL, la funcion que opera, estructura)
					pthread_create(&hebrasConsumidoras[thread],NULL,&hebraConsumidora,(void *)&threads[thread]);
				}
				for (int thread=0;thread<numeroHebras;thread++){
					//pthread_join(hebras, NULL)
					pthread_join(hebrasConsumidoras[thread],NULL);
				}
			}
		}
		free(threads);
		threads =(int*)malloc(numeroHebras*sizeof(int));
		free(hebrasConsumidoras);
		hebrasConsumidoras = (pthread_t *)malloc(numeroHebras*sizeof(pthread_t));
		for (int thread=0;thread<numeroHebras;thread++){
			threads[thread]=(thread+1)*-1;
			//pthread_create(hebras, NULL, la funcion que opera, estructura)
			pthread_create(&hebrasConsumidoras[thread],NULL,&hebraConsumidora,(void *)&threads[thread]);
		}
		for (int thread=0;thread<numeroHebras;thread++){
			//pthread_join(hebras, NULL)
			pthread_join(hebrasConsumidoras[thread],NULL);
		}
  	}
}

