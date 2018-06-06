/*

Developer - Anshul Yadav

for any reference - 
yadav26@gmail.com

www.linkedin.com/in/anshul-yadav-2289b734

Ported from opensource sha C++ code.

*/

#define SHA2_UNPACK32(x, str)                 \
{\
    *((str) + 3) = (char) ((x)      );       \
    *((str) + 2) = (char) ((x) >>  8);       \
    *((str) + 1) = (char) ((x) >> 16);       \
    *((str) + 0) = (char) ((x) >> 24);       \
}

#define SHA2_PACK32(str, x)                   \
{\
    *(x) =   ((uint) *((str) + 3)      )    \
           | ((uint) *((str) + 2) <<  8)    \
           | ((uint) *((str) + 1) << 16)    \
           | ((uint) *((str) + 0) << 24);   \
}

#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))

#define DIGEST_SIZE (256 / 8)
#define SHA224_256_BLOCK_SIZE  (512 / 8)

void swap(char *str1, char *str2)
{
	//printf("SWAP - before s[ %c ] , d[%c]", str1, str2);
	char *temp = str1;
	str1 = str2;
	str2 = temp;

	//printf("SWAP - after s[ %c ] , d[%c]", str1, str2 );
}


/* A utility function to reverse a string  */
void reverse(char *str, int length)
{
	int start = 0;
	int end = length-1;

	//for (int cnt = 0; cnt < end; ++cnt)
	//{
	//	printf("Reverse - str [ %d ] = [%c]", cnt, str[cnt]);
	//}

	while (start < end)
	{
		//printf("Before [%c,%c]", *(str + start), *(str + end));
		swap(*(str + start), *(str + end));
		//printf("After [%c,%c]", *(str + start), *(str + end));

		start++;
		end--;
	}
	return;
}

// Implementation of itoa()
int myitoa(unsigned long num, char* str, int base)
{

	int i = 0;
	bool isNegative = false;

	// Handle 0 explicitely, otherwise empty string is printed for 0 
	if (num == 0)
	{
		str[i++] = '0';
		str[i] = '\0';
		return i;
	}

	// In standard itoa(), negative numbers are handled only with 
	// base 10. Otherwise numbers are considered unsigned.
	if (num < 0 && base == 10)
	{
		isNegative = true;
		num = -num;
	}

	// Process individual digits
	while (num != 0)
	{
		int rem = num % base;
		str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num = num / base;
	}

	// If number is negative, append '-'
	if (isNegative)
		str[i++] = '-';

	
	// Reverse the string
	reverse(str, i);

	return i;

}

//char* itoa(int val, int base) {
//
//	static char buf[32] = { 0 };
//
//	int i = 30;
//
//	for (; val && i; --i, val /= base)
//
//		buf[i] = "0123456789abcdef"[val % base];
//
//	return &buf[i + 1];
//
//}



