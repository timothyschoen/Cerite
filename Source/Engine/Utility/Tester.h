//
//  main.cpp
//  Cerite
//
//  Created by Timothy Schoen on 23/10/2020.
//  Simple demo of library

#include <string>
#include <fstream>
#include <streambuf>
#include <math.h>
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include "Library.h"
#include "Exporter.h"
#include "Parser.h"

namespace fs = std::__fs::filesystem;


struct Tester {
    
    Cerite::Library library = Cerite::Library("../../../circuits/");
    
    Cerite::Patch* loadFuzzface() {
        
        Cerite::Document fuzzface(Cerite::Document::concat("Fuzzface", {
            library.get("probe"),
            library.get("input"),
            library.get("capacitor", {2.2e-6}),
            library.get("resistor", {100e3}),
            library.get("input", {9}),
            
            library.get("resistor", {330}),
            library.get("resistor", {33e3}),
            library.get("resistor", {8200}),
            
            library.get("capacitor", {10e-9}),
            library.get("resistor", {250e3}),
            library.get("resistor", {250e3}),
            library.get("resistor", {500}),
            library.get("resistor", {500}),
            library.get("capacitor", {20e-6}),
            
            library.get("bjt"),
            library.get("bjt"),
            library.get("solver_lu")
        },
        {{1, 0}, {4, 0}, {4, 5}, {5, 6}, {3, 0}, {3, 7}, {3, 9}, {7, 7}, {7, 2}, {2, 1}, {1, 0}, {6, 8}, {8, 0}, {8, 0}, {5, 0, 9}, {9, 6, 7}, {}}));
       
        Cerite::Exporter::toC(fuzzface);
        
        return fuzzface.compile<Cerite::Patch>();
    }
    
    
    Cerite::Patch* loadLowpass() {
        
        //std::string netlist = Cerite::Library::readFile("../../../netlists/netlist.nadl");
        //Cerite::Document rctest = Cerite::Parser::parseNetlist(netlist, library);
        
        Cerite::Document lowpass = Cerite::Parser::parseNetlist("Lowpass \nresistor 2 1 4700 \ncapacitor 1 0 0.000001\n probe 1 0 \ninput 2 0\nsolver_lu", library);
        
        Cerite::Exporter::toC(lowpass);

        
        return lowpass.compile<Cerite::Patch>();
    }
    /*
    Cerite::Patch* loadInductorTest() {
        Cerite::Document res10k = resistor.withArgument("r", 10000);
        Cerite::Document ind = inductor.withArgument("l", 470e-3);
        
        Cerite::Document combined(Cerite::Document::concat("RLHighpass", {res10k, ind, probe, input}, {{2, 1}, {1, 0}, {1, 0}, {2, 0}}));
        
        return Cerite::Compiler::compile(combined);
    }
    
    Cerite::Patch* loadOPTest() {
        
        Cerite::Document resistor4700 = resistor.withArgument("r", 4700);
        Cerite::Document capacitor002 = capacitor.withArgument("c", 4.7e-5);
        
        Cerite::std::string r("r");
        Cerite::std::string c("c");
        
        Cerite::Document combined = Cerite::Document::concat("OPAMP",
                                                         {input,
            resistor,
            resistor,
            resistor.withArgument(r, 1000),
            resistor.withArgument(r, 2300),
            capacitor.withArgument(c, 0.000001),
            capacitor.withArgument(c, 0.000001),
            opamp.withArgument("g", 15).withArgument("umax", 10),
            probe,
            solver,
        }, {{3, 0}, {3, 4}, {4, 2}, {1, 0}, {1, 5}, {2, 0}, {4, 1}, {2, 1, 5}, {5, 0}, {}});
        
        return Cerite::Compiler::compile(combined);
    }
     */
    
    Cerite::Patch* loadOctaver() {
        
        Cerite::Document octaver = Cerite::Document::concat("Octaver",
                                                         {
            library.get("probe"),
            library.get("input"),
            library.get("diode"),
            library.get("diode"),
            library.get("diode"),
            library.get("diode"),
            library.get("solver_pivot"),
        }, {{3, 0}, {1, 2}, {0, 1}, {1, 3}, {2, 3}, {2, 0}, {}});
        
        Cerite::Exporter::toC(octaver);

        
        return octaver.compile<Cerite::Patch>();
    }
     
};
