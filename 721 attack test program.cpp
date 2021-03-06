#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <time.h>
#include <iostream>
//并行专用
//#include <mpi.h>

#define u32 unsigned int
#define u8 unsigned char
u8 StepRound1152(int Round,u32 *key,u32 *IV)
 {
 	u8 res=0;
 	u8 reg0[3000]={0};
 	u8 reg1[3000]={0};
 	u8 reg2[3000]={0};
 	int round=Round;
 	for(int i=0;i<80;i++)
 	{
 		int word=i/32; int bit=i%32;
 		reg0[92-i]=((key[word]>>bit)&1);
 		reg1[83-i]=((IV[word]>>bit)&1);
 	}
 	reg2[0]=1;reg2[1]=1;reg2[2]=1;
 	for(int i=0;i<round-64;i++)
 	{
 		reg0[i+93]=reg2[i+45]^(reg2[i+1]&reg2[i+2])^reg2[i]^reg0[i+24];
 		reg1[i+84]=reg0[i+27]^(reg0[i+1]&reg0[i+2])^reg0[i]^reg1[i+6];
 		reg2[i+111]=reg1[i+15]^(reg1[i+1]&reg1[i+2])^reg1[i]^reg2[i+24];
 	}
 	res=reg0[93+round-65]^reg0[93+round-92]^reg1[84+round-68]^reg1[84+round-83]^reg2[111+round-65]^reg2[111+round-110];
	u8 result=((reg1[221+84-1]<<1)|res); //result 包含两个比特，分别是s1[221]以及Trivium的输出比特
	return result;
 }

void test()
{
	//测试向量代码链接
	//https://asecuritysite.com/encryption/trivium?val1=00000010000000000000&val2=00000000000000000000

	/*例如：测试向量为：
	Key: 00000000000000000000
	IV:  00000000000000000000
	Stream: FBE0BF265859051B517A2E4E239FC97F

	本程序输出结果：FBE0BF26585951B517A2E4E239FC97F5632316197CF2DE7A879FA1B2E9CDF752923268B7382B4C1A
	吻合。
	*/

	 u32 key[3]={0};u32 IV[3]={0};
	 u8 tt=0;
	 u8 xx=0;
	 u8 xy=0;
	 for(int i=1151;i<1500;i++)
	 {
		 u8 temp=StepRound1152(i,key,IV);//输出Trivium
		
		 //printf("%d",temp&1);
         if((((i-1151)%8)==0)&&(i!=1151))
		 {
			 printf("%X",xx);
			 xx=0;
		 }
		 xx = xx  | ((temp&1)<<((i-1151)%8));//只有低比特的temp是trivium输出流
	 }

}


u8 StepRoundIm(u32 *key,u32 *IV)
{
	u8 res=0;
	u8 reg0[1000]={0};
	u8 reg1[1000]={0};
	u8 reg2[1000]={0};
	int round=721; //721-Trivium
	for(int i=0;i<80;i++)
	{
		int word=i/32; int bit=i%32;
		reg0[92-i]=((key[word]>>bit)&1);
		reg1[83-i]=((IV[word]>>bit)&1);
	}
	reg2[0]=1;reg2[1]=1;reg2[2]=1;
	for(int i=0;i<round-60;i++)
	{
		reg0[i+93]=reg2[i+45]^(reg2[i+1]&reg2[i+2])^reg2[i]^reg0[i+24];
		reg1[i+84]=reg0[i+27]^(reg0[i+1]&reg0[i+2])^reg0[i]^reg1[i+6];
		reg2[i+111]=reg1[i+15]^(reg1[i+1]&reg1[i+2])^reg1[i]^reg2[i+24];
	}
	round = round-1 ;//我们实现的trivium比标准慢一拍，我们平移了一步，这个不影响结论正确性。
	//our trivium is slower for one step than the test one, so we shift one step, this do not influence result 
	res=reg0[93+round-65]^reg0[93+round-92]^reg1[84+round-68]^reg1[84+round-83]^reg2[111+round-65]^reg2[111+round-110];
	u8 result=((reg1[221+84-1]<<1)|res);//result 包含两个比特，分别是s1[221]以及Trivium的输出比特
	return result;//it returns two bits: s1[221] and output bit of trivium
}


