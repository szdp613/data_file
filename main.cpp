#include "clockreset.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

using namespace std;

SC_MODULE( Pattern ) {	

	sc_in < bool > rst;
	sc_in_clk clock;
	sc_out < bool > image_valid;

	bool set_or_not; 
		
	void run() {
		if ( rst.read() == 1 ) {
			set_or_not = false;
		}
		else if ( rst.read() == 0 && set_or_not == false) {
			image_valid.write(1);
			set_or_not = true;
		}
		else if ( rst.read() == 0 && set_or_not == true ) {
			image_valid.write(0);
		}
	}
	
	SC_CTOR( Pattern )
	{	
		SC_METHOD( run );
		sensitive << clock.pos();
		dont_initialize();
	}
};

SC_MODULE( Pad_Conv1_MaxP1 ) {	

	sc_in < bool > rst;
	sc_in_clk clock;
	sc_in < bool > image_valid;
	sc_out < bool > stage1_valid;

	float image_pad [3][227][227];
	float conv1_ker [3][11][11];
	float conv1_bias;
	float conv1_out [64][55][55];
	float stage1_out;
		
	void run() {

		float temp;

		if ( rst.read() == 1 ) {
			for(int i = 0; i < 3; i++)
				for(int j = 0; j < 224; j++)
					for(int k = 0; k < 224; k++)
						image_pad[i][j][k] = 0;
		}
		else if ( image_valid.read() == true ) {

			ifstream input_file("data/dog.txt");
			if (input_file.is_open()) {
				
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 224; j++) {
						for (int k = 0; k < 224; k++) {
							input_file >> temp;
							image_pad[i][j+2][k+2] = temp;
						}
					}
				}
				input_file.close();
			} 
			else {
				cout << "Failed of open input image!!" << endl;
				return ;
			}

			cout << fixed << setprecision(25) << image_pad[0][2][2] << endl;
			cout << fixed << setprecision(25) << image_pad[0][2][3] << endl;
			cout << fixed << setprecision(25) << image_pad[2][225][225] << endl;

			ifstream weight1_file("data/conv1_weight.txt");
			ifstream bias1_file("data/conv1_bias.txt");
			if (weight1_file.is_open() && bias1_file.is_open()) {
				for (int z = 0; z < 64; z++) {

					for (int i = 0; i < 3; i++) {
						for (int j = 0; j < 11; j++) {
							for (int k = 0; k < 11; k++) {
								weight1_file >> temp;
								conv1_ker[i][j][k] = temp;
							}
						}
					}
					bias1_file >> temp;
					conv1_bias = temp;

					for (int i = 0; i < 55; i++) {
						for (int j = 0; j < 55; j++) {

							conv1_out[z][i][j] = 0;

							for (int k = 0; k < 3; k++) {
								for (int l = 0; l < 11; l++) {
									for (int m = 0; m < 11; m++) {
										conv1_out[z][i][j] += image_pad[k][i*4+l][j*4+m] * conv1_ker[k][l][m];
									}
								}
							}

							if ( (z == 0 && i == 0 && j == 0) || (z == 0 && i == 0 && j == 1) || (z == 63 && i == 54 && j == 54))
								cout << "test1 : " << conv1_out[z][i][j];

							conv1_out[z][i][j] += conv1_bias;
							if( conv1_out[z][i][j] < 0) 
								conv1_out[z][i][j] = 0;

							if ( (z == 0 && i == 0 && j == 0) || (z == 0 && i == 0 && j == 1) || (z == 63 && i == 54 && j == 54))
								cout << "test2 : "  << conv1_out[z][i][j];

						}
					}

				}
				weight1_file.close();
				bias1_file.close();
			} 
			else {
				cout << "Failed of open conv1 weight and/or bias!!" << endl;
				return ;
			}

			ofstream stage1_file("stage1_output.txt");
			if (stage1_file.is_open()) {

				for (int i = 0; i < 64; i++) {
					for (int j = 0; j < 27; j++) {
						for (int k = 0; k < 27; k++) {

							stage1_out = 1000000;
							for (int l = 0; l < 3; l++) {
								for (int m = 0; m < 3; m++) {
									if(conv1_out[i][j*2+l][k*2+m] < stage1_out) {
										stage1_out = conv1_out[i][j*2+l][k*2+m];
									}
								}
							}

							stage1_file << fixed << setprecision(25) << stage1_out << "\t";
						}
					}
					stage1_file << "\n";
				}

				stage1_file.close();
			}
			else {
				cout << "Failed of open stage1 image!!" << endl;
				return ;
			}

			stage1_valid.write(1);
		}
		else if ( image_valid.read() == false ) {
			stage1_valid.write(0);
		}
	}
	
	SC_CTOR( Pad_Conv1_MaxP1 )
	{	
		SC_METHOD( run );
		sensitive << clock.pos();
		dont_initialize();
	}
};

