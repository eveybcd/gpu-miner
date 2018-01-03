#ifndef ALGORITHM_H
#define ALGORITHM_H

#ifdef __APPLE_CC__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <inttypes.h>
#include <stdbool.h>
#include "ocl/build_kernel.h"   // For the build_kernel_data type

typedef enum {
  ALGO_UNK,
  ALGO_BCD 
} algorithm_type_t;

extern const char *algorithm_type_str[];

extern void gen_hash(const unsigned char *data, unsigned int len, unsigned char *hash);

struct __clState;
struct _dev_blk_ctx;
struct _build_kernel_data;
struct cgpu_info;
struct work;


typedef struct _algorithm_t {
  char     name[4]; /* Human-readable identifier */
  algorithm_type_t type; //algorithm type
  const char *kernelfile; /* alternate kernel file */
  uint32_t n;        /* N (CPU/Memory tradeoff parameter) */
  uint8_t  nfactor;  /* Factor of N above (n = 2^nfactor) */
  double   diff_multiplier1;
  double   diff_multiplier2;
  double   share_diff_multiplier;
  uint32_t xintensity_shift;
  uint32_t intensity_shift;
  uint32_t found_idx;
  unsigned long long   diff_numerator;
  uint32_t diff1targ;
  size_t n_extra_kernels;
  long rw_buffer_size;
  cl_command_queue_properties cq_properties;
  void(*regenhash)(struct work *);
  void(*calc_midstate)(struct work *);
  void(*prepare_work)(struct _dev_blk_ctx *, uint32_t *, uint32_t *);
  cl_int(*queue_kernel)(struct __clState *, struct _dev_blk_ctx *, cl_uint);
  void(*gen_hash)(const unsigned char *, unsigned int, unsigned char *);
  void(*set_compile_options)(struct _build_kernel_data *, struct cgpu_info *, struct _algorithm_t *);
} algorithm_t;

typedef struct _algorithm_settings_t
{
	const char *name;
	algorithm_type_t type;
	const char *kernelfile;
	double   diff_multiplier1;
	double   diff_multiplier2;
	double   share_diff_multiplier;
	uint32_t xintensity_shift;
	uint32_t intensity_shift;
	uint32_t found_idx;
	unsigned long long   diff_numerator;
	uint32_t diff1targ;
	size_t n_extra_kernels;
	long rw_buffer_size;
	cl_command_queue_properties cq_properties;
	void     (*regenhash)(struct work *);
	void     (*calc_midstate)(struct work *);
	void     (*prepare_work)(struct _dev_blk_ctx *, uint32_t *, uint32_t *);
	cl_int   (*queue_kernel)(struct __clState *, struct _dev_blk_ctx *, cl_uint);
	void     (*gen_hash)(const unsigned char *, unsigned int, unsigned char *);
	void     (*set_compile_options)(build_kernel_data *, struct cgpu_info *, algorithm_t *);
} algorithm_settings_t;

/* Set default parameters based on name. */
void set_algorithm(algorithm_t* algo);

/* Set to specific N factor. */
void set_algorithm_nfactor(algorithm_t* algo, const uint8_t nfactor);

/* Compare two algorithm parameters */
bool cmp_algorithm(const algorithm_t* algo1, const algorithm_t* algo2);

#endif /* ALGORITHM_H */