int FindHashforInputString(char* in, int inputlen, unsigned char* buf )
{

	unsigned int m_len = 0;
	unsigned int m_tot_len = 0;

	unsigned int pm_len;
	unsigned int len_b;
	unsigned int block_nb;
	unsigned int new_len, rem_len, tmp_len;

	unsigned int final_length = inputlen;
	uint w[64];
	uint wv[8];
	uint t1, t2;
	int i;
	int j;

	unsigned char *message;
	unsigned char *sub_block;
	unsigned char digest[DIGEST_SIZE] = { 0 };
	unsigned char m_block[128];
	char *shifted_message;

	int gsha256_k[64] = //UL = uint32
	{ 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
		0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
		0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };


	uint m_h[8] = { 0 };
	m_h[0] = 0x6a09e667;
	m_h[1] = 0xbb67ae85;
	m_h[2] = 0x3c6ef372;
	m_h[3] = 0xa54ff53a;
	m_h[4] = 0x510e527f;
	m_h[5] = 0x9b05688c;
	m_h[6] = 0x1f83d9ab;
	m_h[7] = 0x5be0cd19;

	

	tmp_len = SHA224_256_BLOCK_SIZE - m_len;
	rem_len = final_length < tmp_len ? final_length : tmp_len;

	//memcpy(&m_block[m_len], input.c_str(), rem_len);
	for (int i = m_len; i < rem_len; ++i)
	{
		m_block[i] = in[i];
	}
	m_block[rem_len] = '\0';

	if (m_len + final_length < SHA224_256_BLOCK_SIZE) {
		m_len += final_length;
	}
	else
	{
		new_len = final_length - rem_len;

		block_nb = new_len / SHA224_256_BLOCK_SIZE;
		shifted_message = in + rem_len;

		//
		//transform(m_block, 1);
		//transform(const unsigned char *message, unsigned int block_nb)
		block_nb = 1;
		message = m_block;

		for (i = 0; i < (int)block_nb; i++) {
			sub_block = message + (i << 6);
			for (j = 0; j < 16; j++) {
				SHA2_PACK32(&sub_block[j << 2], &w[j]);

			}
			for (j = 16; j < 64; j++) {
				w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];
			}
			for (j = 0; j < 8; j++) {
				wv[j] = m_h[j];
			}
			for (j = 0; j < 64; j++) {
				t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6]) + gsha256_k[j] + w[j];
				t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
				wv[7] = wv[6];
				wv[6] = wv[5];
				wv[5] = wv[4];
				wv[4] = wv[3] + t1;
				wv[3] = wv[2];
				wv[2] = wv[1];
				wv[1] = wv[0];
				wv[0] = t1 + t2;
			}
			for (j = 0; j < 8; j++) {
				m_h[j] += wv[j];
			}
		}

		//
		//transform(shifted_message, block_nb);
		message = (unsigned char*)shifted_message;

		for (i = 0; i < (int)block_nb; i++) {
			sub_block = message + (i << 6);
			for (j = 0; j < 16; j++) {
				SHA2_PACK32(&sub_block[j << 2], &w[j]);
			}
			for (j = 16; j < 64; j++) {
				w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];
			}
			for (j = 0; j < 8; j++) {
				wv[j] = m_h[j];
			}
			for (j = 0; j < 64; j++) {
				t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6]) + gsha256_k[j] + w[j];
				t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
				wv[7] = wv[6];
				wv[6] = wv[5];
				wv[5] = wv[4];
				wv[4] = wv[3] + t1;
				wv[3] = wv[2];
				wv[2] = wv[1];
				wv[1] = wv[0];
				wv[0] = t1 + t2;
			}
			for (j = 0; j < 8; j++) {
				m_h[j] += wv[j];
			}
		}



		rem_len = new_len % SHA224_256_BLOCK_SIZE;
		//memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
		char* p = &shifted_message[block_nb << 6];
		for (int i = 0; i < rem_len; ++i)
		{

			m_block[i] = p[i];
			//printf("%c", m_block[i]);
		}
		m_block[rem_len] = '\0';

		m_len = rem_len;
		m_tot_len += (block_nb + 1) << 6;

	}


	//ctx.final(digest);


	block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9) < (m_len % SHA224_256_BLOCK_SIZE)));
	len_b = (m_tot_len + m_len) << 3;
	pm_len = block_nb << 6;

	//TBD
	//memset(, 0, pm_len - m_len);
	char* p = m_block + m_len;
	for (int i = 0; i < pm_len - m_len; ++i)
	{

		p[i] = 0;
	}
	p[pm_len - m_len] = '\0';

	m_block[m_len] = 0x80;

	SHA2_UNPACK32(len_b, m_block + pm_len - 4);


	///
	//
	//transform(m_block, block_nb);
	message = m_block;

	for (i = 0; i < (int)block_nb; i++) {
		sub_block = message + (i << 6);
		for (j = 0; j < 16; j++) {
			SHA2_PACK32(&sub_block[j << 2], &w[j]);
		}
		for (j = 16; j < 64; j++) {
			w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];
		}
		for (j = 0; j < 8; j++) {
			wv[j] = m_h[j];
		}
		for (j = 0; j < 64; j++) {
			t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6]) + gsha256_k[j] + w[j];
			t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
			wv[7] = wv[6];
			wv[6] = wv[5];
			wv[5] = wv[4];
			wv[4] = wv[3] + t1;
			wv[3] = wv[2];
			wv[2] = wv[1];
			wv[1] = wv[0];
			wv[0] = t1 + t2;
		}
		for (j = 0; j < 8; j++) {
			m_h[j] += wv[j];
		}
	}


	///
	for (i = 0; i < 8; i++) {
		SHA2_UNPACK32(m_h[i], &digest[i << 2]);
	}


	for (int i = 0; i < DIGEST_SIZE; i++)
	{
		buf[i] = digest[i];
	}
	buf[DIGEST_SIZE] = '\0';


	return DIGEST_SIZE;
}


