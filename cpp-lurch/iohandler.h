

#ifndef iohandler_h
#define iohandler_h

#include <fstream>
#include <time.h>
const int MAX_OUTFILES = 30;
const int MAX_FILENAME = 40;
const int INTERNAL_BUFFER = 511;

class IORequest {
    public:
    IORequest() {pending = false; message = ""; file = ""; direction = "out"; pushall=false;}
    void make_send_request() {direction = "out"; pending=true;}
    void make_get_request() {direction = "in"; pending=true;}
    void make_pushallout() {pushall = true;}
    string message;
    string file;
    string direction;
    bool pending;
    bool pushall;
};

class IOHandler {
    public:
    IOHandler() {known_files = 0; memory = 0; last_action = time(0);}
    void handle(IORequest &delivery) {
        int i;
        if (delivery.pushall) {
            for(i=0;i<known_files;i++) pushbuffer(i,0);
            delivery.pending = false;
            last_action = time(0);
            return;
        }
        if (difftime(time(0),last_action) > 20.0) {
            for(i=0;i<known_files;i++) pushbuffer(i,0);
            last_action = time(0);
        }
        if (delivery.direction == "in") {
            ifstream the_file;
            bool filegood;
            string line;
            the_file.open(delivery.file);
            delivery.message = "";
            filegood = the_file.good();
            if (filegood) {
                while (getline(the_file,line)) {
                    delivery.message.append(line);
                    delivery.message += "\n";
                }
            }
            the_file.close();
            for(i=0;i<known_files;i++) {
                if (delivery.file==filenames[i]) {
                    filegood = true;
                    delivery.message = delivery.message + buffers[i];
                    delivery.pending = false;
                    //cout << "WARNING : Experimental internal filesystem usage.\n";
                }
            }
            if (!filegood) {
                {cout << "ERROR: Could not open file " << delivery.file << "\n"; exit(-1);}
            }
            delivery.pending = false;
            return;
        }
        if (known_files + 1 >= memory) {
            int newmem = (memory + 5) * 2;
            string *newfn = new string [newmem];
            string *newbf = new string [newmem];
            for(i=0;i<known_files;i++) {newfn[i] = filenames[i]; newbf[i] = buffers[i];}
            if (memory > 0) {delete[] filenames; delete[] buffers;}
            filenames = newfn; buffers = newbf;
            memory = newmem;
        }
        if (delivery.file.length() > MAX_FILENAME) {cout << "WARNING: Ignoring output to file with long name: " << delivery.file << "\n"; delivery.pending = false; return;}
        
        for(i=0;i<known_files;i++) {
            if (delivery.file == filenames[i]) {
                buffers[i] = buffers[i] + delivery.message;
                
                pushbuffer(known_files,INTERNAL_BUFFER);
                
                delivery.pending = false; return;
            }
        }
        
        if (known_files >= MAX_OUTFILES) {cout << "ERROR: Output exceeds maximum of " << MAX_OUTFILES << " files at once. \n"; exit(-1);}
        filenames[known_files] = delivery.file;
        buffers[known_files] = delivery.message;
        
        the_onefile.open(filenames[i]);
        if (the_onefile.is_open()) {the_onefile << buffers[i]; buffers[i] = "";}
        the_onefile.close();
        
        delivery.pending = false;
        known_files++;
    }
    void cleanup() {
        if (memory > 0) {for(int i=0;i<known_files;i++) pushbuffer(i,0); delete[] filenames;}
        known_files = 0; memory = 0;
    }
    void pushbuffer(int i, int size_threshold) {
        if (buffers[i].length() > size_threshold) {
            the_onefile.open(filenames[i],std::ios_base::app);
            if (the_onefile.is_open()) {the_onefile << buffers[i]; buffers[i] = "";}
            the_onefile.close();
        }
    }
    private:
    string *filenames;
    string *buffers;
    ofstream the_onefile;
    int known_files, memory;
    time_t last_action;
};


#endif
