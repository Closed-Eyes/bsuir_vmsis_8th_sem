#include <iostream>
#include <stdlib.h>
#include <time.h>

void main()
{
	static double mtx1[100][100][8][4];
	static double mtx2[100][100][4][8];
	static double mtx3[100][100][8][8];

	srand(time(NULL));

	for(int i = 0; i < 100; i++){
		for(int j = 0; j < 100; j++){
			for(int k = 0; k < 8; k++){
				for(int m = 0; m < 4; m++){
					mtx1[i][j][k][m] = rand()%100;
				}
			}
		}
	}

	for(int i = 0; i < 100; i++){
		for(int j = 0; j < 100; j++){
			for(int k = 0; k < 4; k++){
				for(int m = 0; m < 8; m++){
					mtx2[i][j][k][m] = rand()%100;
				}
			}
		}
	}

	for(int i = 0; i < 100; i++){
		for(int j = 0; j < 100; j++){
			for(int k = 0; k < 4; k++){
				for(int m = 0; m < 8; m++){
					mtx3[i][j][k][m] = 0;
				}
			}
		}
	}

	for(int i = 0; i < 100; i++){
		for(int j = 0; j < 100; j++){			
			for(int k = 0; k < 100; k++){
				// так должно быть:
				// res = mtx1[i][k] * mtx2[k][j]
				// mtx3[i][j] += res;

				// высчитываем по очереди:
				// обнуление RES = 0
				double res[8][8];
				for(int i0 = 0; i0 < 8; i0++){
					for(int j0 = 0; j0 < 8; j0++){
						res[i0][j0] = 0;
					}
				}

				// высчитывание RES
				for(int i0 = 0; i0 < 8; i0 ++){
					for(int j0 = 0; j0 < 8; j0++){
						double sum = 0;
						for(int k0 = 0; k0 < 4; k0++){
							sum += mtx1[i][k][i0][k0] * mtx2[k][j][k0][j0];
						}
						res[i0][j0] = sum;
					}
				}

				//MTX3 += RES
				for(int i0 = 0; i0 < 8; i0++){
					for(int j0 = 0; j0 < 8; j0++){
						mtx3[i][j][i0][j0] += res[i0][j0];
					}
				}
			}
		}
	}
}