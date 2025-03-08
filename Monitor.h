#include <systemc.h>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

SC_MODULE( Monitor ) {	
	sc_in < bool > rst;
    sc_in < float > fc_8st [1000];

    void print_output(){
        float f1 = 0, f2 = 0, f3 = 0, f4 = 0, f5 = 0;
        int idx1 = 0, idx2 = 0, idx3 = 0, idx4 = 0, idx5 = 0;
        string s1, s2, s3, s4, s5, temp_str;

        ifstream class_name_file("imagenet_classes.txt");
        if(!class_name_file){
            cout << "Can't open imagenet_classes.txt for reading\n";
        }
        cout<<"||=========================================================||\n";
        cout<<left<<"||  "<<setw(3)<<"idx"<<"  |    "<<setw(7)<<"value"<<"   |      "<<setw(20)<<"class name"<<"        ||\n";
        cout<<"||=========================================================||\n";
        for(int i=0;i<1000;i++){
            getline(class_name_file, temp_str);
            if(fc_8st[i] > f1){
                f5 = f4;
                f4 = f3;
                f3 = f2;
                f2 = f1;
                f1 = fc_8st[i];
                idx5 = idx4;
                idx4 = idx3;
                idx3 = idx2;
                idx2 = idx1;
                idx1 = i;
                s5 = s4;
                s4 = s3;
                s3 = s2;
                s2 = s1;
                s1 = temp_str;
            }
            else if(fc_8st[i] > f2){
                f5 = f4;
                f4 = f3;
                f3 = f2;
                f2 = fc_8st[i];
                idx5 = idx4;
                idx4 = idx3;
                idx3 = idx2;
                idx2 = i;
                s5 = s4;
                s4 = s3;
                s3 = s2;
                s2 = temp_str;
            }
            else if(fc_8st[i] > f3){
                f5 = f4;
                f4 = f3;
                f3 = fc_8st[i];
                idx5 = idx4;
                idx4 = idx3;
                idx3 = i;
                s5 = s4;
                s4 = s3;
                s3 = temp_str;
            }
            else if(fc_8st[i] > f4){
                f5 = f4;
                f4 = fc_8st[i];
                idx5 = idx4;
                idx4 = i;
                s5 = s4;
                s4 = temp_str;
            }
            else if(fc_8st[i] > f5){
                f5 = fc_8st[i];
                idx5 = i;
                s5 = temp_str;
            }
        }
        cout<<left<<"||  "<<setw(3)<<idx1<<"  |    "<<setw(7)<<f1<<"   |      "<<setw(20)<<s1<<"        ||\n";
        cout<<left<<"||  "<<setw(3)<<idx2<<"  |    "<<setw(7)<<f2<<"   |      "<<setw(20)<<s2<<"        ||\n";
        cout<<left<<"||  "<<setw(3)<<idx3<<"  |    "<<setw(7)<<f3<<"   |      "<<setw(20)<<s3<<"        ||\n";
        cout<<left<<"||  "<<setw(3)<<idx4<<"  |    "<<setw(7)<<f4<<"   |      "<<setw(20)<<s4<<"        ||\n";
        cout<<left<<"||  "<<setw(3)<<idx5<<"  |    "<<setw(7)<<f5<<"   |      "<<setw(20)<<s5<<"        ||\n";
        cout<<"||=========================================================||\n";
        class_name_file.close();
}
	
	SC_CTOR( Monitor )
	{	
		SC_METHOD( print_output );
		sensitive << rst.neg();
		dont_initialize();
	}
};

