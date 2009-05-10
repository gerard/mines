/*
Licensed under the GNU GPLv2
Created by Raharu
*/

#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#define MINESPERCENT 15

/*
Sencilla funcion que crea una matriz dinamica
*/
int **dynamat(int lin, int col, int relleno) {
  int i, j, **mat;
    
  mat=(int **)malloc(lin*sizeof(int *));
  for(i=0;i<lin;i++) {
    mat[i]=(int *)malloc(col*sizeof(int));
    if(relleno) for(j=0;j<col;j++) mat[i][j]=0;
    }
  return mat;
}

/*
Funcion que crea la matriz de minas y sus valores adyacentes
*/
int **creamatrnd(int min, int lin, int col) {
  int i, j, rand, **mat, *vect;
  
  srandom(time(NULL));
  
  /*
  Esto crea un vector aleatorio (creado en tiempo de ejecucion)
  sin repeticiones
  Solo son aleatorios los "min" primeros valores
  */
  vect=(int *)malloc(lin*col*sizeof(int));
  for(i=0;i<lin*col;i++) vect[i]=i;
  for(i=0;i<min;i++) {
    rand=random()%(lin*col);
    j=vect[i];
    vect[i]=vect[rand];
    vect[rand]=i;
  }
  
  /*
  Crea una matriz en tiempo de ejecucion
  */
  mat=dynamat(lin,col,1);
  
  /*
  Transforma el vector aleatorio a una matriz aleatorio
  Solo se toman los "min" primeros valores
  */
  for(i=0;i<min;i++) mat[vect[i]/col][vect[i]%col]=9;
   
  /*
  Esto determina los valores de las casillas alrededor de las minas
  */
  for(i=0;i<lin;i++)
    for(j=0;j<col;j++)
      if(mat[i][j]>=9) {
        if(i>0) {
	  mat[i-1][j]++;
	  if(j>0) mat[i-1][j-1]++;
	  if(j<col-1) mat[i-1][j+1]++;
	  }
	if(i<lin-1) {
	  mat[i+1][j]++;
	  if(j>0) mat[i+1][j-1]++;
	  if(j<col-1) mat[i+1][j+1]++;
	}
	if(j>0) mat[i][j-1]++;
	if(j<col-1) mat[i][j+1]++;
      }

  /*
  Esto pone a su valor correcto las minas
  */
  for(i=0;i<lin;i++)
    for(j=0;j<col;j++)
      if(mat[i][j]>9) mat[i][j]=9;
      
  /*
  Y devuelve la matriz generada
  */
  return mat;
}


/*
Función que muestra el estado del campo de juego
*/
void muestracampo(int **mat, int **mascara, int lin, int col, int todo) {
  int i, j;

  for(i=0;i<lin;i++)
    for(j=0;j<col;j++)
      if(mascara[i][j]==1 || todo) 
        if(mat[i][j]==0) mvaddch(i,j, ' ');
	else if(mat[i][j]==9) mvaddch(i,j, '*' | A_BOLD);
	else {
	  attron(COLOR_PAIR(mat[i][j]));
	  mvaddch(i, j, mat[i][j]+48);
	  attroff(COLOR_PAIR(mat[i][j]));
	}
      else if(mascara[i][j]==2) mvaddch(i,j, 'p' | A_BOLD);
      else if(mascara[i][j]==0) mvaddch(i,j, '#');
}


/*
Funcion para mover el cursor
*/
int pulsa() {
  int col, lin, maxcol, maxlin, movcol=0, movlin=0;
  
  getyx(stdscr,lin,col);
  getmaxyx(stdscr,maxlin,maxcol);
  while(1) {
    switch(getch()) {
    case KEY_RIGHT:
      movcol=1;
      movlin=0;
      break;
    case KEY_LEFT:
      movcol=-1;
      movlin=0;
      break;
    case KEY_DOWN:
      movlin=1;
      movcol=0;
      break;
    case KEY_UP:
      movlin=-1;
      movcol=0;
      break;
    case 'q':
      return 1;
    case 'w':
      return 2;
    case 'e':
      return 3;
    case 'x':
      return 4;
    }
    move(lin=(lin+movlin+maxlin)%maxlin,col=(col+movcol+maxcol)%maxcol);
  }
}

int despejerecurs(int **mat, int **mascara, int lin, int col, int i, int j) {
  mascara[i][j]=1;
  if(mat[i][j]==0) {
    if(i>0) {
      if(mascara[i-1][j]==0) despejerecurs(mat, mascara, lin, col, i-1,j);
      if(j>0 && mascara[i-1][j-1]==0) despejerecurs(mat, mascara, lin, col, i-1, j-1);
      if(j<col-1 && mascara[i-1][j+1]==0) despejerecurs(mat, mascara, lin, col, i-1, j+1);
    }
    if(i<lin-1) {
      if(mascara[i+1][j]==0) despejerecurs(mat, mascara, lin, col, i+1, j);
      if(j>0 && mascara[i+1][j-1]==0) despejerecurs(mat, mascara, lin, col, i+1, j-1);
      if(j<col-1 && mascara[i+1][j+1]==0) despejerecurs(mat, mascara, lin, col, i+1, j+1);
    }
    if(j>0 && mascara[i][j-1]==0) despejerecurs(mat, mascara, lin, col, i, j-1);
    if(j<col-1 && mascara[i][j+1]==0) despejerecurs(mat, mascara, lin, col, i, j+1);
  }
}


