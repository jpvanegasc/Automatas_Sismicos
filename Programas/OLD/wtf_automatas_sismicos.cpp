// Autómata Sísmico optimizado usando memoria dinámica
#include<iostream>
#include<fstream>
#include<sstream>
#include<cmath>
#include<string>
#include<vector>
#include"Random64.h"
using namespace std;

const int square = 32;
const int Lx = square;
const int Ly = square;

const double F_th = 2.0;
const double alpha = 0.2; // 0.25, 0.2, 0.15, 0.1, 0.075, 0.025

/* ---------- CLASE AUTÓMATA ---------- */
class Automata{
	private:
		double fault[Lx][Ly], max_cell, stress_total; 
		unsigned int fallas_total; bool falla_activa;
		vector<vector<int>> over_F_th;
	public:
		Automata(void){
			max_cell=0; 
			falla_activa=false; fallas_total = 0; 
			stress_total = 0;
		}
		/**
		 * Inicializar los valores de la matriz de estrés
		 * @param ran64: Generador de números aleatorios
		 */
		void inicie(Crandom &ran64){
			for(int i=0; i<Lx; i++)
				for(int j=0; j<Ly; j++){
					// Condiciones de frontera
					if(i == 0) fault[i][j] = 0;
					else if(i == Lx-1) fault[i][j] = 0;
					else if(j == 0) fault[i][j] = 0;
					else if(j == Ly-1) fault[i][j] =0;
					// El resto de la matriz
					else{
						// Cargar la matriz
						double ran_num = ran64.r()*F_th;
						fault[i][j] = ran_num;
						// Actualizar la celda con más estrés
						if(ran_num > max_cell){
							max_cell = ran_num;
						}
					}
				}
		}
		/**
		 * Buscar, anular y guardar (el número de) las celdas que fallaron, 
		 *  repartir la fuerza entre las vecinas y activar o desactivar falla_activa
		 *  buscando solo las celdas que fallaron
		 * @param over_cells: posiciones de las celdas que superaron el umbral
		 */
		void distribuya_short(vector<vector<int>> &over_cells){
			for(int iter = 0; iter<over_cells.size(); iter++){
				int i, j; i = over_cells[iter][0]; j = over_cells[iter][1];
				// Repartir en las vecinas
				double cambiar; cambiar = fault[i][j]*alpha;
				if(i != 1) fault[i-1][j] += cambiar; // Derecha
				if(i != Lx-2) fault[i+1][j] += cambiar; // Izquierda
				if(j != 1) fault[i][j-1] += cambiar; // Arriba
				if(j != Ly-2) fault[i][j+1] += cambiar; // Abajo
				// Borrar celda actual
				fault[i][j] = 0; 
				fallas_total += 1;
			}
		}
		/**
		 * Buscar, anular y guardar (el número de) las celdas que fallaron, 
		 *  repartir la fuerza entre las vecinas y activar o desactivar falla_activa
		 *  buscando celda por celda
		 */
		void distribuya_long(void){
			int fallas_actual=0; max_cell=0; 
			for(int i=1; i<Lx-1; i++)
				for(int j=1; j<Ly-1; j++){
					if(fault[i][j] >= F_th){ // Si hay falla
						// Repartir en las vecinas
						double cambiar; cambiar = fault[i][j]*alpha;
						if(i != 1) fault[i-1][j] += cambiar; // Derecha
						if(i != Lx-2) fault[i+1][j] += cambiar; // Izquierda
						if(j != 1) fault[i][j-1] += cambiar; // Arriba
						if(j != Ly-2) fault[i][j+1] += cambiar; // Abajo
						// Borrar celda actual
						fault[i][j] = 0; 
						fallas_actual += 1; 
					}
					// Actualizar la celda con más estrés
					else if(fault[i][j] > max_cell){
						max_cell = fault[i][j];
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
			// Calcular aumento y sumarlo
			double d_F = F_th - max_cell;
			for(int i=1; i<Lx-1; i++)
				for(int j=1;  j<Ly-1; j++){
					fault[i][j] += d_F;
					if(fault[i][j] >= F_th){
						vector<int> cell_id;
						cell_id.push_back(i); cell_id.push_back(j);
						over_F_th.push_back(cell_id);
					}
				}
			// Distribuir hasta que ninguna celda supere F_th
			distribuya_short(over_F_th);
			over_F_th.clear();
			falla_activa = true;
			while(falla_activa)
				distribuya_long();
		}
		// Retorna la suma de toda la matriz
		double total_stress(void){
			stress_total = 0;
			for(int i=0; i<Lx; i++)
				for(int j=0; j<Ly; j++)
					stress_total += fault[i][j];
			return stress_total;
		}
		void delete_fallas(void){fallas_total = 0;}
		int get_fallas(void){return fallas_total;}
		double max_F(void){return max_cell;}
		void guardar_archivo(int t, bool print_as_matrix);
};

/* ---------- MAIN ---------- */
int main(void){
	Automata Shaky;
	Crandom ran64(1);
	unsigned int t, t_max = 1e8;
	
	Shaky.inicie(ran64);
	ofstream result_file("L32_a02.txt");

	for(t=0; t<t_max; t++){
		Shaky.delete_fallas();
		Shaky.aumente();
		if(t > 9e7){
			result_file<<Shaky.get_fallas()<<'\n';
			cout<<t<<'\n';
			//cout<<t<<'\t'<<Shaky.total_stress()<<'\n';	
			//cout<<Shaky.get_fallas()<<'\n';
		}
	}
	
	result_file<<endl;
	result_file.close();
	
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
	filename = "./Resultados" + a.str();
	filename+= ".dat";
	MiArchivo.open(filename.c_str(),ios::out);
	
	if(print_as_matrix){
		for(int i=0; i<Lx; i++){
			for(int j=0; j<Ly; j++)
					MiArchivo<<fault[i][j]<<'\t';
			MiArchivo << '\n';
		}
		MiArchivo<<endl;
		MiArchivo.close();
	}
	else{
		for(int i=0; i<Lx; i++){
			for(int j=0; j<Ly; j++)
					MiArchivo<< i <<'\t'<< j << '\t' <<fault[i][j]<<'\n';
			MiArchivo<< '\n';
		}
		MiArchivo<<endl;
		MiArchivo.close();
	}
}