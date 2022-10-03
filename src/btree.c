#include "btree.h"

Pagina* CreateBTree(){
  return NULL;
}

//Cap 18: página 355 / adaptação para disco
void Pesquisa(Pagina *p, Record *r){ 
  short i = 1;

  if(p == NULL){
    printf("[ERROR]: Node not found!");
    return;
  }

  while (i < p->n && r->key > p->r[i-1].key) i++;

  if (r->key == p->r[i-1].key){ 
    *r = p->r[i-1];
    return;
  }

  if (r->key < p->r[i-1].key) 
    Pesquisa(p->p[i-1], r);
  else 
    Pesquisa(p->p[i], r);
} 


void InsereNaPagina(Pagina *p, Pagina *pdir, Record r){ 
  short IsNotPosition;
  int k;
  
  k = p->n;  
  IsNotPosition = (k > 0);
  
  while (IsNotPosition){ 
    
    if (r.key >= p->r[k-1].key){ 
      IsNotPosition = FALSE;
      break;
    }
    
    p->r[k]   = p->r[k-1];
    p->p[k+1] = p->p[k];
    k--;
      
    if (k < 1) 
      IsNotPosition = FALSE;
  }
  
  p->r[k] = r; 
  p->p[k+1] = pdir;
  p->n++;

} 

void InsertFixUp(Pagina **p, Pagina **pr, Record r, Record *rr, short *Overflow){ 
  long i=1, j; 
  Pagina *paux;

  if (*p == NULL){ 
    *Overflow = TRUE; 
    *rr = r; 
    *pr = NULL;
    return;
  }

  while (i < (*p)->n && r.key > (*p)->r[i-1].key)  i++;

  if (r.key == (*p)->r[i-1].key){ 
    printf("[ERROR]: Record already exists\n"); 
    *Overflow = FALSE;
    return;
  }

  if (r.key < (*p)->r[i-1].key) i--; 

  /* caminha nas sub-árvores a esquerda e/ou direita da B Tree*/
  InsertFixUp(&(*p)->p[i], pr, r, rr, Overflow);
  
  /* na chamada (linha 101) esse if valida o registro já existente*/
  if (!*Overflow) return;

  if ((*p)->n < MM){ /* página com espaço */ 
      InsereNaPagina(*p, *pr, *rr);
      *Overflow = FALSE;
      return;
  }
  
  /* Overflow: Pagina tem que ser dividida */
  paux = (Pagina*)malloc(sizeof(Pagina));
  paux->n = 0;  
  paux->p[0] = NULL;

  if (i < M + 1){ 
    InsereNaPagina(paux, (*p)->p[MM], (*p)->r[MM-1]);
    (*p)->n--;
    InsereNaPagina(*p, *pr, *rr);
  }else 
    InsereNaPagina(paux, *pr, *rr);
  
  for (j=M+2; j<=MM; j++) 
    InsereNaPagina(paux, (*p)->p[j], (*p)->r[j-1]);

  (*p)->n = M;  
  paux->p[0] = (*p)->p[M+1];
  *rr = (*p)->r[M];  
  *pr = paux;
}


//Cap 18. página 357
void Insere(Pagina **p, Record r){ 
  short Overflow;
  Record rr;
  Pagina *pr, *paux;

  InsertFixUp(p, &pr, r, &rr, &Overflow);

  /* cresce na altura */
  if (Overflow){  
    paux = (Pagina *)malloc(sizeof(Pagina));
    paux->n = 1;  
    paux->r[0] = rr;
    paux->p[0] = *p;  
    paux->p[1] = pr;
    *p = paux;
  }
}