/*
Esta funcion inicializa los colores adecuadamente
*/
int poncolores(void) {
  init_pair(1, COLOR_CYAN, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  init_pair(3, COLOR_GREEN, COLOR_BLACK);
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);
  init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(6, COLOR_RED, COLOR_BLACK);
  init_pair(7, COLOR_RED, COLOR_BLACK);
  init_pair(8, COLOR_RED, COLOR_BLACK);
}

/*
Esta función comprueba si se han destapado todas las casillas
*/
int comprueba(int **mat, int **mascara, int lin, int col) {
  int i, j;

  for(i=0;i<lin;i++)
    for(j=0;j<col;j++)
      if(mascara[i][j]==0 && mat[i][j]!=9) return 0;
  
  return 1;
}

/*
Esta funcion destapa la casilla especificada
*/
int destapa(int **mat, int **mascara, int lin, int col, int i, int j, int *bom) {
  if(mat[i][j]==9) *bom=1;
  else if(mat[i][j]==0) {
    despejerecurs(mat, mascara, lin, col, i, j);
  } else mascara[i][j]=1;
  return 1;
}
/*
Esta funcion determina el numero de minas marcadas circundantes a una casilla
*/
int minasmarcadas(int **mascara, int lin, int col, int i, int j) {
  int cont=0;
  
  if(i>0) {
    if(mascara[i-1][j]==2) cont++;
    if(j>0 && mascara[i-1][j-1]==2) cont++;
    if(j<col-1 && mascara[i-1][j+1]==2) cont++;
  }
  if(i<lin-1) {
    if(mascara[i+1][j]==2) cont++;
    if(j>0 && mascara[i+1][j-1]==2) cont++;
    if(j<col-1 && mascara[i+1][j+1]==2) cont++;
  }
  if(j>0 && mascara[i][j-1]==2) cont++;
  if(j<col-1 && mascara[i][j+1]==2) cont++;

  return cont;
}    


/*
Main
*/
int main() {
  int i, j, lin, col, min, bom=0, accion, **mat, **mascara;

  initscr();
  if(has_colors() == FALSE) {
    endwin();
    printf("Ponte un terminal con colores leñe");
    exit(1);
  }
  start_color();
  raw();
  noecho();
  keypad(stdscr, TRUE);
  poncolores();
  
  
  getmaxyx(stdscr, lin, col);
  min=(lin*col)*MINESPERCENT/100;
  
  
  mat=creamatrnd(min, lin, col);
  mascara=dynamat(lin, col, 1); 

  while(!bom) {
    muestracampo(mat,mascara, lin, col, 0);
    move(i,j);
    accion=pulsa();
    getyx(stdscr,i,j);
    
    if(accion==1 && mascara[i][j]==0) { /*Pulsacion destape*/
      destapa(mat, mascara, lin, col, i, j, &bom);
    } else if(accion==2) {
        if(mascara[i][j]==0) mascara[i][j]=2; /*Pulsacion marca*/
        else if(mascara[i][j]==2) mascara[i][j]=0;
    } else if(accion==4) bom=1;		/*Pulsacion exit*/
    else if(accion==3 && mascara[i][j]==1 && minasmarcadas(mascara, lin, col, i, j)==mat[i][j]) {		/*Pulsacion destape multiple*/
      if(i>0) {
        if(mascara[i-1][j]==0) destapa(mat, mascara, lin, col, i-1,j, &bom);
        if(j>0 && mascara[i-1][j-1]==0) destapa(mat, mascara, lin, col, i-1, j-1, &bom);
        if(j<col-1 && mascara[i-1][j+1]==0) destapa(mat, mascara, lin, col, i-1, j+1, &bom);
      }
      if(i<lin-1) {
        if(mascara[i+1][j]==0) destapa(mat, mascara, lin, col, i+1, j, &bom);
        if(j>0 && mascara[i+1][j-1]==0) destapa(mat, mascara, lin, col, i+1, j-1, &bom);
        if(j<col-1 && mascara[i+1][j+1]==0) destapa(mat, mascara, lin, col, i+1, j+1, &bom);
      }
      if(j>0 && mascara[i][j-1]==0) destapa(mat, mascara, lin, col, i, j-1, &bom);
      if(j<col-1 && mascara[i][j+1]==0) destapa(mat, mascara, lin, col, i, j+1, &bom);
    }    
    if(comprueba(mat, mascara, lin, col)) bom=2; /*Comprobacion de victoria*/
  }
  muestracampo(mat, mascara, lin, col, 1);
  if(bom==2) mvprintw(5,5,"Has ganado!!");
  else mvprintw(5,5,"Has perdido!!");
  getch();
  endwin();
}
