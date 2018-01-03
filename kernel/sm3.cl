#if 0

	#define cpu_to_be16(v) (v)
	#define cpu_to_be32(v) (v)
	#define be16_to_cpu(v) (v)
	#define be32_to_cpu(v) (v)

#else

	#define cpu_to_be16(v) (((v)<< 8) | ((v)>>8))
	#define cpu_to_be32(v) (((v)>>24) | (((v)>>8)&0xff00) | (((v)<<8)&0xff0000) | ((v)<<24))
	#define be16_to_cpu(v) cpu_to_be16(v)
	#define be32_to_cpu(v) cpu_to_be32(v)

#endif

#define SM3_ROTATELEFT(X,n)  (((X)<<(n)) | ((X)>>(32-(n))))

#define SM3_P0(x) ((x) ^  SM3_ROTATELEFT((x),9)  ^ SM3_ROTATELEFT((x),17))
#define SM3_P1(x) ((x) ^  SM3_ROTATELEFT((x),15) ^ SM3_ROTATELEFT((x),23))

#define SM3_FF0(x,y,z) ( (x) ^ (y) ^ (z))
#define SM3_FF1(x,y,z) (((x) & (y)) | ( (x) & (z)) | ( (y) & (z)))

#define SM3_GG0(x,y,z) ( (x) ^ (y) ^ (z))
#define SM3_GG1(x,y,z) (((x) & (y)) | ( (~(x)) & (z)) )


#define SM3_DIGEST_LENGTH	32
#define SM3_BLOCK_SIZE		64
#define SM3_CBLOCK		(SM3_BLOCK_SIZE)
#define SM3_HMAC_SIZE		(SM3_DIGEST_LENGTH)