//Block size on Bit chain is 1 MB
//inlen == sizeof(char) * 1000 * 1000 == 1 MegaBytes
/*

__kernel void sha256_nonce( __global char* input, 
							int inlen, 
							__global unsigned char* out,  
							int outlen,
							unsigned long startIndex )
{
	

	

	unsigned int m_len = 0;
	unsigned int m_tot_len = 0;

	unsigned int pm_len;
	unsigned int len_b;
	unsigned int block_nb;
	unsigned int new_len, rem_len, tmp_len;

	unsigned int final_length = 0;
	uint w[64];
	uint wv[8];
	uint t1, t2;
	int i;
	int j;

	unsigned char *message;
	unsigned char *sub_block;
	unsigned char digest[DIGEST_SIZE] = { 0 };
	unsigned char m_block[128];
	char *shifted_message;
	char* in[512] = { '\0' };

	int num = get_global_id(0);

	int gsha256_k[64] = //UL = uint32
	{ 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
		0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
		0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };


	uint m_h[8] = { 0 };
	m_h[0] = 0x6a09e667;
	m_h[1] = 0xbb67ae85;
	m_h[2] = 0x3c6ef372;
	m_h[3] = 0xa54ff53a;
	m_h[4] = 0x510e527f;
	m_h[5] = 0x9b05688c;
	m_h[6] = 0x1f83d9ab;
	m_h[7] = 0x5be0cd19;

	//printf("\nCL Kernel Input args : inLen = %d\n", inlen);

	//Assign local array -- Should be removed for optimization
	//unsigned int incount = -1;
	//unsigned int breakcounter = 0;

	int ipos = -1;
	int bc = 0;
	int lpos = 0;
	int cnt = 0;
	char tmp[64] ;

	//while (input[++ipos] != '\0')
	//{

	//	if (input[ipos] == ';')
	//	{
	//		if (bc == num)
	//		{
	//			in[final_length] = '\0';

	//			break;
	//		}
	//		else
	//		{

	//			bc++;
	//			final_length = 0;
	//		}
	//	}
	//	else
	//	{
	//		in[final_length++] = input[ipos];
	//	}
	//}


	int newStartIndex = startIndex + num;

	
	//printf("\nCL[ %d ] Input args:  startIndex = %d", num, startIndex);

	for (int i = 0; i < inlen ; ++i)
	{
		in[i] = input[i];
		//printf("num = %d, in[%d] = %c", num, i, in[i]);
	}

	int lenStart = 0;

	lenStart = myitoa(newStartIndex, tmp, 16);
// still reverse function not working we have to redo reverse
	unsigned char* revTemp[64] = { '\0' };
	for (int i = 0; i < lenStart; ++i)
	{
		revTemp[i] = tmp[lenStart - 1 - i] ;

		//printf("Output from itoa - num = %d, temp[%d] = %c", num, i, tmp[i]);
	}
	//for (int i = 0; i < lenStart; ++i)
	//{
	//	printf("Output from itoa - REVTemp temp[%d] = %c", i, revTemp[i]);
	//}

	final_length = inlen + lenStart;

	

	for (int i = inlen; i < final_length+1; ++i)
	{
		in[i] = revTemp[i -inlen];
		//printf("num = %d, in[%d] = %c", num, i, in[i]);
	}
	in[final_length+1] = '\0';

	//for (int i = 0; i < final_length; ++i)
	//{
	//	printf("KL [%d] newString formed :in[%d] = %c", num, i, in[i]);
	//}
	//printf("\nCL[ %d ] Kernel New input: newStartIndex = %d", num, newStartIndex);

	
	tmp_len = SHA224_256_BLOCK_SIZE - m_len;
	rem_len = final_length < tmp_len ? final_length : tmp_len;

	//memcpy(&m_block[m_len], input.c_str(), rem_len);
	for (int i = m_len; i < rem_len; ++i)
	{
		m_block[i] = in[i];
	}
	m_block[rem_len] = '\0';

	if (m_len + final_length < SHA224_256_BLOCK_SIZE) {
		m_len += final_length;
	}
	else
	{
		new_len = final_length - rem_len;

		block_nb = new_len / SHA224_256_BLOCK_SIZE;
		shifted_message = in + rem_len;

		//
		//transform(m_block, 1);
		//transform(const unsigned char *message, unsigned int block_nb)
		block_nb = 1;
		message = m_block;

		for (i = 0; i < (int)block_nb; i++) {
			sub_block = message + (i << 6);
			for (j = 0; j < 16; j++) {
				SHA2_PACK32(&sub_block[j << 2], &w[j]);

			}
			for (j = 16; j < 64; j++) {
				w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];
			}
			for (j = 0; j < 8; j++) {
				wv[j] = m_h[j];
			}
			for (j = 0; j < 64; j++) {
				t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6]) + gsha256_k[j] + w[j];
				t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
				wv[7] = wv[6];
				wv[6] = wv[5];
				wv[5] = wv[4];
				wv[4] = wv[3] + t1;
				wv[3] = wv[2];
				wv[2] = wv[1];
				wv[1] = wv[0];
				wv[0] = t1 + t2;
			}
			for (j = 0; j < 8; j++) {
				m_h[j] += wv[j];
			}
		}

		//
		//transform(shifted_message, block_nb);
		message = (unsigned char*)shifted_message;

		for (i = 0; i < (int)block_nb; i++) {
			sub_block = message + (i << 6);
			for (j = 0; j < 16; j++) {
				SHA2_PACK32(&sub_block[j << 2], &w[j]);
			}
			for (j = 16; j < 64; j++) {
				w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];
			}
			for (j = 0; j < 8; j++) {
				wv[j] = m_h[j];
			}
			for (j = 0; j < 64; j++) {
				t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6]) + gsha256_k[j] + w[j];
				t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
				wv[7] = wv[6];
				wv[6] = wv[5];
				wv[5] = wv[4];
				wv[4] = wv[3] + t1;
				wv[3] = wv[2];
				wv[2] = wv[1];
				wv[1] = wv[0];
				wv[0] = t1 + t2;
			}
			for (j = 0; j < 8; j++) {
				m_h[j] += wv[j];
			}
		}



		rem_len = new_len % SHA224_256_BLOCK_SIZE;
		//memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
		char* p = &shifted_message[block_nb << 6];
		for (int i = 0; i < rem_len; ++i)
		{

			m_block[i] = p[i];
			//printf("%c", m_block[i]);
		}
		m_block[rem_len] = '\0';

		m_len = rem_len;
		m_tot_len += (block_nb + 1) << 6;

	}


	//ctx.final(digest);


	block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9) < (m_len % SHA224_256_BLOCK_SIZE)));
	len_b = (m_tot_len + m_len) << 3;
	pm_len = block_nb << 6;

	//TBD
	//memset(, 0, pm_len - m_len);
	char* p = m_block + m_len;
	for (int i = 0; i < pm_len - m_len; ++i)
	{

		p[i] = 0;
	}
	p[pm_len - m_len] = '\0';

	m_block[m_len] = 0x80;

	SHA2_UNPACK32(len_b, m_block + pm_len - 4);


	///
	//
	//transform(m_block, block_nb);
	message = m_block;

	for (i = 0; i < (int)block_nb; i++) {
		sub_block = message + (i << 6);
		for (j = 0; j < 16; j++) {
			SHA2_PACK32(&sub_block[j << 2], &w[j]);
		}
		for (j = 16; j < 64; j++) {
			w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];
		}
		for (j = 0; j < 8; j++) {
			wv[j] = m_h[j];
		}
		for (j = 0; j < 64; j++) {
			t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6]) + gsha256_k[j] + w[j];
			t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
			wv[7] = wv[6];
			wv[6] = wv[5];
			wv[5] = wv[4];
			wv[4] = wv[3] + t1;
			wv[3] = wv[2];
			wv[2] = wv[1];
			wv[1] = wv[0];
			wv[0] = t1 + t2;
		}
		for (j = 0; j < 8; j++) {
			m_h[j] += wv[j];
		}
	}

	///
	for (i = 0; i < 8; i++) {
		SHA2_UNPACK32(m_h[i], &digest[i << 2]);
	}

	//unsigned char tempout[2 * DIGEST_SIZE + 1];
	//tempout[2 * DIGEST_SIZE] = 0;

	//
	//for (int index = 0; index < DIGEST_SIZE; index++)
	//{
	//	tempout[index] = digest[index];
	//	//printf("KL[%d]: tempout-assignemnt - index=[%d], hash : %02x",num, index, tempout[index]);
	//}


//
//	//printf("KL [%d]: final_len=[%d], pos [%d] hash : %c", num, final_length, final_length, out[final_length]);

	outlen = (DIGEST_SIZE ) + final_length + 1;

	//printf("KL [%d]: outputsize  : %d", num , outlen);
	for (int i = 0; i < outlen && newStartIndex == 0; ++i)
	{
		//printf("onetime initialization");
		out[i] = '\0';
	}

	if (digest[0] == 0x00) {
		if (digest[1] == 0x00)
		{
			if (digest[2] == 0x00)
			{
				//if (digest[3] == 0x00) {
					//if (digest[4] == 0x00){
						//printf("Kernel : NONCE evaluate before :- %x\n", newStartIndex);

				for (int i = 0; i < final_length; ++i)
				{
					out[i] = in[i];
					//printf("KL [%d]: index=[%d], pos [%d] hash : %c", num, i, i, out[i]);
				}

				out[final_length] = '-';
				//
				//	//printf("KL [%d]: final_len=[%d], pos [%d] hash : %c", num, final_length, final_length, out[final_length]);

				for (int i = 0; i < DIGEST_SIZE; ++i)
				{
					outlen = (DIGEST_SIZE)+final_length + 1;
					out[final_length + 1 + i] = digest[i];
				}

				//printf("Kernel : NONCE evaluate after :- %x\n", newStartIndex);

			//}
		//}
			}
		}
	}






	return;
}

*/


