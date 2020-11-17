//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_RUNNER
#pragma once
#include "catch2.hpp"

#include "define_datas.hpp"

int main(int argc, char *argv[]){

    Catch::Session session; // There must be exactly one instance

    return session.run();
}