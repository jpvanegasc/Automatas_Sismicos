#include<iostream>
#include<fstream>
#include<sstream>
#include<cmath>
#include<string>
#include"Random64.h"
using namespace std;

const int square = 256;
const int Lx = square;
const int Ly = square;
const double F_th = 2.0;
const double alpha = 0.25; // 0.25, 0.2, 0.15, 0.1, 0.075, 0.025

void InicieAnimacion(void);
void Cuadro(int t);

/* ---------- CLASE AUTÓMATA ---------- */
class Automata{
	private:
		double juan[Lx][Ly], max_cell, stress_total, energia; 
		int fallas_total, max_cell_cell[2]; 
		bool falla_activa;
	public:
		Automata(void){
			// Iniciar max_cell
			max_cell_cell[0]=0; // Posición x
			max_cell_cell[1]=0; // Posición y
			max_cell=0; // Valor de estrés
			// Iniciar auxiliares
			falla_activa=false; fallas_total = 0; 
			stress_total = 0; energia=0;
		}
		/**
		 * Inicializar los valores de la matriz de estrés
		 * @param ran64: Generador de números aleatorios
		 */
		void inicie(Crandom &ran64){
			for(int i=1; i<Lx-1; i++)
				for(int j=1; j<Ly-1; j++){
					// Cargar la matriz
					double ran_num = ran64.r()*F_th;
					juan[i][j] = ran_num;
					juan[square/2][square/2]=F_th;
					juan[square/2+1][square/2]=F_th;
					juan[square/2-1][square/2]=F_th;
					juan[square/2][square/2+1]=F_th;
					juan[square/2][square/2-1]=F_th;
					juan[square/2+2][square/2]=F_th;
					juan[square/2-2][square/2]=F_th;
					juan[square/2][square/2+2]=F_th;
					juan[square/2][square/2-2]=F_th;
					// Actualizar la celda con más estrés
					if(ran_num>max_cell){
						max_cell_cell[0] = i; max_cell_cell[1] = j; max_cell = ran_num;
					}
				}
			anular_fronteras();
		}
		/**
		 * Buscar, anular y guardar (el número de) las celdas que fallaron, 
		 *  repartir la fuerza entre las vecinas y activar o desactivar falla_activa
		 */
		void distribuya(void){
			int fallas_actual=0; max_cell=0; 
			for(int i=1; i<Lx-1; i++)
				for(int j=1; j<Ly-1; j++){
					if(juan[i][j] >= F_th){ // Si hay falla
						// Repartir en las vecinas
						if(i != 1) juan[i-1][j] += juan[i][j]*alpha; // Derecha
						if(i != Lx-2) juan[i+1][j] += juan[i][j]*alpha; // Izquierda
						if(j != 1) juan[i][j-1] += juan[i][j]*alpha; // Arriba
						if(j != Ly-2) juan[i][j+1] += juan[i][j]*alpha; // Abajo
						// Borrar celda actual
						energia+=juan[i][j]; juan[i][j]=0; fallas_actual += 1; 
					}
					// Actualizar la celda con más estrés
					else if(juan[i][j] > max_cell){
						max_cell_cell[0] = i; max_cell_cell[1] = j; max_cell = juan[i][j];
					}
				}
			if(fallas_actual != 0){ // Si hubo falla
				fallas_total += fallas_actual;
				falla_activa = true;
			}
			else // Si ninguna falló
				falla_activa = false;
		}
		/**
		 * Fallar todas las celdas que en un paso de tiempo superan el valor máximo
		 */
		void aumente(void){
			// Calcular aumento
			double d_F = F_th - max_cell;
			// Aumentar
			for(int i=1; i<Lx-1; i++)
				for(int j=1;  j<Ly-1; j++)
					juan[i][j] += d_F;
			// Distribuir hasta que ninguna celda supere F_th
			falla_activa = true;
			while(falla_activa)
				distribuya();
		}
		// Retorna la suma de toda la matriz
		int stress(void){
			stress_total = 0;
			for(int i=0; i<Lx; i++)
				for(int j=0; j<Ly; j++)
					stress_total += juan[i][j];
			return stress_total;
		}
		void delete_fallas(void){fallas_total = 0;}
        void delete_energia(void){energia=0;}
		int get_fallas(void){return fallas_total;}
    	double get_energia(void){return energia;}
		double max_F(void){return max_cell;}
		void guardar_archivo(int t, bool print_as_matrix);
		void anular_fronteras(void);
};

/* ---------- MAIN ---------- */
int main(void){
	Automata Shaky;
	Crandom ran64(1);
	int t, t_max = 1e4;
	
	Shaky.inicie(ran64);
	
	for(t=0; t<t_max; t++){
	    Shaky.delete_fallas();
		Shaky.aumente();
		Shaky.get_fallas();
	}
	return 0;
}

/* ---------- AUXILIARES ---------- */
/**
 * Guarda la matriz de estrés como un archivo para un paso de tiempo específico
 * @param t: Determina el nombre del archivo con el que se guarda
 * @param print_as_matrix: 
 */
void Automata::guardar_archivo(int t, bool print_as_matrix){
    string filename;
	//ofstream MiArchivo("Resultados.dat"); //COMENTAR PARA ANIMAR
	//DESCOMENTAR PARA ANIMAR
	ofstream MiArchivo;
	stringstream a;
	a << t;
	filename = "./Resultados/Resultados" + a.str();
	filename+= ".dat";
	 MiArchivo.open(filename.c_str(),ios::out);
	
	if(print_as_matrix){
		for(int i=0; i<Lx; i++){
			for(int j=0; j<Ly; j++)
					MiArchivo<<juan[i][j]<<'\t';
			MiArchivo << '\n';
		}
		MiArchivo<<endl;
		MiArchivo.close();
	}
	else{
		for(int i=0; i<Lx; i++){
			for(int j=0; j<Ly; j++)
					MiArchivo<< i <<'\t'<< j << '\t' <<juan[i][j]<<'\n';
			MiArchivo<< '\n';
		}
		MiArchivo<<endl;
		MiArchivo.close();
	}
}
// Técnicamente este no se debería usar, pero ajá
void Automata::anular_fronteras(void){
	for(int i=0; i<Lx; i++){
		juan[i][0]=0;
		juan[i][Ly-1]=0;
	}
	for(int j=0; j<Ly; j++){
		juan[0][j] = 0;
		juan[Lx-1][j] = 0;
	}
}

void InicieAnimacion(void){
  //cout<<"set terminal gif animate"<<endl;
  //cout<<"set output 'pelicula.gif'"<<endl;
  cout<<"set pm3d map"<<endl;
  cout<<"set size ratio -1"<<endl;
  cout<<"set xrange [0:"<<Lx<<"]"<<endl;
  cout<<"set yrange [0:"<<Ly<<"]"<<endl;
}

void Cuadro(int t){
  cout<<"plot 'Resultados"<<t<<".dat' matrix with image"<<endl;
}