SC_MODULE( Pad_Conv2_MaxP2 ) {	

	sc_in < bool > rst;
	sc_in_clk clock;
	sc_in < bool > stage1_valid;
	sc_out < bool > stage2_valid;

	float image_pad [64][31][31];
	float conv2_ker [64][5][5];
	float conv2_bias;
	float conv2_out [192][27][27];
	float stage2_out;
		
	void run() {

		float temp;

		if ( rst.read() == 1 ) {
			for(int i = 0; i < 64; i++)
				for(int j = 0; j < 31; j++)
					for(int k = 0; k < 31; k++)
						image_pad[i][j][k] = 0;
		}
		else if ( stage1_valid.read() == true ) {

			ifstream input_file("stage1_output.txt");
			if (input_file.is_open()) {
				
				for (int i = 0; i < 64; i++) {
					for (int j = 0; j < 27; j++) {
						for (int k = 0; k < 27; k++) {
							input_file >> temp;
							image_pad[i][j+2][k+2] = temp;
						}
					}
				}
				input_file.close();
			} 
			else {
				cout << "Failed of open stage1 image!!" << endl;
				return ;
			}

			cout << fixed << setprecision(25) << image_pad[0][2][2] << endl;
			cout << fixed << setprecision(25) << image_pad[0][2][3] << endl;
			cout << fixed << setprecision(25) << image_pad[63][28][28] << endl;

			ifstream weight2_file("data/conv2_weight.txt");
			ifstream bias2_file("data/conv2_bias.txt");
			if (weight2_file.is_open() && bias2_file.is_open()) {
				for (int z = 0; z < 192; z++) {

					for (int i = 0; i < 64; i++) {
						for (int j = 0; j < 5; j++) {
							for (int k = 0; k < 5; k++) {
								weight2_file >> temp;
								conv2_ker[i][j][k] = temp;
							}
						}
					}
					bias2_file >> temp;
					conv2_bias = temp;

					for (int i = 0; i < 27; i++) {
						for (int j = 0; j < 27; j++) {

							conv2_out[z][i][j] = 0;

							for (int k = 0; k < 64; k++) {
								for (int l = 0; l < 5; l++) {
									for (int m = 0; m < 5; m++) {
										conv2_out[z][i][j] += image_pad[k][i+l][j+m] * conv2_ker[k][l][m];
									}
								}
							}

							conv2_out[z][i][j] += conv2_bias;
							if( conv2_out[z][i][j] < 0) 
								conv2_out[z][i][j] = 0;

						}
					}

				}
				weight2_file.close();
				bias2_file.close();
			} 
			else {
				cout << "Failed of open conv2 weight and/or bias!!" << endl;
				return ;
			}

			ofstream stage2_file("stage2_output.txt");
			if (stage2_file.is_open()) {

				for (int i = 0; i < 192; i++) {
					for (int j = 0; j < 13; j++) {
						for (int k = 0; k < 13; k++) {

							stage2_out = 1000000;
							for (int l = 0; l < 3; l++) {
								for (int m = 0; m < 3; m++) {
									if(conv2_out[i][j*2+l][k*2+m] < stage2_out) {
										stage2_out = conv2_out[i][j*2+l][k*2+m];
									}
								}
							}

							stage2_file << fixed << setprecision(25) << stage2_out << "\t";
						}
					}
					stage2_file << "\n";
				}

				stage2_file.close();
			}
			else {
				cout << "Failed of open stage2 image!!" << endl;
				return ;
			}

			stage2_valid.write(1);
		}
		else if ( stage1_valid.read() == false ) {
			stage2_valid.write(0);
		}
	}
	
	SC_CTOR( Pad_Conv2_MaxP2 )
	{	
		SC_METHOD( run );
		sensitive << clock.pos();
		dont_initialize();
	}
};

