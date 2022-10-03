#include "btree.h"

int main(){

	int reg[] = {10,2,7,5,3,9,16,4,11,1,6,23,14};
	
	Record r;
  	Pagina *btree = CreateBTree();

  	for(int i=0; i<13; i++){
    	r.key = reg[i];
    	Insere(&btree, r);
  	}

  	Imprime(&btree, 0);
	
	return 0;

}