//Block size on Bit chain is 1 MB
//inlen == sizeof(char) * 1000 * 1000 == 1 MegaBytes

int MemCopy_GlobalInputBuffer(__global char* src, int srclen, unsigned char* dst, int dstlen )
{
	if (dstlen < srclen)
	{
		printf("*****Destination buffer[size=%d] is smaller, GPU Crash. BSOD*********", dstlen );
		return -1;
	}

	for (int i = 0; i < srclen; ++i) {
		dst[i] = (unsigned char)src[i];
	}
	dst[srclen] = '\0';

	return srclen;
}

void printBuffer(unsigned char* buff, int len )
{
	printf("\nPrinting buffer for input length = %d", len);

	for (int i = 0; i < len; ++i) {
		printf("Printing Buffer - Buff[%d] = [%02x]-[%c]", i, buff[i], buff[i] );
	}

	return;
}

void printHash(unsigned char*buff)
{

	for (int i = 0; i < DIGEST_SIZE; ++i) {
		printf("Printing HASH - Buff[%d] = [%x][%02x]-[%c]", i, buff[i], buff[i], buff[i]);

	}
	return;
}


///
//To find hash that is smaller then target hash; we dont need to concatenate with any input; we only need to find a numericalstring
//which can generate the hash smaller then the input;
//input string is used only for comparison.
///



