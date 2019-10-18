# pragma once
struct matrixf{
	float digit;
	struct matrixf *up;
	struct matrixf *down;
	struct matrixf *left;
	struct matrixf *right;
};
typedef struct matrixf matrixf;

matrixf *rightMF(matrixf *mf){
	matrixf *leftmf = mf;
	mf = mf->right;
	leftmf->right = mf;
	mf->left = leftmf;
	return mf;
}

matrixf *downMF(matrixf *mf){
	matrixf *upmf = mf;
	mf = mf->down;
	upmf->down = mf;
	mf->up = upmf;
	return mf;
}

matrixf *startLeftMF(matrixf *mf){
	while (mf->left != NULL){
		mf = mf->left;
	}
	return mf;
}

matrixf *startUpMF(matrixf *mf){
	while (mf->up != NULL){
		mf = mf->up;
	}
	return mf;
}

matrixf *createMF(int height, int width){
	matrixf *mf = (matrixf *)malloc(sizeof(matrixf));
	mf->digit = 0.0000;
	mf->up = NULL;
	mf->down = NULL;
	mf->left = NULL;
	mf->right = NULL;
	for (int cont = 0; cont < width - 1; cont++){
		matrixf *auxmf = (matrixf *)malloc(sizeof(matrixf));
		auxmf->up = NULL;
		auxmf->down = NULL;
		auxmf->left = NULL;
		auxmf->right = NULL;
		auxmf->digit = 0.0000;
		mf->right = auxmf;
		auxmf->left = mf;
		mf = rightMF(mf);
	}
	mf = startLeftMF(mf);
	for (int fil = 0; fil < height - 1; fil++){
		matrixf *newmf = (matrixf *)malloc(sizeof(matrixf));
		newmf->up = NULL;
		newmf->down = NULL;
		newmf->left = NULL;
		newmf->right = NULL;
		newmf->digit = 0.0000;
		for (int col = 0; col < width - 1; col++){
			matrixf *auxmf = (matrixf *)malloc(sizeof(matrixf));
			auxmf->up = NULL;
			auxmf->down = NULL;
			auxmf->left = NULL;
			auxmf->right = NULL;
			auxmf->digit = 0.0000;
			newmf->right = auxmf;
			auxmf->left = newmf;
			newmf = rightMF(newmf);
		}
		newmf = startLeftMF(newmf);
		mf->down = newmf;
		newmf->up = mf;
		mf = downMF(mf);
	}
	mf = startLeftMF(mf);
	mf = startUpMF(mf);
	return mf;
}

matrixf *setDateMF(matrixf *mf, int fil,int col, float date){
	if (fil < 0){
		fil = 0;
	}
	if (col < 0){
		col = 0;
	}
	int x = 0, y = 0;
	while ((y < fil) && (mf->down != NULL)){
		mf = downMF(mf);
		y = y + 1;
	}
	while ((x < col) && (mf->right != NULL)){
		mf = rightMF(mf);
		x = x + 1;
	}
	mf->digit = date;
	mf = startLeftMF(mf);
	mf = startUpMF(mf);
	return mf;
}

float getDateMF(matrixf *mf, int fil,int col){
	if (fil < 0){
		fil = 0;
	}
	if (col < 0){
		col = 0;
	}
	int x = 0, y = 0;
	while ((y < fil) && (mf->down != NULL)){
		mf = downMF(mf);
		y = y + 1;
	}
	while ((x < col) && (mf->right != NULL)){
		mf = rightMF(mf);
		x = x + 1;
	}
	float date = mf->digit;
	mf = startLeftMF(mf);
	mf = startUpMF(mf);
	return date;
}

int countFil(matrixf *mf){
	int length = 1;
	while (mf->down != NULL){
		length = length + 1;
		mf = downMF(mf);
	}
	mf = startUpMF(mf);
	return length;
}

int countColumn(matrixf *mf){
	int length = 1;
	while (mf->right != NULL){
		length = length + 1;
		mf = rightMF(mf);
	}
	mf = startLeftMF(mf);
	return length;
}

matrixf *amplifyMF(matrixf *mf){
	matrixf *newMF = createMF(countFil(mf) + 2, countColumn(mf) + 2);
	for (int fil = 0; fil < countFil(mf); fil++){
		for (int col = 0; col < countColumn(mf); col++){
			newMF = setDateMF(newMF, fil + 1, col + 1, getDateMF(mf, fil, col));
		}
	}
	return newMF;
}

matrixf *decreaseMF(matrixf *mf){
	matrixf *newMF = createMF(countFil(mf) - 2, countColumn(mf) - 2);
	for (int fil = 0; fil < countFil(newMF); fil++){
		for (int col = 0; col < countColumn(newMF); col++){
			newMF = setDateMF(newMF, fil, col, getDateMF(mf, fil + 1, col + 1));
		}
	}
	return newMF;
}