SC_MODULE( Pad_Conv3 ) {	

	sc_in < bool > rst;
	sc_in_clk clock;
	sc_in < bool > stage2_valid;
	sc_out < bool > stage3_valid;

	float image_pad [192][15][15];
	float conv3_ker [192][3][3];
	float conv3_bias;
	float conv3_out [384][13][13];
		
	void run() {

		float temp;

		if ( rst.read() == 1 ) {
			for(int i = 0; i < 192; i++)
				for(int j = 0; j < 15; j++)
					for(int k = 0; k < 15; k++)
						image_pad[i][j][k] = 0;
		}
		else if ( stage2_valid.read() == true ) {

			ifstream input_file("stage2_output.txt");
			if (input_file.is_open()) {
				
				for (int i = 0; i < 192; i++) {
					for (int j = 0; j < 13; j++) {
						for (int k = 0; k < 13; k++) {
							input_file >> temp;
							image_pad[i][j+1][k+1] = temp;
						}
					}
				}
				input_file.close();
			} 
			else {
				cout << "Failed of open stage2 image!!" << endl;
				return ;
			}

			cout << fixed << setprecision(25) << image_pad[0][1][1] << endl;
			cout << fixed << setprecision(25) << image_pad[0][1][2] << endl;
			cout << fixed << setprecision(25) << image_pad[191][13][13] << endl;

			ifstream weight3_file("data/conv3_weight.txt");
			ifstream bias3_file("data/conv3_bias.txt");
			if (weight3_file.is_open() && bias3_file.is_open()) {
				for (int z = 0; z < 384; z++) {

					for (int i = 0; i < 192; i++) {
						for (int j = 0; j < 3; j++) {
							for (int k = 0; k < 3; k++) {
								weight3_file >> temp;
								conv3_ker[i][j][k] = temp;
							}
						}
					}
					bias3_file >> temp;
					conv3_bias = temp;

					for (int i = 0; i < 13; i++) {
						for (int j = 0; j < 13; j++) {

							conv3_out[z][i][j] = 0;

							for (int k = 0; k < 192; k++) {
								for (int l = 0; l < 3; l++) {
									for (int m = 0; m < 3; m++) {
										conv3_out[z][i][j] += image_pad[k][i+l][j+m] * conv3_ker[k][l][m];
									}
								}
							}

							conv3_out[z][i][j] +=  conv3_bias;
							if( conv3_out[z][i][j] < 0) 
								conv3_out[z][i][j] = 0;

						}
					}

				}
				weight3_file.close();
				bias3_file.close();
			} 
			else {
				cout << "Failed of open conv3 weight and/or bias!!" << endl;
				return ;
			}

			ofstream stage3_file("stage3_output.txt");
			if (stage3_file.is_open()) {

				for (int i = 0; i < 384; i++) {
					for (int j = 0; j < 13; j++) {
						for (int k = 0; k < 13; k++) {
							stage3_file << fixed << setprecision(25) << conv3_out[i][j][k] << "\t";
						}
					}
					stage3_file << "\n";
				}

				stage3_file.close();
			}
			else {
				cout << "Failed of open stage3 image!!" << endl;
				return ;
			}

			stage3_valid.write(1);
		}
		else if ( stage2_valid.read() == false ) {
			stage3_valid.write(0);
		}
	}
	
	SC_CTOR( Pad_Conv3 )
	{	
		SC_METHOD( run );
		sensitive << clock.pos();
		dont_initialize();
	}
};

