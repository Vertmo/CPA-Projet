#include <vector>
#include <algorithm>
#include "structs.h"

std::map<unsigned int, unsigned int> *reverseMap(std::map<unsigned int, unsigned int> *map) {
  std::map<unsigned int, unsigned int>  *revMap = new std::map<unsigned int, unsigned int>();
  for(std::pair<unsigned int, unsigned int> p: *map) {
    (*revMap)[p.second] = p.first;
  }
  return revMap;
}

void matVectProd(adjArray *arr, double *P) {
  double *B = (double *)malloc(arr->n*sizeof(double));
  for(int i = 0; i < arr->n; i++) B[i] = 0;
  for(int i = 0; i < arr->n; i++) {
    int deg = (arr->cd[i+1]-arr->cd[i]);
    for(int j = arr->cd[i]; j < arr->cd[i+1]; j++) {
      int nj = arr->adj[j];
      B[nj] += P[i]/(double)deg;
    }
  }

  for(int i = 0; i < arr->n; i++) P[i] = B[i];
  free(B);
}

void normalize2(double *P, int n) {
  double norme = 0;
  for(int i = 0; i < n; i++) norme += P[i];
  for(int i = 0; i < n; i++) {
    P[i] += (1.-norme)/(double)n;
  }
}

double *pagerank(adjArray *arr, double alpha, int t) {
  double *ranks = (double *)malloc(arr->n*sizeof(double));
  for(int i = 0; i < arr->n; i++) {
    ranks[i] = (double)1/(double)arr->n;
  }

  for(int i = 0; i < t; i++) {
    matVectProd(arr, ranks);
    for(int j = 0; j < arr->n; j++) {
      ranks[j] = (1 - alpha)*ranks[j] + alpha * ((double)1/(double)arr->n);
    }
    normalize2(ranks, arr->n);
  }

  return ranks;
}

void writeRanks(char *fn, std::vector<std::pair<unsigned int, double>> ranks) {
  FILE *f = fopen(fn, "w");
  for(std::pair<int, double> p: ranks) {
    fprintf(f, "%d %.15g\n", p.first, p.second); //%.17g pour plus de précision d'affichage
  }
  fclose(f);
}

int main(int argc, char **argv) {
  if(argc != 3) {
    printf("usage : ./pagerank.exe <input_file> <output_file>\n");
    return 1;
  }
  char *inName = argv[1];
  char *outName = argv[2];

  unsigned int nbNodes = 0; unsigned int nbEdges = 0;
  std::map<unsigned int, unsigned int> *map = size(inName, &nbNodes, &nbEdges);
  std::map<unsigned int, unsigned int> *revMap = reverseMap(map);

  adjArray *arr = loadAsAdjArray(inName, nbNodes, nbEdges, map);
  printf("Loaded graph\n");

  double *ranks = pagerank(arr, 0.15, 20);
  printf("Calculated ranks\n");

  std::vector<std::pair<unsigned int, double>> rankMap(arr->n);
  for(int i = 0; i < arr->n; i++) {
    rankMap[i] = std::pair<unsigned int, double>((*revMap)[i], ranks[i]);
  }

  writeRanks(outName, rankMap);

  // Best and worst pages
  std::sort(rankMap.begin(), rankMap.end(),
            [] (std::pair<int, double> p1, std::pair<int, double> p2) -> int {
              return p1.second > p2.second;
            });

  printf("Top 5:\n");
  for(int i = 0; i < 5; i++) {
    printf("%d: %.15g\n", rankMap[i].first, rankMap[i].second);
  }

  printf("Flop 5:\n");
  for(int i = 0; i < 5; i++) {
    printf("%d: %.15g\n", rankMap[arr->n-(i+1)].first,  rankMap[arr->n-(i+1)].second);
  }

  freeAdjArray(arr);
  free(ranks);
  delete map; delete revMap;
  return 0;
}
