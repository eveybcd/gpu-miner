

#include "algorithm.h"
#include "sph/sph_sha2.h"
#include "ocl.h"
#include "ocl/build_kernel.h"
#include "algorithm/bcd.h"
#include "compat.h"

#include <inttypes.h>
#include <string.h>

const char *algorithm_type_str[] = {
  "BCD"
};

void sha256(const unsigned char *message, unsigned int len, unsigned char *digest)
{
  sph_sha256_context ctx_sha2;

  sph_sha256_init(&ctx_sha2);
  sph_sha256(&ctx_sha2, message, len);
  sph_sha256_close(&ctx_sha2, (void*)digest);
}

void gen_hash(const unsigned char *data, unsigned int len, unsigned char *hash)
{
  unsigned char hash1[32];
  sph_sha256_context ctx_sha2;

  sph_sha256_init(&ctx_sha2);
  sph_sha256(&ctx_sha2, data, len);
  sph_sha256_close(&ctx_sha2, hash1);
  sph_sha256(&ctx_sha2, hash1, 32);
  sph_sha256_close(&ctx_sha2, hash);
}

void sha256d_midstate(struct work *work)
{
  unsigned char data[64];
  uint32_t *data32 = (uint32_t *)data;
  sph_sha256_context ctx;

  flip64(data32, work->data);
  sph_sha256_init(&ctx);
  sph_sha256(&ctx, data, 64);
  memcpy(work->midstate, ctx.val, 32);
  endian_flip32(work->midstate, work->midstate);
}

#define CL_SET_BLKARG(blkvar) status |= clSetKernelArg(*kernel, num++, sizeof(uint), (void *)&blk->blkvar)
#define CL_SET_VARG(args, var) status |= clSetKernelArg(*kernel, num++, args * sizeof(uint), (void *)var)
#define CL_SET_ARG_N(n, var) do { status |= clSetKernelArg(*kernel, n, sizeof(var), (void *)&var); } while (0)
#define CL_SET_ARG_0(var) CL_SET_ARG_N(0, var)
#define CL_SET_ARG(var) CL_SET_ARG_N(num++, var)
#define CL_NEXTKERNEL_SET_ARG_N(n, var) do { kernel++; CL_SET_ARG_N(n, var); } while (0)
#define CL_NEXTKERNEL_SET_ARG_0(var) CL_NEXTKERNEL_SET_ARG_N(0, var)
#define CL_NEXTKERNEL_SET_ARG(var) CL_NEXTKERNEL_SET_ARG_N(num++, var)

static void append_x11(struct _build_kernel_data *data, struct cgpu_info *cgpu, struct _algorithm_t *algorithm)
{
  char buf[255];
  sprintf(buf, " -D SPH_COMPACT_BLAKE_64=%d -D SPH_LUFFA_PARALLEL=%d -D SPH_KECCAK_UNROLL=%u ",
    ((opt_blake_compact) ? 1 : 0), ((opt_luffa_parallel) ? 1 : 0), (unsigned int)opt_keccak_unroll);
  strcat(data->compiler_options, buf);

  sprintf(buf, "ku%u%s%s", (unsigned int)opt_keccak_unroll, ((opt_blake_compact) ? "bc" : ""), ((opt_luffa_parallel) ? "lp" : ""));
  strcat(data->binary_filename, buf);
}


static void append_x13_compiler_options(struct _build_kernel_data *data, struct cgpu_info *cgpu, struct _algorithm_t *algorithm)
{
  char buf[255];

  append_x11(data, cgpu, algorithm);

  sprintf(buf, " -D SPH_HAMSI_EXPAND_BIG=%d -D SPH_HAMSI_SHORT=%d ",
    (unsigned int)opt_hamsi_expand_big, ((opt_hamsi_short) ? 1 : 0));
  strcat(data->compiler_options, buf);

  sprintf(buf, "big%u%s", (unsigned int)opt_hamsi_expand_big, ((opt_hamsi_short) ? "hs" : ""));
  strcat(data->binary_filename, buf);
}

static cl_int queue_bcd_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel = &clState->kernel;
  unsigned int num = 0;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24); 
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}


static const char *lookup_algorithm_alias(const char *lookup_alias, uint8_t *nfactor)
{
#define ALGO_ALIAS(alias, name) \
  if (strcasecmp(alias, lookup_alias) == 0) return name;

  ALGO_ALIAS("bcd", "bcd");
#undef ALGO_ALIAS
#undef ALGO_ALIAS_NF

  return NULL;
}

void set_algorithm(algorithm_t* algo)
{
  strcpy(algo->name, "bcd");
  algo->kernelfile = "";
  algo->type = ALGO_BCD;
  algo->diff_multiplier1 = 1;
  algo->diff_multiplier2 = 1;
  algo->share_diff_multiplier = 1;
  algo->xintensity_shift = 0;
  algo->intensity_shift = 0;
	algo->found_idx = 0xFF;
	algo->diff_numerator = 0xFFFFULL;
	algo->diff1targ = 0x0000ffffUL;
  algo->n_extra_kernels = 0;
  algo->rw_buffer_size = 0;
	algo->cq_properties = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
	algo->regenhash = bcd_regenhash; 
  algo->calc_midstate = NULL;
  algo->prepare_work = NULL;
	algo->queue_kernel = queue_bcd_kernel;
	algo->gen_hash = gen_hash;
	algo->set_compile_options = append_x13_compiler_options;
}

void set_algorithm_nfactor(algorithm_t* algo, const uint8_t nfactor)
{
  algo->nfactor = nfactor;
  algo->n = (1 << nfactor);
}

bool cmp_algorithm(const algorithm_t* algo1, const algorithm_t* algo2)
{
  return (!safe_cmp(algo1->name, algo2->name) && !safe_cmp(algo1->kernelfile, algo2->kernelfile) && (algo1->nfactor == algo2->nfactor));
}