SC_MODULE( Pad_Conv4 ) {	

	sc_in < bool > rst;
	sc_in_clk clock;
	sc_in < bool > stage3_valid;
	sc_out < bool > stage4_valid;

	float image_pad [384][15][15];
	float conv4_ker [384][3][3];
	float conv4_bias;
	float conv4_out [256][13][13];
		
	void run() {

		float temp;

		if ( rst.read() == 1 ) {
			for(int i = 0; i < 384; i++)
				for(int j = 0; j < 15; j++)
					for(int k = 0; k < 15; k++)
						image_pad[i][j][k] = 0;
		}
		else if ( stage3_valid.read() == true ) {

			ifstream input_file("stage3_output.txt");
			if (input_file.is_open()) {
				
				for (int i = 0; i < 384; i++) {
					for (int j = 0; j < 13; j++) {
						for (int k = 0; k < 13; k++) {
							input_file >> temp;
							image_pad[i][j+1][k+1] = temp;
						}
					}
				}
				input_file.close();
			} 
			else {
				cout << "Failed of open stage3 image!!" << endl;
				return ;
			}

			cout << fixed << setprecision(25) << image_pad[0][1][1] << endl;
			cout << fixed << setprecision(25) << image_pad[0][1][2] << endl;
			cout << fixed << setprecision(25) << image_pad[383][13][13] << endl;

			ifstream weight4_file("data/conv4_weight.txt");
			ifstream bias4_file("data/conv4_bias.txt");
			if (weight4_file.is_open() && bias4_file.is_open()) {
				for (int z = 0; z < 256; z++) {

					for (int i = 0; i < 384; i++) {
						for (int j = 0; j < 3; j++) {
							for (int k = 0; k < 3; k++) {
								weight4_file >> temp;
								conv4_ker[i][j][k] = temp;
							}
						}
					}
					bias4_file >> temp;
					conv4_bias = temp;

					for (int i = 0; i < 13; i++) {
						for (int j = 0; j < 13; j++) {

							conv4_out[z][i][j] = 0;

							for (int k = 0; k < 384; k++) {
								for (int l = 0; l < 3; l++) {
									for (int m = 0; m < 3; m++) {
										conv4_out[z][i][j] += image_pad[k][i+l][j+m] * conv4_ker[k][l][m];
									}
								}
							}

							conv4_out[z][i][j] += conv4_bias;
							if( conv4_out[z][i][j] < 0) 
								conv4_out[z][i][j] = 0;

						}
					}

				}
				weight4_file.close();
				bias4_file.close();
			} 
			else {
				cout << "Failed of open conv4 weight and/or bias!!" << endl;
				return ;
			}

			ofstream stage4_file("stage4_output.txt");
			if (stage4_file.is_open()) {

				for (int i = 0; i < 256; i++) {
					for (int j = 0; j < 13; j++) {
						for (int k = 0; k < 13; k++) {
							stage4_file << fixed << setprecision(25) << conv4_out[i][j][k] << "\t";
						}
					}
					stage4_file << "\n";
				}

				stage4_file.close();
			}
			else {
				cout << "Failed of open stage4 image!!" << endl;
				return ;
			}

			stage4_valid.write(1);
		}
		else if ( stage3_valid.read() == false ) {
			stage4_valid.write(0);
		}
	}
	
	SC_CTOR( Pad_Conv4 )
	{	
		SC_METHOD( run );
		sensitive << clock.pos();
		dont_initialize();
	}
};