__kernel void sha256_nonce_for_smaller_hash( 
		__global unsigned char* input,
		int inlen,
		__global unsigned char* out,
		int outlen,
		unsigned long startIndex,
		__global unsigned char* targetHash
		)
{
	bool found = false;

	int num = get_global_id(0);

	unsigned char digest[DIGEST_SIZE + 1] = { 0 };

	unsigned long newStartIndex = startIndex + num;
	
	//if (num == 0)
		//printf("\nStartIndex = %x\n", startIndex);

	unsigned char inbuff[512] = {'\0'};

	int inlength = inlen;

	if (inlen > 512)
	{
		printf("CL - Kernel Error. Input is larger then(512) kernel can handle.*** inlen=[%d]\n", inlen );
		return;
	}

	inlength = MemCopy_GlobalInputBuffer(input, inlen, inbuff, 512);


	unsigned int final_length = 0;

	char tmp[64] = { 0 };
	int lenStart = 0;


	//printf("\nCL[ %d ] Input args:  startIndex = %d", num, startIndex);

	lenStart = myitoa(newStartIndex, tmp, 16); // base 16


// still reverse function not working we have to redo reverse
	unsigned char revTemp[64] = { '\0' };

	for (int i = 0; i < lenStart; ++i)
	{
		revTemp[i] = tmp[lenStart - 1 - i];
	}

	
	int res = FindHashforInputString(revTemp, lenStart, digest);
	

	//outlen = (DIGEST_SIZE)+final_length + 1;

	////printf("KL [%d]: outputsize  : %d", num , outlen);
	//for (int i = 0; i < DIGEST_SIZE*2 +1 && (newStartIndex - startIndex )== 0; ++i)
	//{
	//	//printf("onetime initialization");
	//	out[i] = '\0';
	//}

	for (int index = 0; index < inlength -1; index++)
	{
	
		if (inbuff[index] > digest[index])
		{
			found = true;
			
			//printf("KL[%x]: comparison check - t[%d] = [%02x]  > f[%02x]", newStartIndex, index, inbuff[index], digest[index]);
			break;
		}
		else if (inbuff[index] < digest[index]) {
			found = false;
			//printf("KL[%x]: comparison check- t[%d] = [%02x]  < f[%02x]", newStartIndex, index, inbuff[index], digest[index]);
			break;
		}
		else {
			
			//printf("KL[%d]: comparison check EQUALITY- t[%d] = [%02x] = f[%02x]", newStartIndex, index, inbuff[index], digest[index]);
		}
	
	}


	//printf("KL[%d]: comparison check- Result Found [%d]", num, found);
	//
	//barrier(CLK_GLOBAL_MEM_FENCE);

	if (found)
	{
		//printHash(inbuff);
		//printHash(digest);

		//if (out[0] != '\0')
		//	return;
		//out[0] = 1;
		printf("Found out[0]=null writing one time. nonce (newStartIndex) =  %x, lenStart = %d", newStartIndex, lenStart);
		int len = 0;
		while (out[len] != '\0') ++len;
		out[len] = ',';
		len += 1;
		for (int i = len; i < len+lenStart; ++i)
		{
			out[i] = revTemp[i-len];
			//printf("KL [%d]: out_hex[%d] = %02x, out_char : %c", num, i, out[i], out[i]);
		}

		
		//out[lenStart] = '-';
		//
		////printf("KL [%d]: out_hex[%d] = %02x, out_char : %c", num, lenStart, out[lenStart], out[lenStart]);

		//lenStart += 1;
		//for (int i = lenStart; i < DIGEST_SIZE + lenStart ; ++i)
		//{
		//	out[i] = digest[i - lenStart];
		//	
		//	//printf("KL [%d]: out_hex[%d] = %02x", num, i, out[i]);
		//}

	}

	
	return;
}