void RemoveFixUp(Pagina *p, Pagina *pai, int PosPai, int *underflow){ 
  Pagina *aux; 
  long disp, j;
  
  if (PosPai < pai->n){ 
    aux = pai->p[PosPai + 1];  
    disp = (aux->n - M + 1) / 2;
    p->r[p->n] = pai->r[PosPai];
    p->p[p->n + 1] = aux->p[0];  
    p->n++;
    
    if (disp > 0){ 
      for (j = 1; j < disp; j++)
        InsereNaPagina(p, aux->p[j], aux->r[j-1]);
      
      pai->r[PosPai] = aux->r[disp-1];  
      aux->n -= disp;
      
      for (j = 0; j < aux->n; j++) 
        aux->r[j] = aux->r[j + disp];
      for (j = 0; j <= aux->n; j++) 
        aux->p[j] = aux->p[j + disp];
      *underflow = FALSE;
    
    }else{ /* intercala aux em p e libera aux */ 
      
      for (j = 1; j <= M; j++)
        InsereNaPagina(p, aux->p[j], aux->r[j-1]);
      free(aux);
        
      for (j = PosPai + 1; j < pai->n; j++){ 
        pai->r[j-1] = pai->r[j];
        pai->p[j] = pai->p[j+1];
      }
      
      pai->n--;
      if (pai->n >= M) 
        *underflow = FALSE;
    }

  }else{ /* aux = Pagina a esquerda de p */ 
    aux = pai->p[PosPai-1]; 
    disp = (aux->n - M + 1) / 2;
    
    for (j = p->n; j >= 1; j--) 
      p->r[j] = p->r[j-1];
    
    p->r[0] = pai->r[PosPai-1];
    
    for (j = p->n; j >= 0; j--) 
      p->p[j+1] = p->p[j];
    
    p->n++;
    if (disp > 0){ /* Existe folga: transf. de aux para p */ 
      for (j = 1; j < disp; j++)
          InsereNaPagina(p, aux->p[aux->n - j + 1], aux->r[aux->n - j]);

      p->p[0] = aux->p[aux->n - disp + 1];
      pai->r[PosPai-1] = aux->r[aux->n - disp];
      aux->n -= disp;  
      *underflow = FALSE;

    }else{ /* intercala p em aux e libera p */ 
      
      for (j = 1; j <= M; j++)
            InsereNaPagina(aux, p->p[j], p->r[j-1]);
      
      free(p);  
      pai->n--;
      
      if (pai->n >= M)  
        *underflow = FALSE;
    }
  }
}

void Antecessor(Pagina *p, int i, Pagina *pai, int *underflow){ 

  if (pai->p[pai->n] != NULL){ 
    Antecessor(p, i, pai->p[pai->n], underflow);
    if (*underflow) 
      RemoveFixUp(pai->p[pai->n], pai, pai->n, underflow);
    return;
  }
  
  p->r[i-1] = pai->r[pai->n - 1]; 
  pai->n--;  
  *underflow = (pai->n < M);

}

//{1,2,3,4,5,6} {2,5,1}
void Remove(Pagina **p, int key, int *underflow){ 
  long j, i = 1;
  Pagina *aux;

  if (*p == NULL){ 
    printf("[ERROR]: Record not found!\n"); 
    *underflow = FALSE;
    return;
  }

  aux = *p;
  while (i < aux->n && key > aux->r[i-1].key) i++;

  if (key == aux->r[i-1].key){ 

    if (aux->p[i-1] == NULL){ 
      aux->n--;
      *underflow = (aux->n < M);

      for (j = i; j <= aux->n; j++){ 
        aux->r[j-1] = aux->r[j];  
        aux->p[j] = aux->p[j+1]; 
      }

      return;

    }

    Antecessor(*p, i, aux->p[i-1], underflow);
    if (*underflow) 
      RemoveFixUp(aux->p[i-1], *p, i - 1, underflow);
    return;
  }

  if (key > aux->r[i-1].key) i++;
  
  Remove(&aux->p[i-1], key, underflow);
  
  if (*underflow) 
    RemoveFixUp(aux->p[i-1], *p, i - 1, underflow);
} 

void Retira(Pagina **p, int key){ 
  int underflow;
  Pagina *aux;

  Remove(p, key, &underflow);
  
  if (underflow && (*p)->n == 0){ /* Arvore diminui na altura */
    aux = *p;   
    *p = aux->p[0]; 
    free(aux);
  }

}    

void Imprime(Pagina **p, int level){ 
  long i;
  
  if (*p == NULL) return;
  
  printf("Nivel %d: ", level);
  for (i = 0; i < (*p)->n; i++)
    printf("%d ",(*p)->r[i].key);
  putchar('\n');
  level++;

  for (i = 0; i <= (*p)->n; i++)
    Imprime(&(*p)->p[i], level);
}