SC_MODULE( Pad_Conv5_MaxP5 ) {	

	sc_in < bool > rst;
	sc_in_clk clock;
	sc_in < bool > stage4_valid;
	sc_out < bool > stage5_valid;

	float image_pad [256][15][15];
	float conv5_ker [256][3][3];
	float conv5_bias;
	float conv5_out [256][13][13];
	float stage5_out;
		
	void run() {

		float temp;

		if ( rst.read() == 1 ) {
			for(int i = 0; i < 256; i++)
				for(int j = 0; j < 15; j++)
					for(int k = 0; k < 15; k++)
						image_pad[i][j][k] = 0;
		}
		else if ( stage4_valid.read() == true ) {

			ifstream input_file("stage4_output.txt");
			if (input_file.is_open()) {
				
				for (int i = 0; i < 256; i++) {
					for (int j = 0; j < 13; j++) {
						for (int k = 0; k < 13; k++) {
							input_file >> temp;
							image_pad[i][j+1][k+1] = temp;
						}
					}
				}
				input_file.close();
			} 
			else {
				cout << "Failed of open stage4 image!!" << endl;
				return ;
			}

			cout << fixed << setprecision(25) << image_pad[0][1][1] << endl;
			cout << fixed << setprecision(25) << image_pad[0][1][2] << endl;
			cout << fixed << setprecision(25) << image_pad[255][13][13] << endl;

			ifstream weight5_file("data/conv5_weight.txt");
			ifstream bias5_file("data/conv5_bias.txt");
			if (weight5_file.is_open() && bias5_file.is_open()) {
				for (int z = 0; z < 256; z++) {

					for (int i = 0; i < 256; i++) {
						for (int j = 0; j < 3; j++) {
							for (int k = 0; k < 3; k++) {
								weight5_file >> temp;
								conv5_ker[i][j][k] = temp;
							}
						}
					}
					bias5_file >> temp;
					conv5_bias = temp;

					for (int i = 0; i < 13; i++) {
						for (int j = 0; j < 13; j++) {

							conv5_out[z][i][j] = 0;

							for (int k = 0; k < 256; k++) {
								for (int l = 0; l < 3; l++) {
									for (int m = 0; m < 3; m++) {
										conv5_out[z][i][j] += image_pad[k][i+l][j+m] * conv5_ker[k][l][m];
									}
								}
							}

							conv5_out[z][i][j] += conv5_bias;
							if( conv5_out[z][i][j] < 0) 
								conv5_out[z][i][j] = 0;

						}
					}

				}
				weight5_file.close();
				bias5_file.close();
			} 
			else {
				cout << "Failed of open conv5 weight and/or bias!!" << endl;
				return ;
			}

			ofstream stage5_file("stage5_output.txt");
			if (stage5_file.is_open()) {

				for (int i = 0; i < 256; i++) {
					for (int j = 0; j < 6; j++) {
						for (int k = 0; k < 6; k++) {

							stage5_out = 1000000;
							for (int l = 0; l < 3; l++) {
								for (int m = 0; m < 3; m++) {
									if(conv5_out[i][j*2+l][k*2+m] < stage5_out) {
										stage5_out = conv5_out[i][j*2+l][k*2+m];
									}
								}
							}

							stage5_file << fixed << setprecision(25) << stage5_out << "\t";
						}
					}
					stage5_file << "\n";
				}

				stage5_file.close();
			}
			else {
				cout << "Failed of open stage5 image!!" << endl;
				return ;
			}

			stage5_valid.write(1);
		}
		else if ( stage4_valid.read() == false ) {
			stage5_valid.write(0);
		}
	}
	
	SC_CTOR( Pad_Conv5_MaxP5 )
	{	
		SC_METHOD( run );
		sensitive << clock.pos();
		dont_initialize();
	}
};

SC_MODULE( FC6 ) {	

	sc_in < bool > rst;
	sc_in_clk clock;
	sc_in < bool > stage5_valid;
	sc_out < bool > stage6_valid;

	float image_pad [9216];
	float fc6_ker [9216];
	float fc6_bias;
	float fc6_out [4096];
		
	void run() {

		float temp;

		if ( rst.read() == 1 ) {
		}
		else if ( stage5_valid.read() == true ) {

			ifstream input_file("stage5_output.txt");
			if (input_file.is_open()) {
				
				for (int i = 0; i < 9216; i++) {
					input_file >> temp;
					image_pad[i] = temp;
				}
				input_file.close();
			} 
			else {
				cout << "Failed of open stage5 image!!" << endl;
				return ;
			}

			cout << fixed << setprecision(25) << image_pad[0] << endl;
			cout << fixed << setprecision(25) << image_pad[1] << endl;
			cout << fixed << setprecision(25) << image_pad[9215] << endl;

			ifstream weight6_file("data/fc6_weight.txt");
			ifstream bias6_file("data/fc6_bias.txt");
			if (weight6_file.is_open() && bias6_file.is_open()) {
				for (int z = 0; z < 4096; z++) {

					for (int i = 0; i < 9216; i++) {
						weight6_file >> temp;
						fc6_ker[i] = temp;
					}
					bias6_file >> temp;
					fc6_bias = temp;


					fc6_out[z] = 0;

					for (int j = 0; j < 9216; j++) 
						fc6_out[z] += image_pad[j] * fc6_ker[j];

					fc6_out[z] += fc6_bias;

					if( fc6_out[z] < 0) 
						fc6_out[z] = 0;

					

				}
				weight6_file.close();
				bias6_file.close();
			} 
			else {
				cout << "Failed of open fc6 weight and/or bias!!" << endl;
				return ;
			}

			ofstream stage6_file("stage6_output.txt");
			if (stage6_file.is_open()) {

				for (int i = 0; i < 4096; i++) {
					stage6_file << fixed << setprecision(25) << fc6_out[i] << "\n";
				}

				stage6_file.close();
			}
			else {
				cout << "Failed of open stage6 image!!" << endl;
				return ;
			}

			stage6_valid.write(1);
		}
		else if ( stage5_valid.read() == false ) {
			stage6_valid.write(0);
		}
	}
	
	SC_CTOR( FC6 )
	{	
		SC_METHOD( run );
		sensitive << clock.pos();
		dont_initialize();
	}
};

