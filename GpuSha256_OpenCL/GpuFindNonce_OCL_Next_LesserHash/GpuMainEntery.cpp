/*
Author : Anshul Yadav
email : yadav26@gmail.com
LinkedIn: https://www.linkedin.com/in/anshul-yadav-2289b734/
*/


#include "CLHashHeader.h"


const char *filename = "GpuFindNonce_OCL_Next_LesserHash.cl.cpp";
const char* kernel_name_nonce = "sha256_nonce";
const char* kernel_name = "sha256_nonce_for_smaller_hash";

string shello = "hello0";
UINT64 start = 0;



//////// 
///initialize OCL_INSTANCE
OCL_INSTANCE g_oclInstance;


bool findsmallerhash(string str1, string str2) {
	
	int i = 0;
	while (i < str2.length() )
	{
		if (str1[i] < str2[i])
			return false;
		else if (str1[i] > str2[i])
			return true;
		else
			i++;
	}
	return false;
}


int main(int argc, char* argv[])
{
	string Target = "0000000399C6AEA5AD0C709A9BC331A3ED6494702BD1D129D8C817A0257A1462"; //665782;

	//char TargetInBytes[] = { 0x12, 0x00, 0x00,0x00, 0x00,0x00, 0x03,0x00, 0x99,0x00, 0xC6,0x00, 0xAE,
	//	0x00, 0xA5,0x00, 0xAD,0x00, 0x0C,0x00, 0x70,0x00, 0x9A,0x00, 0x9B,0x00, 0xC3,0x00, 0x31,0x00, 0xA3,
	//	0x00, 0xED,0x00, 0x64,0x00, 0x94,0x00, 0x70,0x00, 0x2B,0x00, 0xD1,0x00, 0xD1,0x00, 0x29,0x00, 0xD8,
	//	0x00, 0xC8,0x00, 0x17,0x00, 0xA0,0x00, 0x25,0x00, 0x7A,0x00, 0x14,0x00, 0x62, 0x00, }; //665782

	//int targetlen = sizeof(TargetInBytes)/sizeof(char);

	//int r = 0;
	//
	//while (r < targetlen) //TargetInBytes;//"0000000399C6AEA5AD0C709A9BC331A3ED6494702BD1D129D8C817A0257A1462"; //665782
	//{
	////	int x = TargetInBytes[r] | TargetInBytes[r+1] << 1 | TargetInBytes[r + 2] << 2 | TargetInBytes[r + 3] << 3;

	//	char t[4] = { '\0' };
	//	sprintf(t, "%x", TargetInBytes[r]);
	//	Target.append(t);
	//	r ++;
	//}
	//	
	int p = -1;

	Target[0] = 0x00;
	Target[1] = 0x00;
	Target[2] = 0x00;
	//Target[3] = 0x00;
	//Target[4] = 0x00;
	//Target[5] = 0x00;
	//Target[6] = 0x00;
	//Target[7] = 0x00;
	//Target[8] = 0x00;

	
	//while (++p < Target.length())
	//{
	//	printf("p %d =[%c] [%02x] \n", p, Target[p], Target[p]);
	//}

	int outputSize = sizeof(char) * (shello.length() * HASH_LENGTH  * 2 );

	unsigned char* output = (unsigned char *)malloc(outputSize);

	std::vector <string> vStrings;

	char bff[64] = "\0";

	cout << "\nGPUTHREADS #  " << GPUTHREADS << endl;

	memset(&g_oclInstance, 0, sizeof(g_oclInstance));

	int ts = GetTickCount();

	std::vector <string> hashCollection;

	long int tsNonce = GetTickCount();
	string nonce;
	string  newstr=shello;

	
	cl_int	status = InitializeOCL(&g_oclInstance, filename, newstr.c_str(), output);
	if (status != 0)
	{
		return 0;
	}
	//newstr.clear();

	//vStrings.clear();

	char buff[65] = { '\0' };
	std::pair <string, string> kvp;
	
	std::map<string, string> MapNonce;
	
	map<string, string> ::iterator mapit = MapNonce.begin();


	bool bIsSmaller = false;

	while(MapNonce.size() < 1 )
	//while(start < start+ 4*GPUTHREADS)
	{


		int inlength = newstr.length();
		memset(output, '\0', outputSize);
		//int ts = GetTickCount();
		status = RunGpu_Loads(&g_oclInstance, newstr.c_str(), (char*)output, kernel_name, start, outputSize, Target );

		//int te = GetTickCount();

		//cout << "\nEnd Counter = " << te - ts << endl;

		//ts = GetTickCount();

		start += GPUTHREADS;

		for (int k = 0; k < GPUTHREADS; k++)
		{
			int outlen = -1;
			if (output[0] == '\0')
				continue;

			unsigned char* travStart = output;
			string noncestr;
			char bftochar[2];
			while (travStart[++outlen] -! '-' )
			{
				if (travStart[outlen] == '-')
					break;
				sprintf(bftochar, "%c", travStart[outlen]);
				noncestr.append(string( bftochar));

				//cout << travStart[outlen] << " " ;

			}
		
			travStart = &travStart[outlen +1];

			sprintf(buff, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				travStart[0], travStart[1], travStart[2], travStart[3], travStart[4],
				travStart[5], travStart[6], travStart[7], travStart[8], travStart[9],
				travStart[10], travStart[11], travStart[12], travStart[13], travStart[14],
				travStart[15], travStart[16], travStart[17], travStart[18], travStart[19],
				travStart[20], travStart[21], travStart[22], travStart[23], travStart[24],
				travStart[25], travStart[26], travStart[27], travStart[28], travStart[29],
				travStart[30], travStart[31]
			);

			travStart[32] = '\0';

			string tmp = buff;

			//cout << tmp << endl;
			
			//noncestr = noncestr.substr(5, noncestr.size() );
			
			MapNonce.insert(pair <string, string>(tmp, noncestr));

			//hashCollection.push_back(tmp);
			
			

			//if(MapNonce.size() > 0 )
			// bIsSmaller = findsmallerhash( Target ,mapit->first);
			 //if (bIsSmaller == true)
			//	 break;

			memset(output, '\0', outputSize );

		}


		//cout << "\n 0x"<<hex << start<<dec <<" - decimal_start = " << start << endl;
	
	}

	long int teNonce = GetTickCount();

	cout << "\n Input String = " << shello << endl;
	cout << "\n Input Target Hash = " << Target << endl;
	
	mapit = MapNonce.begin();

	int i_hex = std::stoi((mapit->second).c_str(), nullptr, 16);
	cout << "\n Nonce = 0x " <<hex<< i_hex << endl;
	
	cout << "\n HASH =  " << mapit->first.c_str() << endl << dec ;
	cout << "\n Nonce found in  time = " << teNonce - tsNonce << endl;
	if (output != NULL)
	{
		free(output);
		output = NULL;
	}


	ReleaseOCL(&g_oclInstance);

	cout << endl;

	return SUCCESS;
}

