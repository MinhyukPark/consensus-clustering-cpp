#include <iostream>

#include "argparse.h"
#include "library.h"
#include "simple_consensus.h"
#include "threshold_consensus.h"

int main(int argc, char* argv[]) {
    argparse::ArgumentParser main_program("consensus-clustering");
    argparse::ArgumentParser simple_consensus("simple");
    simple_consensus.add_description("Simple consensus algorithm");

    argparse::ArgumentParser threshold_consensus("threshold");
    threshold_consensus.add_description("Threshold consensus algorithm (set threshold to 1 for strict consensus)");

    simple_consensus.add_argument("--edgelist")
        .required()
        .help("Network edge-list file");
    simple_consensus.add_argument("--threshold")
        .default_value(double(1.0))
        .help("Threshold value")
        .scan<'f', double>();
    simple_consensus.add_argument("--partition-file")
        .required()
        .help("Clustering partition file where the first column is one of (leiden-cpm, leiden-mod, louvain), second column is its weight, and the third column is the clustering method parameter (resolution value for leiden-cpm, ignored for leiden-mod and louvain. One can put -1 here in these cases).");
    simple_consensus.add_argument("--final-algorithm")
        .help("Final clustering algorithm to be used (leiden-cpm, leiden-mod, louvain)")
        .action([](const std::string& value) {
            static const std::vector<std::string> choices = {"leiden-cpm", "leiden-mod", "louvain"};
            if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
                return value;
            }
            throw std::invalid_argument("--algorithm can only take in leiden-cpm, leiden-mod, or louvain.");
        });
    simple_consensus.add_argument("--final-resolution")
        .default_value(double(0.01))
        .help("Resolution value for the final run. Only used if --final-algorithm is leiden-cpm")
        .scan<'f', double>();
    simple_consensus.add_argument("--delta")
        .default_value(double(0.02))
        .help("Convergence parameter")
        .scan<'f', double>();
    simple_consensus.add_argument("--max-iter")
        .default_value(int(2))
        .help("Maximum number of iterations in simple consensus")
        .scan<'d', int>();
    simple_consensus.add_argument("--partitions")
        .default_value(int(10))
        .help("Number of partitions in consensus clustering")
        .scan<'d', int>();
    simple_consensus.add_argument("--num-processors")
        .default_value(int(1))
        .help("Number of processors")
        .scan<'d', int>();
    simple_consensus.add_argument("--output-file")
        .required()
        .help("Output clustering file");
    simple_consensus.add_argument("--log-file")
        .required()
        .help("Output log file");
    simple_consensus.add_argument("--log-level")
        .default_value(int(1))
        .help("Log level where 0 = silent, 1 = info, 2 = verbose")
        .scan<'d', int>();

    threshold_consensus.add_argument("--edgelist")
        .required()
        .help("Network edge-list file");
    threshold_consensus.add_argument("--threshold")
        .default_value(double(1.0))
        .help("Threshold value")
        .scan<'f', double>();
    threshold_consensus.add_argument("--partition-file")
        .required()
        .help("Clustering partition file where the first column is one of (leiden-cpm, leiden-mod, louvain), second column is its weight, and the third column is the clustering method parameter (resolution value for leiden-cpm, ignored for leiden-mod and louvain. One can put -1 here in these cases).");
    threshold_consensus.add_argument("--final-algorithm")
        .help("Final clustering algorithm to be used (leiden-cpm, leiden-mod, louvain)")
        .action([](const std::string& value) {
            static const std::vector<std::string> choices = {"leiden-cpm", "leiden-mod", "louvain"};
            if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
                return value;
            }
            throw std::invalid_argument("--algorithm can only take in leiden-cpm, leiden-mod, or louvain.");
        });
    threshold_consensus.add_argument("--final-resolution")
        .default_value(double(0.01))
        .help("Resolution value for the final run. Only used if --final-algorithm is leiden-cpm")
        .scan<'f', double>();
    threshold_consensus.add_argument("--partitions")
        .default_value(int(10))
        .help("Number of partitions in consensus clustering")
        .scan<'d', int>();
    threshold_consensus.add_argument("--num-processors")
        .default_value(int(1))
        .help("Number of processors")
        .scan<'d', int>();
    threshold_consensus.add_argument("--output-file")
        .required()
        .help("Output clustering file");
    threshold_consensus.add_argument("--log-file")
        .required()
        .help("Output log file");
    threshold_consensus.add_argument("--log-level")
        .default_value(int(1))
        .help("Log level where 0 = silent, 1 = info, 2 = verbose")
        .scan<'d', int>();

    main_program.add_subparser(simple_consensus);
    main_program.add_subparser(threshold_consensus);
    try {
        main_program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << main_program;
        std::exit(1);
    }

    if(main_program.is_subcommand_used(simple_consensus)) {
        std::string edgelist = simple_consensus.get<std::string>("--edgelist");
        std::string partition_file = simple_consensus.get<std::string>("--partition-file");
        std::string final_algorithm = simple_consensus.get<std::string>("--final-algorithm");
        double threshold = simple_consensus.get<double>("--threshold");
        double final_resolution = simple_consensus.get<double>("--final-resolution");
        double delta = simple_consensus.get<double>("--delta");
        int max_iter = simple_consensus.get<int>("--max-iter");
        int num_partitions = simple_consensus.get<int>("--partitions");
        int num_processors = simple_consensus.get<int>("--num-processors");
        std::string output_file = simple_consensus.get<std::string>("--output-file");
        std::string log_file = simple_consensus.get<std::string>("--log-file");
        int log_level = simple_consensus.get<int>("--log-level");
        Consensus* sc = new SimpleConsensus(edgelist, partition_file, final_algorithm, threshold, final_resolution, delta, max_iter, num_partitions, num_processors, output_file, log_file, log_level);
        sc->main();
        delete sc;
    } else if (main_program.is_subcommand_used(threshold_consensus)) {
        std::string edgelist = threshold_consensus.get<std::string>("--edgelist");
        std::string partition_file = threshold_consensus.get<std::string>("--partition-file");
        std::string final_algorithm = threshold_consensus.get<std::string>("--final-algorithm");
        double threshold = threshold_consensus.get<double>("--threshold");
        double final_resolution = threshold_consensus.get<double>("--final-resolution");
        int num_partitions = threshold_consensus.get<int>("--partitions");
        int num_processors = threshold_consensus.get<int>("--num-processors");
        std::string output_file = threshold_consensus.get<std::string>("--output-file");
        std::string log_file = threshold_consensus.get<std::string>("--log-file");
        int log_level = threshold_consensus.get<int>("--log-level");
        Consensus* tc = new ThresholdConsensus(edgelist, partition_file, final_algorithm, threshold, final_resolution, num_partitions, num_processors, output_file, log_file, log_level);
        tc->main();
        delete tc;
    }
}
