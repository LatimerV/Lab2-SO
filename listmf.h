# pragma once
struct listmf{
	matrixf *mf;
	struct listmf *next;
	struct listmf *previous;
};
typedef struct listmf listmf;

listmf *createListMF(matrixf *mf){
	listmf *listmatrixf=(listmf *)malloc(sizeof(listmf));
	listmatrixf->mf=mf;
	listmatrixf->next=NULL;
	listmatrixf->previous=NULL;
	return listmatrixf;
}

listmf *nextListMF(listmf *listmatrixf){
	listmf* previous=listmatrixf;
	listmatrixf=listmatrixf->next;
	previous->next=listmatrixf;
	listmatrixf->previous=previous;
	return listmatrixf;
}

listmf *initialListMF(listmf *listmatrixf){
	while (listmatrixf->previous!=NULL){
		listmatrixf=listmatrixf->previous;
	}
	return listmatrixf;
}

int lengthListMF(listmf *listmatrixf){
	listmf *list_matrixf=listmatrixf;
	int length=0,fin=0;
	if (list_matrixf->mf==NULL){
		return length;
	}
	while(fin==0){
		if(list_matrixf->next==NULL){
			length=length+1;
			fin=1;
		}
		else{
			length=length+1;
			list_matrixf=nextListMF(list_matrixf);
		}
	}
	list_matrixf=initialListMF(list_matrixf);
	return length;
}

listmf *insertListMF(listmf *listmatrixf,matrixf *mf,int pos){
	if (lengthListMF(listmatrixf)==0){
		listmatrixf->mf=mf;
		listmatrixf->next=NULL;
		listmatrixf->previous=NULL;
		return listmatrixf;
	}
	else if (pos==0){
		listmf *newlistmatrixf = createListMF(mf);
		newlistmatrixf->previous=NULL;
		newlistmatrixf->next=listmatrixf;
		listmatrixf->previous=newlistmatrixf;
		listmatrixf=initialListMF(listmatrixf);
		return listmatrixf;
	}
	else if(pos>=lengthListMF(listmatrixf)){
		while (listmatrixf->next!=NULL){
			listmatrixf=nextListMF(listmatrixf);
		}
		listmf *newlistmatrixf = createListMF(mf);
		newlistmatrixf->previous=listmatrixf;
		listmatrixf->next=newlistmatrixf;
		listmatrixf=initialListMF(nextListMF(listmatrixf));
		return listmatrixf;
	}
	else{
		for(int x=0;x<pos-1;x++){
			listmatrixf=nextListMF(listmatrixf);
		}
		listmf *newlistmatrixf = createListMF(mf);
		listmf *auxlistmatrixf=listmatrixf->next;
		newlistmatrixf->previous=listmatrixf;
		newlistmatrixf->next=auxlistmatrixf;
		auxlistmatrixf->previous=newlistmatrixf;
		listmatrixf->next=newlistmatrixf;
		listmatrixf=initialListMF(nextListMF(listmatrixf));
		return listmatrixf;
	}
}

listmf *createArrayListMF(int length){
	listmf *newlistmatrixf = createListMF(NULL);
	while (length>lengthListMF(newlistmatrixf)){
		newlistmatrixf = insertListMF(newlistmatrixf,NULL,0);
	}
	return newlistmatrixf;
}

listmf *setListMF(listmf *listmatrixf, matrixf *mf,int pos){
	if (pos<0){
		pos=0;
	}
	int x=0;
	while ((x<pos)&&(listmatrixf->next!=NULL)){
		listmatrixf=nextListMF(listmatrixf);
		x=x+1;
	}
	listmatrixf->mf=mf;
	listmatrixf=initialListMF(listmatrixf);
	return listmatrixf;
}

matrixf *getListMF(listmf *listmatrixf,int pos){
	if (pos<0){
		pos=0;
	}
	int x=0;
	while ((x<pos)&&(listmatrixf->next!=NULL)){
		listmatrixf=nextListMF(listmatrixf);
		x=x+1;
	}
	matrixf *resultmf=listmatrixf->mf;
	listmatrixf=initialListMF(listmatrixf);
	return resultmf;
}

listmf *copyListMF(listmf *listmatrixf){
	listmf *copylistmatrixf=createListMF(NULL);
	for(int x=0;x<lengthListMF(listmatrixf);x++){
		copylistmatrixf=insertListMF(copylistmatrixf,getListMF(listmatrixf,x),x);
	}
	return copylistmatrixf;
}

int emptyListMF(listmf *listmatrixf){
	int empty = 1;
	for (int i=0;i<lengthListMF(listmatrixf);i++){
		if (getListMF(listmatrixf,i)!=NULL){
			empty = 0;
			break;
		}
	}
	return empty;
}

int fullListMF(listmf *listmatrixf){
	int full = 1;
	for (int i=0;i<lengthListMF(listmatrixf);i++){
		if (getListMF(listmatrixf,i)==NULL){
			full = 0;
			break;
		}
	}
	return full;
}