int main()
{
		int rank;      int size;  
	/*并行,for parallel
		MPI_Init(0,0);  
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
    MPI_Comm_size(MPI_COMM_WORLD, &size);  
    */

	//test();//首先测试Trivium的正确性
	
    u32 IV[3]={0};
		u32 key[3]={0};
		u32 key_wrong[3]={0};
		//The index can be changed according to your choice of cubes.
	/*
	In the new attack on 721, we nullified 80-29=51 IV bits and only 29 free IV bits is considered.
	Then we use techniques in CRYPTO 2018 paper to find p1=s1[221]. In addition, we find the 
	29 degree IV-term is in z721, i.e.,the accurate degree of z721 is 29. However, in (1+s1[221])*z721, 
	there is no such 29 degree IV-term (which is achieved by IV representation), i.e., degree of (1+s1[221])*z721 is <29. 
	Moreover, when given the wrong key guessing in s1[221], the 29 degree IV-term also appears. So this 29-dimension cube 
	could be served as key-recovery distinguisher. 

	Finally, we find several such 29-dimension cubes. The new testing program for 721 attack will be given to Yonglin Hao 
	for correctness checking.
	*/
		
		int cube_index_total[17][29]={
		{0,4,10,16,20,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,60,62,66,68,70,74,76},
		{0,2,4,8,14,16,20,24,28,30,32,34,36,38,42,44,46,48,50,52,54,56,60,62,66,68,70,74,76},
		{0,4,6,10,14,16,20,24,28,30,32,34,36,38,42,44,46,48,50,52,54,56,60,62,66,68,70,74,76},
		{0,4,8,10,14,16,20,24,28,30,32,34,36,38,42,44,46,48,50,52,54,56,60,62,66,68,70,74,76},
		{0,4,10,16,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,52,54,56,60,62,66,68,70,74,76},
		{0,2,4,8,16,20,22,24,28,30,32,34,36,38,40,42,44,46,48,50,54,56,60,62,66,68,70,74,76},
		{0,4,8,14,16,20,22,24,28,30,32,34,36,38,40,42,44,46,48,52,54,56,60,62,66,68,70,74,76},
		{0,4,8,10,14,20,24,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,60,62,66,68,70,74,76},
		{0,4,10,14,20,22,24,26,28,30,32,34,36,38,42,44,46,48,50,52,54,56,60,62,66,68,70,74,76},
		{0,2,4,8,14,16,20,22,24,28,30,32,34,36,38,42,44,46,48,50,54,56,60,62,66,68,70,74,76},
		{0,4,10,14,16,20,24,26,28,30,32,34,36,38,40,42,44,46,48,52,54,56,60,62,66,68,70,74,76},
		{10,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,60,62,66,68,70,74,76},
		{4,10,16,18,20,22,24,26,28,30,32,36,38,40,42,44,46,48,50,52,54,56,60,62,66,68,70,74,76},
		{0,2,4,8,14,16,20,24,28,30,32,34,36,38,40,42,44,46,48,50,54,56,60,62,66,68,70,74,76},
		{0,4,10,14,16,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,60,62,66,68,70,74,76},
		{0,2,4,16,20,22,24,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,60,62,66,68,70,74,76},
		{0,2,4,8,16,20,24,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,60,62,66,68,70,74,76}
	};
	//确定s1[221]涉及到的密钥
	//related key bits in s1[221]
	int key_location[17]={8,9,46,71,72,73,59,60,52,10,17,18,19,1,26,27,28};
	
	int cube_index[29]={0};

	rank=0;
	//我们用大型机并行测试了上面17个cube，单核话，只测试第一个cube
	//if only use pc, we just test the first cube
	for(int i=0;i<29;i++)
		cube_index[i]=cube_index_total[rank][i];
	
	int len1=0;int len2=0;int len3=0;
	for(int i=0;i<29;i++)
	{
		if(cube_index[i]<32)
			len1++;
		else if(cube_index[i]<64)
			len2++;
		else
			len3++;
	}
	int test_time=10;
	int sum1=0;
	int sum2=0;
	int sum3=0;
	for(int rkey=0;rkey<test_time;rkey++)
	{
		srand((unsigned)time(NULL) & (rkey));//100个不同的密钥
		for(int i=0;i<10;i++)//right random keys
		{
			int word=i/4;int bit=i%4;
			key[word] = (key[word]<<(8*bit)) | (rand()&0xff);
		}
		key_wrong[0]=key[0];
		key_wrong[1]=key[1];
		key_wrong[2]=key[2];
		for(int i=0;i<17;i++)//wrong guessing at the key_location[17]
		{
			int word=key_location[i]/32; int bit=key_location[i]%32;
			key_wrong[word] ^=((rand()&1)<<bit);
		}
		unsigned char res=0;
		unsigned char res_mul=0;
		unsigned char res_wrong=0;
		for(unsigned int s1=0;s1<(1<<len1);s1++)
		{
			IV[0]=0;
			for(int i1=0;i1<len1;i1++)
				IV[0] |= (((s1>>i1)&1)<<(cube_index[i1]));
			for(unsigned int s2=0;s2<(1<<len2);s2++)
			{
				IV[1]=0;
				for(int i2=0;i2<len2;i2++)
					IV[1] |= (((s2>>i2)&1)<<(cube_index[len1+i2]-32));
				for(unsigned int s3=0;s3<(1<<len3);s3++)
				{
					IV[2]=0;
					for(int i3=0;i3<len3;i3++)
						IV[2] |= (((s3>>i3)&1)<<(cube_index[len1+len2+i3]-64));
					unsigned char r=StepRoundIm(key,IV);
					unsigned char rr=r&1;//r只有两比特，低比特为z721
					res^=rr;//z721上的cube加和
					//r的高比特为s1[221]
					res_mul ^=((1^((r>>1)&1))&rr);//(1+s1[221])*z721的cube加和
					r=StepRoundIm(key_wrong,IV);//
					res_wrong ^=((1^((r>>1)&1))&rr);//(1+s1'[221])*z721的cube加和
				}	
			}
		}

		sum1=sum1+res;//100个正确密钥中，z721上的cube加和为1的数量
		sum2=sum2+res_mul;//100个正确密钥中，(1+s1[221])*z721上的cube加和为1的数量
		sum3=sum3+res_wrong;//100个错误密钥中，(1+s1'[221])*z721上的cube加和为1的数量

		

	
	}
	char file[20]={0};sprintf(file,"test.txt");
	FILE *fp=fopen(file,"wb");   
	fprintf(fp,"%d %d %d",sum1,sum2,sum3);
	fclose(fp);
	//MPI_Finalize();
	
	//getchar();
	return 0;
	
}