#include <iostream>

#include "argparse.h"
#include "library.h"
#include "consensus.h"

int main(int argc, char* argv[]) {
    argparse::ArgumentParser main_program("consensus-clustering");
    main_program.add_argument("--edgelist")
        .required()
        .help("Network edge-list file");
    main_program.add_argument("--threshold")
        .default_value(double(1.0))
        .help("Threshold value")
        .scan<'f', double>();
    main_program.add_argument("--algorithm")
        .help("Clustering algorithm (leiden-cpm, leiden-mod, louvain)")
        .action([](const std::string& value) {
            static const std::vector<std::string> choices = {"leiden-cpm", "leiden-mod", "louvain"};
            if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
                return value;
            }
            throw std::invalid_argument("--algorithm can only take in leiden-cpm, leiden-mod, or louvain.");
        });
    main_program.add_argument("--resolution")
        .default_value(double(0.01))
        .help("Resolution value for leiden-cpm")
        .scan<'f', double>();
    main_program.add_argument("--partitions")
        .default_value(int(10))
        .help("Number of partitions in consensus clustering")
        .scan<'d', int>();
    main_program.add_argument("--num-processors")
        .default_value(int(1))
        .help("Number of processors")
        .scan<'d', int>();
    main_program.add_argument("--output-file")
        .required()
        .help("Output clustering file");
    main_program.add_argument("--log-file")
        .required()
        .help("Output log file");
    main_program.add_argument("--log-level")
        .default_value(int(1))
        .help("Log level where 0 = silent, 1 = info, 2 = verbose")
        .scan<'d', int>();

    try {
        main_program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << main_program;
        std::exit(1);
    }

    std::string edgelist = main_program.get<std::string>("--edgelist");
    std::string algorithm = main_program.get<std::string>("--algorithm");
    double threshold = main_program.get<double>("--threshold");
    double resolution = main_program.get<double>("--resolution");
    int num_partitions = main_program.get<int>("--partitions");
    int num_processors = main_program.get<int>("--num-processors");
    std::string output_file = main_program.get<std::string>("--output-file");
    std::string log_file = main_program.get<std::string>("--log-file");
    int log_level = main_program.get<int>("--log-level");
    Consensus* consensus = new Consensus(edgelist, algorithm, threshold, resolution, num_partitions, num_processors, output_file, log_file, log_level);
    consensus->main();
    delete consensus;

    /* Library* library = new Library(); */
    /* std::cout<<library->square(5)<<std::endl; */
    /* Obj* obj = new Obj(); */
    /* std::cout<<obj->test_obj()<<std::endl; */

    /* std::string edgelist = main_program.get<std::string>("--edgelist"); */
    /* std::cout << "edge list was " << edgelist << std::endl; */
    /* std::string algorithm = main_program.get<std::string>("--algorithm"); */
    /* std::cout << "algorithm was " << edgelist << std::endl; */
}
