/* SPDX-License-Identifier: GPL-2.0-or-later */


#ifndef G10_MPI_H
#define G10_MPI_H

#include <linux/types.h>
#include <linux/scatterlist.h>

#define BYTES_PER_MPI_LIMB	(BITS_PER_LONG / 8)
#define BITS_PER_MPI_LIMB	BITS_PER_LONG

typedef unsigned long int mpi_limb_t;
typedef signed long int mpi_limb_signed_t;

struct gcry_mpi {
	int alloced;		
	int nlimbs;		
	int nbits;		
	int sign;		
	unsigned flags;		
	
	
	mpi_limb_t *d;		
};

typedef struct gcry_mpi *MPI;

#define mpi_get_nlimbs(a)     ((a)->nlimbs)


MPI mpi_alloc(unsigned nlimbs);
void mpi_free(MPI a);
int mpi_resize(MPI a, unsigned nlimbs);

MPI mpi_copy(MPI a);


MPI mpi_read_raw_data(const void *xbuffer, size_t nbytes);
MPI mpi_read_from_buffer(const void *buffer, unsigned *ret_nread);
MPI mpi_read_raw_from_sgl(struct scatterlist *sgl, unsigned int len);
void *mpi_get_buffer(MPI a, unsigned *nbytes, int *sign);
int mpi_read_buffer(MPI a, uint8_t *buf, unsigned buf_len, unsigned *nbytes,
		    int *sign);
int mpi_write_to_sgl(MPI a, struct scatterlist *sg, unsigned nbytes,
		     int *sign);


int mpi_mod(MPI rem, MPI dividend, MPI divisor);


int mpi_powm(MPI res, MPI base, MPI exp, MPI mod);


int mpi_cmp_ui(MPI u, ulong v);
int mpi_cmp(MPI u, MPI v);


int mpi_sub_ui(MPI w, MPI u, unsigned long vval);


void mpi_normalize(MPI a);
unsigned mpi_get_nbits(MPI a);
int mpi_test_bit(MPI a, unsigned int n);
int mpi_set_bit(MPI a, unsigned int n);
int mpi_rshift(MPI x, MPI a, unsigned int n);


int mpi_add(MPI w, MPI u, MPI v);
int mpi_sub(MPI w, MPI u, MPI v);
int mpi_addm(MPI w, MPI u, MPI v, MPI m);
int mpi_subm(MPI w, MPI u, MPI v, MPI m);


int mpi_mul(MPI w, MPI u, MPI v);
int mpi_mulm(MPI w, MPI u, MPI v, MPI m);


int mpi_tdiv_r(MPI rem, MPI num, MPI den);
int mpi_fdiv_r(MPI rem, MPI dividend, MPI divisor);




static inline unsigned int mpi_get_size(MPI a)
{
	return a->nlimbs * BYTES_PER_MPI_LIMB;
}
#endif 
