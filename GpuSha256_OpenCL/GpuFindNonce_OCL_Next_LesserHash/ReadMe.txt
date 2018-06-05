Author : Anshul Yadav
email : yadav26@gmail.com
LinkedIn: https://www.linkedin.com/in/anshul-yadav-2289b734/


This is version 3.0 code
1. Code restructuring
2. Added gpu kernel to find the least hash
3. CPU side only checking output first character before refilling new kernels in gpu.
4. Map is filled for only one entry that has to be smallest then Target
5. Output buffer parsed from format <Nonce><seperator><hash smaller then target>
6. Best performance till now with GPUTHreads = 0.1 million

Errata
1. Smaller numbers consuming stack, need to minimize the cpu stack usage by transfer memory to heap.
2. Gpu running out of memory on very small searches.
3. Failing in test case where target is leading with six zeros - 00000000