SC_MODULE( FC7 ) {	

	sc_in < bool > rst;
	sc_in_clk clock;
	sc_in < bool > stage6_valid;
	sc_out < bool > stage7_valid;

	float image_pad [4096];
	float fc7_ker [4096];
	float fc7_bias;
	float fc7_out [4096];
		
	void run() {

		float temp;

		if ( rst.read() == 1 ) {
		}
		else if ( stage6_valid.read() == true ) {

			ifstream input_file("stage6_output.txt");
			if (input_file.is_open()) {
				
				for (int i = 0; i < 4096; i++) {
					input_file >> temp;
					image_pad[i] = temp;
				}
				input_file.close();
			} 
			else {
				cout << "Failed of open stage6 image!!" << endl;
				return ;
			}

			cout << fixed << setprecision(25) << image_pad[0] << endl;
			cout << fixed << setprecision(25) << image_pad[1] << endl;
			cout << fixed << setprecision(25) << image_pad[4095] << endl;

			ifstream weight7_file("data/fc7_weight.txt");
			ifstream bias7_file("data/fc7_bias.txt");
			if (weight7_file.is_open() && bias7_file.is_open()) {
				for (int z = 0; z < 4096; z++) {

					for (int i = 0; i < 4096; i++) {
						weight7_file >> temp;
						fc7_ker[i] = temp;
					}
					bias7_file >> temp;
					fc7_bias = temp;


					fc7_out[z] = 0;

					for (int j = 0; j < 4096; j++) 
						fc7_out[z] += image_pad[j] * fc7_ker[j];

					fc7_out[z] += fc7_bias;

					if( fc7_out[z] < 0) 
						fc7_out[z] = 0;

					

				}
				weight7_file.close();
				bias7_file.close();
			} 
			else {
				cout << "Failed of open fc7 weight and/or bias!!" << endl;
				return ;
			}

			ofstream stage7_file("stage7_output.txt");
			if (stage7_file.is_open()) {

				for (int i = 0; i < 4096; i++) {
					stage7_file << fixed << setprecision(25) << fc7_out[i] << "\n";
				}

				stage7_file.close();
			}
			else {
				cout << "Failed of open stage7 image!!" << endl;
				return ;
			}

			stage7_valid.write(1);
		}
		else if ( stage6_valid.read() == false ) {
			stage7_valid.write(0);
		}
	}
	
	SC_CTOR( FC7 )
	{	
		SC_METHOD( run );
		sensitive << clock.pos();
		dont_initialize();
	}
};

SC_MODULE( FC8 ) {	

	sc_in < bool > rst;
	sc_in_clk clock;
	sc_in < bool > stage7_valid;
	sc_out < bool > stage8_valid;

	float image_pad [4096];
	float fc8_ker [4096];
	float fc8_bias;
	float fc8_out [1000];
		
	void run() {

		float temp;

		if ( rst.read() == 1 ) {
		}
		else if ( stage7_valid.read() == true ) {

			ifstream input_file("stage7_output.txt");
			if (input_file.is_open()) {
				
				for (int i = 0; i < 4096; i++) {
					input_file >> temp;
					image_pad[i] = temp;
				}
				input_file.close();
			} 
			else {
				cout << "Failed of open stage7 image!!" << endl;
				return ;
			}

			cout << fixed << setprecision(25) << image_pad[0] << endl;
			cout << fixed << setprecision(25) << image_pad[1] << endl;
			cout << fixed << setprecision(25) << image_pad[4095] << endl;

			ifstream weight8_file("data/fc8_weight.txt");
			ifstream bias8_file("data/fc8_bias.txt");
			if (weight8_file.is_open() && bias8_file.is_open()) {
				for (int z = 0; z < 1000; z++) {

					for (int i = 0; i < 4096; i++) {
						weight8_file >> temp;
						fc8_ker[i] = temp;
					}
					bias8_file >> temp;
					fc8_bias = temp;


					fc8_out[z] = 0;

					for (int j = 0; j < 4096; j++) 
						fc8_out[z] += image_pad[j] * fc8_ker[j];

					fc8_out[z] += fc8_bias;

				}

				float total = 0;
				for (int i = 0; i < 1000; i++) {
					total += exp(fc8_out[i]);
				}
				for (int i = 0; i < 1000; i++) {
					fc8_out[i] = exp(fc8_out[i]) / total;
				}

				weight8_file.close();
				bias8_file.close();
			} 
			else {
				cout << "Failed of open fc8 weight and/or bias!!" << endl;
				return ;
			}

			ofstream stage8_file("stage8_output.txt");
			if (stage8_file.is_open()) {

				for (int i = 0; i < 1000; i++) {
					stage8_file << fixed << setprecision(25) << fc8_out[i] << "\n";
				}

				stage8_file.close();
			}
			else {
				cout << "Failed of open stage8 image!!" << endl;
				return ;
			}

			stage8_valid.write(1);
		}
		else if ( stage7_valid.read() == false ) {
			stage8_valid.write(0);
		}
	}
	
	SC_CTOR( FC8 )
	{	
		SC_METHOD( run );
		sensitive << clock.pos();
		dont_initialize();
	}
};

