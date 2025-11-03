#include <chrono>
using namespace std;
//questa semplice funzione calcola il tempo medio impiegato ad eseguire la funzione Func times volte, sfruttando la libreria chrono (migliore rispetto a time)
template<typename Func>
double computeTime(Func f, int times) {
    double total_duration = 0.0;
    for(int i = 0; i < times; ++i) {
        auto start = chrono::high_resolution_clock::now();
        f();
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> duration_ms = end - start;
        total_duration += duration_ms.count();
    }
    return total_duration / times; // Ritorna la media in millisecondi
}