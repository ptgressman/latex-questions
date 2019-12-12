//
//  polynomials.h
//  
//
//  Created by Philip Gressman on 12/12/18.
//
#include <iostream>
using namespace std;
#include <cstdlib>
#include "iohandler.h"
#include "treesbigfour.h"
#include "treeparser.h"




int main(int argc, char * argv [])
{
    bool result;
    DataTree myroot("");
    IOHandler world;
    
    myroot.set_IOHandler(&world);
    
    
    IORequest getstuff;
    
    DataTree *tester;
    tester = &myroot;
    
    if (argc == 2) {
        getstuff.make_get_request(); getstuff.file = argv[1];
        world.handle(getstuff);
    } else { cout << "No file\n"; exit(-1);}
    
    if (getstuff.message == "") {cout << "Empty file\n"; exit(-1);}
    
    cout << "MSSG: Parsing....\n";
    
    result = DataTreeFromXML(tester,getstuff.message);
    
    if (!result) {exit(-1);}
    
    cout << "MSSG: Parsing finished.\n";
    
    //cout << tester->XMLFMT();
    
    string randseedstring = tester->get_tag()->get_attribute("randseed");
    if ( randseedstring != "") {
        tree_randseed = 0;
        int ctr;
        for(ctr = 0; ctr < randseedstring.length(); ctr++) {
            tree_randseed = 2 * tree_randseed + (int)(randseedstring[ctr]);
            if (ctr > 20) {ctr = randseedstring.length();}
        }
        //srand(tree_randseed);
        rand_gen = mt19937(tree_randseed);
    }


    cout << "MSSG: Executing...\n";
    result = myroot.execute_all();
    
    if (result) {cout << "MSSG: Execution finished: Success.\n";} else {cout << "MSSG: Execution finished: Failure.\n";}
    
    myroot.cleanup();
    
    
    world.cleanup();
    return 0;


}


    
    
    

