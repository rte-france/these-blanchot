//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_RUNNER
#include "catch2.hpp"

#include "functions_test.h"

int main(int argc, char *argv[]){

    Catch::Session session; // There must be exactly one instance

    std::string test_file = ""; // Some user variable you want to be able to set

    // Build a new parser on top of Catch's
    using namespace Catch::clara;
    auto cli
        = session.cli() // Get Catch's composite command line parser
        | Opt(test_file, "test file") // bind variable to a new option, with a hint string
        ["-t"]["--testfile"]    // the option names it will respond to
        ("test file describing the test to run"); // description string for the help output

  // Now pass the new composite back to Catch so it uses that
    session.cli(cli);

    // Let Catch (using Clara) parse the command line
    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0) // Indicates a command line error
        return returnCode;

    // test_file now has been set in its variable
    if (test_file != "")
        std::cout << "test file running ici : " << test_file << std::endl;



    return session.run();
}