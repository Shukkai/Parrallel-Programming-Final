#include "ACO.h"
#include "ACO_omp.h"
#include "ACO_thread.h"
#include "ga_cpu.h"
#include "reader.h"
#include "solver.h"
#include <chrono>
#include "ga_omp.h"
#include "ga_thread.h"

enum parallel_type
{
    SERIAL,
    OMP,
    THREAD,
};

inline int hashit(std::string const &inString)
{
    if (inString == "serial")
        return SERIAL;
    if (inString == "omp")
        return OMP;
    if (inString == "thread")
        return THREAD;
    return SERIAL;
}

int main(int argc, char *argv[])
{
    std::string filename = (argc > 0) ? argv[1] : "tsp_graph/a280.tsp";
    std::string type = (argc > 1) ? argv[2] : "aco";
    std::string parallel = (argc > 2) ? argv[3] : "serial";
    TSPReader reader;
    if (!reader.readFile(filename))
    {
        std::cerr << "Failed to read TSP file." << std::endl;
        return 1;
    }

    const int thread_num = 8;
    std::vector<int> bestTour;
    double bestDistance = 0.0, elapsedTime = 0.0;
    std::cout << "Solving TSP...\n";
    if (type == "aco")
    {
        auto start = std::chrono::high_resolution_clock::now();

        if (parallel == "omp")
        {
            ACOOmp solver(reader.getPoints());
            solver.solve();
            bestTour = solver.getTour();
            bestDistance = solver.getDistance();
        }
        else if (parallel == "thread")
        {
            ACOThread solver(reader.getPoints(), thread_num);
            solver.solve();
            bestTour = solver.getTour();
            bestDistance = solver.getDistance();
        }
        else
        {
            ACO solver(reader.getPoints());
            solver.solve();
            bestTour = solver.getTour();
            bestDistance = solver.getDistance();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        elapsedTime = duration.count();
    }
    else if (type == "ga")
    {
        // Solve TSP
        auto start = std::chrono::high_resolution_clock::now();
        std::pair<std::vector<int>, double> result;
        if (parallel == "omp")
        {
            ga_omp gaomp(reader.getPoints(), 100, 10000, 0.05, 0.8);
            start = std::chrono::high_resolution_clock::now();
            result = gaomp.solve();
        }
        else if (parallel == "thread")
        {
            ga_thread gathread(reader.getPoints(), 100, 10000, 0.05, 0.8, thread_num);
            start = std::chrono::high_resolution_clock::now();
            result = gathread.solve();
        }
        else
        {
            GeneticTSP ga(reader.getPoints(), 100, 10000, 0.05, 0.8);
            start = std::chrono::high_resolution_clock::now();
            result = ga.solve();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        bestTour = result.first;
        bestDistance = result.second;
        elapsedTime = duration.count();
    }

    // write results to file
    std::string res_fname = "res/" + filename + "_" + type;
    switch (hashit(parallel))
    {
    case 1:
        res_fname += "_omp.txt";
        break;
    case 2:
        res_fname += "_thread_" + std::to_string(thread_num) + ".txt";
        break;
    default:
        res_fname += "_serial.txt";
        break;
    }

    std::ofstream file;
    file.open(res_fname);
    file << "Testcase: " << filename << "\n";
    file << "Time taken: " << elapsedTime << " milliseconds\n";
    file << "Total distance: " << bestDistance << "\n";
    file << "Best tour found:\n";
    for (int city : bestTour)
    {
        file << city << "\n";
    }

    // print results to console
    std::cout << "Distance: " << bestDistance << "\nSolution found in " << elapsedTime << " milliseconds\n";
    
    return 0;
}