int sc_main( int argc, char* argv[] ) {
	sc_signal < bool > clk, rst;
	sc_signal < bool > image_valid;
	sc_signal < bool > stage1_valid;
	sc_signal < bool > stage2_valid;
	sc_signal < bool > stage3_valid;
	sc_signal < bool > stage4_valid;
	sc_signal < bool > stage5_valid;
	sc_signal < bool > stage6_valid;
	sc_signal < bool > stage7_valid;
	sc_signal < bool > stage8_valid;
	
	Reset m_Reset( "m_Reset", 10 );
	Clock m_clock( "m_clock", 5, 200 );
	Pattern m_Pattern( "m_Pattern" );
	Pad_Conv1_MaxP1 m_Pad_Conv1_MaxP1( "m_Pad_Conv1_MaxP1" );
	Pad_Conv2_MaxP2 m_Pad_Conv2_MaxP2( "m_Pad_Conv2_MaxP2" );
	Pad_Conv3 m_Pad_Conv3( "m_Pad_Conv3" );
	Pad_Conv4 m_Pad_Conv4( "m_Pad_Conv4" );
	Pad_Conv5_MaxP5 m_Pad_Conv5_MaxP5( "m_Pad_Conv5_MaxP5" );
	FC6 m_FC6( "m_FC6" );
	FC7 m_FC7( "m_FC7" );
	FC8 m_FC8( "m_FC8" );

	m_Reset( rst );
	m_clock( clk );

	m_Pattern.rst( rst );
	m_Pattern.clock( clk );
	m_Pattern.image_valid( image_valid );

	m_Pad_Conv1_MaxP1.rst( rst );
	m_Pad_Conv1_MaxP1.clock( clk );
	m_Pad_Conv1_MaxP1.image_valid( image_valid );
	m_Pad_Conv1_MaxP1.stage1_valid( stage1_valid );

	m_Pad_Conv2_MaxP2.rst( rst );
	m_Pad_Conv2_MaxP2.clock( clk );
	m_Pad_Conv2_MaxP2.stage1_valid( stage1_valid );
	m_Pad_Conv2_MaxP2.stage2_valid( stage2_valid );

	m_Pad_Conv3.rst( rst );
	m_Pad_Conv3.clock( clk );
	m_Pad_Conv3.stage2_valid( stage2_valid );
	m_Pad_Conv3.stage3_valid( stage3_valid );

	m_Pad_Conv4.rst( rst );
	m_Pad_Conv4.clock( clk );
	m_Pad_Conv4.stage3_valid( stage3_valid );
	m_Pad_Conv4.stage4_valid( stage4_valid );

	m_Pad_Conv5_MaxP5.rst( rst );
	m_Pad_Conv5_MaxP5.clock( clk );
	m_Pad_Conv5_MaxP5.stage4_valid( stage4_valid );
	m_Pad_Conv5_MaxP5.stage5_valid( stage5_valid );

	m_FC6.rst( rst );
	m_FC6.clock( clk );
	m_FC6.stage5_valid( stage5_valid );
	m_FC6.stage6_valid( stage6_valid );

	m_FC7.rst( rst );
	m_FC7.clock( clk );
	m_FC7.stage6_valid( stage6_valid );
	m_FC7.stage7_valid( stage7_valid );

	m_FC8.rst( rst );
	m_FC8.clock( clk );
	m_FC8.stage7_valid( stage7_valid );
	m_FC8.stage8_valid( stage8_valid );
	
	sc_start( 500, SC_NS );
	return 0;

}
