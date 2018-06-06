/*
Author : Anshul Yadav
email : yadav26@gmail.com
LinkedIn: https://www.linkedin.com/in/anshul-yadav-2289b734/
*/


#include "CLHashHeader.h"

#include "CpuSHA256.h"

const char *filename = "GpuFindNonce_OCL_Next_LesserHash.cl.cpp";
const char* kernel_name_nonce = "sha256_nonce";
const char* kernel_name = "sha256_nonce_for_smaller_hash";

string shello = "hello0";
UINT64 start = 0x1c11d982d;// 0x14ba88787;//0x8588fdb4;// 0x665782;
UINT64 GPUTHREADS = 100000;


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

void convertHash64tobyte32(string s, unsigned char* d)
{

	int pos = 0, i = 0;

	while (pos < 64)
	{
		string t = s.substr(pos, 2);

		pos += 2;
		int it = std::stoi(t, nullptr, 16);
		d[i++] = (unsigned char)it;

		//cout << t << " = "<< hex << d[i]<< dec << " \n";
		//printf("i=%d str=%s, d = %02x, %c \n", i, t.c_str(), d[i], d[i]);
		//++i;
	}
}

int main(int argc, char* argv[])
{
	//string Target1 = "0000000399c6aea5ad0c709a9bc331a3ed6494702bd1d129d8c817a0257a1462"; //665782
	//string Target1 = "00000002234205614eaaa251bc8bc3aa635a9684c489f68609bacb71b11bf3fc"; //3627efee
	//string Target1 = "000000017d7ad95cd582ce7ccd8e73b13bec114d07f344517bfa78d9a61907fd"; //8588fdb4
	//string Target1 = "00000000f3b400f5c7a95d62e411a1419fe63826acd5385d3a40a22d353980d4"; //14ba88787
	string Target1 = "000000001a84fe93cf3e0bc70b1915bcb26c5125b6f7a9e95874e654c843493d"; //14ba88787

	int pos = 0, i = 0;
	unsigned char input[33] = { '\0' };

	convertHash64tobyte32(Target1, input);
		
	for (int i = 0; i<32; ++i)
		printf("%02x", input[i]);

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
	//string  newstr=Target;

	
	cl_int	status = InitializeOCL(&g_oclInstance, filename, input, output);
	if (status != 0)
	{
		return 0;
	}

	char buff[65] = { '\0' };
	std::pair <string, string> kvp;
	
	std::map<string, string> MapNonce;
	
	map<string, string> ::iterator mapit = MapNonce.begin();


	bool bIsSmaller = false;
	
	while(MapNonce.size() < 1 )
	//while(start < start+ 4*GPUTHREADS)
	{
		int inlength = sizeof(input)/sizeof(unsigned char);
	
		memset(output, '\0', outputSize);
		
		//int ts = GetTickCount();
		status = RunGpu_Loads(GPUTHREADS, &g_oclInstance, input, inlength ,(char*)output, kernel_name, start, outputSize, Target1 );

		//int te = GetTickCount();

		//cout << "\nEnd Counter = " << te - ts << endl;

		//ts = GetTickCount();

		start += GPUTHREADS;

		for (int k = 0; k < GPUTHREADS && MapNonce.size() < 1; k++)
		{

			int outlen = -1;
			if (output[0] == '\0')
				continue;

			unsigned char* travStart = output;
			string strOut = (char*)output;
			int cpos = 0; // strOut.find_first_of(',');
			
			while (cpos != -1)
			{
				cpos = strOut.find_first_of(',');
				string t = strOut.substr(0, cpos); 
				strOut = strOut.substr(cpos+1, strOut.length());

				if (t == "")
					continue;
				//pos = cpos;
				hashCollection.push_back(t);

			}
			for (auto&&x : hashCollection)
			{
				string hash = sha256(x);
				MapNonce.insert(pair <string, string>( hash, x ));
			}


			memset(output, '\0', outputSize );

		}


		//cout << "\n 0x"<<hex << start<<dec <<" - decimal_start = " << start << endl;
	
	}

	long int teNonce = GetTickCount();

//	cout << "\n Input String = " << shello << endl;
	cout << "\n Input Target Hash = " << Target1 << endl;
	
	mapit = MapNonce.begin();

	try {
		//UINT64 i_hex = std::stoi((mapit->second).c_str(), nullptr, 16);
		cout << "\n Nonce = " << (mapit->second).c_str() << endl;
	}
	catch (exception e)
	{
		cout << "\n Nonce = INVALID INPUT"<< endl;
	}
	
	
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

